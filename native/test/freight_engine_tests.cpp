#include "freight_engine.h"
#include "freight_worker.h"
#include "native_sea_geography.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
using namespace v3;
void Check(bool ok, const char* label) {
  if (!ok) { std::printf("FAIL: %s\n", label); std::exit(1); }
}
EconomySnapshot Snapshot() {
  EconomySnapshot s;
  s.world = 1; s.revision = 1;
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
bool HasEdge(const FreightView& view, uint32_t edge, uint8_t* band = nullptr) {
  for (const auto& segment : view.segments) {
    if (segment.edge != edge) continue;
    if (band) *band = segment.band;
    return true;
  }
  return false;
}
int main() {
  {
    // Content identity must cover native facts even before the formal adapter
    // is enabled. Same revision/key/handle can have different route geometry.
    NativeWorldTradeSnapshot original;
    original.states={{10,1,7},{30,3,8},{40,4,8}};
    original.centers={{10,10},{30,30},{40,30}};
    NativeTradeDestination dest;dest.state=30;dest.selected=1;
    dest.candidates={{{{1,2}},{0x7f000001},100001},{{{3,4}},{0x88000002},200003}};
    original.groups={{10,{10,40},{dest}},{30,{30},{}}};
    original.statePairs={{10,30,{{1,100001},{2,200003}}},{30,10,{{1,400007}}}};
    original.groupPairs=original.statePairs;
    original.straits={{0x7f000001,{1,2},{0,1},0},{0x88000002,{3,4},{2,3},1}};
    NativeSplineNetwork geometry;geometry.width=100;geometry.height=50;
    geometry.points={{1,{1,2}},{2,{3,4}},{3,{5,6}}};
    geometry.strips={{0x100000007,{1,2}},{0x200000007,{2,3}}};
    geometry.connections={{0x1000000400000002,{0x100000007,0x200000007}},{0x2000000400000002,{}}};
    NativeSeaGeography sea;sea.definitions={{0,{1,2}},{1,{3,4}}};
    sea.provinces={{1,{{101,11},{2,3}}},{2,{{102,12},{1}}}};
    sea.states={{10,{101,11}},{30,{102,12}}};
    sea.stateHubs={{10,true},{30,false}};
    original.geometry=std::make_shared<const NativeSplineNetwork>(geometry);
    original.seaGeography=std::make_shared<const NativeSeaGeography>(sea);
    FreightEngine identity(Geo());auto snapshot=Snapshot();
    auto set=[&](const auto& trade){snapshot.nativeTrade=std::make_shared<const NativeWorldTradeSnapshot>(trade);
      Check(identity.SetSnapshot(snapshot),"same-revision native content accepted");return identity.SnapshotFingerprint();};
    Check(identity.SetSnapshot(snapshot),"model-only identity accepted");
    const auto model=identity.SnapshotFingerprint(),baseline=set(original);
    Check(baseline!=model,"native input cannot inherit model cache identity");
    auto reordered=original;
    std::reverse(reordered.states.begin(),reordered.states.end());
    std::reverse(reordered.groups.begin(),reordered.groups.end());
    std::reverse(reordered.groups.back().states.begin(),reordered.groups.back().states.end());
    std::reverse(reordered.statePairs.begin(),reordered.statePairs.end());
    std::reverse(reordered.statePairs.back().goods.begin(),reordered.statePairs.back().goods.end());
    std::reverse(reordered.groupPairs.begin(),reordered.groupPairs.end());
    std::reverse(reordered.groupPairs.back().goods.begin(),reordered.groupPairs.back().goods.end());
    std::reverse(reordered.straits.begin(),reordered.straits.end());
    auto reverseSea=sea;std::reverse(reverseSea.definitions.begin(),reverseSea.definitions.end());
    reordered.geometry=std::make_shared<const NativeSplineNetwork>(geometry);
    reordered.seaGeography=std::make_shared<const NativeSeaGeography>(reverseSea);
    Check(set(reordered)==baseline,"native container iteration order and allocation addresses are not cache identity");
    auto changed=[&](auto edit,const char* why){
      auto trade=original;auto lines=geometry;auto geography=sea;
      edit(trade,lines,geography);
      trade.geometry=std::make_shared<const NativeSplineNetwork>(std::move(lines));
      trade.seaGeography=std::make_shared<const NativeSeaGeography>(std::move(geography));
      Check(set(trade)!=baseline,why);Check(set(original)==baseline,"original owned content restores its original identity");
    };
    changed([](auto& t,auto&,auto&){++t.statePairs[0].goods[0].valueFixed;},"one native 1e-5 quantity change invalidates cached statistics");
    changed([](auto& t,auto&,auto&){++t.groupPairs[0].goods[0].valueFixed;},"independent grouped export evidence participates in identity");
    changed([](auto& t,auto&,auto&){t.centers[40]=10;},"current area center change invalidates old cached route association");
    changed([](auto& t,auto&,auto&){std::swap(t.statePairs[0].source,t.statePairs[0].destination);},"native flow direction participates in identity");
    changed([](auto& t,auto&,auto&){++t.statePairs[0].goods[0].good;},"native goods association participates in identity");
    changed([](auto& t,auto&,auto&){t.groups[0].destinations[0].selected=0;},"native selected alternative invalidates cached geometry");
    changed([](auto& t,auto&,auto&){t.groups[0].destinations[0].selected.reset();},"unavailable selection cannot reuse a selected route");
    changed([](auto& t,auto&,auto&){std::reverse(t.groups[0].destinations[0].candidates.begin(),t.groups[0].destinations[0].candidates.end());},"native candidate order remains significant for stable ties");
    changed([](auto& t,auto&,auto&){++t.groups[0].destinations[0].candidates[1].distanceFixed;},"native path distance participates in identity");
    changed([](auto& t,auto&,auto&){++t.groups[0].destinations[0].candidates[1].provinces[0].last;},"selected province set participates in identity");
    changed([](auto& t,auto&,auto&){t.groups[0].destinations[0].candidates[1].straits.clear();},"native path passage constraints participate in identity");
    changed([](auto& t,auto&,auto&){t.groups[0].states.pop_back();},"native group membership participates in identity");
    changed([](auto& t,auto&,auto&){++t.groups[0].anchor;},"native route representative participates in identity");
    changed([](auto& t,auto&,auto&){++t.groups[0].destinations[0].state;},"native destination participates in identity");
    changed([](auto& t,auto&,auto&){++t.states[0].market;},"state market reassignment invalidates scoped totals");
    changed([](auto& t,auto&,auto&){++t.states[0].region;},"state region reassignment invalidates geometry identity");
    changed([](auto& t,auto&,auto&){t.states[0].id+=0x1000000;},"full state generation participates in identity");
    changed([](auto& t,auto&,auto&){t.straits.pop_back();},"closing a native canal invalidates cached geometry");
    changed([](auto& t,auto&,auto&){t.straits[0].id+=0x1000000;},"full strait generation cannot reuse the prior route");
    changed([](auto& t,auto&,auto&){++t.straits[0].definition;},"runtime strait definition association participates in identity");
    changed([](auto& t,auto&,auto&){++t.straits[0].tariffModes[0];},"native tariffs invalidate selection-dependent results");
    changed([](auto& t,auto&,auto&){++t.straits[0].seaProvinces[0];},"native passage endpoints participate in identity");
    changed([](auto&,auto& g,auto&){g.points[2].x+=.25f;},"same native keys and endpoints with changed controls invalidate geometry");
    changed([](auto&,auto& g,auto&){g.points[2].z+=.25f;},"native second map coordinate participates in identity");
    changed([](auto&,auto& g,auto&){++g.width;},"current map width participates in identity");
    changed([](auto&,auto& g,auto&){++g.height;},"current map height participates in identity");
    changed([](auto&,auto& g,auto&){std::reverse(g.strips.begin()->second.begin(),g.strips.begin()->second.end());},"authored control orientation participates in identity");
    changed([](auto&,auto& g,auto&){g.strips[0x300000007]=g.strips.at(0x100000007);g.strips.erase(0x100000007);},"native segment ID upper bits are not truncated");
    changed([](auto&,auto& g,auto&){std::reverse(g.connections.begin()->second.begin(),g.connections.begin()->second.end());},"native ordered segment membership participates in identity");
    changed([](auto&,auto& g,auto&){g.connections.erase(std::next(g.connections.begin()));},"known empty connection differs from absent connection");
    changed([](auto&,auto&,auto& s){++s.definitions[0].seaProvinces[0];},"unbuilt canal definition endpoints invalidate passability geometry");
    changed([](auto&,auto&,auto& s){++s.provinces[1].region.geo;},"same province with a new map anchor cannot reuse old keys");
    changed([](auto&,auto&,auto& s){++s.provinces[1].region.definition;},"province region definition participates in identity");
    changed([](auto&,auto&,auto& s){std::reverse(s.provinces[1].adjacent.begin(),s.provinces[1].adjacent.end());},"native adjacency enumeration order stays significant");
    changed([](auto&,auto&,auto& s){++s.states[10].geo;},"state naval anchor participates in identity");
    changed([](auto&,auto&,auto& s){++s.states[10].definition;},"state region definition participates in identity");
    changed([](auto&,auto&,auto& s){s.stateHubs[10]=false;},"current hub eligibility invalidates native port geometry");
    changed([](auto&,auto&,auto& s){s.stateHubs.erase(30);},"unknown hub eligibility differs from a known ineligible endpoint");
    auto missing=original;missing.geometry.reset();Check(set(missing)!=baseline,"missing geometry cannot hit copied native geometry");
    missing=original;missing.seaGeography.reset();Check(set(missing)!=baseline,"missing passability identity cannot hit copied native constraints");
    Check(identity.Stats().allocations==0 && identity.Stats().views==0,"native fingerprinting does not run an allocation or publish a view");
  }
  {
    FreightEngine nativeOnly(Geo());auto original=Snapshot();
    original.nativeTrade=std::make_shared<const NativeWorldTradeSnapshot>();
    Check(nativeOnly.SetSnapshot(original),"owned original trade snapshot handed to worker engine");
    FreightFilter scope;scope.market=7;auto pending=nativeOnly.Build(scope);
    Check(pending->error=="native trade geometry unavailable" && !pending->allocation && !pending->view &&
        pending->ribbons.empty() && nativeOnly.Stats().allocations==0,
        "incomplete original trade adapter cannot fall back to model routes or model statistics");
  }
  {
    FreightEngine nativeRows(Geo());nativeRows.SetSnapshot(Snapshot());
    FreightFilter all;all.market=7;auto full=nativeRows.Build(all);
    auto selected=all;selected.restrictGoods=true;selected.allowedGoods={1};
    auto grain=nativeRows.Build(selected);
    Check(grain->allocationReused && grain->view->segments.size()==1 && grain->view->filteredFreight==3,
          "native row membership restricts only shipment contributions");
    Check(grain->view->marketFreight==full->view->marketFreight && grain->view->targetImport==8 &&
          grain->view->intermediaryDemand->target==8 && grain->view->heatReference==full->view->heatReference,
          "native row filtering preserves denominator, goods target, Freight target and heat reference");
    selected.allowedGoods={2,1};auto both=nativeRows.Build(selected);
    selected.allowedGoods={1,2,1};auto reordered=nativeRows.Build(selected);
    Check(reordered->allocationReused && reordered->viewReused && reordered->ribbons.size()==both->ribbons.size(),
          "native goods sorting/duplicates do not invalidate a settled view");
    selected.allowedGoods.clear();auto none=nativeRows.Build(selected);
    Check(none->allocationReused && none->ribbons.empty() && none->view->marketFreight==13,
          "empty native selection is a known empty view, not an all-goods fallback");
  }
  FreightEngine engine(Geo());
  std::string reason;
  Check(engine.Build({})->error == "no economy snapshot", "no snapshot is an explicit state");
  auto snapshot = Snapshot();
  Check(engine.SetSnapshot(snapshot, &reason), reason.c_str());
  FreightFilter filter; filter.market = 7;
  auto first = engine.Build(filter);
  Check(first->error.empty(), first->error.c_str());
  Check(!first->allocationReused && !first->viewReused, "cold build computes both layers");
  Check(first->ribbons.size() == 2, "both market edges become native ribbons");
  Check(first->world == 1 && first->economyRevision == 1 && first->epoch == engine.Epoch(),
        "frame identity matches the accepted snapshot");
  auto repeat = engine.Build(filter);
  Check(repeat->allocationReused && repeat->viewReused, "unchanged view reuses both caches");
  Check(repeat->ribbons.size() == first->ribbons.size() && SameRibbon(*repeat->ribbons[0], *first->ribbons[0]),
        "cached view produces identical native geometry");
  Check(engine.Stats().allocations == 1 && engine.Stats().views == 1, "unchanged view performs no new work");
  Check(engine.Stats().allocationHits == 1 && engine.Stats().viewHits == 1, "cache hits are observable");

  // A filter only changes the aggregate; the market denominator and heat scale
  // come from the full snapshot and stay fixed.
  auto search = filter; search.search = "grain";
  auto filtered = engine.Build(search);
  Check(filtered->allocationReused && !filtered->viewReused, "filter change reuses allocation only");
  Check(filtered->ribbons.size() == 1, "filter selects the matching good");
  Check(filtered->view->marketFreight == first->view->marketFreight, "filter never changes the market denominator");
  Check(filtered->view->heatReference == first->view->heatReference, "filter never rescales market heat");
  uint8_t fullBand = 0, filteredBand = 0;
  Check(HasEdge(*first->view, 0, &fullBand) && HasEdge(*filtered->view, 0, &filteredBand) &&
        fullBand == filteredBand, "visible edge keeps its heat band while filtered");
  auto hover = filter; hover.hoverGood = 2;
  auto hovered = engine.Build(hover);
  Check(hovered->allocationReused, "hover never re-runs the allocation");
  Check(hovered->view->marketFreight == first->view->marketFreight, "hover keeps the market total");

  // Calibration is part of the native-geometry identity, not of the allocation.
  auto moved = engine.Build(filter, 1, 0.5f, 2);
  Check(moved->allocationReused && !moved->viewReused, "calibration rebuilds geometry without allocation");
  Check(std::abs(moved->ribbons[0]->points[0].y - (first->ribbons[0]->points[0].y + 2)) < 1e-5,
        "calibration offset reaches the native controls");

  // Ordering is not identity: a shuffled adapter result keeps the caches warm.
  auto shuffled = snapshot;
  std::reverse(shuffled.orders.begin(), shuffled.orders.end());
  std::reverse(shuffled.goods.begin(), shuffled.goods.end());
  Check(engine.SetSnapshot(shuffled), "shuffled snapshot accepted");
  auto afterShuffle = engine.Build(filter);
  Check(afterShuffle->allocationReused && afterShuffle->viewReused, "iteration order does not invalidate caches");
  Check(afterShuffle->ribbons.size() == first->ribbons.size() &&
        SameRibbon(*afterShuffle->ribbons[0], *first->ribbons[0]), "reordered input keeps identical geometry");

  // Stale adapter results cannot replace newer data.
  auto oldRevision = snapshot; oldRevision.revision = 0;
  Check(!engine.SetSnapshot(oldRevision, &reason) && reason == "stale economy revision",
        "older economy revision rejected");
  auto oldWorld = snapshot; oldWorld.world = 0;
  Check(!engine.SetSnapshot(oldWorld, &reason) && reason == "stale world generation",
        "older world generation rejected");

  // A genuinely newer revision replaces both caches.
  auto next = snapshot; next.revision = 2; next.orders[0].production = 4000;
  Check(engine.SetSnapshot(next), "newer revision accepted");
  auto rebuilt = engine.Build(filter);
  Check(!rebuilt->allocationReused && !rebuilt->viewReused, "new revision invalidates allocation and view");
  Check(rebuilt->economyRevision == 2, "frame reports the new revision");

  // Same revision with different numbers is still a different snapshot.
  auto changed = next; changed.orders[0].production = 5000;
  Check(engine.SetSnapshot(changed), "changed content under the same revision accepted");
  auto changedFrame = engine.Build(filter);
  Check(!changedFrame->allocationReused, "changed content cannot hit the old allocation");
  Check(engine.SnapshotFingerprint() != 0, "fingerprint is available for diagnostics");

  auto coefficientChange=changed;
  coefficientChange.goods[0].freightPerOrder=2;
  Check(engine.SetSnapshot(coefficientChange), "Freight coefficient change accepted");
  auto coefficientFrame=engine.Build(filter);
  Check(!coefficientFrame->allocationReused && !coefficientFrame->viewReused,
        "Freight coefficient participates in both cache identities");
  Check(coefficientFrame->view->intermediaryDemand && coefficientFrame->view->intermediaryDemand->target==11,
        "updated Freight coefficient reaches cached view");
  Check(coefficientFrame->view->allocatedImport==changedFrame->view->allocatedImport,
        "Freight conversion does not change original allocated orders");
  Check(coefficientFrame->ribbons.size()==changedFrame->ribbons.size(),
        "Freight conversion does not create extra routes");
  for (size_t i=0; i<coefficientFrame->ribbons.size(); ++i)
    Check(SameRibbon(*coefficientFrame->ribbons[i],*changedFrame->ribbons[i]),
          "Freight conversion leaves allocated width and native geometry identity unchanged");
  coefficientChange.goods[0].freightIntermediary=true;
  Check(engine.SetSnapshot(coefficientChange), "intermediary classification change accepted");
  auto intermediaryFrame=engine.Build(filter);
  Check(!intermediaryFrame->allocationReused && intermediaryFrame->view->intermediaryDemand->target==5,
        "intermediary classification invalidates caches and excludes recursive demand");

  // World reset: no snapshot, advanced epoch, old frames cannot be applied.
  auto nontradeable=changed;
  nontradeable.goods[0].tradeable=false;
  Check(engine.SetSnapshot(nontradeable), "nontradeable metadata accepted without revision change");
  auto monetaryFrame=engine.Build(filter);
  Check(!monetaryFrame->allocationReused && !monetaryFrame->viewReused,
        "tradeability participates in allocation and view identities");
  for (const auto& shipment:monetaryFrame->allocation->shipments)
    Check(shipment.good!=nontradeable.goods[0].id,"cached former tradeable shipments cannot survive classification change");

  FreightEngine resetEngine(Geo());
  Check(resetEngine.SetSnapshot(Snapshot()), "reset engine accepts snapshot");
  auto beforeReset = resetEngine.Build(filter);
  resetEngine.ResetWorld();
  const uint64_t epoch = resetEngine.Epoch();
  Check(beforeReset->epoch != epoch, "reset advances the epoch");
  Check(resetEngine.Build(filter)->error == "no economy snapshot", "reset drops the economy snapshot");
  Check(!resetEngine.SetSnapshot(Snapshot(), &reason) && reason=="stale world generation",
        "reset retires the previous world even while no snapshot exists");
  auto retiredLater=Snapshot(); retiredLater.revision=999;
  Check(!resetEngine.SetSnapshot(retiredLater, &reason), "a higher revision cannot revive a retired world");
  auto nextWorld=Snapshot();nextWorld.world=2;
  Check(resetEngine.SetSnapshot(nextWorld), "post-reset newer world accepted");
  auto afterReset = resetEngine.Build(filter);
  Check(!afterReset->allocationReused && afterReset->epoch == epoch, "post-reset caches are cold");

  // Bounded caches: many filters cannot grow the view cache without limit.
  FreightEngine bounded(Geo());
  Check(bounded.SetSnapshot(Snapshot()), "bounded engine snapshot");
  for (int i = 0; i < 40; ++i) {
    FreightFilter many; many.market = 7; many.search = "g" + std::to_string(i);
    bounded.Build(many);
  }
  Check(bounded.ViewCacheSize() <= FreightEngine::kViewCapacity, "view cache stays bounded");
  FreightFilter recent; recent.market = 7; recent.search = "g39";
  auto hit = bounded.Build(recent);
  Check(hit->viewReused, "most recent view survives eviction");
  FreightFilter evicted; evicted.market = 7; evicted.search = "g0";
  Check(!bounded.Build(evicted)->viewReused, "oldest view was evicted");

  // Cancellation publishes nothing and never caches a partial result.
  auto cancelled = engine.Build(filter, 1, 0, 0, [] { return true; });
  Check(cancelled->error == "cancelled" && !cancelled->allocation && !cancelled->view,
        "cancelled build publishes no state");
  int calls = 0;
  auto midCancel = engine.Build(filter, 1, 0, 0, [&] { return ++calls >= 3; });
  Check(midCancel->error == "cancelled", "cancellation during allocation is reported");

  // An invalid graph is a capability error, never a silent empty view.
  auto broken = std::make_shared<RouteGeography>();
  FreightEngine noGraph(broken);
  Check(noGraph.SetSnapshot(Snapshot()), "no-graph snapshot accepted");
  Check(noGraph.Build(filter)->error == "transport graph unavailable", "missing graph reported");

  // Worker: allocation/view assembly happens off the calling thread and the
  // snapshot identity travels with the reply.
  FreightWorker worker(Geo());
  FreightJob job;
  job.snapshot = std::make_shared<const EconomySnapshot>(Snapshot());
  job.filter = filter;
  job.owner = 11;
  const auto latest = worker.Request(job);
  auto reply = worker.Poll();
  const auto deadline = GetTickCount64() + 5000;
  while ((!reply || reply->generation != latest) && GetTickCount64() < deadline) {
    Sleep(1);
    reply = worker.Poll();
  }
  Check(reply && reply->generation == latest, "freight worker completes the request");
  Check(reply->value.error.empty() && reply->value.frame && reply->value.frame->ribbons.size() == 2,
        "worker returns the assembled frame");
  Check(reply->value.owner == 11, "reply retains the requesting owner");
  const auto firstStats = reply->value.stats;
  Check(firstStats.allocations == 1 && firstStats.views == 1, "worker reported one allocation and view");
  FreightJob filterJob;
  filterJob.updateSnapshot = false;
  filterJob.filter = search;
  filterJob.owner = 12;
  const auto second = worker.Request(std::move(filterJob));
  reply = worker.Poll();
  const auto secondDeadline = GetTickCount64() + 5000;
  while ((!reply || reply->generation != second) && GetTickCount64() < secondDeadline) {
    Sleep(1);
    reply = worker.Poll();
  }
  Check(reply && reply->generation == second && reply->value.owner == 12, "worker handles filter-only job");
  Check(reply->value.stats.allocations == 1 && reply->value.stats.allocationHits == 1 &&
        reply->value.stats.views == 2, "filter-only job reuses the allocation");
  auto refreshed=Snapshot();++refreshed.revision;
  FreightJob refreshJob;refreshJob.snapshot=std::make_shared<const EconomySnapshot>(refreshed);
  refreshJob.filter=filter;refreshJob.owner=11;
  worker.Request(refreshJob);
  FreightJob coalesced;coalesced.updateSnapshot=false;coalesced.filter=filter;coalesced.owner=12;
  const auto refreshGeneration=worker.Request(coalesced);
  const auto refreshDeadline=GetTickCount64()+5000;
  do {reply=worker.Poll();if(reply && reply->generation==refreshGeneration)break;Sleep(1);}
  while(GetTickCount64()<refreshDeadline);
  Check(reply && reply->generation==refreshGeneration && reply->value.frame &&
        reply->value.frame->economyRevision==refreshed.revision && reply->value.owner==12,
        "filter coalescing must retain the newest pending snapshot");
  worker.Cancel();
  Check(!worker.Poll(), "cancel clears the completed reply");

  FreightJob staleJob;
  auto stale = Snapshot(); stale.revision = 0;
  staleJob.snapshot = std::make_shared<const EconomySnapshot>(stale);
  staleJob.filter = filter;
  const auto staleRequest = worker.Request(std::move(staleJob));
  reply = worker.Poll();
  const auto staleDeadline = GetTickCount64() + 5000;
  while ((!reply || reply->generation != staleRequest) && GetTickCount64() < staleDeadline) {
    Sleep(1);
    reply = worker.Poll();
  }
  Check(reply && reply->value.error == "stale economy revision" && !reply->value.frame,
        "worker rejects a stale snapshot after newer data");

  const auto epochBeforeReset=uint64_t(0);
  worker.Reset();
  worker.Cancel(); // reset must survive mailbox cancellation/replacement
  FreightJob afterWorldReset;
  auto worldTwo=Snapshot();worldTwo.world=2;
  afterWorldReset.snapshot=std::make_shared<const EconomySnapshot>(worldTwo);
  afterWorldReset.filter=filter;
  const auto resetGeneration=worker.Request(std::move(afterWorldReset));
  const auto resetDeadline=GetTickCount64()+5000;
  do { reply=worker.Poll(); if(reply && reply->generation==resetGeneration) break; Sleep(1); }
  while(GetTickCount64()<resetDeadline);
  Check(reply && reply->value.frame && reply->value.frame->epoch>epochBeforeReset,
        "world reset cannot be dropped by debounce coalescing or Cancel");
  worker.Reset();worker.Cancel();
  FreightJob retiredJob;
  worldTwo.revision=999;
  retiredJob.snapshot=std::make_shared<const EconomySnapshot>(worldTwo);
  retiredJob.filter=filter;
  const auto retiredGeneration=worker.Request(std::move(retiredJob));
  const auto retiredDeadline=GetTickCount64()+5000;
  do { reply=worker.Poll(); if(reply && reply->generation==retiredGeneration) break; Sleep(1); }
  while(GetTickCount64()<retiredDeadline);
  Check(reply && reply->value.error=="stale world generation" && !reply->value.frame,
        "coalesced reset rejects a later revision of the retired world");
  worker.Stop();

  std::puts("PASS: freight allocation/view caches, fixed heat scale, revision/world gating, ordering identity, bounded eviction, cancellation and worker handoff");
}
