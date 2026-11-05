#include "native_port_approach.h"
#include <cstdio>
#include <cstdlib>
#include <map>
#include <string>
using namespace v3;
void Check(bool ok,const char* why){if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
struct Memory {
  std::map<uint64_t,uint8_t> bytes;bool current=true;
  template<class T>void Put(uint64_t at,T value){for(size_t i=0;i<sizeof(value);++i)bytes[at+i]=((uint8_t*)&value)[i];}
  void Zero(uint64_t at,size_t n){for(size_t i=0;i<n;++i)bytes[at+i]=0;}
  bool Read(uint64_t at,void* out,size_t n){
    for(size_t i=0;i<n;++i){auto p=bytes.find(at+i);if(p==bytes.end())return false;((uint8_t*)out)[i]=p->second;}
    return true;
  }
};
int main(){
  constexpr uint64_t base=0x140000000,root=0x200000,machine=0x210000,world=0x220000,
      db=0x300000,entries=0x310000,stateObject=0x320000,stateRegion=0x340000,
      provinceTable=0x360000,p1=0x380000,p2=0x381000,templates=0x390000,templateData=0x391000,
      seaTemplate=0x392000,seaRegion=0x393000,
      countryDb=0x3a0000,countryEntries=0x3a1000,country=0x3a2000,
      marketDb=0x3b0000,marketEntries=0x3b1000,market=0x3b2000,
      navalConns=0x3e0000,ndefLand=0x430000,ndefSea=0x430100,ncdef=0x440000;
  constexpr uint32_t state=0xab000001,owner=0;
  constexpr uint64_t nkey=(uint64_t(0x8493e0)<<35)|(uint64_t(0x800108)<<6)|3;
  Memory good;
  auto region=[&](uint64_t at,uint32_t id,uint32_t geo){
    good.Zero(at,0x50);good.Put<uint64_t>(at,base+0x046ba948);
    good.Put<uint32_t>(at+0x10,id);good.Put<uint32_t>(at+0x40,geo);good.Put<uint8_t>(at+0x44,1);
  };
  // Selected state: region id 17 / geo 42, owner country 0 -> market 7.
  // Slot 1 holds the selected state in the current typed state database.
  good.Put<uint64_t>(base+0x05bd69c8,db);good.Put<uint64_t>(db+0x20,entries);
  good.Put<uint32_t>(db+0x2c,2);good.Zero(entries,32);good.Put<uint64_t>(entries+8+16,stateObject);
  good.Put<uint64_t>(stateObject,base+0x046bad90);good.Put<uint32_t>(stateObject+8,state);
  good.Put<uint8_t>(stateObject+0x1258,0);good.Put<uint32_t>(stateObject+0xb78,UINT32_MAX);
  good.Put<uint32_t>(stateObject+0xed8,owner);good.Put<uint64_t>(stateObject+0xf10,stateRegion);
  region(stateRegion,17,42);
  good.Put<uint64_t>(base+0x05c42c20,countryDb);good.Put<uint64_t>(countryDb+0x20,countryEntries);
  good.Put<uint32_t>(countryDb+0x2c,1);good.Put<uint64_t>(countryEntries+8,country);
  good.Put<uint64_t>(country,base+0x0468fce0);good.Put<uint32_t>(country+0x18,owner);
  good.Put<uint32_t>(country+0xaf4,7);
  good.Put<uint64_t>(base+0x05c43518,marketDb);good.Put<uint64_t>(marketDb+0x20,marketEntries);
  good.Put<uint32_t>(marketDb+0x2c,8);good.Put<uint64_t>(marketEntries+8+7*16,market);
  good.Put<uint64_t>(market,base+0x046d4670);good.Put<uint32_t>(market+8,7);
  // Province runtime table: 1 = sea physical province (no owner), 2 = the port's
  // land province, owned by the selected state.
  good.Put<uint64_t>(machine+0xb0,provinceTable);good.Put<uint32_t>(machine+0xb8,4);good.Put<uint32_t>(machine+0xbc,4);
  good.Put<uint64_t>(provinceTable+8,p1);good.Put<uint64_t>(provinceTable+16,p2);
  good.Zero(p1,0x28);good.Zero(p2,0x28);
  good.Put<uint64_t>(p1,base+0x046ac1c0);good.Put<int32_t>(p1+0x18,-1);
  good.Put<uint64_t>(p2,base+0x046ac1c0);good.Put<int32_t>(p2+0x18,int32_t(state));
  // Province template registry: physical sea province 1 -> region 900/700.
  good.Put<uint64_t>(base+0x05bd7308,templates);good.Put<uint64_t>(templates+0x2a0,templateData);
  good.Put<uint32_t>(templates+0x2ac,4);good.Put<uint64_t>(templateData+8,seaTemplate);
  good.Put<uint64_t>(seaTemplate,base+0x046ab568);good.Put<uint64_t>(seaTemplate+0xc8,seaRegion);
  region(seaRegion,900,700);
  // Current travel network: one naval type-5 access with land province 2 and sea province 1.
  good.Put<uint64_t>(base+0x05c07c68,root);good.Put<uint64_t>(root,base+0x04660618);
  good.Put<uint64_t>(root+0x608,machine);good.Put<uint64_t>(machine,base+0x04723c88);
  good.Put<uint64_t>(machine+0x120,world);good.Put<uint64_t>(world,base+0x0468ef80);
  good.Put<uint64_t>(world+0x5028,base+0x046b1d48);
  good.Put<uint64_t>(world+0x5028+0x58,navalConns);good.Put<uint32_t>(world+0x5028+0x60,2);good.Put<uint32_t>(world+0x5028+0x64,1);
  good.Put<uint64_t>(world+0x5028+0x68,1);
  good.Zero(navalConns,0x40);good.Zero(ncdef,0x28);good.Zero(ndefLand,0x38);good.Zero(ndefSea,0x38);
  good.Put<uint32_t>(ndefLand+4,0x200);good.Put<uint32_t>(ndefLand+8,0x800108);
  good.Put<uint16_t>(ndefLand+0xc,2);good.Put<uint16_t>(ndefLand+0xe,2);
  good.Put<uint32_t>(ndefSea+4,0x80);good.Put<uint32_t>(ndefSea+8,0x8493e0);
  good.Put<uint16_t>(ndefSea+0xc,1);good.Put<uint16_t>(ndefSea+0xe,0xffff);
  good.Put<uint32_t>(ncdef+4,5);good.Put<uint64_t>(ncdef+8,ndefLand);
  good.Put<uint64_t>(ncdef+0x10,ndefSea);good.Put<uint64_t>(ncdef+0x20,nkey);
  good.Put<uint32_t>(navalConns,0x2000);good.Put<uint64_t>(navalConns+8,ncdef);
  std::string why;
  auto run=[&](Memory& memory){
    GoodsMemoryReader reader=[&](uint64_t a,void* p,size_t n){return memory.Read(a,p,n);};
    return DecodeNativePortApproaches(reader,base,state,2,[&]{return memory.current;},&why);};
  auto result=run(good);
  if(!result)std::printf("reason=%s\n",why.c_str());
  Check(result && result->state==state && result->market==7 && result->eligibleShores==1 &&
      result->queries.size()==1,"selected state binds its current type-5 port access");
  const auto& query=result->queries.front();
  Check(query.ownerState==state && query.market==7 && query.stateDefinition==17 && query.stateGeo==42 &&
      query.seaDefinition==900 && query.seaGeo==700 && query.stateAnchor==0x800108 &&
      query.seaAnchor==0x8493e0 && query.logicalProvince==2 && query.physicalProvince==1 &&
      query.spline==nkey,"state, sea region, anchors, provinces and spline stay distinct");
  {
    Memory bad=good;bad.Put<uint32_t>(world+0x5028+0x64,2);
    bad.Zero(navalConns,0x80);
    bad.Put<uint32_t>(navalConns,0x2000);bad.Put<uint64_t>(navalConns+8,ncdef);
    bad.Put<uint32_t>(navalConns+0x40,0x2000);bad.Put<uint64_t>(navalConns+0x48,ncdef);
    Check(!run(bad),"duplicate current port transfer id rejected");
  }
  {
    Memory bad=good;bad.Put<uint32_t>(ncdef+4,8);
    auto none=run(bad);Check(none && none->eligibleShores==0 && none->queries.empty(),
        "non-port connection types never become an approach");
  }
  {
    Memory bad=good;bad.Put<uint64_t>(ncdef+0x20,nkey&~uint64_t(3));
    Check(!run(bad),"a non type-3 spline cannot masquerade as port access");
  }
  {
    Memory bad=good;bad.Put<uint64_t>(seaTemplate+0xc8,0);
    Check(!run(bad),"missing physical sea region rejected");
  }
  {
    Memory bad=good;bad.Put<uint32_t>(stateObject+0xf10,0);
    Check(!run(bad),"missing selected state region rejected");
  }
  {
    Memory bad=good;bad.current=false;
    Check(!run(bad),"retired caller rejected");
  }
  good.current=true;
  auto owned=run(good);good.bytes.clear();
  Check(owned && owned->queries[0]==query,"port metadata owns values after source destruction");
  std::puts("PASS: 1.15 typed port access, sea region identity, invalid types, retirement and owned values");
}
