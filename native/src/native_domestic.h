#pragma once
#include "native_world_trade.h"
#include "native_splines.h"

namespace v3 {
// Adapter-only bridge for a separately validated original permission call.
// The callback runs only inside the caller's native access scope; it must not
// retain either pointer. This helper is not bound to a game function yet.
template<class Reader,class Current,class Select>
std::optional<std::map<uint32_t,std::vector<uint32_t>>> DecodeDomesticStraitAccess(
    const Reader& reader,uint64_t module,const NativeDomesticSources& domestic,
    const NativeWorldTradeSnapshot& trade,const Current& current,const Select& select,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<std::map<uint32_t,std::vector<uint32_t>>>{if(error)*error=why;return {};};
  if(!module || module>UINT64_MAX-0x059667f8 || !current())return fail("missing current domestic access scope");
  EconomyReadWindow window(reader);
  auto read=[&](uint64_t a,void* p,size_t n){return window.Read(a,p,n);};
  auto field=[&](uint64_t p,uint64_t off,auto& value){return p && off<=UINT64_MAX-p && read(p+off,&value,sizeof(value));};
  uint64_t countriesDb=0,countries=0,straitsDb=0,straits=0,type=0;uint32_t countrySlots=0,straitSlots=0;
  if(!field(module,0x05964e20,countriesDb) || !field(countriesDb,0x20,countries) ||
     !field(countriesDb,0x2c,countrySlots) || countrySlots>65536 ||
     !field(module,0x059667f8,straitsDb) || !field(straitsDb,0,type) || type!=module+0x04439720 ||
     !field(straitsDb,0x20,straits) || !field(straitsDb,0x2c,straitSlots) || straitSlots>4096)
    return fail("invalid domestic access databases");
  std::map<uint32_t,uint64_t> countryObjects,straitObjects;
  for(const auto& lane:domestic.shipping.lanes){
    if(lane.kind!=0 || lane.country==UINT32_MAX)return fail("invalid domestic access country");
    if(countryObjects.count(lane.country))continue;
    uint64_t object=0;uint32_t id=UINT32_MAX;
    if((lane.country&0xffffff)>=countrySlots || !field(countries,8+uint64_t(lane.country&0xffffff)*16,object) ||
       !field(object,0,type) || type!=module+0x04494cb8 || !field(object,0x18,id) || id!=lane.country)
      return fail("stale domestic access country");
    countryObjects.emplace(id,object);
  }
  for(const auto& item:trade.straits){
    uint64_t object=0,definition=0;uint32_t id=UINT32_MAX,definitionId=UINT32_MAX;
    std::array<uint16_t,2> sea{};std::array<uint8_t,4> controls{};uint8_t marker=0;
    if(item.id==UINT32_MAX || (item.id&0xffffff)>=straitSlots ||
       !field(straits,8+uint64_t(item.id&0xffffff)*16,object) || !field(object,0,type) || type!=module+0x04490548 ||
       !field(object,8,id) || id!=item.id || !field(object,0xa0,definition) ||
       !field(definition,0,type) || type!=module+0x044f85a0 || !field(definition,0x10,definitionId) ||
       !field(definition,0x44,sea) || !field(definition,0x4b,marker) || !field(object,0xa8,controls) ||
       definitionId!=item.definition || sea!=item.seaProvinces || controls!=item.accessControls || marker!=item.canalDefinition ||
       !straitObjects.emplace(id,object).second)return fail("stale domestic access strait");
  }
  std::map<uint32_t,std::vector<uint32_t>> result;
  for(const auto& [country,countryObject]:countryObjects){
    auto& allowed=result[country];
    for(const auto& [id,straitObject]:straitObjects){
      if(!current())return fail("retired domestic access scope");
      const auto decision=select(straitObject,countryObject);
      if(!decision)return fail("domestic original access unavailable");
      if(*decision)allowed.push_back(id);
    }
  }
  // The original decision also depends on diplomacy and treaties, beyond the
  // fields this adapter reads. A second observation catches changed results;
  // it does not replace the caller's native access lease or prove ABA safety.
  for(const auto& [country,countryObject]:countryObjects){
    const auto& allowed=result.at(country);
    for(const auto& [id,straitObject]:straitObjects){
      if(!current())return fail("retired domestic access scope");
      const auto decision=select(straitObject,countryObject);
      if(!decision || *decision!=std::binary_search(allowed.begin(),allowed.end(),id))
        return fail("domestic original access changed during copy");
    }
  }
  if(!window.Unchanged() || !current())return fail("domestic access inputs changed during copy");
  if(error)error->clear();return result;
}
// Current original sources only. No embedded map, string/ID mapping table,
// engine call or model allocation runs in this adapter. The caller retains the
// verified completed-command scope and combined observation window.
template<class Reader,class Current>
std::optional<NativeDomesticSources> DecodeNativeDomesticSources(const Reader& reader,uint64_t module,
    const std::vector<NativeStateEconomy>& states,uint32_t goodCount,const NativeSplineNetwork& network,
    const Current& current,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<NativeDomesticSources>{if(error)*error=why;return {};};
  if(!module || module>UINT64_MAX-0x05964e28 || states.empty() || states.size()>65536 ||
     !goodCount || goodCount>128 || !current())return fail("missing current domestic source identity");
  EconomyReadWindow window(reader);
  GoodsMemoryReader read=[&](uint64_t a,void* p,size_t n){return window.Read(a,p,n);};
  auto field=[&](uint64_t p,uint64_t off,auto& value){return p && off<=UINT64_MAX-p && read(p+off,&value,sizeof(value));};
  auto u64=[](const auto& b,size_t at){uint64_t n=0;std::memcpy(&n,b.data()+at,8);return n;};
  auto u32=[](const auto& b,size_t at){uint32_t n=0;std::memcpy(&n,b.data()+at,4);return n;};
  auto u16=[](const auto& b,size_t at){uint16_t n=0;std::memcpy(&n,b.data()+at,2);return n;};
  NativeDomesticSources result;std::map<uint32_t,uint32_t> markets;
  auto counter=[&](uint64_t object,uint64_t offset,std::vector<int64_t>& values){
    std::array<uint8_t,0x30> header{};
    if(offset>UINT64_MAX-object || !read(object+offset,header.data(),header.size()))return false;
    const uint32_t cap=u32(header,0x10),count=u32(header,0x14);
    const std::array<uint64_t,2> masks{{u64(header,0x20),u64(header,0x28)}};
    if(count>cap || cap>128 || (count && count!=goodCount))return false;
    for(uint32_t g=goodCount;g<128;++g)if(masks[g/64]&(uint64_t(1)<<(g%64)))return false;
    values.assign(goodCount,0);
    if(!count)return !masks[0] && !masks[1];
    if(!read(u64(header,8),values.data(),values.size()*8))return false;
    for(uint32_t g=0;g<goodCount;++g){
      if(!(masks[g/64]&(uint64_t(1)<<(g%64))))values[g]=0;
      else if(values[g]<0)return false;
    }
    return true;
  };
  for(const auto& state:states){
    const auto object=ReadNativeStateObject(read,module,state.id);
    if(!object || !markets.emplace(state.id,state.market).second)return fail("invalid domestic state roster");
    std::vector<int64_t> production,consumption;
    if(!counter(object,0xc8,production) || !counter(object,0x78,consumption))return fail("invalid domestic goods counter");
    for(uint32_t g=0;g<goodCount;++g)if(production[g] || consumption[g])
      result.orders.push_back({state.id,UINT32_MAX,state.market,g,production[g]/100,consumption[g]/100});
  }
  auto shipping=DecodeNativeShipping(read,module,current,error);if(!shipping)return {};
  std::map<uint32_t,std::set<uint32_t>> references;
  for(const auto& lane:shipping->lanes)if(lane.kind==0){
    if(!markets.count(lane.fromState) || !markets.count(lane.toState))return fail("domestic lane endpoint is not a current state");
    auto found=references.find(lane.country);
    if(found==references.end()){
      uint64_t db=0,entries=0,country=0,type=0;uint32_t slots=0,id=UINT32_MAX;
      if(!field(module,0x05964e20,db) || !field(db,0x20,entries) || !field(db,0x2c,slots) || slots>65536 ||
         (lane.country&0xffffff)>=slots || !field(entries,8+uint64_t(lane.country&0xffffff)*16,country) ||
         !field(country,0,type) || type!=module+0x04494cb8 || !field(country,0x18,id) || id!=lane.country)
        return fail("domestic lane country is not current");
      std::array<uint8_t,0x18> header{};
      if(country>UINT64_MAX-0x350 || !read(country+0x338,header.data(),header.size()) ||
         u32(header,12)>u32(header,8) || u32(header,8)>131072)return fail("invalid country port-connection roster");
      std::vector<uint32_t> ids(u32(header,12));
      if(!ids.empty() && !read(u64(header,0),ids.data(),ids.size()*4))return fail("unreadable country port connections");
      std::set<uint32_t> unique(ids.begin(),ids.end());
      if(unique.size()!=ids.size())return fail("duplicate country port connection");
      found=references.emplace(lane.country,std::move(unique)).first;
    }
    if(!found->second.count(lane.id))return fail("domestic lane absent from original country port connections");
    result.shipping.lanes.push_back(lane);
  }
  uint64_t root=0,machine=0,world=0,type=0,provinces=0;uint32_t provinceCap=0,provinceCount=0;
  if(!field(module,0x0592d128,root) || !field(root,0x608,machine) || !field(machine,0x120,world) ||
     !field(machine,0xb0,provinces) || !field(machine,0xb8,provinceCap) || !field(machine,0xbc,provinceCount) ||
     provinceCount>provinceCap || provinceCap>131072 || !field(world,0x5618,type) || type!=module+0x044bb380)
    return fail("current domestic travel/province manager unavailable");
  std::array<uint8_t,0x18> header{};
  if(world>UINT64_MAX-0x5698 || !read(world+0x5680,header.data(),header.size()) ||
     u32(header,12)>u32(header,8) || u32(header,8)>131072)return fail("invalid domestic travel roster");
  std::vector<uint64_t> roster(u32(header,12));
  if(!roster.empty() && !read(u64(header,0),roster.data(),roster.size()*8))return fail("unreadable domestic travel roster");
  std::set<uint64_t> seen;std::set<uint32_t> ids;
  auto endpoint=[&](uint64_t object,uint64_t expected)->std::optional<uint32_t>{
    uint64_t vt=0,definition=0,province=0;uint16_t provinceId=0;uint32_t state=UINT32_MAX;
    if(!field(object,0,vt) || vt!=module+0x044e3d78 || !field(object,0x78,definition) || definition!=expected ||
       !field(definition,0,vt) || vt!=module+0x044f8a78 || !field(definition,0x44,provinceId) ||
       !provinceId || provinceId==UINT16_MAX || provinceId>=provinceCount ||
       !field(provinces,uint64_t(provinceId)*8,province) || !field(province,0,vt) || vt!=module+0x04437a00 ||
       !field(province,0x18,state) || !markets.count(state))return {};
    return state;
  };
  for(uint64_t object:roster){
    std::array<uint8_t,0x38> row{};std::array<uint8_t,0x60> def{};
    if(!seen.insert(object).second || !read(object,row.data(),row.size()) || u64(row,0)!=module+0x044e08d8 ||
       u32(row,8)==UINT32_MAX || !ids.insert(u32(row,8)).second || u32(row,0x20)>7)
      return fail("invalid domestic travel connection");
    const auto kind=u32(row,0x20);
    if(kind!=2 && kind!=3 && kind!=4)continue; // original currently usable land categories
    const uint64_t definition=u64(row,0x18);
    if(!read(definition,def.data(),def.size()) || u64(def,0)!=module+0x04515e20 || u32(def,0x10)==UINT32_MAX)
      return fail("invalid domestic travel definition");
    const uint64_t key=u64(def,0x50);
    if(kind==4 && (u32(def,0x58)!=1 || (key&63)!=1))return fail("current built rail has no rail spline definition");
    if((key&63)>1)continue; // sea/coastal/other definitions are not a new sea solver
    const auto link=network.connections.find(key);
    if(link==network.connections.end())return fail("domestic travel key absent from current spline table");
    auto source=endpoint(u64(row,0x28),u64(def,0x40)),destination=endpoint(u64(row,0x30),u64(def,0x48));
    if(!source || !destination)return fail("domestic travel endpoint province/state mismatch");
    const bool modeled=*source!=*destination && markets.at(*source)==markets.at(*destination);
    if(!modeled && kind!=4)continue;
    if(link->second.empty()){
      if(kind==4)return fail("current built rail has no physical path");
      continue;
    }
    std::vector<NativeSplinePoint> points;uint32_t anchor=uint32_t(key>>6)&0x1fffffff;
    for(uint64_t stripId:link->second){
      auto found=network.strips.find(stripId);
      if(found==network.strips.end() || (stripId&63)!=(key&63) || found->second.size()<2)
        return fail("invalid domestic land spline strip");
      const auto& refs=found->second;const bool along=refs.front()==anchor;
      if(!along && refs.back()!=anchor)return fail("disconnected domestic land spline");
      for(size_t i=points.empty()?0:1;i<refs.size();++i){
        auto p=network.points.find(refs[along?i:refs.size()-1-i]);
        if(p==network.points.end())return fail("domestic land spline point unavailable");
        points.push_back(p->second);
      }
      anchor=along?refs.back():refs.front();
    }
    if(anchor!=uint32_t(key>>35) || points.size()<2 || points.size()>4096)
      return fail("domestic land spline endpoints differ");
    auto same=[](const std::array<int64_t,2>& fixed,const NativeSplinePoint& p){
      const double x=double(fixed[0])/100000,z=double(fixed[1])/100000;
      return !(fixed[0]%100000) && !(fixed[1]%100000) && std::isfinite(p.x) && std::isfinite(p.z) &&
          x<=p.x+.001 && p.x<x+1.001 && z<=p.z+.001 && p.z<z+1.001;
    };
    std::array<int64_t,2> origin{};std::array<uint8_t,0x18> pathHeader{};
    if(!read(u64(def,0x40)+0x48,origin.data(),sizeof(origin)) || definition>UINT64_MAX-0x80 ||
       !read(definition+0x68,pathHeader.data(),pathHeader.size()) ||
       u32(pathHeader,12)!=points.size()-1 || u32(pathHeader,12)>u32(pathHeader,8) || u32(pathHeader,8)>4096)
      return fail("domestic land definition path bounds differ");
    bool along=same(origin,points.front());
    if(!along){if(!same(origin,points.back()))return fail("domestic land origin differs from current spline");
      std::reverse(points.begin(),points.end());}
    std::vector<std::array<int64_t,3>> path(points.size()-1);
    if(!read(u64(pathHeader,0),path.data(),path.size()*24))return fail("unreadable domestic land definition path");
    for(size_t i=0;i<path.size();++i)if(!same({path[i][1],path[i][2]},points[i+1]))
      return fail("domestic land definition geometry differs from current spline");
    NativeLandConnection copy{u32(row,8),*source,*destination,kind,key,along};
    if(kind==4)result.builtRails.push_back(copy);
    if(modeled)result.land.push_back(copy);
  }
  if(!window.Unchanged())return fail("domestic sources changed during copy");
  if(!current())return fail("retired domestic sources");
  if(error)error->clear();return result;
}
} // namespace v3
