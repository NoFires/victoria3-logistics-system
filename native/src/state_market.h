#pragma once
#include "goods_catalog.h"
#include <optional>

namespace v3 {
// Exact 1.13.11 State.GetMarket: use its owner or explicit market-area branch,
// resolve the native market ID, then validate that typed market object. The
// state+0x48 grouping is unrelated to this getter and must not select a market.
inline std::optional<uint32_t> DecodeNativeStateMarket(const GoodsMemoryReader& reader,
    uint64_t module,uint64_t state,uint32_t expectedId) {
  if(!reader || !module || module>UINT64_MAX-0x05964ef0 || !state || expectedId==UINT32_MAX)return {};
  struct Copy {uint64_t address;std::vector<uint8_t> bytes;};
  std::vector<Copy> copies;
  auto read=[&](uint64_t address,void* out,size_t size){
    if(!address || size>UINT64_MAX-address || !reader(address,out,size))return false;
    copies.push_back({address,std::vector<uint8_t>((uint8_t*)out,(uint8_t*)out+size)});return true;
  };
  auto field=[&](uint64_t object,uint64_t offset,auto& value){
    return object && offset<=UINT64_MAX-object && read(object+offset,&value,sizeof(value));
  };
  auto type=[&](uint64_t object,uint32_t rva){uint64_t vt=0;return field(object,0,vt) && vt==module+rva;};
  auto lookup=[&](uint32_t global,uint32_t id,uint32_t idOffset,uint32_t vtable)->uint64_t{
    uint64_t db=0,entries=0,object=0;uint32_t capacity=0,actual=UINT32_MAX;
    if(id==UINT32_MAX || !field(module,global,db) || !field(db,0x20,entries) ||
       !field(db,0x2c,capacity) || capacity>1000000 || (id&0xffffff)>=capacity ||
       !field(entries,8+uint64_t(id&0xffffff)*0x10,object) || !type(object,vtable) ||
       !field(object,idOffset,actual) || actual!=id)return 0;
    return object;
  };
  uint32_t id=UINT32_MAX,specialId=UINT32_MAX,market=UINT32_MAX;uint8_t special=0;
  if(!type(state,0x044de308) || !field(state,8,id) || id!=expectedId ||
     !field(state,0x11b8,special) || special>1 || !field(state,0xb48,specialId))return {};
  if(special && specialId!=UINT32_MAX){
    const auto area=lookup(0x05964ea0,specialId,8,0x04430cd8);
    if(!area || !field(area,0x10,market))return {};
  }else{
    uint32_t owner=UINT32_MAX;
    if(!field(state,0xe48,owner))return {};
    const auto country=lookup(0x05964e20,owner,0x18,0x04494cb8);
    if(!country || !field(country,0xa24,market))return {};
  }
  if(!lookup(0x05964ed8,market,8,0x0449c648))return {};
  // Reject ownership/database/handle changes observed during this bounded copy.
  for(const auto& copy:copies){std::vector<uint8_t> after(copy.bytes.size());
    if(!reader(copy.address,after.data(),after.size()) || after!=copy.bytes)return {};}
  return market;
}
} // namespace v3
