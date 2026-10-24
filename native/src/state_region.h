#pragma once
#include "goods_catalog.h"
#include <optional>

namespace v3 {
// Exact 1.13.11 State.GetStateRegion chain: state+0xE78 is a region
// definition. Its engine ID is distinct from both runtime state and map IDs.
inline std::optional<std::string> DecodeNativeStateRegionKey(const GoodsMemoryReader& reader,
    uint64_t module,uint64_t state,uint32_t expectedId) {
  if(!reader || !module || module>UINT64_MAX-0x044E7850 || !state || state>UINT64_MAX-0xe80)return {};
  uint32_t id=0;uint64_t definition=0;
  if(!reader(state+8,&id,4) || id!=expectedId || !reader(state+0xe78,&definition,8) ||
     !definition || definition>UINT64_MAX-0x38)return {};
  std::array<uint8_t,0x38> before{},after{};
  if(!reader(definition,before.data(),before.size()))return {};
  uint64_t type=0,size=0,capacity=0;uint32_t definitionId=0;
  std::memcpy(&type,before.data(),8);std::memcpy(&definitionId,before.data()+0x10,4);
  std::memcpy(&size,before.data()+0x28,8);std::memcpy(&capacity,before.data()+0x30,8);
  if(type!=module+0x044E7850 || definitionId==UINT32_MAX || !size || size>128 || size>capacity ||
     (capacity<16 && capacity!=15))return {};
  std::string key(size+1,'\0');uint64_t heap=0;
  if(capacity<16)std::memcpy(key.data(),before.data()+0x18,key.size());
  else{std::memcpy(&heap,before.data()+0x18,8);
    if(!heap || key.size()>UINT64_MAX-heap || !reader(heap,key.data(),key.size()))return {};}
  if(key.back()!=0)return {};
  for(size_t i=0;i<size;++i){char c=key[i];
    if(!((c>='A'&&c<='Z')||(c>='a'&&c<='z')||(c>='0'&&c<='9')||c=='_'))return {};}
  uint64_t definitionAfter=0;uint32_t idAfter=0;
  if(!reader(state+8,&idAfter,4) || idAfter!=id || !reader(state+0xe78,&definitionAfter,8) ||
     definitionAfter!=definition || !reader(definition,after.data(),after.size()) || before!=after)return {};
  if(heap){std::string copy(key.size(),'\0');if(!reader(heap,copy.data(),copy.size())||copy!=key)return {};}
  key.pop_back();return key;
}
} // namespace v3
