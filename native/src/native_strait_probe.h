#pragma once
#include "goods_catalog.h"
#include "native_shipping.h"
#include <optional>

namespace v3 {
// Borrowed classifier outputs are readable only during its caller's lifetime.
// Copying them does not establish a world epoch or authorize a later native call.
struct NativeStraitOutputHeader {
  uint64_t data=0;
  uint32_t capacity=0,count=0;
  bool operator==(const NativeStraitOutputHeader&) const=default;
};
inline std::optional<NativeStraitOutputHeader> ReadNativeStraitOutputHeader(
    const GoodsMemoryReader& read,uint64_t address) {
  NativeStraitOutputHeader h;
  if(!read || !address || sizeof(h)>UINT64_MAX-address || !read(address,&h,sizeof(h)) ||
     h.count>h.capacity || h.capacity>65536 || h.count>4096 ||
     (h.capacity && (!h.data || uint64_t(h.capacity)*4>UINT64_MAX-h.data)))return {};
  return h;
}
struct NativeStraitClassification {
  std::vector<uint32_t> allowed,denied;
  bool operator==(const NativeStraitClassification&) const=default;
};
inline std::optional<NativeStraitClassification> CopyNativeStraitClassification(
    const GoodsMemoryReader& read,uint64_t allowed,uint64_t denied,
    uint32_t allowedPrefix,uint32_t deniedPrefix) {
  const auto a=ReadNativeStraitOutputHeader(read,allowed),d=ReadNativeStraitOutputHeader(read,denied);
  if(!a || !d || allowedPrefix>a->count || deniedPrefix>d->count ||
     uint64_t(a->count-allowedPrefix)+d->count-deniedPrefix>4096)return {};
  NativeStraitClassification out;
  auto copy=[&](const auto& h,uint32_t prefix,auto& values){
    values.resize(h.count-prefix);
    return values.empty() || read(h.data+uint64_t(prefix)*4,values.data(),values.size()*4);
  };
  if(!copy(*a,allowedPrefix,out.allowed) || !copy(*d,deniedPrefix,out.denied))return {};
  std::set<uint32_t> unique;
  for(const auto* values:{&out.allowed,&out.denied})for(auto id:*values)
    if(id==UINT32_MAX || !unique.insert(id).second)return {};
  // Reread both descriptors and copied suffixes. This rejects observed changes;
  // it is not an atomic snapshot or an engine-access lease.
  if(ReadNativeStraitOutputHeader(read,allowed)!=a || ReadNativeStraitOutputHeader(read,denied)!=d)return {};
  std::vector<uint32_t> check;
  if(!copy(*a,allowedPrefix,check) || check!=out.allowed ||
     !copy(*d,deniedPrefix,check) || check!=out.denied)return {};
  return out;
}
struct NativeLaneSelection {
  uint32_t country=UINT32_MAX,fromState=UINT32_MAX,toState=UINT32_MAX;
  int64_t distance=0;
  std::vector<NativeProvinceRange> provinces;
  bool operator==(const NativeLaneSelection&) const=default;
};
// Copy the original constructor's already selected inputs, or the matching
// fields of its returned lane. Neither form performs a lookup or selects a path.
inline std::optional<NativeLaneSelection> CopyNativeLaneSelection(
    const GoodsMemoryReader& read,uint64_t country,uint64_t from,uint64_t to,
    uint64_t provinceSet,uint64_t distance) {
  if(!read)return {};
  NativeLaneSelection out;NativeStraitOutputHeader h;
  auto copy=[&](uint64_t a,void* p,size_t n){return a && n<=UINT64_MAX-a && read(a,p,n);};
  if(!copy(country,&out.country,4) || !copy(from,&out.fromState,4) || !copy(to,&out.toState,4) ||
     !copy(distance,&out.distance,8) || !copy(provinceSet,&h,sizeof(h)) ||
     out.country==UINT32_MAX || out.fromState==UINT32_MAX || out.toState==UINT32_MAX ||
     out.fromState==out.toState || out.distance<0 || !h.count || h.count>h.capacity || h.capacity>65534 ||
     !h.data || uint64_t(h.capacity)*4>UINT64_MAX-h.data)return {};
  std::vector<std::array<uint16_t,2>> ranges(h.count),check(h.count);
  if(!copy(h.data,ranges.data(),ranges.size()*4))return {};
  uint32_t previous=0;
  for(const auto& range:ranges){
    const uint32_t last=uint32_t(range[0])+range[1];
    if(!range[0] || range[0]<=previous || last>=UINT16_MAX)return {};
    out.provinces.push_back({range[0],uint16_t(last)});previous=last;
  }
  NativeStraitOutputHeader after;NativeLaneSelection identities;
  if(!copy(country,&identities.country,4) || !copy(from,&identities.fromState,4) ||
     !copy(to,&identities.toState,4) || !copy(distance,&identities.distance,8) ||
     !copy(provinceSet,&after,sizeof(after)) || after!=h ||
     identities.country!=out.country || identities.fromState!=out.fromState ||
     identities.toState!=out.toState || identities.distance!=out.distance ||
     !copy(h.data,check.data(),check.size()*4) || check!=ranges)return {};
  return out;
}
} // namespace v3
