#pragma once
#include "goods_catalog.h"
#include "offsets.h"

namespace v3 {
struct NativeStatePanelState {
  bool available=false, ingame=false, stateOpen=false;
  uint32_t state=UINT32_MAX;
  std::string tab, error;
};
// Static 1.13.11 candidate only: no runtime caller until live ownership and
// state-ID correspondence are verified. Reads owned copies, never calls getters.
inline NativeStatePanelState DecodeNativeStatePanel(const GoodsMemoryReader& reader,uint64_t module) {
  NativeStatePanelState result;
  auto fail=[](const char* why){return NativeStatePanelState{false,false,false,UINT32_MAX,{},why};};
  constexpr uint32_t stateVtable=0x045CC848;
  if(!reader || !module || module>UINT64_MAX-kRvaInterfaceIdlerGlobal-8)
    return fail("invalid interface module or reader");
  struct Copy {uint64_t address;std::vector<uint8_t> bytes;};
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
    if(!object || object>UINT64_MAX-offset || !read(object+offset,bytes.data(),bytes.size()))return false;
    uint64_t size=0,capacity=0;
    std::memcpy(&size,bytes.data()+16,8);std::memcpy(&capacity,bytes.data()+24,8);
    if(!size || size>128 || size>capacity || (capacity<16 && capacity!=15))return false;
    std::string key(size+1,'\0');
    if(capacity<16)std::memcpy(key.data(),bytes.data(),key.size());
    else{uint64_t p=0;std::memcpy(&p,bytes.data(),8);if(!read(p,key.data(),key.size()))return false;}
    if(key.back()!=0)return false;
    key.pop_back();
    for(char c:key)if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_'))return false;
    out=std::move(key);return true;
  };
  auto stable=[&](){
    for(const auto& c:copies){std::vector<uint8_t> after(c.bytes.size());
      if(!reader(c.address,after.data(),after.size()) || after!=c.bytes)return false;}
    return true;
  };
  uint64_t idler=0,machine=0,gfx=0,handler=0,bar=0,panels=0,panel=0,history=0;
  int32_t phase=0,count=0,historyCount=0;uint8_t closed=0;
  if(!field(module,kRvaInterfaceIdlerGlobal,idler) || !type(idler,kRvaInterfaceIdlerVtable) ||
     !field(idler,0x18,machine) || !field(machine,0x588,phase))return fail("interface identity unavailable");
  if(phase!=4){
    if(!stable())return fail("interface changed during copy");
    result.available=true;return result;
  }
  if(!field(idler,0x10,gfx) || !type(gfx,kRvaInterfaceGfxVtable) ||
     !field(gfx,0x20,handler) || !type(handler,kRvaInterfaceHandlerVtable) ||
     !field(handler,0xd8,bar) || bar!=handler+0xd8 ||
     !field(handler,0xe0,panels) || !field(handler,0xec,count) || count!=56 ||
     !field(panels,14*8,panel) || !type(panel,stateVtable))return fail("state panel ownership unavailable");
  std::string name;
  if(!text(panel,8,name) || name!="states_panel" || !text(panel,0x30,result.tab) ||
     !field(panel,0x70,result.state) || !field(bar,0x2c,historyCount) ||
     historyCount<0 || historyCount>1000000 || !field(bar,0x40,closed) || closed>1)
    return fail("invalid state panel state");
  if(historyCount>0 && !closed){
    int32_t kind=0;
    if(!field(bar,0x20,history) || !field(history,uint64_t(historyCount-1)*0x30,kind) || kind<0 || kind>=56)
      return fail("invalid panel history");
    result.stateOpen=kind==14;
  }
  if(result.stateOpen && result.state==UINT32_MAX)return fail("open state has no identity");
  if(!stable())return fail("state panel changed during copy");
  result.available=result.ingame=true;return result;
}
} // namespace v3
