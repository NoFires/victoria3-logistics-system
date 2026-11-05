#pragma once
#include "native_economy.h"

namespace v3 {
struct NativePortApproach {
  uint32_t connection=0,ownerState=0,market=0;
  uint32_t stateDefinition=0,stateGeo=0,seaDefinition=0,seaGeo=0;
  uint32_t stateAnchor=0,seaAnchor=0;
  uint16_t logicalProvince=0,physicalProvince=0;
  uint64_t spline=0;
  bool operator==(const NativePortApproach&)const=default;
};
struct NativePortApproaches {
  uint32_t state=0,market=0,eligibleShores=0,foreignMarketShores=0;
  std::vector<NativePortApproach> queries;
  bool operator==(const NativePortApproaches&)const=default;
};
// Exact 1.15.0 selected state -> current type-5 port access -> the physical sea
// side. Input metadata only: every eligible shore is kept, no port is chosen.
// Reads the current travel network (world+0x5028 naval connections, +0x58),
// the 1.15 province runtime table and the current province-template region.
template<class Reader,class Current>
std::optional<NativePortApproaches> DecodeNativePortApproaches(const Reader& reader,uint64_t module,
    uint32_t state,uint16_t logicalProvince,const Current& current,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<NativePortApproaches>{if(error)*error=why;return {};};
  if(!reader || !module || module>UINT64_MAX-0x05c07c68 || state==UINT32_MAX ||
     !logicalProvince || logicalProvince==UINT16_MAX || !current())
    return fail("missing current port approach identity");
  EconomyReadWindow window(reader);
  GoodsMemoryReader read=[&](uint64_t a,void* p,size_t n){return window.Read(a,p,n);};
  auto field=[&](uint64_t p,uint64_t off,auto& value){
    return p && off<=UINT64_MAX-p && read(p+off,&value,sizeof(value));};
  auto u64=[](const uint8_t* b,size_t at){uint64_t n=0;std::memcpy(&n,b+at,8);return n;};
  auto u32=[](const uint8_t* b,size_t at){uint32_t n=0;std::memcpy(&n,b+at,4);return n;};
  auto u16=[](const uint8_t* b,size_t at){uint16_t n=0;std::memcpy(&n,b+at,2);return n;};
  auto region=[&](uint64_t p,uint32_t& id,uint32_t& geo){
    uint64_t vt=0;uint8_t valid=0;
    return field(p,0,vt) && vt==module+0x046ba948 && field(p,0x10,id) && id!=UINT32_MAX &&
        field(p,0x40,geo) && geo!=UINT32_MAX && field(p,0x44,valid) && valid==1;
  };
  const uint64_t object=ReadNativeStateObject(read,module,state);
  const auto market=object?DecodeNativeStateMarket(read,module,object,state):std::optional<uint32_t>{};
  uint64_t stateRegion=0;uint32_t stateDefinition=0,stateGeo=0;
  if(!object || !market || !field(object,0xf10,stateRegion) ||
     !region(stateRegion,stateDefinition,stateGeo))return fail("invalid selected port state");
  uint64_t root=0,machine=0,world=0,type=0,provinces=0;
  uint32_t provinceCap=0,provinceCount=0;
  if(!field(module,0x05c07c68,root) || !field(root,0,type) || type!=module+0x04660618 ||
     !field(root,0x608,machine) || !field(machine,0,type) || type!=module+0x04723c88 ||
     !field(machine,0x120,world) || !field(world,0,type) || type!=module+0x0468ef80 ||
     !field(machine,0xb0,provinces) || !field(machine,0xb8,provinceCap) ||
     !field(machine,0xbc,provinceCount) || provinceCount>provinceCap || provinceCap>131072 ||
     !provinceCount || world>UINT64_MAX-0x5028)
    return fail("current travel/province manager unavailable");
  const uint64_t manager=world+0x5028;
  if(!field(manager,0,type) || type!=module+0x046b1d48)return fail("invalid current travel network manager");
  // Current province template array (1.15 CGraphicalMap template registry).
  uint64_t templates=0,templateData=0;uint32_t templateCount=0;
  if(!field(module,0x05bd7308,templates) || !field(templates,0x2a0,templateData) ||
     !field(templates,0x2ac,templateCount) || !templateCount || templateCount>131072)
    return fail("current province template registry unavailable");
  auto provinceState=[&](uint16_t id)->std::optional<uint32_t>{
    if(!id || id>=provinceCount)return {};
    uint64_t provinceObject=0,vt=0;int32_t owner=-1;
    if(!field(provinces,uint64_t(id)*8,provinceObject) || !provinceObject ||
       !field(provinceObject,0,vt) || vt!=module+0x046ac1c0 ||
       !field(provinceObject,0x18,owner) || owner==-1)return {};
    return uint32_t(owner);
  };
  auto seaTemplate=[&](uint16_t id,uint32_t& definition,uint32_t& geo)->bool{
    if(!id || id>=templateCount)return false;
    uint64_t object=0,vt=0,seaRegion=0;
    if(!field(templateData,uint64_t(id)*8,object) || !object ||
       !field(object,0,vt) || vt!=module+0x046ab568 ||
       !field(object,0xc8,seaRegion) || !region(seaRegion,definition,geo))return false;
    return true;
  };
  auto array=[&](uint64_t address,uint64_t& data,uint32_t& cap,uint32_t& count){
    uint64_t alloc=0;
    return field(address,0,data) && field(address,8,cap) && field(address,0xc,count) &&
        field(address,0x10,alloc) && count<=cap && cap<=200000 && (count==0 || data);
  };
  struct NodeDef {uint16_t flags=0;uint32_t anchor=0;uint16_t province=0;uint16_t linked=0;};
  auto nodeDef=[&](uint64_t address,NodeDef& out){
    std::array<uint8_t,0x38> row{};
    if(!address || !read(address,row.data(),row.size()))return false;
    out.flags=u16(row.data(),4);out.anchor=u32(row.data(),8);
    out.province=u16(row.data(),0xc);out.linked=u16(row.data(),0xe);
    return out.anchor!=UINT32_MAX && out.province!=UINT16_MAX;
  };
  uint64_t data=0;uint32_t cap=0,count=0;
  if(!array(manager+0x58,data,cap,count))return fail("invalid current port transfer table");
  std::vector<uint8_t> rows(size_t(count)*0x40);
  if(count && !read(data,rows.data(),rows.size()))return fail("unreadable current port transfer table");
  NativePortApproaches out;out.state=state;out.market=*market;
  std::set<uint32_t> ids;
  for(uint32_t i=0;i<count;++i){
    if(!current())return fail("retired current port approach");
    const uint8_t* row=rows.data()+size_t(i)*0x40;
    const uint32_t id=u32(row,0);
    std::array<uint8_t,0x28> def{};
    if(!read(u64(row,8),def.data(),def.size()))return fail("unreadable current port definition");
    if(def[4]!=5)continue;
    const uint64_t key=u64(def.data(),0x20);
    if((key&63)!=3)return fail("invalid current port transfer spline");
    NodeDef a{},b{};
    if(!nodeDef(u64(def.data(),8),a) || !nodeDef(u64(def.data(),0x10),b))
      return fail("invalid current port transfer node");
    const auto first=provinceState(a.linked),second=provinceState(b.linked);
    if(bool(first)==bool(second))return fail("ambiguous current port transfer state");
    const bool landFirst=bool(first);
    const uint32_t landState=uint32_t(first?*first:*second);
    const uint32_t seaProvince=landFirst?b.province:a.province;
    const uint32_t landAnchor=landFirst?a.anchor:b.anchor;
    const uint32_t seaAnchor=landFirst?b.anchor:a.anchor;
    if(landAnchor==seaAnchor || !ids.insert(id).second)
      return fail("invalid current port transfer identity");
    if(landState!=state)continue;
    uint32_t seaDefinition=0,seaGeo=0;
    if(!seaTemplate(uint16_t(seaProvince),seaDefinition,seaGeo))
      return fail("current port physical sea region unavailable");
    if(seaDefinition==stateDefinition && seaGeo==stateGeo)++out.eligibleShores;
    else{
      ++out.eligibleShores;
      NativePortApproach query;
      query.connection=id;query.ownerState=landState;query.market=*market;
      query.stateDefinition=stateDefinition;query.stateGeo=stateGeo;
      query.seaDefinition=seaDefinition;query.seaGeo=seaGeo;
      query.stateAnchor=landAnchor;query.seaAnchor=seaAnchor;
      query.logicalProvince=uint16_t(landFirst?a.linked:b.linked);
      query.physicalProvince=uint16_t(seaProvince);
      query.spline=key;
      out.queries.push_back(query);
    }
  }
  if(!window.Unchanged())return fail("port approach changed during copy");
  if(!current())return fail("retired current port approach");
  if(error)error->clear();return out;
}
} // namespace v3
