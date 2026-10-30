#include "native_trade_view.h"
#include "arrow_geometry.h"
#include "flow_width.h"
#include <limits>

namespace v3 {
namespace {
bool Add(int64_t& total,int64_t value){
  if(value<0 || total>INT64_MAX-value)return false;total+=value;return true;
}
std::string Lower(std::string text){for(char& c:text)if(c>='A' && c<='Z')c+='a'-'A';return text;}
bool Matches(const FreightGood& good,const FreightFilter& filter){
  if(filter.restrictGoods)return std::find(filter.allowedGoods.begin(),filter.allowedGoods.end(),good.id)!=filter.allowedGoods.end();
  return (filter.search.empty() || Lower(good.key+" "+good.name).find(Lower(filter.search))!=std::string::npos) &&
      (filter.category.empty() || good.category==filter.category) &&
      (!filter.restrictCategories || std::find(filter.categories.begin(),filter.categories.end(),good.category)!=filter.categories.end());
}
} // namespace

std::optional<NativeTradeFacts> BuildNativeTradeFacts(const EconomySnapshot& snapshot,
    const std::function<bool()>& cancelled,std::string* error){
  auto fail=[&](const char* why)->std::optional<NativeTradeFacts>{if(error)*error=why;return {};};
  if(cancelled && cancelled())return fail("cancelled");
  if(!snapshot.world || !snapshot.revision || !snapshot.nativeTrade || !snapshot.nativeTrade->geometry ||
     !snapshot.nativeTrade->seaGeography || !snapshot.nativeTrade->geometry->width || !snapshot.nativeTrade->geometry->height)
    return fail("native trade geometry unavailable");
  std::set<uint32_t> goods;
  for(const auto& good:snapshot.goods)if(good.key.empty() || !goods.insert(good.id).second)return fail("invalid native trade goods catalog");
  if(goods.empty() || goods.size()>128)return fail("invalid native trade goods catalog");
  auto bindings=BindNativeTradeRoutes(*snapshot.nativeTrade,error);if(!bindings)return {};
  NativeTradeFacts result;result.world=snapshot.world;result.revision=snapshot.revision;
  result.source=snapshot.nativeTrade;result.bindings=std::move(*bindings);
  std::map<std::pair<size_t,size_t>,size_t> shapes;
  for(const auto& binding:result.bindings){
    if(cancelled && cancelled())return fail("cancelled");
    for(const auto& amount:result.source->statePairs[binding.pair].goods)
      if(!goods.count(amount.good))return fail("native trade good outside catalog");
    std::optional<size_t> shape;
    if(binding.sourceGroup && binding.route){
      const auto key=std::pair{*binding.sourceGroup,*binding.route};
      auto found=shapes.find(key);
      if(found==shapes.end()){
        const auto& destination=result.source->groups[key.first].destinations[key.second];
        NativeTradeShape value;
        value.geometry=BuildNativeTradeGeometry(*result.source,binding.sourceCenter,destination,&value.unavailable,cancelled);
        if(value.unavailable=="cancelled")return fail("cancelled");
        found=shapes.emplace(key,result.shapes.size()).first;result.shapes.push_back(std::move(value));
      }
      shape=found->second;
    }
    result.shapeForBinding.push_back(shape);
  }
  if(result.source->domestic){
    result.domestic=BuildNativeDomesticModel(snapshot,cancelled,error);
    if(!result.domestic)return {};
    const auto& network=*result.source->geometry;
    std::map<uint32_t,uint32_t> markets;std::map<uint32_t,std::set<uint64_t>> physical;
    for(const auto& state:result.source->states){markets.emplace(state.id,state.market);physical.try_emplace(state.market);}
    std::set<uint32_t> handles;
    for(const auto& rail:result.source->domestic->builtRails){
      if(cancelled && cancelled())return fail("cancelled");
      if(rail.type!=4 || (rail.spline&63)!=1 || !handles.insert(rail.id).second ||
         !markets.count(rail.source) || !markets.count(rail.destination))return fail("invalid current built rail identity");
      const auto binding=network.connections.find(rail.spline);
      if(binding==network.connections.end() || binding->second.empty())return fail("current built rail binding unavailable");
      uint32_t anchor=uint32_t(rail.spline>>6)&0x1fffffff;
      for(uint64_t id:binding->second){
        const auto strip=network.strips.find(id);
        if((id&63)!=1 || strip==network.strips.end() || strip->second.size()<2)return fail("invalid current built rail strip");
        const auto& controls=strip->second;
        if(controls.front()==anchor)anchor=controls.back();
        else if(controls.back()==anchor)anchor=controls.front();
        else return fail("disconnected current built rail strips");
        if(markets.at(rail.source)==markets.at(rail.destination))physical.at(markets.at(rail.source)).insert(id);
      }
      if(anchor!=uint32_t(rail.spline>>35))return fail("current built rail endpoints differ");
    }
    std::map<uint32_t,double> lengths;
    for(const auto& [market,strips]:physical){
      double length=0;
      for(uint64_t id:strips){
        if(cancelled && cancelled())return fail("cancelled");
        const NativeSplinePoint* previous=nullptr;
        for(uint32_t control:network.strips.at(id)){
          const auto point=network.points.find(control);
          if(point==network.points.end() || !std::isfinite(point->second.x) || !std::isfinite(point->second.z))
            return fail("current built rail control unavailable");
          if(previous)length+=std::hypot(double(point->second.x)-previous->x,double(point->second.z)-previous->z);
          previous=&point->second;
        }
      }
      if(!std::isfinite(length))return fail("current built rail length overflow");
      lengths.emplace(market,length);
    }
    result.builtRailMapLengths=std::move(lengths);
  }
  if(error)error->clear();return result;
}

std::optional<NativeTradeView> BuildNativeTradeView(const EconomySnapshot& snapshot,const NativeTradeFacts& facts,
    const FreightFilter& filter,const std::function<bool()>& cancelled,std::string* error){
  auto fail=[&](const char* why)->std::optional<NativeTradeView>{if(error)*error=why;return {};};
  if(cancelled && cancelled())return fail("cancelled");
  if(filter.mapDirection<-1 || filter.mapDirection>1 ||
     (filter.mapDirection && filter.kind!=FreightViewKind::State) ||
     (filter.mapTransport!=FreightTransport::All && filter.mapTransport!=FreightTransport::Land &&
      filter.mapTransport!=FreightTransport::Sea))return fail("invalid native map filter");
  if(filter.colorMode!=FreightColorMode::Business && filter.colorMode!=FreightColorMode::Goods &&
     filter.colorMode!=FreightColorMode::Native)return fail("invalid native color mode");
  const auto transportMatches=[&](bool sea){return filter.mapTransport==FreightTransport::All ||
      filter.mapTransport==(sea?FreightTransport::Sea:FreightTransport::Land);};
  if(!snapshot.world || !snapshot.revision || snapshot.world!=facts.world || snapshot.revision!=facts.revision ||
     !facts.source || facts.shapeForBinding.size()!=facts.bindings.size())
    return fail("native trade facts unavailable");
  std::map<uint32_t,uint32_t> markets;
  for(const auto& state:facts.source->states)markets.emplace(state.id,state.market);
  if(std::none_of(markets.begin(),markets.end(),[&](const auto& row){return row.second==filter.market;}) ||
     (filter.kind==FreightViewKind::State && (!markets.count(filter.state) || markets.at(filter.state)!=filter.market)))
    return fail("native trade scope unavailable");
  std::map<uint32_t,const FreightGood*> goods;
  std::map<uint32_t,int64_t> totals;
  for(const auto& good:snapshot.goods){goods.emplace(good.id,&good);totals.emplace(good.id,0);}
  for(uint32_t id:filter.allowedGoods)if(filter.restrictGoods && !goods.count(id))return fail("unknown native filtered good");
  if(filter.hoverGood>=0 && (uint64_t(filter.hoverGood)>UINT32_MAX || !goods.count(uint32_t(filter.hoverGood))))return fail("unknown native hovered good");
  NativeTradeView result;result.world=snapshot.world;result.revision=snapshot.revision;
  result.colorMode=filter.colorMode;result.highlightGood=filter.hoverGood;
  if(facts.builtRailMapLengths){
    const auto length=facts.builtRailMapLengths->find(filter.market);
    if(length==facts.builtRailMapLengths->end())return fail("current market railway scope unavailable");
    result.builtRailMapLength=length->second;
  }
  std::map<uint64_t,int64_t> heat;
  std::map<std::pair<uint64_t,int>,NativeSegmentQuantity> segments;
  for(size_t i=0;i<facts.bindings.size();++i){
    if(cancelled && cancelled())return fail("cancelled");
    const auto& binding=facts.bindings[i];const auto& pair=facts.source->statePairs.at(binding.pair);
    if(markets.at(pair.source)!=filter.market && markets.at(pair.destination)!=filter.market)continue;
    const NativeTradeGeometry* geometry=nullptr;
    if(const auto shape=facts.shapeForBinding[i]){
      if(*shape>=facts.shapes.size())return fail("invalid native trade shape identity");
      if(facts.shapes[*shape].geometry)geometry=&*facts.shapes[*shape].geometry;
    }
    const bool scoped=filter.kind!=FreightViewKind::State || pair.source==filter.state || pair.destination==filter.state;
    const int direction=filter.kind==FreightViewKind::State?(pair.destination==filter.state?1:-1):0;
    for(const auto& amount:pair.goods){
      if(!goods.count(amount.good))return fail("native trade good outside catalog");
      if(!Add(result.marketQuantity,amount.valueFixed))return fail("native trade quantity overflow");
      if(geometry)for(const auto& strip:geometry->strips)if(!Add(heat[strip.id],amount.valueFixed))return fail("native heat quantity overflow");
      if(!scoped)continue;
      if(!Add(result.scopeQuantity,amount.valueFixed) || !Add(totals[amount.good],amount.valueFixed) ||
         (geometry && !Add(result.scopeDrawable,amount.valueFixed)))return fail("native scope quantity overflow");
      if(!Matches(*goods.at(amount.good),filter))continue;
      if(!Add(result.selectionQuantity,amount.valueFixed))return fail("native selection quantity overflow");
      if(filter.hoverGood>=0 && uint64_t(filter.hoverGood)!=amount.good)continue;
      if(filter.colorMode==FreightColorMode::Goods && filter.hoverGood<0)continue;
      if((filter.mapDirection && direction!=filter.mapDirection) || !transportMatches(true))continue;
      if(!Add(result.filteredQuantity,amount.valueFixed))return fail("native filtered quantity overflow");
      if(geometry)for(const auto& strip:geometry->strips){
        auto& segment=segments[{strip.id,direction}];segment.strip=strip.id;segment.direction=direction;
        if(!Add(segment.quantity,amount.valueFixed))return fail("native segment quantity overflow");
        // Absolute net is bounded by the checked positive total, so neither
        // signed addition below can overflow its int64 original-unit range.
        segment.netQuantity+=strip.forward?amount.valueFixed:-amount.valueFixed;
      }
    }
  }
  if(facts.domestic){
    if(facts.domestic->seaEdges.size()!=facts.domestic->edges.size())return fail("domestic transport identity unavailable");
    result.domesticDemand=FreightDemand{};
    for(const auto& balance:facts.domestic->allocation.balances){
      if(balance.market!=filter.market || (filter.kind==FreightViewKind::State && balance.state!=filter.state))continue;
      if(!Add(result.domesticTarget,balance.targetImport) || !Add(result.domesticAllocated,balance.allocatedImport) ||
         balance.allocatedImport>balance.targetImport || !Add(result.domesticUnmet,balance.targetImport-balance.allocatedImport))
        return fail("domestic demand quantity overflow");
      result.domesticDemand->target+=balance.freightDemand.target;
      result.domesticDemand->fulfilled+=balance.freightDemand.fulfilled;
      result.domesticDemand->unmet+=balance.freightDemand.unmet;
    }
    for(const auto& shipment:facts.domestic->allocation.shipments){
      if(cancelled && cancelled())return fail("cancelled");
      if(shipment.market!=filter.market)continue;
      if(shipment.quantity<=0 || shipment.quantity>INT64_MAX/100 || !goods.count(shipment.good))
        return fail("invalid domestic allocated quantity");
      const int64_t quantity=shipment.quantity*100;
      if(!Add(result.marketQuantity,quantity) || !Add(result.domesticMarketQuantity,quantity))
        return fail("domestic market quantity overflow");
      const bool scoped=filter.kind!=FreightViewKind::State || shipment.source==filter.state || shipment.destination==filter.state;
      const int direction=filter.kind==FreightViewKind::State?(shipment.destination==filter.state?1:-1):0;
      const bool selected=scoped && Matches(*goods.at(shipment.good),filter);
      bool complete=!shipment.route.empty(),modeMatches=false;
      for(const auto& edge:shipment.route){
        if(edge.id>=facts.domestic->edges.size())return fail("invalid domestic allocated edge identity");
        complete&=facts.domestic->edges[edge.id].complete;
        modeMatches|=transportMatches(facts.domestic->seaEdges[edge.id]);
      }
      const bool hovered=selected && (filter.hoverGood<0 || uint64_t(filter.hoverGood)==shipment.good) &&
          (filter.colorMode!=FreightColorMode::Goods || filter.hoverGood>=0) &&
          (!filter.mapDirection || direction==filter.mapDirection) && modeMatches;
      if(scoped && (!Add(result.scopeQuantity,quantity) || !Add(result.domesticScopeQuantity,quantity) ||
         (complete && !Add(result.scopeDrawable,quantity)) || !Add(totals[shipment.good],quantity)))return fail("domestic scope quantity overflow");
      if(selected && (!Add(result.selectionQuantity,quantity) || !Add(result.domesticSelectionQuantity,quantity)))
        return fail("domestic selection quantity overflow");
      if(hovered && !Add(result.filteredQuantity,quantity))return fail("domestic filtered quantity overflow");
      for(const auto& edge:shipment.route){
        if(edge.id>=facts.domestic->edges.size())return fail("invalid domestic allocated edge identity");
        for(const auto& strip:facts.domestic->edges[edge.id].strips){
          if(!Add(heat[strip.id],quantity))return fail("domestic heat quantity overflow");
          if(!hovered || !transportMatches(facts.domestic->seaEdges[edge.id]))continue;
          auto& segment=segments[{strip.id,direction}];segment.strip=strip.id;segment.direction=direction;
          if(!Add(segment.quantity,quantity))return fail("domestic segment quantity overflow");
          segment.netQuantity+=(strip.forward==edge.forward)?quantity:-quantity;
        }
      }
    }
  }
  for(const auto& [id,quantity]:heat)result.heatReference=std::max(result.heatReference,quantity);
  for(const auto& [id,quantity]:totals)result.goods.push_back({id,quantity});
  for(auto& [key,segment]:segments){
    const double relative=result.heatReference>0?double(segment.quantity)/double(result.heatReference):0;
    segment.band=uint8_t(std::min(4u,unsigned(std::min(1.0,relative)*5)));
    result.segments.push_back(segment);
  }
  if(error)error->clear();return result;
}

std::optional<RibbonBatch> BuildNativeTradeRibbons(const NativeTradeFacts& facts,const NativeTradeView& view,
    float coordScale,float heightOffset,const std::function<bool()>& cancelled,std::string* error){
  auto fail=[&](const char* why)->std::optional<RibbonBatch>{if(error)*error=why;return {};};
  if(cancelled && cancelled())return fail("cancelled");
  if(!view.world || view.world!=facts.world || view.revision!=facts.revision)return fail("stale native trade geometry scope");
  if(!facts.source || !facts.source->geometry)return fail("native spline geometry unavailable");
  if(view.colorMode!=FreightColorMode::Business && view.colorMode!=FreightColorMode::Goods &&
     view.colorMode!=FreightColorMode::Native)return fail("invalid native color mode");
  if(view.colorMode==FreightColorMode::Goods && !view.segments.empty() &&
     (view.highlightGood<0 || uint64_t(view.highlightGood)>UINT32_MAX))return fail("single-good color identity unavailable");
  const auto& network=*facts.source->geometry;RibbonBatch result;
  static constexpr const char* palettes[]={"ironroadTrafficGreen","ironroadTrafficLime","ironroadTrafficYellow","ironroadTrafficOrange","ironroadTrafficRed"};
  for(const auto& segment:view.segments){
    if(cancelled && cancelled())return fail("cancelled");
    if(segment.quantity<=0)return fail("invalid native segment quantity");
    const auto strip=network.strips.find(segment.strip);
    if(strip==network.strips.end())return fail("native strip geometry unavailable");
    const bool forward=segment.netQuantity>=0;std::vector<Vec3> points;
    for(size_t i=0;i<strip->second.size();++i){
      const auto point=network.points.find(strip->second[forward?i:strip->second.size()-i-1]);
      if(point==network.points.end())return fail("native control point unavailable");
      points.push_back({point->second.x,0,point->second.z});
    }
    std::string palette=segment.direction>0?"ironroadBuildingOutput":segment.direction<0?"ironroadBuildingInput":palettes[std::min<unsigned>(segment.band,4)];
    if(view.colorMode==FreightColorMode::Native)palette="ironroadBuildingOutput";
    else if(view.colorMode==FreightColorMode::Goods)palette=palettes[uint64_t(view.highlightGood)%5];
    if(!segment.netQuantity)palette+="Balanced";
    auto ribbon=MakeRibbon(FlowWidthStyle(palette,double(segment.quantity)/kNativeTradeQuantityScale),
        NativeArrowPoints(points,float(network.height),coordScale,0,heightOffset,false,true));
    if(!ribbon)return fail("invalid native ribbon geometry or style");
    result.push_back(std::move(ribbon));
  }
  if(error)error->clear();return PrepareRibbonBatch(std::move(result));
}
} // namespace v3
