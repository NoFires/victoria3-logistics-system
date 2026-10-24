#include "preview_worker.h"
#include "arrow_geometry.h"
#include "flow_width.h"

namespace v3 {
PreviewWorker::PreviewWorker(std::shared_ptr<const RouteGeography> geography)
    : m_geography(std::move(geography)),
      m_worker([this](const PreviewRequest& request, const Worker::Cancelled& cancelled) {
        return Compute(request, cancelled);
      }) {}

PreviewResult PreviewWorker::Compute(const PreviewRequest& request, const Worker::Cancelled& cancelled) {
  LARGE_INTEGER started, ended, frequency;
  QueryPerformanceCounter(&started); QueryPerformanceFrequency(&frequency);
  PreviewResult result;
  result.from = request.from; result.to = request.to; result.mode = request.mode;
  const auto& geo = *m_geography;
  const auto from = std::find_if(geo.states.begin(), geo.states.end(), [&](const auto& state) {
    return state.id == request.from;
  });
  const auto to = std::find_if(geo.states.begin(), geo.states.end(), [&](const auto& state) {
    return state.id == request.to;
  });
  if (from == geo.states.end() || to == geo.states.end() || from == to) {
    result.error = "select two different states"; return result;
  }
  const auto key = std::make_tuple(geo.revision, request.from, request.to, request.mode);
  auto found = m_routes.find(key);
  result.routeCacheHit = found != m_routes.end();
  if (found == m_routes.end()) {
    RouteValue route;
    if (request.mode == 2) {
      auto authored = geo.splines.Route(*from, *to, geo.heights, 31, cancelled);
      route.points = std::move(authored.points); route.usedNodes = int(authored.edgeIds.size());
    } else if (request.mode == 1) {
      route.points = BuildProvinceRoute(*from, *to, geo.provinces, geo.heights, &route.usedNodes, cancelled);
    } else if (request.mode == 0) {
      route.points = BuildStraightRoute(*from, *to, geo.heights, 48);
    }
    if (cancelled()) return result;
    // Bound the routing cache separately from the native pool. Failed searches
    // are cached too; changing style/calibration never re-runs the same search.
    if (m_routes.size() >= 128) {
      auto oldest = std::min_element(m_routes.begin(), m_routes.end(), [](const auto& a, const auto& b) {
        return a.second.used < b.second.used;
      });
      m_routes.erase(oldest);
    }
    found = m_routes.emplace(key, std::move(route)).first;
  }
  found->second.used = ++m_clock;
  result.usedNodes = found->second.usedNodes; result.rawPoints = found->second.points.size();
  if (found->second.points.empty()) {
    result.error = request.mode == 2 ? "no connected authored route; choose province/chord explicitly"
                                    : "no connected province route; choose chord explicitly";
  } else {
    auto points = NativeArrowPoints(found->second.points, geo.heights.z1, request.coordScale,
        request.heightScale, request.heightOffset, geo.heights.Valid());
    auto ribbon = MakeRibbon(FlowWidthStyle(request.style, request.previewConsumption), std::move(points));
    if (ribbon) result.ribbons = PrepareRibbonBatch({std::move(ribbon)});
    else result.error = "invalid, degenerate or oversized route geometry";
  }
  QueryPerformanceCounter(&ended);
  result.computeMs = double(ended.QuadPart - started.QuadPart) * 1000.0 / double(frequency.QuadPart);
  return result;
}
} // namespace v3
