#pragma once
#include "goods_catalog.h"
#include "offsets.h"
#include "native_goods_filter.h"
#include <limits>

namespace v3 {
struct NativeMarketPanelState {
  bool available=false, ingame=false, marketOpen=false;
  uint32_t market=UINT32_MAX;
  std::string tab, error;
  std::optional<std::vector<uint32_t>> visibleGoods;
  // Diagnostic only: panel+0x70 is the GUI market id the vanilla getter resolves,
  // while the snapshot groups states by `state+0x48`. R84 live evidence showed
  // panel market 0 ("British market") absent from the snapshot's 52 ids, so the
  // two spaces are NOT interchangeable. This carries the candidate simulation id
  // the GUI market object holds at +0x374; nothing may authorise a view from it
  // until the mapping is verified.
  uint32_t simulationMarket=UINT32_MAX;
};

// Read the candidate simulation market id for a GUI market id from the vanilla
// GUI market registry (layout mirrored from native_goods_filter.h). Returns false
// on any ambiguity, busy lock or unreadable field; the caller must fail closed.
// Reads are not part of the caller's stability copy set.
inline bool ReadGuiMarketSimulationCandidate(const GoodsMemoryReader& reader,uint64_t module,
                                             uint32_t guiMarket,uint32_t& candidate) {
  candidate=UINT32_MAX;
  if(!reader || !module || module>UINT64_MAX-kRvaGuiMarketRegistry-8 || guiMarket==UINT32_MAX)
    return false;
  uint64_t registry=0,entries=0;
  int32_t count=0,capacity=0;
  uint32_t lock=0;
  if(!reader(module+kRvaGuiMarketRegistry,&registry,sizeof(registry)) || !registry)return false;
  if(!reader(registry,&entries,sizeof(entries)) || !entries)return false;
  if(!reader(registry+8,&capacity,sizeof(capacity)))return false;
  if(!reader(registry+12,&count,sizeof(count)))return false;
  if(!reader(registry+0x18,&lock,sizeof(lock)) || (lock&1))return false;
  if(count<0 || count>2048 || capacity<count)return false;
  uint64_t match=0;
  for(int32_t i=0;i<count;++i){
    uint64_t object=0,vtable=0;uint32_t id=0;
    if(!reader(entries+uint64_t(i)*8,&object,sizeof(object)) || !object)return false;
    if(!reader(object,&vtable,sizeof(vtable)) || vtable!=module+kRvaGuiMarketVtable)return false;
    if(!reader(object+0x10,&id,sizeof(id)))return false;
    if(id==guiMarket){
      if(match)return false; // ambiguous identity is never resolved by guessing
      match=object;
    }
  }
  if(!match)return false;
  if(!reader(match+0x374,&candidate,sizeof(candidate)))return false;
  return true;
}
// Exact 1.13.11 profile, copied on the existing UI timer thread. No engine
// functions, RTTI casts, GUI effects or retained pointers. Closed-state chain
// was observed live in R73; open/tab transitions still need DLL acceptance.
inline NativeMarketPanelState DecodeNativeMarketPanel(const GoodsMemoryReader& reader,
                                                       uint64_t module) {
  NativeMarketPanelState result;
  auto fail=[&](const char* why){return NativeMarketPanelState{false,false,false,UINT32_MAX,{},why};};
  if(!reader || !module || module>UINT64_MAX-kRvaInterfaceIdlerGlobal-8)
    return fail("invalid interface module or reader");
  struct Copy {uint64_t address; std::vector<uint8_t> bytes;};
  std::vector<Copy> copies;
  auto read=[&](uint64_t address,void* out,size_t n){
    if(!address || !n || n>UINT64_MAX-address || !reader(address,out,n))return false;
    copies.push_back({address,std::vector<uint8_t>(static_cast<uint8_t*>(out),static_cast<uint8_t*>(out)+n)});
    return true;
  };
  auto field=[&](uint64_t object,uint64_t offset,auto& value){
    return object && object<=UINT64_MAX-offset && read(object+offset,&value,sizeof(value));
  };
  auto type=[&](uint64_t object,uint32_t rva){uint64_t vt=0;return field(object,0,vt)&&vt==module+rva;};
  auto text=[&](uint64_t object,uint64_t offset,std::string& out){
    std::array<uint8_t,32> bytes{};
    if(!object || object>UINT64_MAX-offset || !read(object+offset,bytes.data(),32))return false;
    uint64_t size=0,capacity=0;
    std::memcpy(&size,bytes.data()+16,8);std::memcpy(&capacity,bytes.data()+24,8);
    if(!size || size>128 || size>capacity || (capacity<16 && capacity!=15))return false;
    std::string key(size+1,'\0');
    if(capacity<16)std::memcpy(key.data(),bytes.data(),key.size());
    else {uint64_t address=0;std::memcpy(&address,bytes.data(),8);if(!read(address,key.data(),key.size()))return false;}
    if(key.back()!=0)return false;
    key.pop_back();
    for(char c:key)if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_'))return false;
    out=std::move(key);return true;
  };
  auto stable=[&](){
    for(const auto& c:copies){std::vector<uint8_t> after(c.bytes.size());
      if(!reader(c.address,after.data(),after.size())||after!=c.bytes)return false;}
    return true;
  };
  uint64_t idler=0,machine=0,gfx=0,handler=0,barSelf=0,panels=0,panel=0,history=0;
  int32_t phase=0,count=0,historyCount=0;uint8_t closed=0;
  if(!field(module,kRvaInterfaceIdlerGlobal,idler) || !type(idler,kRvaInterfaceIdlerVtable) ||
     !field(idler,0x18,machine) || !field(machine,0x588,phase))return fail("interface identity unavailable");
  if(phase!=4){
    if(!stable())return fail("interface changed during copy");
    result.available=true;return result;
  }
  if(!field(idler,0x10,gfx) || !type(gfx,kRvaInterfaceGfxVtable) ||
     !field(gfx,0x20,handler) || !type(handler,kRvaInterfaceHandlerVtable) ||
     !field(handler,0xd8,barSelf) || barSelf!=handler+0xd8 ||
     !field(handler,0xe0,panels) || !field(handler,0xec,count) || count!=56 ||
     !field(panels,9*8,panel) || !type(panel,kRvaMarketPanelVtable))
    return fail("market panel ownership unavailable");
  std::string name;
  if(!text(panel,8,name) || name!="market_panel" || !text(panel,0x30,result.tab) ||
     !field(panel,0x70,result.market) || !field(barSelf,0x2c,historyCount) ||
     historyCount<0 || historyCount>1000000 || !field(barSelf,0x40,closed) || closed>1)
    return fail("invalid market panel state");
  // Same predicate as native IsPanelOpen: nonempty history, !closed, last kind=9.
  if(historyCount>0 && !closed){
    int32_t kind=0;
    if(!field(barSelf,0x20,history) || !field(history,uint64_t(historyCount-1)*0x30,kind) ||
       kind<0 || kind>=56)return fail("invalid panel history");
    result.marketOpen=kind==9;
  }
  if(result.marketOpen && result.market==UINT32_MAX)return fail("open market has no identity");
  if(result.marketOpen){
    uint32_t candidate=UINT32_MAX;
    if(ReadGuiMarketSimulationCandidate(reader,module,result.market,candidate))
      result.simulationMarket=candidate;
  }
  if(result.marketOpen && result.tab=="ironroad_logistics") {
    uint64_t excluded=0;int32_t excludedCount=0,excludedCapacity=0;
    std::vector<uint8_t> exclusions;
    if(!field(panel,0x110,excluded) || !field(panel,0x118,excludedCapacity) ||
       !field(panel,0x11c,excludedCount) || excludedCount<0 || excludedCount>4 || excludedCapacity<excludedCount)
      return fail("invalid native categories");
    exclusions.resize(excludedCount);
    if(excludedCount && !read(excluded,exclusions.data(),exclusions.size()))return fail("unreadable native categories");
    uint8_t seen=0;
    for(uint8_t c:exclusions){if(c>=4 || (seen&(1u<<c)))return fail("invalid native category value");seen|=1u<<c;}
    std::array<uint8_t,32> textBytes{};
    if(!read(panel+0x88,textBytes.data(),textBytes.size()))return fail("native search state unavailable");
    uint64_t size=0,cap=0;std::memcpy(&size,textBytes.data()+16,8);std::memcpy(&cap,textBytes.data()+24,8);
    if(size>4096 || size>cap || (cap<16 && cap!=15))return fail("invalid native search state");
    std::string query(size+1,'\0');
    if(cap<16)std::memcpy(query.data(),textBytes.data(),query.size());
    else {uint64_t p=0;std::memcpy(&p,textBytes.data(),8);if(!read(p,query.data(),query.size()))return fail("unreadable native search state");}
    if(query.back()!=0)return fail("unterminated native search state");
    query.pop_back();
    auto filter=DecodeNativeGoodsFilter(reader,module,result.market,query,exclusions);
    result.visibleGoods=std::move(filter.goods);result.error=std::move(filter.error);
  }
  if(!stable())return fail("market panel changed during copy");
  result.available=result.ingame=true;return result;
}
} // namespace v3
