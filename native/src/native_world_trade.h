#pragma once
#include "native_economy.h"
#include "native_shipping.h"
#include <map>
#include <set>

namespace v3 {
struct NativeTradePath {
  std::vector<NativeProvinceRange> provinces;
  std::vector<uint32_t> straits;
  int64_t distanceFixed=0;
};
struct NativeTradeDestination {
  uint32_t state=0;
  // Alternatives are NOT current shipments. The native selector must choose
  // one in the verified adapter scope before a formal view can consume it.
  std::vector<NativeTradePath> candidates;
  std::optional<uint32_t> selected;
};
struct NativeTradeGroup {
  uint32_t anchor=0;
  std::vector<uint32_t> states;
  std::vector<NativeTradeDestination> destinations;
};
inline constexpr int64_t kNativeTradeQuantityScale=100000;
struct NativeTradeAmount {uint32_t good=0;int64_t valueFixed=0;};
struct NativeStateTradePair {
  uint32_t source=0,destination=0;
  // Original exports tooltip 01962ED0 queries source+18 with destination's
  // primary handle (+8). Table owner exports to its peer, not the reverse.
  // Preserve 1e5 quantities; totals/merchant marine/treaty terms are not these
  // detailed records. No assumption about the common subobject's +8 ID.
  std::vector<NativeTradeAmount> goods;
};
struct NativeTradeStrait {
  uint32_t id=0;
  std::array<uint16_t,2> seaProvinces{};
  std::array<uint8_t,2> tariffModes{}; // not canal-open flags
  uint32_t definition=UINT32_MAX; // definition ID is not this runtime handle
  std::array<uint8_t,4> accessControls{}; // raw original +A8..AB, not per-country permission
  uint8_t canalDefinition=0; // raw original definition +4B
};
struct NativeSplineNetwork;
struct NativeSeaGeography;
struct NativeLandConnection {
  uint32_t id=0,source=0,destination=0,type=0;
  uint64_t spline=0; // current definition's full native connection key
  bool forward=true; // definition origin follows the native connection key
  bool operator==(const NativeLandConnection&)const=default;
};
struct NativeDomesticSources {
  std::vector<StateOrders> orders; // copied local counters, milli-orders; region unused
  NativeShippingSnapshot shipping;
  std::vector<NativeLandConnection> land;
  std::vector<NativeLandConnection> builtRails; // all current type4, including intra-state/cross-market
  // Optional owned original per-country passage results, using full handles.
  // No engine pointers escape the capture window.
  std::optional<std::map<uint32_t,std::vector<uint32_t>>> straitAccess;
};
struct NativeWorldTradeSnapshot {
  std::vector<NativeStateEconomy> states;
  // Current CMarketArea+48, exactly as01221F180/013C4E70 use it. UINT32_MAX
  // means no current area/center. Cached group representatives can be stale.
  std::map<uint32_t,uint32_t> centers;
  std::vector<NativeTradeGroup> groups;
  std::vector<NativeStateTradePair> statePairs;
  // Original sea-region panel013BF240 queries source-member+8B8 counters with
  // destination representatives. Raw peers may also exist outside the current
  // route cache: preserve full handles, never remap them by numeric similarity.
  std::vector<NativeStateTradePair> groupPairs;
  std::vector<NativeTradeStrait> straits;
  std::shared_ptr<const NativeSplineNetwork> geometry;
  std::shared_ptr<const NativeSeaGeography> seaGeography;
  std::shared_ptr<const NativeDomesticSources> domestic;
};

// Exact 1.13.11 cached world-market paths and detailed state trade records.
// Caller owns the verified native access scope. No allocation solver or
// geometry inference runs here. An optional adapter-only callback
// invokes the verified native selector while the caller still owns the scope;
// its retained pointer is range-checked and converted to an owned index.
// Without it, multiple alternatives remain explicitly unselected.
template<class Reader,class Current>
std::optional<NativeWorldTradeSnapshot> DecodeNativeWorldTrade(const Reader& reader,uint64_t module,
    const std::vector<NativeStateEconomy>& states,uint32_t goodCount,const Current& current,
    const std::function<uint64_t(uint64_t)>& select={},std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<NativeWorldTradeSnapshot>{if(error)*error=why;return {};};
  if(!current() || !module || module>UINT64_MAX-0x059667f8-8 || states.empty() ||
     states.size()>65536 || !goodCount || goodCount>128)return fail("missing current world trade identity");
  EconomyReadWindow window(reader);
  GoodsMemoryReader read=[&](uint64_t a,void* p,size_t n){return window.Read(a,p,n);};
  auto field=[&](uint64_t p,uint64_t off,auto& value){return p && off<=UINT64_MAX-p && read(p+off,&value,sizeof(value));};
  auto u64=[](const auto& bytes,size_t at){uint64_t n=0;std::memcpy(&n,bytes.data()+at,8);return n;};
  auto u32=[](const auto& bytes,size_t at){uint32_t n=0;std::memcpy(&n,bytes.data()+at,4);return n;};
  auto u16=[](const auto& bytes,size_t at){uint16_t n=0;std::memcpy(&n,bytes.data()+at,2);return n;};
  std::unordered_map<uint32_t,uint64_t> objects;
  for(const auto& state:states){
    const auto object=ReadNativeStateObject(read,module,state.id);
    if(!object || object>UINT64_MAX-0x8b8 || !objects.emplace(state.id,object).second)return fail("invalid world trade state roster");
  }
  NativeWorldTradeSnapshot result;
  result.states=states;
  uint64_t areaDb=0,areaEntries=0;uint32_t areaSlots=0;
  if(!field(module,0x05964ea0,areaDb) || !field(areaDb,0x20,areaEntries) ||
     !field(areaDb,0x2c,areaSlots) || areaSlots>65536)return fail("invalid current trade area database");
  std::map<uint32_t,uint32_t> areaCenters;
  for(const auto& state:states){
    uint32_t areaId=UINT32_MAX,center=UINT32_MAX;
    if(!field(objects.at(state.id),0xb48,areaId))return fail("unreadable current state trade area");
    if(areaId!=UINT32_MAX){
      auto found=areaCenters.find(areaId);
      if(found==areaCenters.end()){
        uint64_t area=0,type=0;uint32_t id=UINT32_MAX;
        if((areaId&0xffffff)>=areaSlots || !field(areaEntries,uint64_t(areaId&0xffffff)*16+8,area) ||
           !field(area,0,type) || type!=module+0x04430cd8 || !field(area,8,id) || id!=areaId ||
           !field(area,0x48,center) || (center!=UINT32_MAX && !objects.count(center)))
          return fail("invalid current trade area or center handle");
        found=areaCenters.emplace(areaId,center).first;
      }
      center=found->second;
    }
    result.centers.emplace(state.id,center);
  }
  uint64_t db=0,type=0,entries=0;uint32_t slots=0,active=0;
  if(!field(module,0x059667f8,db) || !field(db,0,type) || type!=module+0x04439720 ||
     !field(db,0x20,entries) || !field(db,0x2c,slots) || !field(db,0x3c,active) ||
     slots>4096 || active>slots)return fail("invalid current strait database");
  std::vector<uint64_t> straitRoster(size_t(slots)*2);
  if(slots && !read(entries,straitRoster.data(),straitRoster.size()*8))return fail("unreadable current strait roster");
  std::unordered_set<uint32_t> straitIds;std::unordered_set<uint64_t> seenObjects;
  for(uint32_t slot=0;slot<slots;++slot){
    const uint64_t object=straitRoster[size_t(slot)*2+1];if(!object)continue;
    std::array<uint8_t,0xb0> row{};std::array<uint8_t,0x4c> definition{};
    if(!seenObjects.insert(object).second || !read(object,row.data(),row.size()) ||
       u64(row,0)!=module+0x04490548 || u32(row,8)==UINT32_MAX || (u32(row,8)&0xffffff)!=slot ||
       !read(u64(row,0xa0),definition.data(),definition.size()) || u64(definition,0)!=module+0x044f85a0 || u32(definition,0x10)==UINT32_MAX ||
       !u16(definition,0x44) || u16(definition,0x44)==UINT16_MAX || !u16(definition,0x46) ||
       u16(definition,0x46)==UINT16_MAX || u16(definition,0x44)==u16(definition,0x46) || row[0xac]>3 || row[0xad]>3)
      return fail("invalid current strait type, handle or endpoints");
    straitIds.insert(u32(row,8));
    result.straits.push_back({u32(row,8),{u16(definition,0x44),u16(definition,0x46)},{row[0xac],row[0xad]},u32(definition,0x10),
        {row[0xa8],row[0xa9],row[0xaa],row[0xab]},definition[0x4b]});
  }
  if(result.straits.size()!=active)return fail("current strait active count mismatch");
  uint64_t root=0,machine=0,world=0;
  if(!field(module,0x0592d128,root) || !field(root,0x608,machine) || !field(machine,0x120,world) ||
     !world || world>UINT64_MAX-0x8b0)return fail("current world trade source unavailable");
  // Read only live buckets. Native empty maps may use shared sentinel storage;
  // a known zero count does not authorize reading unused bucket pointers.
  auto buckets=[&](uint64_t address,size_t stride,std::vector<uint8_t>& bytes,uint64_t& table){
    std::array<uint8_t,0x28> header{};
    if(!read(address,header.data(),header.size()))return false;
    const uint32_t count=u32(header,0x10),mask=u32(header,0x14);
    if(!count){bytes.clear();table=0;return true;}
    if(mask>65535 || ((mask+1)&mask) || count>mask+1 || header[0x18]==255)return false;
    bytes.resize((size_t(mask)+header[0x18]+1)*stride);
    table=u64(header,8);
    if(!read(table,bytes.data(),bytes.size()))return false;
    size_t found=0;
    for(size_t i=0;i<bytes.size();i+=stride){if(bytes[i+4]==255)return false;if(bytes[i+4])++found;}
    return found==count;
  };
  std::array<uint8_t,0x18> groupHeader{};
  if(!read(world+0x898,groupHeader.data(),groupHeader.size()) || u32(groupHeader,12)>u32(groupHeader,8) ||
     u32(groupHeader,8)>4096)return fail("invalid world trade group bounds");
  std::vector<std::array<uint8_t,0x48>> groups(u32(groupHeader,12));
  if(!groups.empty() && !read(u64(groupHeader,0),groups.data(),groups.size()*0x48))return fail("unreadable world trade groups");
  std::unordered_set<uint32_t> anchors,members;
  for(const auto& row:groups){
    NativeTradeGroup group;group.anchor=u32(row,0);
    const uint32_t cap=u32(row,0x10),count=u32(row,0x14);
    if(!objects.count(group.anchor) || !anchors.insert(group.anchor).second || !count || count>cap || cap>65536)
      return fail("invalid world trade group membership bounds");
    group.states.resize(count);
    if(!read(u64(row,8),group.states.data(),group.states.size()*4))return fail("unreadable world trade group members");
    for(uint32_t id:group.states){
      if(!objects.count(id) || !members.insert(id).second)return fail("unknown or duplicate world trade group member");
    }
    // Original013BF640 uses the representative and member roster separately.
    // A trade center may represent a group without belonging to its members.
    result.groups.push_back(std::move(group));
  }
  // Original013C4E70 reaches groups through world+800, not by a member's
  // nearest port or old representative. Verify the current lookup is exactly
  // the copied group roster before using owned representative->index lookup.
  std::vector<uint8_t> lookup;uint64_t lookupTable=0;
  if(!buckets(world+0x800,0x18,lookup,lookupTable))return fail("invalid current trade group lookup");
  std::set<uint32_t> indexed;
  for(size_t i=0;i<lookup.size();i+=0x18)if(lookup[i+4]){
    const uint64_t pointer=u64(lookup,i+0x10),begin=u64(groupHeader,0);
    const uint32_t key=u32(lookup,i+8);
    if(pointer<begin || (pointer-begin)%0x48 || (pointer-begin)/0x48>=groups.size() ||
       result.groups[(pointer-begin)/0x48].anchor!=key || !indexed.insert(key).second)
      return fail("current trade group lookup differs from copied roster");
  }
  if(indexed.size()!=groups.size())return fail("incomplete current trade group lookup");
  for(size_t i=0;i<groups.size();++i){
    std::vector<uint8_t> rows;uint64_t table=0;
    const uint64_t address=u64(groupHeader,0)+i*0x48+0x20;
    if(!buckets(address,0x28,rows,table))return fail("invalid world trade destination table");
    std::unordered_set<uint32_t> destinations;
    for(size_t j=0;j<rows.size();j+=0x28){
      if(!rows[j+4])continue;
      NativeTradeDestination dest;dest.state=u32(rows,j+8);
      if(!anchors.count(dest.state) || dest.state==result.groups[i].anchor || !destinations.insert(dest.state).second)
        return fail("unknown or duplicate world trade destination");
      const uint32_t cap=u32(rows,j+0x18),count=u32(rows,j+0x1c);
      if(count>cap || cap>256)return fail("invalid native path alternatives bounds");
      std::vector<std::array<uint8_t,0x38>> paths(count);
      if(count && !read(u64(rows,j+0x10),paths.data(),paths.size()*0x38))return fail("unreadable native path alternatives");
      for(const auto& path:paths){
        NativeTradePath copy;std::memcpy(&copy.distanceFixed,path.data()+0x30,8);
        const uint32_t rangeCap=u32(path,8),rangeCount=u32(path,12),straitCap=u32(path,0x20),straitCount=u32(path,0x24);
        if(copy.distanceFixed<0 || rangeCount>rangeCap || rangeCap>65534 || straitCount>straitCap || straitCap>1024)
          return fail("invalid native path distance or set bounds");
        std::vector<std::array<uint16_t,2>> ranges(rangeCount);
        if(rangeCount && !read(u64(path,0),ranges.data(),ranges.size()*4))return fail("unreadable native path province set");
        uint32_t previous=0;
        for(const auto& range:ranges){
          const uint32_t first=range[0],last=first+range[1];
          if(!first || last>=UINT16_MAX || first<=previous)return fail("invalid native path province range");
          copy.provinces.push_back({uint16_t(first),uint16_t(last)});previous=last;
        }
        copy.straits.resize(straitCount);
        if(straitCount && !read(u64(path,0x18),copy.straits.data(),copy.straits.size()*4))return fail("unreadable native path straits");
        for(uint32_t id:copy.straits)if(!straitIds.count(id))return fail("native path references an unavailable strait");
        dest.candidates.push_back(std::move(copy));
      }
      // The original selector returns the sole path directly, and uses its
      // current native cost comparison only when there are alternatives.
      if(count==1)dest.selected=0;
      else if(count && select){
        if(!current())return fail("retired before native path selection");
        const uint64_t chosen=select(table+j+0x10),begin=u64(rows,j+0x10);
        if(chosen<begin || (chosen-begin)%0x38 || (chosen-begin)/0x38>=count)
          return fail("native selector returned a path outside its current alternatives");
        dest.selected=uint32_t((chosen-begin)/0x38);
      }
      result.groups[i].destinations.push_back(std::move(dest));
    }
  }
  for(const auto& state:states)for(unsigned grouped=0;grouped<2;++grouped){
    std::vector<uint8_t> rows;uint64_t table=0;
    if(!buckets(objects.at(state.id)+(grouped?0x8b8:0x890),0x60,rows,table))return fail("invalid state trade table");
    std::unordered_set<uint32_t> peers;
    for(size_t j=0;j<rows.size();j+=0x60){
      if(!rows[j+4])continue;
      NativeStateTradePair pair;pair.source=state.id;pair.destination=u32(rows,j+8);
      if(!objects.count(pair.destination) || !peers.insert(pair.destination).second)
        return fail("unknown or duplicate state trade peer");
      const uint64_t data=u64(rows,j+0x18),mask0=u64(rows,j+0x30),mask1=u64(rows,j+0x38);
      const uint32_t cap=u32(rows,j+0x20),count=u32(rows,j+0x24);
      if(u64(rows,j+0x10)!=module+0x044283b0 || count>cap || cap>128 || (count && count!=goodCount))
        return fail("invalid state trade goods counter");
      const std::array<uint64_t,2> masks{{mask0,mask1}};
      for(uint32_t good=goodCount;good<128;++good)if(masks[good/64]&(uint64_t(1)<<(good%64)))
        return fail("state trade good outside catalog");
      if(!count && (mask0 || mask1))return fail("state trade mask without quantities");
      std::vector<int64_t> values(count);
      if(count && !read(data,values.data(),values.size()*8))return fail("unreadable state trade quantities");
      for(uint32_t good=0;good<count;++good)if(masks[good/64]&(uint64_t(1)<<(good%64))){
        if(values[good]<0)return fail("negative state trade quantity");
        if(values[good])pair.goods.push_back({good,values[good]});
      }
      (grouped?result.groupPairs:result.statePairs).push_back(std::move(pair));
    }
  }
  if(!window.Unchanged())return fail("world trade changed during copy");
  if(!current())return fail("retired world trade snapshot");
  if(error)error->clear();return result;
}
} // namespace v3
