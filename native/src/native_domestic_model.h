#pragma once
#include "native_trade_geometry.h"
#include <type_traits>

namespace v3 {
struct NativeDomesticModel {
  FreightResult allocation;
  std::vector<NativeTradeGeometry> edges; // model edge index -> current native strips
  std::vector<bool> seaEdges; // same indices; current port lane vs current land connection
  uint32_t landConnections=0,seaConnections=0,unavailableSea=0,partialSea=0;
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
    model.stateTerminals.emplace(state.id,uint32_t(graph.nodes.size()));
    graph.nodes.push_back({state.id,0,0}); // terminals are actual states; coordinates do not select them
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
  auto add=[&](uint32_t from,uint32_t to,uint32_t type,NativeTradeGeometry geometry,std::optional<double> nativeDistance={}){
    if(!markets.count(from) || !markets.count(to) || from==to || markets.at(from)!=markets.at(to))return false;
    double length=0;
    for(const auto& strip:geometry.strips){
      const auto found=network.strips.find(strip.id);if(found==network.strips.end() || found->second.size()<2)return false;
      const NativeSplinePoint* previous=nullptr;
      for(uint32_t id:found->second){
        const auto p=network.points.find(id);if(p==network.points.end())return false;
        if(previous)length+=std::hypot(double(p->second.x)-previous->x,double(p->second.z)-previous->z);
        previous=&p->second;
      }
    }
    if(nativeDistance)length=*nativeDistance;
    if(length<0 || (!nativeDistance && length==0) || !std::isfinite(float(length)))return false;
    const auto id=uint32_t(graph.edges.size());
    graph.edges.push_back({id,model.stateTerminals.at(from),model.stateTerminals.at(to),type,0,0,float(length)});
    if(type==1)model.builtRailEdges.insert(id);
    result.edges.push_back(std::move(geometry));result.seaEdges.push_back(type==2);return true;
  };
  for(const auto* current:ordered(trade.domestic->land)){
    const auto& connection=*current;
    if(cancelled && cancelled())return fail("cancelled");
    if(connection.type!=2 && connection.type!=3 && connection.type!=4)return fail("unavailable domestic land connection");
    const auto binding=network.connections.find(connection.spline);
    if(binding==network.connections.end() || binding->second.empty() || (connection.spline&63)>1)
      return fail("current domestic land geometry unavailable");
    NativeTradeGeometry shape;uint32_t anchor=uint32_t(connection.spline>>6)&0x1fffffff;
    for(uint64_t strip:binding->second){
      const auto controls=network.strips.find(strip);
      if(controls==network.strips.end() || controls->second.size()<2)return fail("invalid current domestic land strip");
      const auto& points=controls->second;const bool along=points.front()==anchor;
      if(!along && points.back()!=anchor)return fail("disconnected current domestic land geometry");
      shape.strips.push_back({strip,along});anchor=along?points.back():points.front();
    }
    if(anchor!=uint32_t(connection.spline>>35))return fail("current domestic land endpoint mismatch");
    if(!connection.forward){std::reverse(shape.strips.begin(),shape.strips.end());for(auto& strip:shape.strips)strip.forward=!strip.forward;}
    if(!add(connection.source,connection.destination,connection.type==4?1:0,std::move(shape)))
      return fail("invalid domestic land scope or geometry");
    ++result.landConnections;
  }
  for(const auto* current:ordered(trade.domestic->shipping.lanes)){
    const auto& lane=*current;
    if(cancelled && cancelled())return fail("cancelled");
    if(lane.kind!=0 || !markets.count(lane.fromState) || !markets.count(lane.toState))return fail("invalid domestic original port lane");
    if(markets.at(lane.fromState)!=markets.at(lane.toState))continue;
    if(lane.provinces.empty())continue; // known empty ORIGINAL path grants no connection
    if(lane.travelDistanceFixed<0)return fail("invalid current native shipping distance");
    NativeTradeDestination destination;destination.state=lane.toState;destination.selected=0;
    NativeTradePath path;path.provinces=lane.provinces;
    // Prefer owned original country-specific passage results when available.
    // The current capture does not yet provide them; preserve its legacy
    // registration-only behavior until the native access window is verified.
    if(trade.domestic->straitAccess){
      const auto found=trade.domestic->straitAccess->find(lane.country);
      if(found==trade.domestic->straitAccess->end() || !std::is_sorted(found->second.begin(),found->second.end()) ||
         std::adjacent_find(found->second.begin(),found->second.end())!=found->second.end())
        return fail("invalid current domestic strait access");
      for(uint32_t id:found->second)
        if(std::none_of(trade.straits.begin(),trade.straits.end(),[&](const auto& strait){return strait.id==id;}))
          return fail("domestic strait access references unavailable strait");
      path.straits=found->second;
    }else for(const auto& strait:trade.straits)path.straits.push_back(strait.id);
    destination.candidates.push_back(std::move(path));
    auto shape=BuildNativeTradeGeometry(trade,lane.fromState,destination,nullptr,cancelled,true);
    NativeTradeGeometry available;
    if(!shape){if(cancelled && cancelled())return fail("cancelled");++result.unavailableSea;available.complete=false;}
    else {available=std::move(*shape);if(!available.complete)++result.partialSea;}
    // Current CShippingLane is the connectivity fact. Original native travel
    // distance(+60), including a valid zero within one sea region, is its model
    // route metric. A missing graphics key cannot erase this known connection.
    if(!add(lane.fromState,lane.toState,2,std::move(available),double(lane.travelDistanceFixed)/kNativeShippingDistanceScale))
      return fail("invalid domestic port lane scope or distance");
    ++result.seaConnections;
  }
  FreightPolicy policy;policy.graphRevision=snapshot.revision;
  result.allocation=AllocateFreight(model,graph,policy,cancelled);
  if(!result.allocation.complete){if(error)*error=result.allocation.error;return {};}
  if(error)error->clear();return result;
}
} // namespace v3
