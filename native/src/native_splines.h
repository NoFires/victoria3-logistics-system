#pragma once
#include "native_economy.h"
#include <cmath>
#include <map>
#include <set>

namespace v3 {
struct NativeSplinePoint {float x=0,z=0;}; // original bottom-origin coordinates
struct NativeSplineNetwork {
  uint32_t width=0,height=0;
  std::map<uint32_t,NativeSplinePoint> points; // native anchor ID, low 29 bits
  std::map<uint64_t,std::vector<uint32_t>> strips; // native segment ID, low 56 bits
  std::map<uint64_t,std::vector<uint64_t>> connections; // native full connection key
};

// Exact-build CJominiSplineNetworkDatabase data, reached by the original sea
// highlighter. Copies current native geometry, not the embedded vanilla map.
// Caller owns its verified access window; no getter/highlight/editor call.
template<class Reader,class Current>
std::optional<NativeSplineNetwork> DecodeNativeSplineNetwork(const Reader& reader,uint64_t module,
    const Current& current,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<NativeSplineNetwork>{if(error)*error=why;return {};};
  if(!module || module>UINT64_MAX-0x05901fa8 || !current())return fail("missing current native spline source");
  EconomyReadWindow window(reader);
  auto read=[&](uint64_t at,void* p,size_t n){return window.Read(at,p,n);};
  auto field=[&](uint64_t p,uint64_t off,auto& value){return p && off<=UINT64_MAX-p && read(p+off,&value,sizeof(value));};
  auto u64=[](const auto& bytes,size_t at){uint64_t n=0;std::memcpy(&n,bytes.data()+at,8);return n;};
  auto u32=[](const auto& bytes,size_t at){uint32_t n=0;std::memcpy(&n,bytes.data()+at,4);return n;};
  uint64_t graphics=0,type=0,database=0,templates=0,pixels=0;
  NativeSplineNetwork result;
  if(!field(module,0x05901fa0,graphics) || !field(graphics,0,type) || type!=module+0x0464e4e8 ||
     !field(graphics,0x2c0,database) || !database || database>UINT64_MAX-0x78 ||
     !field(module,0x058fccf0,templates) || !field(templates,0,type) || type!=module+0x0441c938 ||
     !field(templates,0x240,pixels) || !field(pixels,0x50,result.width) || !field(pixels,0x54,result.height) ||
     !result.width || result.width>65536 || !result.height || result.height>65536)
    return fail("native spline database or current map dimensions unavailable");
  auto table=[&](uint64_t address,size_t stride,size_t occupancy,std::vector<uint8_t>& rows){
    std::array<uint8_t,0x28> header{};
    if(!read(address,header.data(),header.size()))return false;
    const uint32_t count=u32(header,0x10),mask=u32(header,0x14);
    if(!count){rows.clear();return true;}
    if(count>1000000 || mask>0x1fffff || ((mask+1)&mask) || count>mask+1 || header[0x18]==255)return false;
    rows.resize((size_t(mask)+header[0x18]+1)*stride);
    if(!read(u64(header,8),rows.data(),rows.size()))return false;
    size_t found=0;
    for(size_t at=0;at<rows.size();at+=stride){
      if(rows[at+occupancy]==255)return false;
      found+=rows[at+occupancy]!=0;
    }
    return found==count;
  };
  std::vector<uint8_t> rows;
  if(!table(database,0x1c,0,rows))return fail("invalid native spline point table");
  for(size_t at=0;at<rows.size();at+=0x1c)if(rows[at]){
    const uint32_t id=u32(rows,at+4)&0x1fffffff;
    NativeSplinePoint p;std::memcpy(&p,rows.data()+at+8,sizeof(p));
    if(id==0x1fffffff || !std::isfinite(p.x) || !std::isfinite(p.z) || !result.points.emplace(id,p).second)
      return fail("invalid or duplicate native spline point");
  }
  if(!table(database+0x28,0x30,4,rows))return fail("invalid native spline segment table");
  for(size_t at=0;at<rows.size();at+=0x30)if(rows[at+4]){
    const uint64_t id=u64(rows,at+8)&0xffffffffffffff;
    const uint32_t cap=u32(rows,at+0x18),count=u32(rows,at+0x1c);
    if(id==0xffffffffffffff || (u64(rows,at+0x28)&0xffffffffffffff)!=id ||
       count>cap || cap>65536 || result.strips.count(id))return fail("invalid native spline segment bounds or identity");
    std::vector<uint32_t> refs(count);
    if(count && !read(u64(rows,at+0x10),refs.data(),refs.size()*4))return fail("unreadable native spline controls");
    for(auto& point:refs){point&=0x1fffffff;if(!result.points.count(point))return fail("native spline references an unknown point");}
    result.strips.emplace(id,std::move(refs));
  }
  if(!table(database+0x50,0x38,4,rows))return fail("invalid native spline connection table");
  for(size_t at=0;at<rows.size();at+=0x38)if(rows[at+4]){
    const uint64_t key=u64(rows,at+8);
    const uint32_t cap=u32(rows,at+0x18),count=u32(rows,at+0x1c);
    if(u64(rows,at+0x28)!=key || count>cap || cap>65536 || result.connections.count(key))
      return fail("invalid native spline connection bounds or identity");
    std::vector<uint64_t> refs(count);
    if(count && !read(u64(rows,at+0x10),refs.data(),refs.size()*8))return fail("unreadable native spline segment list");
    for(auto& strip:refs){strip&=0xffffffffffffff;if(!result.strips.count(strip))return fail("native connection references an unknown segment");}
    result.connections.emplace(key,std::move(refs));
  }
  if(result.points.empty() || result.strips.empty() || result.connections.empty())return fail("native spline geometry not loaded");
  if(!window.Unchanged())return fail("native spline source changed during copy");
  if(!current())return fail("retired native spline source");
  if(error)error->clear();return result;
}

// Worker-side geometry conversion only. No route search or quantity allocation.
// Dense edge IDs are local to this graph, never truncated native 56-bit IDs.
// Presence of geometry is NOT evidence that a canal is currently traversable.
inline std::optional<SplineGraph> BuildNativeSplineGraph(const NativeSplineNetwork& network,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<SplineGraph>{if(error)*error=why;return {};};
  if(!network.width || !network.height)return fail("missing native map dimensions");
  SplineGraph graph;graph.width=float(network.width);graph.height=float(network.height);
  std::set<uint64_t> used;
  for(const auto& [key,refs]:network.connections)if((key&63)<=3){
    if(refs.empty())continue; // known empty geometry is distinct from a missing key
    uint32_t current=uint32_t(key>>6)&0x1fffffff;
    for(uint64_t id:refs){
      auto strip=network.strips.find(id);
      if(strip==network.strips.end() || (id&63)!=(key&63) || strip->second.size()<2)
        return fail("unknown or mismatched native spline segment");
      const auto& points=strip->second;
      if(current==points.front())current=points.back();
      else if(current==points.back())current=points.front();
      else return fail("disconnected native spline connection");
      used.insert(id);
    }
    if(current!=uint32_t(key>>35))return fail("native spline connection endpoint mismatch");
  }
  std::map<uint64_t,uint32_t> edges;
  auto junction=[&](uint32_t id,const NativeSplinePoint& point){
    const auto index=uint32_t(graph.nodes.size());
    const auto [where,added]=graph.byId.emplace(id,index);
    if(added)graph.nodes.push_back({id,point.x,float(network.height)-point.z});
    return where->second;
  };
  for(uint64_t id:used){
    const auto& refs=network.strips.at(id);
    const uint32_t first=uint32_t(graph.points.size());double length=0;
    for(uint32_t ref:refs){
      const auto p=network.points.find(ref);if(p==network.points.end())return fail("missing native spline control");
      if(!std::isfinite(p->second.x) || !std::isfinite(p->second.z))return fail("nonfinite native spline control");
      SplinePoint point{p->second.x,float(network.height)-p->second.z};
      if(graph.points.size()>first){const auto& prev=graph.points.back();length+=std::hypot(double(point.x)-prev.x,double(point.z)-prev.z);}
      graph.points.push_back(point);
    }
    if(!(length>0) || !std::isfinite(float(length)))return fail("degenerate native spline segment");
    const uint32_t edge=uint32_t(graph.edges.size());edges.emplace(id,edge);
    graph.edges.push_back({edge,junction(refs.front(),network.points.at(refs.front())),
        junction(refs.back(),network.points.at(refs.back())),uint32_t(id&63),first,uint32_t(refs.size()),float(length)});
  }
  graph.adjacency.resize(graph.nodes.size());
  for(const auto& edge:graph.edges){graph.adjacency[edge.from].push_back(edge.id);graph.adjacency[edge.to].push_back(edge.id);}
  for(const auto& [key,refs]:network.connections)if((key&63)<=3){
    graph.nativeConnections[key];
    for(uint64_t id:refs)graph.nativeConnections[key].push_back(edges.at(id));
  }
  if(error)error->clear();return graph;
}
} // namespace v3
