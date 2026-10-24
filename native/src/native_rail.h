#pragma once
#include "route_data.h"
#include <array>
#include <cstring>
#include <cmath>
#include <optional>
#include <unordered_set>

namespace v3 {
struct NativeRailSnapshot {
  std::unordered_set<uint32_t> builtEdges;
  uint32_t potentialConnections=0,builtConnections=0;
};

// Exact 1.13.11 read-only adapter. Current CTravelConnection type4 is railway;
// its definition's 64-bit spline key binds to format3's physical strip list.
// No runtime connection ID or state ID is treated as a physical strip ID.
template<class Reader,class Current>
std::optional<NativeRailSnapshot> DecodeNativeRails(const Reader& reader,uint64_t module,
    const SplineGraph& graph,const Current& current,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<NativeRailSnapshot>{if(error)*error=why;return {};};
  if(!current() || !module || module>UINT64_MAX-0x0592d128 || graph.nativeConnections.empty())
    return fail("missing current railway/map identity");
  struct Copy {uint64_t address;std::vector<uint8_t> bytes;};
  std::vector<Copy> copies;size_t copied=0;
  auto read=[&](uint64_t a,void* out,size_t n){
    if(!a || n>UINT64_MAX-a || n>16777216-copied || !reader(a,out,n))return false;
    copied+=n;copies.push_back({a,{(uint8_t*)out,(uint8_t*)out+n}});return true;
  };
  auto field=[&](uint64_t p,uint64_t o,auto& v){return p && o<=UINT64_MAX-p && read(p+o,&v,sizeof(v));};
  auto u64=[](const auto& bytes,size_t offset){uint64_t v=0;std::memcpy(&v,bytes.data()+offset,8);return v;};
  auto u32=[](const auto& bytes,size_t offset){uint32_t v=0;std::memcpy(&v,bytes.data()+offset,4);return v;};
  uint64_t root=0,machine=0,world=0,managerType=0;
  if(!field(module,0x0592d128,root) || !field(root,0x608,machine) || !field(machine,0x120,world) ||
     !world || world>UINT64_MAX-0x5690 || !field(world,0x5618,managerType) || managerType!=module+0x044bb380)
    return fail("current travel connection manager unavailable");
  std::array<uint8_t,0x18> header{};
  if(!read(world+0x5680,header.data(),header.size()))return fail("unreadable travel connection roster");
  const uint64_t objects=u64(header,0);const uint32_t capacity=u32(header,8),count=u32(header,12);
  if(!objects || !count || count>capacity || capacity>131072)
    return fail("invalid travel connection roster");
  std::vector<uint64_t> roster(count);
  if(!read(objects,roster.data(),roster.size()*8))return fail("unreadable travel connection entries");
  std::unordered_map<uint32_t,const SplineEdge*> edges;
  for(const auto& edge:graph.edges)if(!edges.emplace(edge.id,&edge).second)return fail("duplicate physical strip");
  std::unordered_set<uint64_t> seenObjects,seenKeys;std::unordered_set<uint32_t> seenIds;
  NativeRailSnapshot result;
  // Native travel positions floor authored float coordinates before fixed-point
  // conversion. Image-Z storage can lose one float ULP at an integer boundary.
  auto samePoint=[&](int64_t x,int64_t z,const SplinePoint& p){
    const double px=p.x,pz=double(graph.height)-p.z;
    if(x%100000 || z%100000 || !std::isfinite(px) || !std::isfinite(pz))return false;
    const double nx=double(x)/100000,nz=double(z)/100000;
    return nx<=px+.001 && px<nx+1.001 && nz<=pz+.001 && pz<nz+1.001;
  };
  for(uint64_t object:roster){
    std::array<uint8_t,0x38> row{};
    if(!seenObjects.insert(object).second || !read(object,row.data(),row.size()) ||
       u64(row,0)!=module+0x044e08d8 || u32(row,8)==UINT32_MAX || !seenIds.insert(u32(row,8)).second ||
       u32(row,0x20)>7)return fail("invalid current travel connection");
    const uint64_t definition=u64(row,0x18);
    std::array<uint8_t,0x60> def{};
    if(!read(definition,def.data(),def.size()) || u64(def,0)!=module+0x04515e20 || u32(def,0x10)==UINT32_MAX)
      return fail("invalid travel connection definition");
    const uint32_t type=u32(row,0x20);
    if(u32(def,0x58)!=1){if(type==4)return fail("railway has no verified spline binding");continue;}
    const uint64_t key=u64(def,0x50);
    auto binding=graph.nativeConnections.find(key);
    if((key&63)!=1 || !seenKeys.insert(key).second || binding==graph.nativeConnections.end() || binding->second.empty())
      return fail("current railway differs from map bindings");
    ++result.potentialConnections;
    std::vector<SplinePoint> points;uint32_t anchor=uint32_t((key>>6)&0x1fffffff);
    for(uint32_t id:binding->second){
      auto it=edges.find(id);if(it==edges.end())return fail("unknown physical railway strip");
      const auto& edge=*it->second;
      if(edge.type!=1 || edge.from>=graph.nodes.size() || edge.to>=graph.nodes.size() || edge.count<2 ||
         uint64_t(edge.first)+edge.count>graph.points.size())return fail("invalid physical railway strip");
      bool forward=graph.nodes[edge.from].id==anchor;
      if(!forward && graph.nodes[edge.to].id!=anchor)return fail("disconnected physical railway binding");
      for(uint32_t j=points.empty()?0:1;j<edge.count;++j)
        points.push_back(graph.points[edge.first+(forward?j:edge.count-1-j)]);
      anchor=graph.nodes[forward?edge.to:edge.from].id;
    }
    if(anchor!=uint32_t(key>>35) || points.size()<2 || points.size()>4096)
      return fail("invalid railway connection endpoints");
    std::array<uint8_t,0x18> pathHeader{};
    if(definition>UINT64_MAX-0x80 || !read(definition+0x68,pathHeader.data(),pathHeader.size()) ||
       u32(pathHeader,12)!=points.size()-1 || u32(pathHeader,12)>u32(pathHeader,8) || u32(pathHeader,8)>4096)
      return fail("railway path count differs from current map");
    std::array<uint8_t,0x80> from{},to{};
    if(!read(u64(row,0x28),from.data(),from.size()) || !read(u64(row,0x30),to.data(),to.size()) ||
       u64(from,0)!=module+0x044e3d78 || u64(to,0)!=module+0x044e3d78 ||
       u64(from,0x78)!=u64(def,0x40) || u64(to,0x78)!=u64(def,0x48))
      return fail("railway endpoint definition changed");
    std::array<int64_t,2> start{};
    if(u64(def,0x40)>UINT64_MAX-0x58 || !read(u64(def,0x40)+0x48,start.data(),sizeof(start)) ||
       !samePoint(start[0],start[1],points.front()))return fail("railway origin differs from map");
    std::vector<std::array<int64_t,3>> path(points.size()-1);
    if(!read(u64(pathHeader,0),path.data(),path.size()*24))return fail("unreadable railway path");
    for(size_t j=0;j<path.size();++j)
      if(!samePoint(path[j][1],path[j][2],points[j+1]))return fail("railway shape differs from map");
    if(type==4){
      ++result.builtConnections;
      for(uint32_t id:binding->second)if(!result.builtEdges.insert(id).second)return fail("duplicate built railway strip");
    }
  }
  for(const auto& [key,refs]:graph.nativeConnections)
    if((key&63)==1 && !seenKeys.count(key))return fail("map railway missing from current world");
  for(const auto& copy:copies){
    std::vector<uint8_t> bytes(copy.bytes.size());
    if(!reader(copy.address,bytes.data(),bytes.size()) || bytes!=copy.bytes)return fail("railway data changed during copy");
  }
  if(!current())return fail("retired railway snapshot");
  if(error)error->clear();return result;
}
}  // namespace v3
