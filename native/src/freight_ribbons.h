#pragma once
#include "freight.h"
#include "ribbon_cache.h"
#include "arrow_geometry.h"
#include "flow_width.h"

namespace v3 {
// Worker-side conversion only; native objects are created later by RibbonCache.
// Opposing traffic is aggregated once per physical strip in market view. The
// glyph follows net allocated movement; width represents total movement.
inline RibbonBatch BuildFreightRibbons(const FreightView& view, const SplineGraph& graph,
    const HeightField& heights, float coordScale=1, float heightScale=0, float heightOffset=0) {
  RibbonBatch ribbons;
  std::unordered_map<uint32_t,const SplineEdge*> edges;
  for (const auto& edge:graph.edges) edges.emplace(edge.id,&edge);
  for (const auto& segment:view.segments) {
    const auto it=edges.find(segment.edge);
    if (it==edges.end() || !std::isfinite(segment.quantity) || segment.quantity<=0) continue;
    const auto& edge=*it->second;
    if (edge.count<2 || uint64_t(edge.first)+edge.count>graph.points.size()) continue;
    std::vector<Vec3> points;
    points.reserve(edge.count);
    const bool forward=segment.netQuantity>=0;
    for (uint32_t i=0;i<edge.count;++i) {
      const auto& p=graph.points[edge.first+(forward?i:edge.count-i-1)];
      points.push_back({p.x,heights.Sample(p.x,p.z),p.z});
    }
    static constexpr const char* marketPalettes[]={"ironroadTrafficGreen","ironroadTrafficLime",
        "ironroadTrafficYellow","ironroadTrafficOrange","ironroadTrafficRed"};
    const char* palette=segment.direction>0?"ironroadBuildingOutput":
        (segment.direction<0?"ironroadBuildingInput":marketPalettes[std::min<unsigned>(segment.band,4)]);
    auto native=NativeArrowPoints(points,heights.z1,coordScale,heightScale,heightOffset,heights.Valid());
    auto ribbon=MakeRibbon(FlowWidthStyle(palette,segment.quantity),std::move(native));
    if (ribbon) ribbons.push_back(std::move(ribbon));
  }
  return PrepareRibbonBatch(std::move(ribbons));
}
} // namespace v3
