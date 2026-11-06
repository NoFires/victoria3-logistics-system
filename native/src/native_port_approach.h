#pragma once
#include "native_travel.h"
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
  if(!module || module>UINT64_MAX-0x05c42c28 || state==UINT32_MAX || !logicalProvince ||
     logicalProvince==UINT16_MAX || !current())return fail("missing current port approach identity");
  EconomyReadWindow window(reader);
  GoodsMemoryReader read=[&](uint64_t a,void* p,size_t n){return window.Read(a,p,n);};
  auto field=[&](uint64_t p,uint64_t off,auto& value){return p && off<=UINT64_MAX-p && read(p+off,&value,sizeof(value));};
  auto region=[&](uint64_t p,uint32_t& id,uint32_t& geo){
    uint64_t vt=0;uint8_t valid=0;
    return field(p,0,vt) && vt==module+0x046ba948 && field(p,0x10,id) && id!=UINT32_MAX &&
        field(p,0x40,geo) && geo!=UINT32_MAX && field(p,0x44,valid) && valid==1;
  };
  NativePortApproaches out;out.state=state;
  const uint64_t object=ReadNativeStateObject(read,module,state);
  const auto selectedMarket=object?DecodeNativeStateMarket(read,module,object,state):std::optional<uint32_t>{};
  uint64_t stateRegion=0;uint32_t stateDefinition=0,stateGeo=0;
  if(!object || !field(object,0xf10,stateRegion) || !region(stateRegion,stateDefinition,stateGeo) ||
     !selectedMarket)return fail("invalid selected port state");
  out.market=*selectedMarket;
  uint64_t root=0,machine=0,vt=0,provinces=0,map=0,definitions=0;
  uint32_t provinceCap=0,provinceCount=0,definitionCap=0,definitionCount=0;
  if(!field(module,0x05c07c68,root) || !field(root,0x608,machine) ||
     !field(machine,0xb0,provinces) || !field(machine,0xb8,provinceCap) || !field(machine,0xbc,provinceCount) ||
     provinceCount>provinceCap || provinceCap>131072 ||
     !field(module,0x05bd7308,map) || !field(map,0,vt) || vt!=module+0x046d6850 ||
     !field(map,0x2a0,definitions) || !field(map,0x2a8,definitionCap) || !field(map,0x2ac,definitionCount) ||
     !definitionCount || definitionCount>definitionCap || definitionCount>65535 || definitionCap>131072)
    return fail("invalid current port province context");
  auto sea=[&](uint16_t id,uint64_t expected,uint32_t& definition,uint32_t& geo){
    uint64_t p=0,type=0,r=0;uint8_t excluded=0;
    return id && id!=UINT16_MAX && id<definitionCount && field(definitions,uint64_t(id)*8,p) &&
        field(p,0,type) && type==module+0x046ab568 && field(p,0xc8,r) && (!expected || r==expected) &&
        region(r,definition,geo) && field(r,0x1c8,excluded) && !excluded;
  };
  uint32_t logicalDefinition=0,logicalGeo=0;
  if(!sea(logicalProvince,0,logicalDefinition,logicalGeo))return fail("invalid selected sea province");
  auto travel=DecodeNativeTravelNetwork(read,module,current,error);if(!travel)return {};
  auto node=[&](const NativeTravelEndpoint& endpoint)->const NativeTravelNode&{
    return travel->nodes[endpoint.naval][endpoint.index];
  };
  std::map<uint32_t,std::vector<uint32_t>> physicalPorts;
  for(const auto& link:travel->connections[1]){
    if(link.kind!=6 || link.development!=5 || link.spline!=UINT64_MAX)continue;
    for(size_t side=0;side<2;++side){
      const auto& port=link.endpoints[side];const auto& land=link.endpoints[1-side];
      if(port.naval && !land.naval && (node(port).roles&0x380)==0x200 && (node(land).roles&0x7f)==2)
        physicalPorts[port.index].push_back(land.index);
    }
  }
  for(const auto& link:travel->connections[1]){
    if(link.kind!=5 || link.development!=5)continue;
    if((link.spline&63)!=3 || !link.endpoints[0].naval || !link.endpoints[1].naval)
      return fail("invalid current shore connection");
    const size_t port=(node(link.endpoints[0]).roles&0x380)==0x200?0:1,water=1-port;
    const auto& portNode=node(link.endpoints[port]);const auto& waterNode=node(link.endpoints[water]);
    if((portNode.roles&0x380)!=0x200 || (waterNode.roles&0x380)!=0x80)
      return fail("invalid current shore node roles");
    const auto physical=physicalPorts.find(link.endpoints[port].index);
    if(physical==physicalPorts.end())return fail("current shore has no physical port binding");
    for(uint32_t land:physical->second){
      if(!current())return fail("retired physical port source");
      const auto& landNode=travel->nodes[0][land];uint64_t ownerProvince=0,provinceDefinition=0,ownerRegion=0;
      uint32_t owner=UINT32_MAX,id=0,geo=0;
      if(!landNode.province || landNode.province==UINT16_MAX || landNode.province>=provinceCount ||
         !field(provinces,uint64_t(landNode.province)*8,ownerProvince) || !field(ownerProvince,0,vt) || vt!=module+0x046ac1c0 ||
         !field(ownerProvince,8,provinceDefinition) || !field(provinceDefinition,0,vt) || vt!=module+0x046ab568 ||
         !field(provinceDefinition,0xc8,ownerRegion) || !region(ownerRegion,id,geo))
        return fail("invalid current physical port province");
      if(ownerRegion!=stateRegion){
        if(id==stateDefinition && geo==stateGeo)return fail("ambiguous port state region identity");
        continue;
      }
      if(!field(ownerProvince,0x18,owner))return fail("invalid port province owner");
      const auto ownerObject=ReadNativeStateObject(read,module,owner);
      const auto market=ownerObject?DecodeNativeStateMarket(read,module,ownerObject,owner):std::optional<uint32_t>{};
      uint64_t actualRegion=0;
      if(!ownerObject || !field(ownerObject,0xf10,actualRegion) || actualRegion!=stateRegion || !market)
        return fail("stale port owning state");
      if(*market!=out.market){++out.foreignMarketShores;continue;}
      uint64_t waterDefinition=0,physicalRegion=0,data=0;uint32_t capacity=0,count=0;
      NativePortApproach candidate;candidate.connection=link.id;candidate.ownerState=owner;candidate.market=*market;
      candidate.stateDefinition=stateDefinition;candidate.stateGeo=stateGeo;candidate.logicalProvince=logicalProvince;
      candidate.stateAnchor=portNode.control;candidate.seaAnchor=waterNode.control;candidate.spline=link.spline;
      // Original DCB6B0 -> 18F6AA0 uses the first member of a valid,
      // non-excluded sea region as its canonical province, not as a port choice.
      if(!waterNode.province || waterNode.province==UINT16_MAX || waterNode.province>=definitionCount ||
         !field(definitions,uint64_t(waterNode.province)*8,waterDefinition) ||
         !field(waterDefinition,0,vt) || vt!=module+0x046ab568 || !field(waterDefinition,0xc8,physicalRegion) ||
         !field(physicalRegion,0x60,data) || !field(physicalRegion,0x68,capacity) || !field(physicalRegion,0x6c,count) ||
         !count || count>capacity || capacity>65535 || !data || uint64_t(capacity)*2>UINT64_MAX-data ||
         !field(data,0,candidate.physicalProvince) ||
         !sea(candidate.physicalProvince,physicalRegion,candidate.seaDefinition,candidate.seaGeo) ||
         candidate.stateAnchor>=0x1fffffff || candidate.seaAnchor>=0x1fffffff || candidate.stateAnchor==candidate.seaAnchor)
        return fail("invalid physical port sea binding");
      const uint32_t first=uint32_t(link.spline>>6)&0x1fffffff,last=uint32_t(link.spline>>35);
      if(!((first==candidate.stateAnchor && last==candidate.seaAnchor) ||
           (last==candidate.stateAnchor && first==candidate.seaAnchor)))return fail("shore spline and current controls differ");
      ++out.eligibleShores;
      if(candidate.seaDefinition!=logicalDefinition || candidate.seaGeo!=logicalGeo)out.queries.push_back(candidate);
    }
  }
  if(!window.Unchanged() || !current())return fail("port approach source changed or retired");
  if(error)error->clear();return out;
}
} // namespace v3
