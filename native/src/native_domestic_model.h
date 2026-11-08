#pragma once
#include "native_trade_geometry.h"
#include <type_traits>
#include <tuple>

namespace v3 {
struct NativeDomesticModel {
  FreightResult allocation;
  std::vector<NativeTradeGeometry> edges; // model edge index -> current native strips
  std::vector<bool> seaEdges; // same indices; current port lane vs current land connection
  // Bit63 distinguishes copied original keyless paths from native spline IDs.
  std::map<uint64_t,std::vector<NativeSplinePoint>> portPaths;
  uint32_t landConnections=0,seaConnections=0,unavailableSea=0,partialSea=0;
  // Diagnostic identities of the lanes whose geometry chain is not complete.
  std::vector<uint32_t> unavailableSeaIds,partialSeaIds;
  std::map<uint32_t,std::string> unavailableSeaReasons;
};
// Only the domestic allocation is a model. Its network consists of CURRENT
// original land connections and original domestic port lanes. Sea lane shapes
// are composed from their original membership, never recomputed over a sea map.
inline std::optional<NativeDomesticModel> BuildNativeDomesticModel(const EconomySnapshot& snapshot,
    const std::function<bool()>& cancelled={},std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<NativeDomesticModel>{if(error)*error=why;return {};};
  if(!snapshot.nativeTrade || !snapshot.nativeTrade->domestic || !snapshot.nativeTrade->geometry ||
     !snapshot.nativeTrade->seaGeography)return fail("current domestic sources unavailable");
  const auto& trade=*snapshot.nativeTrade;const auto& network=*trade.geometry;
  // Content identity ignores registry enumeration. Graph tie order must do the
  // same, including parallel equal-cost physical connections.
  auto ordered=[](const auto& values){
    using Item=typename std::remove_cvref_t<decltype(values)>::value_type;
    std::vector<const Item*> result;result.reserve(values.size());
    for(const auto& value:values)result.push_back(&value);
    std::sort(result.begin(),result.end(),[](const auto* a,const auto* b){return a->id<b->id;});
    return result;
  };
  NativeDomesticModel result;EconomySnapshot model;model.world=snapshot.world;model.revision=snapshot.revision;model.goods=snapshot.goods;
  SplineGraph graph;graph.width=float(network.width);graph.height=float(network.height);
  std::map<uint32_t,uint32_t> markets;
  for(const auto* current:ordered(trade.states)){
    const auto& state=*current;
    if(!markets.emplace(state.id,state.market).second)return fail("duplicate domestic state");
  }
  std::set<uint32_t> goods;
  for(const auto& good:model.goods)goods.insert(good.id);
  std::map<std::pair<uint32_t,uint32_t>,StateOrders> orders;
  for(const auto& order:trade.domestic->orders){
    if(!markets.count(order.state) || markets.at(order.state)!=order.market || !goods.count(order.good) ||
       order.production<0 || order.consumption<0 || !orders.emplace(std::pair{order.state,order.good},order).second)
      return fail("invalid domestic local orders");
  }
  // Original detailed exports already move these goods. Add their inputs and
  // outputs to the respective local balance BEFORE modeling remaining domestic
  // transfers. Aggregate at native precision, then quantize once to milli-orders.
  std::map<std::pair<uint32_t,uint32_t>,std::array<int64_t,2>> external;
  for(const auto& pair:trade.statePairs){
    if(!markets.count(pair.source) || !markets.count(pair.destination))return fail("unknown native residual endpoint");
    for(const auto& amount:pair.goods){
      if(!goods.count(amount.good) || amount.valueFixed<0)return fail("invalid native residual good");
      auto& outgoing=external[{pair.source,amount.good}][1];auto& incoming=external[{pair.destination,amount.good}][0];
      if(outgoing>INT64_MAX-amount.valueFixed || incoming>INT64_MAX-amount.valueFixed)return fail("native residual quantity overflow");
      outgoing+=amount.valueFixed;incoming+=amount.valueFixed;
    }
  }
  for(const auto& [key,amount]:external){
    auto [where,inserted]=orders.try_emplace(key,StateOrders{key.first,UINT32_MAX,markets.at(key.first),key.second,0,0});
    auto& order=where->second;const auto imported=amount[0]/100,exported=amount[1]/100;
    if(order.production>INT64_MAX-imported || order.consumption>INT64_MAX-exported)return fail("domestic residual orders overflow");
    order.production+=imported;order.consumption+=exported;
  }
  for(const auto& [key,order]:orders)model.orders.push_back(order);
  // State orders remain aggregated at a witnessed primary land hub when known.
  // Physical transit never merges a state's other access points.
  using Area=std::tuple<uint32_t,uint32_t,uint32_t>; // market, definition, current geo
  using Node=std::pair<Area,uint64_t>; // exact control, or opaque missing sea endpoint
  std::map<uint32_t,Area> areas;
  std::map<uint32_t,std::vector<uint32_t>> access;
  for(const auto& [state,market]:markets){
    const auto region=trade.seaGeography->states.find(state);
    areas[state]=region==trade.seaGeography->states.end() || region->second.definition==UINT32_MAX ||
        region->second.geo==UINT32_MAX?Area{market,UINT32_MAX,state}:
        Area{market,region->second.definition,region->second.geo};
    access.emplace(state,std::vector<uint32_t>{});
  }
  std::map<Node,uint32_t> nodes;
  auto node=[&](uint32_t state,uint64_t control){
    auto [where,inserted]=nodes.emplace(Node{areas.at(state),control},uint32_t(graph.nodes.size()));
    if(inserted)graph.nodes.push_back({where->second,0,0}); // owned indices, not map identities
    access.at(state).push_back(where->second);return where->second;
  };
  auto add=[&](uint32_t from,uint32_t to,uint32_t type,NativeTradeGeometry geometry,
      std::optional<double> nativeDistance={},std::optional<std::array<uint64_t,2>> endpoints={}){
    if(!markets.count(from) || !markets.count(to) || markets.at(from)!=markets.at(to))return false;
    double length=0;
    if(nativeDistance)length=*nativeDistance;
    else for(const auto& strip:geometry.strips){
      const auto found=network.strips.find(strip.id);if(found==network.strips.end() || found->second.size()<2)return false;
      const NativeSplinePoint* previous=nullptr;
      for(uint32_t id:found->second){
        const auto p=network.points.find(id);
        if(p==network.points.end() || !std::isfinite(p->second.x) || !std::isfinite(p->second.z))return false;
        if(previous)length+=std::hypot(double(p->second.x)-previous->x,double(p->second.z)-previous->z);
        previous=&p->second;
      }
    }
    if(length<0 || (!nativeDistance && length==0) || !std::isfinite(float(length)))return false;
    if(!endpoints){
      if(geometry.strips.empty())return false;
      const auto& first=geometry.strips.front();const auto& last=geometry.strips.back();
      const auto& a=network.strips.at(first.id);const auto& b=network.strips.at(last.id);
      endpoints=std::array<uint64_t,2>{first.forward?a.front():a.back(),last.forward?b.back():b.front()};
    }
    const auto a=node(from,(*endpoints)[0]),b=node(to,(*endpoints)[1]);
    const auto id=uint32_t(result.edges.size());
    result.edges.push_back(std::move(geometry));result.seaEdges.push_back(type==2 || type==4);
    // Split states at the same witnessed port can share a zero-length terminal;
    // a self-loop grants no extra connectivity or physical transport distance.
    if(a!=b){
      graph.edges.push_back({id,a,b,type,0,0,float(length)});
      if(type==1)model.builtRailEdges.insert(id);
    }
    return true;
  };
  std::map<uint64_t,std::array<int64_t,2>> anonymousPositions;
  auto addLand=[&](const NativeLandConnection& connection){
    if(connection.type!=2 && connection.type!=3 && connection.type!=4)return false;
    if(connection.direct){
      const auto& leg=*connection.direct;
      if(connection.spline!=UINT64_MAX || connection.type==4 || !connection.forward || leg.distanceFixed<0)return false;
      std::vector<NativeSplinePoint> points;
      for(size_t side=0;side<2;++side){
        const auto anchor=leg.anchors[side];NativeSplinePoint p{};
        if((anchor&~uint64_t(UINT32_MAX))==kNativeAnonymousLandNode){
          const auto [where,inserted]=anonymousPositions.emplace(anchor,leg.positions[side]);
          if(!inserted && where->second!=leg.positions[side])return false;
          p={float(double(leg.positions[side][0])/100000),float(double(leg.positions[side][1])/100000)};
          if(!network.width || !network.height || p.x<-float(network.width) || p.x>2.0f*network.width ||
             p.z<0 || p.z>network.height)return false;
        }else{
          if(anchor>=0x1fffffff)return false;
          const auto point=network.points.find(uint32_t(anchor));
          if(point==network.points.end() || double(leg.positions[side][0])!=std::floor(double(point->second.x))*100000 ||
             double(leg.positions[side][1])!=std::floor(double(point->second.z))*100000)return false;
          p=point->second;
        }
        if(!std::isfinite(p.x) || !std::isfinite(p.z))return false;
        points.push_back(p);
      }
      NativeTradeGeometry shape;
      if(points[0].x!=points[1].x || points[0].z!=points[1].z){
        const uint64_t strip=(uint64_t(3)<<62)|(uint64_t(connection.id)<<6);
        if(!result.portPaths.emplace(strip,std::move(points)).second)return false;
        shape.strips.push_back({strip,true});
      }else shape.complete=leg.distanceFixed==0;
      return add(connection.source,connection.destination,0,std::move(shape),double(leg.distanceFixed)/100000,leg.anchors);
    }
    const auto binding=network.connections.find(connection.spline);
    if(binding==network.connections.end() || binding->second.empty() || (connection.spline&63)>1)return false;
    NativeTradeGeometry shape;uint32_t anchor=uint32_t(connection.spline>>6)&0x1fffffff;
    const auto start=anchor;
    for(uint64_t strip:binding->second){
      const auto controls=network.strips.find(strip);
      if(controls==network.strips.end() || controls->second.size()<2 || (strip&63)!=(connection.spline&63))return false;
      const auto& points=controls->second;const bool along=points.front()==anchor;
      if(!along && points.back()!=anchor)return false;
      shape.strips.push_back({strip,along});anchor=along?points.back():points.front();
    }
    if(anchor!=uint32_t(connection.spline>>35) || anchor==start)return false;
    if(!connection.forward){std::reverse(shape.strips.begin(),shape.strips.end());for(auto& strip:shape.strips)strip.forward=!strip.forward;}
    return add(connection.source,connection.destination,connection.type==4?1:0,std::move(shape));
  };
  for(const auto* connection:ordered(trade.domestic->land)){
    if(cancelled && cancelled())return fail("cancelled");
    if(connection->source==connection->destination || !addLand(*connection))return fail("invalid current domestic land connection");
    ++result.landConnections;
  }
  std::map<uint32_t,uint64_t> portAnchors;
  for(const auto* transfer:ordered(trade.domestic->portTransfers))
    portAnchors.emplace(transfer->state,transfer->anchors[1]);
  for(const auto* current:ordered(trade.domestic->shipping.lanes)){
    const auto& lane=*current;
    if(cancelled && cancelled())return fail("cancelled");
    if(lane.kind!=0 || lane.fromState==lane.toState || !markets.count(lane.fromState) || !markets.count(lane.toState))
      return fail("invalid domestic original port lane");
    if(markets.at(lane.fromState)!=markets.at(lane.toState))continue;
    if(lane.provinces.empty())continue; // known empty ORIGINAL path grants no connection
    if(lane.travelDistanceFixed<0)return fail("invalid current native shipping distance");
    NativeTradeDestination destination;destination.state=lane.toState;destination.selected=0;
    NativeTradePath path;path.provinces=lane.provinces;
    // Per-lane results take precedence, including a known denied/empty list.
    // A missing entry in an engaged map is not a country/registry fallback.
    // Production capture is still unbound; the legacy path remains unchanged.
    const auto& access=trade.domestic->laneStraitAccess?trade.domestic->laneStraitAccess:trade.domestic->straitAccess;
    if(access){
      const auto found=access->find(trade.domestic->laneStraitAccess?lane.id:lane.country);
      if(found==access->end() || !std::is_sorted(found->second.begin(),found->second.end()) ||
         std::adjacent_find(found->second.begin(),found->second.end())!=found->second.end())
        return fail("invalid current domestic strait access");
      for(uint32_t id:found->second)
        if(std::none_of(trade.straits.begin(),trade.straits.end(),[&](const auto& strait){return strait.id==id;}))
          return fail("domestic strait access references unavailable strait");
      path.straits=found->second;
    }else for(const auto& strait:trade.straits)path.straits.push_back(strait.id);
    destination.candidates.push_back(std::move(path));
    // Current CShippingLane is the connectivity fact. Original native travel
    // distance(+60), including a valid zero within one sea region, is its model
    // route metric. A missing graphics key cannot erase this known connection.
    auto endpoint=[&](uint32_t state,uint32_t side)->uint64_t{
      // 1.15.0: the current port-access transfer carries the sea-side anchor
      // the lane physically joins. The state region geo is a different space
      // and no longer matches the port anchor, so prefer the witnessed port.
      const auto port=portAnchors.find(state);
      if(port!=portAnchors.end())return port->second;
      const auto region=trade.seaGeography->states.find(state);
      if(region!=trade.seaGeography->states.end() && region->second.definition!=UINT32_MAX && region->second.geo!=UINT32_MAX)
        return ((region->second.geo*100)&0x1fffffff)|0x800000; // original naval anchor encoding
      // Known original lane, unknown port identity: keep its endpoint isolated
      // from roads/other lanes, rather than using a loose graphics end as a port.
      return (uint64_t(1)<<63)|(uint64_t(lane.id)<<1)|side;
    };
    // The port access splines are added as their own edges below; keep the
    // lane geometry sea-only and let its resolved chain ends define the graph
    // endpoints, so geometry and graph stay identical even without transfers.
    std::string laneError;
    auto shape=BuildNativeTradeGeometry(trade,lane.fromState,destination,&laneError,cancelled,true,true);
    NativeTradeGeometry available;
    std::array<uint64_t,2> laneEndpoints{endpoint(lane.fromState,0),endpoint(lane.toState,1)};
    if(!shape){if(cancelled && cancelled())return fail("cancelled");++result.unavailableSea;
      result.unavailableSeaIds.push_back(lane.id);
      result.unavailableSeaReasons[lane.id]=laneError.empty()?"unknown":laneError;
      available.complete=false;}
    else{
      available=std::move(*shape);
      laneEndpoints={available.startAnchor,available.endAnchor};
      if(!available.complete){++result.partialSea;result.partialSeaIds.push_back(lane.id);}
    }
    if(!add(lane.fromState,lane.toState,2,std::move(available),double(lane.travelDistanceFixed)/kNativeShippingDistanceScale,
        laneEndpoints))
      return fail("invalid domestic port lane scope or distance");
    ++result.seaConnections;
  }
  for(const auto* connection:ordered(trade.domestic->localLand)){
    if(cancelled && cancelled())return fail("cancelled");
    if(connection->source!=connection->destination || !addLand(*connection))return fail("invalid current intra-state connection");
  }
  std::set<uint32_t> portIds;
  for(const auto* transfer:ordered(trade.domestic->portTransfers)){
    if(cancelled && cancelled())return fail("cancelled");
    if(!areas.count(transfer->state) || transfer->id==UINT32_MAX || !portIds.insert(transfer->id).second ||
       transfer->anchors[0]==transfer->anchors[1] || (transfer->spline&63)!=3)
      return fail("invalid owned native port transfer");
    const auto binding=network.connections.find(transfer->spline);
    if(binding==network.connections.end() || binding->second.empty())
      return fail("current port transfer has no native geometry");
    // Keep the port spline's actual anchors. A matching primary-hub suffix
    // cannot substitute for the separate current cross-network access leg.
    std::vector<uint32_t> controls;
    {
      uint32_t anchor=uint32_t(transfer->spline>>6)&0x1fffffff;const auto start=anchor;
      for(uint64_t strip:binding->second){
        const auto found=network.strips.find(strip);
        if(found==network.strips.end() || found->second.size()<2 || (strip&63)!=(transfer->spline&63))
          return fail("invalid current port transfer strip");
        const auto& points=found->second;const bool along=points.front()==anchor;
        if(!along && points.back()!=anchor)return fail("disconnected current port transfer spline");
        if(along){for(uint32_t c:points)controls.push_back(c);anchor=points.back();}
        else{for(size_t i=points.size();i-->0;)controls.push_back(points[i]);anchor=points.front();}
      }
      if(anchor!=uint32_t(transfer->spline>>35) || anchor==start)
        return fail("current port transfer endpoints differ");
    }
    std::vector<uint32_t> distinct;
    for(uint32_t c:controls)if(distinct.empty()||distinct.back()!=c)distinct.push_back(c);
    if(distinct.size()<2 || transfer->anchors[0]!=distinct.front() && transfer->anchors[0]!=distinct.back())
      return fail("native port transfer land anchor not on spline");
    NativeTradeGeometry shape;
    uint32_t anchor=uint32_t(transfer->spline>>6)&0x1fffffff;
    for(uint64_t strip:binding->second){
      const auto found=network.strips.find(strip);
      const auto& points=found->second;const bool along=points.front()==anchor;
      shape.strips.push_back({strip,along});anchor=along?points.back():points.front();
    }
    if(transfer->anchors[0]!=distinct.front()){
      std::reverse(shape.strips.begin(),shape.strips.end());
      for(auto& strip:shape.strips)strip.forward=!strip.forward;
    }
    // 1.15.0 port access uses its own model fee1, not the sea tariff0.25.
    if(!add(transfer->state,transfer->state,4,std::move(shape),{},
        std::array<uint64_t,2>{transfer->anchors[0],transfer->anchors[1]}))
      return fail("invalid native port transfer scope or distance");
  }
  for(const auto* current:ordered(trade.domestic->portAccess)){
    const auto& access=*current;
    if(cancelled && cancelled())return fail("cancelled");
    if(!areas.count(access.state) || access.id==UINT32_MAX || !portIds.insert(access.id).second ||
       access.anchors[0]==access.anchors[1] || access.distanceFixed<0)
      return fail("invalid owned native port access");
    std::vector<NativeSplinePoint> points;
    bool geometryKnown=true;
    for(size_t side=0;side<2;++side){
      const auto point=network.points.find(access.anchors[side]);
      if(point==network.points.end()){geometryKnown=false;continue;}
      if(!std::isfinite(point->second.x) || !std::isfinite(point->second.z) ||
         double(access.positions[side][0])!=std::floor(double(point->second.x))*100000 ||
         double(access.positions[side][1])!=std::floor(double(point->second.z))*100000)
        return fail("native port access differs from current controls");
      points.push_back(point->second);
    }
    NativeTradeGeometry shape;
    if(geometryKnown && (points[0].x!=points[1].x || points[0].z!=points[1].z)){
      const uint64_t strip=(uint64_t(1)<<63)|(uint64_t(access.id)<<6);
      if(!result.portPaths.emplace(strip,std::move(points)).second)return fail("duplicate native port access handle");
      shape.strips.push_back({strip,true});
    }else shape.complete=geometryKnown && access.distanceFixed==0;
    if(!add(access.state,access.state,4,std::move(shape),double(access.distanceFixed)/100000,
        std::array<uint64_t,2>{access.anchors[0],access.anchors[1]}))
      return fail("invalid native port access scope or distance");
  }
  // Hub injection points belong to their own state; a state with several
  // witnessed hubs keeps all of them. A member state without its own hub
  // keeps borrowing exactly one area hub — but only while the area's hub
  // set is unambiguous (the previous single-hub behaviour).
  std::map<uint32_t,std::vector<uint32_t>> stateHubs;
  std::map<Area,std::vector<uint32_t>> areaHubs;
  for(const auto& [state,controls]:trade.domestic->landHubs){
    if(!areas.count(state))return fail("invalid current domestic land hub");
    for(const auto control:controls){
      if(!network.points.count(control) && std::none_of(trade.domestic->portAccess.begin(),trade.domestic->portAccess.end(),
          [&](const auto& port){return port.state==state && port.anchors[0]==control;}))
        return fail("invalid current domestic land hub");
      const auto index=node(state,control);
      stateHubs[state].push_back(index);
      areaHubs[areas.at(state)].push_back(index);
    }
  }
  const auto dedupe=[](auto& map){
    for(auto& [key,list]:map){
      std::sort(list.begin(),list.end());
      list.erase(std::unique(list.begin(),list.end()),list.end());
    }
  };
  dedupe(stateHubs);dedupe(areaHubs);
  for(auto& [state,points]:access){
    if(auto own=stateHubs.find(state);own!=stateHubs.end()){
      std::vector<uint32_t> kept;
      for(const auto point:points)
        if(std::binary_search(own->second.begin(),own->second.end(),point))kept.push_back(point);
      std::sort(kept.begin(),kept.end());
      kept.erase(std::unique(kept.begin(),kept.end()),kept.end());
      points=std::move(kept);
      continue;
    }
    if(auto area=areaHubs.find(areas.at(state));
        area!=areaHubs.end() && area->second.size()==1){points={area->second.front()};continue;}
    std::sort(points.begin(),points.end());points.erase(std::unique(points.begin(),points.end()),points.end());
  }
  FreightPolicy policy;policy.graphRevision=snapshot.revision;
  result.allocation=AllocateFreight(model,graph,policy,cancelled,access);
  if(!result.allocation.complete){if(error)*error=result.allocation.error;return {};}
  if(error)error->clear();return result;
}
} // namespace v3
