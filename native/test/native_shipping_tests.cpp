#include "native_shipping.h"
#include <cstdio>
#include <cstdlib>
#include <map>

void require(bool value,const char* what){if(!value){std::printf("FAIL: %s\n",what);std::exit(1);}}
struct Memory {
  std::map<uint64_t,uint8_t> bytes;bool current=true;uint64_t mutate=0;int reads=0;
  template<class T>void Put(uint64_t address,T value){auto* p=(uint8_t*)&value;for(size_t i=0;i<sizeof(value);++i)bytes[address+i]=p[i];}
  void Zero(uint64_t address,size_t count){for(size_t i=0;i<count;++i)bytes[address+i]=0;}
  bool Read(uint64_t address,void* out,size_t size){
    for(size_t i=0;i<size;++i){auto it=bytes.find(address+i);if(it==bytes.end())return false;((uint8_t*)out)[i]=it->second;}
    if(address==mutate && ++reads==1)bytes[address]^=1;
    return true;
  }
};
int main(){
  constexpr uint64_t base=0x10000000,db=0x10000,entries=0x20000,lane=0x40000,ranges=0x50000;
  Memory good;good.Put<uint64_t>(base+0x05966760,db);good.Zero(db,0x40);
  good.Put<uint64_t>(db,base+0x04439410);good.Put<uint64_t>(db+0x20,entries);
  good.Put<uint32_t>(db+0x2c,2050);good.Put<uint32_t>(db+0x3c,1);good.Zero(entries,2050*16);
  good.Put<uint64_t>(entries+2049*16+8,lane);good.Zero(lane,0x68);
  good.Put<uint64_t>(lane,base+0x04427d98);good.Put<uint32_t>(lane+8,0xab000801);
  good.Put<uint64_t>(lane+0x10,base+0x04427d60);good.Put<uint32_t>(lane+0x18,0);
  good.Put<uint32_t>(lane+0x1c,1);good.Put<uint16_t>(lane+0x20,0x65);
  good.Put<uint32_t>(lane+0x28,0xcd000000);good.Put<uint64_t>(lane+0x30,ranges);
  good.Put<uint32_t>(lane+0x38,2);good.Put<uint32_t>(lane+0x3c,2);
  good.Put<uint32_t>(lane+0x48,0x11000000);good.Put<uint32_t>(lane+0x4c,0x22000001);
  good.Put<uint16_t>(lane+0x50,0xffff);good.Put<int64_t>(lane+0x58,1234567);
  good.Put<int64_t>(lane+0x60,2345678);
  good.Put<std::array<uint16_t,2>>(ranges,{10,2});good.Put<std::array<uint16_t,2>>(ranges+4,{900,0});
  auto check=[&](Memory& m){return v3::DecodeNativeShipping([&](auto a,auto b,auto c){return m.Read(a,b,c);},base,[&]{return m.current;});};
  auto result=check(good);
  require(result && result->lanes.size()==1,"full roster includes slot 2049 with generation");
  const auto& row=result->lanes.front();
  require(row.id==0xab000801 && row.country==0 && row.kind==1 && row.treaty==0xcd000000 &&
      row.fromState==0x11000000 && row.toState==0x22000001 && row.merchantMarineDemandFixed==1234567 &&
      row.travelDistanceFixed==2345678,"native fields retain full handles and fixed precision");
  require(row.provinces.size()==2 && row.provinces[0].first==10 && row.provinces[0].last==12 &&
      row.provinces[1].first==900 && row.provinces[1].last==900,"range delta is inclusive membership, not an ordered route");
  auto test=[&](auto change,const char* what){auto m=good;change(m);require(!check(m),what);};
  test([&](auto& m){m.template Put<uint64_t>(db,base);},"unknown database type rejected");
  test([&](auto& m){m.template Put<uint32_t>(db+0x2c,131073);},"slot allocation bounded");
  test([&](auto& m){m.template Put<uint32_t>(db+0x3c,2051);},"active count exceeds slots rejected");
  test([&](auto& m){m.template Put<uint32_t>(db+0x3c,0);},"missing active lane rejected");
  test([&](auto& m){m.template Put<uint64_t>(entries+8,lane);},"duplicate object or wrong slot rejected");
  test([&](auto& m){m.template Put<uint32_t>(lane+8,0xab000800);},"full handle slot mismatch rejected");
  test([&](auto& m){m.template Put<uint32_t>(lane+8,UINT32_MAX);},"invalid native handle rejected");
  test([&](auto& m){m.template Put<uint64_t>(lane,base+0x0442c328);},"null object primary type rejected");
  test([&](auto& m){m.template Put<uint64_t>(lane+0x10,base);},"secondary type must match exact class");
  test([&](auto& m){m.template Put<uint32_t>(lane+0x1c,2);},"uninitialized lane kind rejected");
  test([&](auto& m){m.template Put<uint16_t>(lane+0x20,0x4b);},"goods transfer requires native treaty reference");
  test([&](auto& m){m.template Put<uint32_t>(lane+0x28,UINT32_MAX);},"invalid treaty reference rejected");
  test([&](auto& m){m.template Put<uint32_t>(lane+0x18,UINT32_MAX);},"invalid country rejected");
  test([&](auto& m){m.template Put<uint32_t>(lane+0x48,UINT32_MAX);},"invalid endpoint rejected");
  test([&](auto& m){m.template Put<uint32_t>(lane+0x4c,0x11000000);},"same endpoints rejected");
  test([&](auto& m){m.template Put<int64_t>(lane+0x60,-1);},"negative native travel distance rejected");
  test([&](auto& m){m.template Put<uint32_t>(lane+0x3c,3);},"range count exceeds capacity rejected");
  test([&](auto& m){m.template Put<uint32_t>(lane+0x38,65535);},"range capacity bounded");
  test([&](auto& m){m.template Put<std::array<uint16_t,2>>(ranges,{65534,1});},"province range cannot include sentinel");
  test([&](auto& m){m.template Put<std::array<uint16_t,2>>(ranges,{0,2});},"province zero rejected");
  test([&](auto& m){m.template Put<std::array<uint16_t,2>>(ranges+4,{12,0});},"overlapping ranges rejected");
  test([&](auto& m){m.template Put<std::array<uint16_t,2>>(ranges+4,{5,0});},"unordered ranges rejected");
  test([&](auto& m){m.template Put<uint64_t>(lane+0x30,UINT64_MAX-2);},"range address overflow rejected");
  test([&](auto& m){m.bytes.erase(ranges);},"unreadable range rejected");
  for(auto address:{base+0x05966760,db,entries,lane,ranges})
    test([&](auto& m){m.mutate=address;},"any observed input mutation rejected");
  test([&](auto& m){m.current=false;},"retired request rejected");
  auto port=good;port.Put<uint32_t>(lane+0x1c,0);port.Put<uint16_t>(lane+0x20,0);
  require(check(port) && check(port)->lanes[0].treaty==UINT32_MAX,"port connection has no invented treaty or commodity");
  port.Put<int64_t>(lane+0x60,0);require(check(port) && check(port)->lanes[0].travelDistanceFixed==0,
      "zero original distance is valid for travel within one sea region, not unavailable");
  auto empty=good;empty.Put<uint32_t>(db+0x2c,0);empty.Put<uint32_t>(db+0x3c,0);empty.Put<uint64_t>(db+0x20,0);
  require(check(empty) && check(empty)->lanes.empty(),"known empty native database remains distinct from unavailable");
  auto zero=good;zero.Put<uint64_t>(entries+2049*16+8,0);zero.Put<uint64_t>(entries+8,lane);
  zero.Put<uint32_t>(lane+8,0);zero.Put<uint32_t>(lane+0x28,0);
  require(check(zero) && check(zero)->lanes[0].id==0 && check(zero)->lanes[0].treaty==0,"zero handles are valid identities");
  auto noRoute=good;noRoute.Put<uint32_t>(lane+0x3c,0);noRoute.Put<uint64_t>(lane+0x30,0);
  require(check(noRoute) && check(noRoute)->lanes[0].provinces.empty(),"empty computed membership never invents a route");
  int calls=0;
  require(!v3::DecodeNativeShipping([&](auto a,auto b,auto c){return good.Read(a,b,c);},base,[&]{return ++calls==1;}),"retired before publication rejected");
  good.bytes.clear();require(result->lanes[0].provinces[1].first==900,"copy owns all data after source destruction");
  std::puts("PASS: native shipping roster, full handles, inclusive province sets, bounds, mutation and retirement");
}
