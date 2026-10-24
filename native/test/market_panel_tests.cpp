#include "market_panel.h"
#include <cstdio>
#include <cstdlib>
using namespace v3;
void Check(bool ok,const char* why){if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
  static constexpr uint64_t module=0x140000000,root=0x10000,gfx=0x11000,machine=0x12000,
      handler=0x13000,panels=0x14000,panel=0x15000,history=0x16000,key=0x17000,
      registry=0x18000,entries=0x19000,marketWrapper=0x1a000,rows=0x1b000,row=0x1c000,definition=0x1d000,exclusions=0x1e000;
  std::array<uint8_t,0x10000> bytes{};
  uint64_t global=root;
  uint64_t registryGlobal=registry;
  template<class T>void Put(uint64_t a,T value){std::memcpy(bytes.data()+a-root,&value,sizeof(value));}
  void Text(uint64_t object,uint64_t offset,const char* value){
    uint64_t n=std::strlen(value);Put(object+offset+16,n);Put(object+offset+24,uint64_t(n<16?15:31));
    if(n<16)std::memcpy(bytes.data()+object+offset-root,value,n+1);
    else{Put(object+offset,key);std::memcpy(bytes.data()+key-root,value,n+1);}
  }
  Fixture(){
    Put(root,module+kRvaInterfaceIdlerVtable);Put(root+0x18,machine);Put(machine+0x588,int32_t(4));
    Put(root+0x10,gfx);Put(gfx,module+kRvaInterfaceGfxVtable);Put(gfx+0x20,handler);
    Put(handler,module+kRvaInterfaceHandlerVtable);Put(handler+0xd8,handler+0xd8);
    Put(handler+0xe0,panels);Put(handler+0xec,int32_t(56));Put(panels+9*8,panel);
    Put(panel,module+kRvaMarketPanelVtable);Text(panel,8,"market_panel");Text(panel,0x30,"default");
    Put(panel+0x70,uint32_t(UINT32_MAX));Put(handler+0xd8+0x40,uint8_t(1));
    Put(handler+0xd8+0x20,history);
    Text(panel,0x88,"");Put(panel+0x110,exclusions);
    Put(registry,entries);Put(registry+8,int32_t(1));Put(registry+12,int32_t(1));Put(entries,marketWrapper);
    Put(marketWrapper,module+kRvaGuiMarketVtable);Put(marketWrapper+0x10,uint32_t(79));
    Text(marketWrapper,0x328,"");Put(marketWrapper+0x60,rows);
    Put(marketWrapper+0x68,int32_t(1));Put(marketWrapper+0x6c,int32_t(1));Put(rows,row);
    Put(row,module+kRvaGuiGoodsVtable);Put(row+0x10,definition);Put(row+0x18,uint32_t(UINT32_MAX));Put(row+0x1c,uint32_t(79));
    Put(definition,module+kRvaGoodVtable);Put(definition+0x10,uint32_t(7));Put(definition+0x40,uint8_t(2));Put(definition+0x41,uint8_t(1));
  }
  void Open(uint32_t market=79){
    Put(panel+0x70,market);Text(panel,0x30,"ironroad_logistics");
    Put(handler+0xd8+0x40,uint8_t(0));Put(handler+0xd8+0x2c,int32_t(2));
    Put(history,int32_t(3));Put(history+0x30,int32_t(9));
  }
  bool Read(uint64_t a,void* out,size_t n)const{
    if(a==module+kRvaInterfaceIdlerGlobal && n==8){std::memcpy(out,&global,8);return true;}
    if(a==module+kRvaGuiMarketRegistry && n==8){std::memcpy(out,&registryGlobal,8);return true;}
    if(a<root || a-root>bytes.size() || n>bytes.size()-(a-root))return false;
    std::memcpy(out,bytes.data()+a-root,n);return true;
  }
  auto Decode()const{return DecodeNativeMarketPanel([&](auto a,auto p,auto n){return Read(a,p,n);},module);}
};
int main(){
  Fixture f;auto s=f.Decode();Check(s.available && s.ingame && !s.marketOpen && s.market==UINT32_MAX && s.tab=="default","never-opened market is a known closed state");
  f.Open();s=f.Decode();Check(s.available && s.marketOpen && s.market==79 && s.tab=="ironroad_logistics","last native history entry identifies open market and heap tab key");
  Check(s.visibleGoods && *s.visibleGoods==std::vector<uint32_t>{7},"exact native cached row IDs reach copied UI state");
  f.Text(Fixture::panel,0x88,"谷物");f.Text(Fixture::marketWrapper,0x328,"谷物");s=f.Decode();
  Check(s.visibleGoods && *s.visibleGoods==std::vector<uint32_t>{7},"localized query uses native results without English-key rematching");
  f.Text(Fixture::panel,0x88,"煤");Check(!f.Decode().visibleGoods,"new query cannot reuse old native search rows");
  f=Fixture();f.Open();f.Put(Fixture::marketWrapper+0x320,uint8_t(1));Check(!f.Decode().visibleGoods,"dirty cache is unavailable, not a known empty filter");
  f=Fixture();f.Open();f.Put(Fixture::marketWrapper+0x54,uint32_t(1));Check(!f.Decode().visibleGoods,"cache writer is never waited on");
  f=Fixture();f.Open();f.Put(Fixture::marketWrapper+0x6c,int32_t(0));s=f.Decode();Check(s.visibleGoods && s.visibleGoods->empty(),"clean zero rows means a known empty match set");
  f=Fixture();f.Open();f.Put(Fixture::panel+0x118,int32_t(1));f.Put(Fixture::panel+0x11c,int32_t(1));f.Put(Fixture::exclusions,uint8_t(1));
  Check(!f.Decode().visibleGoods,"excluded category still in cache means pending native refresh");
  f.Put(Fixture::marketWrapper+0x6c,int32_t(0));s=f.Decode();Check(s.visibleGoods && s.visibleGoods->empty(),"native exclusions are not inverted into selected categories");
  f=Fixture();f.Open();f.Put(Fixture::row+0x1c,uint32_t(80));Check(!f.Decode().visibleGoods,"cached rows must belong to current panel market");
  f=Fixture();f.Open();f.Put(Fixture::registry+8,int32_t(2));f.Put(Fixture::registry+12,int32_t(2));f.Put(Fixture::entries+8,uint64_t(Fixture::marketWrapper));
  Check(!f.Decode().visibleGoods,"ambiguous native registry entry has no first-match fallback");
  f=Fixture();f.Open();int cacheReads=0;
  s=DecodeNativeMarketPanel([&](auto a,auto p,auto n){
    if(a==Fixture::marketWrapper+0x320 && ++cacheReads==2)f.Put(Fixture::marketWrapper+0x320,uint8_t(1));
    return f.Read(a,p,n);
  },Fixture::module);
  Check(!s.visibleGoods,"cache mutation rejects copied membership");
  f=Fixture();f.Open();
  f.Put(Fixture::history+0x30,int32_t(3));s=f.Decode();Check(s.available && !s.marketOpen,"earlier cached market state cannot override another active panel");
  f.Open();f.Put(Fixture::handler+0xd8+0x40,uint8_t(1));Check(!f.Decode().marketOpen,"closed flag wins over retained history and selected tab");
  f.Open();f.Text(Fixture::panel,0x30,"default");s=f.Decode();Check(s.marketOpen && s.tab=="default","market open differs from selected logistics tab");
  f=Fixture();f.Put(Fixture::machine+0x588,int32_t(1));s=f.Decode();Check(s.available && !s.ingame && !s.marketOpen,"non-ingame phase does not follow stale panel pointers");
  f=Fixture();f.Put(Fixture::handler,uint64_t(123));Check(!f.Decode().available,"wrong handler type rejected");
  f=Fixture();f.Put(Fixture::handler+0xd8,uint64_t(0));Check(!f.Decode().available,"bar ownership identity verified");
  f=Fixture();f.Put(Fixture::panels+9*8,uint64_t(UINT64_MAX-3));Check(!f.Decode().available,"bad panel pointer fails safely");
  f=Fixture();f.Open();f.Put(Fixture::panel+0x70,uint32_t(UINT32_MAX));Check(!f.Decode().available,"open panel without market cannot select a guessed market");
  f=Fixture();f.Put(Fixture::handler+0xd8+0x2c,int32_t(-1));Check(!f.Decode().available,"negative history count rejected");
  f=Fixture();f.Text(Fixture::panel,8,"goods_panel");Check(!f.Decode().available,"matching vtable alone cannot replace panel name");
  f=Fixture();int repeats=0;
  s=DecodeNativeMarketPanel([&](auto a,auto p,auto n){
    if(a==Fixture::module+kRvaInterfaceIdlerGlobal && ++repeats==2)f.global=0;
    return f.Read(a,p,n);
  },Fixture::module);
  Check(!s.available,"idler replacement during copy rejects mixed state");
  f=Fixture();f.Open();repeats=0;
  s=DecodeNativeMarketPanel([&](auto a,auto p,auto n){
    if(a==Fixture::key && ++repeats==2)f.bytes[Fixture::key-Fixture::root]='x';
    return f.Read(a,p,n);
  },Fixture::module);
  Check(!s.available,"selected tab heap mutation rejected");
  std::puts("PASS native market panel state, cached native search/category IDs, localized queries, empty/unavailable distinction and mutation rejection");
}
