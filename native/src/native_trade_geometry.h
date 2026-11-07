#pragma once
#include "native_ports.h"

namespace v3 {
struct NativeDirectedStrip {
  uint64_t id=0; // original56-bit identity; bit63 tags an owned original keyless port path
  bool forward=false;
  bool operator==(const NativeDirectedStrip&)const=default;
};
struct NativeTradeGeometry {
  NativeRouteConnections connections;
  std::vector<NativeDirectedStrip> strips;
  bool complete=true; // false for partial original port geometry or a missing physical transfer
  // Resolved chain ends in the current spline control space. A sea-only lane
  // uses exactly these as its graph endpoints, so geometry and graph never
  // disagree when a current port transfer is absent.
  uint32_t startAnchor=0,endAnchor=0;
};
// Compose the original port-connection mapping with the CURRENT endpoints used
// by013C4E70 and its selected native ProvinceSet. Order only an unambiguous chain
// of those exact strips. No shortest path, nearest coast or branch choice.
// This composition must still be checked visually; it is not a call to the
// original world-market renderer (025E8450 callers observed CShippingLane).
inline std::optional<NativeTradeGeometry> BuildNativeTradeGeometry(const NativeWorldTradeSnapshot& trade,
    uint32_t source,const NativeTradeDestination& destination,std::string* error=nullptr,
    const std::function<bool()>& cancelled={},bool allowPartialPort=false,bool seaOnly=false) {
  auto fail=[&](const char* why)->std::optional<NativeTradeGeometry>{if(error)*error=why;return {};};
  if(cancelled && cancelled())return fail("cancelled");
  if(!trade.geometry || !trade.seaGeography)return fail("current native route geography unavailable");
  NativeRouteConnections connections;
  std::array<uint32_t,2> endpointAnchors{};
  // 1.15 port access splines are the authoritative port data only when the
  // current capture actually produced them. Synthetic/legacy inputs without
  // any transfer keep the original sea-geography port selection instead of
  // silently losing every international shape.
  const bool domesticPorts=trade.domestic && !trade.domestic->portTransfers.empty();
  bool intraRegion=false;
  if(domesticPorts){
    // 1.15.0: port access is a current type-5 spline between a port-side land
    // node and its sea-side anchor. The domestic model draws those access
    // splines as their own port edges, so a lane geometry built for the model
    // must stay sea-only and end at the same sea anchors its graph edge uses.
    // Native international route shapes still include the port splines.
    auto sea=SelectNativeSeaConnections(trade,*trade.seaGeography,destination,error);
    if(!sea)return {};
    connections.sea=std::move(*sea);
    // A lane whose selected path stays inside one sea region has no sea spline;
    // its two port access splines then form the authored chain (valid zero
    // within one region). Only that explicit case keeps the land anchors.
    intraRegion=seaOnly && connections.sea.keys.empty();
    for(size_t i=0;i<2;++i){
      const uint32_t state=i?destination.state:source;
      bool found=false;
      for(const auto& transfer:trade.domestic->portTransfers)
        if(transfer.state==state){
          found=true;
          if(seaOnly && !intraRegion)endpointAnchors[i]=transfer.anchors[1];
          else{connections.ports[i]=transfer.spline;endpointAnchors[i]=transfer.anchors[0];}
          break;
        }
      if(seaOnly && !intraRegion && !found){
        // A state without a current port access still uses the original
        // naval-anchor encoding, matching the model's own endpoint fallback.
        const auto& regions=trade.seaGeography->states;
        const auto region=regions.find(state);
        if(region==regions.end() || region->second.geo==UINT32_MAX)
          return fail("native lane sea anchor unavailable");
        endpointAnchors[i]=((region->second.geo*100)&0x1fffffff)|0x800000;
      }
    }
  }else{
    auto legacy=SelectNativeRouteConnections(trade,*trade.seaGeography,source,destination,error);
    if(!legacy)return {};
    connections=std::move(*legacy);
    const auto& regions=trade.seaGeography->states;
    endpointAnchors[0]=((regions.at(source).geo*100)&0x1fffffff)|0x800000;
    endpointAnchors[1]=((regions.at(destination.state).geo*100)&0x1fffffff)|0x800000;
  }
  // The sea-only shortcut applies only to the domestic transfer path;
  // the legacy port selector must still report a missing endpoint as partial.
  const bool seaOnlyPorts=domesticPorts && seaOnly && !intraRegion;
  bool complete=seaOnlyPorts?true:(connections.ports[0] && connections.ports[1]);
  if((!complete && !allowPartialPort) || (!seaOnlyPorts && !connections.ports[0] && !connections.ports[1]))
    return fail("native endpoint port connection unavailable");
  const auto& network=*trade.geometry;
  if(!network.width || !network.height)return fail("current native map dimensions unavailable");
  std::set<uint64_t> keys(connections.sea.keys.begin(),connections.sea.keys.end()),used;
  for(const auto& port:connections.ports)if(port)keys.insert(*port);
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
  uint32_t start=endpointAnchors[0];
  uint32_t end=endpointAnchors[1];
  if(!complete){
    const uint32_t known=connections.ports[0]?start:end;
    std::optional<uint32_t> loose;
    for(const auto& [point,edges]:incident)if(point!=known && edges.size()==1){
      if(loose)return fail("partial native port geometry has ambiguous ends");loose=point;
    }
    if(!loose)return fail("partial native port geometry has no witnessed loose end");
    // This is the last EXISTING original sea control, not a fabricated shore
    // point or a claim that it is the selected state's physical port.
    if(connections.ports[0])end=*loose;else start=*loose;
  }
  // 1.15 sea-only lanes join the sea anchors of the current port transfers.
  // When a state has no current transfer, its geo-derived naval anchor is not
  // necessarily a control of the selected sea chain. Keep the authored sea
  // chain and its own last control as that end instead of discarding the whole
  // lane; the missing physical port stays marked partial (no shore point is
  // fabricated and the graph uses the same resolved anchor).
  if(seaOnlyPorts && (start==end || !incident.count(start) || !incident.count(end))){
    std::vector<uint32_t> loose;
    for(const auto& [point,edges]:incident)if(edges.size()==1)loose.push_back(point);
    if(loose.size()==2){
      const bool hasStart=incident.count(start)!=0,hasEnd=incident.count(end)!=0;
      if(hasStart && !hasEnd)end=loose[0]==start?loose[1]:loose[0];
      else if(!hasStart && hasEnd)start=loose[0]==end?loose[1]:loose[0];
      else if(!hasStart && !hasEnd)
        // Sea keys are an unordered membership set with canonical endpoint
        // packing, not a source/destination witness. Keep the known lane
        // quantity separate; neither travel direction is drawable here.
        return fail("native lane direction unavailable without a witnessed endpoint");
      if(hasStart!=hasEnd)complete=false;
    }
  }
  if(start==end || !incident.count(start) || !incident.count(end))return fail("native route endpoints are not distinct current port anchors");
  for(const auto& [point,edges]:incident)if(edges.size()!=(point==start || point==end?1u:2u))
    return fail("native route geometry is not a unique endpoint chain");
  NativeTradeGeometry result;result.connections=std::move(connections);result.complete=complete;
  result.startAnchor=start;result.endAnchor=end;
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
