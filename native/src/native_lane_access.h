#pragma once
#include "native_shipping.h"
#include <algorithm>
#include <map>
#include <set>
#include <utility>

namespace v3 {
struct NativeStraitClassification {
  std::vector<uint32_t> allowed,denied;
  bool operator==(const NativeStraitClassification&) const=default;
};
struct NativeLaneSelection {
  uint32_t country=UINT32_MAX,fromState=UINT32_MAX,toState=UINT32_MAX;
  int64_t distance=0;
  std::vector<NativeProvinceRange> provinces;
  bool operator==(const NativeLaneSelection&) const=default;
};
struct NativeLaneAccessScope {
  uint64_t world=0,revision=0,generation=0;
  bool operator==(const NativeLaneAccessScope&) const=default;
};
struct NativeLaneAccessRecord {
  uint32_t lane=UINT32_MAX;
  NativeLaneSelection selected;
  NativeStraitClassification access;
};
struct NativeLaneAccessBatch {
  NativeLaneAccessScope scope;
  std::vector<NativeLaneAccessRecord> records;
};
// Join owned original selections to the current copied roster. The caller must
// obtain expected independently from its current capture/collection epoch and
// recheck that epoch before publishing. Equality here grants no engine lease.
// A generation is an owned collection generation, never a masked lane slot or
// a native world pointer. No missing lane falls back to another country's data.
inline std::optional<std::map<uint32_t,std::vector<uint32_t>>> BindNativeLaneAccess(
    const NativeLaneAccessBatch& batch,const NativeLaneAccessScope& expected,
    const NativeShippingSnapshot& shipping,const std::vector<uint32_t>& straits,
    std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<std::map<uint32_t,std::vector<uint32_t>>>{
    if(error)*error=why;return {};
  };
  if(!expected.world || !expected.revision || !expected.generation || batch.scope!=expected)
    return fail("stale native lane access scope");
  if(shipping.lanes.size()>131072 || batch.records.size()>131072 || straits.size()>4096)
    return fail("native lane access bounds exceeded");
  std::set<uint32_t> registered;
  for(uint32_t id:straits)if(id==UINT32_MAX || !registered.insert(id).second)
    return fail("invalid native lane access strait roster");
  std::map<uint32_t,const NativeShippingLane*> lanes;
  std::set<uint32_t> identities;
  for(const auto& lane:shipping.lanes){
    if(lane.id==UINT32_MAX || !identities.insert(lane.id).second || lane.kind>1)
      return fail("invalid native lane access shipping roster");
    if(lane.kind!=0 || lane.provinces.empty())continue;
    if(lane.country==UINT32_MAX || lane.fromState==UINT32_MAX || lane.toState==UINT32_MAX ||
       lane.fromState==lane.toState || lane.travelDistanceFixed<0 || lane.provinces.size()>65534)
      return fail("invalid native lane access selection");
    uint32_t previous=0;
    for(const auto& range:lane.provinces){
      if(!range.first || range.first<=previous || range.last<range.first || range.last==UINT16_MAX)
        return fail("invalid native lane access province range");
      previous=range.last;
    }
    lanes.emplace(lane.id,&lane);
  }
  if(lanes.size()!=batch.records.size())return fail("incomplete native lane access batch");
  std::map<uint32_t,std::vector<uint32_t>> result;
  for(const auto& record:batch.records){
    const auto found=lanes.find(record.lane);
    if(found==lanes.end() || result.count(record.lane))return fail("unknown or duplicate native lane access record");
    const auto& lane=*found->second;const auto& selected=record.selected;
    if(selected.country!=lane.country || selected.fromState!=lane.fromState || selected.toState!=lane.toState ||
       selected.distance!=lane.travelDistanceFixed || selected.provinces!=lane.provinces)
      return fail("native lane access selection changed");
    if(record.access.allowed.size()>registered.size() || record.access.denied.size()>registered.size() ||
       record.access.allowed.size()+record.access.denied.size()!=registered.size())
      return fail("incomplete native lane access classification");
    std::set<uint32_t> classified;
    for(const auto* list:{&record.access.allowed,&record.access.denied})for(uint32_t id:*list)
      if(!registered.count(id) || !classified.insert(id).second)
        return fail("invalid native lane access classification");
    auto allowed=record.access.allowed;
    std::sort(allowed.begin(),allowed.end());
    result.emplace(record.lane,std::move(allowed));
  }
  if(error)error->clear();return result;
}
} // namespace v3
