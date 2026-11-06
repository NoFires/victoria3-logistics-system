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
    if(at==0x10000000+0x05c07c68 && ++reads==1 && mutate)Put<uint64_t>(at,0);
    return true;
  }
  void Array(uint64_t at,uint64_t data,uint32_t count){Zero(at,24);Put(at,data);Put(at+8,count);Put(at+12,count);}
  void Node(uint64_t at,uint64_t definition,uint32_t id,uint16_t roles,uint16_t province,uint32_t control,std::array<int64_t,2> position){
    Zero(at,0x30);Zero(definition,0x20);Put(at,id);Put(at+6,province);Put(at+8,definition);
    Put(definition,id);Put(definition+4,roles);Put(definition+8,control);Put(definition+12,province);Put(definition+16,position);
  }
};
int main(){
  constexpr uint64_t base=0x10000000,root=0x200000,machine=0x210000,world=0x220000,
      db=0x300000,entries=0x310000,s0=0x320000,s1=0x330000,connection=0x400000,definition=0x410000,
      nodes=0x420000,defs=0x430000,naval=0x450000,navalDef=0x450100,port=0x450200,portDef=0x450300,
      key=(uint64_t(200)<<35)|(uint64_t(100)<<6),strip=0x100000000;
  constexpr uint32_t first=0xab000001,second=0xcd000002;
  Memory good;good.Put<uint64_t>(base+0x05bd69c8,db);good.Put<uint64_t>(db+0x20,entries);good.Put<uint32_t>(db+0x2c,3);
  good.Zero(entries,48);good.Put<uint64_t>(entries+24,s0);good.Put<uint64_t>(entries+40,s1);
  for(unsigned i=0;i<2;++i){
    const auto object=i?s1:s0;const auto id=i?second:first;const uint64_t region=0x4e0000+i*0x100,province=0x440000+i*0x100,provinceDef=0x441000+i*0x100;
    good.Put<uint64_t>(object,base+0x046bad90);good.Put<uint32_t>(object+8,id);good.Zero(object+0x80,0x30);good.Zero(object+0xd0,0x30);
    good.Put<uint64_t>(object+0xf10,region);good.Put<uint64_t>(region,base+0x046ba948);
    good.Put<uint32_t>(region+0x10,77+i);good.Put<uint32_t>(region+0x40,29+i);good.Put<uint8_t>(region+0x44,1);
    good.Put<uint64_t>(0x360000+8*(i+1),province);good.Put<uint64_t>(province,base+0x046ac1c0);
    good.Put<uint64_t>(province+8,provinceDef);good.Put<uint32_t>(province+0x18,id);
    good.Put<uint64_t>(provinceDef,base+0x046ab568);good.Put<uint64_t>(provinceDef+0xc8,region);
  }
  good.Put<uint64_t>(s0+0xd8,0x340000);good.Put<uint32_t>(s0+0xe0,2);good.Put<uint32_t>(s0+0xe4,2);
  good.Put<uint64_t>(s0+0xf0,1);good.Put<int64_t>(0x340000,1234500);good.Put<int64_t>(0x340008,-100);
  good.Put<uint64_t>(base+0x05c42d88,0x350000);good.Put<uint64_t>(0x350000,base+0x0470cc00);
  good.Put<uint64_t>(0x350020,0x351000);good.Put<uint32_t>(0x35002c,0);good.Put<uint32_t>(0x35003c,0);
  good.Put<uint64_t>(base+0x05c07c68,root);good.Put<uint64_t>(root,base+0x04660618);
  good.Put<uint64_t>(root+0x608,machine);good.Put<uint64_t>(machine+0x120,world);
  good.Put<uint64_t>(machine+0xb0,0x360000);good.Put<uint32_t>(machine+0xb8,3);good.Put<uint32_t>(machine+0xbc,3);
  good.Put<uint64_t>(world+0x5028,base+0x046b1d48);
  good.Array(world+0x5038,nodes,2);good.Array(world+0x5050,connection,1);
  good.Array(world+0x5068,0,0);good.Array(world+0x5080,0,0);
  good.Node(nodes,defs,0,0x401,1,100,{200000,300000});good.Node(nodes+0x30,defs+0x100,1,2,2,200,{500000,600000});
  good.Zero(connection,0x40);good.Put<uint8_t>(connection+4,2);good.Put<uint64_t>(connection+8,definition);
  good.Put<uint64_t>(connection+0x10,nodes);good.Put<uint64_t>(connection+0x18,nodes+0x30);good.Put<int64_t>(connection+0x20,345678);
  good.Zero(definition,0x28);good.Put<uint8_t>(definition+4,1);good.Put<uint64_t>(definition+8,defs);
  good.Put<uint64_t>(definition+0x10,defs+0x100);good.Put<int64_t>(definition+0x18,345678);good.Put<uint64_t>(definition+0x20,key);
  NativeSplineNetwork network;network.width=999;network.height=777;
  network.points={{100,{2.2f,3.4f}},{200,{5.1f,6.9f}},{300,{7.2f,8.3f}}};network.strips={{strip,{100,200}}};network.connections={{key,{strip}}};
  std::vector<NativeStateEconomy> states={{first,UINT32_MAX,7},{second,UINT32_MAX,7}};std::string error;
  auto run=[&](Memory& memory,const auto& map){return DecodeNativeDomesticSources(
      [&](auto a,auto p,auto n){return memory.Read(a,p,n);},base,states,2,map,[&]{return memory.current;},&error);};
  auto copy=run(good,network);if(!copy)std::printf("reason: %s\n",error.c_str());
  Check(copy && copy->orders.size()==1 && copy->orders[0].production==12345 && copy->orders[0].state==first &&
      copy->orders[0].region==UINT32_MAX,"current fixed counters become milli-orders with full state handles");
  Check(copy->land.size()==1 && copy->land[0].source==first && copy->land[0].destination==second &&
      copy->land[0].spline==key && copy->landHubs==std::map<uint32_t,uint32_t>{{first,100}},
      "new arrays bind exact province owners, spline key and coastal primary land role");
  auto direct=good;direct.Put<uint64_t>(definition+0x20,UINT64_MAX);
  auto noSpline=run(direct,network);
  Check(noSpline && noSpline->land.size()==1,"current keyless ground connection remains part of the physical domestic network");
  auto reject=[&](auto change,const char* why){auto m=good;auto n=network;change(m,n);Check(!run(m,n),why);};
  reject([&](auto& m,auto&){m.template Put<uint8_t>(connection+4,7);},"unknown runtime development rejected");
  reject([&](auto& m,auto&){m.template Put<uint8_t>(connection+4,4);},"built rail requires original rail kind and spline");
  reject([&](auto& m,auto&){m.template Put<uint64_t>(0x440000,base);},"untyped runtime province rejected");
  reject([&](auto& m,auto&){m.template Put<uint32_t>(0x440018,1);},"numeric slot cannot impersonate full state handle");
  reject([&](auto& m,auto&){m.template Put<uint64_t>(0x4410c8,0x4e0100);},"province definition and actual owner region must agree");
  reject([&](auto& m,auto&){m.template Put<uint32_t>(0x4e0040,UINT32_MAX);},"unknown current map identity rejected");
  reject([&](auto& m,auto&){m.template Put<uint8_t>(0x4e0044,0);},"invalid current primary hub region rejected");
  reject([&](auto& m,auto&){m.template Put<uint32_t>(defs+8,101);},"current control must equal the authored connection endpoint");
  reject([&](auto& m,auto&){m.template Put<int64_t>(defs+0x10,200001);},"changed node position cannot reuse geometry");
  reject([&](auto&,auto& n){n.connections.clear();},"missing current connection has no embedded-map fallback");
  reject([&](auto&,auto& n){n.points.at(200).x=25;},"same key changed endpoint geometry rejected");
  reject([&](auto& m,auto&){m.template Put<int64_t>(0x340000,-1);},"negative active quantity is not zero");
  reject([&](auto& m,auto&){m.mutate=true;m.reads=0;},"mutation during combined observation rejected");
  reject([&](auto& m,auto&){m.current=false;},"retired source rejected");
  for(uint8_t type:{uint8_t(0),uint8_t(1),uint8_t(5),uint8_t(6)}){
    auto m=good;m.Put(connection+4,type);auto none=run(m,network);Check(none && none->land.empty(),"unavailable or non-land development is not enabled");
  }
  states[1].market=8;Check(run(good,network)->land.empty(),"another market cannot carry domestic allocation");states[1].market=7;
  auto rail=good;auto railNetwork=network;rail.Put<uint8_t>(connection+4,4);rail.Put<uint8_t>(definition+4,2);rail.Put<uint64_t>(definition+0x20,key|1);
  railNetwork.strips={{strip|1,{100,200}}};railNetwork.connections={{key|1,{strip|1}}};
  Check(run(rail,railNetwork)->builtRails.size()==1,"only actual developed rail is built track");
  auto unbuilt=rail;unbuilt.Put<uint8_t>(connection+4,2);Check(run(unbuilt,railNetwork)->builtRails.empty(),"authored rail alone is not laid track");
  auto intra=rail;intra.Put<uint32_t>(0x440118,first);intra.Put<uint64_t>(0x4411c8,0x4e0000);
  auto within=run(intra,railNetwork);Check(within && within->builtRails.size()==1 && within->land.empty() && within->localLand.size()==1,"intra-state physical rail joins legs without self shipments");
  auto road=intra;road.Put<uint8_t>(connection+4,2);Check(run(road,railNetwork)->localLand.size()==1,"intra-state road remains a physical transfer");
  states[1].market=8;auto border=run(rail,railNetwork);states[1].market=7;
  Check(border && border->builtRails.size()==1 && border->land.empty(),"cross-market built rail is known without enabling cross-market domestic flow");
  auto reverse=good;reverse.Put<uint32_t>(defs+8,200);reverse.Put<uint32_t>(defs+0x108,100);
  reverse.Put<std::array<int64_t,2>>(defs+16,{500000,600000});reverse.Put<std::array<int64_t,2>>(defs+0x110,{200000,300000});
  Check(!run(reverse,network)->land[0].forward && run(reverse,network)->landHubs.at(first)==200,"original controls determine reversed spline direction");
  auto portMemory=good;portMemory.Array(world+0x5068,naval,1);portMemory.Array(world+0x5080,port,1);
  portMemory.Node(naval,navalDef,0,0x200,99,300,{700000,800000});portMemory.Zero(port,0x40);portMemory.Zero(portDef,0x28);
  portMemory.Put<uint8_t>(port+4,5);portMemory.Put<uint64_t>(port+8,portDef);portMemory.Put<uint64_t>(port+0x10,naval);portMemory.Put<uint64_t>(port+0x18,nodes+0x30);
  portMemory.Put<int64_t>(port+0x20,999999);portMemory.Put<uint8_t>(portDef+4,6);portMemory.Put<uint64_t>(portDef+8,navalDef);portMemory.Put<uint64_t>(portDef+0x10,defs+0x100);
  portMemory.Put<int64_t>(portDef+0x18,223606);portMemory.Put<uint64_t>(portDef+0x20,UINT64_MAX);
  portMemory.Put<uint16_t>(defs+0x104,0xc02);auto ownedPort=run(portMemory,network);
  Check(ownedPort && ownedPort->portTransfers.size()==1 && ownedPort->portTransfers[0].state==second &&
      ownedPort->portTransfers[0].anchors==std::array<uint32_t,2>{300,200} && ownedPort->portTransfers[0].origin==std::array<int64_t,2>{700000,800000} &&
      ownedPort->portTransfers[0].path==std::vector<std::array<int64_t,3>>{{223606,500000,600000}},
      "current original keyless port leg keeps definition distance and exact endpoints, independently of traversal cost");
  auto badPort=[&](auto change){auto m=portMemory;change(m);Check(!run(m,network),"invalid original port binding rejected");};
  badPort([&](auto& m){m.template Put<uint32_t>(navalDef+8,UINT32_MAX);});
  badPort([&](auto& m){m.template Put<uint64_t>(port+0x18,nodes);});
  badPort([&](auto& m){m.template Put<uint64_t>(0x4411c8,0x4e0000);});
  auto unrelated=portMemory;unrelated.Put<uint16_t>(defs+0x104,0x401);
  unrelated.Put<uint16_t>(defs+4,2);Check(run(unrelated,network)->portTransfers.empty(),"native links to non-port land roles do not become physical port legs");
  portMemory.bytes.clear();Check(ownedPort->portTransfers[0].path[0][0]==223606,"port geometry survives source destruction");
  auto ship=good;ship.Put<uint32_t>(0x35002c,1);ship.Put<uint32_t>(0x35003c,1);ship.Zero(0x351000,16);ship.Put<uint64_t>(0x351008,0x460000);ship.Zero(0x460000,0x68);
  ship.Put<uint64_t>(0x460000,base+0x04670978);ship.Put<uint64_t>(0x460010,base+0x04661088);ship.Put<uint32_t>(0x460018,0xab000000);
  ship.Put<uint32_t>(0x460048,first);ship.Put<uint32_t>(0x46004c,second);ship.Put<uint64_t>(base+0x05c42c20,0x470000);
  ship.Put<uint64_t>(0x470020,0x471000);ship.Put<uint32_t>(0x47002c,1);ship.Put<uint64_t>(0x471008,0x480000);
  ship.Put<uint64_t>(0x480000,base+0x0468fce0);ship.Put<uint32_t>(0x480018,0xab000000);ship.Array(0x4803a0,0x490000,1);ship.Put<uint32_t>(0x490000,0);
  Check(run(ship,network)->shipping.lanes.size()==1,"current country+3A0 owns the complete port-connection reference");
  {
    auto owned=run(ship,network);owned->shipping.lanes.push_back(owned->shipping.lanes[0]);
    NativeWorldTradeSnapshot trade;trade.straits={{0x12000000,{10,11},{0,0},0,{1,2,3,4},1}};
    ship.Put<uint64_t>(base+0x05c43138,0x4a0000);ship.Put<uint64_t>(0x4a0000,base+0x0471cc78);
    ship.Put<uint64_t>(0x4a0020,0x4b0000);ship.Put<uint32_t>(0x4a002c,1);ship.Put<uint64_t>(0x4b0008,0x4c0000);
    ship.Put<uint64_t>(0x4c0000,base+0x046c9758);ship.Put<uint32_t>(0x4c0008,0x12000000);
    ship.Put<uint64_t>(0x4c0120,0x4d0000);ship.Put<std::array<uint8_t,4>>(0x4c0128,{1,2,3,4});
    ship.Put<uint64_t>(0x4d0000,base+0x04786528);ship.Put<uint32_t>(0x4d0010,0);
    ship.Put<std::array<uint16_t,2>>(0x4d0044,{10,11});ship.Put<uint8_t>(0x4d0058,1);
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
        []{return true;},[&](uint64_t,uint64_t)->std::optional<bool>{ship.Put<uint8_t>(0x4c0128,9);return true;}),
        "changed controls during original access refuse the whole result");
    ship.Put<uint8_t>(0x4c0128,1);ship.Put<uint32_t>(0x4c0008,0x13000000);
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
