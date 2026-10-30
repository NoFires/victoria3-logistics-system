#include "native_world_trade.h"
#include "native_trade_identity.h"
#include <cstdio>
#include <cstdlib>
#include <map>

void Check(bool ok,const char* why){if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
  static constexpr uint64_t module=0x140000000,stateDb=0x10000,stateEntries=0x20000,
    first=0x40000,last=0x50000,third=0x55000,root=0x100000,machine=0x101000,world=0x102000,
    groups=0x110000,members=0x120000,destinations=0x130000,paths=0x140000,ranges=0x150000,
    straitList=0x160000,straitDb=0x170000,straitEntries=0x180000,strait=0x190000,
    definition=0x1a0000,pairs=0x1b0000,values=0x1c0000,areaDb=0x1d0000,
    areaEntries=0x1e0000,firstArea=0x1f0000,lastArea=0x200000,groupLookup=0x210000;
  static constexpr uint32_t lastId=0xab000801,straitId=0x7f000000;
  std::map<uint64_t,uint8_t> bytes;uint64_t mutate=0;int reads=0;bool current=true;
  std::vector<v3::NativeStateEconomy> states{{0,7,0},{lastId,8,1},{3,7,0}};
  std::string error;
  template<class T>void Put(uint64_t a,T v){for(size_t i=0;i<sizeof(v);++i)bytes[a+i]=((uint8_t*)&v)[i];}
  void Zero(uint64_t a,size_t n){for(size_t i=0;i<n;++i)bytes[a+i]=0;}
  void Map(uint64_t a,uint64_t data){
    Zero(a,0x28);Put(a+8,data);Put(a+0x10,uint32_t(1));Put(a+0x14,uint32_t(1));
  }
  Fixture(){
    Put(module+0x05964ef0,stateDb);Put(stateDb+0x20,stateEntries);Put(stateDb+0x2c,uint32_t(2050));
    Zero(stateEntries,2050*16);Put(stateEntries+8,first);Put(stateEntries+2049*16+8,last);Put(stateEntries+3*16+8,third);
    for(auto [a,id]:{std::pair{first,uint32_t(0)},std::pair{last,lastId},std::pair{third,uint32_t(3)}}){
      Put(a,module+0x044de308);Put(a+8,id);Zero(a+0x890,0x50);
      Put(a+0xb48,uint32_t(id==lastId?1:0));
    }
    Put(module+0x05964ea0,areaDb);Put(areaDb+0x20,areaEntries);Put(areaDb+0x2c,uint32_t(2));
    Put(areaEntries+8,firstArea);Put(areaEntries+24,lastArea);
    for(auto [a,id,center]:{std::tuple{firstArea,uint32_t(0),uint32_t(0)},std::tuple{lastArea,uint32_t(1),lastId}}){
      Put(a,module+0x04430cd8);Put(a+8,id);Put(a+0x48,center);
    }
    Put(module+0x059667f8,straitDb);Put(straitDb,module+0x04439720);Put(straitDb+0x20,straitEntries);
    Put(straitDb+0x2c,uint32_t(1));Put(straitDb+0x3c,uint32_t(1));Zero(straitEntries,16);Put(straitEntries+8,strait);
    Zero(strait,0xb0);Put(strait,module+0x04490548);Put(strait+8,straitId);Put(strait+0xa0,definition);Put(strait+0xad,uint8_t(3));
    Zero(definition,0x4c);Put(definition,module+0x044f85a0);Put(definition+0x44,uint16_t(11));Put(definition+0x46,uint16_t(12));
    Put(module+0x0592d128,root);Put(root+0x608,machine);Put(machine+0x120,world);
    Zero(world+0x898,0x18);Put(world+0x898,groups);Put(world+0x8a0,uint32_t(2));Put(world+0x8a4,uint32_t(2));
    Zero(groups,2*0x48);Put(groups,uint32_t(0));Put(groups+8,members);Put(groups+0x10,uint32_t(2));Put(groups+0x14,uint32_t(2));
    Put(members,uint32_t(0));Put(members+4,uint32_t(3));Put(groups+0x48,lastId);Put(groups+0x50,members+0x100);
    Put(groups+0x58,uint32_t(1));Put(groups+0x5c,uint32_t(1));Put(members+0x100,lastId);
    Map(world+0x800,groupLookup);Put(world+0x810,uint32_t(2));Zero(groupLookup,2*0x18);
    Put(groupLookup+4,uint8_t(1));Put(groupLookup+8,uint32_t(0));Put(groupLookup+0x10,groups);
    Put(groupLookup+0x18+4,uint8_t(1));Put(groupLookup+0x18+8,lastId);Put(groupLookup+0x18+0x10,groups+0x48);
    Map(groups+0x20,destinations);Zero(destinations,2*0x28);Put(destinations+4,uint8_t(1));Put(destinations+8,lastId);
    Put(destinations+0x10,paths);Put(destinations+0x18,uint32_t(2));Put(destinations+0x1c,uint32_t(2));
    // Second group has a known empty candidate list, distinct from bad memory.
    Map(groups+0x68,destinations+0x100);Zero(destinations+0x100,2*0x28);Put(destinations+0x104,uint8_t(1));Put(destinations+0x108,uint32_t(0));
    Zero(paths,2*0x38);
    for(unsigned i=0;i<2;++i){
      auto a=paths+i*0x38;Put(a,ranges+i*4);Put(a+8,uint32_t(1));Put(a+12,uint32_t(1));
      Put(a+0x30,int64_t(i?123456789:987654321));
    }
    Put(ranges,std::array<uint16_t,2>{10,0});Put(ranges+4,std::array<uint16_t,2>{11,1});
    Put(paths+0x38+0x18,straitList);Put(paths+0x38+0x20,uint32_t(1));Put(paths+0x38+0x24,uint32_t(1));Put(straitList,straitId);
    Map(first+0x890,pairs);Zero(pairs,2*0x60);Put(pairs+4,uint8_t(1));Put(pairs+8,lastId);
    Put(pairs+0x10,module+0x044283b0);Put(pairs+0x18,values);Put(pairs+0x20,uint32_t(2));Put(pairs+0x24,uint32_t(2));
    Put(pairs+0x30,uint64_t(1));Put(values,int64_t(100001));Put(values+8,int64_t(-999));
    Map(first+0x8b8,pairs); // Same fixture bytes, independent original tables.
  }
  bool Read(uint64_t a,void* out,size_t n){
    for(size_t i=0;i<n;++i){auto f=bytes.find(a+i);if(f==bytes.end())return false;((uint8_t*)out)[i]=f->second;}
    if(a==mutate && ++reads==1)bytes[a]^=1;return true;
  }
  auto Decode(uint32_t goodCount=2){return v3::DecodeNativeWorldTrade([&](auto a,auto p,auto n){return Read(a,p,n);},
      module,states,goodCount,[&]{return current;},{},&error);}
};
int main(){
  Fixture f;const auto original=f.bytes;auto result=f.Decode();
  if(!result)std::printf("error: %s\n",f.error.c_str());
  Check(result && result->groups.size()==2 && result->statePairs.size()==1 && result->states.size()==3 &&
      result->states[0].id==0 && result->states[1].id==Fixture::lastId && result->states[1].market==1,
      "full native groups and independent state/region/market identities copied");
  f.Put(Fixture::groups+0x14,uint32_t(1));f.Put(Fixture::members,uint32_t(3));
  auto represented=f.Decode();
  Check(represented && represented->groups[0].anchor==0 && represented->groups[0].states==std::vector<uint32_t>{3},
      "original trade representative may differ from every group member (live groups893/868/602/361/356)");
  f.bytes=original;
  const auto& dest=result->groups[0].destinations[0];
  Check(dest.state==Fixture::lastId && dest.candidates.size()==2 && dest.candidates[0].distanceFixed==987654321 &&
      dest.candidates[1].distanceFixed==123456789 && dest.candidates[1].straits==std::vector<uint32_t>{Fixture::straitId},
      "preserve both candidates and full strait handles; do not silently pick first");
  Check(!dest.selected,"multiple candidates remain unknown without the verified native selector");
  Check(result->straits[0].id==Fixture::straitId && result->straits[0].definition==0,
      "runtime strait generation handle is separate from its native definition ID");
  for(unsigned i=0;i<4;++i){
    f.Put(Fixture::strait+0xa8+i,uint8_t(1));auto changed=f.Decode();
    Check(changed && changed->straits[0].accessControls[i]==1 && !v3::NativeTradeEquivalent(*result,*changed),
        "same-handle strait control changes invalidate cached native facts");
    f.bytes=original;
  }
  f.Put(Fixture::definition+0x4b,uint8_t(1));auto canalChanged=f.Decode();
  Check(canalChanged && canalChanged->straits[0].canalDefinition==1 && !v3::NativeTradeEquivalent(*result,*canalChanged),
      "same-definition canal marker changes invalidate cached native facts");
  f.bytes=original;
  bool changedDuringRead=false;
  Check(!v3::DecodeNativeWorldTrade([&](auto a,auto p,auto n){
      const bool ok=f.Read(a,p,n);
      if(ok && a==Fixture::strait && !changedDuringRead){changedDuringRead=true;f.Put(Fixture::strait+0xa8,uint8_t(1));}
      return ok;},Fixture::module,f.states,2,[]{return true;}),
      "control mutation during native snapshot refuses publication");
  f.bytes=original;
  int selections=0;
  auto selected=v3::DecodeNativeWorldTrade([&](auto a,auto p,auto n){return f.Read(a,p,n);},Fixture::module,f.states,2,
      []{return true;},[&](uint64_t vector){++selections;Check(vector==Fixture::destinations+0x10,"native selector receives original live vector");
        return Fixture::paths+0x38;});
  Check(selected && selected->groups[0].destinations[0].selected==1 && selections==1,
      "verified selector chooses second candidate and only an owned index escapes");
  f.Put(Fixture::straitList,uint32_t(0));
  Check(!v3::DecodeNativeWorldTrade([&](auto a,auto p,auto n){return f.Read(a,p,n);},Fixture::module,f.states,2,
      []{return true;},[](uint64_t){Check(false,"unavailable canal cannot reach native selector");return uint64_t(0);}),
      "unavailable strait rejected before native invocation");
  f.bytes=original;
  Check(!v3::DecodeNativeWorldTrade([&](auto a,auto p,auto n){return f.Read(a,p,n);},Fixture::module,f.states,2,
      []{return true;},[&](uint64_t){f.Put(Fixture::paths+0x30,int64_t(1));return Fixture::paths+0x38;}),
      "input mutation during selector rejects complete owned copy");
  f.bytes=original;
  for(auto bad:{uint64_t(0),Fixture::paths+1,Fixture::paths+2*0x38})
    Check(!v3::DecodeNativeWorldTrade([&](auto a,auto p,auto n){return f.Read(a,p,n);},Fixture::module,f.states,2,
        []{return true;},[&](uint64_t){return bad;}),"foreign, interior or stale selector pointer rejected");
  f.Put(Fixture::destinations+0x1c,uint32_t(1));
  auto sole=f.Decode();Check(sole && sole->groups[0].destinations[0].selected==0,
      "single candidate follows the original direct return without a native call");
  f.bytes=original;
  int callbackChecks=0;
  Check(!v3::DecodeNativeWorldTrade([&](auto a,auto p,auto n){return f.Read(a,p,n);},Fixture::module,f.states,2,
      [&]{return ++callbackChecks==1;},[&](uint64_t){Check(false,"retired source cannot invoke native selector");return uint64_t(0);}),
      "retirement before native selection rejected");
  Check(dest.candidates[1].provinces[0].first==11 && dest.candidates[1].provinces[0].last==12 &&
      result->groups[1].destinations[0].candidates.empty(),"inclusive ranges and known empty alternatives preserved");
  Check(result->statePairs[0].source==0 && result->statePairs[0].destination==Fixture::lastId &&
      result->statePairs[0].goods.size()==1 && result->statePairs[0].goods[0].valueFixed==100001,
      "table owner exports to the full primary peer handle; no common-subobject ID assumption");
  Check(result->statePairs[0].goods[0].valueFixed/v3::kNativeTradeQuantityScale==1 &&
      result->statePairs[0].goods[0].valueFixed%v3::kNativeTradeQuantityScale==1,
      "1.00001 native quantity never truncated through milli-orders");
  Check(result->groupPairs.size()==1 && result->groupPairs[0].source==0 &&
      result->groupPairs[0].destination==Fixture::lastId && result->groupPairs[0].goods[0].valueFixed==100001,
      "original grouped export table copied independently, not as reverse imports");
  Check(result->centers.at(0)==0 && result->centers.at(3)==0 && result->centers.at(Fixture::lastId)==Fixture::lastId,
      "current area center handles copied independently of cached membership");
  f.Map(Fixture::last+0x890,Fixture::pairs+0x200);f.Zero(Fixture::pairs+0x200,2*0x60);
  f.Put(Fixture::pairs+0x204,uint8_t(1));f.Put(Fixture::pairs+0x208,uint32_t(0));
  f.Put(Fixture::pairs+0x210,Fixture::module+0x044283b0);f.Put(Fixture::pairs+0x218,Fixture::values+0x100);
  f.Put(Fixture::pairs+0x220,uint32_t(2));f.Put(Fixture::pairs+0x224,uint32_t(2));f.Put(Fixture::pairs+0x230,uint64_t(2));
  f.Put(Fixture::values+0x100,int64_t(-333));f.Put(Fixture::values+0x108,int64_t(200003));
  auto reverse=f.Decode();
  Check(reverse && reverse->statePairs.size()==2 && reverse->statePairs[0].source==0 &&
      reverse->statePairs[1].source==Fixture::lastId && reverse->statePairs[1].destination==0 &&
      reverse->statePairs[1].goods[0].good==1 && reverse->statePairs[1].goods[0].valueFixed==200003,
      "opposite directed exports retain independent goods/precision rather than netting or double counting");
  f.bytes=original;
  auto test=[&](auto change,const char* why){f.bytes=original;f.states={{0,7,0},{Fixture::lastId,8,1},{3,7,0}};
    f.current=true;f.mutate=0;f.reads=0;change(f);Check(!f.Decode(),why);};
  test([](auto& m){m.current=false;},"retired before copying rejected");
  test([](auto& m){m.states.push_back(m.states[0]);},"duplicate source state rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::last+8,2049);},"recycled full state handle checked");
  test([](auto& m){m.template Put<uint32_t>(Fixture::lastArea+8,0x1000001);},"recycled full area handle checked");
  test([](auto& m){m.template Put<uint32_t>(Fixture::firstArea+0x48,999);},"unknown current trade center rejected");
  test([](auto& m){m.template Put<uint64_t>(Fixture::firstArea,1);},"unknown area type rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::areaDb+0x2c,1);},"area slot bounds checked");
  test([](auto& m){m.template Put<uint64_t>(Fixture::groupLookup+0x10,Fixture::groups+1);},"interior group lookup pointer rejected");
  test([](auto& m){m.template Put<uint64_t>(Fixture::groupLookup+0x10,Fixture::groups+0x48);},"group lookup key/pointer mismatch rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::world+0x810,0);},"missing current group lookup cannot use cached vector alone");
  test([](auto& m){m.template Put<uint64_t>(Fixture::straitDb,1);},"unknown strait database rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::straitDb+0x3c,0);},"strait active count mismatch rejected");
  test([](auto& m){m.template Put<uint64_t>(Fixture::definition,1);},"unknown strait definition rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::definition+0x10,UINT32_MAX);},"unavailable strait definition ID rejected");
  test([](auto& m){m.template Put<uint8_t>(Fixture::strait+0xad,4);},"unknown tariff enum rejected");
  test([](auto& m){m.template Put<uint16_t>(Fixture::definition+0x44,UINT16_MAX);},"sea province sentinel rejected");
  test([](auto& m){m.template Put<uint64_t>(Fixture::machine+0x120,0);},"null world rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::world+0x8a0,1);},"group count above capacity rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::groups+0x14,3);},"member count above capacity rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::members+4,999);},"unknown member rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::members+4,Fixture::lastId);},"one state cannot belong to two groups");
  test([](auto& m){m.template Put<uint32_t>(Fixture::groups+0x48,0);},"duplicate representative rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::destinations+8,3);},"non-representative destination rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::groups+0x34,2);},"non power-of-two bucket mask rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::groups+0x30,2);},"bucket count mismatch rejected");
  test([](auto& m){m.template Put<uint64_t>(Fixture::groups+0x28,UINT64_MAX-8);},"bucket pointer overflow rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::destinations+0x1c,3);},"candidate count above capacity rejected");
  test([](auto& m){m.template Put<int64_t>(Fixture::paths+0x30,-1);},"negative path distance rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::paths+12,2);},"province set bounds rejected");
  test([](auto& m){m.template Put<std::array<uint16_t,2>>(Fixture::ranges,{65534,1});},"province set overflow rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::straitList,0);},"stale generation cannot impersonate current strait");
  test([](auto& m){m.template Put<uint32_t>(Fixture::pairs+8,2049);},"full peer handle checked");
  test([](auto& m){m.template Put<uint64_t>(Fixture::first+0x8c0,0);},"unreadable grouped table rejects complete snapshot");
  test([](auto& m){m.template Put<uint64_t>(Fixture::pairs+0x10,1);},"unknown goods counter type rejected");
  test([](auto& m){m.template Put<uint64_t>(Fixture::pairs+0x38,1);},"goods mask outside catalog rejected");
  test([](auto& m){m.template Put<int64_t>(Fixture::values,-1);},"negative populated trade value rejected");
  test([](auto& m){m.bytes.erase(Fixture::ranges);},"unreadable path rejected");
  for(auto a:{Fixture::module+0x0592d128,Fixture::groups,Fixture::members,Fixture::paths,Fixture::ranges,Fixture::straitList,
      Fixture::pairs,Fixture::values,Fixture::strait,Fixture::definition,Fixture::first+0xb48,Fixture::firstArea+0x48,Fixture::groupLookup})
    test([&](auto& m){m.mutate=a;},"observed mutation in any component rejects whole candidate");
  f.bytes=original;f.current=true;f.mutate=0;f.reads=0;int calls=0;
  Check(!v3::DecodeNativeWorldTrade([&](auto a,auto p,auto n){return f.Read(a,p,n);},Fixture::module,
      f.states,2,[&]{return ++calls==1;}),"retirement at publication rejected");
  f.bytes.clear();Check(dest.candidates[1].straits[0]==Fixture::straitId && result->statePairs[0].goods[0].valueFixed==100001 &&
      result->states[1].id==Fixture::lastId,"states, paths, straits and quantities survive source destruction");
  std::puts("PASS: native world-market candidates, full state/strait handles, exact trade values, bounds and combined mutation/retirement");
}
