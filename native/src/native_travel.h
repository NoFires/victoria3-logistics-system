#pragma once
#include "native_economy.h"

namespace v3 {
struct NativeTravelNode {
  uint32_t id=0,control=0,area=UINT32_MAX;
  uint16_t province=UINT16_MAX,roles=0;
  uint8_t flags=0;
  std::array<int64_t,2> position{}; // current definition, fixed1e5
  bool operator==(const NativeTravelNode&)const=default;
};
struct NativeTravelEndpoint {
  uint32_t index=0;
  bool naval=false;
  bool operator==(const NativeTravelEndpoint&)const=default;
};
struct NativeTravelConnection {
  uint32_t id=0;
  uint8_t kind=0,development=0,flags=0;
  uint64_t spline=UINT64_MAX;
  int64_t distanceFixed=0,costFixed=0;
  std::array<NativeTravelEndpoint,2> endpoints{};
  bool operator==(const NativeTravelConnection&)const=default;
};
struct NativeTravelNetwork {
  // Original array identity is separate from each array-local native ID.
  std::array<std::vector<NativeTravelNode>,2> nodes;
  std::array<std::vector<NativeTravelConnection>,2> connections;
};

// 1.15.0: world+5028, CNode stride30, CConnection stride40; their definitions
// are plain structs, not the removed CTravelNode/CTravelConnection objects.
// Caller owns the native access window. No engine call, route search, state-ID
// mapping or geometry fallback runs here; only owned numeric values escape.
template<class Reader,class Current>
std::optional<NativeTravelNetwork> DecodeNativeTravelNetwork(const Reader& reader,uint64_t module,
    const Current& current,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<NativeTravelNetwork>{if(error)*error=why;return {};};
  if(!module || module>UINT64_MAX-0x05c07c70 || !current())return fail("missing current travel identity");
  EconomyReadWindow window(reader);
  auto read=[&](uint64_t at,void* p,size_t n){return window.Read(at,p,n);};
  auto field=[&](uint64_t p,uint64_t off,auto& v){return p && off<=UINT64_MAX-p && read(p+off,&v,sizeof(v));};
  auto u64=[](const auto& b,size_t at){uint64_t n=0;std::memcpy(&n,b.data()+at,8);return n;};
  auto u32=[](const auto& b,size_t at){uint32_t n=0;std::memcpy(&n,b.data()+at,4);return n;};
  auto u16=[](const auto& b,size_t at){uint16_t n=0;std::memcpy(&n,b.data()+at,2);return n;};
  uint64_t root=0,machine=0,world=0,type=0;
  if(!field(module,0x05c07c68,root) || !field(root,0,type) || type!=module+0x04660618 ||
     !field(root,0x608,machine) || !field(machine,0x120,world) || world>UINT64_MAX-0x50a0 ||
     !field(world,0x5028,type) || type!=module+0x046b1d48)return fail("current travel manager unavailable");
  NativeTravelNetwork result;
  std::array<uint64_t,2> nodeData{};
  std::array<std::vector<uint64_t>,2> definitions;
  auto array=[&](uint64_t offset,size_t stride,std::vector<uint8_t>& rows,uint64_t& data){
    std::array<uint8_t,0x18> h{};
    if(!read(world+0x5028+offset,h.data(),h.size()))return false;
    const uint32_t cap=u32(h,8),count=u32(h,12);data=u64(h,0);
    if(count>cap || cap>131072 || (cap && !data) || uint64_t(count)*stride>UINT64_MAX-data)return false;
    rows.resize(size_t(count)*stride);return rows.empty() || read(data,rows.data(),rows.size());
  };
  for(size_t naval=0;naval<2;++naval){
    std::vector<uint8_t> rows;
    if(!array(naval?0x40:0x10,0x30,rows,nodeData[naval]))return fail("invalid current travel node array");
    for(size_t at=0;at<rows.size();at+=0x30){
      if(!current())return fail("retired travel nodes");
      const uint64_t definition=u64(rows,at+8);std::array<uint8_t,0x20> d{};
      if(!read(definition,d.data(),d.size()) || u32(rows,at)!=at/0x30 || u32(d,0)!=u32(rows,at) ||
         u16(rows,at+6)!=u16(d,0xc) || bool(u16(d,4)&0x380)!=bool(naval))
        return fail("travel node identity or array mismatch");
      NativeTravelNode node;node.id=u32(rows,at);node.control=u32(d,8);node.roles=u16(d,4);
      node.province=u16(d,0xc);node.flags=rows[at+4];node.area=u32(rows,at+0x10);
      std::memcpy(node.position.data(),d.data()+0x10,16);
      result.nodes[naval].push_back(node);definitions[naval].push_back(definition);
    }
  }
  for(size_t naval=0;naval<2;++naval){
    std::vector<uint8_t> rows;uint64_t data=0;
    if(!array(naval?0x58:0x28,0x40,rows,data))return fail("invalid current travel connection array");
    for(size_t at=0;at<rows.size();at+=0x40){
      if(!current())return fail("retired travel connections");
      std::array<uint8_t,0x28> d{};
      if(!read(u64(rows,at+8),d.data(),d.size()) || u32(rows,at)!=at/0x40 || u32(d,0)!=u32(rows,at) ||
         !d[4] || d[4]>8 || rows[at+4]>6)return fail("invalid current travel connection identity");
      NativeTravelConnection connection;connection.id=u32(rows,at);connection.kind=d[4];
      connection.development=rows[at+4];connection.flags=rows[at+5];connection.spline=u64(d,0x20);
      std::memcpy(&connection.distanceFixed,d.data()+0x18,8);
      std::memcpy(&connection.costFixed,rows.data()+at+0x20,8);
      if(connection.distanceFixed<0 || connection.costFixed<0)return fail("negative current travel distance or cost");
      for(size_t side=0;side<2;++side){
        const uint64_t definition=u64(d,8+side*8);std::array<uint8_t,8> node{};
        if(!read(definition,node.data(),node.size()))return fail("unreadable travel endpoint definition");
        const size_t arrayIndex=bool(u16(node,4)&0x380);const uint32_t index=u32(node,0);
        if(index>=definitions[arrayIndex].size() || definitions[arrayIndex][index]!=definition ||
           u64(rows,at+0x10+side*8)!=nodeData[arrayIndex]+uint64_t(index)*0x30)
          return fail("travel endpoint does not match its current node array");
        connection.endpoints[side]={index,bool(arrayIndex)};
      }
      result.connections[naval].push_back(connection);
    }
  }
  if(!window.Unchanged())return fail("travel network changed during copy");
  if(!current())return fail("retired travel network");
  if(error)error->clear();return result;
}
} // namespace v3
