#include "native_economy.h"
#include <cstdio>
#include <cstdlib>
#include <map>
using namespace v3;
void Check(bool ok,const char* why){if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
  static constexpr uint64_t module=0x140000000,db=0x10000,entries=0x20000,
      first=0x40000,last=0x50000,region=0x60000,country=0x70000,market=0x80000,
      countryDb=0x90000,marketDb=0xa0000;
  static constexpr uint32_t lastId=0xab000801;
  std::map<uint64_t,uint8_t> bytes;
  std::unordered_map<std::string,uint32_t> regions{{"STATE_TEST",7}};
  std::string error;
  template<class T>void Put(uint64_t at,T value){for(size_t i=0;i<sizeof(value);++i)bytes[at+i]=((uint8_t*)&value)[i];}
  void Zero(uint64_t at,size_t n){for(size_t i=0;i<n;++i)bytes[at+i]=0;}
  void Counter(uint64_t state,uint64_t off,uint64_t data,int64_t value,uint32_t good){
    Zero(state+off,0x50);Put(state+off+8,data);Put(state+off+0x10,uint32_t(2));
    Put(state+off+0x14,uint32_t(2));Put(state+off+0x20,uint64_t(1)<<good);
    Zero(data,16);Put(data+good*8,value);
  }
  Fixture(){
    Put(module+0x05964ef0,db);Put(db+0x20,entries);Put(db+0x2c,uint32_t(2050));
    Zero(entries,2050*16);Put(entries+8,first);Put(entries+2049*16+8,last);
    Zero(region,0x38);Put(region,module+0x044e7850);Put(region+0x10,uint32_t(99));
    const char key[]="STATE_TEST";for(size_t i=0;i<sizeof(key);++i)bytes[region+0x18+i]=key[i];
    Put(region+0x28,uint64_t(sizeof(key)-1));Put(region+0x30,uint64_t(15));
    Put(module+0x05964e20,countryDb);Put(countryDb+0x20,countryDb+0x100);
    Put(countryDb+0x2c,uint32_t(1));Put(countryDb+0x108,country);
    Put(country,module+0x04494cb8);Put(country+0x18,uint32_t(0));Put(country+0xa24,uint32_t(0));
    Put(module+0x05964ed8,marketDb);Put(marketDb+0x20,marketDb+0x100);
    Put(marketDb+0x2c,uint32_t(1));Put(marketDb+0x108,market);
    Put(market,module+0x0449c648);Put(market+8,uint32_t(0));
    for(auto [state,id]:{std::pair{first,uint32_t(0)},std::pair{last,lastId}}){
      Put(state,module+0x044de308);Put(state+8,id);Put(state+0xe78,region);
      Put(state+0x11b8,uint8_t(0));Put(state+0xb48,UINT32_MAX);Put(state+0xe48,uint32_t(0));
    }
    Counter(first,0xc8,0xb0000,300123,0);Counter(first,0x78,0xb1000,100099,1);
    Counter(last,0xc8,0xb2000,0,0);Counter(last,0x78,0xb3000,250050,0);
  }
  bool Read(uint64_t at,void* out,size_t n)const{
    for(size_t i=0;i<n;++i){auto f=bytes.find(at+i);if(f==bytes.end())return false;((uint8_t*)out)[i]=f->second;}return true;
  }
  auto Decode(){return DecodeNativeEconomy([&](auto a,auto p,auto n){return Read(a,p,n);},module,regions,2,[]{return true;},&error);}
  auto TradeStates(){return DecodeNativeTradeStates([&](auto a,auto p,auto n){return Read(a,p,n);},module,[]{return true;},&error);}
};
int main(){
  Fixture f;auto original=f.bytes;auto copy=f.Decode();
  Check(copy && copy->states.size()==2 && copy->orders.size()==3,"whole typed roster includes IDs beyond the old 2048 scan");
  Check(copy->states.back().id==Fixture::lastId && copy->states.back().region==7 && copy->states.back().market==0,
        "full recycled handle and independent region/market zero preserved");
  Check(copy->orders[0].production==3001 && copy->orders[1].consumption==1000 && copy->orders[2].consumption==2500,
        "native fixed-point converts to milli-orders once with truncation");
  {
    auto states=f.TradeStates();
    Check(states && states->size()==2 && states->back().id==Fixture::lastId &&
        states->front().region==UINT32_MAX && states->back().region==UINT32_MAX && states->back().market==0,
        "native roster preserves full runtime handles and explicit market zero without inventing derived-map IDs");
    auto oldRegions=f.regions;f.regions.clear();
    Check(!f.Decode() && f.TradeStates().has_value(),
        "reproduced legacy embedded-map prerequisite is absent from native trade roster");
    f.regions=oldRegions;
    const char modKey[]="MOD_REGION";
    for(size_t i=0;i<sizeof(modKey);++i)f.bytes[Fixture::region+0x18+i]=modKey[i];
    Check(!f.Decode() && f.TradeStates().has_value(),
        "a current definition key unknown to the embedded map cannot block native trade");
    f.bytes=original;f.bytes.erase(0xb0000);
    Check(!f.Decode() && f.TradeStates().has_value(),
        "native trade does not read irrelevant legacy production counters");
    f.bytes=original;
    for(auto [at,value]:{std::pair{Fixture::db+0x2c,uint32_t(65537)},
                       std::pair{Fixture::last+8,uint32_t(0xab000800)},
                       std::pair{Fixture::country+0xa24,UINT32_MAX}}){
      f.Put(at,value);Check(!f.TradeStates(),"native roster still rejects bounds, recycled full handles and unavailable markets");f.bytes=original;
    }
    f.Put(Fixture::entries+16+8,Fixture::first);Check(!f.TradeStates(),"duplicate native state cannot be silently coalesced");f.bytes=original;
    f.Put(Fixture::last,uint64_t(1));Check(!f.TradeStates(),"native state type remains mandatory without legacy map gate");f.bytes=original;
    bool changed=false;
    auto unstable=DecodeNativeTradeStates([&](auto a,auto p,auto n){
      if(a==Fixture::last+8 && !changed){changed=true;f.Put(Fixture::first+8,uint32_t(0xff000000));}
      return f.Read(a,p,n);
    },Fixture::module,[]{return true;});
    Check(!unstable,"whole native roster reread rejects an earlier handle changing during a later copy");f.bytes=original;
    unsigned currentCalls=0;
    Check(!DecodeNativeTradeStates([&](auto a,auto p,auto n){return f.Read(a,p,n);},Fixture::module,[&]{return ++currentCalls<4;}),
        "native roster retirement during enumeration prevents publication");
    Check(!DecodeNativeTradeStates({},Fixture::module,[]{return true;}),"missing native roster reader rejected");
    Check(!DecodeNativeTradeStates([&](auto a,auto p,auto n){return f.Read(a,p,n);},Fixture::module,{}),"missing current native roster request rejected");
  }
  EconomyReadWindow window([&](auto a,auto p,auto n){return f.Read(a,p,n);});
  auto combined=DecodeNativeEconomy([&](auto a,auto p,auto n){return window.Read(a,p,n);},
      Fixture::module,f.regions,2,[]{return true;});
  Check(combined && window.Unchanged(),"combined candidate starts with a stable order copy");
  f.Put(0xb0000,int64_t(999999));
  Check(!window.Unchanged(),"later decoder changing an earlier decoder's inputs rejects combined candidate");
  f.bytes=original;
  EconomyReadWindow empty({});uint64_t unused=0;
  Check(!empty.Read(1,&unused,8) && !empty.Unchanged(),"missing/empty observation cannot authorize publication");
  Check(!ReadNativeStateObject([&](auto a,auto p,auto n){return f.Read(a,p,n);},Fixture::module,2049),
        "lower 24-bit slot cannot impersonate the full handle");
  auto reject=[&](const char* label){Check(!f.Decode(),label);f.bytes=original;};
  f.Put(Fixture::db+0x2c,uint32_t(65537));reject("oversized registry rejected");
  f.Put(Fixture::db+0x20,UINT64_MAX-8);reject("registry address overflow rejected");
  f.Put(Fixture::last+8,uint32_t(0xab000800));reject("wrong slot rejected");
  f.Put(Fixture::last,uint64_t(1));reject("wrong state type rejected instead of choosing a candidate");
  f.Put(Fixture::entries+16+8,Fixture::first);reject("duplicate state object rejected");
  f.Put(Fixture::first+0xe78,uint64_t(0));reject("missing region unavailable");
  f.Put(Fixture::country+0xa24,UINT32_MAX);reject("missing market unavailable");
  f.Put(Fixture::first+0xc8+0x14,uint32_t(3));reject("counter size outside catalog rejected");
  f.Put(Fixture::first+0xc8+0x10,uint32_t(1));reject("counter capacity violation rejected");
  f.Put(Fixture::first+0xc8+0x28,uint64_t(1));reject("mask outside catalog rejected");
  f.Put(0xb0000,int64_t(-1));reject("negative order cannot be silently changed to zero");
  f.bytes.erase(0xb0000);reject("unreadable quantity rejected");
  f.Counter(Fixture::first,0xc8,0xb0000,0,0);f.Put(Fixture::first+0xc8+0x14,uint32_t(0));
  f.Put(Fixture::first+0xc8+0x20,uint64_t(0));
  Check(f.Decode().has_value(),"unallocated empty counter is known zero");f.bytes=original;
  unsigned calls=0;
  auto retired=DecodeNativeEconomy([&](auto a,auto p,auto n){return f.Read(a,p,n);},Fixture::module,f.regions,2,
      [&]{return ++calls<3;});
  Check(!retired,"request retired after reread cannot publish");
  bool mutated=false;
  auto changed=DecodeNativeEconomy([&](auto a,auto p,auto n){
    if(a==Fixture::last+8 && !mutated){mutated=true;f.Put(0xb0000,int64_t(400000));}
    return f.Read(a,p,n);
  },Fixture::module,f.regions,2,[]{return true;});
  Check(!changed,"early state's order changing during a later state copy rejects the whole result");f.bytes=original;
  calls=0;
  changed=DecodeNativeEconomy([&](auto a,auto p,auto n){
    if(a==Fixture::db+0x20 && ++calls==4)f.Put(a,uint64_t(0));
    return f.Read(a,p,n);
  },Fixture::module,f.regions,2,[]{return true;});
  Check(!changed,"roster replacement across copy rejected");
  Check(!DecodeNativeEconomy({},Fixture::module,f.regions,2,[]{return true;}),"missing reader rejected");
  std::puts("PASS: native economy whole typed roster, generations, counter units/bounds, safe reads, mutation and retirement");
}
