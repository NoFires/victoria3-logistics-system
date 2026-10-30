#include "logistics_session.h"
#include "native_market_owner.h"
#include "native_state_owner.h"
#include "native_trade_identity.h"
#include <cstdio>
#include <cstdlib>
#include <functional>
using namespace v3;
void Check(bool ok, const char* label) {
  if (!ok) { std::printf("FAIL: %s\n", label); std::exit(1); }
}
EconomySnapshot Snapshot(uint64_t world = 1, uint64_t revision = 1) {
  EconomySnapshot s;
  s.world = world; s.revision = revision;
  s.goods = {{1, "grain", "staple", 1, false}, {2, "iron", "industrial", 1, false}};
  s.orders = {{10, 1, 7, 1, 3000, 0}, {30, 3, 7, 1, 0, 3000},
              {10, 1, 7, 2, 5000, 0}, {40, 4, 7, 2, 0, 5000}};
  return s;
}
GeographyHandle Geo() {
  auto g = std::make_shared<RouteGeography>();
  g->splines.nodes = {{100, 0, 0}, {200, 1, 0}, {300, 2, 0}, {400, 3, 0}};
  g->splines.byId = {{100, 0}, {200, 1}, {300, 2}, {400, 3}};
  g->splines.points = {{0, 0}, {0, 1}, {2, 0}, {2, 1}};
  g->splines.edges = {{0, 0, 2, 0, 0, 2, 1}, {256, 0, 3, 0, 2, 2, 2}};
  return g;
}
std::shared_ptr<const LogisticsDocument> Await(LogisticsSession& session,
                                               const std::function<bool(const LogisticsDocument&)>& ready) {
  const auto deadline = GetTickCount64() + 5000;
  while (GetTickCount64() < deadline) {
    auto document = session.Poll();
    if (document && (!ready || ready(*document))) return document;
    Sleep(1);
  }
  return {};
}
int main() {
  {
    LogisticsSession nativeSession(Geo());
    auto native=std::make_shared<NativeWorldTradeSnapshot>();
    native->states={{10,1,7},{30,3,7},{40,4,7}};native->centers={{10,10},{30,30},{40,30}};
    NativeTradeDestination destination;destination.state=30;destination.selected=1;
    destination.candidates={{{{1,2}},{0x7f000001},100001},{{{3,4}},{},200003}};
    native->groups={{10,{10},{destination}},{30,{30,40},{}}};
    native->statePairs={{10,30,{{1,100001}}},{30,10,{{1,200003},{2,300007}}}};native->groupPairs=native->statePairs;
    native->straits={{0x7f000001,{1,2},{0,1},0},{0x88000002,{3,4},{1,2},1}};
    NativeSplineNetwork geometry;geometry.width=100;geometry.height=50;
    geometry.points={{1,{1,2}},{2,{3,4}},{3,{5,6}}};geometry.strips={{0x100000002,{1,2,3}}};
    geometry.connections={{0x1000000400000002,{0x100000002}},{0x2000000400000002,{}}};
    NativeSeaGeography sea;sea.definitions={{0,{1,2}},{1,{3,4}}};
    sea.provinces={{1,{{101,11},{2,3}}},{2,{{102,12},{1}}}};
    sea.states={{10,{101,11}},{30,{102,12}}};sea.stateHubs={{10,true},{30,false}};
    native->geometry=std::make_shared<const NativeSplineNetwork>(geometry);
    native->seaGeography=std::make_shared<const NativeSeaGeography>(sea);
    auto snapshot=Snapshot();snapshot.nativeTrade=native;
    Check(nativeSession.SetSnapshot(std::make_shared<const EconomySnapshot>(snapshot)),"native handoff baseline accepted");
    auto repeated=snapshot;++repeated.revision;
    auto clone=std::make_shared<NativeWorldTradeSnapshot>(*native);
    clone->geometry=std::make_shared<const NativeSplineNetwork>(geometry);
    clone->seaGeography=std::make_shared<const NativeSeaGeography>(sea);repeated.nativeTrade=clone;
    Check(nativeSession.HasEquivalentSnapshot(repeated),"identical owned native contents retain accepted identity despite new allocations/revision");
    auto reordered=std::make_shared<NativeWorldTradeSnapshot>(*clone);
    std::reverse(reordered->states.begin(),reordered->states.end());
    std::reverse(reordered->groups.begin(),reordered->groups.end());
    std::reverse(reordered->groups.front().states.begin(),reordered->groups.front().states.end());
    std::reverse(reordered->statePairs.begin(),reordered->statePairs.end());
    std::reverse(reordered->groupPairs.begin(),reordered->groupPairs.end());
    std::reverse(reordered->statePairs.front().goods.begin(),reordered->statePairs.front().goods.end());
    std::reverse(reordered->groupPairs.front().goods.begin(),reordered->groupPairs.front().goods.end());
    std::reverse(reordered->straits.begin(),reordered->straits.end());
    auto reorderedSea=sea;std::reverse(reorderedSea.definitions.begin(),reorderedSea.definitions.end());
    reordered->seaGeography=std::make_shared<const NativeSeaGeography>(reorderedSea);
    auto resorted=repeated;resorted.nativeTrade=reordered;
    Check(nativeSession.HasEquivalentSnapshot(resorted),"irrelevant native enumeration order cannot blank a stable view or cancel its caches");
    auto changed=[&](auto edit,const char* why){
      auto trade=*native;auto lines=geometry;auto geography=sea;edit(trade,lines,geography);
      trade.geometry=std::make_shared<const NativeSplineNetwork>(lines);
      trade.seaGeography=std::make_shared<const NativeSeaGeography>(geography);
      auto candidate=repeated;candidate.nativeTrade=std::make_shared<const NativeWorldTradeSnapshot>(trade);
      Check(!nativeSession.HasEquivalentSnapshot(candidate),why);
    };
    changed([](auto& t,auto&,auto&){t.straits.clear();},"closed native canal refreshes even when every economy order is unchanged");
    changed([](auto& t,auto&,auto&){++t.straits[0].id;},"full strait generation changes refresh");
    changed([](auto& t,auto&,auto&){++t.straits[0].definition;},"native strait definition changes refresh");
    changed([](auto& t,auto&,auto&){++t.straits[0].tariffModes[0];},"current native passage cost changes refresh");
    changed([](auto& t,auto&,auto&){++t.straits[0].seaProvinces[0];},"current strait endpoints refresh");
    changed([](auto& t,auto&,auto&){t.groups[0].destinations[0].selected=0;},"changed original route choice refreshes without legacy-order changes");
    changed([](auto& t,auto&,auto&){t.groups[0].destinations[0].selected.reset();},"unselected native route cannot retain a selected frame");
    changed([](auto& t,auto&,auto&){std::reverse(t.groups[0].destinations[0].candidates.begin(),t.groups[0].destinations[0].candidates.end());},
        "native alternative order affects current index and tie behavior, unlike container enumeration");
    changed([](auto& t,auto&,auto&){++t.groups[0].destinations[0].candidates[0].distanceFixed;},"native alternative cost changes refresh");
    changed([](auto& t,auto&,auto&){++t.groups[0].destinations[0].candidates[1].provinces[0].last;},"native selected ProvinceSet changes refresh");
    changed([](auto& t,auto&,auto&){t.groups[0].destinations[0].candidates[0].straits.clear();},"native alternative passages refresh");
    changed([](auto& t,auto&,auto&){++t.groups[0].anchor;},"native cached representative changes refresh");
    changed([](auto& t,auto&,auto&){t.groups[1].states.pop_back();},"native cached membership changes refresh");
    changed([](auto& t,auto&,auto&){++t.groups[0].destinations[0].state;},"native cached destination changes refresh");
    changed([](auto& t,auto&,auto&){t.centers[40]=10;},"current native center changes refresh even with identical orders");
    changed([](auto& t,auto&,auto&){++t.states[0].market;},"owned native market scope changes refresh");
    changed([](auto& t,auto&,auto&){++t.states[0].region;},"owned native region changes refresh");
    changed([](auto& t,auto&,auto&){++t.states[0].id;},"owned full native state identity changes refresh");
    changed([](auto& t,auto&,auto&){++t.statePairs[0].goods[0].valueFixed;},"single native1e-5 detailed export change refreshes");
    changed([](auto& t,auto&,auto&){++t.groupPairs[0].goods[0].valueFixed;},"independent grouped evidence changes refresh");
    changed([](auto& t,auto&,auto&){std::swap(t.statePairs[0].source,t.statePairs[0].destination);},"native export direction changes refresh");
    changed([](auto& t,auto&,auto&){++t.statePairs[0].goods[0].good;},"native good identity changes refresh");
    changed([](auto&,auto& g,auto&){g.points.at(2).x+=.25f;},"same-key native control geometry changes refresh");
    changed([](auto&,auto& g,auto&){std::reverse(g.strips.begin()->second.begin(),g.strips.begin()->second.end());},"original native control order changes refresh");
    changed([](auto&,auto& g,auto&){g.connections.begin()->second.clear();},"native connection becoming known empty refreshes");
    changed([](auto&,auto& g,auto&){g.connections.erase(0x2000000400000002);},"missing and known-empty native connection are distinct");
    changed([](auto&,auto& g,auto&){++g.height;},"current native map dimensions refresh");
    changed([](auto&,auto&,auto& g){++g.definitions[0].seaProvinces[0];},"closed/unbuilt canal definition changes refresh");
    changed([](auto&,auto&,auto& g){++g.states.at(10).geo;},"same native state with changed map anchor refreshes");
    changed([](auto&,auto&,auto& g){++g.provinces.at(1).region.definition;},"native province region-definition changes refresh");
    changed([](auto&,auto&,auto& g){std::reverse(g.provinces.at(1).adjacent.begin(),g.provinces.at(1).adjacent.end());},"original native adjacency tie order changes refresh");
    changed([](auto&,auto&,auto& g){g.stateHubs.at(10)=false;},"current native port eligibility changes refresh");
    auto missing=repeated;auto incomplete=std::make_shared<NativeWorldTradeSnapshot>(*native);incomplete->geometry.reset();missing.nativeTrade=incomplete;
    Check(!nativeSession.HasEquivalentSnapshot(missing),"missing native geometry cannot retain prior loaded frame");
    incomplete=std::make_shared<NativeWorldTradeSnapshot>(*native);incomplete->seaGeography.reset();missing.nativeTrade=incomplete;
    Check(!nativeSession.HasEquivalentSnapshot(missing),"missing native geography cannot retain prior loaded frame");
    repeated.nativeTrade.reset();Check(!nativeSession.HasEquivalentSnapshot(repeated),"native-to-model source transition is never equivalent");
    LogisticsSession modelSession(Geo());modelSession.SetSnapshot(std::make_shared<const EconomySnapshot>(Snapshot()));
    Check(!modelSession.HasEquivalentSnapshot(snapshot),"model-to-native source transition cannot be hidden by unchanged orders");
  }
  LogisticsSession session(Geo(), FreightPolicy{});
  std::string reason;
  Check(!session.SetSnapshot(nullptr, &reason) && reason == "missing economy snapshot",
        "missing snapshot rejected");
  Check(session.SetSnapshot(std::make_shared<const EconomySnapshot>(Snapshot()), &reason),
        reason.c_str());
  LogisticsSelection selection;
  selection.kind = FreightViewKind::Market;
  session.SetSelection(kOwnerNativePanel, kOwnerNativePanel, selection);
  auto full = Await(session, {});
  Check(full != nullptr, "session produces a document");
  Check(full->world == 1 && full->revision == 1, "document carries the snapshot identity");
  Check(full->market == 7, "unspecified market resolves to the first snapshot market");
  Check(full->allFreight == 13 && full->filteredFreight == 13, "market freight totals");
  Check(full->targetImport == 8 && full->allocatedImport == 8 && full->unmetDemand == 0,
        "document separates model target, allocated and unmet demand");
  Check(full->heatReference == 5, "document carries the fixed market heat reference");
  Check(full->goods.size() == 2, "both market goods are copied");
  auto status = session.Status();
  Check(status.owner == kOwnerNativePanel, "owner tag retained");
  Check(status.snapshotReady && status.haveFrame, "status reports a ready frame");
  Check(status.world == 1 && status.revision == 1 && status.generation > 0, "status identity");
  Check(status.ribbons == 2 && status.allocationBuilds == 1 && status.viewBuilds == 1,
        "cold request builds one allocation and one view");
  auto originalFrame=session.Frame();
  Check(session.FrameIsCurrent(originalFrame,status.applied), "published frame passes the render publication gate");
  Check(!session.Poll(), "one completed generation publishes its copied document only once");
  Check(session.Frame()==originalFrame && session.FrameIsCurrent(originalFrame,status.applied),
        "consuming a document keeps its render frame active");

  // Paused periodic captures used to cancel the worker and blank the UI despite
  // having exactly the same inputs. Retain the accepted identity on this path.
  auto captured=Snapshot(1,2);
  if (!session.HasEquivalentSnapshot(captured))
    session.SetSnapshot(std::make_shared<const EconomySnapshot>(captured));
  Check(session.Frame()==originalFrame && session.FrameIsCurrent(originalFrame,status.applied) &&
        session.Status().generation==status.generation && !session.Poll(),
        "unchanged periodic capture must retain the published frame without cancellation");
  const std::vector<std::function<void(EconomySnapshot&)>> changes={
    [](auto& s){++s.world;}, [](auto& s){s.revision=0;},
    [](auto& s){++s.primaryMarket;}, [](auto& s){s.builtRailEdges.insert(256);},
    [](auto& s){++s.orders[0].production;}, [](auto& s){++s.orders[0].consumption;},
    [](auto& s){++s.orders[0].state;}, [](auto& s){++s.orders[0].region;},
    [](auto& s){++s.orders[0].market;}, [](auto& s){++s.orders[0].good;},
    [](auto& s){s.orders.pop_back();}, [](auto& s){s.goods.pop_back();},
    [](auto& s){++s.goods[0].id;}, [](auto& s){s.goods[0].key="coal";},
    [](auto& s){s.goods[0].category="luxury";}, [](auto& s){s.goods[0].name="localized";},
    [](auto& s){s.goods[0].weight=2;}, [](auto& s){s.goods[0].freightPerOrder=2;},
    [](auto& s){s.goods[0].localOnly=true;}, [](auto& s){s.goods[0].tradeable=false;},
    [](auto& s){s.goods[0].freightIntermediary=true;}
  };
  for (const auto& change:changes) {
    auto changed=captured;change(changed);
    Check(!session.HasEquivalentSnapshot(changed),"changed inputs or stale identity cannot skip refresh");
  }

  // An in-flight hover must retire the old MAP frame, but its statistics are
  // still exact for this market/filter. This also covers publication arriving
  // after the hover starts and before its worker reply.
  {
    LogisticsSession hoverSession(Geo());
    hoverSession.SetSnapshot(std::make_shared<const EconomySnapshot>(Snapshot()));
    auto hoverSelection=selection;
    hoverSession.SetSelection(kOwnerNativePanel,kOwnerNativePanel,hoverSelection);
    Check(Await(hoverSession,{})!=nullptr,"hover regression starts from a settled document");
    auto hoverFrame=hoverSession.Frame();auto applied=hoverSession.Status().applied;
    hoverSelection.hoverGood=1;
    hoverSession.SetSelection(kOwnerHover,kOwnerHover,hoverSelection);
    Check(!hoverSession.FrameIsCurrent(hoverFrame,applied) && hoverSession.StatisticsAreCurrent(hoverFrame),
          "hover keeps summary and goods columns valid while rejecting the old map frame");
    Check(Await(hoverSession,{})!=nullptr,"hover reply settles");
    hoverSession.Close(kOwnerHover);
    Check(hoverSession.StatisticsAreCurrent(hoverFrame),"leaving hover retains identical statistics");
    Check(Await(hoverSession,{})!=nullptr,"leaving hover settles");
    hoverSelection.market=99;hoverSelection.explicitMarket=true;
    hoverSession.SetSelection(kOwnerNativePanel,kOwnerNativePanel,hoverSelection);
    Check(!hoverSession.StatisticsAreCurrent(hoverFrame),"market switch immediately withdraws statistics");
    auto replaced=Snapshot();replaced.orders[1].consumption=1000;
    hoverSession.SetSnapshot(std::make_shared<const EconomySnapshot>(replaced));
    hoverSelection=selection;
    hoverSession.SetSelection(kOwnerNativePanel,kOwnerNativePanel,hoverSelection);
    Check(!hoverSession.StatisticsAreCurrent(hoverFrame),"same-version snapshot replacement withdraws old statistics");
    Check(Await(hoverSession,{})!=nullptr && !hoverSession.StatisticsAreCurrent(hoverFrame),
          "completed replacement cannot revive an old allocation with equal world/revision");
    hoverSession.ResetWorld();
    Check(!hoverSession.HasEquivalentSnapshot(captured) && !hoverSession.StatisticsAreCurrent(hoverFrame),
          "retired world cannot reuse identical capture or statistics");
    hoverSession.SetSnapshot(std::make_shared<const EconomySnapshot>(Snapshot(2)),nullptr,true);
    Check(!hoverSession.HasEquivalentSnapshot(Snapshot(2,2)),"synthetic input cannot suppress a real capture");
  }

  // Search only changes the aggregate; the denominator and heat scale stay fixed.
  selection.search = "grain";
  session.SetSelection(kOwnerNativePanel, kOwnerNativePanel, selection);
  Check(!session.FrameIsCurrent(originalFrame,status.applied), "pending selection immediately rejects old render publication");
  Check(!session.StatisticsAreCurrent(originalFrame),"search changes must withdraw prior statistics immediately");
  auto grain = Await(session, [](const LogisticsDocument& d) { return d.filteredFreight == 3; });
  Check(grain != nullptr, "search filter produces the filtered aggregate");
  Check(grain->allFreight == 13 && grain->heatReference == 5,
        "filter preserves market denominator and heat scale");
  Check(grain->targetImport == 8 && grain->unmetDemand == 0,
        "filter preserves model demand totals");
  status = session.Status();
  Check(status.allocationBuilds == 1 && status.viewBuilds == 2, "filter reuses allocation, rebuilds view");
  Check(status.ribbons == 1, "filtered frame contains only the visible edge");

  selection.search = "";
  selection.hoverGood = 2;
  session.SetSelection(kOwnerNativePanel, kOwnerNativePanel, selection);
  auto hover = Await(session, [](const LogisticsDocument& d) { return d.filteredFreight == 10; });
  Check(hover != nullptr, "hover selects the hovered good");
  Check(session.Status().allocationBuilds == 1, "hover never reruns allocation");

  // Close cancels and stops publishing; the snapshot is retained for reopen.
  session.Close(kOwnerNativePanel);
  Sleep(20);
  Check(!session.Poll(), "closed session publishes nothing");
  Check(!session.Status().haveFrame, "closed session reports no visible frame");
  Check(!session.StatisticsAreCurrent(originalFrame),"closing the last owner withdraws statistics");
  session.SetSelection(kOwnerNativePanel, kOwnerNativePanel, selection);
  auto reopened = Await(session, {});
  Check(reopened != nullptr, "reopen publishes again from the retained snapshot");
  Check(session.Status().viewReused && session.Status().allocationReused,
        "reopen reuses allocation and view caches");

  auto stale = Snapshot(); stale.revision = 0;
  Check(!session.SetSnapshot(std::make_shared<const EconomySnapshot>(stale), &reason) &&
        reason == "stale economy revision", "stale revision rejected by the session");

  auto next = Snapshot(2, 1); next.orders[0].production = 9000; next.orders[1].consumption = 6000;
  Check(session.SetSnapshot(std::make_shared<const EconomySnapshot>(next)),
        "new world accepted");
  Check(!session.Frame() && !session.Status().haveFrame,
        "accepting a new snapshot immediately retires the old visible frame");
  Check(!session.FrameIsCurrent(originalFrame,status.applied), "snapshot replacement blocks old render publication");
  auto updated = Await(session, [](const LogisticsDocument& d) { return d.world == 2; });
  Check(updated != nullptr, "new world produces a new document");
  Check(updated->allFreight != 13, "new snapshot numbers are used");
  status = session.Status();
  Check(status.world == 2 && status.allocationBuilds == 2, "world change rebuilds the allocation");

  session.ResetWorld();
  Sleep(20);
  Check(!session.Poll() && !session.Status().snapshotReady, "reset world clears the session view");
  Check(!session.SetSnapshot(std::make_shared<const EconomySnapshot>(next), &reason) &&
        reason=="stale world generation", "session rejects retired world while snapshot is absent");
  auto staleAfterReset=Snapshot(1,999);
  Check(!session.SetSnapshot(std::make_shared<const EconomySnapshot>(staleAfterReset), &reason),
        "a late old world cannot bypass reset by using a high revision");
  auto later = Snapshot(3, 1);
  Check(session.SetSnapshot(std::make_shared<const EconomySnapshot>(later)), "post-reset snapshot accepted");
  session.SetSelection(kOwnerNativePanel, kOwnerNativePanel, selection);
  auto postReset = Await(session, [](const LogisticsDocument& d) { return d.world == 3; });
  Check(postReset != nullptr, "post-reset session publishes a new world frame");

  // Multi-owner arbitration: a transient hover overrides the base panel, and
  // closing it restores the base view instead of clearing the map.
  LogisticsSelection base;
  base.kind = FreightViewKind::Market;
  session.SetSelection(kOwnerNativePanel, kOwnerNativePanel, base);
  auto baseView = Await(session, [](const LogisticsDocument& d) { return d.filteredFreight == 13; });
  Check(baseView != nullptr, "base panel owns the map");
  LogisticsSelection hoverSelection;
  hoverSelection.kind = FreightViewKind::Market;
  hoverSelection.hoverGood = 2;
  session.SetSelection(kOwnerHover, kOwnerHover, hoverSelection);
  auto hoverView = Await(session, [](const LogisticsDocument& d) { return d.filteredFreight == 10; });
  Check(hoverView != nullptr, "hover owner overrides the base selection");
  Check(session.Status().owner == kOwnerHover, "status reports the active hover owner");
  session.Close(kOwnerHover);
  auto restored = Await(session, [](const LogisticsDocument& d) { return d.filteredFreight == 13; });
  Check(restored != nullptr, "closing hover restores the base view");
  Check(session.Status().owner == kOwnerNativePanel, "base owner restored");
  Check(session.Status().viewReused || session.Status().allocationReused,
        "restoring the base reuses the cached allocation");

  // The debug preview outranks the panel and hover; hover churn cannot clear it.
  LogisticsSelection debugSelection;
  debugSelection.kind = FreightViewKind::Market;
  debugSelection.search = "grain";
  session.SetSelection(kOwnerDebugPreview, kOwnerDebugPreview, debugSelection);
  auto debugView = Await(session, [](const LogisticsDocument& d) { return d.filteredFreight == 3; });
  Check(debugView != nullptr, "debug preview owns the map at the highest priority");
  session.SetSelection(kOwnerHover, kOwnerHover, hoverSelection);
  session.Poll();
  Check(session.Status().owner == kOwnerDebugPreview, "hover cannot steal an active debug owner");
  session.Close(kOwnerHover);
  session.Close(kOwnerDebugPreview);
  auto backToBase = Await(session, [](const LogisticsDocument& d) { return d.filteredFreight == 13; });
  Check(backToBase != nullptr, "base survives closing the other owners");

  // Closing the base while hover is active leaves the hover owner in charge,
  // and closing every owner cancels the view without touching the snapshot.
  session.SetSelection(kOwnerHover, kOwnerHover, hoverSelection);
  Await(session, [](const LogisticsDocument& d) { return d.filteredFreight == 10; });
  session.Close(kOwnerNativePanel);
  Check(session.Status().owner == kOwnerHover, "closing the base leaves the hover active");
  Check(session.Status().snapshotReady, "closing owners retains the copied snapshot");
  session.Close(kOwnerHover);
  Check(!session.Status().active && !session.Status().haveFrame,
        "closing every owner clears the active view");

  {
    LogisticsSession native(Geo(),FreightPolicy{});NativeMarketOwner owner;
    auto economy=Snapshot();economy.primaryMarket=7;
    economy.orders.push_back({50,1,8,1,1000,0});economy.orders.push_back({60,3,8,1,0,1000});
    economy.orders.push_back({70,1,0,1,1000,0});economy.orders.push_back({80,3,0,1,0,1000});
    Check(native.SetSnapshot(std::make_shared<const EconomySnapshot>(economy)),"native bridge snapshot accepted");
    std::unordered_set<uint32_t> markets{0,7,8};NativeMarketPanelState panel;
    panel.available=panel.ingame=true;panel.tab="default";
    Check(!owner.Update(native,panel,markets,true).changed && !native.Poll() && native.Status().allocationBuilds==0,
          "snapshot availability never activates a closed formal panel or runs allocation");
    panel.marketOpen=true;panel.market=8;panel.tab="ironroad_logistics";
    panel.visibleGoods=std::vector<uint32_t>{1,2};
    Check(owner.Update(native,panel,markets,true).changed,"logistics tab activates formal owner");
    auto panelDoc=Await(native,[](const auto& d){return d.market==8;});
    Check(panelDoc && panelDoc->market==8 && panelDoc->targetImport==1,"panel market replaces heuristic primary market");
    auto gen=native.Status().generation;
    Check(!owner.Update(native,panel,markets,true).changed && !native.Poll() && native.Status().generation==gen,
          "identical UI timer samples cannot restart settled work");
    auto priorFrame=native.Frame();auto priorApplied=native.Status().applied;
    panel.market=7;Check(owner.Update(native,panel,markets,true).changed && !native.FrameIsCurrent(priorFrame,priorApplied),
          "market switch immediately rejects previous frame at session seam");
    Check(Await(native,[](const auto& d){return d.market==7;})!=nullptr,"switch produces the newly selected market");
    const auto builds=native.Status().allocationBuilds;
    panel.visibleGoods=std::vector<uint32_t>{1};owner.Update(native,panel,markets,true);
    auto onlyGrain=Await(native,[](const auto& d){return d.market==7 && d.filteredFreight==3;});
    Check(onlyGrain && onlyGrain->allFreight==13 && onlyGrain->targetImport==8 &&
          onlyGrain->intermediaryDemand->target==8 && native.Status().allocationBuilds==builds,
          "native matched rows change only the view, not allocation, market denominator or Freight demand");
    panel.visibleGoods=std::vector<uint32_t>{};owner.Update(native,panel,markets,true);
    Check(Await(native,[](const auto& d){return d.filteredFreight==0 && d.allFreight==13;})!=nullptr,
          "known empty native search has no ribbons but retains all-market statistics");
    panel.visibleGoods.reset();Check(owner.Update(native,panel,markets,true).closed && !native.Frame(),
          "pending native search rows withdraw the prior view instead of masquerading as zero matches");
    panel.visibleGoods=std::vector<uint32_t>{1,2};owner.Update(native,panel,markets,true);
    Check(Await(native,{})!=nullptr,"settled native filter restores the view");
    panel.marketOpen=false;Check(owner.Update(native,panel,markets,true).closed && !native.Frame() && !native.Poll(),
          "panel close cancels frame and cannot republish retained reply");
    panel.marketOpen=true;owner.Update(native,panel,markets,true);
    Check(Await(native,{})!=nullptr,"reopen restores formal view");
    panel.tab="default";Check(owner.Update(native,panel,markets,true).closed,"leaving logistics tab closes only formal owner");
    panel.tab="ironroad_logistics";panel.market=999;
    Check(!owner.Update(native,panel,markets,true).changed && !native.Poll(),"unknown market has no heuristic fallback");
    panel.market=8;owner.Update(native,panel,markets,true,7);
    Check(Await(native,[](const auto& d){return d.market==7;})!=nullptr,"explicit debug override remains controllable");
    Check(owner.Update(native,panel,markets,false).closed,"missing metadata withdraws active formal view");
    auto synthetic=economy;synthetic.revision=2;
    native.SetSnapshot(std::make_shared<const EconomySnapshot>(synthetic),nullptr,true);
    Check(!owner.Update(native,panel,markets,true).changed && !native.Poll(),"synthetic snapshot cannot activate formal owner");
    native.ResetWorld();auto newWorld=economy;newWorld.world=2;
    native.SetSnapshot(std::make_shared<const EconomySnapshot>(newWorld));
    Check(owner.Update(native,panel,markets,true).changed && Await(native,{})!=nullptr,"new world can reacquire current panel state");
    LogisticsSelection debug;debug.market=7;
    native.SetSelection(kOwnerDebugPreview,kOwnerDebugPreview,debug);
    Check(Await(native,[](const auto& d){return d.market==7;})!=nullptr,"debug owner can temporarily take priority");
    auto debugFrame=native.Frame();auto debugApplied=native.Status().applied;
    panel.marketOpen=false;
    Check(owner.Update(native,panel,markets,true).closed && native.Frame()==debugFrame &&
          native.FrameIsCurrent(debugFrame,debugApplied),"formal panel close preserves another owner's current frame");
    native.Close(kOwnerDebugPreview);panel.marketOpen=true;panel.market=0;
    Check(owner.Update(native,panel,markets,true).changed,"explicit native market zero can activate");
    Check(Await(native,[](const auto& d){return d.market==0 && d.targetImport==1;})!=nullptr,
          "native market zero is not silently replaced by primary market seven");
    panel.market=7;owner.Update(native,panel,markets,true);Await(native,{});
    const auto hoverBuilds=native.Status().allocationBuilds;
    const std::unordered_map<std::string,uint32_t> goodsByKey{{"grain",1},{"iron",2}};
    const auto context=owner.Context();
    GuiVariableProbe hover;hover.ok=true;
    hover.entries={{0,"ironroad_market_hover_grain",context}};
    auto baseFrame=native.Frame();auto baseApplied=native.Status().applied;
    Check(owner.UpdateHover(native,hover,goodsByKey) && !native.FrameIsCurrent(baseFrame,baseApplied),
          "native enter invalidates the base frame before settling");
    auto hovered=Await(native,[](const auto& d){return d.filteredFreight==3;});
    Check(hovered && native.Status().owner==kOwnerHover && hovered->allFreight==13 &&
          hovered->intermediaryDemand->target==8 && native.Status().allocationBuilds==hoverBuilds,
          "native hover extracts one good without changing the whole-market denominator or Freight demand");
    auto hoverGeneration=native.Status().generation;
    Check(!owner.UpdateHover(native,hover,goodsByKey) && native.Status().generation==hoverGeneration,
          "unchanged native hover cannot restart the worker");
    hover.entries.push_back({0,"ironroad_market_hover_iron",context});
    Check(owner.UpdateHover(native,hover,goodsByKey) && Await(native,[](const auto& d){return d.filteredFreight==13;})!=nullptr,
          "ambiguous simultaneous native rows restore the base, never choose the first");
    hover.entries.erase(hover.entries.begin());owner.UpdateHover(native,hover,goodsByKey);
    Check(Await(native,[](const auto& d){return d.filteredFreight==10;})!=nullptr,
          "a late leave from the previous row preserves the newer row's independent key");
    auto unreadable=hover;unreadable.ok=false;owner.UpdateHover(native,unreadable,goodsByKey);
    Check(Await(native,[](const auto& d){return d.filteredFreight==13;})!=nullptr,
          "failed native variable read withdraws only temporary hover");
    hover.entries={{0,"ironroad_market_hover_unknown",context}};
    Check(!owner.UpdateHover(native,hover,goodsByKey),"unknown goods keys cannot become a native selection");
    panel.visibleGoods=std::vector<uint32_t>{1};owner.Update(native,panel,markets,true);Await(native,{});
    hover.entries={{0,"ironroad_market_hover_iron",context}};
    Check(!owner.UpdateHover(native,hover,goodsByKey),"hover cannot reintroduce a good excluded by native filters");
    hover.entries={{0,"ironroad_market_hover_grain",context}};owner.UpdateHover(native,hover,goodsByKey);Await(native,{});
    panel.marketOpen=false;owner.Update(native,panel,markets,true);
    Check(!native.Frame() && !native.Poll() && !native.Status().active,
          "native panel close cancels both base and its hover without waiting for settle");
    panel.marketOpen=true;owner.Update(native,panel,markets,true);Await(native,{});
    Check(owner.Context()!=context && !owner.UpdateHover(native,hover,goodsByKey),
          "reopening rejects a retained hover entry from the previous panel context");
    hover.entries[0].value=owner.Context();owner.UpdateHover(native,hover,goodsByKey);Await(native,{});
    panel.tab="default";owner.Update(native,panel,markets,true);
    Check(!native.Status().active && !native.Frame(),"tab change also cancels native hover");
    panel.tab="ironroad_logistics";owner.Update(native,panel,markets,true);Await(native,{});
    hover.entries[0].value=owner.Context();owner.UpdateHover(native,hover,goodsByKey);Await(native,{});
    native.SetSelection(kOwnerDebugPreview,kOwnerDebugPreview,debug);Await(native,{});
    debugFrame=native.Frame();debugApplied=native.Status().applied;owner.Close(native);
    Check(native.Frame()==debugFrame && native.FrameIsCurrent(debugFrame,debugApplied),
          "native base plus hover closing cannot erase an independent debug owner's frame");
    Check(IsNativeLogisticsOwner(kOwnerHover) && IsNativeLogisticsOwner(kOwnerNativePanel) &&
          !IsNativeLogisticsOwner(kOwnerDebugPreview),"formal hover renders with ImGui hidden, debug remains independent");
  }
  {
    LogisticsSession native(Geo());NativeStateOwner owner;
    auto economy=std::make_shared<const EconomySnapshot>(Snapshot());
    native.SetSnapshot(economy);auto roster=CopyNativeStateRoster(economy);
    Check(roster.markets.size()==3 && roster.markets.at(30)==7,"copied state roster deduplicates agreeing goods rows");
    NativeStatePanelState panel;panel.available=panel.ingame=true;panel.state=30;panel.tab="local_goods";
    NativeMarketOwner marketOwner;
    NativeMarketPanelState marketPanel;marketPanel.available=marketPanel.ingame=marketPanel.marketOpen=true;
    marketPanel.market=0;marketPanel.tab="ironroad_logistics";marketPanel.visibleGoods=std::vector<uint32_t>{1};
    Check(!marketOwner.Update(native,marketPanel,{7},true).changed,
          "unmapped GUI market must stay inactive while state snapshot remains usable");
    Check(!owner.Update(native,panel,roster,true).changed && !native.Poll(),"closed state cannot activate a view");
    panel.stateOpen=true;Check(owner.Update(native,panel,roster,true).changed,"open native state acquires its own owner");
    auto doc=Await(native,{});auto frame=native.Frame();auto applied=native.Status().applied;
    Check(doc && doc->kind==FreightViewKind::State && doc->state==30 && doc->market==7 &&
          doc->selectionFreight==3 && doc->allFreight==13 && native.Status().owner==kOwnerNativeState &&
          owner.CanPublish(native,*doc,frame,applied),"state frame and publication retain scope plus fixed market denominator");
    Check(owner.ReadyId()=="30" && FormatStateFreightGoods(doc.get(),30).size()==2,"state ready ID matches canonical row payload");
    auto missing=panel;missing.available=false;
    Check(owner.Update(native,missing,roster,true).closed && !owner.CanPublish(native,*doc,frame,applied),
          "unreadable state panel withdraws the queued rows");
    owner.Update(native,panel,roster,true);doc=Await(native,{});frame=native.Frame();applied=native.Status().applied;
    auto generation=native.Status().generation;
    Check(!owner.Update(native,panel,roster,true).changed && native.Status().generation==generation && !native.Poll(),
          "unchanged state sampling does not restart worker or duplicate publication");
    auto wrong=*doc;wrong.kind=FreightViewKind::Market;
    Check(!owner.CanPublish(native,wrong,frame,applied),"market document cannot be paired with state frame");
    wrong=*doc;wrong.synthetic=true;
    Check(!owner.CanPublish(native,wrong,frame,applied),"synthetic document cannot use a real state frame");
    const auto builds=native.Status().allocationBuilds;
    panel.state=40;owner.Update(native,panel,roster,true);
    Check(!owner.CanPublish(native,*doc,frame,applied),"state switch immediately rejects queued publication");
    doc=Await(native,{});
    Check(doc && doc->state==40 && doc->selectionFreight==10 && native.Status().allocationBuilds==builds,
          "switching states reuses allocation and extracts only the new state's flows");
    auto changed=Snapshot();for(auto& row:changed.orders)if(row.state==40)row.market=8;
    auto replacement=std::make_shared<const EconomySnapshot>(changed);native.SetSnapshot(replacement);
    auto changedDoc=Await(native,{});
    Check(changedDoc && !owner.CanPublish(native,*changedDoc,native.Frame(),native.Status().applied),
          "new frame with equal version cannot use an owner validated against replaced source");
    Check(!native.SnapshotIsCurrent(economy) && owner.Update(native,panel,roster,true).closed && !native.Poll(),
          "equal-world/revision replacement rejects stale roster identity");
    roster=CopyNativeStateRoster(replacement);owner.Update(native,panel,roster,true);doc=Await(native,{});
    Check(doc && doc->market==8 && doc->state==40 && doc->allocatedImport==0,
          "fresh roster selects changed state's actual market without guessing primary market");
    frame=native.Frame();applied=native.Status().applied;
    panel.tab="buildings";
    Check(owner.Update(native,panel,roster,true).closed && !native.Frame() &&
          !owner.CanPublish(native,*doc,frame,applied) && !native.Poll(),"tab close cancels frame and late state publication");
    panel.tab="local_goods";owner.Update(native,panel,roster,true);Check(Await(native,{})!=nullptr,"state reopening restores cached view");
    Check(owner.Update(native,panel,roster,false).closed,"missing metadata withdraws state owner");
    panel.state=999;Check(!owner.Update(native,panel,roster,true).changed,"unknown state has no fallback to first roster entry");
    panel.state=30;owner.Update(native,panel,roster,true);Await(native,{});
    LogisticsSelection debug;debug.market=7;native.SetSelection(kOwnerDebugPreview,kOwnerDebugPreview,debug);Await(native,{});
    frame=native.Frame();applied=native.Status().applied;
    panel.stateOpen=false;
    Check(owner.Update(native,panel,roster,true).closed && native.FrameIsCurrent(frame,applied),
          "state close preserves another owner's settled frame");
    native.Close(kOwnerDebugPreview);
    LogisticsSelection base;base.market=7;native.SetSelection(kOwnerNativePanel,kOwnerNativePanel,base);Await(native,{});
    panel.stateOpen=true;owner.Update(native,panel,roster,true);doc=Await(native,{});
    Check(doc && doc->kind==FreightViewKind::State,"state owner overrides retained market base");
    owner.Close(native);doc=Await(native,{});
    Check(doc && doc->kind==FreightViewKind::Market && native.Status().owner==kOwnerNativePanel,
          "closing only state owner restores retained market base");
    native.Close(kOwnerNativePanel);owner.Update(native,panel,roster,true);doc=Await(native,{});
    frame=native.Frame();applied=native.Status().applied;native.ResetWorld();
    Check(owner.Update(native,panel,roster,true).closed && !owner.CanPublish(native,*doc,frame,applied),
          "world reset invalidates roster and state publication immediately");
    auto next=Snapshot(2);economy=std::make_shared<const EconomySnapshot>(next);native.SetSnapshot(economy);
    roster=CopyNativeStateRoster(economy);owner.Update(native,panel,roster,true);doc=Await(native,{});
    Check(doc && doc->world==2 && owner.CanPublish(native,*doc,native.Frame(),native.Status().applied),
          "new-world copied roster can acquire the current state");
    next.revision=2;economy=std::make_shared<const EconomySnapshot>(next);native.SetSnapshot(economy,nullptr,true);
    roster=CopyNativeStateRoster(economy);
    Check(owner.Update(native,panel,roster,true).closed,"synthetic snapshots cannot retain a formal state owner");
    next=Snapshot(3);next.orders[0].state=0;next.orders[0].market=0;next.orders[1].market=0;
    economy=std::make_shared<const EconomySnapshot>(next);native.SetSnapshot(economy);roster=CopyNativeStateRoster(economy);
    panel.state=0;owner.Update(native,panel,roster,true);doc=Await(native,{});
    Check(doc && doc->state==0 && doc->market==0 && owner.ReadyId()=="0","state zero and market zero remain explicit identities");
    next.revision=2;next.orders[0].state=0x80000011;
    economy=std::make_shared<const EconomySnapshot>(next);native.SetSnapshot(economy);roster=CopyNativeStateRoster(economy);
    panel.state=0x80000011;owner.Update(native,panel,roster,true);doc=Await(native,{});
    Check(doc && owner.ReadyId()=="-2147483631" && owner.CanPublish(native,*doc,native.Frame(),native.Status().applied),
          "state publication gate matches native signed ID strings");
    auto invalid=next;invalid.orders.push_back(invalid.orders[0]);invalid.orders.back().market=9;
    Check(!CopyNativeStateRoster(std::make_shared<const EconomySnapshot>(invalid)).snapshot,"conflicting state markets reject whole roster");
    invalid=next;invalid.orders[0].state=UINT32_MAX;
    Check(!CopyNativeStateRoster(std::make_shared<const EconomySnapshot>(invalid)).snapshot,"unknown state IDs reject whole roster");
    Check(IsNativeLogisticsOwner(kOwnerNativeState),"state native owner does not require debug UI visibility");
  }
  std::puts("PASS: logistics session gating, multi-owner arbitration, native market bridge and state roster/scope/publication lifecycle (offline)");
}
