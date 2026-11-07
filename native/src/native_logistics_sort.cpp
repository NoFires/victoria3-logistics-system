#include "native_logistics_sort.h"
#include "game.h"
#include "offsets.h"
#include "log.h"
#include "market_panel.h"
#include "state_panel.h"
#include <windows.h>
#include <MinHook.h>
#include <atomic>
#include <algorithm>
#include <cstring>

namespace v3 {
namespace {
struct SsoKey {
  char text[16]{};
  uint64_t size=0,capacity=15;
  explicit SsoKey(const char* value){size=std::strlen(value);std::memcpy(text,value,size);}
};
static_assert(sizeof(SsoKey)==32);
using Sort=void(__fastcall*)(void*,const SsoKey*);
using Quantity=int64_t*(__fastcall*)(void*,int64_t*);
using Find=void*(__fastcall*)(void*,const SsoKey*);
using Register=void(__fastcall*)(void*,const SsoKey*,void*);
using Clone=void*(__fastcall*)(void*,void*);
using Refresh=void(__fastcall*)(void*);
Sort originalSort=nullptr;
Quantity originalMarketQuantity=nullptr,originalStateQuantity=nullptr;
Find findSort=nullptr;Register registerSort=nullptr;
Refresh refreshSort=nullptr;
uint8_t* module=nullptr;
std::atomic<std::shared_ptr<const NativeLogisticsSortValues>> published;
std::atomic<bool> refreshPending=false;
struct Scope {
  FreightViewKind kind;
  uint32_t id;
  std::shared_ptr<const NativeLogisticsSortValues> values;
};
thread_local const Scope* active=nullptr;
bool Read(uintptr_t address,void* value,size_t size){
  SIZE_T copied=0;
  return address>=0x10000 && size<=UINTPTR_MAX-address &&
      ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(address),value,size,&copied) && copied==size;
}
template<class T> bool Read(uintptr_t address,T& value){return Read(address,&value,sizeof(value));}
bool CustomKey(const SsoKey* source){
  SsoKey key("");
  if(!Read(reinterpret_cast<uintptr_t>(source),key) || (key.size!=13 && key.size!=14) ||
      key.capacity<key.size || key.capacity>4096 || (key.capacity<16 && key.capacity!=15))return false;
  char text[15]{};
  if(key.capacity==15)std::memcpy(text,key.text,key.size+1);
  else {
    uintptr_t data=0;std::memcpy(&data,key.text,sizeof(data));
    if(!Read(data,text,key.size+1))return false;
  }
  return (key.size==13 && !std::memcmp(text,"ironroad_work",14)) ||
      (key.size==14 && !std::memcmp(text,"ironroad_share",15));
}
bool Owner(uintptr_t sorter,Scope& scope){
  uintptr_t type=0;const auto base=reinterpret_cast<uintptr_t>(module);
  if(sorter>=0x198 && Read(sorter-0x198,type) && type==base+kRvaGuiMarketVtable){
    scope.kind=FreightViewKind::Market;return Read(sorter-0x198+0x10,scope.id) && scope.id!=UINT32_MAX;
  }
  if(sorter>=0xd8 && Read(sorter-0xd8,type) && type==base+0x0485db38){
    scope.kind=FreightViewKind::State;return Read(sorter-0xd8+0x70,scope.id) && scope.id!=UINT32_MAX;
  }
  return false;
}
bool Good(uintptr_t row,const Scope& scope,uint32_t& id){
  if(row<0x10000 || row>UINTPTR_MAX-0x20)return false;
  uintptr_t type=0,definition=0;uint32_t state=0,market=0;
  const auto base=reinterpret_cast<uintptr_t>(module);
  return Read(row,type) && type==base+kRvaGuiGoodsVtable && Read(row+0x10,definition) &&
      definition<=UINTPTR_MAX-0x14 && Read(row+0x18,state) && Read(row+0x1c,market) &&
      (scope.kind==FreightViewKind::Market?state==UINT32_MAX && market==scope.id:state==scope.id) &&
      Read(definition,type) && type==base+kRvaGoodVtable && Read(definition+0x10,id) && id<128;
}
bool CurrentRows(uintptr_t sorter,const Scope& scope,std::vector<uint32_t>* order=nullptr){
  uintptr_t vector=0,rows=0;uint32_t capacity=0,count=0;
  if(!Read(sorter,vector) || vector>UINTPTR_MAX-16 || !Read(vector,rows) || !Read(vector+8,capacity) || !Read(vector+12,count) ||
      count>128 || capacity<count || rows>UINTPTR_MAX-count*sizeof(uintptr_t))return false;
  std::array<bool,128> seen{};
  for(uint32_t i=0;i<count;++i){
    uintptr_t row=0;uint32_t id=0;
    if(!Read(rows+i*sizeof(row),row) || !Good(row,scope,id) || seen[id] || scope.values->quantities[id]<0)return false;
    seen[id]=true;
    if(order)order->push_back(id);
  }
  return true;
}
uintptr_t CurrentSorter(const NativeLogisticsSortValues& values){
  const auto base=reinterpret_cast<uintptr_t>(module);
  const GoodsMemoryReader reader=[](uint64_t address,void* out,size_t size){return Read(address,out,size);};
  if(values.kind==FreightViewKind::Market){
    const auto panel=DecodeNativeMarketPanel(reader,base);
    if(!panel.available || !panel.marketOpen || panel.market!=values.market || panel.tab!="ironroad_logistics")return 0;
    uintptr_t registry=0,entries=0,match=0;uint32_t count=0,capacity=0,lock=0;
    if(!Read(base+kRvaGuiMarketRegistry,registry) || !Read(registry,entries) || !Read(registry+8,capacity) ||
        !Read(registry+12,count) || count>2048 || capacity<count || !Read(registry+0x18,lock) || (lock&1))return 0;
    for(uint32_t i=0;i<count;++i){
      uintptr_t object=0,type=0;uint32_t id=0;
      if(!Read(entries+i*8,object) || !Read(object,type) || type!=base+kRvaGuiMarketVtable || !Read(object+0x10,id))return 0;
      if(id==values.market){if(match)return 0;match=object;}
    }
    return match?match+0x198:0;
  }
  const auto panel=DecodeNativeStatePanel(reader,base);
  if(!panel.available || !panel.stateOpen || panel.state!=values.state || panel.tab!="local_goods")return 0;
  uintptr_t idler=0,gfx=0,handler=0,panels=0,object=0;
  if(!Read(base+kRvaInterfaceIdlerGlobal,idler) || !Read(idler+0x10,gfx) || !Read(gfx+0x20,handler) ||
      !Read(handler+0xf0,panels) || !Read(panels+14*8,object))return 0;
  return object+0xd8;
}
bool RefreshCurrentSorter(uintptr_t sorter,const std::shared_ptr<const NativeLogisticsSortValues>& values){
  if(!sorter || !refreshSort)return false;
  // The original refresh preserves the selected key and its direction.
  if(!CustomKey(reinterpret_cast<const SsoKey*>(sorter+8)))return true;
  Scope scope{};scope.values=values;
  if(!values || !Owner(sorter,scope) || scope.kind!=values->kind ||
      scope.id!=(scope.kind==FreightViewKind::Market?values->market:values->state) || !CurrentRows(sorter,scope))return false;
  refreshSort(reinterpret_cast<void*>(sorter));return true;
}
bool Alias(void* sorter,const SsoKey* key){
  auto* registry=static_cast<uint8_t*>(sorter)+0x50;
  if(findSort(registry,key))return true;
  const SsoKey balance("balance");
  auto* value=static_cast<uint8_t*>(findSort(registry,&balance));uintptr_t object=0,table=0,copy=0;
  if(!value || !Read(reinterpret_cast<uintptr_t>(value)+0x38,object) || !Read(object,table) ||
      table!=reinterpret_cast<uintptr_t>(module)+0x047b6938 || !Read(table,copy))return false;
  // Native std::function cloning/registration owns every allocator and release.
  struct Function {
    alignas(8) std::array<uintptr_t,8> data{};
    ~Function(){if(data[7]){
      auto* object=reinterpret_cast<void*>(data[7]);auto** table=*reinterpret_cast<void***>(object);
      reinterpret_cast<void(__fastcall*)(void*,bool)>(table[4])(object,object!=data.data());
    }}
  } temporary;
  temporary.data[7]=reinterpret_cast<uintptr_t>(reinterpret_cast<Clone>(copy)(reinterpret_cast<void*>(object),temporary.data.data()));
  if(!temporary.data[7])return false;
  registerSort(sorter,key,temporary.data.data());
  if(temporary.data[7])return false;
  return findSort(registry,key)!=nullptr;
}
void __fastcall SortThunk(void* sorter,const SsoKey* key){
  struct Restore {const Scope* previous;~Restore(){active=previous;}} restore{active};
  active=nullptr;
  Scope scope{};
  if(!CustomKey(key) || !Owner(reinterpret_cast<uintptr_t>(sorter),scope) || !Alias(sorter,key)){
    originalSort(sorter,key);return;
  }
  scope.values=published.load();
  if(scope.values && (scope.values->kind!=scope.kind ||
      (scope.kind==FreightViewKind::Market?scope.values->market:scope.values->state)!=scope.id ||
      !CurrentRows(reinterpret_cast<uintptr_t>(sorter),scope)))scope.values.reset();
  active=&scope;
  originalSort(sorter,key);
  static std::atomic<unsigned> observations{0};
  if(observations.fetch_add(1)<16){
    std::vector<uint32_t> order;
    const bool ready=scope.values && CurrentRows(reinterpret_cast<uintptr_t>(sorter),scope,&order);
    const auto value=[&](uint32_t id){return scope.values->quantities[id];};
    const bool ascending=ready && std::is_sorted(order.begin(),order.end(),[&](auto a,auto b){return value(a)<value(b);});
    const bool descending=ready && std::is_sorted(order.begin(),order.end(),[&](auto a,auto b){return value(a)>value(b);});
    V3_INFO("native logistics sort kind=%u id=%u ready=%d rows=%zu ascending=%d descending=%d world=%llu revision=%llu",
        unsigned(scope.kind),scope.id,int(ready),order.size(),int(ascending),int(descending),
        (unsigned long long)(ready?scope.values->world:0),(unsigned long long)(ready?scope.values->revision:0));
  }
}
int64_t* QuantityValue(void* row,int64_t* output,Quantity original){
  if(!active)return original(row,output);
  uint32_t id=0;
  // Unready payloads compare equal; the formal labels keep their own -- gate.
  if(output)*output=active->values && Good(reinterpret_cast<uintptr_t>(row),*active,id)?
      active->values->quantities[id]:0;
  return output;
}
int64_t* __fastcall MarketQuantity(void* row,int64_t* output){return QuantityValue(row,output,originalMarketQuantity);}
int64_t* __fastcall StateQuantity(void* row,int64_t* output){return QuantityValue(row,output,originalStateQuantity);}
} // namespace
void PublishNativeLogisticsSort(const LogisticsDocument* document){
  auto values=CopyNativeLogisticsSortValues(document);
  published.store(values?std::make_shared<const NativeLogisticsSortValues>(std::move(*values)):nullptr);
  refreshPending.store(values.has_value());
}
void RefreshNativeLogisticsSort(){
  if(!module || !refreshPending.exchange(false))return;
  const auto values=published.load();
  if(values && !RefreshCurrentSorter(CurrentSorter(*values),values) && published.load()==values)refreshPending.store(true);
}
bool InstallNativeLogisticsSort(uint8_t* base,std::string* error){
  const auto fail=[&](const char* why){if(error)*error=why;return false;};
  if(!base || !Game::Instance().Ready() || Game::Instance().Base()!=base)return fail("native sort profile unavailable");
  struct Entry {uint32_t rva;unsigned char bytes[12];};
  static constexpr Entry entries[]={
      {0x019d4c60,{0x48,0x89,0x5c,0x24,0x10,0x48,0x89,0x74,0x24,0x18,0x48,0x89}},
      {0x019d4ef0,{0x48,0x89,0x5c,0x24,0x10,0x48,0x89,0x74,0x24,0x18,0x57,0x48}},
      {0x019d8490,{0x48,0x89,0x5c,0x24,0x08,0x48,0x89,0x74,0x24,0x10,0x4c,0x89}},
      {0x019e2650,{0x48,0x89,0x5c,0x24,0x10,0x48,0x89,0x74,0x24,0x18,0x57,0x41}},
      {0x0200f110,{0x48,0x89,0x5c,0x24,0x20,0x56,0x57,0x41,0x56,0x48,0x83,0xec}},
      {0x0200dd50,{0x48,0x89,0x5c,0x24,0x18,0x48,0x89,0x6c,0x24,0x20,0x56,0x57}}};
  for(const auto& entry:entries)if(std::memcmp(base+entry.rva,entry.bytes,sizeof(entry.bytes)))return fail("native sort entry signature mismatch");
  module=base;findSort=reinterpret_cast<Find>(base+0x019e2650);registerSort=reinterpret_cast<Register>(base+0x019d8490);
  refreshSort=reinterpret_cast<Refresh>(base+0x019d4ef0);
  void* targets[]={base+0x019d4c60,base+0x0200f110,base+0x0200dd50};
  void* thunks[]={reinterpret_cast<void*>(&SortThunk),reinterpret_cast<void*>(&MarketQuantity),reinterpret_cast<void*>(&StateQuantity)};
  void** originals[]={reinterpret_cast<void**>(&originalSort),reinterpret_cast<void**>(&originalMarketQuantity),reinterpret_cast<void**>(&originalStateQuantity)};
  size_t created=0;
  for(;created<3;++created)if(MH_CreateHook(targets[created],thunks[created],originals[created])!=MH_OK)break;
  if(created==3){
    bool queued=true;
    for(auto* target:targets)queued=MH_QueueEnableHook(target)==MH_OK && queued;
    if(queued && MH_ApplyQueued()==MH_OK){V3_INFO("native logistics sorting enabled: original selector, direction flags and stable sorter");return true;}
  }
  while(created){auto* target=targets[--created];MH_DisableHook(target);MH_RemoveHook(target);}
  return fail("native sort hooks unavailable");
}
} // namespace v3
