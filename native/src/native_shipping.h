#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace v3 {
inline constexpr int64_t kNativeShippingDistanceScale=100000;
struct NativeProvinceRange {uint16_t first=0,last=0;bool operator==(const NativeProvinceRange&)const=default;};
struct NativeShippingLane {
  uint32_t id=0,country=0,kind=0,treaty=UINT32_MAX;
  uint32_t fromState=0,toState=0;
  uint16_t extraProvince=UINT16_MAX;
  // +58 is GetMerchantMarineDemandRaw, at 100000 fixed units per unit.
  // +60 is original travel distance, forwarded by0131A590 to01419540/
  // 0141AE70 and SHIPPING_LANE_BREAKDOWN_TRAVEL_DISTANCE, fixed1e5. Not km.
  int64_t merchantMarineDemandFixed=0,travelDistanceFixed=0;
  // Sorted inclusive membership ranges. They do not encode travel order.
  std::vector<NativeProvinceRange> provinces;
  bool operator==(const NativeShippingLane&)const=default;
};
struct NativeShippingSnapshot {std::vector<NativeShippingLane> lanes;};

// Exact 1.13.11 typed database and CShippingLane layout. Call inside the
// verified game-access scope; stable rereads alone do not grant that scope.
// Result owns numeric data only. No native getter, pathfinder or highlight call.
template<class Reader,class Current>
std::optional<NativeShippingSnapshot> DecodeNativeShipping(const Reader& reader,uint64_t module,
    const Current& current,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<NativeShippingSnapshot>{if(error)*error=why;return {};};
  if(!current() || !module || module>UINT64_MAX-0x05966768)return fail("missing current shipping identity");
  struct Copy {uint64_t address;std::vector<uint8_t> bytes;};
  std::vector<Copy> copies;size_t total=0;
  auto read=[&](uint64_t address,void* out,size_t size){
    if(!address || !size || size>UINT64_MAX-address || size>16*1024*1024-total || !reader(address,out,size))return false;
    total+=size;copies.push_back({address,{(uint8_t*)out,(uint8_t*)out+size}});return true;
  };
  auto field=[&](uint64_t p,uint64_t off,auto& value){return p && off<=UINT64_MAX-p && read(p+off,&value,sizeof(value));};
  auto u64=[](const auto& bytes,size_t at){uint64_t n=0;std::memcpy(&n,bytes.data()+at,8);return n;};
  auto u32=[](const auto& bytes,size_t at){uint32_t n=0;std::memcpy(&n,bytes.data()+at,4);return n;};
  auto u16=[](const auto& bytes,size_t at){uint16_t n=0;std::memcpy(&n,bytes.data()+at,2);return n;};
  uint64_t db=0,type=0,entries=0;uint32_t slots=0,active=0;
  if(!field(module,0x05966760,db) || !field(db,0,type) || type!=module+0x04439410 ||
     !field(db,0x20,entries) || !field(db,0x2c,slots) || !field(db,0x3c,active) ||
     slots>131072 || active>slots)return fail("invalid shipping database type or bounds");
  std::vector<uint64_t> roster(size_t(slots)*2);
  if(slots && !read(entries,roster.data(),roster.size()*8))return fail("unreadable shipping roster");
  NativeShippingSnapshot result;std::unordered_set<uint64_t> seen;
  for(uint32_t slot=0;slot<slots;++slot){
    const uint64_t object=roster[size_t(slot)*2+1];if(!object)continue;
    std::array<uint8_t,0x68> row{};
    if(!seen.insert(object).second || !read(object,row.data(),row.size()) ||
       u64(row,0)!=module+0x04427d98 || u64(row,0x10)!=module+0x04427d60 ||
       u32(row,8)==UINT32_MAX || (u32(row,8)&0xffffff)!=slot)
      return fail("invalid shipping lane type or full handle");
    NativeShippingLane lane;lane.id=u32(row,8);lane.country=u32(row,0x18);lane.kind=u32(row,0x1c);
    lane.fromState=u32(row,0x48);lane.toState=u32(row,0x4c);lane.extraProvince=u16(row,0x50);
    std::memcpy(&lane.merchantMarineDemandFixed,row.data()+0x58,8);std::memcpy(&lane.travelDistanceFixed,row.data()+0x60,8);
    if(lane.travelDistanceFixed<0)return fail("invalid native shipping travel distance");
    if(lane.kind>1 || lane.country==UINT32_MAX || lane.fromState==UINT32_MAX ||
       lane.toState==UINT32_MAX || lane.fromState==lane.toState)return fail("invalid shipping lane identity");
    // Native names distinguish port_connection (0) and goods_transfer (1).
    // Goods transfer refers to CTreaty, whose articles carry goods/amounts.
    if(lane.kind==1){
      if(u16(row,0x20)!=0x65 || u32(row,0x28)==UINT32_MAX)return fail("invalid goods transfer treaty reference");
      lane.treaty=u32(row,0x28);
    }
    const uint32_t capacity=u32(row,0x38),count=u32(row,0x3c);
    if(count>capacity || capacity>65534)return fail("invalid shipping province set bounds");
    std::vector<std::array<uint16_t,2>> ranges(count);
    if(count && !read(u64(row,0x30),ranges.data(),ranges.size()*4))return fail("unreadable shipping province set");
    uint32_t previous=0;
    for(const auto& range:ranges){
      const uint32_t first=range[0],last=first+range[1];
      if(!first || last>=UINT16_MAX || first<=previous)return fail("invalid shipping province range");
      lane.provinces.push_back({uint16_t(first),uint16_t(last)});previous=last;
    }
    result.lanes.push_back(std::move(lane));
  }
  if(result.lanes.size()!=active)return fail("shipping roster active count mismatch");
  for(const auto& copy:copies){std::vector<uint8_t> bytes(copy.bytes.size());
    if(!reader(copy.address,bytes.data(),bytes.size()) || bytes!=copy.bytes)return fail("shipping data changed during copy");
  }
  if(!current())return fail("retired shipping snapshot");
  if(error)error->clear();return result;
}
} // namespace v3
