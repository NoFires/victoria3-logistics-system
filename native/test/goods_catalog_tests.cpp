#include "goods_catalog.h"
#include "goods_snapshot.h"
#include <cstdio>
#include <cstdlib>
using namespace v3;
void Check(bool ok,const char* text) { if(!ok){std::printf("FAIL: %s\n",text);std::exit(1);} }
struct Fixture {
  std::array<uint8_t,0x2000> memory{};
  static constexpr uint64_t base=0x10000, db=base+0x100, vector=base+0x200;
  GoodsCatalogLayout layout{0x140001000,0x140002000};
  template<class T> void Put(uint64_t address,T value){std::memcpy(memory.data()+address-base,&value,sizeof(value));}
  Fixture() {
    Put(db,layout.databaseVtable);Put(db+0x80,vector);Put(db+0x8c,uint32_t(4));
    const char* keys[]={"grain","services","gold","long_example_goods_key"};
    for(uint32_t id=0;id<4;++id){
      uint64_t good=base+0x300+0x70*id;Put(vector+8*id,good);Put(good,layout.goodVtable);Put(good+0x10,id);
      const uint64_t size=std::strlen(keys[id]);Put(good+0x28,size);Put(good+0x30,uint64_t(size<16?15:31));
      if(size<16)std::memcpy(memory.data()+good-base+0x18,keys[id],size+1);
      else{Put(good+0x18,base+0x800);std::memcpy(memory.data()+0x800,keys[id],size+1);}
      Put(good+0x40,uint8_t(id==1?3:id==2?0:2));Put(good+0x41,uint8_t(2));
    }
  }
  bool Read(uint64_t address,void* out,size_t bytes)const {
    if(address<base || address-base>memory.size() || bytes>memory.size()-(address-base))return false;
    std::memcpy(out,memory.data()+address-base,bytes);return true;
  }
  GoodsCatalog Decode()const{return DecodeGoodsCatalog([&](auto a,auto p,auto n){return Read(a,p,n);},db,layout);}
};
struct ContextFixture {
  Fixture goods;
  static constexpr uint64_t module=0x140000000, tlsArray=0x30000, storage=0x40000;
  std::array<uint8_t,0x8650> tls{};
  std::array<uint8_t,256> slots{};
  uint64_t first=storage;
  template<class T> void Tls(size_t offset,T value){std::memcpy(tls.data()+offset,&value,sizeof(value));}
  template<class T> void Slot(size_t offset,T value){std::memcpy(slots.data()+offset,&value,sizeof(value));}
  ContextFixture(){
    goods.layout={module+kRvaGoodsDatabaseVtable,module+kRvaGoodVtable};
    goods.Put(Fixture::db,goods.layout.databaseVtable);
    for(uint32_t i=0;i<4;++i)goods.Put(Fixture::base+0x300+0x70*i,goods.layout.goodVtable);
    Tls(0x8600,uint32_t(1));Tls(0x8648,uint64_t(0x100000003));
    Slot(0,uint64_t(UINT64_MAX-3));Slot(8,uint8_t(1)); // first populated slot is deliberately invalid
    Slot(3*16,uint64_t(Fixture::db));Slot(3*16+8,uint8_t(1));
  }
  bool Read(uint64_t a,void* out,size_t n)const {
    auto bytes=[&](uint64_t start,const auto& data){
      if(a<start || a-start>data.size() || n>data.size()-(a-start))return false;
      std::memcpy(out,data.data()+a-start,n);return true;
    };
    if(a==tlsArray && n==8){std::memcpy(out,&first,8);return true;}
    return bytes(storage,tls) || bytes(module+kRvaGoodsDatabaseSlots,slots) || goods.Read(a,out,n);
  }
  GoodsCatalog Decode()const{return DecodeThreadGoodsCatalog([&](auto a,auto p,auto n){return Read(a,p,n);},module,tlsArray);}
};
int main(){
  Fixture f;auto catalog=f.Decode();Check(catalog.ok && catalog.goods.size()==4,"complete owned catalog");
  Check(catalog.goods[0].key=="grain" && catalog.goods[0].tradeable,"SSO key and normal tradeability");
  Check(catalog.goods[1].localOnly && !catalog.goods[1].tradeable,"local flag takes priority over trade flag");
  Check(!catalog.goods[2].localOnly && !catalog.goods[2].tradeable,"non-local nontradeable metadata stays distinct");
  Check(catalog.goods[3].key=="long_example_goods_key","heap key copied");
  f.memory.fill(0);Check(catalog.goods[3].key=="long_example_goods_key","returned data has no native string dependency");
  auto bad=[](Fixture f,const char* why){auto result=f.Decode();Check(!result.ok && result.goods.empty() && !result.error.empty(),why);};
  f=Fixture();f.Put(Fixture::db+0x8c,uint32_t(129));bad(f,"bounded catalog count");
  f=Fixture();f.Put(Fixture::vector,uint64_t(UINT64_MAX-3));bad(f,"pointer overflow rejected before reader");
  f=Fixture();f.Put(Fixture::base+0x300+0x10,uint32_t(3));bad(f,"ordinal cannot replace verified ID");
  f=Fixture();f.Put(Fixture::base+0x300,uint64_t(0x1234));bad(f,"wrong native type rejected");
  f=Fixture();f.Put(Fixture::base+0x300+0x28,uint64_t(16));bad(f,"SSO length cannot exceed capacity");
  f=Fixture();f.Put(Fixture::base+0x300+0x18,uint8_t(0));bad(f,"embedded NUL cannot silently truncate key");
  f=Fixture();std::memcpy(f.memory.data()+0x370+0x18,"grain\0",6);f.Put(Fixture::base+0x370+0x28,uint64_t(5));bad(f,"duplicate keys rejected");
  f=Fixture();int reads=0;
  auto changed=DecodeGoodsCatalog([&](auto a,auto p,auto n){
    if(a==Fixture::db && ++reads==2)f.Put(Fixture::db+0x8c,uint32_t(3));return f.Read(a,p,n);
  },Fixture::db,f.layout);
  Check(!changed.ok && changed.goods.empty(),"mid-copy database replacement rejects entire catalog");
  f=Fixture();int heapReads=0;
  changed=DecodeGoodsCatalog([&](auto a,auto p,auto n){
    if(a==Fixture::base+0x800 && ++heapReads==2)f.memory[0x800]='x';return f.Read(a,p,n);
  },Fixture::db,f.layout);
  Check(!changed.ok && changed.goods.empty(),"heap content mutation rejected even when definition is unchanged");
  ContextFixture ctx;
  auto current=ctx.Decode();Check(current.ok && current.goods[0].key=="grain","current TLS context selects slot 3, never first populated slot 0");
  ctx.Tls(0x8600,uint32_t(0));Check(!ctx.Decode().ok,"uninitialized context is not initialized by decoder");
  ctx=ContextFixture();ctx.Tls(0x8648,uint64_t(16));Check(!ctx.Decode().ok,"out-of-range context rejected");
  ctx=ContextFixture();ctx.Slot(3*16+8,uint8_t(0));Check(!ctx.Decode().ok,"inactive current slot is not replaced by another context");
  ctx=ContextFixture();int contextReads=0;
  auto contextChanged=DecodeThreadGoodsCatalog([&](auto a,auto p,auto n){
    if(a==ContextFixture::tlsArray && ++contextReads==2)ctx.Tls(0x8648,uint64_t(0x200000003));
    return ctx.Read(a,p,n);
  },ContextFixture::module,ContextFixture::tlsArray);
  Check(!contextChanged.ok && contextChanged.goods.empty(),"context token change invalidates completed catalog");
  EconomySnapshot snapshot;snapshot.world=9;snapshot.revision=7;
  snapshot.orders={{10,1,2,2,0,5000},{11,2,2,1,0,7000},{12,3,2,0,9000,0}};
  std::string error;Check(PopulateSnapshotGoods(snapshot,current,&error),"native IDs resolve the full goods catalog regardless of order");
  Check(snapshot.goods.size()==4 && snapshot.goods[0].id==0 && snapshot.goods[0].key=="grain" && snapshot.goods[0].category=="industrial","copied key/category replace goodN");
  Check(snapshot.goods[3].id==3 && snapshot.goods[3].key=="long_example_goods_key",
      "a native trade good without production or consumption remains available for filtering");
  Check(snapshot.goods[1].localOnly && !snapshot.goods[1].tradeable && !snapshot.goods[2].localOnly && !snapshot.goods[2].tradeable,"local and non-local nontradeable classifications reach snapshot");
  Check(snapshot.orders[0].consumption==5000 && snapshot.orders[1].consumption==7000 && snapshot.world==9 && snapshot.revision==7,"metadata merge preserves raw orders and snapshot identity");
  EconomySnapshot noOrders;noOrders.world=9;noOrders.revision=8;
  Check(PopulateSnapshotGoods(noOrders,current,&error) && noOrders.goods.size()==4 && noOrders.orders.empty(),
      "known zero production/consumption does not suppress the current native trade catalog");
  snapshot.orders[0].good=99;Check(!PopulateSnapshotGoods(snapshot,current,&error) && snapshot.goods.empty() && !error.empty(),"unknown order ID rejects snapshot without goodN fallback");
  Check(snapshot.orders[0].consumption==5000,"unavailable metadata does not zero demand");
  snapshot.orders[0].good=2;current.goods[0].category=4;
  Check(!PopulateSnapshotGoods(snapshot,current,&error) && snapshot.goods.empty(),"unknown category is unavailable, not guessed");
  std::puts("PASS goods catalog decoding, TLS context isolation and copied metadata snapshot bridge (fixture tests)");
}
