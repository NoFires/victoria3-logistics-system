#pragma once
#include "native_sea_geography.h"

namespace v3 {
// Raw read-only equivalent of the eligibility test used by025E8070/02607620.
// Current manager transforms, not authored fallback positions or nearest coast.
// Caller owns the verified access scope and outer combined snapshot window.
template<class Reader,class Current>
std::optional<std::map<uint32_t,bool>> DecodeNativePortHubs(const Reader& reader,uint64_t module,
    const std::map<uint32_t,NativeSeaRegion>& states,const Current& current,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<std::map<uint32_t,bool>>{if(error)*error=why;return {};};
  if(!module || module>UINT64_MAX-0x0592d130 || states.empty() || states.size()>65536 || !current())
    return fail("missing current native port identity");
  EconomyReadWindow window(reader);
  auto read=[&](uint64_t at,void* out,size_t size){return window.Read(at,out,size);};
  auto field=[&](uint64_t p,uint64_t off,auto& value){return p && off<=UINT64_MAX-p && read(p+off,&value,sizeof(value));};
  auto u64=[](const auto& b,size_t at){uint64_t n=0;std::memcpy(&n,b.data()+at,8);return n;};
  auto u32=[](const auto& b,size_t at){uint32_t n=0;std::memcpy(&n,b.data()+at,4);return n;};
  auto array=[&](uint64_t at,uint32_t limit,std::array<uint8_t,0x18>& h){
    return read(at,h.data(),h.size()) && u32(h,12)<=u32(h,8) && u32(h,8)<=limit;
  };
  uint64_t root=0,machine=0,world=0,graphics=0,type=0,manager=0;
  if(!field(module,0x0592d128,root) || !field(root,0x608,machine) || !field(machine,0x120,world) ||
     !field(module,0x05901fa0,graphics) || !field(graphics,0,type) || type!=module+0x0464e4e8 ||
     !field(graphics,0x290,manager) || !manager || manager>UINT64_MAX-0x210 || !world || world>UINT64_MAX-0x3c88)
    return fail("current native port sources unavailable");
  std::array<uint8_t,0x18> regionHeader{},typesHeader{},transformsHeader{};
  if(!array(world+0x3c70,65536,regionHeader) || !u32(regionHeader,12) ||
     !array(module+0x051385c0,64,typesHeader) || u32(typesHeader,12)!=5 ||
     !array(manager+0x1f8,65536,transformsHeader))return fail("invalid native port source bounds");
  std::map<uint32_t,bool> result;
  std::map<uint32_t,uint32_t> wanted; // geo ID -> region definition ID
  for(const auto& [id,region]:states){
    if(region.definition>=u32(regionHeader,12) || region.geo==UINT32_MAX)
      return fail("native state region outside current world");
    uint64_t object=0,definition=0;
    if(!field(u64(regionHeader,0),uint64_t(region.definition)*8,object) ||
       !field(object,0,type) || type!=module+0x044df758 || !field(object,0x90,definition))
      return fail("unavailable current native state region");
    std::array<uint8_t,0x48> row{};
    if(!read(definition,row.data(),row.size()) || u64(row,0)!=module+0x044e7850 ||
       u32(row,0x10)!=region.definition || u32(row,0x40)!=region.geo || row[0x44]!=1)
      return fail("state geography differs from current native port region");
    auto [found,inserted]=wanted.emplace(region.geo,region.definition);
    if(!inserted && found->second!=region.definition)return fail("ambiguous native port geo identity");
    result.emplace(id,false);
  }
  std::array<uint64_t,10> types{};
  if(!read(u64(typesHeader,0),types.data(),sizeof(types)))return fail("unreadable native hub types");
  std::map<uint32_t,bool> available;
  // Original hub order is3,0,2,1,4. Eligibility is any successful type: its
  // position/type does not enter the connection key or endpoint-only score.
  for(unsigned category:{3u,0u,2u,1u,4u}){
    const uint64_t object=types[category*2];
    if(!field(object,0,type))return fail("unavailable native hub type");
    if(type==module+0x047435c8)continue; // original null type: IsValid returns false
    std::array<uint8_t,0xa0> row{};
    // The registered hubs are CGameObjectLocator, not parsed CMapObjectType.
    // Its native IsValid slot50 is constant true0624690; +90 is a category ID.
    if(type!=module+0x047295f0 || !read(object,row.data(),row.size()))return fail("unknown native hub type");
    const uint32_t cap=u32(row,0x58),count=u32(row,0x5c),index=u32(row,0x98);
    // Finalize02C04CF0 assigns this runtime transform index.
    if(count>cap || cap>65536 || index>=u32(transformsHeader,12))return fail("invalid native hub instance bounds");
    std::array<uint8_t,0x18> positions{};
    const uint64_t headers=u64(transformsHeader,0),offset=uint64_t(index)*0x18;
    if(!headers || offset>UINT64_MAX-headers || !array(headers+offset,65536,positions) || u32(positions,12)!=count)
      return fail("native hub transforms do not match current instances");
    std::vector<std::array<uint8_t,0x30>> instances(count);
    if(count && !read(u64(row,0x50),instances.data(),size_t(count)*0x30))return fail("unreadable native hub instances");
    for(size_t i=1;i<instances.size();++i)if(u32(instances[i-1],4)>u32(instances[i],4))
      return fail("native hub instances are not in original lookup order");
    for(const auto& [geo,definition]:wanted){
      auto found=std::lower_bound(instances.begin(),instances.end(),geo,
          [&](const auto& instance,uint32_t id){return u32(instance,4)<id;});
      if(found==instances.end() || u32(*found,4)!=geo)continue;
      const size_t slot=size_t(found-instances.begin());std::array<float,10> transform{};
      const uint64_t data=u64(positions,0);
      if(!data || slot*0x28>UINT64_MAX-data || !read(data+slot*0x28,transform.data(),sizeof(transform)) ||
         !std::isfinite(transform[0]) || !std::isfinite(transform[2]))return fail("invalid current native hub position");
      // Literal02607620 condition: do not tighten it to both coordinates >=0.
      available[geo]|=(transform[0]>=0 || transform[2]>=0) && (transform[0]!=0 || transform[2]!=0);
    }
  }
  for(const auto& [id,region]:states)result.at(id)=available[region.geo];
  if(!window.Unchanged())return fail("native port sources changed during copy");
  if(!current())return fail("retired native port sources");
  if(error)error->clear();return result;
}

inline std::array<uint64_t,2> NativePortSplineKeys(uint32_t sea,uint32_t land) {
  auto keys=NativeSeaSplineKeys(sea,land);
  for(auto& key:keys)key=(key&~uint64_t(63))|3; // literal025E8070: same packing, type3
  return keys;
}
struct NativeRouteConnections {
  NativeSeaConnections sea;
  std::array<std::optional<uint64_t>,2> ports; // source/destination; known empty native link may be selected
};
inline std::optional<NativeRouteConnections> SelectNativeRouteConnections(const NativeWorldTradeSnapshot& trade,
    const NativeSeaGeography& geography,uint32_t source,const NativeTradeDestination& destination,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<NativeRouteConnections>{if(error)*error=why;return {};};
  auto sea=SelectNativeSeaConnections(trade,geography,destination,error);if(!sea)return {};
  NativeRouteConnections result;result.sea=std::move(*sea);
  if(destination.candidates.empty()){if(error)error->clear();return result;}
  const std::array<uint32_t,2> endpoints{{source,destination.state}};
  for(uint32_t id:endpoints)if(!geography.states.count(id) || !geography.stateHubs.count(id))
    return fail("native endpoint geography or hub eligibility unavailable");
  std::set<uint16_t> members;
  for(const auto& range:destination.candidates[*destination.selected].provinces)
    for(uint32_t id=range.first;id<=range.last;++id)members.insert(uint16_t(id));
  //025E8450/8690 visit the original ordered set and its native adjacency.
  //025E7F00 score depends only on the endpoint's buildings, never on candidate
  // province/key. Every candidate for one endpoint ties, so original first wins.
  for(uint16_t id:members){
    const auto& province=geography.provinces.at(id);
    for(uint16_t peer:province.adjacent)if(!members.count(peer)){
      for(size_t endpoint=0;endpoint<2;++endpoint){
        if(result.ports[endpoint] || !geography.stateHubs.at(endpoints[endpoint]))continue;
        for(uint64_t key:NativePortSplineKeys(province.region.geo,geography.states.at(endpoints[endpoint]).geo))
          if(trade.geometry->connections.count(key)){result.ports[endpoint]=key;break;}
      }
    }
  }
  if(error)error->clear();return result;
}
} // namespace v3
