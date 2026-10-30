#include "native_domestic.h"
#include <cstdio>
#include <cstdlib>
using namespace v3;
void Check(bool value,const char* why){if(!value){std::printf("FAIL: %s\n",why);std::exit(1);}}
struct Memory {
  std::map<uint64_t,uint8_t> bytes;bool current=true,mutate=false;unsigned reads=0;
  template<class T>void Put(uint64_t at,T value){for(size_t i=0;i<sizeof(value);++i)bytes[at+i]=((uint8_t*)&value)[i];}
  void Zero(uint64_t at,size_t n){for(size_t i=0;i<n;++i)bytes[at+i]=0;}
  bool Read(uint64_t at,void* out,size_t n){
    for(size_t i=0;i<n;++i){auto p=bytes.find(at+i);if(p==bytes.end())return false;((uint8_t*)out)[i]=p->second;}
    if(at==0x10000000+0x0592d128 && ++reads==1 && mutate)Put<uint64_t>(at,0);
    return true;
  }
};
int main(){
  constexpr uint64_t base=0x10000000,root=0x200000,machine=0x210000,world=0x220000,
      db=0x300000,entries=0x310000,s0=0x320000,s1=0x330000,connection=0x400000,definition=0x410000,
      key=(uint64_t(200)<<35)|(uint64_t(100)<<6),strip=0x100000000;
  constexpr uint32_t first=0xab000001,second=0xcd000002;
  Memory good;
  good.Put<uint64_t>(base+0x05964ef0,db);good.Put<uint64_t>(db+0x20,entries);good.Put<uint32_t>(db+0x2c,3);
  good.Zero(entries,48);good.Put<uint64_t>(entries+24,s0);good.Put<uint64_t>(entries+40,s1);
  for(auto [object,id]:{std::pair{s0,first},std::pair{s1,second}}){
    good.Put<uint64_t>(object,base+0x044de308);good.Put<uint32_t>(object+8,id);
    good.Zero(object+0x78,0x30);good.Zero(object+0xc8,0x30);
  }
  good.Put<uint64_t>(s0+0xd0,0x340000);good.Put<uint32_t>(s0+0xd8,2);good.Put<uint32_t>(s0+0xdc,2);
  good.Put<uint64_t>(s0+0xe8,1);good.Put<int64_t>(0x340000,1234500);good.Put<int64_t>(0x340008,-100); // unmasked ignored
  good.Put<uint64_t>(base+0x05966760,0x350000);good.Put<uint64_t>(0x350000,base+0x04439410);
  good.Put<uint64_t>(0x350020,0x351000);good.Put<uint32_t>(0x35002c,0);good.Put<uint32_t>(0x35003c,0);
  good.Put<uint64_t>(base+0x0592d128,root);good.Put<uint64_t>(root+0x608,machine);good.Put<uint64_t>(machine+0x120,world);
  good.Put<uint64_t>(machine+0xb0,0x360000);good.Put<uint32_t>(machine+0xb8,3);good.Put<uint32_t>(machine+0xbc,3);
  good.Put<uint64_t>(world+0x5618,base+0x044bb380);good.Zero(world+0x5680,24);
  good.Put<uint64_t>(world+0x5680,0x370000);good.Put<uint32_t>(world+0x5688,1);good.Put<uint32_t>(world+0x568c,1);
  good.Put<uint64_t>(0x370000,connection);good.Zero(connection,0x38);good.Put<uint64_t>(connection,base+0x044e08d8);
  good.Put<uint64_t>(connection+0x18,definition);good.Put<uint32_t>(connection+0x20,2);
  good.Zero(definition,0x80);good.Put<uint64_t>(definition,base+0x04515e20);good.Put<uint64_t>(definition+0x50,key);
  for(unsigned i=0;i<2;++i){
    const uint64_t node=0x420000+i*0x100,def=0x430000+i*0x100,province=0x440000+i*0x100;
    good.Put<uint64_t>(connection+0x28+i*8,node);good.Put<uint64_t>(definition+0x40+i*8,def);
    good.Put<uint64_t>(node,base+0x044e3d78);good.Put<uint64_t>(node+0x78,def);
    good.Put<uint64_t>(def,base+0x044f8a78);good.Put<uint16_t>(def+0x44,uint16_t(i+1));
    good.Put<std::array<int64_t,2>>(def+0x48,i?std::array<int64_t,2>{500000,600000}:std::array<int64_t,2>{200000,300000});
    good.Put<uint64_t>(0x360000+8*(i+1),province);good.Put<uint64_t>(province,base+0x04437a00);
    good.Put<uint32_t>(province+0x18,i?second:first);
  }
  good.Put<uint64_t>(definition+0x68,0x450000);good.Put<uint32_t>(definition+0x70,1);good.Put<uint32_t>(definition+0x74,1);
  good.Put<std::array<int64_t,3>>(0x450000,{0,500000,600000});
  NativeSplineNetwork network;network.width=999;network.height=777;
  network.points={{100,{2.2f,3.4f}},{200,{5.1f,6.9f}}};network.strips={{strip,{100,200}}};network.connections={{key,{strip}}};
  std::vector<NativeStateEconomy> states={{first,UINT32_MAX,7},{second,UINT32_MAX,7}};std::string error;
  auto run=[&](Memory& memory,const auto& map){return DecodeNativeDomesticSources(
      [&](auto a,auto p,auto n){return memory.Read(a,p,n);},base,states,2,map,[&]{return memory.current;},&error);};
  auto copy=run(good,network);
  if(!copy)std::printf("reason: %s\n",error.c_str());
  Check(copy && copy->orders.size()==1 && copy->orders[0].state==first && copy->orders[0].production==12345 &&
      copy->orders[0].region==UINT32_MAX,"current local orders preserve full states and explicit scale without derived map");
  Check(copy->land.size()==1 && copy->land[0].source==first && copy->land[0].destination==second &&
      copy->land[0].spline==key && copy->land[0].type==2,"current original province/state endpoints bind full native spline key");
  auto reject=[&](auto change,const char* why){auto m=good;auto n=network;change(m,n);Check(!run(m,n),why);};
  reject([&](auto& m,auto&){m.template Put<uint32_t>(connection+0x20,8);},"unknown current category rejected");
  reject([&](auto& m,auto&){m.template Put<uint32_t>(connection+0x20,4);},"current built-rail category requires a rail definition and spline");
  reject([&](auto& m,auto&){m.template Put<uint64_t>(0x440000,base);},"untyped runtime province rejected");
  reject([&](auto& m,auto&){m.template Put<uint32_t>(0x440018,1);},"low slot cannot impersonate full state handle");
  reject([&](auto& m,auto&){m.template Put<uint16_t>(0x430044,3);},"province count bound enforced");
  reject([&](auto&,auto& n){n.connections.clear();},"missing current connection never falls back to map table");
  reject([&](auto&,auto& n){n.points.at(200).x=25;},"same key changed geometry fails definition comparison");
  reject([&](auto& m,auto&){m.template Put<uint64_t>(definition+0x68,UINT64_MAX-8);},"path address overflow rejected");
  reject([&](auto& m,auto&){m.template Put<int64_t>(0x340000,-1);},"negative active goods quantity rejected");
  reject([&](auto& m,auto&){m.mutate=true;m.reads=0;},"combined domestic copy rejects source mutation");
  reject([&](auto& m,auto&){m.current=false;},"retired domestic source rejected");
  for(unsigned type:{0u,1u,5u,6u,7u}){auto m=good;m.Put<uint32_t>(connection+0x20,type);
    auto none=run(m,network);Check(none && none->land.empty(),"non-land or unavailable runtime connection is not enabled");}
  states[1].market=8;Check(run(good,network)->land.empty(),"land model cannot cross into another current market");states[1].market=7;
  auto rail=good;auto railNetwork=network;
  rail.Put<uint32_t>(connection+0x20,4);rail.Put<uint32_t>(definition+0x58,1);rail.Put<uint64_t>(definition+0x50,key|1);
  railNetwork.strips={{strip|1,{100,200}}};railNetwork.connections={{key|1,{strip|1}}};
  auto built=run(rail,railNetwork);Check(built && built->builtRails.size()==1 && built->land.size()==1,
      "all currently built rail connections are copied independently of modeled land flow");
  auto intra=rail;intra.Put<uint32_t>(0x440118,first);auto within=run(intra,railNetwork);
  Check(within && within->builtRails.size()==1 && within->land.empty(),"intra-state built rail survives without creating self shipments");
  states[1].market=8;auto border=run(rail,railNetwork);states[1].market=7;
  Check(border && border->builtRails.size()==1 && border->land.empty(),"cross-market built rail remains known without enabling cross-market model flow");
  auto unbuilt=rail;unbuilt.Put<uint32_t>(connection+0x20,3);
  Check(run(unbuilt,railNetwork)->builtRails.empty(),"authored rail without current built category is not laid track");
  auto ship=good;ship.Put<uint32_t>(0x35002c,1);ship.Put<uint32_t>(0x35003c,1);
  ship.Zero(0x351000,16);ship.Put<uint64_t>(0x351008,0x460000);ship.Zero(0x460000,0x68);
  ship.Put<uint64_t>(0x460000,base+0x04427d98);ship.Put<uint64_t>(0x460010,base+0x04427d60);
  ship.Put<uint32_t>(0x460018,0xab000000);ship.Put<uint32_t>(0x460048,first);ship.Put<uint32_t>(0x46004c,second);
  ship.Put<uint64_t>(base+0x05964e20,0x470000);ship.Put<uint64_t>(0x470020,0x471000);ship.Put<uint32_t>(0x47002c,1);
  ship.Put<uint64_t>(0x471008,0x480000);ship.Put<uint64_t>(0x480000,base+0x04494cb8);ship.Put<uint32_t>(0x480018,0xab000000);
  ship.Zero(0x480338,24);ship.Put<uint64_t>(0x480338,0x490000);ship.Put<uint32_t>(0x480340,1);ship.Put<uint32_t>(0x480344,1);
  ship.Put<uint32_t>(0x490000,0);
  Check(run(ship,network)->shipping.lanes.size()==1,"domestic lanes require exact original country port-connection reference");
  {
    auto owned=run(ship,network);owned->shipping.lanes.push_back(owned->shipping.lanes[0]);
    NativeWorldTradeSnapshot trade;trade.straits={{0x12000000,{10,11},{0,0},0,{1,2,3,4},1}};
    ship.Put<uint64_t>(base+0x059667f8,0x4a0000);ship.Put<uint64_t>(0x4a0000,base+0x04439720);
    ship.Put<uint64_t>(0x4a0020,0x4b0000);ship.Put<uint32_t>(0x4a002c,1);ship.Put<uint64_t>(0x4b0008,0x4c0000);
    ship.Put<uint64_t>(0x4c0000,base+0x04490548);ship.Put<uint32_t>(0x4c0008,0x12000000);
    ship.Put<uint64_t>(0x4c00a0,0x4d0000);ship.Put<std::array<uint8_t,4>>(0x4c00a8,{1,2,3,4});
    ship.Put<uint64_t>(0x4d0000,base+0x044f85a0);ship.Put<uint32_t>(0x4d0010,0);
    ship.Put<std::array<uint16_t,2>>(0x4d0044,{10,11});ship.Put<uint8_t>(0x4d004b,1);
    int calls=0;
    auto access=DecodeDomesticStraitAccess([&](auto a,auto p,auto n){return ship.Read(a,p,n);},base,*owned,trade,
        []{return true;},[&](uint64_t strait,uint64_t country)->std::optional<bool>{
          ++calls;Check(strait==0x4c0000 && country==0x480000,"validated objects reach adapter only");return true;});
    Check(access && access->at(0xab000000)==std::vector<uint32_t>{0x12000000} && calls==2,
        "owned original access is deduplicated by country and preserves full strait handles");
    auto denied=DecodeDomesticStraitAccess([&](auto a,auto p,auto n){return ship.Read(a,p,n);},base,*owned,trade,
        []{return true;},[](uint64_t,uint64_t)->std::optional<bool>{return false;});
    Check(denied && denied->at(0xab000000).empty(),"known denied access differs from unavailable original access");
    int decisions=0;
    Check(!DecodeDomesticStraitAccess([&](auto a,auto p,auto n){return ship.Read(a,p,n);},base,*owned,trade,
        []{return true;},[&](uint64_t,uint64_t)->std::optional<bool>{return ++decisions==1;}),
        "changed original access decision without control-byte changes refuses publication");
    decisions=0;
    Check(!DecodeDomesticStraitAccess([&](auto a,auto p,auto n){return ship.Read(a,p,n);},base,*owned,trade,
        []{return true;},[&](uint64_t,uint64_t)->std::optional<bool>{return ++decisions>1;}),
        "newly allowed access during copy is not treated as an old denial");
    decisions=0;
    Check(!DecodeDomesticStraitAccess([&](auto a,auto p,auto n){return ship.Read(a,p,n);},base,*owned,trade,
        []{return true;},[&](uint64_t,uint64_t)->std::optional<bool>{if(++decisions==1)return true;return {};}),
        "unavailable repeated access cannot validate an earlier decision");
    Check(!DecodeDomesticStraitAccess([&](auto a,auto p,auto n){return ship.Read(a,p,n);},base,*owned,trade,
        []{return true;},[](uint64_t,uint64_t)->std::optional<bool>{return {}; }),
        "unavailable original access cannot be published as a decision");
    int leaseChecks=0;
    Check(!DecodeDomesticStraitAccess([&](auto a,auto p,auto n){return ship.Read(a,p,n);},base,*owned,trade,
        [&]{return ++leaseChecks<2;},[](uint64_t,uint64_t)->std::optional<bool>{Check(false,"retired source reached callback");return true;}),
        "retired access scope rejects results");
    Check(!DecodeDomesticStraitAccess([&](auto a,auto p,auto n){return ship.Read(a,p,n);},base,*owned,trade,
        []{return true;},[&](uint64_t,uint64_t)->std::optional<bool>{ship.Put<uint8_t>(0x4c00a8,9);return true;}),
        "changed controls during original access refuse the whole result");
    ship.Put<uint8_t>(0x4c00a8,1);ship.Put<uint32_t>(0x4c0008,0x13000000);
    Check(!DecodeDomesticStraitAccess([&](auto a,auto p,auto n){return ship.Read(a,p,n);},base,*owned,trade,
        []{return true;},[](uint64_t,uint64_t)->std::optional<bool>{Check(false,"stale strait reached callback");return true;}),
        "recycled strait cannot reach original access callback");
    ship.Put<uint32_t>(0x4c0008,0x12000000);ship.Put<uint32_t>(0x480018,0xac000000);
    Check(!DecodeDomesticStraitAccess([&](auto a,auto p,auto n){return ship.Read(a,p,n);},base,*owned,trade,
        []{return true;},[](uint64_t,uint64_t)->std::optional<bool>{Check(false,"stale country reached callback");return true;}),
        "recycled country cannot reach original access callback");
    ship.Put<uint32_t>(0x480018,0xab000000);
  }
  ship.Put<uint32_t>(0x490000,0x1000000);Check(!run(ship,network),"retired shipping handle cannot match numeric slot");
  std::puts("PASS: current domestic orders, original port references, typed province/state land endpoints, geometry and retirement");
}
