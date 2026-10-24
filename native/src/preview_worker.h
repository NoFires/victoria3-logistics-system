#pragma once
#include "latest_worker.h"
#include "ribbon_cache.h"
#include "route_geography.h"
#include <map>
#include <tuple>

namespace v3 {
struct PreviewRequest {
  int from = 0, to = 0, mode = 2; // stable state IDs, not roster positions
  std::string style;
  float coordScale = 1, heightScale = 0, heightOffset = 0;
  double previewConsumption = 100; // explicit manual preview, never a live economy reading
};
struct PreviewResult {
  int from = 0, to = 0, mode = 2, usedNodes = 0;
  size_t rawPoints = 0;
  bool routeCacheHit = false;
  double computeMs = 0;
  std::string error;
  RibbonBatch ribbons;
};
class PreviewWorker {
 public:
  using Worker = LatestWorker<PreviewRequest, PreviewResult>;
  explicit PreviewWorker(std::shared_ptr<const RouteGeography> geography);
  uint64_t Request(PreviewRequest request) { return m_worker.Submit(std::move(request)); }
  uint64_t Cancel() { return m_worker.Cancel(); }
  uint64_t Generation() const { return m_worker.Generation(); }
  std::shared_ptr<const Worker::Reply> Poll() { return m_worker.Poll(); }
 private:
  PreviewResult Compute(const PreviewRequest&, const Worker::Cancelled&);
  struct RouteValue {
    std::vector<Vec3> points;
    int usedNodes = 0;
    uint64_t used = 0;
  };
  std::shared_ptr<const RouteGeography> m_geography;
  std::map<std::tuple<uint64_t,int,int,int>, RouteValue> m_routes;
  uint64_t m_clock = 0;
  Worker m_worker; // destroyed/joined BEFORE the data/cache it uses
};
} // namespace v3
