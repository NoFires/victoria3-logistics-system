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
// 1.15.0 layout: NTravelNetwork keeps separate land and naval node/connection
// containers at world+0x5028. Land roads and built rail still reference the
// current spline connection by key. Keyless ground legs need current original
// corridor references; their geometry is copied from the original definition.
template<class Reader,class Current>
std::optional<NativeDomesticSources> DecodeNativeDomesticSources(const Reader& reader,uint64_t module,
    const std::vector<NativeStateEconomy>& states,uint32_t goodCount,const NativeSplineNetwork& network,
    const Current& current,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<NativeDomesticSources>{if(error)*error=why;return {};};
  if(!module || module>UINT64_MAX-0x05c07c70 || states.empty() || states.size()>65536 ||
     !goodCount || goodCount>128 || !current())return fail("missing current domestic source identity");
  EconomyReadWindow window(reader);
  GoodsMemoryReader read=[&](uint64_t a,void* p,size_t n){return window.Read(a,p,n);};
  auto field=[&](uint64_t p,uint64_t off,auto& value){return p && off<=UINT64_MAX-p && read(p+off,&value,sizeof(value));};
  auto u64=[](const auto& b,size_t at){uint64_t n=0;std::memcpy(&n,b.data()+at,8);return n;};
  auto u32=[](const auto& b,size_t at){uint32_t n=0;std::memcpy(&n,b.data()+at,4);return n;};
  auto u16=[](const auto& b,size_t at){uint16_t n=0;std::memcpy(&n,b.data()+at,2);return n;};
  auto pu64=[](const uint8_t* b,size_t at){uint64_t n=0;std::memcpy(&n,b+at,8);return n;};
  auto pu32=[](const uint8_t* b,size_t at){uint32_t n=0;std::memcpy(&n,b+at,4);return n;};
  auto pu16=[](const uint8_t* b,size_t at){uint16_t n=0;std::memcpy(&n,b+at,2);return n;};
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
    // 1.15.0 state counters: production +0xD0, consumption +0x80. Both have the
    // same 0x30 header and good-count masks as 1.13.11 (verified live on the
    // current save); the old +0xC8/+0x78 pair moved by the inserted state field.
    if(!counter(object,0xd0,production) || !counter(object,0x80,consumption))
      return fail("invalid domestic goods counter");
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
  uint64_t root=0,machine=0,world=0,manager=0,type=0,provinces=0;
  uint32_t provinceCap=0,provinceCount=0;
  if(!field(module,0x05c07c68,root) || !field(root,0,type) || type!=module+0x04660618 ||
     !field(root,0x608,machine) || !field(machine,0,type) || type!=module+0x04723c88 ||
     !field(machine,0x120,world) || !field(world,0,type) || type!=module+0x0468ef80 ||
     !field(machine,0xb0,provinces) || !field(machine,0xb8,provinceCap) ||
     !field(machine,0xbc,provinceCount) || provinceCount>provinceCap || provinceCap>131072 ||
     !provinceCount || world>UINT64_MAX-0x5028)
    return fail("current domestic travel/province manager unavailable");
  manager=world+0x5028;
  if(!field(manager,0,type) || type!=module+0x046b1d48)return fail("invalid current travel network manager");
  std::map<uint16_t,uint32_t> provinceStates;
  auto provinceState=[&](uint16_t id)->std::optional<uint32_t>{
    if(!id || id>=provinceCount)return {};
    if(auto found=provinceStates.find(id);found!=provinceStates.end()){
      if(found->second==UINT32_MAX)return {};
      return found->second;
    }
    uint64_t object=0,vt=0;int32_t state=-1;
    if(!field(provinces,uint64_t(id)*8,object) || !object || !field(object,0,vt) ||
       vt!=module+0x046ac1c0 || !field(object,0x18,state) || state==-1 ||
       !markets.count(uint32_t(state))){provinceStates.emplace(id,UINT32_MAX);return {};}
    provinceStates.emplace(id,uint32_t(state));
    return uint32_t(state);
  };
  auto array=[&](uint64_t address,uint64_t& data,uint32_t& cap,uint32_t& count){
    uint64_t alloc=0;
    return field(address,0,data) && field(address,8,cap) && field(address,0xc,count) &&
        field(address,0x10,alloc) && count<=cap && cap<=200000 && (count==0 || data);
  };
  struct NodeDef {uint16_t flags=0;uint32_t anchor=0;uint16_t province=0;uint16_t linked=0;
    uint32_t id=0;std::array<int64_t,2> position{};};
  std::map<uint64_t,NodeDef> nodeDefs;
  auto nodeDef=[&](uint64_t address,NodeDef& out){
    if(auto found=nodeDefs.find(address);found!=nodeDefs.end()){out=found->second;return true;}
    std::array<uint8_t,0x38> row{};
    if(!address || !read(address,row.data(),row.size()))return false;
    NodeDef copy;copy.flags=u16(row,4);copy.anchor=u32(row,8);
    copy.province=u16(row,0xc);copy.linked=u16(row,0xe);
    copy.id=u32(row,0);std::memcpy(copy.position.data(),row.data()+0x10,16);
    if(copy.anchor==UINT32_MAX || copy.province==UINT16_MAX)return false;
    nodeDefs.emplace(address,copy);out=copy;return true;
  };
  auto runtimeNode=[&](uint64_t definition,uint64_t runtime,NodeDef& n){
    if(!nodeDef(definition,n))return false;
    uint64_t data=0;uint32_t cap=0,count=0;
    if(!array(manager+((n.flags&0x380)?0x40:0x10),data,cap,count) || n.id>=count ||
       uint64_t(n.id)*0x30>UINT64_MAX-data || runtime!=data+uint64_t(n.id)*0x30)return false;
    std::array<uint8_t,0x10> row{};
    return read(runtime,row.data(),row.size()) && u32(row,0)==n.id &&
        u16(row,6)==n.province && u64(row,8)==definition;
  };
  std::map<uint32_t,uint32_t> hubs;
  auto hub=[&](uint32_t state,uint32_t anchor){
    const auto [where,inserted]=hubs.emplace(state,anchor);
    return inserted || where->second==anchor;
  };
  uint64_t data=0;uint32_t cap=0,count=0;
  if(!array(manager+0x28,data,cap,count))return fail("invalid domestic land connection table");
  std::vector<uint8_t> rows(size_t(count)*0x40);
  if(count && !read(data,rows.data(),rows.size()))return fail("unreadable domestic land connections");
  for(uint32_t i=0;i<count;++i){
    const uint8_t* row=rows.data()+size_t(i)*0x40;
    const uint32_t id=pu32(row,0);
    const uint16_t rowFlags=pu16(row,4);
    std::array<uint8_t,0x28> def{};
    if(!read(pu64(row,8),def.data(),def.size()))return fail("unreadable domestic land definition");
    const uint32_t kind=def[4];
    const uint64_t key=u64(def,0x20);
    if(key==UINT64_MAX){
      if(kind!=1 || (row[4]!=2 && row[4]!=3))continue;
      // CA8990 allows general transit only with current corridor references.
      // Unreferenced legs need the original query's source/destination areas;
      // they cannot be inserted into this context-free model graph.
      const uint32_t capacity=pu32(row,0x30),references=pu32(row,0x34);
      if(references>capacity || capacity>200000)return fail("invalid keyless land corridor references");
      if(!references)continue;
      std::vector<uint32_t> owners(references);
      if(!read(pu64(row,0x28),owners.data(),owners.size()*4))return fail("unreadable keyless land corridor references");
      NodeDef a{},b{};
      if(id!=i || u32(def,0)!=id || !runtimeNode(u64(def,8),pu64(row,0x10),a) ||
         !runtimeNode(u64(def,0x10),pu64(row,0x18),b) || (a.flags&0x380) || (b.flags&0x380))
        return fail("invalid current keyless land endpoints");
      const auto source=provinceState(a.province),destination=provinceState(b.province);
      if(!source || !destination || a.anchor>0x1fffffff || b.anchor>0x1fffffff)
        return fail("invalid keyless land owner or control");
      NativeDirectLandLeg leg;leg.positions={a.position,b.position};
      std::memcpy(&leg.distanceFixed,def.data()+0x18,8);
      if(leg.distanceFixed<0)return fail("negative original keyless land distance");
      for(size_t side=0;side<2;++side){
        const auto& n=side?b:a;
        leg.anchors[side]=n.anchor==0x1fffffff?kNativeAnonymousLandNode|n.id:n.anchor;
        if((n.flags&1) && (n.anchor==0x1fffffff || !hub(side?*destination:*source,n.anchor)))
          return fail("invalid current keyless primary land hub");
      }
      NativeLandConnection copy{id,*source,*destination,row[4],key,true,leg};
      if(*source==*destination)result.localLand.push_back(copy);
      else if(markets.at(*source)==markets.at(*destination))result.land.push_back(copy);
      continue;
    }
    if(kind==1){if((key&63)!=0)continue;}
    else if(kind==2){if((key&63)!=1 || rowFlags!=4)continue;} // only current built rail
    else continue;
    NodeDef a{},b{};
    if(!nodeDef(u64(def,8),a) || !nodeDef(u64(def,0x10),b))
      return fail("invalid domestic land node definition");
    const uint32_t start=uint32_t(key>>6)&0x1fffffff,end=uint32_t(key>>35);
    bool forward;
    if(a.anchor==start && b.anchor==end)forward=true;
    else if(a.anchor==end && b.anchor==start)forward=false;
    else return fail("domestic land spline endpoints differ");
    const auto source=provinceState(a.province),destination=provinceState(b.province);
    if(!source || !destination)return fail("domestic travel endpoint province/state mismatch");
    if((a.flags&1) && !hub(*source,a.anchor))return fail("ambiguous current primary land hub");
    if((b.flags&1) && !hub(*destination,b.anchor))return fail("ambiguous current primary land hub");
    NativeLandConnection copy{id,*source,*destination,kind==2?4u:2u,key,forward};
    const bool modeled=*source!=*destination && markets.at(*source)==markets.at(*destination);
    const bool local=*source==*destination;
    if(kind==2)result.builtRails.push_back(copy);
    if(modeled)result.land.push_back(copy);
    if(local)result.localLand.push_back(copy);
  }
  if(!array(manager+0x58,data,cap,count))return fail("invalid domestic port transfer table");
  rows.assign(size_t(count)*0x40,0);
  if(count && !read(data,rows.data(),rows.size()))return fail("unreadable domestic port transfers");
  std::set<uint32_t> transferIds;
  for(uint32_t i=0;i<count;++i){
    const uint8_t* row=rows.data()+size_t(i)*0x40;
    const uint32_t id=pu32(row,0);
    std::array<uint8_t,0x28> def{};
    if(!read(pu64(row,8),def.data(),def.size()))return fail("unreadable domestic port transfer definition");
    if(def[4]==6){
      // CA8550 traverses this actual cross-network leg; CA7D40 emits its
      // definition endpoints/distance unchanged when the spline key is absent.
      if(id!=i || u32(def,0)!=id || row[4]!=5 || u64(def,0x20)!=UINT64_MAX)
        return fail("invalid current keyless port access identity");
      std::array<NodeDef,2> nodes;
      for(size_t side=0;side<2;++side){
        const uint64_t definition=u64(def,8+side*8);
        if(!runtimeNode(definition,pu64(row,0x10+side*8),nodes[side]))
          return fail("port access endpoint identity differs");
      }
      const bool firstLand=(nodes[0].flags&0x7f)==2 && !(nodes[0].flags&0x380);
      const bool secondLand=(nodes[1].flags&0x7f)==2 && !(nodes[1].flags&0x380);
      if(!((firstLand && (nodes[1].flags&0x380)==0x200) ||
           (secondLand && (nodes[0].flags&0x380)==0x200)))
        return fail("unsupported current port access roles");
      const auto& land=nodes[firstLand?0:1];const auto& sea=nodes[firstLand?1:0];
      const auto state=provinceState(land.province);
      int64_t distance=0;std::memcpy(&distance,def.data()+0x18,8);
      if(!state || land.anchor>=0x1fffffff || sea.anchor>=0x1fffffff ||
         land.anchor==sea.anchor || distance<0)
        return fail("invalid current port access state, controls or distance");
      result.portAccess.push_back({id,*state,{land.anchor,sea.anchor},{land.position,sea.position},distance});
      continue;
    }
    if(def[4]!=5)continue;
    const uint64_t key=u64(def,0x20);
    if(key==UINT64_MAX || (key&63)!=3)return fail("invalid current port transfer spline");
    NodeDef a{},b{};
    if(!nodeDef(u64(def,8),a) || !nodeDef(u64(def,0x10),b))
      return fail("invalid domestic port transfer node");
    const auto first=provinceState(a.linked),second=provinceState(b.linked);
    if(bool(first)==bool(second))return fail("ambiguous domestic port transfer state");
    const uint32_t state=first?*first:*second;
    const uint32_t start=uint32_t(key>>6)&0x1fffffff,end=uint32_t(key>>35);
    if(!((a.anchor==start && b.anchor==end) || (b.anchor==start && a.anchor==end)))
      return fail("domestic port transfer anchors differ from spline");
    // The endpoint with the linked land province is the port side; the other is
    // the sea-side node used by the lane. Keep the raw spline anchors for the
    // geometry chain; the separate kind6 leg joins the actual land network.
    uint32_t landAnchor=first?a.anchor:b.anchor,seaAnchor=first?b.anchor:a.anchor;
    if(landAnchor==seaAnchor || !transferIds.insert(id).second)
      return fail("invalid owned native port transfer");
    result.portTransfers.push_back({id,state,{landAnchor,seaAnchor},key,true});
  }
  for(const auto& [state,anchor]:hubs)result.landHubs.emplace(state,anchor);
  if(!window.Unchanged())return fail("domestic sources changed during copy");
  if(!current())return fail("retired domestic sources");
  if(error)error->clear();return result;
}
} // namespace v3
