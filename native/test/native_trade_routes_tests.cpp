#include "native_trade_routes.h"
#include <cstdio>
#include <cstdlib>

using namespace v3;
void Check(bool ok,const char* why){if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
NativeWorldTradeSnapshot Fixture(){
  NativeWorldTradeSnapshot trade;
  for(uint32_t id:{0u,10u,11u,20u,21u,22u})trade.states.push_back({id,id+100,id+200});
  for(uint32_t id:{0u,10u,11u})trade.centers[id]=0;
  for(uint32_t id:{20u,21u,22u})trade.centers[id]=20;
  NativeTradeDestination route;route.state=20;route.candidates.resize(2);route.selected=1;
  route.candidates[0].provinces={{4,5}};route.candidates[1].provinces={{6,7}};
  trade.groups={{0,{10,11},{route}},{20,{21,22},{}}};
  trade.statePairs={{10,21,{{1,100001}}},{10,22,{{1,300007}}},{11,21,{{2,200003}}},
      {10,11,{{3,900007}}},{21,10,{{1,400009}}}};
  trade.groupPairs={{10,20,{{1,400008}}},{11,20,{{2,200003}}},{10,0,{{3,900007}}},{21,0,{{1,400009}}}};
  return trade;
}
int main(){
  auto trade=Fixture();std::string error;
  auto bindings=BindNativeTradeRoutes(trade,&error);
  Check(bindings && bindings->size()==5,"all positive detailed records bind after exact original grouped-table agreement");
  Check((*bindings)[0].pair==0 && (*bindings)[0].sourceGroup==0 && (*bindings)[0].destinationCenter==20 &&
      (*bindings)[0].route==0 && trade.groups[0].destinations[0].selected==1,
      "current native centers bind the original second selected alternative without selecting or reallocating");
  Check((*bindings)[3].sourceCenter==(*bindings)[3].destinationCenter && !(*bindings)[3].route &&
      !(*bindings)[4].route,"same group and absent reverse route retained without fabricating sea geometry");
  auto empty=trade;empty.groups[0].destinations[0].candidates.clear();empty.groups[0].destinations[0].selected.reset();
  auto knownEmpty=BindNativeTradeRoutes(empty);
  Check(knownEmpty && (*knownEmpty)[0].route==0 && empty.groups[0].destinations[0].candidates.empty(),
      "known empty native destination remains distinguishable from no destination");
  empty=trade;empty.groups[0].destinations[0].selected.reset();
  Check(BindNativeTradeRoutes(empty).has_value() && !empty.groups[0].destinations[0].selected,
      "unselected alternatives remain unselected; quantities do not grant routing permission");
  empty=trade;empty.centers[10]=11;
  empty.groupPairs.back().destination=11; // Reverse exports target that same current center.
  auto staleSource=BindNativeTradeRoutes(empty);
  Check(staleSource && !(*staleSource)[0].sourceGroup && !(*staleSource)[0].route,
      "current center missing from original lookup cannot use its old group membership as fallback");
  empty=trade;empty.centers[21]=22;empty.centers[22]=22;
  for(auto& pair:empty.groupPairs)if(pair.destination==20)pair.destination=22;
  auto staleDestination=BindNativeTradeRoutes(empty);
  Check(staleDestination && (*staleDestination)[0].sourceGroup==0 && !(*staleDestination)[0].route &&
      (*staleDestination)[0].destinationCenter==22,
      "original grouped center outside cached representatives remains valid quantity with unavailable route (live five areas)");
  auto oldMembership=empty;oldMembership.centers[21]=20;oldMembership.centers[22]=20;
  Check(!BindNativeTradeRoutes(oldMembership),"negative control: inferring current centers from old members fails exact original quantities");
  empty=trade;empty.statePairs.push_back({0,20,{}});empty.groupPairs.push_back({0,20,{}});
  Check(BindNativeTradeRoutes(empty)->size()==5,"known empty counters outside explicit participating members contribute nothing");
  auto bad=[&](auto change,const char* why){auto copy=trade;change(copy);Check(!BindNativeTradeRoutes(copy,&error),why);};
  bad([](auto& t){++t.groupPairs[0].goods[0].valueFixed;},"one original 1e-5 discrepancy rejects the association");
  bad([](auto& t){t.groupPairs.clear();},"detailed totals cannot replace absent original grouped evidence");
  bad([](auto& t){t.statePairs.clear();},"unattributed original grouped goods cannot be invented as state shipments");
  bad([](auto& t){t.statePairs[0].source=0;},"changing the detailed source cannot retain another source's grouped evidence");
  empty=trade;empty.statePairs[0].destination=20;
  Check(BindNativeTradeRoutes(empty).has_value(),"actual detailed peer may be a representative when its current area evidence agrees");
  bad([](auto& t){t.groupPairs[0].destination=21;},"grouped peer must agree with current destination center");
  bad([](auto& t){t.centers.erase(21);},"missing current area evidence cannot infer destination center from members");
  bad([](auto& t){t.centers[21]=UINT32_MAX;},"positive exports without current destination center rejected");
  bad([](auto& t){t.statePairs[0].source|=0x1000000;},"full-generation state identity never masked to slot");
  bad([](auto& t){t.groups[1].states.push_back(10);},"ambiguous membership rejected");
  bad([](auto& t){t.states.push_back(t.states[0]);},"duplicate roster rejected");
  bad([](auto& t){t.groups[1].anchor=0;},"duplicate representative rejected");
  bad([](auto& t){t.statePairs.push_back(t.statePairs[0]);},"duplicate detailed pair rejected");
  bad([](auto& t){t.groupPairs.push_back(t.groupPairs[0]);},"duplicate grouped pair rejected");
  bad([](auto& t){t.statePairs[0].goods.push_back(t.statePairs[0].goods[0]);},"duplicate good cannot double quantities");
  bad([](auto& t){t.statePairs[0].goods[0].valueFixed=-1;},"negative quantity rejected");
  bad([](auto& t){t.statePairs[0].goods[0].valueFixed=0;},"zero entries must be normalized by decoder, not treated as shipments");
  bad([](auto& t){t.statePairs[0].goods[0].valueFixed=INT64_MAX;},"exact integer aggregation detects overflow");
  bad([](auto& t){t.groups[0].destinations[0].selected=2;},"out-of-range native selection rejected");
  bad([](auto& t){t.groups[0].destinations.push_back(t.groups[0].destinations[0]);},"duplicate destination rejected");
  std::reverse(trade.statePairs.begin(),trade.statePairs.end());std::reverse(trade.groupPairs.begin(),trade.groupPairs.end());
  Check(BindNativeTradeRoutes(trade).has_value(),"independent native enumeration order does not affect exact agreement");
  Check(trade.statePairs.back().goods[0].valueFixed==100001 && trade.groupPairs.back().goods[0].valueFixed==400008,
      "view association never mutates native quantities or direction");
  std::puts("PASS: native exact grouped/detailed exports, current area centers, stale/selected/missing/empty routes and overflow");
}
