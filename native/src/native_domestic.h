#pragma once
#include "native_world_trade.h"
#include "native_splines.h"
#include "native_travel.h"

namespace v3 {
// Adapter-only bridge for a separately validated original permission call.
// The callback runs only inside the caller's native access scope; it must not
// retain either pointer. This helper is not bound to a game function yet.
template<class Reader,class Current,class Select>
std::optional<std::map<uint32_t,std::vector<uint32_t>>> DecodeDomesticStraitAccess(
    const Reader& reader,uint64_t module,const NativeDomesticSources& domestic,
    const NativeWorldTradeSnapshot& trade,const Current& current,const Select& select,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<std::map<uint32_t,std::vector<uint32_t>>>{if(error)*error=why;return {};};
  if(!module || module>UINT64_MAX-0x05c43138 || !current())return fail("missing current domestic access scope");
  EconomyReadWindow window(reader);
  auto read=[&](uint64_t a,void* p,size_t n){return window.Read(a,p,n);};
  auto field=[&](uint64_t p,uint64_t off,auto& value){return p && off<=UINT64_MAX-p && read(p+off,&value,sizeof(value));};
  uint64_t countriesDb=0,countries=0,straitsDb=0,straits=0,type=0;uint32_t countrySlots=0,straitSlots=0;
  if(!field(module,0x05c42c20,countriesDb) || !field(countriesDb,0x20,countries) ||
     !field(countriesDb,0x2c,countrySlots) || countrySlots>65536 ||
     !field(module,0x05c43138,straitsDb) || !field(straitsDb,0,type) || type!=module+0x0471cc78 ||
     !field(straitsDb,0x20,straits) || !field(straitsDb,0x2c,straitSlots) || straitSlots>4096)
    return fail("invalid domestic access databases");
  std::map<uint32_t,uint64_t> countryObjects,straitObjects;
  for(const auto& lane:domestic.shipping.lanes){
    if(lane.kind!=0 || lane.country==UINT32_MAX)return fail("invalid domestic access country");
    if(countryObjects.count(lane.country))continue;
    uint64_t object=0;uint32_t id=UINT32_MAX;
    if((lane.country&0xffffff)>=countrySlots || !field(countries,8+uint64_t(lane.country&0xffffff)*16,object) ||
       !field(object,0,type) || type!=module+0x0468fce0 || !field(object,0x18,id) || id!=lane.country)
      return fail("stale domestic access country");
    countryObjects.emplace(id,object);
  }
  for(const auto& item:trade.straits){
    uint64_t object=0,definition=0;uint32_t id=UINT32_MAX,definitionId=UINT32_MAX;
    std::array<uint16_t,2> sea{};std::array<uint8_t,4> controls{};uint8_t marker=0;
    if(item.id==UINT32_MAX || (item.id&0xffffff)>=straitSlots ||
       !field(straits,8+uint64_t(item.id&0xffffff)*16,object) || !field(object,0,type) || type!=module+0x046c9758 ||
       !field(object,8,id) || id!=item.id || !field(object,0x120,definition) ||
       !field(definition,0,type) || type!=module+0x04786528 || !field(definition,0x10,definitionId) ||
       !field(definition,0x44,sea) || !field(definition,0x58,marker) || !field(object,0x128,controls) ||
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
  if(!module || module>UINT64_MAX-0x05c42c28 || states.empty() || states.size()>65536 ||
     !goodCount || goodCount>128 || !current())return fail("missing current domestic source identity");
  EconomyReadWindow window(reader);
  GoodsMemoryReader read=[&](uint64_t a,void* p,size_t n){return window.Read(a,p,n);};
  auto field=[&](uint64_t p,uint64_t off,auto& value){return p && off<=UINT64_MAX-p && read(p+off,&value,sizeof(value));};
  auto u64=[](const auto& b,size_t at){uint64_t n=0;std::memcpy(&n,b.data()+at,8);return n;};
  auto u32=[](const auto& b,size_t at){uint32_t n=0;std::memcpy(&n,b.data()+at,4);return n;};
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
    if(!counter(object,0xd0,production) || !counter(object,0x80,consumption))return fail("invalid domestic goods counter");
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
      if(!field(module,0x05c42c20,db) || !field(db,0x20,entries) || !field(db,0x2c,slots) || slots>65536 ||
         (lane.country&0xffffff)>=slots || !field(entries,8+uint64_t(lane.country&0xffffff)*16,country) ||
         !field(country,0,type) || type!=module+0x0468fce0 || !field(country,0x18,id) || id!=lane.country)
        return fail("domestic lane country is not current");
      std::array<uint8_t,0x18> header{};
      if(country>UINT64_MAX-0x3b8 || !read(country+0x3a0,header.data(),header.size()) ||
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
  uint64_t root=0,machine=0,provinces=0;uint32_t provinceCap=0,provinceCount=0;
  if(!field(module,0x05c07c68,root) || !field(root,0x608,machine) ||
     !field(machine,0xb0,provinces) || !field(machine,0xb8,provinceCap) || !field(machine,0xbc,provinceCount) ||
     provinceCount>provinceCap || provinceCap>131072)return fail("current domestic province manager unavailable");
  auto travel=DecodeNativeTravelNetwork(read,module,current,error);if(!travel)return {};
  auto node=[&](const NativeTravelEndpoint& endpoint)->const NativeTravelNode&{
    return travel->nodes[endpoint.naval][endpoint.index];
  };
  auto endpoint=[&](const NativeTravelNode& n)->std::optional<uint32_t>{
    uint64_t province=0,type=0,definition=0,region=0,stateRegion=0;uint32_t state=UINT32_MAX;
    if(!n.province || n.province==UINT16_MAX || n.province>=provinceCount ||
       !field(provinces,uint64_t(n.province)*8,province) || !field(province,0,type) || type!=module+0x046ac1c0 ||
       !field(province,0x18,state) || !markets.count(state) || !field(province,8,definition) ||
       !field(definition,0,type) || type!=module+0x046ab568 || !field(definition,0xc8,region) ||
       !field(ReadNativeStateObject(read,module,state),0xf10,stateRegion) || region!=stateRegion)return {};
    return state;
  };
  for(const auto& connection:travel->connections[1]){
    if(connection.kind!=6 || connection.development!=5 || connection.spline!=UINT64_MAX)continue;
    const auto& from=node(connection.endpoints[0]);const auto& to=node(connection.endpoints[1]);
    const bool fromLand=!connection.endpoints[0].naval && (from.roles&0x7f)==2;
    const bool toLand=!connection.endpoints[1].naval && (to.roles&0x7f)==2;
    if(!((fromLand && connection.endpoints[1].naval && (to.roles&0x380)==0x200) ||
         (toLand && connection.endpoints[0].naval && (from.roles&0x380)==0x200)))continue;
    const auto state=endpoint(fromLand?from:to);
    if(!state || from.control>=0x1fffffff || to.control>=0x1fffffff || from.control==to.control)
      return fail("native port access province or control mismatch");
    NativePortTransfer transfer;transfer.id=connection.id;transfer.state=*state;
    transfer.anchors={from.control,to.control};transfer.origin=from.position;
    // CA8550 builds the original leg from these definition positions/distance;
    // CA7D40 emits that leg unchanged when the native spline key is absent.
    transfer.path.push_back({connection.distanceFixed,to.position[0],to.position[1]});
    result.portTransfers.push_back(std::move(transfer));
  }
  for(const auto& connection:travel->connections[0]){
    const auto kind=connection.development;
    if(kind!=2 && kind!=3 && kind!=4)continue;
    const uint64_t key=connection.spline;
    if(kind==4 && (connection.kind!=2 || (key&63)!=1))return fail("current built rail has no rail spline definition");
    if(key==UINT64_MAX || (key&63)>1)continue;
    const auto& from=node(connection.endpoints[0]);const auto& to=node(connection.endpoints[1]);
    if(connection.endpoints[0].naval || connection.endpoints[1].naval)return fail("land connection uses naval endpoint");
    const auto source=endpoint(from),destination=endpoint(to);
    if(!source || !destination)return fail("domestic travel endpoint province/state mismatch");
    const bool modeled=*source!=*destination && markets.at(*source)==markets.at(*destination);
    const bool local=*source==*destination;
    if(!modeled && !local && kind!=4)continue;
    const auto link=network.connections.find(key);
    if(link==network.connections.end())return fail("domestic travel key absent from current spline table");
    if(link->second.empty()){
      if(kind==4)return fail("current built rail has no physical path");
      continue;
    }
    uint32_t anchor=uint32_t(key>>6)&0x1fffffff;const uint32_t last=uint32_t(key>>35);
    const bool along=from.control==anchor && to.control==last;
    if(!along && (from.control!=last || to.control!=anchor))return fail("domestic land controls differ from current spline");
    std::vector<NativeSplinePoint> points;
    for(uint64_t stripId:link->second){
      const auto found=network.strips.find(stripId);
      if(found==network.strips.end() || (stripId&63)!=(key&63) || found->second.size()<2)
        return fail("invalid domestic land spline strip");
      const auto& refs=found->second;const bool forward=refs.front()==anchor;
      if(!forward && refs.back()!=anchor)return fail("disconnected domestic land spline");
      for(size_t i=points.empty()?0:1;i<refs.size();++i){
        const auto point=network.points.find(refs[forward?i:refs.size()-1-i]);
        if(point==network.points.end() || !std::isfinite(point->second.x) || !std::isfinite(point->second.z))
          return fail("domestic land spline point unavailable");
        points.push_back(point->second);
      }
      anchor=forward?refs.back():refs.front();
    }
    if(anchor!=last || points.size()<2 || points.size()>4096)return fail("domestic land spline endpoints differ");
    auto same=[](const std::array<int64_t,2>& fixed,const NativeSplinePoint& p){
      return double(fixed[0])==std::floor(double(p.x))*100000 && double(fixed[1])==std::floor(double(p.z))*100000;
    };
    if(!same(from.position,along?points.front():points.back()) || !same(to.position,along?points.back():points.front()))
      return fail("domestic land definition geometry differs from current spline");
    NativeLandConnection copy{connection.id,*source,*destination,kind,key,along};
    for(size_t side=0;side<2;++side){
      const auto& n=side?to:from;if((n.roles&0x7f)!=1)continue;
      uint64_t region=0,type=0;uint32_t definitionId=UINT32_MAX,geo=UINT32_MAX;uint8_t valid=0;
      const auto state=side?*destination:*source;
      if(!field(ReadNativeStateObject(read,module,state),0xf10,region) || !field(region,0,type) || type!=module+0x046ba948 ||
         !field(region,0x10,definitionId) || definitionId==UINT32_MAX || !field(region,0x40,geo) || geo==UINT32_MAX ||
         !field(region,0x44,valid) || valid!=1)return fail("current primary land hub region mismatch");
      const auto [where,inserted]=result.landHubs.emplace(state,n.control);
      if(!inserted && where->second!=n.control)return fail("ambiguous current primary land hub");
    }
    if(kind==4)result.builtRails.push_back(copy);
    if(modeled)result.land.push_back(copy);
    if(local)result.localLand.push_back(copy);
  }
  if(!window.Unchanged())return fail("domestic sources changed during copy");
  if(!current())return fail("retired domestic sources");
  if(error)error->clear();return result;
}
} // namespace v3
