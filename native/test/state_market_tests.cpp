#include "state_market.h"
#include <cstdio>
#include <cstdlib>
#include <map>
using namespace v3;
void Check(bool ok,const char* why){if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
 static constexpr uint64_t module=0x140000000,state=0x10000,country=0x20000,market=0x30000,area=0x40000;
 std::map<uint64_t,uint8_t> bytes;
 template<class T>void Put(uint64_t a,T value){for(size_t i=0;i<sizeof(value);++i)bytes[a+i]=((uint8_t*)&value)[i];}
 void Database(uint32_t global,uint64_t db,uint64_t object,uint32_t id,uint32_t idOffset,uint32_t type){
  Put(module+global,db);Put(db+0x20,db+0x100);Put(db+0x2c,uint32_t((id&0xffffff)+1));
  Put(db+0x108+uint64_t(id&0xffffff)*0x10,object);Put(object,module+type);Put(object+idOffset,id);
 }
 Fixture(){
  Put(state,module+0x044de308);Put(state+8,uint32_t(433));Put(state+0x48,uint32_t(111));
  Put(state+0x11b8,uint8_t(0));Put(state+0xb48,uint32_t(UINT32_MAX));Put(state+0xe48,uint32_t(1));
  Database(0x05964e20,0x50000,country,1,0x18,0x04494cb8);Put(country+0xa24,uint32_t(0));
  Database(0x05964ed8,0x60000,market,0,8,0x0449c648);
  Database(0x05964ea0,0x70000,area,4,8,0x04430cd8);Put(area+0x10,uint32_t(0));
 }
 bool Read(uint64_t a,void* p,size_t n)const{
  for(size_t i=0;i<n;++i){auto it=bytes.find(a+i);if(it==bytes.end())return false;((uint8_t*)p)[i]=it->second;}return true;
 }
 auto Decode(){return DecodeNativeStateMarket([&](auto a,auto p,auto n){return Read(a,p,n);},module,state,433);}
};
int main(){
 Fixture f;Check(f.Decode()==0,"runtime state433 resolves owner market0, not field48=111");
 auto original=f.bytes;
 f.Put(Fixture::state+0x11b8,uint8_t(1));f.Put(Fixture::state+0xb48,uint32_t(4));
 Check(f.Decode()==0,"explicit market-area branch matches the vanilla getter");
 f.Put(Fixture::country+0xa24,uint32_t(9));Check(f.Decode()==0,"explicit area never falls back to country market");
 f.Put(Fixture::area+0x10,uint32_t(UINT32_MAX));Check(!f.Decode(),"unknown explicit area market refuses fallback");
 f.bytes=original;f.Put(Fixture::state+0x11b8,uint8_t(1));Check(f.Decode()==0,"sentinel area ID follows owner branch");
 f.bytes=original;f.Put(Fixture::state+8,uint32_t(434));Check(!f.Decode(),"state identity mismatch rejected");
 f.bytes=original;f.Put(Fixture::country+0x18,uint32_t(0x01000001));Check(!f.Decode(),"recycled country handle generation rejected");
 f.bytes=original;f.Put(Fixture::market+8,uint32_t(0x01000000));Check(!f.Decode(),"recycled market handle generation rejected");
 for(auto object:{Fixture::state,Fixture::country,Fixture::market}){
  f.bytes=original;f.Put(object,uint64_t(1));Check(!f.Decode(),"wrong native object type rejected");
 }
 f.bytes=original;f.Put(Fixture::state+0x11b8,uint8_t(2));Check(!f.Decode(),"invalid branch flag rejected");
 f.bytes=original;f.Put(Fixture::state+0xe48,uint32_t(UINT32_MAX));Check(!f.Decode(),"unknown owner is unavailable, not zero");
 f.bytes=original;f.Put(uint64_t(0x50000+0x2c),uint32_t(1));Check(!f.Decode(),"out-of-capacity owner ID rejected");
 f.bytes=original;f.Put(uint64_t(0x60000+0x20),uint64_t(UINT64_MAX-1));Check(!f.Decode(),"database slot arithmetic overflow rejected");
 f.bytes=original;f.bytes.erase(Fixture::country+0xa24);Check(!f.Decode(),"unreadable market stays unavailable");
 f.bytes=original;unsigned reads=0;
 auto changed=DecodeNativeStateMarket([&](auto a,auto p,auto n){
  if(a==Fixture::country+0xa24 && ++reads==2)f.Put(a,uint32_t(1));return f.Read(a,p,n);
 },Fixture::module,Fixture::state,433);
 Check(!changed,"owner market changing during copy rejected");
 Check(!DecodeNativeStateMarket({},Fixture::module,Fixture::state,433),"missing reader rejected");
 Check(!DecodeNativeStateMarket([&](auto a,auto p,auto n){return f.Read(a,p,n);},UINT64_MAX,Fixture::state,433),"invalid module rejected");
 std::puts("PASS native state market: owner/area branches, explicit zero, types, generations, missing data and copy mutation");
}
