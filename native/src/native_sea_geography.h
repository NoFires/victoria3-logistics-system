#pragma once
#include "native_world_trade.h"
#include "native_splines.h"
#include <algorithm>

namespace v3 {
struct NativeStraitDefinition {uint32_t id=0;std::array<uint16_t,2> seaProvinces{};};
struct NativeSeaRegion {uint32_t definition=0,geo=0;};
struct NativeSeaProvince {
  NativeSeaRegion region;
  std::vector<uint16_t> adjacent; // native order, not a new route
};
struct NativeSeaGeography {
  std::vector<NativeStraitDefinition> definitions; // includes unbuilt canals
  std::map<uint16_t,NativeSeaProvince> provinces;
  std::map<uint32_t,NativeSeaRegion> states; // full runtime state IDs
  std::map<uint32_t,bool> stateHubs; // original current hub eligibility, not a port-building claim
};

// Current-thread definition context only. No singleton getter/TLS initializer,
// lock or native highlighting call. Copy only provinces referenced by native
// paths/definitions; the full pointer roster is bounded but land objects are not
// scanned. Caller owns the verified native access scope and combined window.
template<class Reader,class Current>
std::optional<NativeSeaGeography> DecodeNativeSeaGeography(const Reader& reader,uint64_t module,uint64_t tlsArray,
    const std::vector<NativeStateEconomy>& states,const NativeWorldTradeSnapshot& trade,const Current& current,
    std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<NativeSeaGeography>{if(error)*error=why;return {};};
  if(!module || module>UINT64_MAX-0x05964ef8 || !tlsArray || states.empty() || states.size()>65536 || !current())
    return fail("missing current sea geography identity");
  EconomyReadWindow window(reader);
  GoodsMemoryReader read=[&](uint64_t a,void* p,size_t n){return window.Read(a,p,n);};
  auto field=[&](uint64_t p,uint64_t off,auto& value){return p && off<=UINT64_MAX-p && read(p+off,&value,sizeof(value));};
  auto u64=[](const auto& bytes,size_t at){uint64_t n=0;std::memcpy(&n,bytes.data()+at,8);return n;};
  auto u32=[](const auto& bytes,size_t at){uint32_t n=0;std::memcpy(&n,bytes.data()+at,4);return n;};
  auto u16=[](const auto& bytes,size_t at){uint16_t n=0;std::memcpy(&n,bytes.data()+at,2);return n;};
  uint64_t tls=0,token=0,database=0,type=0,objects=0;uint32_t initialized=0,cap=0,count=0;uint8_t active=0;
  if(!field(tlsArray,0,tls) || !field(tls,0x8600,initialized) || !(initialized&1) ||
     !field(tls,0x8648,token) || uint32_t(token)>=16 ||
     !field(module,0x0514d418+16*uint32_t(token),database) ||
     !field(module,0x0514d420+16*uint32_t(token),active) || active!=1 ||
     !field(database,0,type) || type!=module+0x04424140 || !field(database,0x80,objects) ||
     !field(database,0x88,cap) || !field(database,0x8c,count) || count>cap || cap>4096)
    return fail("current strait definition context unavailable");
  std::vector<uint64_t> roster(count);
  if(count && !read(objects,roster.data(),roster.size()*8))return fail("unreadable strait definition roster");
  NativeSeaGeography result;std::set<uint16_t> wanted;std::set<uint64_t> seen;
  for(uint32_t id=0;id<count;++id){
    std::array<uint8_t,0x48> row{};
    if(!seen.insert(roster[id]).second || !read(roster[id],row.data(),row.size()) || u64(row,0)!=module+0x044f85a0 ||
       u32(row,0x10)!=id || !u16(row,0x44) || u16(row,0x44)==UINT16_MAX || !u16(row,0x46) ||
       u16(row,0x46)==UINT16_MAX || u16(row,0x44)==u16(row,0x46))return fail("invalid current strait definition");
    result.definitions.push_back({id,{u16(row,0x44),u16(row,0x46)}});
    wanted.insert(u16(row,0x44));wanted.insert(u16(row,0x46));
  }
  for(const auto& strait:trade.straits)
    if(strait.definition>=result.definitions.size() ||
       result.definitions[strait.definition].seaProvinces!=strait.seaProvinces)
      return fail("runtime strait does not match current definition context");
  if(trade.domestic)for(const auto& lane:trade.domestic->shipping.lanes)
    for(const auto& range:lane.provinces)
      for(uint32_t id=range.first;id<=range.last;++id)wanted.insert(uint16_t(id));
  for(const auto& group:trade.groups)for(const auto& destination:group.destinations)
    for(const auto& path:destination.candidates){
      uint32_t previous=0;
      for(const auto& range:path.provinces){
        if(!range.first || range.first<=previous || range.first>range.last || range.last==UINT16_MAX)
          return fail("invalid native sea province set");
        for(uint32_t id=range.first;id<=range.last;++id)wanted.insert(uint16_t(id));
        previous=range.last;
      }
    }
  uint64_t map=0,provinces=0;
  if(!field(module,0x058fccf0,map) || !field(map,0,type) || type!=module+0x0441c938 ||
     !field(map,0x2a0,provinces) || !field(map,0x2a8,cap) || !field(map,0x2ac,count) ||
     !count || count>65535 || count>cap || cap>131072)return fail("current province map unavailable");
  roster.resize(count);
  if(!read(provinces,roster.data(),roster.size()*8))return fail("unreadable current province roster");
  std::map<uint64_t,NativeSeaRegion> regions;
  auto region=[&](uint64_t object)->std::optional<NativeSeaRegion>{
    if(auto found=regions.find(object);found!=regions.end())return found->second;
    std::array<uint8_t,0x48> row{};
    if(!read(object,row.data(),row.size()) || u64(row,0)!=module+0x044e7850 ||
       u32(row,0x10)==UINT32_MAX || row[0x44]!=1 || u32(row,0x40)==UINT32_MAX)return {};
    NativeSeaRegion copy{u32(row,0x10),u32(row,0x40)};regions.emplace(object,copy);return copy;
  };
  seen.clear();
  for(uint16_t id:wanted){
    if(id>=roster.size() || !seen.insert(roster[id]).second || !field(roster[id],0,type) ||
       type!=module+0x044dc488)return fail("unknown native sea province");
    uint64_t definition=0;
    if(!field(roster[id],0xc8,definition))return fail("unreadable native sea region");
    auto identity=region(definition);if(!identity)return fail("invalid native sea region identity");
    NativeSeaProvince copy;copy.region=*identity;
    std::array<uint8_t,0x18> header{};
    if(!read(roster[id]+0x50,header.data(),header.size()) ||
       u32(header,12)>u32(header,8) || u32(header,8)>65535)return fail("invalid native sea adjacency bounds");
    std::vector<std::array<uint8_t,0x28>> adjacent(u32(header,12));
    if(!adjacent.empty() && !read(u64(header,0),adjacent.data(),adjacent.size()*0x28))return fail("unreadable native sea adjacency");
    for(const auto& edge:adjacent){const uint32_t peer=u32(edge,0);
      if(!peer || peer>=roster.size() || peer==UINT16_MAX)return fail("unknown native sea adjacent province");
      copy.adjacent.push_back(uint16_t(peer));
    }
    result.provinces.emplace(id,std::move(copy));
  }
  for(const auto& state:states){
    const auto object=ReadNativeStateObject(read,module,state.id);uint64_t definition=0;
    if(!object || !field(object,0xe78,definition))return fail("unavailable native state region");
    auto identity=region(definition);
    if(!identity || !result.states.emplace(state.id,*identity).second)return fail("invalid or duplicate native state geography");
  }
  if(!window.Unchanged())return fail("native sea geography changed during copy");
  if(!current())return fail("retired native sea geography");
  if(error)error->clear();return result;
}

// Literal uint32 multiplication and bit packing from original025E8280. Both
// original probes are retained; only keys present in the current native table
// can be returned. This does not choose a path or fabricate a shore connector.
inline std::array<uint64_t,2> NativeSeaSplineKeys(uint32_t first,uint32_t second) {
  const uint32_t a=first*100,b=second*100,aa=(a&0x1fffffff)|0x800000,bb=(b&0x1f7fffff)|0x800000;
  return {{(((uint64_t(aa<bb?b:a)<<29)|(aa<bb?a:b)|0x10000000800000ull)<<6)|2,
           (((uint64_t(bb<aa?a:b)<<29)|(bb<aa?b:a)|0x10000000800000ull)<<6)|2}};
}
struct NativeSeaConnections {
  std::vector<uint64_t> keys; // membership, not travel order
  uint32_t blockedStraitAdjacencies=0;
};
inline std::optional<NativeSeaConnections> SelectNativeSeaConnections(const NativeWorldTradeSnapshot& trade,
    const NativeSeaGeography& geography,const NativeTradeDestination& destination,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<NativeSeaConnections>{if(error)*error=why;return {};};
  if(!trade.geometry)return fail("current native spline geometry unavailable");
  NativeSeaConnections result;
  if(destination.selected && *destination.selected>=destination.candidates.size())return fail("native selected index out of bounds");
  if(destination.candidates.empty()){if(error)error->clear();return result;}
  if(!destination.selected || *destination.selected>=destination.candidates.size())return fail("native route not selected");
  const auto& path=destination.candidates[*destination.selected];
  auto pair=[](uint16_t a,uint16_t b){return (uint32_t(std::min(a,b))<<16)|std::max(a,b);};
  std::set<uint32_t> governed,permitted;
  for(size_t id=0;id<geography.definitions.size();++id){
    const auto& d=geography.definitions[id];
    if(d.id!=id || !d.seaProvinces[0] || !d.seaProvinces[1] || d.seaProvinces[0]==UINT16_MAX ||
       d.seaProvinces[1]==UINT16_MAX || d.seaProvinces[0]==d.seaProvinces[1])
      return fail("invalid owned strait definition identity");
    governed.insert(pair(d.seaProvinces[0],d.seaProvinces[1]));
  }
  std::map<uint32_t,const NativeTradeStrait*> straits;
  for(const auto& strait:trade.straits)
    if(strait.id==UINT32_MAX || !straits.emplace(strait.id,&strait).second || strait.definition>=geography.definitions.size() ||
       geography.definitions[strait.definition].seaProvinces!=strait.seaProvinces)
      return fail("invalid current strait/definition binding");
  for(uint32_t id:path.straits){
    auto found=straits.find(id);if(found==straits.end())return fail("selected path references an unavailable current strait");
    const auto& s=*found->second;permitted.insert(pair(s.seaProvinces[0],s.seaProvinces[1]));
  }
  std::set<uint16_t> members;uint32_t previous=0;
  for(const auto& range:path.provinces){
    if(!range.first || range.first<=previous || range.first>range.last || range.last==UINT16_MAX)
      return fail("invalid selected province set");
    for(uint32_t id=range.first;id<=range.last;++id){
      if(!geography.provinces.count(uint16_t(id)))return fail("selected province geography unavailable");
      members.insert(uint16_t(id));
    }
    previous=range.last;
  }
  std::set<uint64_t> keys;
  for(uint16_t id:members){
    const auto& from=geography.provinces.at(id);
    for(uint16_t peer:from.adjacent)if(members.count(peer)){
      const auto seaPair=pair(id,peer);
      if(governed.count(seaPair) && !permitted.count(seaPair)){++result.blockedStraitAdjacencies;continue;}
      const auto& to=geography.provinces.at(peer);
      for(uint64_t key:NativeSeaSplineKeys(from.region.geo,to.region.geo)){
        const auto line=trade.geometry->connections.find(key);
        if(line!=trade.geometry->connections.end() && !line->second.empty())keys.insert(key);
      }
    }
  }
  result.keys.assign(keys.begin(),keys.end());
  if(error)error->clear();return result;
}
} // namespace v3
