#include "native_sea_geography.h"
#include <cstdio>
#include <cstdlib>
#include <tuple>

void Check(bool ok,const char* why){if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
  static constexpr uint64_t module=0x140000000,tlsArray=0x10000,tls=0x20000,database=0x30000,
      definitions=0x40000,def0=0x50000,def1=0x50100,map=0x60000,roster=0x70000,
      province=0x80000,region=0x90000,adjacency=0xa0000,stateDb=0xb0000,statesTable=0xc0000,
      state0=0xd0000,state1=0xd1000;
  static constexpr uint32_t secondState=0xab000801,naturalStrait=0x7f000000,canalStrait=0x55000001;
  std::map<uint64_t,uint8_t> bytes;uint64_t mutate=0;int reads=0;bool current=true;
  std::vector<v3::NativeStateEconomy> states{{0,7,0},{secondState,8,1}};
  v3::NativeWorldTradeSnapshot trade;std::string error;
  template<class T>void Put(uint64_t at,T value){for(size_t i=0;i<sizeof(value);++i)bytes[at+i]=((uint8_t*)&value)[i];}
  void Zero(uint64_t at,size_t size){for(size_t i=0;i<size;++i)bytes[at+i]=0;}
  static uint64_t Key(unsigned a,unsigned b){return v3::NativeSeaSplineKeys(10+a,10+b)[0];}
  Fixture(){
    Put(tlsArray,tls);Put(tls+0x8600,uint32_t(1));Put(tls+0x8648,uint64_t(0xabc000000003));
    Put(module+0x0514d418+3*16,database);Put(module+0x0514d420+3*16,uint8_t(1));
    Put(database,module+0x04424140);Put(database+0x80,definitions);Put(database+0x88,uint32_t(2));Put(database+0x8c,uint32_t(2));
    Put(definitions,def0);Put(definitions+8,def1);
    for(unsigned i=0;i<2;++i){
      auto at=def0+i*0x100;Zero(at,0x48);Put(at,module+0x044f85a0);Put(at+0x10,i);
      Put(at+0x44,uint16_t(i?3:1));Put(at+0x46,uint16_t(i?4:2));
    }
    Put(module+0x058fccf0,map);Put(map,module+0x0441c938);Put(map+0x2a0,roster);
    Put(map+0x2a8,uint32_t(16));Put(map+0x2ac,uint32_t(16));Zero(roster,16*8);
    for(unsigned i=1;i<=4;++i){
      const auto object=province+i*0x200,definition=region+i*0x100;
      Put(roster+i*8,object);Put(object,module+0x044dc488);Put(object+0xc8,definition);
      Zero(definition,0x48);Put(definition,module+0x044e7850);Put(definition+0x10,uint32_t(500+i));
      Put(definition+0x40,uint32_t(10+i));Put(definition+0x44,uint8_t(1));
      const std::array<unsigned,2> peers=i==1?std::array<unsigned,2>{2,3}:i==2?std::array<unsigned,2>{1,4}:
          i==3?std::array<unsigned,2>{1,4}:std::array<unsigned,2>{3,2};
      Zero(object+0x50,0x18);Put(object+0x50,adjacency+i*0x100);Put(object+0x58,uint32_t(2));Put(object+0x5c,uint32_t(2));
      Zero(adjacency+i*0x100,2*0x28);Put(adjacency+i*0x100,uint32_t(peers[0]));Put(adjacency+i*0x100+0x28,uint32_t(peers[1]));
    }
    Put(module+0x05964ef0,stateDb);Put(stateDb+0x20,statesTable);Put(stateDb+0x2c,uint32_t(2050));
    Put(statesTable+8,state0);Put(statesTable+2049*16+8,state1);
    for(auto [at,id,definition]:{std::tuple{state0,uint32_t(0),region+0x100},std::tuple{state1,secondState,region+0x200}}){
      Put(at,module+0x044de308);Put(at+8,id);Put(at+0xe78,definition);
    }
    trade.straits.push_back({naturalStrait,{3,4},{0,0},1}); // runtime handle != definition ID
    v3::NativeTradePath detour;detour.provinces={{1,4}};detour.straits={naturalStrait};detour.distanceFixed=30000000;
    v3::NativeTradeDestination destination;destination.state=secondState;destination.candidates={detour};destination.selected=0;
    v3::NativeTradeGroup group;group.anchor=0;group.states={0};group.destinations={destination};trade.groups={group};
    auto geometry=std::make_shared<v3::NativeSplineNetwork>();geometry->width=8192;geometry->height=3616;
    for(unsigned i=1;i<=4;++i)geometry->points[0x800000+(10+i)*100]={float(i==2||i==4?100:0),float(i>=3?100:0)};
    unsigned sequence=1;
    for(auto [a,b]:{std::pair{1u,2u},std::pair{1u,3u},std::pair{3u,4u},std::pair{2u,4u}}){
      const uint64_t id=(uint64_t(sequence++)<<32)|2;
      geometry->strips[id]={0x800000+(10+a)*100,0x800000+(10+b)*100};geometry->connections[Key(a,b)]={id};
    }
    trade.geometry=std::move(geometry);
  }
  bool Read(uint64_t at,void* out,size_t size){
    for(size_t i=0;i<size;++i){auto f=bytes.find(at+i);if(f==bytes.end())return false;((uint8_t*)out)[i]=f->second;}
    if(at==mutate && ++reads==1)bytes[at]^=1;return true;
  }
  auto Decode(){return v3::DecodeNativeSeaGeography([&](auto a,auto p,auto n){return Read(a,p,n);},
      module,tlsArray,states,trade,[&]{return current;},&error);}
};
int main(){
  // Independent calibration from actual installed .splnet connection records,
  // not a fixture built with the key helper under test.
  Check(v3::NativeSeaSplineKeys(3021,3023)==std::array<uint64_t,2>{0x424e6e021270502,0x424e6e021270502} &&
      v3::NativeSeaSplineKeys(3023,3021)==std::array<uint64_t,2>{0x424e6e021270502,0x424e6e021270502} &&
      v3::NativeSeaSplineKeys(3006,3024)[0]==0x424ea0021258e02 &&
      v3::NativeSeaSplineKeys(3024,3026)[0]==0x424f04021275002,
      "original key packing matches independent native authored connection keys in both directions");
  Fixture f;const auto original=f.bytes;const auto originalTrade=f.trade;auto copy=f.Decode();
  if(!copy)std::printf("error: %s\n",f.error.c_str());
  Check(copy && copy->definitions.size()==2 && copy->provinces.size()==4 && copy->states.size()==2,
      "all definitions, including an unbuilt canal, and current referenced geography copied");
  Check(copy->definitions[0].seaProvinces==std::array<uint16_t,2>{1,2} &&
      copy->states.at(0).definition==501 && copy->states.at(Fixture::secondState).geo==12 &&
      copy->provinces.at(3).region.definition==503 && copy->provinces.at(3).region.geo==13,
      "runtime full state handles, definition IDs, province IDs and geo IDs remain separate");
  auto destination=f.trade.groups[0].destinations[0];
  auto closed=v3::SelectNativeSeaConnections(f.trade,*copy,destination,&f.error);
  Check(closed && closed->keys.size()==3 && closed->blockedStraitAdjacencies==2 &&
      std::find(closed->keys.begin(),closed->keys.end(),Fixture::Key(1,2))==closed->keys.end(),
      "unbuilt canal shortcut excluded even though both provinces, adjacency and native spline exist");
  auto nativeGraph=v3::BuildNativeSplineGraph(*f.trade.geometry);
  Check(nativeGraph && nativeGraph->nativeConnections.count(Fixture::Key(1,2)),"negative condition includes real shortcut geometry");
  auto blind=*copy;blind.definitions.clear();auto noRules=f.trade;noRules.straits.clear();auto noPassages=destination;
  noPassages.candidates[0].straits.clear();auto unsafe=v3::SelectNativeSeaConnections(noRules,blind,noPassages);
  Check(unsafe && unsafe->keys.size()==4,"negative control reproduces blind induced-adjacency shortcut");
  auto opened=f.trade;opened.straits.push_back({Fixture::canalStrait,{1,2},{0,0},0});
  auto stillDetour=v3::SelectNativeSeaConnections(opened,*copy,destination);
  Check(stillDetour && stillDetour->keys==closed->keys,"existing runtime canal alone cannot override the original selected route passages");
  auto directPath=destination.candidates[0];directPath.provinces={{1,2}};directPath.straits={Fixture::canalStrait};
  directPath.distanceFixed=10000000;destination.candidates.push_back(std::move(directPath));
  destination.selected=1;auto direct=v3::SelectNativeSeaConnections(opened,*copy,destination);
  Check(direct && direct->keys==std::vector<uint64_t>{Fixture::Key(1,2)},"native second candidate uses the newly opened passage without the detour");
  Check(!v3::SelectNativeSeaConnections(f.trade,*copy,destination),"closed canal with stale selected passage is unavailable, never model fallback");
  opened.straits.back().id=0x22000001;
  Check(!v3::SelectNativeSeaConnections(opened,*copy,destination),"recycled strait slot cannot impersonate the selected full handle");
  opened.straits.back().id=Fixture::canalStrait;
  destination.selected.reset();Check(!v3::SelectNativeSeaConnections(opened,*copy,destination),"multiple candidates without native selection remain unknown");
  destination.selected=99;Check(!v3::SelectNativeSeaConnections(f.trade,*copy,destination),"out-of-bounds native selection rejected");
  destination=f.trade.groups[0].destinations[0];auto missing=*copy;missing.provinces.erase(1);
  Check(!v3::SelectNativeSeaConnections(f.trade,missing,destination),"missing current province never falls back to vanilla geography");
  // Same province/definition, newly loaded geo anchor and authored geometry.
  f.Put(Fixture::region+0x100+0x40,uint32_t(101));auto remapped=f.Decode();
  auto remappedTrade=f.trade;auto geometry=std::make_shared<v3::NativeSplineNetwork>(*f.trade.geometry);
  const uint32_t oldAnchor=0x800000+11*100,newAnchor=0x800000+101*100;
  geometry->points[newAnchor]=geometry->points.at(oldAnchor);geometry->points.erase(oldAnchor);
  for(auto& [id,refs]:geometry->strips)for(auto& ref:refs)if(ref==oldAnchor)ref=newAnchor;
  for(unsigned peer:{2u,3u}){
    auto refs=geometry->connections.at(Fixture::Key(1,peer));geometry->connections.erase(Fixture::Key(1,peer));
    geometry->connections[v3::NativeSeaSplineKeys(101,10+peer)[0]]=std::move(refs);
  }
  remappedTrade.geometry=geometry;auto remappedGraph=v3::BuildNativeSplineGraph(*geometry);
  auto remappedLines=remapped?v3::SelectNativeSeaConnections(remappedTrade,*remapped,destination):std::nullopt;
  Check(remappedGraph && remappedLines && remappedLines->keys.size()==3 &&
      std::find(remappedLines->keys.begin(),remappedLines->keys.end(),v3::NativeSeaSplineKeys(101,13)[0])!=remappedLines->keys.end() &&
      std::find(remappedLines->keys.begin(),remappedLines->keys.end(),Fixture::Key(1,3))==remappedLines->keys.end(),
      "same province/definition with changed current geo anchors uses the new native geometry and keeps the canal blocked");
  f.bytes=original;
  auto test=[&](auto change,const char* why){f.bytes=original;f.trade=originalTrade;f.states={{0,7,0},{Fixture::secondState,8,1}};
    f.current=true;f.mutate=0;f.reads=0;change(f);Check(!f.Decode(),why);};
  test([](auto& m){m.current=false;},"retired request rejected before copying");
  test([](auto& m){m.template Put<uint32_t>(Fixture::tls+0x8600,0);},"uninitialized TLS cannot be initialized by decoder");
  test([](auto& m){m.template Put<uint64_t>(Fixture::tls+0x8648,16);},"unknown current context rejected");
  test([](auto& m){m.template Put<uint8_t>(Fixture::module+0x0514d420+3*16,0);
    m.template Put<uint64_t>(Fixture::module+0x0514d418,Fixture::database);m.template Put<uint8_t>(Fixture::module+0x0514d420,1);},
      "populated other context is not a fallback for inactive current context");
  test([](auto& m){m.template Put<uint64_t>(Fixture::database,1);},"wrong definition database rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::database+0x8c,3);},"definition count above capacity rejected");
  test([](auto& m){m.template Put<uint64_t>(Fixture::definitions+8,Fixture::def0);},"duplicate definition pointer rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::def0+0x10,99);},"definition vector identity mismatch rejected");
  test([](auto& m){m.template Put<uint16_t>(Fixture::def0+0x44,UINT16_MAX);},"unbuilt definition province sentinel rejected");
  test([](auto& m){m.trade.straits[0].definition=0;},"runtime and definition identities cannot be conflated");
  test([](auto& m){m.template Put<uint64_t>(Fixture::map,1);},"wrong current map rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::map+0x2a8,15);},"province count above capacity rejected");
  test([](auto& m){m.template Put<uint64_t>(Fixture::roster+8,0);},"missing requested province object rejected");
  test([](auto& m){m.template Put<uint64_t>(Fixture::province+0x200,1);},"wrong province type rejected");
  test([](auto& m){m.template Put<uint8_t>(Fixture::region+0x100+0x44,0);},"unavailable geo identity rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::province+0x200+0x5c,3);},"adjacency count above capacity rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::adjacency+0x100,99);},"unknown adjacent province rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::state1+8,2049);},"recycled state handle rejected");
  test([](auto& m){m.states.push_back(m.states[0]);},"duplicate current state rejected");
  test([](auto& m){m.bytes.erase(Fixture::definitions);},"unreadable definition roster rejected");
  for(auto address:{Fixture::tlsArray,Fixture::tls+0x8648,Fixture::module+0x0514d418+3*16,Fixture::database+0x80,
      Fixture::definitions,Fixture::def0,Fixture::roster,Fixture::region+0x100,Fixture::province+0x200+0x50,
      Fixture::adjacency+0x100,Fixture::state0+0xe78})
    test([&](auto& m){m.mutate=address;},"mutation in any copied identity or geography rejects the combined copy");
  f.bytes=original;f.trade=originalTrade;f.current=true;f.mutate=0;f.reads=0;int checks=0;
  Check(!v3::DecodeNativeSeaGeography([&](auto a,auto p,auto n){return f.Read(a,p,n);},Fixture::module,Fixture::tlsArray,
      f.states,f.trade,[&]{return ++checks==1;}),"retirement before publication rejected");
  f.bytes.clear();Check(copy->definitions[0].seaProvinces[1]==2 && copy->provinces.at(1).adjacent[1]==3,
      "owned definitions and geography survive source destruction");
  std::puts("PASS: current-context strait definitions, unbuilt-canal shortcut rejection, native selected passages, full handles, geography identity, bounds and retirement");
}
