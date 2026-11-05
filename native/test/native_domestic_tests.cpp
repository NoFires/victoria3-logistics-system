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
    if(at==0x10000000+0x05c07c68 && mutate)Put<uint64_t>(at,0);
    return true;
  }
};
int main(){
  constexpr uint64_t base=0x10000000,root=0x200000,machine=0x210000,world=0x220000,
      db=0x300000,entries=0x310000,s0=0x320000,s1=0x330000,
      provinceTable=0x360000,p0=0x380000,p1=0x381000,
      landNodes=0x3b0000,landConns=0x3c0000,navalConns=0x3e0000,
      ndefA=0x430000,ndefB=0x430100,cdef=0x440000,crow=0x450000,
      nndefA=0x470000,nndefB=0x470100,ncdef=0x480000,ncrow=0x490000,
      key=(uint64_t(200)<<35)|(uint64_t(100)<<6),nkey=(uint64_t(0x8493e0)<<35)|(uint64_t(0x800064)<<6)|3,
      strip=0x100000000,nstrip=0x100000001;
  constexpr uint32_t first=0xab000001,second=0xcd000002;
  Memory good;
  good.Put<uint64_t>(base+0x05bd69c8,db);good.Put<uint64_t>(db+0x20,entries);good.Put<uint32_t>(db+0x2c,3);
  good.Zero(entries,48);good.Put<uint64_t>(entries+24,s0);good.Put<uint64_t>(entries+40,s1);
  for(auto [object,id]:{std::pair{s0,first},std::pair{s1,second}}){
    good.Put<uint64_t>(object,base+0x046bad90);good.Put<uint32_t>(object+8,id);
    good.Zero(object+0x80,0x30);good.Zero(object+0xd0,0x30);
  }
  good.Put<uint64_t>(s0+0xd8,0x340000);good.Put<uint32_t>(s0+0xe0,2);good.Put<uint32_t>(s0+0xe4,2);
  good.Put<uint64_t>(s0+0xf0,1);good.Put<int64_t>(0x340000,1234500);good.Put<int64_t>(0x340008,-100);
  // Empty but valid shipping database: no lanes.
  good.Put<uint64_t>(base+0x05c42d88,0x350000);good.Put<uint64_t>(0x350000,base+0x0470cc00);
  good.Put<uint64_t>(0x350020,0x351000);good.Put<uint32_t>(0x35002c,0);good.Put<uint32_t>(0x35003c,0);
  good.Put<uint64_t>(base+0x05c07c68,root);good.Put<uint64_t>(root,base+0x04660618);
  good.Put<uint64_t>(root+0x608,machine);good.Put<uint64_t>(machine,base+0x04723c88);
  good.Put<uint64_t>(machine+0x120,world);good.Put<uint64_t>(world,base+0x0468ef80);
  good.Put<uint64_t>(machine+0xb0,provinceTable);good.Put<uint32_t>(machine+0xb8,3);good.Put<uint32_t>(machine+0xbc,3);
  good.Put<uint64_t>(provinceTable+8,p0);good.Put<uint64_t>(provinceTable+16,p1);
  good.Zero(p0,0x28);good.Zero(p1,0x28);
  good.Put<uint64_t>(p0,base+0x046ac1c0);good.Put<uint32_t>(p0+0x18,first);
  good.Put<uint64_t>(p1,base+0x046ac1c0);good.Put<uint32_t>(p1+0x18,second);
  // NTravelNetwork manager and its land/naval containers.
  good.Put<uint64_t>(world+0x5028,base+0x046b1d48);
  good.Put<uint64_t>(world+0x5028+0x10,landNodes);good.Put<uint32_t>(world+0x5028+0x18,2);good.Put<uint32_t>(world+0x5028+0x1c,0);
  good.Put<uint64_t>(world+0x5028+0x28,crow);good.Put<uint32_t>(world+0x5028+0x30,1);good.Put<uint32_t>(world+0x5028+0x34,1);
  good.Put<uint64_t>(world+0x5028+0x58,ncrow);good.Put<uint32_t>(world+0x5028+0x60,1);good.Put<uint32_t>(world+0x5028+0x64,1);
  good.Put<uint64_t>(world+0x5028+0x20,1);good.Put<uint64_t>(world+0x5028+0x38,1);good.Put<uint64_t>(world+0x5028+0x68,1);
  good.Zero(landNodes,0x60);good.Zero(crow,0x40);good.Zero(ncrow,0x40);
  good.Zero(cdef,0x28);good.Zero(ncdef,0x28);
  good.Zero(ndefA,0x38);good.Zero(ndefB,0x38);good.Zero(nndefA,0x38);good.Zero(nndefB,0x38);
  // Node definitions: anchors, provinces and (for land) the primary hub bit.
  good.Put<uint32_t>(ndefA+4,1);good.Put<uint32_t>(ndefA+8,100);good.Put<uint16_t>(ndefA+0xc,1);good.Put<uint16_t>(ndefA+0xe,0xffff);
  good.Put<uint32_t>(ndefB+8,200);good.Put<uint16_t>(ndefB+0xc,2);good.Put<uint16_t>(ndefB+0xe,0xffff);
  good.Put<uint32_t>(cdef+4,1);good.Put<uint64_t>(cdef+8,ndefA);good.Put<uint64_t>(cdef+0x10,ndefB);good.Put<uint64_t>(cdef+0x20,key);
  good.Put<uint32_t>(crow,0x1000);good.Put<uint64_t>(crow+8,cdef);
  good.Put<uint64_t>(landNodes+8,ndefA);good.Put<uint64_t>(landNodes+0x30+8,ndefB);
  // Naval port-access connection: type 5, spline key type 3, linked land province 2.
  good.Put<uint32_t>(nndefA+4,0x200);good.Put<uint32_t>(nndefA+8,0x800064);good.Put<uint16_t>(nndefA+0xc,1);good.Put<uint16_t>(nndefA+0xe,2);
  good.Put<uint32_t>(nndefB+4,0x80);good.Put<uint32_t>(nndefB+8,0x8493e0);good.Put<uint16_t>(nndefB+0xc,1);good.Put<uint16_t>(nndefB+0xe,0xffff);
  good.Put<uint32_t>(ncdef+4,5);good.Put<uint64_t>(ncdef+8,nndefA);good.Put<uint64_t>(ncdef+0x10,nndefB);good.Put<uint64_t>(ncdef+0x20,nkey);
  good.Put<uint32_t>(ncrow,0x2000);good.Put<uint64_t>(ncrow+8,ncdef);
  NativeSplineNetwork network;network.width=999;network.height=777;
  network.points={{100,{2.2f,3.4f}},{200,{5.1f,6.9f}},{0x800064,{7.0f,8.0f}},{0x8493e0,{9.0f,10.0f}}};
  network.strips={{strip,{100,200}},{nstrip,{0x800064,0x8493e0}}};
  network.connections={{key,{strip}},{nkey,{nstrip}}};
  std::vector<NativeStateEconomy> states={{first,UINT32_MAX,7},{second,UINT32_MAX,7}};std::string error;
  auto run=[&](Memory& memory){return DecodeNativeDomesticSources(
      [&](auto a,auto p,auto n){return memory.Read(a,p,n);},base,states,2,network,[&]{return memory.current;},&error);};
  auto copy=run(good);
  if(!copy)std::printf("reason: %s\n",error.c_str());
  Check(copy && copy->orders.size()==1 && copy->orders[0].state==first && copy->orders[0].production==12345 &&
      copy->orders[0].region==UINT32_MAX,"current local orders preserve full states and explicit scale without derived map");
  Check(copy && copy->land.size()==1 && copy->land[0].source==first && copy->land[0].destination==second &&
      copy->land[0].spline==key && copy->land[0].type==2,"current original province/state endpoints bind full native spline key");
  Check(copy && copy->portTransfers.size()==1 && copy->portTransfers[0].state==second &&
      copy->portTransfers[0].anchors==std::array<uint32_t,2>{0x800064,0x8493e0} &&
      copy->portTransfers[0].spline==nkey,"port access binds the linked land province and both current anchors");
  Check(copy && copy->landHubs==std::map<uint32_t,uint32_t>{{first,100}},
      "primary land hub uses the current node role bit and exact control");
  {
    auto rail=good;rail.Put<uint32_t>(cdef+4,2);rail.Put<uint64_t>(cdef+0x20,(uint64_t(200)<<35)|(uint64_t(100)<<6)|1);
    auto unbuilt=run(rail);Check(unbuilt && unbuilt->land.empty() && unbuilt->builtRails.empty(),
        "unbuilt rail row is not admitted as a road or built rail");
    rail.Put<uint16_t>(crow+4,4);
    auto built=run(rail);Check(built && built->builtRails.size()==1 && built->land.size()==1 && built->land[0].type==4,
        "row flag 4 is the current built-rail marker");
  }
  {
    auto reversed=good;reversed.Put<uint32_t>(ndefA+8,200);reversed.Put<uint32_t>(ndefB+8,100);
    auto owned=run(reversed);
    Check(owned && owned->land.size()==1 && !owned->land[0].forward && owned->landHubs.at(first)==200,
        "land direction follows the spline key when the definition endpoints are reversed");
    reversed.bytes.clear();Check(owned->land[0].spline==key && owned->landHubs.at(first)==200,
        "owned land and hub copies survive engine source destruction");
  }
  auto rejected=[&](auto change,const char* why){auto m=good;change(m);Check(!run(m),why);};
  rejected([&](auto& m){m.template Put<uint32_t>(provinceTable+16,0);},"unknown province endpoint rejected");
  rejected([&](auto& m){m.template Put<uint32_t>(p1+0x18,0x999);},"province state outside roster rejected");
  rejected([&](auto& m){m.template Put<uint64_t>(cdef+0x20,(uint64_t(200)<<35)|(uint64_t(101)<<6));},
      "spline key endpoints must match the current node anchors");
  rejected([&](auto& m){m.template Put<uint32_t>(ncdef+4,5);m.template Put<uint64_t>(ncdef+0x20,key);},
      "port access key type must be the current naval category");
  rejected([&](auto& m){m.template Put<uint16_t>(nndefB+0xe,1);},"two linked land provinces are ambiguous");
  rejected([&](auto& m){m.template Put<uint32_t>(world+0x5028,base);},"unknown travel manager rejected");
  auto retired=good;retired.current=false;Check(!run(retired),"retired sources rejected");
  auto mutated=good;mutated.mutate=true;Check(!run(mutated),"input mutation during copy rejected");
  std::printf("PASS: 1.15 domestic sources — counters, land/rail rows, hubs, port access and fail-closed bounds\n");
  return 0;
}
