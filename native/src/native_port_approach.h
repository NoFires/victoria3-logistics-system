#pragma once
#include "native_economy.h"
#include <set>

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
// Current selected state -> actual typed port connection -> physical sea.
// This is input metadata, not a native route or a choice between ports. Keep
// all eligible candidates and the actual owner when a region is split.
template<class Reader,class Current>
std::optional<NativePortApproaches> DecodeNativePortApproaches(const Reader& reader,uint64_t module,
    uint32_t state,uint16_t logicalProvince,const Current& current,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<NativePortApproaches>{if(error)*error=why;return {};};
  if(!module || module>UINT64_MAX-0x05964ef8 || state==UINT32_MAX || !logicalProvince ||
     logicalProvince==UINT16_MAX || !current())return fail("missing current port approach identity");
  EconomyReadWindow window(reader);
  GoodsMemoryReader read=[&](uint64_t a,void* p,size_t n){return window.Read(a,p,n);};
  auto field=[&](uint64_t p,uint64_t off,auto& value){return p && off<=UINT64_MAX-p && read(p+off,&value,sizeof(value));};
  auto u64=[](const auto& b,size_t at){uint64_t n=0;std::memcpy(&n,b.data()+at,8);return n;};
  auto u32=[](const auto& b,size_t at){uint32_t n=0;std::memcpy(&n,b.data()+at,4);return n;};
  auto u16=[](const auto& b,size_t at){uint16_t n=0;std::memcpy(&n,b.data()+at,2);return n;};
  auto region=[&](uint64_t p,uint32_t& id,uint32_t& geo){
    uint64_t vt=0;uint8_t valid=0;
    return field(p,0,vt) && vt==module+0x044e7850 && field(p,0x10,id) && id!=UINT32_MAX &&
        field(p,0x40,geo) && geo!=UINT32_MAX && field(p,0x44,valid) && valid==1;
  };
  NativePortApproaches out;out.state=state;
  const uint64_t object=ReadNativeStateObject(read,module,state);
  const auto selectedMarket=object?DecodeNativeStateMarket(read,module,object,state):std::optional<uint32_t>{};
  uint64_t stateRegion=0;uint32_t stateDefinition=0,stateGeo=0;
  if(!object || !field(object,0xe78,stateRegion) || !region(stateRegion,stateDefinition,stateGeo) ||
     !selectedMarket)return fail("invalid selected port state");
  out.market=*selectedMarket;
  uint64_t root=0,machine=0,world=0,vt=0,provinces=0,map=0,definitions=0;
  uint32_t provinceCap=0,provinceCount=0,definitionCap=0,definitionCount=0;
  if(!field(module,0x0592d128,root) || !field(root,0x608,machine) || !field(machine,0x120,world) ||
     !field(machine,0xb0,provinces) || !field(machine,0xb8,provinceCap) || !field(machine,0xbc,provinceCount) ||
     provinceCount>provinceCap || provinceCap>131072 ||
     !field(module,0x058fccf0,map) || !field(map,0,vt) || vt!=module+0x0441c938 ||
     !field(map,0x2a0,definitions) || !field(map,0x2a8,definitionCap) || !field(map,0x2ac,definitionCount) ||
     !definitionCount || definitionCount>definitionCap || definitionCount>65535 || definitionCap>131072)
    return fail("invalid current port province context");
  auto sea=[&](uint16_t id,uint64_t expected,uint32_t& definition,uint32_t& geo){
    uint64_t p=0,type=0,r=0;uint8_t excluded=0;
    return id && id!=UINT16_MAX && id<definitionCount && field(definitions,uint64_t(id)*8,p) &&
        field(p,0,type) && type==module+0x044dc488 && field(p,0xc8,r) && (!expected || r==expected) &&
        region(r,definition,geo) && field(r,0x1c8,excluded) && !excluded;
  };
  uint32_t logicalDefinition=0,logicalGeo=0;
  if(!sea(logicalProvince,0,logicalDefinition,logicalGeo))return fail("invalid selected sea province");
  std::array<uint8_t,24> header{};
  if(!field(world,0x5618,vt) || vt!=module+0x044bb380 || world>UINT64_MAX-0x5698 ||
     !read(world+0x5680,header.data(),header.size()) || u32(header,12)>u32(header,8) || u32(header,8)>131072)
    return fail("invalid current port connection roster");
  std::vector<uint64_t> roster(u32(header,12));
  if(!roster.empty() && !read(u64(header,0),roster.data(),roster.size()*8))return fail("unreadable port connection roster");
  std::set<uint32_t> seen;
  for(uint64_t p:roster){
    std::array<uint8_t,0x38> row{};
    if(!read(p,row.data(),row.size()) || u64(row,0)!=module+0x044e08d8 ||
       u32(row,8)==UINT32_MAX || !seen.insert(u32(row,8)).second || u32(row,0x20)>7)
      return fail("invalid current port connection");
    if(u32(row,0x20)!=5)continue;
    std::array<uint8_t,0x60> def{};
    if(!read(u64(row,0x18),def.data(),def.size()) || u64(def,0)!=module+0x04515e20 || u32(def,0x10)==UINT32_MAX)
      return fail("invalid current port definition");
    if(u32(def,0x58)!=4)continue;
    std::array<std::array<uint8_t,0x68>,2> nodes{};
    for(size_t i=0;i<2;++i)
      if(!read(u64(def,0x40+i*8),nodes[i].data(),nodes[i].size()) || u64(nodes[i],0)!=module+0x044f8a78)
        return fail("invalid port node definition");
    const size_t land=u64(nodes[0],0x60)==0x400?0:1,water=1-land;
    if(u64(nodes[land],0x60)!=0x400 || (u64(nodes[water],0x60)!=0x100 && u64(nodes[water],0x60)!=0x200))
      return fail("invalid port node roles");
    if(u64(nodes[land],0x58)!=stateRegion){
      uint32_t id=0,geo=0;
      if(!region(u64(nodes[land],0x58),id,geo) || (id==stateDefinition && geo==stateGeo))
        return fail("ambiguous port state region identity");
      continue;
    }
    for(size_t i=0;i<2;++i){
      std::array<uint8_t,0x80> runtime{};
      if(!read(u64(row,0x28+i*8),runtime.data(),runtime.size()) || u64(runtime,0)!=module+0x044e3d78 ||
         u64(runtime,0x78)!=u64(def,0x40+i*8))return fail("stale port runtime node");
    }
    const uint16_t province=u16(nodes[land],0x44);
    uint64_t ownerProvince=0;uint32_t owner=UINT32_MAX;
    if(!province || province==UINT16_MAX || province>=provinceCount ||
       !field(provinces,uint64_t(province)*8,ownerProvince) || !field(ownerProvince,0,vt) || vt!=module+0x04437a00 ||
       !field(ownerProvince,0x18,owner))return fail("invalid port province owner");
    const auto ownerObject=ReadNativeStateObject(read,module,owner);uint64_t ownerRegion=0;
    const auto market=ownerObject?DecodeNativeStateMarket(read,module,ownerObject,owner):std::optional<uint32_t>{};
    if(!ownerObject || !field(ownerObject,0xe78,ownerRegion) || ownerRegion!=stateRegion ||
       !market)return fail("stale port owning state");
    if(*market!=out.market){++out.foreignMarketShores;continue;}
    const auto physicalRegion=u64(nodes[water],0x58);uint64_t data=0;uint32_t capacity=0,count=0;
    NativePortApproach candidate;candidate.connection=u32(row,8);candidate.ownerState=owner;candidate.market=*market;
    candidate.stateDefinition=stateDefinition;candidate.stateGeo=stateGeo;candidate.logicalProvince=logicalProvince;
    candidate.stateAnchor=u32(nodes[land],0x40);candidate.seaAnchor=u32(nodes[water],0x40);candidate.spline=u64(def,0x50);
    // Original14144FEF0 takes the first member of a valid, non-excluded sea
    // region. This is its canonical representation, not a candidate choice.
    if(!field(physicalRegion,0x60,data) || !field(physicalRegion,0x68,capacity) || !field(physicalRegion,0x6c,count) ||
       !count || count>capacity || capacity>65535 || !data || uint64_t(capacity)*2>UINT64_MAX-data ||
       !field(data,0,candidate.physicalProvince) ||
       !sea(candidate.physicalProvince,physicalRegion,candidate.seaDefinition,candidate.seaGeo) ||
       candidate.stateAnchor>=0x20000000 || candidate.seaAnchor>=0x20000000 ||
       candidate.stateAnchor==candidate.seaAnchor || (candidate.spline&63)!=3)
      return fail("invalid physical port sea binding");
    ++out.eligibleShores;
    if(candidate.seaDefinition!=logicalDefinition || candidate.seaGeo!=logicalGeo)out.queries.push_back(candidate);
  }
  if(!window.Unchanged() || !current())return fail("port approach source changed or retired");
  if(error)error->clear();return out;
}
} // namespace v3
