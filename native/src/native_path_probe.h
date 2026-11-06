#pragma once
#include "goods_catalog.h"
#include "native_economy.h"
#include <array>
#include <cstring>
#include <limits>
#include <optional>
#include <string>
#include <vector>

namespace v3 {
struct NativePathEndpoint {
  uint32_t node=UINT32_MAX;
  std::array<int64_t,2> position{};
  bool operator==(const NativePathEndpoint&)const=default;
};
struct NativePathStep {
  uint32_t connection=UINT32_MAX;
  NativePathEndpoint from,to;
  int64_t distance=0,cost=0;
  bool operator==(const NativePathStep&)const=default;
};
struct NativePathResult {
  std::vector<NativePathStep> steps;
  int64_t distance=0,cost=0;
};
struct NativeFleetPathRequest {
  uint32_t country=UINT32_MAX,traversal=UINT32_MAX;
  NativePathEndpoint from,to;
  bool operator==(const NativeFleetPathRequest&)const=default;
};
// Bounded read-only equivalent of the original node-ref lookup140C59AF0.
// Only typed fleet finders with two valid node refs are observed. A coordinate
// fallback or another finder is not silently assigned a nearby native node.
inline std::optional<NativeFleetPathRequest> ObserveNativeFleetPathRequest(
    const GoodsMemoryReader& reader,uint64_t module,uint64_t finder,uint64_t request) {
  if(!reader || !module || module>UINT64_MAX-0x059667f8 || !finder || !request)return {};
  EconomyReadWindow window(reader);
  auto field=[&](uint64_t p,uint64_t off,auto& value){
    return p && off<=UINT64_MAX-p && window.Read(p+off,&value,sizeof(value));
  };
  NativeFleetPathRequest out;
  uint64_t type=0,country=0,db=0,entries=0;uint32_t kind=UINT32_MAX,slots=0;
  if(!field(finder,0,type) || type!=module+0x044a0438 || !field(finder,0x168,kind) || kind!=1 ||
     !field(finder,0x180,country) || !field(country,0,type) || type!=module+0x04494cb8 ||
     !field(country,0x18,out.country) || out.country==UINT32_MAX || !field(finder,0x190,out.traversal) ||
     !field(module,0x059667f0,db) || !field(db,0,type) || type!=module+0x04439800 ||
     !field(db,0x20,entries) || !field(db,0x2c,slots) || slots>131072)return {};
  for(size_t i=0;i<2;++i){
    auto& endpoint=i?out.to:out.from;uint64_t object=0,definition=0;uint32_t actual=UINT32_MAX;
    if(!field(request,0x18+i*0x20,endpoint.node) || endpoint.node==UINT32_MAX ||
       (endpoint.node&0xffffff)>=slots || !field(entries,8+uint64_t(endpoint.node&0xffffff)*16,object) ||
       !field(object,0,type) || type!=module+0x044e3d78 || !field(object,8,actual) || actual!=endpoint.node ||
       !field(object,0x78,definition) || !field(definition,0,type) || type!=module+0x044f8a78 ||
       !field(definition,0x48,endpoint.position))return {};
  }
  if(!window.Unchanged())return {};
  return out;
}
// Diagnostic exact-node paths only, after original successful search and while
// its caller still owns the result. This does not grant an engine-access lease,
// validate connection objects/permissions or authorize formal geometry. Custom
// transitions (connection UINT32_MAX) are unsupported, never drawn as chords.
inline std::optional<NativePathResult> CopyNativePathResult(
    const GoodsMemoryReader& read,uint64_t result,const NativePathEndpoint& from,
    const NativePathEndpoint& to,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<NativePathResult>{if(error)*error=why;return {};};
  auto copy=[&](uint64_t a,void* p,size_t n){return read && a && n<=UINT64_MAX-a && read(a,p,n);};
  if(!result || result>UINT64_MAX-0x68 || from.node==UINT32_MAX || to.node==UINT32_MAX)
    return fail("missing native path endpoint or result");
  std::array<uint8_t,0x28> header{},after{};
  if(!copy(result+0x40,header.data(),header.size()))return fail("unreadable native path header");
  auto value=[]<class T>(const auto& bytes,size_t at){T n{};std::memcpy(&n,bytes.data()+at,sizeof(n));return n;};
  const auto data=value.operator()<uint64_t>(header,0);
  const auto capacity=value.operator()<uint32_t>(header,8),count=value.operator()<uint32_t>(header,12);
  if(count>capacity || count>4096 || capacity>65536 ||
     (capacity && (!data || uint64_t(capacity)*64>UINT64_MAX-data)))
    return fail("invalid native path array bounds");
  NativePathResult out;
  out.distance=value.operator()<int64_t>(header,0x18);out.cost=value.operator()<int64_t>(header,0x20);
  if(out.distance<0 || out.cost<0)return fail("invalid native path totals");
  std::vector<std::array<uint8_t,64>> rows(count),check(count);
  if(count && !copy(data,rows.data(),rows.size()*64))return fail("unreadable native path steps");
  int64_t distance=0,cost=0;NativePathEndpoint expected=from;
  for(const auto& row:rows){
    NativePathStep step;step.connection=value.operator()<uint32_t>(row,0);
    step.from={value.operator()<uint32_t>(row,4),{value.operator()<int64_t>(row,8),value.operator()<int64_t>(row,0x10)}};
    step.to={value.operator()<uint32_t>(row,0x18),{value.operator()<int64_t>(row,0x20),value.operator()<int64_t>(row,0x28)}};
    step.distance=value.operator()<int64_t>(row,0x30);step.cost=value.operator()<int64_t>(row,0x38);
    if(step.connection==UINT32_MAX)return fail("unsupported native custom transition");
    if(step.from!=expected || step.to.node==UINT32_MAX)return fail("native path endpoint or continuity mismatch");
    if(step.distance<0 || step.cost<0 || step.distance>std::numeric_limits<int64_t>::max()-distance ||
       step.cost>std::numeric_limits<int64_t>::max()-cost)return fail("invalid native path step totals");
    distance+=step.distance;cost+=step.cost;expected=step.to;out.steps.push_back(step);
  }
  if(expected!=to || distance!=out.distance || cost!=out.cost)return fail("native path destination or totals mismatch");
  // Borrowed memory is copied twice without calling engine getters. Observed
  // changes are rejected; this is not an atomic snapshot or lifetime witness.
  if(!copy(result+0x40,after.data(),after.size()) || after!=header ||
     (count && (!copy(data,check.data(),check.size()*64) || check!=rows)))
    return fail("native path changed during copy");
  if(error)error->clear();return out;
}
} // namespace v3
