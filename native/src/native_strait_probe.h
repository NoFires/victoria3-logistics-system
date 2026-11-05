#pragma once
#include "goods_catalog.h"
#include "native_lane_access.h"
#include "native_economy.h"
#include <algorithm>
#include <bitset>
#include <optional>
#include <type_traits>

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
// Values observed on the original caller's thread, never a heap/access lease.
struct NativeThreadAllocatorObservation {
  uint32_t index=0;
  uint64_t tls=0,heap=0;
  bool operator==(const NativeThreadAllocatorObservation&)const=default;
};
inline std::optional<NativeThreadAllocatorObservation> ObserveNativeThreadAllocator(
    const GoodsMemoryReader& reader,uint64_t module,uint64_t tlsArray) {
  if(!reader || !module || module>UINT64_MAX-0x058f8c94 || !tlsArray)return {};
  EconomyReadWindow window(reader);NativeThreadAllocatorObservation out;
  auto field=[&](uint64_t p,uint64_t off,auto& value){
    return p && off<=UINT64_MAX-p && window.Read(p+off,&value,sizeof(value));
  };
  uint64_t type=0,allocate=0,release=0;uint8_t initialized=0;
  if(!field(module,0x058f8c90,out.index) || out.index>=4096 ||
     !field(tlsArray,uint64_t(out.index)*8,out.tls) ||
     !field(out.tls,0x44,initialized) || initialized!=1 ||
     !field(out.tls,0x38,out.heap) || !out.heap ||
     !field(module,0x0512cdc0,type) || type!=module+0x04419230 ||
     !field(type,8,allocate) || allocate!=module+0x0061be50 ||
     !field(type,16,release) || release!=module+0x0061be90 || !window.Unchanged())return {};
  return out;
}
struct NativeTradeBufferObservation {
  NativeStraitOutputHeader array;
  uint64_t allocator=0;
  bool operator==(const NativeTradeBufferObservation&)const=default;
};
// Observe only the descriptor. No borrowed data is read or freed later; raw
// addresses must remain within one natural wrapper's stack-local trace.
inline std::optional<NativeTradeBufferObservation> ObserveNativeTradeBuffer(
    const GoodsMemoryReader& reader,uint64_t address) {
  static_assert(sizeof(NativeTradeBufferObservation)==24);
  EconomyReadWindow window(reader);NativeTradeBufferObservation out;
  if(!window.Read(address,&out,sizeof(out)) || !out.allocator || out.array.count>out.array.capacity ||
     out.array.count>4096 || out.array.capacity>65536 ||
     (out.array.capacity && (!out.array.data || uint64_t(out.array.capacity)*4>UINT64_MAX-out.array.data)) ||
     (!out.array.capacity && out.array.data) || !window.Unchanged())return {};
  return out;
}
struct NativeTradeSearchResult {
  uint16_t fromProvince=UINT16_MAX,toProvince=UINT16_MAX;
  int64_t distance=0;
  std::vector<NativeProvinceRange> provinces;
  bool operator==(const NativeTradeSearchResult&)const=default;
};
struct NativePortSearchInputs {
  std::vector<uint32_t> sources,targets;
  bool operator==(const NativePortSearchInputs&)const=default;
};
// Original1414500F0 borrows two arrays of FULL runtime states. Zero is valid;
// their order/duplicates cannot be replaced by province or region identity.
inline std::optional<NativePortSearchInputs> CopyNativePortSearchInputs(
    const GoodsMemoryReader& reader,uint64_t sources,uint64_t targets) {
  if(!reader)return {};
  EconomyReadWindow window(reader);NativePortSearchInputs out;
  auto array=[&](uint64_t address,std::vector<uint32_t>& values){
    std::array<uint8_t,24> bytes{};NativeStraitOutputHeader h;
    if(!window.Read(address,bytes.data(),bytes.size()))return false;
    std::memcpy(&h,bytes.data(),sizeof(h));
    if(!h.count || h.count>h.capacity || h.count>4096 || h.capacity>65536 || !h.data ||
       uint64_t(h.capacity)*4>UINT64_MAX-h.data)return false;
    values.resize(h.count);
    if(!window.Read(h.data,values.data(),values.size()*4))return false;
    return std::find(values.begin(),values.end(),UINT32_MAX)==values.end();
  };
  if(!array(sources,out.sources) || !array(targets,out.targets) || !window.Unchanged())return {};
  return out;
}
struct NativePortSearchSelection {
  uint32_t fromState=UINT32_MAX,toState=UINT32_MAX;
  NativeTradeSearchResult path;
};
// Observe the wrapper's final state outputs BEFORE its caller releases them.
// The caller must witness exactly its nested original search and bind the
// world/request/lifetime separately. Initialized output alone proves no call.
inline std::optional<NativePortSearchSelection> CopyNativePortSearchSelection(
    const GoodsMemoryReader& reader,uint64_t provinceSet,uint64_t fromProvince,uint64_t toProvince,
    uint64_t fromState,uint64_t toState,uint64_t distance,const NativePortSearchInputs& input,
    const NativeTradeSearchResult& nested) {
  if(!reader || input.sources.empty() || input.targets.empty() || input.sources.size()>4096 || input.targets.size()>4096)return {};
  for(const auto* values:{&input.sources,&input.targets})
    if(std::find(values->begin(),values->end(),UINT32_MAX)!=values->end())return {};
  EconomyReadWindow window(reader);NativePortSearchSelection out;
  std::array<uint8_t,24> bytes{};NativeStraitOutputHeader h;
  if(!window.Read(provinceSet,bytes.data(),bytes.size()) ||
     !window.Read(fromProvince,&out.path.fromProvince,2) || !window.Read(toProvince,&out.path.toProvince,2) ||
     !window.Read(fromState,&out.fromState,4) || !window.Read(toState,&out.toState,4) ||
     !window.Read(distance,&out.path.distance,8))return {};
  std::memcpy(&h,bytes.data(),sizeof(h));
  if(h.count>h.capacity || h.count>4096 || h.capacity>65536 || out.path.distance<0 ||
     (h.capacity && (!h.data || uint64_t(h.capacity)*4>UINT64_MAX-h.data)))return {};
  std::vector<std::array<uint16_t,2>> ranges(h.count);std::bitset<65536> members;
  if(h.count && !window.Read(h.data,ranges.data(),ranges.size()*4))return {};
  for(const auto& range:ranges){
    const uint32_t last=uint32_t(range[0])+range[1];
    if(!range[0] || last>=UINT16_MAX)return {};
    for(uint32_t id=range[0];id<=last;++id){if(members[id])return {};members.set(id);}
    out.path.provinces.push_back({range[0],uint16_t(last)});
  }
  if(ranges.empty()){
    if(out.path.fromProvince!=UINT16_MAX || out.path.toProvince!=UINT16_MAX || out.path.distance ||
       out.fromState!=UINT32_MAX || out.toState!=UINT32_MAX)return {};
  }else if(!members[out.path.fromProvince] || !members[out.path.toProvince] ||
      std::find(input.sources.begin(),input.sources.end(),out.fromState)==input.sources.end() ||
      std::find(input.targets.begin(),input.targets.end(),out.toState)==input.targets.end())return {};
  if(out.path!=nested || !window.Unchanged())return {};
  return out;
}
struct NativeTradeSearchInputs {
  std::vector<uint16_t> sources,targets;
  std::vector<uint32_t> forcedStraits;
  std::vector<std::array<uint16_t,2>> blockedPairs;
  uint16_t selector=UINT16_MAX;
  bool operator==(const NativeTradeSearchInputs&)const=default;
};
// Borrowed by the original seven-slot trade search. Preserve both option
// arrays, including duplicates/unknown extra restrictions, without interpreting
// them as registered permissions. No input pointer survives this copy.
inline std::optional<NativeTradeSearchInputs> CopyNativeTradeSearchInputs(
    const GoodsMemoryReader& reader,uint64_t sources,uint64_t targets,uint64_t options) {
  if(!reader || !sources || !targets || !options || options>UINT64_MAX-0x32)return {};
  EconomyReadWindow window(reader);NativeTradeSearchInputs out;
  auto array=[&](uint64_t address,auto& values){
    std::array<uint8_t,24> bytes{};NativeStraitOutputHeader h;
    if(!window.Read(address,bytes.data(),bytes.size()))return false;
    std::memcpy(&h,bytes.data(),sizeof(h));
    const size_t width=sizeof(typename std::decay_t<decltype(values)>::value_type);
    if(h.count>h.capacity || h.count>4096 || h.capacity>65536 ||
       (h.capacity && (!h.data || uint64_t(h.capacity)*width>UINT64_MAX-h.data)))return false;
    values.resize(h.count);
    return values.empty() || window.Read(h.data,values.data(),values.size()*width);
  };
  if(!array(sources,out.sources) || !array(targets,out.targets) ||
     !array(options,out.forcedStraits) || !array(options+0x18,out.blockedPairs) ||
     !window.Read(options+0x30,&out.selector,2))return {};
  for(const auto* values:{&out.sources,&out.targets}){
    if(values->empty())return {};
    for(auto id:*values)if(!id || id==UINT16_MAX)return {};
  }
  if(!window.Unchanged())return {};
  return out;
}
// Diagnostic result of original14141F1D0, not a callable ABI or an access lease.
// Its returned address alone does not indicate reachability. Empty ranges with
// both selected IDsFFFF and zero distance mean a known failed search; unreadable
// or inconsistent data is unknown. Preserve native range order, never sort it
// into a route or infer country/map/request identity from province IDs alone.
inline std::optional<NativeTradeSearchResult> CopyNativeTradeSearchResult(
    const GoodsMemoryReader& read,uint64_t result,const std::vector<uint16_t>& sources,
    const std::vector<uint16_t>& targets) {
  if(!read || !result || result>UINT64_MAX-0x28 || sources.empty() || targets.empty() ||
     sources.size()>4096 || targets.size()>4096)return {};
  for(const auto* values:{&sources,&targets})for(auto id:*values)if(!id || id==UINT16_MAX)return {};
  std::array<uint8_t,0x28> header{},after{};
  if(!read(result,header.data(),header.size()))return {};
  NativeStraitOutputHeader array;NativeTradeSearchResult out;
  std::memcpy(&array,header.data(),sizeof(array));
  std::memcpy(&out.fromProvince,header.data()+0x18,2);std::memcpy(&out.toProvince,header.data()+0x1a,2);
  std::memcpy(&out.distance,header.data()+0x20,8);
  if(array.count>array.capacity || array.count>4096 || array.capacity>65536 || out.distance<0 ||
     (array.capacity && (!array.data || uint64_t(array.capacity)*4>UINT64_MAX-array.data)))return {};
  std::vector<std::array<uint16_t,2>> ranges(array.count),check(array.count);
  if(array.count && !read(array.data,ranges.data(),ranges.size()*4))return {};
  std::bitset<65536> members;
  for(const auto& range:ranges){
    const uint32_t last=uint32_t(range[0])+range[1];
    if(!range[0] || last>=UINT16_MAX)return {};
    for(uint32_t id=range[0];id<=last;++id){if(members[id])return {};members.set(id);}
    out.provinces.push_back({range[0],uint16_t(last)});
  }
  if(ranges.empty()){
    if(out.fromProvince!=UINT16_MAX || out.toProvince!=UINT16_MAX || out.distance)return {};
  } else if(!members[out.fromProvince] || !members[out.toProvince] ||
      std::find(sources.begin(),sources.end(),out.fromProvince)==sources.end() ||
      std::find(targets.begin(),targets.end(),out.toProvince)==targets.end())return {};
  if(!read(result,after.data(),after.size()) || after!=header ||
     (array.count && (!read(array.data,check.data(),check.size()*4) || check!=ranges)))return {};
  return out;
}
} // namespace v3
