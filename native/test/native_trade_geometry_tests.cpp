#include "native_trade_geometry.h"
#include <cstdio>
#include <cstdlib>

using namespace v3;
void Check(bool ok,const char* why){if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
  static constexpr uint32_t a=0x8493e0,b=0x849444,c=0x8494a8,from=0x800064,to=0x800190;
  static constexpr uint64_t source=0x100000003,dest=0x200000003,shortCut=0x300000002,
      longA=0x400000002,longB=0x500000002,sourceKey=0x04249f0020001903,destKey=0x0424a22020006403;
  NativeWorldTradeSnapshot trade;NativeSeaGeography sea;NativeSplineNetwork network;NativeTradeDestination route;
  Fixture(){
    network.width=8192;network.height=3616;
    // Preserve a curved authored control sequence across the map seam. The
    // geometry ordering must not choose an invented chord or sort by X.
    network.points={{from,{8190,100}},{a,{8191,100}},{b,{2,101}},{c,{1,101}},{to,{3,100}},
        {71,{8190.5f,99}},{72,{2.5f,99}},{73,{8191.5f,98}},{74,{8191.75f,100}},{75,{1.5f,102}}};
    network.strips={{source,{from,71,a}},{dest,{to,72,b}},{shortCut,{a,73,b}},
        {longA,{a,74,c}},{longB,{b,75,c}}};
    // Literal original packing calibrated in R108/R110; do not construct these
    // fixture connections through the selector under test.
    network.connections={{sourceKey,{source}},{destKey,{dest}},
        {0x0424a2202124f802,{shortCut}},{0x0424a5402124f802,{longA}},{0x0424a54021251102,{longB}}};
    sea.definitions={{0,{10,11}}}; // canal exists in definitions, not runtime
    sea.provinces={{10,{{310,3000},{11,12,20}}},{11,{{311,3001},{10,12,20}}},
        {12,{{312,3002},{10,11,20}}}};
    sea.states={{100,{1,1}},{200,{4,4}}};sea.stateHubs={{100,true},{200,true}};
    route.state=200;route.selected=0;route.candidates={{{{10,12}},{},200000},{{{10,11}},{0x7f000001},100000}};
  }
  auto Build(std::string* error=nullptr,const std::function<bool()>& cancelled={}){
    trade.geometry=std::make_shared<const NativeSplineNetwork>(network);
    trade.seaGeography=std::make_shared<const NativeSeaGeography>(sea);
    return BuildNativeTradeGeometry(trade,100,route,error,cancelled);
  }
};
int main(){
  Fixture f;std::string error;auto result=f.Build(&error);
  if(!result)std::printf("error: %s\n",error.c_str());
  Check(result && result->strips==std::vector<NativeDirectedStrip>{{f.source,true},{f.longA,true},{f.longB,false},{f.dest,false}},
      "closed native canal keeps exact selected long chain and orients every original strip from source to destination");
  Check(result->connections.sea.blockedStraitAdjacencies==2 && result->connections.ports[0]==f.sourceKey &&
      result->connections.ports[1]==f.destKey,"native canal passage and endpoint port mapping remain authoritative");
  Check(f.trade.geometry->strips.at(f.source)==std::vector<uint32_t>{f.from,71,f.a} &&
      f.trade.geometry->points.at(74).x==8191.75f,"seam coordinates and curved authored controls are never replaced or mutated");
  auto reverse=f.route;reverse.state=100;
  auto backward=BuildNativeTradeGeometry(f.trade,200,reverse);
  Check(backward && backward->strips==std::vector<NativeDirectedStrip>{{f.dest,true},{f.longB,true},{f.longA,false},{f.source,false}},
      "opposite directed native record reverses both original strip order and control orientation");
  f.trade.straits={{0x7f000001,{10,11},{0,0},0}};
  auto existing=f.Build();Check(existing && existing->strips==result->strips,
      "current canal existence cannot override the original selected long alternative");
  f.route.selected=1;auto opened=f.Build();
  Check(opened && opened->strips==std::vector<NativeDirectedStrip>{{f.source,true},{f.shortCut,true},{f.dest,false}},
      "only original changed selection uses the newly available canal shortcut");
  f.trade.straits.clear();Check(!f.Build(),"closing the canal rejects its old selected full handle without reusing geometry");
  f.trade.straits={{0x80000001,{10,11},{0,0},0}};
  Check(!f.Build(),"a recycled strait slot cannot impersonate the selected full generation");
  f=Fixture();f.network.connections.erase(f.destKey);f.Build();
  auto partial=BuildNativeTradeGeometry(f.trade,100,f.route,nullptr,{},true);
  Check(partial && !partial->complete && partial->strips==std::vector<NativeDirectedStrip>{{f.source,true},{f.longA,true},{f.longB,false}},
      "partial port geometry preserves the witnessed source direction and exact closed-canal detour");
  Check(!BuildNativeTradeGeometry(f.trade,100,f.route),"international default still requires both native port endpoints");
  f.route.selected=1;
  Check(!BuildNativeTradeGeometry(f.trade,100,f.route,nullptr,{},true),"partial mode cannot grant passage through a closed selected canal");
  f=Fixture();f.network.connections.erase(f.destKey);f.sea.definitions.clear();f.Build();
  Check(!BuildNativeTradeGeometry(f.trade,100,f.route,nullptr,{},true),"partial mode cannot select one branch from ambiguous original geometry");
  f=Fixture();f.sea.definitions.clear();
  Check(!f.Build(&error) && error=="native route geometry is not a unique endpoint chain",
      "negative control: raw induced adjacency creates a branch; no arbitrary shortest-path or first-branch choice");
  auto bad=[](auto change,const char* why){Fixture value;change(value);Check(!value.Build(),why);};
  bad([](auto& v){v.route.selected.reset();},"unknown native selection does not default to first");
  bad([](auto& v){v.route.candidates.clear();v.route.selected.reset();},"no native candidate cannot fabricate a port route");
  bad([](auto& v){v.network.connections.at(v.sourceKey).clear();},"known empty native port connection stays unavailable");
  bad([](auto& v){v.sea.stateHubs.at(100)=false;},"ineligible current source cannot gain a nearest coast fallback");
  bad([](auto& v){v.sea.states.erase(100);},"unknown full source state geography rejected");
  bad([](auto& v){v.network.strips.at(v.longA)={v.c,74,v.c};},"disconnected native connection control order rejected");
  bad([](auto& v){v.network.connections.at(v.sourceKey)={v.dest};},"native connection cannot substitute another port's controls");
  bad([](auto& v){v.network.points.erase(74);},"missing authored control stays unavailable");
  bad([](auto& v){v.network.points.at(74).x=std::numeric_limits<float>::quiet_NaN();},"nonfinite controls rejected");
  bad([](auto& v){v.network.width=0;},"missing current map dimensions rejected");
  bad([](auto& v){v.network.points.at(71)=v.network.points.at(v.from);v.network.points.at(v.a)=v.network.points.at(v.from);},
      "zero-length physical strip rejected");
  f=Fixture();Check(!f.Build(nullptr,[]{return true;}),"retired worker request performs no geometry work");
  unsigned calls=0;Check(!f.Build(nullptr,[&]{return ++calls>2;}),"cancellation during geometry assembly rejected");
  f=Fixture();result=f.Build();f.trade={};f.network={};f.sea={};
  Check(result && result->strips.front().id==0x100000003 && result->strips.back().id==0x200000003,
      "owned directed geometry survives source destruction and retains distinct IDs sharing low32 bits");
  std::puts("PASS: native selected chain direction, closed/open/recycled canals, authored controls/seam, branch rejection and cancellation");
}
