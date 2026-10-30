#pragma once
#include "native_ports.h"

namespace v3 {
struct NativeDirectedStrip {
  uint64_t id=0; // full original56-bit identity, never a truncated graph edge
  bool forward=false;
  bool operator==(const NativeDirectedStrip&)const=default;
};
struct NativeTradeGeometry {
  NativeRouteConnections connections;
  std::vector<NativeDirectedStrip> strips;
  bool complete=true; // false only for witnessed partial original port-lane geometry
};
// Compose the original port-connection mapping with the CURRENT endpoints used
// by013C4E70 and its selected native ProvinceSet. Order only an unambiguous chain
// of those exact strips. No shortest path, nearest coast or branch choice.
// This composition must still be checked visually; it is not a call to the
// original world-market renderer (025E8450 callers observed CShippingLane).
inline std::optional<NativeTradeGeometry> BuildNativeTradeGeometry(const NativeWorldTradeSnapshot& trade,
    uint32_t source,const NativeTradeDestination& destination,std::string* error=nullptr,
    const std::function<bool()>& cancelled={},bool allowPartialPort=false) {
  auto fail=[&](const char* why)->std::optional<NativeTradeGeometry>{if(error)*error=why;return {};};
  if(cancelled && cancelled())return fail("cancelled");
  if(!trade.geometry || !trade.seaGeography)return fail("current native route geography unavailable");
  auto connections=SelectNativeRouteConnections(trade,*trade.seaGeography,source,destination,error);
  if(!connections)return {};
  const bool complete=connections->ports[0] && connections->ports[1];
  if((!complete && !allowPartialPort) || (!connections->ports[0] && !connections->ports[1]))
    return fail("native endpoint port connection unavailable");
  const auto& network=*trade.geometry;
  if(!network.width || !network.height)return fail("current native map dimensions unavailable");
  std::set<uint64_t> keys(connections->sea.keys.begin(),connections->sea.keys.end()),used;
  for(const auto& port:connections->ports)if(port)keys.insert(*port);
  std::map<uint32_t,std::vector<uint64_t>> incident;
  for(uint64_t key:keys){
    if(cancelled && cancelled())return fail("cancelled");
    const auto connection=network.connections.find(key);
    if(connection==network.connections.end() || connection->second.empty())return fail("native route connection has no geometry");
    uint32_t current=uint32_t(key>>6)&0x1fffffff;
    for(uint64_t id:connection->second){
      const auto strip=network.strips.find(id);
      if(strip==network.strips.end() || (id&63)!=(key&63) || strip->second.size()<2)
        return fail("invalid native route strip");
      const auto& controls=strip->second;
      if(current==controls.front())current=controls.back();
      else if(current==controls.back())current=controls.front();
      else return fail("disconnected native connection control order");
      if(!used.insert(id).second)continue; // one physical strip, once per flow
      double length=0;const NativeSplinePoint* previous=nullptr;
      for(uint32_t ref:controls){
        const auto point=network.points.find(ref);
        if(point==network.points.end() || !std::isfinite(point->second.x) || !std::isfinite(point->second.z))
          return fail("unavailable native route control point");
        if(previous)length+=std::hypot(double(point->second.x)-previous->x,double(point->second.z)-previous->z);
        previous=&point->second;
      }
      if(!(length>0) || !std::isfinite(length))return fail("degenerate native route strip");
      incident[controls.front()].push_back(id);incident[controls.back()].push_back(id);
    }
    if(current!=uint32_t(key>>35))return fail("native connection endpoint mismatch");
  }
  const auto& regions=trade.seaGeography->states;
  // Same low29 naval anchor encoding used by original025E8070/type3 keys.
  uint32_t start=((regions.at(source).geo*100)&0x1fffffff)|0x800000;
  uint32_t end=((regions.at(destination.state).geo*100)&0x1fffffff)|0x800000;
  if(!complete){
    const uint32_t known=connections->ports[0]?start:end;
    std::optional<uint32_t> loose;
    for(const auto& [point,edges]:incident)if(point!=known && edges.size()==1){
      if(loose)return fail("partial native port geometry has ambiguous ends");loose=point;
    }
    if(!loose)return fail("partial native port geometry has no witnessed loose end");
    // This is the last EXISTING original sea control, not a fabricated shore
    // point or a claim that it is the selected state's physical port.
    if(connections->ports[0])end=*loose;else start=*loose;
  }
  if(start==end || !incident.count(start) || !incident.count(end))return fail("native route endpoints are not distinct current port anchors");
  for(const auto& [point,edges]:incident)if(edges.size()!=(point==start || point==end?1u:2u))
    return fail("native route geometry is not a unique endpoint chain");
  NativeTradeGeometry result;result.connections=std::move(*connections);result.complete=complete;
  std::set<uint64_t> visited;uint32_t current=start;
  while(current!=end){
    if(cancelled && cancelled())return fail("cancelled");
    const auto next=std::find_if(incident.at(current).begin(),incident.at(current).end(),
        [&](uint64_t edge){return !visited.count(edge);});
    if(next==incident.at(current).end())return fail("native route chain does not reach destination");
    const auto& strip=network.strips.at(*next);
    const bool forward=strip.front()==current;
    result.strips.push_back({*next,forward});visited.insert(*next);
    current=forward?strip.back():strip.front();
  }
  if(visited!=used)return fail("native route contains disconnected geometry");
  if(error)error->clear();return result;
}
} // namespace v3
