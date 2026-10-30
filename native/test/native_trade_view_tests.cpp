#include "native_trade_view.h"
#include "freight_engine.h"
#include "logistics_session.h"
#include "native_state_owner.h"
#include "native_market_owner.h"
#include "native_trade_identity.h"
#include <cstdio>
#include <cstdlib>

using namespace v3;
void Check(bool value,const char* why){if(!value){std::printf("FAIL: %s\n",why);std::exit(1);}}
EconomySnapshot Snapshot(){
  EconomySnapshot snapshot;snapshot.world=1;snapshot.revision=1;snapshot.primaryMarket=7;
  snapshot.goods={{0,"unused","luxury",1,false},{1,"grain","staple",50,false},{2,"iron","industrial",70,false}};
  auto trade=std::make_shared<NativeWorldTradeSnapshot>();
  trade->states={{100,1,7},{200,4,8},{300,4,8},{400,5,7}};
  trade->centers={{100,100},{200,200},{300,200},{400,UINT32_MAX}};
  NativeTradeDestination destination;destination.state=200;destination.selected=0;
  destination.candidates={{{{10,10}},{},100000}};
  auto reverse=destination;reverse.state=100;
  trade->groups={{100,{100},{destination}},{200,{200,300},{reverse}}};
  trade->statePairs={{100,200,{{1,100001},{2,200003}}},{200,100,{{1,100001}}},
      {300,100,{{2,300007}}},{200,300,{{1,900011}}}};
  trade->groupPairs={{100,200,{{1,100001},{2,200003}}},{200,100,{{1,100001}}},
      {300,100,{{2,300007}}},{200,200,{{1,900011}}}};
  NativeSplineNetwork network;network.width=8192;network.height=3616;
  network.points={{0x800064,{0,1}},{0x8493e0,{10,1}},{0x800190,{20,1}},{71,{4,4}},{72,{16,4}}};
  network.strips={{0x100000003,{0x800064,71,0x8493e0}},{0x200000003,{0x800190,72,0x8493e0}}};
  network.connections={{0x04249f0020001903,{0x100000003}},{0x04249f0020006403,{0x200000003}}};
  NativeSeaGeography sea;sea.definitions={{0,{10,11}}};sea.provinces={{10,{{310,3000},{20}}}};
  sea.states={{100,{1,1}},{200,{4,4}},{300,{4,4}},{400,{5,5}}};
  sea.stateHubs={{100,true},{200,true},{300,true},{400,false}};
  trade->geometry=std::make_shared<const NativeSplineNetwork>(network);
  trade->seaGeography=std::make_shared<const NativeSeaGeography>(sea);snapshot.nativeTrade=trade;
  return snapshot;
}
int64_t Good(const NativeTradeView& view,uint32_t id){for(const auto& row:view.goods)if(row.good==id)return row.quantity;return -1;}
std::string Variable(const auto& rows,const std::string& key){for(const auto& [name,value]:rows)if(name==key)return value;return {};}
std::shared_ptr<const LogisticsDocument> Await(LogisticsSession& session){
  const auto deadline=GetTickCount64()+5000;
  while(GetTickCount64()<deadline){if(auto doc=session.Poll())return doc;Sleep(1);}return {};
}
int main(){
  {
    EconomySnapshot input;input.world=1;input.revision=1;input.goods={{1,"grain","staple",1,false}};
    auto trade=std::make_shared<NativeWorldTradeSnapshot>();trade->states={{100,UINT32_MAX,7},{400,UINT32_MAX,7}};
    NativeSplineNetwork network;network.width=8192;network.height=3616;
    constexpr uint32_t a=0x800000+100,b=0x800000+500,s=0x800000+300000,t=0x800000+300100;
    constexpr uint64_t p=(uint64_t(1)<<32)|3,q=(uint64_t(2)<<32)|3,e=(uint64_t(3)<<32)|2;
    network.points={{a,{1,1}},{s,{2,1}},{t,{3,1}},{b,{4,1}}};
    network.strips={{p,{a,s}},{e,{s,t}},{q,{t,b}}};
    network.connections={{NativePortSplineKeys(3000,1)[0],{p}},{NativeSeaSplineKeys(3000,3001)[0],{e}},
                         {NativePortSplineKeys(3001,5)[0],{q}}};
    NativeSeaGeography geo;geo.definitions={{0,{10,11}}};
    geo.provinces={{10,{{300,3000},{11,20}}},{11,{{301,3001},{10,21}}}};
    geo.states={{100,{1,1}},{400,{5,5}}};geo.stateHubs={{100,true},{400,true}};
    trade->geometry=std::make_shared<const NativeSplineNetwork>(network);
    trade->seaGeography=std::make_shared<const NativeSeaGeography>(geo);
    trade->straits={{0x1000001,{10,11},{0,0},0}};
    auto sources=std::make_shared<NativeDomesticSources>();
    sources->orders={{100,UINT32_MAX,7,1,1000,0},{400,UINT32_MAX,7,1,0,1000}};
    NativeShippingLane lane;lane.id=1;lane.country=5;lane.fromState=100;lane.toState=400;lane.provinces={{10,11}};
    sources->shipping.lanes.push_back(lane);lane.id=2;lane.country=9;sources->shipping.lanes.push_back(lane);
    sources->straitAccess=std::map<uint32_t,std::vector<uint32_t>>{{5,{0x1000001}},{9,{}}};
    trade->domestic=sources;input.nativeTrade=trade;
    auto facts=BuildNativeDomesticModel(input);
    Check(facts && facts->edges.size()==2 && facts->edges[0].complete && !facts->edges[1].complete &&
          facts->allocation.shipments.size()==1 && facts->allocation.shipments[0].quantity==1000,
          "country-specific owned access changes domestic geometry while preserving final model quantity");
    auto changed=std::make_shared<NativeWorldTradeSnapshot>(*trade);
    auto other=std::make_shared<NativeDomesticSources>(*sources);
    other->straitAccess=std::map<uint32_t,std::vector<uint32_t>>{{5,{}},{9,{0x1000001}}};changed->domestic=other;
    Check(!NativeTradeEquivalent(*trade,*changed),"country access change invalidates cached trade facts");
    input.nativeTrade=changed;auto reversed=BuildNativeDomesticModel(input);
    Check(reversed && !reversed->edges[0].complete && reversed->edges[1].complete &&
          reversed->allocation.shipments[0].quantity==1000,"opposite country access refreshes geometry, not model quantity");
    other->straitAccess=std::map<uint32_t,std::vector<uint32_t>>{{5,{0x1000001}}};
    Check(!BuildNativeDomesticModel(input),"missing current country access is not an implicit permit");
    other->straitAccess=std::map<uint32_t,std::vector<uint32_t>>{{5,{0x2000001}},{9,{}}};
    Check(!BuildNativeDomesticModel(input),"stale strait handle cannot authorize a current edge");
  }
  {
    EconomySnapshot input;input.world=1;input.revision=1;input.goods={{1,"grain","staple",1,false}};
    auto trade=std::make_shared<NativeWorldTradeSnapshot>();trade->states={{0,UINT32_MAX,7},{9,UINT32_MAX,7},{20,UINT32_MAX,8}};
    NativeSplineNetwork network;network.width=100;network.height=100;
    network.points={{100,{1,1}},{200,{4,1}},{300,{4,5}},{400,{7,1}},{500,{10,5}}};
    constexpr uint64_t a=(uint64_t(1)<<40)|1,b=a+64,c=a+128,d=a+192;
    constexpr uint64_t k1=(uint64_t(200)<<35)|(uint64_t(100)<<6)|1,
        k2=(uint64_t(300)<<35)|(uint64_t(100)<<6)|1,k3=(uint64_t(400)<<35)|(uint64_t(200)<<6)|1,
        k4=(uint64_t(500)<<35)|(uint64_t(400)<<6)|1;
    network.strips={{a,{100,200}},{b,{200,300}},{c,{200,400}},{d,{400,500}}};
    network.connections={{k1,{a}},{k2,{a,b}},{k3,{c}},{k4,{d}}};
    NativeDomesticSources sources;sources.builtRails={{1,0,0,4,k1,true},{2,0,9,4,k2,true},
        {3,9,0,4,k1,false},{4,9,20,4,k3,true},{5,20,20,4,k4,true}};
    trade->geometry=std::make_shared<const NativeSplineNetwork>(network);
    trade->seaGeography=std::make_shared<const NativeSeaGeography>();
    trade->domestic=std::make_shared<const NativeDomesticSources>(sources);input.nativeTrade=trade;
    FreightFilter filter;filter.market=7;auto facts=BuildNativeTradeFacts(input);Check(bool(facts),"rail-only owned snapshot accepted");
    auto view=BuildNativeTradeView(input,*facts,filter);Check(bool(view),"rail-only market view accepted");
    auto doc=BuildLogisticsDocument(input,*view,filter);
    Check(doc.builtRailLength==7,"unique built physical rail length includes intra-state, excludes unused/cross-market, no shipment needed");
    Check(doc.railMapUnits==7 && !doc.railKilometres && FormatLaidTrack(&doc)=="7.00 MU",
        "formal rail statistic is explicitly map units and never invented kilometres");
    auto scoped=filter;scoped.kind=FreightViewKind::State;scoped.state=9;scoped.hoverGood=1;scoped.restrictGoods=true;
    auto stateView=BuildNativeTradeView(input,*facts,scoped);
    Check(stateView && stateView->builtRailMapLength==7 && stateView->selectionQuantity==0,
        "state, hover and empty goods filter cannot change market infrastructure length");
    auto other=filter;other.market=8;auto otherView=BuildNativeTradeView(input,*facts,other);
    Check(otherView && otherView->builtRailMapLength==5,"another market has only its own internal unique built strips");
    FreightEngine cached({});Check(cached.SetSnapshot(input),"rail snapshot enters production cache");auto original=cached.Build(filter);
    auto changed=input;auto source=std::make_shared<NativeWorldTradeSnapshot>(*trade);
    std::reverse(sources.builtRails.begin(),sources.builtRails.end());source->domestic=std::make_shared<const NativeDomesticSources>(sources);changed.nativeTrade=source;
    Check(NativeTradeEquivalent(*trade,*source) && cached.SetSnapshot(changed) && cached.Build(filter)->nativeFacts==original->nativeFacts,
        "rail registry permutation preserves exact content and cached facts");
    sources.builtRails.erase(std::remove_if(sources.builtRails.begin(),sources.builtRails.end(),[](const auto& r){return r.id==2;}),sources.builtRails.end());
    source=std::make_shared<NativeWorldTradeSnapshot>(*trade);source->domestic=std::make_shared<const NativeDomesticSources>(sources);changed.nativeTrade=source;
    Check(!NativeTradeEquivalent(*trade,*source) && cached.SetSnapshot(changed),"same-revision current built rail change invalidates identity");
    auto fewer=cached.Build(filter);Check(fewer && fewer->nativeView->builtRailMapLength==3 && fewer->nativeFacts!=original->nativeFacts,
        "rail removal refreshes physical length without any goods quantity change");
    auto empty=sources;empty.builtRails.clear();source=std::make_shared<NativeWorldTradeSnapshot>(*trade);
    source->domestic=std::make_shared<const NativeDomesticSources>(empty);changed.nativeTrade=source;
    auto emptyFacts=BuildNativeTradeFacts(changed);auto emptyView=BuildNativeTradeView(changed,*emptyFacts,filter);
    Check(emptyView && emptyView->builtRailMapLength==0,"known no built rail is zero despite authored rail splines");
    auto zeroDoc=BuildLogisticsDocument(changed,*emptyView,filter);
    Check(FormatLaidTrack(&zeroDoc)=="0.00 MU","known zero railway length is distinct from missing native sources");
    const auto reject=[&](auto change,const char* why){
      auto s=std::make_shared<NativeWorldTradeSnapshot>(*trade);auto n=network;auto r=*trade->domestic;change(n,r);
      s->geometry=std::make_shared<const NativeSplineNetwork>(n);s->domestic=std::make_shared<const NativeDomesticSources>(r);
      auto bad=input;bad.nativeTrade=s;Check(!BuildNativeTradeFacts(bad),why);
    };
    reject([&](auto&,auto& r){r.builtRails[0].source=99;},"unknown current rail state cannot fabricate zero");
    reject([&](auto&,auto& r){r.builtRails[0].type=3;},"unbuilt category cannot become physical built infrastructure");
    reject([&](auto& n,auto&){n.connections.erase(k1);},"missing actual built rail binding refused");
    reject([&](auto& n,auto&){n.strips.at(b)={400,300};},"disconnected current built rail refused");
    reject([&](auto& n,auto&){n.points.at(200).x=std::numeric_limits<float>::quiet_NaN();},"nonfinite rail geometry refused");
    reject([&](auto&,auto& r){r.builtRails.push_back(r.builtRails[0]);},"duplicate current runtime rail handle refused");
    auto edited=network;edited.points.at(300).z=9;source=std::make_shared<NativeWorldTradeSnapshot>(*trade);
    source->geometry=std::make_shared<const NativeSplineNetwork>(edited);changed.nativeTrade=source;
    Check(cached.SetSnapshot(changed),"same-key current rail geometry update accepted");
    Check(cached.Build(filter)->nativeView->builtRailMapLength==11,"same-key new geometry uses current length and invalidates old cache");
    auto invalid=doc;invalid.railMapUnits=-1;Check(FormatLaidTrack(&invalid)=="--","negative map length never published");
    invalid.railMapUnits=std::numeric_limits<double>::infinity();Check(FormatLaidTrack(&invalid)=="--","nonfinite map length never published");
    invalid=doc;invalid.synthetic=true;Check(FormatLaidTrack(&invalid)=="--","synthetic length never enters formal card");
  }
  {
    EconomySnapshot input;input.world=1;input.revision=1;input.goods={{1,"grain","staple",1,false}};
    auto trade=std::make_shared<NativeWorldTradeSnapshot>();trade->states={{0,UINT32_MAX,7},{9,UINT32_MAX,7},{10,UINT32_MAX,7}};
    NativeSplineNetwork network;network.width=100;network.height=100;
    network.points={{100,{1,1}},{200,{2,1}},{300,{1,2}},{400,{1.5f,1}}};
    network.strips={{256,{100,200}},{512,{100,300}},{768,{100,400,200}}};
    constexpr uint64_t first=(uint64_t(200)<<35)|(uint64_t(100)<<6),second=(uint64_t(300)<<35)|(uint64_t(100)<<6);
    network.connections={{first,{256}},{second,{512}},{first|1,{768}}};
    NativeDomesticSources domestic;
    domestic.orders={{0,UINT32_MAX,7,1,1000,0},{9,UINT32_MAX,7,1,0,1000},{10,UINT32_MAX,7,1,0,1000}};
    domestic.land={{1,0,9,2,first,true},{2,0,10,2,second,true},{3,0,9,2,first|1,true}};
    trade->geometry=std::make_shared<const NativeSplineNetwork>(network);
    trade->seaGeography=std::make_shared<const NativeSeaGeography>();
    trade->domestic=std::make_shared<const NativeDomesticSources>(domestic);input.nativeTrade=trade;
    auto original=BuildNativeDomesticModel(input);
    Check(original && original->allocation.shipments.size()==1,"scarce domestic supply has one final shipment");
    const auto& expected=original->allocation.shipments[0];
    Check(expected.source==0 && expected.destination==9 && expected.quantity==1000 &&
        original->edges.at(expected.route.at(0).id).strips==std::vector<NativeDirectedStrip>{{256,true}},
        "equal-cost domestic allocation has a stable full-state and physical-connection tie order");
    FreightFilter selection;selection.market=7;selection.kind=FreightViewKind::State;selection.state=9;
    FreightEngine cached({});Check(cached.SetSnapshot(input),"permutation fixture enters the production engine");
    auto baseline=cached.Build(selection);
    for(unsigned permutation=1;permutation<8;++permutation){
      auto shuffled=std::make_shared<NativeWorldTradeSnapshot>(*trade);auto sources=domestic;
      if(permutation&1)std::reverse(shuffled->states.begin(),shuffled->states.end());
      if(permutation&2)std::reverse(sources.land.begin(),sources.land.end());
      if(permutation&4)std::reverse(sources.orders.begin(),sources.orders.end());
      shuffled->domestic=std::make_shared<const NativeDomesticSources>(sources);auto changed=input;changed.nativeTrade=shuffled;
      Check(NativeTradeEquivalent(*trade,*shuffled),"domestic enumeration permutations have identical content identity");
      auto model=BuildNativeDomesticModel(changed);
      Check(model && model->allocation.shipments.size()==1,"permuted domestic graph retains one final shipment");
      const auto& actual=model->allocation.shipments[0];
      Check(actual.source==expected.source && actual.destination==expected.destination && actual.market==expected.market &&
          actual.good==expected.good && actual.quantity==expected.quantity && actual.route.size()==expected.route.size() &&
          model->edges.at(actual.route[0].id).strips==original->edges.at(expected.route[0].id).strips,
          "equivalent enumeration cannot change domestic allocation or its selected physical strips");
      FreightEngine fresh({});Check(fresh.SetSnapshot(changed) && cached.SetSnapshot(changed),"permuted domestic source accepted");
      auto cold=fresh.Build(selection);auto warm=cached.Build(selection);
      Check(cold && cold->error.empty() && cold->nativeView && cold->nativeView->scopeQuantity==100000 && cold->ribbons.size()==1 &&
          warm->nativeFacts==baseline->nativeFacts && warm->nativeView==baseline->nativeView &&
          std::equal(cold->ribbons[0]->points.begin(),cold->ribbons[0]->points.end(),
              baseline->ribbons[0]->points.begin(),baseline->ribbons[0]->points.end(),
              [](const auto& a,const auto& b){return a.x==b.x && a.y==b.y && a.z==b.z;}),
          "cold replay, cached state quantity and displayed geometry agree after enumeration changes");
    }
  }
  auto snapshot=Snapshot();std::string error;auto facts=BuildNativeTradeFacts(snapshot,{},&error);
  Check(facts && facts->bindings.size()==4 && facts->shapes.size()==2,"original route shapes cached by native source/destination rather than recomputed per good");
  FreightFilter filter;filter.market=7;
  auto view=BuildNativeTradeView(snapshot,*facts,filter,{},&error);
  Check(view && view->marketQuantity==700012 && view->scopeQuantity==700012 && view->scopeDrawable==700012 &&
      view->selectionQuantity==700012 && view->filteredQuantity==700012,"cross-market native imports and exports counted once, without model production/orders/weights");
  Check(Good(*view,1)==200002 && Good(*view,2)==500010 && Good(*view,0)==0,"full original catalog includes known zero without requiring legacy orders");
  Check(view->segments.size()==2 && view->heatReference==700012,"one contribution per full native physical strip with all-market heat reference");
  for(const auto& segment:view->segments)Check(segment.quantity==700012 && std::abs(segment.netQuantity)==100004,
      "native net direction and total width quantities remain independent");
  auto ribbons=BuildNativeTradeRibbons(*facts,*view);
  Check(ribbons && ribbons->size()==2,"native geometry uses owned current controls without embedded map or model solver");
  for(const auto& ribbon:*ribbons){
    Check(ribbon->points.size()==5 && ribbon->points[0].z==1 && ribbon->points[1].z==1 && ribbon->points[2].z==4,
        "original native controls receive endpoint padding and exactly one effective native coordinate transform");
  }
  auto precise=snapshot;auto preciseSource=std::make_shared<NativeWorldTradeSnapshot>(*snapshot.nativeTrade);
  auto preciseNetwork=*preciseSource->geometry;preciseNetwork.points.at(0x800064).z=.00001f;
  preciseSource->geometry=std::make_shared<const NativeSplineNetwork>(preciseNetwork);precise.nativeTrade=preciseSource;
  auto preciseFacts=BuildNativeTradeFacts(precise);auto preciseView=BuildNativeTradeView(precise,*preciseFacts,filter);
  auto preciseRibbons=BuildNativeTradeRibbons(*preciseFacts,*preciseView);bool exactEndpoint=false;
  for(const auto& ribbon:*preciseRibbons)for(const auto& point:ribbon->points)
    exactEndpoint|=point.x==0 && point.z==.00001f;
  Check(exactEndpoint,"already-native coordinates must not lose small control values through two map-height reflections");
  auto document=BuildLogisticsDocument(snapshot,*view,filter);
  Check(document.nativeTrade && document.allFreight==0 && !document.intermediaryDemand && !document.railKilometres &&
      !document.railMapUnits && FormatLaidTrack(&document)=="--" &&
      FormatFreightWork(&document)=="7.00012" && FormatFreightDemand(&document)=="-- / -- / --",
      "native quantities are separate from model work/Freight demand/unknown rail calibration");
  auto rows=FormatFreightGoods(&document);
  Check(Variable(rows,"ironroad_good_work_grain")=="2.00002" && Variable(rows,"ironroad_good_share_grain")=="28.6%" &&
      Variable(rows,"ironroad_good_work_unused")=="0.00","native formatting preserves original1e-5 values and fixed market denominator");
  auto hovered=filter;hovered.hoverGood=1;
  auto hover=BuildNativeTradeView(snapshot,*facts,hovered);
  Check(hover && hover->selectionQuantity==700012 && hover->filteredQuantity==200002 && hover->marketQuantity==700012 &&
      hover->heatReference==view->heatReference,"hover changes map contribution only, never card/market share or heat denominator");
  for(const auto& segment:hover->segments)Check(segment.quantity==200002 && segment.netQuantity==0,"equal opposing flows retain both known directions");
  auto balanced=BuildNativeTradeRibbons(*facts,*hover);
  Check(balanced && balanced->size()==2,"opposing native flows aggregate once per physical ribbon");
  for(const auto& ribbon:*balanced)Check(ribbon->style.find("BalancedW0")!=std::string::npos,"net-zero original traffic cannot invent a one-way glyph");
  auto goodsMode=filter;goodsMode.colorMode=FreightColorMode::Goods;
  auto unselected=BuildNativeTradeView(snapshot,*facts,goodsMode);
  Check(unselected && unselected->segments.empty() && unselected->filteredQuantity==0 &&
      unselected->selectionQuantity==view->selectionQuantity && unselected->scopeQuantity==view->scopeQuantity,
      "single-good color mode waits for a good without changing cards or quantities");
  goodsMode.hoverGood=1;auto highlighted=BuildNativeTradeView(snapshot,*facts,goodsMode);
  auto highlightRibbons=BuildNativeTradeRibbons(*facts,*highlighted);
  Check(highlighted && highlightRibbons && highlightRibbons->size()==balanced->size() && highlighted->filteredQuantity==hover->filteredQuantity,
      "single-good mode extracts the same final goods contributions");
  for(const auto& ribbon:*highlightRibbons)Check(ribbon->style=="ironroadTrafficLimeBalancedW0",
      "good ID determines stable existing palette and preserves width/net-zero glyph policy");
  auto nativeColor=hovered;nativeColor.colorMode=FreightColorMode::Native;
  auto nativeColored=BuildNativeTradeView(snapshot,*facts,nativeColor);auto nativeRibbons=BuildNativeTradeRibbons(*facts,*nativeColored);
  Check(nativeRibbons && nativeRibbons->size()==balanced->size(),"native appearance retains current exact geometry");
  for(const auto& ribbon:*nativeRibbons)Check(ribbon->style=="ironroadBuildingOutputBalancedW0",
      "native appearance is explicitly the already verified building green, not arbitrary selected tint");
  auto invalidColor=filter;invalidColor.colorMode=FreightColorMode(99);
  Check(!BuildNativeTradeView(snapshot,*facts,invalidColor),"invalid color mode does not silently choose an appearance");
  auto restricted=filter;restricted.restrictGoods=true;restricted.allowedGoods={2};restricted.search="not-an-English-key";
  auto selected=BuildNativeTradeView(snapshot,*facts,restricted);
  Check(selected && selected->selectionQuantity==500010 && selected->marketQuantity==700012 && Good(*selected,1)==200002,
      "original matched row IDs remain authoritative for localized filters");
  restricted.allowedGoods.clear();auto empty=BuildNativeTradeView(snapshot,*facts,restricted);
  Check(empty && empty->selectionQuantity==0 && empty->segments.empty() && empty->scopeQuantity==700012,
      "known empty native filter has zero selected quantity and retains full scope statistics");
  auto other=filter;other.market=8;other.kind=FreightViewKind::State;other.state=200;
  auto state=BuildNativeTradeView(snapshot,*facts,other);
  Check(state && state->marketQuantity==1600023 && state->scopeQuantity==1300016 && state->scopeDrawable==400005 &&
      Good(*state,1)==1100013 && Good(*state,2)==200003,"state counts exact original incoming/outgoing records once and excludes another state's trade");
  auto stateDoc=BuildLogisticsDocument(snapshot,*state,other);
  Check(Variable(FormatStateFreightGoods(&stateDoc,200),"ironroad_state_work_200_grain")=="11.00013" &&
      FormatStateFreightGoods(&stateDoc,300).empty(),"native state variable publication requires its exact full state scope");
  auto incoming=other;incoming.mapDirection=1;
  auto imports=BuildNativeTradeView(snapshot,*facts,incoming);
  Check(imports && imports->filteredQuantity==300004 && imports->scopeQuantity==state->scopeQuantity &&
      imports->selectionQuantity==state->selectionQuantity && Good(*imports,1)==Good(*state,1),
      "state imports map filter retains complete state rows and market denominator");
  for(const auto& segment:imports->segments)Check(segment.direction==1,"imports cannot retain exported ribbons");
  auto outgoing=other;outgoing.mapDirection=-1;
  auto exports=BuildNativeTradeView(snapshot,*facts,outgoing);
  Check(exports && exports->filteredQuantity==1000012 && exports->scopeDrawable==state->scopeDrawable,
      "exports retain known quantities even when original geometry is missing");
  for(const auto& segment:exports->segments)Check(segment.direction==-1,"exports cannot retain imported ribbons");
  auto landOnly=other;landOnly.mapTransport=FreightTransport::Land;
  auto noSea=BuildNativeTradeView(snapshot,*facts,landOnly);
  Check(noSea && noSea->filteredQuantity==0 && noSea->segments.empty() && noSea->scopeQuantity==state->scopeQuantity,
      "original sea trade cannot appear as land or erase known state quantities");
  auto invalidFilter=other;invalidFilter.mapDirection=2;
  Check(!BuildNativeTradeView(snapshot,*facts,invalidFilter),"invalid map direction is rejected");
  invalidFilter=other;invalidFilter.mapTransport=FreightTransport(99);
  Check(!BuildNativeTradeView(snapshot,*facts,invalidFilter),"invalid map transport is rejected");
  auto zero=filter;zero.kind=FreightViewKind::State;zero.state=400;
  auto zeroView=BuildNativeTradeView(snapshot,*facts,zero);
  Check(zeroView && zeroView->scopeQuantity==0 && zeroView->segments.empty(),"known zero state exists without legacy order rows or a current trade center");
  auto roster=CopyNativeStateRoster(std::make_shared<const EconomySnapshot>(snapshot));
  Check(roster.markets.size()==4 && roster.markets.at(400)==7,"native state roster comes from verified original states, not legacy orders");
  auto bad=filter;bad.market=999;Check(!BuildNativeTradeView(snapshot,*facts,bad),"unknown market remains unavailable instead of plausible zero");
  bad=other;bad.state=100;Check(!BuildNativeTradeView(snapshot,*facts,bad),"state from another market rejected");
  bad=filter;bad.restrictGoods=true;bad.allowedGoods={999};Check(!BuildNativeTradeView(snapshot,*facts,bad),"unknown native matched ID rejected");
  bad=filter;bad.hoverGood=0x100000001ll;Check(!BuildNativeTradeView(snapshot,*facts,bad),"high hover bits cannot impersonate known native good");
  Check(!BuildNativeTradeFacts(snapshot,[]{return true;}) && !BuildNativeTradeView(snapshot,*facts,filter,[]{return true;}) &&
      !BuildNativeTradeRibbons(*facts,*view,1,0,[]{return true;}),"cancellation withdraws every native worker stage");
  auto stale=snapshot;++stale.world;Check(!BuildNativeTradeView(stale,*facts,filter),"old native facts cannot be relabelled as a new world");
  stale=snapshot;++stale.revision;Check(!BuildNativeTradeView(stale,*facts,filter),"old native facts cannot be relabelled as a newer revision");
  auto staleView=*view;++staleView.world;Check(!BuildNativeTradeRibbons(*facts,staleView),"new view identity cannot authorize old native geometry");
  auto missing=snapshot;auto data=std::make_shared<NativeWorldTradeSnapshot>(*snapshot.nativeTrade);
  auto sea=*data->seaGeography;sea.stateHubs.at(100)=false;data->seaGeography=std::make_shared<const NativeSeaGeography>(sea);missing.nativeTrade=data;
  auto incomplete=BuildNativeTradeFacts(missing);auto without=BuildNativeTradeView(missing,*incomplete,filter);
  Check(incomplete && without && without->scopeQuantity==700012 && without->scopeDrawable==0 && without->segments.empty(),
      "missing original geometry preserves known quantity and cannot fabricate transported route or model fallback");
  auto overflow=snapshot;data=std::make_shared<NativeWorldTradeSnapshot>(*snapshot.nativeTrade);
  data->statePairs[0].goods[0].valueFixed=INT64_MAX;data->groupPairs[0].goods[0].valueFixed=INT64_MAX;overflow.nativeTrade=data;
  auto large=BuildNativeTradeFacts(overflow);Check(large && !BuildNativeTradeView(overflow,*large,filter),"scope integer overflow cannot wrap native totals into zero");
  FreightEngine engine({});Check(engine.SetSnapshot(snapshot),"native engine accepts copied source without embedded graph");
  auto frame=engine.Build(filter);
  Check(frame && frame->error.empty() && frame->nativeFacts && frame->nativeView && !frame->allocation && !frame->view &&
      frame->ribbons.size()==2,"formal native engine branch never constructs legacy allocations or model views");
  auto hoverFrame=engine.Build(hovered);auto cached=engine.Build(filter);
  Check(hoverFrame->nativeFacts==frame->nativeFacts && cached->nativeView==frame->nativeView && cached->viewReused &&
      engine.Stats().allocations==1,"native facts and view caches remain warm across goods hover/reopen");
  auto highlightedFrame=engine.Build(goodsMode);auto nativeColorFrame=engine.Build(nativeColor);
  Check(highlightedFrame->nativeView!=hoverFrame->nativeView && nativeColorFrame->nativeView!=highlightedFrame->nativeView &&
      highlightedFrame->nativeFacts==frame->nativeFacts && engine.Build(goodsMode)->nativeView==highlightedFrame->nativeView &&
      engine.Stats().allocations==1,"color modes use separate view/style caches without rebuilding allocation or current facts");
  auto allState=engine.Build(other),importState=engine.Build(incoming),exportState=engine.Build(outgoing),landState=engine.Build(landOnly);
  Check(allState->nativeView!=importState->nativeView && importState->nativeView!=exportState->nativeView &&
      landState->ribbons.empty() && engine.Build(incoming)->nativeView==importState->nativeView &&
      engine.Stats().allocations==1,"map direction and transport are distinct cached views of the same immutable native facts");
  auto legacyChanged=snapshot;legacyChanged.orders={{999,123,456,1,999999,0}};legacyChanged.builtRailEdges={123};
  Check(engine.SetSnapshot(legacyChanged),"unrelated legacy diagnostic fields accepted");
  auto stillNative=engine.Build(filter);
  Check(stillNative->nativeFacts==frame->nativeFacts && stillNative->nativeView==frame->nativeView,
      "legacy model orders/rail policy cannot rebuild unchanged original trade facts or view");
  LogisticsSession session({});session.SetSnapshot(std::make_shared<const EconomySnapshot>(snapshot));
  LogisticsSelection selection;selection.market=7;selection.explicitMarket=true;
  session.SetSelection(kOwnerNativePanel,kOwnerNativePanel,selection);
  auto nativeDoc=Await(session);auto current=session.Frame();auto status=session.Status();
  Check(nativeDoc && nativeDoc->nativeTrade && FormatFreightWork(nativeDoc.get())=="7.00012" &&
      current && current->nativeFacts && session.FrameIsCurrent(current,status.applied),"owned native worker reply reaches current formal document and render frame");
  ++legacyChanged.revision;
  Check(session.HasEquivalentSnapshot(legacyChanged) && session.FrameIsCurrent(current,status.applied),
      "periodic legacy-order changes cannot blank an unchanged original trade display");
  selection.hoverGood=1;session.SetSelection(kOwnerHover,kOwnerHover,selection);
  Check(session.StatisticsAreCurrent(current) && !session.FrameIsCurrent(current,status.applied),"pending native hover preserves valid statistics while retiring old map frame");
  auto hoverDoc=Await(session);Check(hoverDoc && FormatFreightWork(hoverDoc.get())=="7.00012" &&
      FormatFreightGoods(hoverDoc.get())==FormatFreightGoods(nativeDoc.get()),"settled native hover keeps card/rows unchanged");
  session.Close(kOwnerHover);Check(Await(session)!=nullptr,"native base owner restores after hover closes");
  session.CloseAll();Check(!session.Frame() && !session.FrameIsCurrent(current,status.applied),"native close immediately withdraws submitted/pending frame");
  {
    LogisticsSession scoped({});auto copied=std::make_shared<const EconomySnapshot>(snapshot);
    scoped.SetSnapshot(copied);NativeStateOwner owner;auto stateRoster=CopyNativeStateRoster(copied);
    NativeStatePanelState panel;panel.available=panel.ingame=panel.stateOpen=true;panel.state=100;panel.tab="local_goods";
    owner.Update(scoped,panel,stateRoster,true);auto doc=Await(scoped);auto full=scoped.Frame();auto applied=scoped.Status().applied;
    const auto context=owner.Context();const auto keys=owner.FilterKeys();
    Check(doc && keys.size()==4 && context!="ironroad-state-inactive","state controls receive an owned current scope");
    GuiVariableProbe vars;vars.ok=true;vars.entries={{0,keys[0],context+":grain"},{0,keys[1],context+":imports"},{0,keys[2],context+":sea"}};
    Check(owner.UpdateFilters(scoped,vars,{{"grain",1},{"iron",2}}) && scoped.StatisticsAreCurrent(full) &&
        !scoped.FrameIsCurrent(full,applied) && owner.CanPublish(scoped,*doc,full,applied),
        "pending state map filters retire only map frame while complete rows stay publishable");
    auto selectedDoc=Await(scoped);auto selectedFrame=scoped.Frame();
    Check(selectedDoc && selectedFrame->filter.hoverGood==1 && selectedFrame->filter.mapDirection==1 &&
        selectedFrame->filter.mapTransport==FreightTransport::Sea && selectedFrame->nativeView->filteredQuantity==100001 &&
        FormatStateFreightGoods(doc.get(),100)==FormatStateFreightGoods(selectedDoc.get(),100) &&
        selectedFrame->nativeFacts==full->nativeFacts,"goods/direction/transport combine without reallocating or changing state row quantities");
    vars.entries.push_back({0,keys[3],context+":native"});
    Check(owner.UpdateFilters(scoped,vars,{{"grain",1}}) && scoped.StatisticsAreCurrent(selectedFrame),
        "pending state color change preserves scope statistics");
    auto coloredDoc=Await(scoped);
    Check(coloredDoc && scoped.Frame()->filter.colorMode==FreightColorMode::Native &&
        FormatStateFreightGoods(coloredDoc.get(),100)==FormatStateFreightGoods(doc.get(),100),
        "state color selection reaches worker and preserves complete goods rows");
    auto generation=scoped.Status().generation;
    Check(!owner.UpdateFilters(scoped,vars,{{"grain",1}}) && scoped.Status().generation==generation,
        "unchanged state controls do not replace settled worker requests");
    auto invalid=vars;invalid.entries[0].value=context+":unknown-mod-good";
    Check(!owner.UpdateFilters(scoped,invalid,{{"grain",1}}),"unknown goods key cannot impersonate first catalog entry");
    invalid=vars;invalid.entries[1].value=context+":Imports";
    Check(!owner.UpdateFilters(scoped,invalid,{{"grain",1}}),"malformed direction cannot silently choose another direction");
    invalid=vars;invalid.entries.push_back(invalid.entries[1]);
    Check(!owner.UpdateFilters(scoped,invalid,{{"grain",1}}),"ambiguous duplicate state variables are refused");
    invalid=vars;invalid.ok=false;
    Check(!owner.UpdateFilters(scoped,invalid,{{"grain",1}}),"unreadable control probe cannot submit a new selection");
    panel.state=400;owner.Update(scoped,panel,stateRoster,true);
    Check(owner.Context()!=context && !owner.UpdateFilters(scoped,vars,{{"grain",1}}) &&
        !owner.CanPublish(scoped,*doc,full,applied),"old state controls and queued rows cannot authorize a new state");
    auto switched=Await(scoped);Check(switched && scoped.Frame()->filter.hoverGood==-1 &&
        scoped.Frame()->filter.mapDirection==0 && scoped.Frame()->filter.mapTransport==FreightTransport::All,
        "switching states resets all map filters");
    const auto beforeClose=owner.Context();panel.tab="default";owner.Update(scoped,panel,stateRoster,true);
    Check(owner.FilterKeys().empty() && !scoped.Frame(),"closing state tab withdraws controls and map");
    panel.tab="local_goods";owner.Update(scoped,panel,stateRoster,true);
    Check(owner.Context()!=beforeClose && Await(scoped)!=nullptr,"reopening identical state has a fresh control generation");
  }
  {
    LogisticsSession scoped({});scoped.SetSnapshot(std::make_shared<const EconomySnapshot>(snapshot));NativeMarketOwner owner;
    NativeMarketPanelState panel;panel.available=panel.ingame=panel.marketOpen=true;panel.market=7;
    panel.tab="ironroad_logistics";panel.visibleGoods=std::vector<uint32_t>{1,2};owner.Update(scoped,panel,{7},true);
    auto base=Await(scoped);const auto context=owner.Context();
    GuiVariableProbe vars;vars.ok=true;vars.entries={{0,"ironroad_market_color_v1",context+":goods"}};
    Check(base && owner.UpdateColor(scoped,vars),"current native market color control accepted");
    auto waiting=Await(scoped);Check(waiting && scoped.Frame()->ribbons.empty() &&
        FormatFreightGoods(waiting.get())==FormatFreightGoods(base.get()),"market single-good mode waits while retaining complete card and rows");
    vars.entries.push_back({0,"ironroad_market_hover_grain",context});
    Check(owner.UpdateHover(scoped,vars,{{"grain",1}}),"single-good market hover accepted with current context");
    auto highlight=Await(scoped);auto prior=scoped.Frame();
    Check(highlight && prior->filter.colorMode==FreightColorMode::Goods && prior->ribbons.size()==2,
        "market hover inherits selected color mode");
    vars.entries[0].value=context+":native";
    Check(owner.UpdateColor(scoped,vars) && scoped.StatisticsAreCurrent(prior),"changing color updates both market base and active hover owner");
    auto native=Await(scoped);Check(native && scoped.Frame()->filter.colorMode==FreightColorMode::Native &&
        FormatFreightGoods(native.get())==FormatFreightGoods(base.get()),"active hover cannot keep the previous color after a switch");
    vars.entries.pop_back();owner.UpdateHover(scoped,vars,{{"grain",1}});auto restored=Await(scoped);
    Check(restored && scoped.Frame()->filter.colorMode==FreightColorMode::Native && scoped.Frame()->filter.hoverGood==-1,
        "leaving hover restores base filters with current color");
    auto invalid=vars;invalid.entries[0].value=context+":unknown";Check(!owner.UpdateColor(scoped,invalid),"unknown market color cannot choose first mode");
    panel.marketOpen=false;owner.Update(scoped,panel,{7},true);panel.marketOpen=true;owner.Update(scoped,panel,{7},true);
    Check(!owner.UpdateColor(scoped,vars) && owner.Context()!=context,"old market color context cannot cross reopening");
    auto reopened=Await(scoped);Check(reopened && scoped.Frame()->filter.colorMode==FreightColorMode::Business,
        "market reopening resets the business palette");
  }
  {
    auto domesticSnapshot=Snapshot();auto source=std::make_shared<NativeWorldTradeSnapshot>(*domesticSnapshot.nativeTrade);
    auto originalNetwork=*source->geometry;
    constexpr uint64_t landKey=(uint64_t(901)<<35)|(uint64_t(900)<<6),landStrip=0x300000000;
    originalNetwork.points.emplace(900,NativeSplinePoint{0,2});originalNetwork.points.emplace(901,NativeSplinePoint{20,2});
    originalNetwork.strips.emplace(landStrip,std::vector<uint32_t>{900,901});originalNetwork.connections.emplace(landKey,std::vector<uint64_t>{landStrip});
    auto inputs=std::make_shared<NativeDomesticSources>();
    inputs->orders={{100,UINT32_MAX,7,1,1000000,0},{400,UINT32_MAX,7,1,0,500000}};
    inputs->land={{77,100,400,2,landKey,true}};
    source->domestic=inputs;source->geometry=std::make_shared<const NativeSplineNetwork>(originalNetwork);domesticSnapshot.nativeTrade=source;
    auto domesticFacts=BuildNativeTradeFacts(domesticSnapshot,{},&error);
    if(!domesticFacts)std::printf("domestic error: %s\n",error.c_str());
    auto domesticView=domesticFacts?BuildNativeTradeView(domesticSnapshot,*domesticFacts,filter):std::nullopt;
    Check(domesticFacts && domesticFacts->domestic && domesticView && domesticView->domesticMarketQuantity==50000000 &&
        domesticView->marketQuantity==50700012 && domesticView->domesticAllocated==500000 && domesticView->domesticUnmet==0,
        "domestic logistics allocates known demand on current land links while preserving independent original trade");
    Check(domesticFacts->domestic->allocation.shipments.size()==1 &&
        domesticFacts->domestic->allocation.shipments[0].source==100 && domesticFacts->domestic->allocation.shipments[0].destination==400 &&
        domesticFacts->domestic->allocation.shipments[0].quantity==500000,"domestic trace retains source, destination, good and final milli-order allocation");
    auto domesticRibbons=BuildNativeTradeRibbons(*domesticFacts,*domesticView);
    Check(domesticRibbons && domesticRibbons->size()==3,"current land geometry is drawn alongside original sea trade");
    auto domesticDocument=BuildLogisticsDocument(domesticSnapshot,*domesticView,filter);
    Check(domesticDocument.nativeTrade->domesticScopeQuantity==50000000 &&
        domesticDocument.nativeTrade->scopeQuantity-domesticDocument.nativeTrade->domesticScopeQuantity==700012 &&
        FormatFreightDemand(&domesticDocument)=="500.00 / 500.00 / 0.00",
        "formal document separates original quantity, domestic model quantity and Freight target/fulfilled/unmet");
    auto selection=filter;selection.hoverGood=2;
    auto noGrain=BuildNativeTradeView(domesticSnapshot,*domesticFacts,selection);
    Check(noGrain && noGrain->segments.size()==2 && noGrain->marketQuantity==domesticView->marketQuantity &&
        noGrain->selectionQuantity==domesticView->selectionQuantity && noGrain->domesticAllocated==500000,
        "hover extracts domestic goods without changing allocation, card, demand or share denominator");
    auto stateFilter=filter;stateFilter.kind=FreightViewKind::State;stateFilter.state=400;
    auto domesticState=BuildNativeTradeView(domesticSnapshot,*domesticFacts,stateFilter);
    Check(domesticState && domesticState->scopeQuantity==50000000 && domesticState->domesticTarget==500000 &&
        domesticState->segments.size()==1 && domesticState->segments[0].direction==1,"domestic state counts its final imports once with incoming direction");
    auto disconnected=std::make_shared<NativeDomesticSources>(*inputs);disconnected->land.clear();source->domestic=disconnected;
    auto disconnectedFacts=BuildNativeTradeFacts(domesticSnapshot);auto disconnectedView=BuildNativeTradeView(domesticSnapshot,*disconnectedFacts,filter);
    Check(disconnectedView && disconnectedView->domesticMarketQuantity==0 && disconnectedView->domesticTarget==500000 &&
        disconnectedView->domesticUnmet==500000 && disconnectedView->marketQuantity==700012,
        "disconnected domestic network retains unmet demand with no invented movement, width or placeholder connection");
    auto seaNetwork=originalNetwork;seaNetwork.points.emplace(0x8001f4,NativeSplinePoint{20,1});
    constexpr uint64_t portStrip=0x400000003;
    seaNetwork.strips.emplace(portStrip,std::vector<uint32_t>{0x8001f4,72,0x8493e0});
    seaNetwork.connections.emplace(NativePortSplineKeys(3000,5)[0],std::vector<uint64_t>{portStrip});
    source->geometry=std::make_shared<const NativeSplineNetwork>(seaNetwork);
    auto sea=*source->seaGeography;sea.stateHubs.at(400)=true;source->seaGeography=std::make_shared<const NativeSeaGeography>(sea);
    auto ports=std::make_shared<NativeDomesticSources>(*disconnected);NativeShippingLane lane;lane.fromState=100;lane.toState=400;lane.provinces={{10,10}};
    ports->shipping.lanes.push_back(lane);source->domestic=ports;
    auto portFacts=BuildNativeTradeFacts(domesticSnapshot,{},&error);
    Check(portFacts && portFacts->domestic->seaConnections==1 && portFacts->domestic->allocation.shipments.size()==1,
        "domestic sea quantity is allocated on original port-lane geometry without inventing a sea path");
    Check(portFacts->domestic->allocation.shipments[0].quantity==500000 && portFacts->domestic->allocation.shipments[0].distance==0,
        "valid zero original within-region sea distance never erases positive final allocated quantity");
    auto parallelPorts=std::make_shared<NativeDomesticSources>(*ports);auto longerLane=lane;longerLane.id=4;longerLane.travelDistanceFixed=100000;
    parallelPorts->shipping.lanes.push_back(longerLane);source->domestic=parallelPorts;
    auto orderedPorts=BuildNativeDomesticModel(domesticSnapshot);
    std::reverse(parallelPorts->shipping.lanes.begin(),parallelPorts->shipping.lanes.end());
    auto reversedPorts=BuildNativeDomesticModel(domesticSnapshot);
    Check(orderedPorts && reversedPorts && orderedPorts->allocation.shipments.size()==1 && reversedPorts->allocation.shipments.size()==1 &&
        orderedPorts->allocation.shipments[0].quantity==reversedPorts->allocation.shipments[0].quantity &&
        orderedPorts->allocation.shipments[0].distance==reversedPorts->allocation.shipments[0].distance &&
        orderedPorts->allocation.shipments[0].route[0].id==reversedPorts->allocation.shipments[0].route[0].id &&
        orderedPorts->edges[0].strips==reversedPorts->edges[0].strips,
        "original port-lane enumeration preserves modeled quantity, selected edge identity and physical geometry");
    source->domestic=ports;
    auto portView=BuildNativeTradeView(domesticSnapshot,*portFacts,filter);auto portRibbons=BuildNativeTradeRibbons(*portFacts,*portView);
    Check(portView && portView->domesticMarketQuantity==50000000 && portRibbons && portRibbons->size()==3,
        "domestic sea contributions and international trade share exact original physical strips");
    {
      // The native lane can be regenerated with another physical port while
      // local orders and the market remain unchanged (initial-load retarget).
      auto old=std::make_shared<NativeWorldTradeSnapshot>(*source);
      old->states.push_back({600,UINT32_MAX,7});old->centers.emplace(600,UINT32_MAX);
      auto network=seaNetwork;constexpr uint64_t newPort=0x500000003;
      network.points.emplace(0x800258,NativeSplinePoint{25,1});
      network.strips.emplace(newPort,std::vector<uint32_t>{0x800258,72,0x8493e0});
      network.connections.emplace(NativePortSplineKeys(3000,6)[0],std::vector<uint64_t>{newPort});
      old->geometry=std::make_shared<const NativeSplineNetwork>(network);
      auto geography=sea;geography.states.emplace(600,NativeSeaRegion{6,6});geography.stateHubs.emplace(600,true);
      old->seaGeography=std::make_shared<const NativeSeaGeography>(geography);
      auto initial=std::make_shared<NativeDomesticSources>(*ports);
      initial->land={{88,600,400,2,landKey,true}};old->domestic=initial;
      auto before=domesticSnapshot;before.nativeTrade=old;
      FreightEngine cache({});Check(cache.SetSnapshot(before),"initial port snapshot accepted");
      auto oldFrame=cache.Build(filter);Check(oldFrame && oldFrame->error.empty(),"initial port frame built");
      Check(oldFrame->nativeFacts->domestic->edges[1].complete &&
          oldFrame->nativeFacts->domestic->edges[1].strips.back().id==portStrip &&
          std::any_of(oldFrame->nativeView->segments.begin(),oldFrame->nativeView->segments.end(),
              [&](const auto& segment){return segment.strip==portStrip;}),
          "initial frame actually contains the old physical port before regeneration");
      auto changed=std::make_shared<NativeWorldTradeSnapshot>(*old);
      auto regenerated=std::make_shared<NativeDomesticSources>(*initial);
      regenerated->shipping.lanes[0].id=0x1000000;
      regenerated->shipping.lanes[0].toState=600;changed->domestic=regenerated;
      auto after=before;after.nativeTrade=changed;
      Check(!NativeTradeEquivalent(*old,*changed) && cache.SetSnapshot(after),"native lane generation and port change invalidate unchanged-revision content");
      auto fresh=cache.Build(filter);
      Check(fresh && fresh->error.empty() && fresh->nativeFacts!=oldFrame->nativeFacts &&
          fresh->nativeView->domesticMarketQuantity==oldFrame->nativeView->domesticMarketQuantity,
          "regenerated port rebuilds facts while preserving the same final shipment quantity");
      const auto& edge=fresh->nativeFacts->domestic->edges[1];
      Check(edge.complete && edge.strips.back().id==newPort &&
          std::none_of(fresh->nativeView->segments.begin(),fresh->nativeView->segments.end(),
              [&](const auto& segment){return segment.strip==portStrip;}),
          "retargeted native lane ends at its new physical port and removes the old port contribution");
      LogisticsSession session({});session.SetSnapshot(std::make_shared<const EconomySnapshot>(before));
      LogisticsSelection chosen;chosen.market=7;chosen.explicitMarket=true;
      session.SetSelection(kOwnerNativePanel,kOwnerNativePanel,chosen);
      Check(bool(Await(session)),"initial port document published");
      auto prior=session.Frame();auto applied=session.Status().applied;
      Check(!session.HasEquivalentSnapshot(after),"same-orders new native port cannot disappear behind session equivalence");
      session.SetSnapshot(std::make_shared<const EconomySnapshot>(after));
      Check(!session.FrameIsCurrent(prior,applied),"old port frame retires before refreshed publication");
      Check(bool(Await(session)) && session.Frame()->nativeFacts->domestic->edges[1].strips.back().id==newPort,
          "formal worker publishes the regenerated port instead of reviving the initial endpoint");
    }
    {
      auto mixed=std::make_shared<NativeWorldTradeSnapshot>(*source);mixed->states.push_back({600,UINT32_MAX,7});
      mixed->centers.emplace(600,UINT32_MAX);
      auto inputs=std::make_shared<NativeDomesticSources>(*ports);
      inputs->orders={{100,UINT32_MAX,7,1,1000000,0},{600,UINT32_MAX,7,1,0,500000}};
      inputs->land={{88,400,600,2,landKey,true}};mixed->domestic=inputs;
      auto mixedSnapshot=domesticSnapshot;mixedSnapshot.nativeTrade=mixed;
      auto mixedFacts=BuildNativeTradeFacts(mixedSnapshot,{},&error);
      Check(mixedFacts && mixedFacts->domestic->allocation.shipments.size()==1 &&
          mixedFacts->domestic->allocation.shipments[0].route.size()==2,"mixed transport fixture allocates one original sea/land route");
      auto scope=filter;scope.kind=FreightViewKind::State;scope.state=600;
      auto whole=BuildNativeTradeView(mixedSnapshot,*mixedFacts,scope);
      scope.mapTransport=FreightTransport::Land;auto land=BuildNativeTradeView(mixedSnapshot,*mixedFacts,scope);
      scope.mapTransport=FreightTransport::Sea;auto sea=BuildNativeTradeView(mixedSnapshot,*mixedFacts,scope);
      Check(whole && land && sea && whole->segments.size()==3 && land->segments.size()==1 && sea->segments.size()==2 &&
          land->segments[0].strip==landStrip && land->filteredQuantity==50000000 && sea->filteredQuantity==50000000 &&
          land->scopeQuantity==whole->scopeQuantity && sea->domesticAllocated==whole->domesticAllocated &&
          land->heatReference==whole->heatReference,"mixed route mode filters paint only selected connection segments, count quantity once, preserve allocation and heat");
      scope.mapDirection=-1;auto none=BuildNativeTradeView(mixedSnapshot,*mixedFacts,scope);
      Check(none && none->segments.empty() && none->filteredQuantity==0 && none->scopeQuantity==50000000,
          "direction and transport intersection can be empty without erasing known state quantities");
    }
    auto missingPort=sea;missingPort.stateHubs.at(400)=false;source->seaGeography=std::make_shared<const NativeSeaGeography>(missingPort);
    auto missingPortFacts=BuildNativeTradeFacts(domesticSnapshot);auto missingPortView=BuildNativeTradeView(domesticSnapshot,*missingPortFacts,filter);
    Check(missingPortFacts->domestic->seaConnections==1 && missingPortFacts->domestic->partialSea==1 &&
        missingPortView->domesticMarketQuantity==50000000 && missingPortView->domesticUnmet==0 && missingPortView->scopeDrawable==700012,
        "missing visual port retains original connectivity and modeled quantity, while fully drawable quantity stays separate");
    const auto& partial=missingPortFacts->domestic->edges.at(0);
    Check(!partial.complete && partial.strips==std::vector<NativeDirectedStrip>{{0x100000003,true}},
        "partial port lane draws only the exact witnessed source-to-sea strip, never a fabricated destination bridge");
    auto noPorts=missingPort;noPorts.stateHubs.at(100)=false;source->seaGeography=std::make_shared<const NativeSeaGeography>(noPorts);
    auto invisibleFacts=BuildNativeTradeFacts(domesticSnapshot);auto invisibleView=BuildNativeTradeView(domesticSnapshot,*invisibleFacts,filter);
    Check(invisibleFacts->domestic->unavailableSea==1 && invisibleFacts->domestic->edges[0].strips.empty() &&
        invisibleView->domesticMarketQuantity==50000000 && invisibleView->scopeDrawable==0 && invisibleView->segments.empty(),
        "known original connection can allocate without drawable endpoints, without guessing map direction or geometry");
    auto noPath=std::make_shared<NativeDomesticSources>(*ports);noPath->shipping.lanes[0].provinces.clear();source->domestic=noPath;
    auto noPathFacts=BuildNativeTradeFacts(domesticSnapshot);auto noPathView=BuildNativeTradeView(domesticSnapshot,*noPathFacts,filter);
    Check(noPathFacts->domestic->seaConnections==0 && noPathView->domesticMarketQuantity==0 && noPathView->domesticUnmet==500000,
        "known empty original path grants no domestic connectivity and retains genuine unmet demand");
    source->domestic=ports;
    auto distanceChange=std::make_shared<NativeDomesticSources>(*ports);distanceChange->shipping.lanes[0].travelDistanceFixed=-1;source->domestic=distanceChange;
    Check(!BuildNativeTradeFacts(domesticSnapshot),"negative original shipping distance is not a model fallback cost");
    source->domestic=ports;
    source->seaGeography=std::make_shared<const NativeSeaGeography>(sea);
    source->domestic=inputs;
    auto reduced=std::make_shared<NativeWorldTradeSnapshot>(*source);
    auto sparse=std::make_shared<NativeDomesticSources>(*inputs);
    sparse->orders={{100,UINT32_MAX,7,1,100000,0},{400,UINT32_MAX,7,1,0,100000}};reduced->domestic=sparse;
    reduced->statePairs={{100,200,{{1,4000000}}}};reduced->groupPairs=reduced->statePairs;
    auto residualSnapshot=domesticSnapshot;residualSnapshot.nativeTrade=reduced;
    auto residualFacts=BuildNativeTradeFacts(residualSnapshot);auto residualView=BuildNativeTradeView(residualSnapshot,*residualFacts,filter);
    Check(residualView && residualView->domesticMarketQuantity==6000000 && residualView->domesticUnmet==40000 &&
        residualView->marketQuantity==10000000,"already exported native goods reduce domestic supply and cannot be allocated a second time");
    sparse=std::make_shared<NativeDomesticSources>(*sparse);sparse->orders={{100,UINT32_MAX,7,1,0,0},{400,UINT32_MAX,7,1,0,100000}};
    reduced->domestic=sparse;reduced->statePairs={{200,100,{{1,4000000}}}};reduced->groupPairs=reduced->statePairs;
    auto importedFacts=BuildNativeTradeFacts(residualSnapshot);auto importedView=BuildNativeTradeView(residualSnapshot,*importedFacts,filter);
    Check(importedView && importedView->domesticMarketQuantity==4000000 && importedView->domesticUnmet==60000,
        "native imports can supply downstream domestic demand without changing original trade records");
    FreightEngine domesticEngine({});Check(domesticEngine.SetSnapshot(domesticSnapshot),"domestic snapshot enters native worker branch");
    auto initial=domesticEngine.Build(filter);
    auto changed=domesticSnapshot;auto newSource=std::make_shared<NativeWorldTradeSnapshot>(*source);auto newInputs=std::make_shared<NativeDomesticSources>(*inputs);
    newInputs->orders[1].consumption=400000;newSource->domestic=newInputs;changed.nativeTrade=newSource;
    Check(domesticEngine.SetSnapshot(changed),"same revision changed domestic order accepted");auto rebuilt=domesticEngine.Build(filter);
    Check(rebuilt->nativeFacts!=initial->nativeFacts && rebuilt->nativeView->domesticMarketQuantity==40000000,
        "domestic quantities participate in content identity and cannot reuse stale original-only cache");
    LogisticsSession domesticSession({});domesticSession.SetSnapshot(std::make_shared<const EconomySnapshot>(domesticSnapshot));
    Check(!domesticSession.HasEquivalentSnapshot(changed),"periodic domestic change is not hidden by native trade equivalence");
  }
  std::puts("PASS: original native quantities, scope/share/filter/hover, exact precision, geometry/directions, caches and formal worker/document handoff");
}
