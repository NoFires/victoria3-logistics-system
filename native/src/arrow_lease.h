#pragma once
#include "goods_catalog.h"
#include <map>
#include <unordered_set>

namespace v3 {
// Render-owned storage from a successful upload. This is never sent to a worker.
struct NativeArrowLease {
  std::string style;
  uint64_t arrow=0,type=0,path=0,points=0,vertex=0,constant=0;
};

// R92: exact-build resource registration survives a normal world replacement.
// Check current asset identity AND callback membership before reusing that pool.
// Equal copies detect observed changes; the caller still owns the render window.
inline bool ValidateNativeArrowLeases(const GoodsMemoryReader& reader,uint64_t module,
    uint64_t graphics,const std::vector<NativeArrowLease>& leases,
    const std::function<bool()>& current,std::string* error) {
  auto fail=[&](const char* reason){if(error)*error=reason;return false;};
  if(!reader || !current || !current() || !module || module>UINT64_MAX-0x0592d5c8 ||
     !graphics || leases.size()>8192)return fail("missing native recovery identity");
  struct Copy {uint64_t address;std::vector<uint8_t> bytes;};
  std::vector<Copy> copies;
  auto read=[&](uint64_t address,void* out,size_t size){
    if(!address || !size || size>UINT64_MAX-address || !reader(address,out,size))return false;
    copies.push_back({address,{(uint8_t*)out,(uint8_t*)out+size}});return true;
  };
  auto field=[&](uint64_t object,uint64_t offset,auto& value){
    return object && offset<=UINT64_MAX-object && read(object+offset,&value,sizeof(value));
  };
  uint64_t liveGraphics=0;
  if(!field(module,0x0592d5c8,liveGraphics) || liveGraphics!=graphics)
    return fail("native graphics identity changed");
  std::map<std::string,uint64_t> styles;
  std::unordered_set<uint64_t> arrows;
  for(const auto& lease:leases){
    if(!lease.style.starts_with("ironroad") || lease.style.size()>128 || !lease.arrow ||
       !lease.type || !lease.path || !lease.points || !lease.vertex || !lease.constant ||
       !arrows.insert(lease.arrow).second)return fail("invalid native pool lease");
    auto [it,inserted]=styles.emplace(lease.style,lease.type);
    if(!inserted && it->second!=lease.type)return fail("ambiguous native pool style");
  }
  if(!styles.empty()){
    uint64_t entries=0;uint32_t mask=0;uint8_t initialized=0,tail=0;
    const uint64_t map=module+0x0513f268;
    if(!field(module,0x0513f2b8,initialized) || initialized!=1 ||
       !field(map,0x30,entries) || !field(map,0x3c,mask) || !field(map,0x40,tail) ||
       mask>65535 || (mask&(mask+1)) ||
       entries>UINT64_MAX-uint64_t(mask+tail+2)*0x30)
      return fail("native arrow asset map unavailable");
    std::unordered_set<std::string> found;
    for(uint32_t i=0;i<=mask+tail+1;++i){
      std::array<uint8_t,0x30> item{};
      if(!read(entries+uint64_t(i)*item.size(),item.data(),item.size()))
        return fail("unreadable native arrow asset entry");
      if(!item[4] || item[4]==255)continue;
      uint64_t size=0,capacity=0,data=0,type=0;
      std::memcpy(&size,item.data()+0x18,8);std::memcpy(&capacity,item.data()+0x20,8);
      if(!size || size>128 || size>capacity || (capacity<16 && capacity!=15))
        return fail("invalid native arrow asset key");
      std::string key(size+1,'\0');
      if(capacity<16)std::memcpy(key.data(),item.data()+8,key.size());
      else{std::memcpy(&data,item.data()+8,8);
        if(!read(data,key.data(),key.size()))return fail("unreadable native arrow asset key");}
      if(key.back())return fail("unterminated native arrow asset key");
      key.pop_back();
      if(!styles.count(key))continue;
      std::memcpy(&type,item.data()+0x28,8);
      if(type!=styles.at(key) || !found.insert(key).second)
        return fail("native arrow asset identity changed");
    }
    if(found.size()!=styles.size())return fail("native arrow asset removed");
  }
  std::map<uint64_t,std::unordered_set<uint64_t>> registered;
  size_t total=0;
  for(const auto& [style,type]:styles){
    uint64_t vt=0,entries=0,allocator=0;uint32_t capacity=0,count=0;
    if(!field(type,0,vt) || vt!=module+0x0481daa8 || !field(type,0x38,entries) ||
       !field(type,0x40,capacity) || !field(type,0x44,count) || !field(type,0x48,allocator) ||
       count>capacity || capacity>16384 || !count || total+count>8192 ||
       allocator!=module+0x0512cdc0)return fail("native arrow registration unavailable");
    total+=count;
    std::vector<uint8_t> items(size_t(count)*0x48);
    if(!read(entries,items.data(),items.size()))return fail("unreadable native arrow registration");
    auto& members=registered[type];
    for(uint32_t i=0;i<count;++i){uint64_t arrow=0;std::memcpy(&arrow,items.data()+size_t(i)*0x48,8);
      if(!arrow || !members.insert(arrow).second)return fail("ambiguous native arrow registration");}
  }
  for(const auto& lease:leases){
    uint64_t vt=0,type=0,path=0,vertex=0,constant=0,points=0,allocator=0;
    uint32_t capacity=0,count=0;
    if(!registered[lease.type].count(lease.arrow) || !field(lease.arrow,0,vt) ||
       vt!=module+0x046588e8 || !field(lease.arrow,0x50,type) || type!=lease.type ||
       !field(lease.arrow,0x70,path) || path!=lease.path ||
       !field(lease.arrow,0x68,vertex) || vertex!=lease.vertex ||
       !field(lease.arrow,0x60,constant) || constant!=lease.constant ||
       !field(path,0x10,points) || points!=lease.points || !field(path,0x18,capacity) ||
       !field(path,0x1c,count) || capacity!=4096 || count<4 || count>capacity ||
       !field(path,0x20,allocator) || allocator!=module+0x0512cdc0)
      return fail("native arrow object or retained resources changed");
  }
  for(const auto& copy:copies){std::vector<uint8_t> after(copy.bytes.size());
    if(!reader(copy.address,after.data(),after.size()) || after!=copy.bytes)
      return fail("native pool changed during recovery copy");}
  if(!current())return fail("world or request changed during native recovery");
  if(error)error->clear();return true;
}
} // namespace v3
