#include "freight.h"
#include "freight_ribbons.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

using namespace v3;
void Require(bool ok,const char* why) { if (!ok) { std::printf("FAIL: %s\n",why);std::exit(1); } }
void Near(double a,double b,const char* why) { Require(std::abs(a-b)<1e-7,why); }
SplineGraph Graph() {
  SplineGraph g;
  // Costs force residual reassignment: A->X is initially cheapest, but the
  // optimum is A->Y and B->X. Greedy A->X would leave an expensive B->Y route.
  g.nodes={{100,0,0},{200,1,0},{300,2,0},{400,3,0}};
  g.byId={{100,0},{200,1},{300,2},{400,3}};
  g.edges={{0,0,2,0,0,2,1},{256,0,3,0,0,2,2},{512,1,2,0,0,2,2},{768,1,3,0,0,2,100}};
  return g;
}
EconomySnapshot Snapshot() {
  EconomySnapshot s;s.world=1;s.revision=1;s.goods={{1,"grain","staple",1,false}};
  s.orders={{10,1,7,1,1000,0},{20,2,7,1,1000,0},{30,3,7,1,0,1000},{40,4,7,1,0,1000}};
  return s;
}
int main() {
  auto graph=Graph();auto s=Snapshot();FreightPolicy policy;
  auto result=AllocateFreight(s,graph,policy);
  Require(result.complete,result.error.c_str());Require(result.shipments.size()==2,"two final allocations");
  double cost=0,freight=0;int64_t quantity=0;
  for (const auto& shipment:result.shipments) {cost+=shipment.cost;freight+=shipment.freight;quantity+=shipment.quantity;}
  Near(cost,4,"globally minimal transport cost after residual reassignment");
  Near(freight,4,"physical freight uses final shipments, not tentative augmentations");
  Require(quantity==2000,"quantity conservation");
  // Distinct runtime states may share an authored region/hub (split states).
  // Every sink terminal at that hub must survive final-flow decomposition.
  auto shared=Snapshot();
  shared.orders={{10,1,7,1,3000,0},{30,3,7,1,0,1000},{31,3,7,1,0,2000}};
  auto sharedFlow=AllocateFreight(shared,graph,policy);
  Require(sharedFlow.complete,sharedFlow.error.c_str());
  Require(sharedFlow.shipments.size()==2,"shared hub keeps both importing states");
  Require(sharedFlow.balances[1].allocatedImport==1000 && sharedFlow.balances[2].allocatedImport==2000,
          "shared hub fulfills each state's own demand without losing a terminal");
  Require(sharedFlow.balances[0].allocatedExport==3000,"shared hub export conservation");
  auto sharedShuffled=shared;std::reverse(sharedShuffled.orders.begin(),sharedShuffled.orders.end());
  auto sharedRepeat=AllocateFreight(sharedShuffled,graph,policy);
  Require(sharedRepeat.complete && sharedRepeat.shipments.size()==sharedFlow.shipments.size(),
          "shared hub supports shuffled input");
  for(size_t i=0;i<sharedFlow.shipments.size();++i)
    Require(sharedFlow.shipments[i].destination==sharedRepeat.shipments[i].destination &&
            sharedFlow.shipments[i].quantity==sharedRepeat.shipments[i].quantity,
            "shared hub allocation is deterministic by state identity");
  auto sharedShortage=shared;sharedShortage.orders[0].production=1500;
  auto shortageFlow=AllocateFreight(sharedShortage,graph,policy);
  Require(shortageFlow.complete && shortageFlow.balances[0].allocatedExport==1500 &&
          shortageFlow.balances[1].allocatedImport+shortageFlow.balances[2].allocatedImport==1500,
          "shared import hub preserves shortages and quantity conservation");
  auto sharedBoth=shared;sharedBoth.orders[0].production=1000;
  sharedBoth.orders.push_back({11,1,7,1,2000,0});
  auto sharedBothFlow=AllocateFreight(sharedBoth,graph,policy);
  Require(sharedBothFlow.complete && sharedBothFlow.balances[0].allocatedExport==1000 &&
          sharedBothFlow.balances[3].allocatedExport==2000 && sharedBothFlow.balances[1].allocatedImport==1000 &&
          sharedBothFlow.balances[2].allocatedImport==2000,
          "shared export and import hubs retain every state's allocation");
  for (const auto& b:result.balances) Require(b.allocatedImport<=b.targetImport && b.allocatedExport<=b.availableExport,"allocations obey snapshot bounds");
  auto shuffled=s;std::reverse(shuffled.orders.begin(),shuffled.orders.end());
  auto repeat=AllocateFreight(shuffled,graph,policy);
  Require(repeat.complete && repeat.shipments.size()==result.shipments.size(),"deterministic shuffled input");
  for (size_t i=0;i<repeat.shipments.size();++i) Require(repeat.shipments[i].source==result.shipments[i].source && repeat.shipments[i].destination==result.shipments[i].destination,"stable allocation order");

  // One route crossing two edges contributes its freight once to the market,
  // while each edge receives throughput once. Filtering cannot re-run allocation.
  auto line=Graph();line.nodes.resize(3);line.byId={{100,0},{200,1},{300,2}};
  line.edges={{0,0,1,0,0,2,3},{256,1,2,3,0,2,4}};
  auto one=Snapshot();one.orders={{10,1,7,1,3000,0},{30,3,7,1,0,5000}};
  auto flow=AllocateFreight(one,line,policy);Require(flow.complete,flow.error.c_str());
  Require(flow.shipments.size()==1 && flow.shipments[0].quantity==3000,"shortage leaves unallocated target");
  Require(flow.balances[1].allocatedImport==3000 && flow.balances[1].targetImport==5000,"target import distinct from actual allocation");
  Near(flow.shipments[0].distance,7,"distance uses ordered physical edges");
  Require(flow.shipments[0].route[1].id==256,"authored coastal corridor retained");
  FreightFilter filter;filter.market=7;
  auto view=BuildFreightView(one,flow,filter);
  Near(view.marketFreight,21,"market freight counted once");
  Require(view.segments.size()==2,"two unique physical segments");
  Near(view.segments[0].throughput,3,"edge intensity is throughput, not full route charge");
  Near(view.segments[0].quantity,3,"line width uses allocated quantity, not unmet target consumption");
  Near(view.goods[0].share,1,"goods share denominator is all market freight");
  Near(view.targetImport,5,"target import totals the market scope");
  Near(view.allocatedImport,3,"allocated import is bounded by target");
  Near(view.unmetDemand,2,"unmet demand stays separate from drawn freight");
  filter.search="iron";auto hidden=BuildFreightView(one,flow,filter);
  Require(hidden.segments.empty(),"goods search filters view");Near(hidden.marketFreight,21,"filter preserves market denominator");
  Near(hidden.targetImport,5,"filter preserves target import");Near(hidden.unmetDemand,2,"filter preserves unmet demand");
  filter.search="";filter.hoverGood=2;Require(BuildFreightView(one,flow,filter).segments.empty(),"hover selection filters good");
  filter.hoverGood=-1;filter.kind=FreightViewKind::State;filter.state=30;
  auto stateImportView=BuildFreightView(one,flow,filter);
  Require(stateImportView.segments[0].direction==1,"incoming state colour direction");
  Near(stateImportView.targetImport,5,"state target import");Near(stateImportView.allocatedImport,3,"state allocated import");
  Near(stateImportView.unmetDemand,2,"state unmet demand");
  filter.state=10;Require(BuildFreightView(one,flow,filter).segments[0].direction==-1,"outgoing state colour direction");
  Near(BuildFreightView(one,flow,filter).unmetDemand,0,"exporting state has no unmet import");
  filter.state=20;Require(BuildFreightView(one,flow,filter).segments.empty(),"unrelated state hidden");
  auto old=one;old.world=2;Require(BuildFreightView(old,flow,filter).segments.empty(),"world reload rejects stale result");

  auto disconnected=policy;disconnected.closedEdges={256};
  auto none=AllocateFreight(one,line,disconnected);Require(none.complete && none.shipments.empty(),"disconnected routes allocate no phantom flow");
  auto separate=one;separate.orders[1].market=8;
  Require(AllocateFreight(separate,line,policy).shipments.empty(),"allocation never crosses markets");
  auto local=one;local.goods[0].localOnly=true;
  Require(AllocateFreight(local,line,policy).shipments.empty(),"local-only goods excluded");
  auto monetary=one;monetary.goods[0].key="gold";monetary.goods[0].tradeable=false;
  auto monetaryResult=AllocateFreight(monetary,line,policy);
  Require(monetaryResult.complete && monetaryResult.shipments.empty(),"non-local nontradeable goods excluded");
  for (const auto& balance:monetaryResult.balances) {
    Require(balance.targetImport==0 && balance.freightDemand.target==0 && balance.freightDemand.unmet==0,
            "nontradeable source creates neither original transport orders nor intermediary Freight demand");
  }
  Require(monetary.orders[1].consumption==5000,"exclusion preserves copied source consumption");
  auto railway=line;railway.edges[1].type=1;
  Require(AllocateFreight(one,railway,policy).shipments.empty(),"unbuilt railroad cannot route freight");
  auto built=one;built.builtRailEdges={256};
  auto rail=AllocateFreight(built,railway,policy);Require(rail.complete && rail.shipments.size()==1,"built railroad admitted");
  Near(rail.builtRailLength,4,"laid rail sums unique built length");
  Near(rail.shipments[0].freight,21,"rail cost changes do not change physical distance freight");
  Near(rail.shipments[0].cost,13.8,"rail tariff affects route cost");

  // A region owns several authored hubs. When the city hub is served only by
  // unbuilt rail, the state must still use another of its hubs instead of
  // looking unreachable.
  SplineGraph hubs;
  hubs.nodes={{100,0,0},{103,1,0},{200,2,0}};
  hubs.byId={{100,0},{103,1},{200,2}};
  hubs.points={{0,0},{0,1},{1,0},{1,1}};
  hubs.edges={{1,0,1,1,0,2,1},{2,1,2,0,2,2,2}};
  auto hubSnapshot=Snapshot();
  hubSnapshot.orders={{10,1,7,1,1000,0},{20,2,7,1,0,1000}};
  auto hubFlow=AllocateFreight(hubSnapshot,hubs,policy);
  Require(hubFlow.complete,hubFlow.error.c_str());
  Require(hubFlow.shipments.size()==1,"unbuilt city rail falls back to another region hub");
  Require(hubFlow.shipments[0].route.size()==1 && hubFlow.shipments[0].route[0].id==2,
          "fallback hub uses the allowed authored road, not the unbuilt rail");
  Require(hubFlow.balances[1].allocatedImport==1000 && hubFlow.balances[1].targetImport==1000,
          "fallback hub satisfies the target import");
  auto closedHub=policy;closedHub.closedEdges={2};
  auto blocked=AllocateFreight(hubSnapshot,hubs,closedHub);
  Require(blocked.complete && blocked.shipments.empty(),"closed fallback road leaves the demand unmet");

  auto duplicate=one;duplicate.orders.push_back(duplicate.orders[0]);
  Require(!AllocateFreight(duplicate,line,policy).complete,"duplicate orders rejected");
  auto invalid=policy;invalid.costPerDistance[0]=NAN;
  Require(!AllocateFreight(one,line,invalid).complete,"nonfinite policy rejected");
  int checks=0;auto stopped=AllocateFreight(one,line,policy,[&]{ return ++checks>=3; });
  Require(stopped.cancelled && !stopped.complete && stopped.shipments.empty(),"cancelled batch publishes no partial allocations");
  // All quantities use fixed absolute bands, independent of selected goods.
  for (unsigned i=1;i<kFlowWidths.size();++i) Require(kFlowWidths[i]>kFlowWidths[i-1],"width bands strictly increase");
  Require(FlowWidthStyle("ironroadBuildingOutput",100)==FlowWidthStyle("ironroadBuildingOutput",199),"within-band quantity change preserves exact geometry style");
  Require(FlowWidthStyle("ironroadBuildingOutput",100)!=FlowWidthStyle("ironroadBuildingOutput",200),"crossing consumption threshold changes native width style");
  Require(FlowWidthStyle("ironroadBuildingOutput",0).empty(),"zero consumption draws no line");
  Require(FlowWidthStyle("ironroadBuildingOutput",NAN).empty(),"invalid consumption draws no line");
  line.points={{0,0},{1,0},{1,0},{2,0}};line.edges[1].first=2;
  FreightView widths;
  widths.segments={{0,1,1,0,5,5,0},{256,1,-1,0,5000,-5000,6}};
  HeightField flat;flat.z1=100;
  auto ribbons=BuildFreightRibbons(widths,line,flat);
  Require(ribbons.size()==2,"quantity-aware freight segments become native ribbon requests");
  auto green=std::find_if(ribbons.begin(),ribbons.end(),[](auto& r){return r->style=="ironroadBuildingOutputW0";});
  auto orange=std::find_if(ribbons.begin(),ribbons.end(),[](auto& r){return r->style=="ironroadBuildingInputW6";});
  Require(green!=ribbons.end() && orange!=ribbons.end(),"incoming/outgoing colour and consumption width are independent");
  Require((*orange)->points.front().x==2 && (*orange)->points.back().x==1,"native freight geometry follows net shipment direction");
  widths.segments.clear();
  for (unsigned band=0;band<5;++band) widths.segments.push_back({0,1,0,(uint8_t)band,100,100,2});
  auto spectrum=BuildFreightRibbons(widths,line,flat);
  Require(spectrum.size()==5,"all five traffic colours retain distinct native style identity");

  // Market heat has one fixed reference from the complete snapshot. Hovering or
  // searching a good extracts contributions; it must never rescale the colours
  // of the edges that remain visible (MASTER_PLAN section 6).
  auto set=Snapshot();
  set.goods={{1,"grain","staple",1,false},{2,"iron","industrial",1,false}};
  set.orders={{10,1,7,1,1000,0},{30,3,7,1,0,1000},{10,1,7,2,5000,0},{40,4,7,2,0,5000}};
  auto setFlow=AllocateFreight(set,graph,policy);Require(setFlow.complete,setFlow.error.c_str());
  FreightFilter market;market.market=7;
  auto full=BuildFreightView(set,setFlow,market);
  Require(full.segments.size()==2,"both goods contribute one physical edge each");
  Near(full.heatReference,5,"heat reference uses full weighted throughput");
  Near(full.targetImport,6,"market target import totals both goods");
  Near(full.allocatedImport,6,"market allocated import totals both goods");
  Near(full.unmetDemand,0,"fully served market has no unmet demand");
  auto bandOf=[](const FreightView& v,uint32_t edge)->int {
    for (const auto& s:v.segments) if (s.edge==edge) return s.band;return -1;
  };
  Require(bandOf(full,0)==1 && bandOf(full,256)==4,"bands derive from the market-wide scale");
  market.hoverGood=1;auto hovered=BuildFreightView(set,setFlow,market);
  Require(hovered.segments.size()==1 && bandOf(hovered,0)==1,"hover keeps the market heat scale");
  Near(hovered.heatReference,5,"hover preserves the reference scale");
  Near(hovered.marketFreight,full.marketFreight,"hover preserves the market denominator");
  market.hoverGood=-1;market.search="grain";auto searched=BuildFreightView(set,setFlow,market);
  Require(searched.segments.size()==1 && bandOf(searched,0)==1,"search keeps the market heat scale");
  market.search="";market.kind=FreightViewKind::State;market.state=30;
  auto stateView=BuildFreightView(set,setFlow,market);
  Require(stateView.segments.size()==1 && stateView.segments[0].direction==1,"state view keeps direction colours");
  Near(stateView.heatReference,5,"state view retains the market reference scale");
  Near(stateView.selectionFreight,1,"state card includes only that state's incident shipments");
  Require(stateView.goods.size()==1 && stateView.goods[0].good==1 && stateView.goods[0].freight==1,
          "state Freight column must not receive another state's goods work");
  Near(stateView.goods[0].share,1./11,"state row retains the whole-market denominator");
  market.state=10;
  Near(BuildFreightView(set,setFlow,market).selectionFreight,11,"exporting state counts each outgoing shipment once");
  market.kind=FreightViewKind::Market;market.hoverGood=1;
  auto mapOnlyHover=BuildFreightView(set,setFlow,market);
  Near(mapOnlyHover.selectionFreight,11,"market card ignores temporary goods hover");
  Near(mapOnlyHover.filteredFreight,1,"hover still isolates the drawn good's work");
  market.category="industrial";
  auto industrial=BuildFreightView(set,setFlow,market);
  Near(industrial.selectionFreight,10,"card aggregates the current category before hover");
  Near(industrial.filteredFreight,0,"hover cannot bypass the underlying category");
  Near(industrial.marketFreight,11,"selected-filter card never changes the global share denominator");
  market.category="";market.hoverGood=-1;
  market.kind=FreightViewKind::Market;market.market=8;
  auto empty=BuildFreightView(set,setFlow,market);
  Require(empty.segments.empty() && empty.heatReference==0,"empty market has no fake heat scale");

  // Freight intermediary demand must exist before routing, not just rename
  // the work performed by successful shipments. 10 grain orders at 2 Freight
  // plus 4 tools orders at .5 Freight = 22 target; available grain serves 12,
  // and the tools consumer is disconnected, leaving 10 Freight unmet.
  auto demandSnapshot = Snapshot();
  demandSnapshot.goods = {{1,"grain","staple",7,false,"Grain",2},
                          {2,"tools","industrial",3,false,"Tools",.5},
                          {3,"services","staple",1,true},
                          {4,"Freight","industrial",1,false},
                          {5,"custom_intermediary","industrial",1,false,"",1,true}};
  demandSnapshot.orders = {{10,1,7,1,6000,0},{30,3,7,1,2000,12000},
                           {10,1,7,2,4000,0},{90,9,7,2,0,4000},
                           {90,9,7,3,0,100000},{90,9,7,4,0,100000},
                           {90,9,7,5,0,100000},
                           {80,8,8,1,0,3000}};
  auto demandFlow = AllocateFreight(demandSnapshot,graph,policy);
  Require(demandFlow.complete && demandFlow.freightDemandRevision==kFreightDemandRevision,
          "Freight demand publishes only with a complete allocation");
  FreightFilter demandFilter;demandFilter.market=7;
  auto demandView=BuildFreightView(demandSnapshot,demandFlow,demandFilter);
  Require(demandView.intermediaryDemand.has_value(),"Freight demand is available for a valid snapshot");
  Near(demandView.intermediaryDemand->target,22,"net imports converted before allocation including unreachable tools");
  Near(demandView.intermediaryDemand->fulfilled,12,"only final allocated goods fulfill intermediary demand");
  Near(demandView.intermediaryDemand->unmet,10,"goods shortage and unreachable goods remain unmet");
  Near(demandView.targetImport,14,"Freight units do not overwrite original goods orders");
  Near(demandView.allocatedImport,6,"original allocated quantity is unchanged by the conversion coefficient");
  Near(demandFlow.balances[1].freightDemand.target,20,"state and good retain their source contribution");
  Near(demandFlow.balances[3].freightDemand.unmet,2,"disconnected demand remains represented");
  Require(demandFlow.balances[4].freightDemand.target==0 && demandFlow.balances[5].freightDemand.target==0 &&
          demandFlow.balances[6].freightDemand.target==0,"local goods and Freight itself cannot recursively generate demand");
  demandFilter.hoverGood=1;demandFilter.search="grain";
  const auto demandHover=BuildFreightView(demandSnapshot,demandFlow,demandFilter);
  Near(demandHover.intermediaryDemand->target,22,"hover and search cannot change Freight demand");
  Near(demandHover.intermediaryDemand->unmet,10,"hover cannot erase unmet Freight demand");
  demandFilter.market=8;
  auto isolatedDemand=BuildFreightView(demandSnapshot,demandFlow,demandFilter);
  Near(isolatedDemand.intermediaryDemand->target,6,"Freight demand is isolated by market");
  Near(isolatedDemand.intermediaryDemand->fulfilled,0,"other markets cannot supply the unmet original goods");
  demandFilter.market=7;demandFilter.kind=FreightViewKind::State;demandFilter.state=90;
  Near(BuildFreightView(demandSnapshot,demandFlow,demandFilter).intermediaryDemand->target,2,
       "state scope isolates source demand even when no route exists");
  demandFilter.kind=FreightViewKind::Market;demandFilter.market=999;
  Near(BuildFreightView(demandSnapshot,demandFlow,demandFilter).intermediaryDemand->target,0,
       "known empty market has zero demand");
  auto legacyDemand=demandFlow;legacyDemand.freightDemandRevision=0;
  Require(!BuildFreightView(demandSnapshot,legacyDemand,demandFilter).intermediaryDemand,
          "missing intermediary calculation is unknown, not fake zero");
  auto staleDemand=demandSnapshot;++staleDemand.world;
  Require(!BuildFreightView(staleDemand,demandFlow,demandFilter).intermediaryDemand,
          "stale world cannot publish intermediary demand");
  for (double coefficient : std::array<double,5>{0.,-1.,INFINITY,NAN,1000001.}) {
    auto badDemand=demandSnapshot;badDemand.goods[0].freightPerOrder=coefficient;
    Require(!AllocateFreight(badDemand,graph,policy).complete,"invalid Freight conversion rejected");
  }
  auto cancelledDemand=AllocateFreight(demandSnapshot,graph,policy,[]{return true;});
  Require(cancelledDemand.cancelled && cancelledDemand.freightDemandRevision==0 && cancelledDemand.balances.empty(),
          "cancelled demand calculation publishes no partial ledger");
  auto reweighted=demandSnapshot;reweighted.goods[0].weight=99;
  auto reweightedFlow=AllocateFreight(reweighted,graph,policy);
  Near(reweightedFlow.balances[1].freightDemand.target,20,"physical weight is independent of Freight conversion");
  Require(reweightedFlow.balances[1].allocatedImport==6000,"weight changes cannot invent goods supply");
  auto repriced=policy;repriced.costPerDistance[0]=9;
  auto repricedFlow=AllocateFreight(demandSnapshot,graph,repriced);
  Near(repricedFlow.balances[1].freightDemand.target,20,"tariffs do not redefine intermediary units");
  std::puts("PASS: native freight optimum, conservation, shortage, direction, hover/filter, rail availability, market isolation, cancellation and stale-world rejection");
  std::puts("PASS: Freight intermediary target/fulfilled/unmet, source attribution, shortage/disconnection, market/state scope, non-recursion, invalid inputs and filter invariance");
}
