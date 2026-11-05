#pragma once
#include "native_economy.h"
#include "native_shipping.h"
#include <bitset>
#include <set>

namespace v3 {
struct NativeSeaCacheRoute {
  uint16_t source=0,destination=0;
  int64_t distanceFixed=0;
  std::vector<NativeProvinceRange> provinces; // original membership, not ordered edges
};
// Exact14141FA80 cached branch only; no search, allocator or engine call.
// Caller retains the current native access/build-completion window. An empty
// observed cache entry is NOT proof of failed search or country permission.
template<class Reader,class Current>
std::optional<NativeSeaCacheRoute> DecodeNativeSeaCacheRoute(const Reader& reader,uint64_t module,
    uint16_t source,uint16_t destination,const Current& current,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<NativeSeaCacheRoute>{if(error)*error=why;return {};};
  if(!module || module>UINT64_MAX-0x0592d130 || !source || source==UINT16_MAX || !destination ||
     destination==UINT16_MAX || source==destination || !current())return fail("missing distinct current sea cache endpoints");
  EconomyReadWindow window(reader);
  auto field=[&](uint64_t p,uint64_t off,auto& value){return p && off<=UINT64_MAX-p && window.Read(p+off,&value,sizeof(value));};
  auto u32=[](const auto& bytes,size_t at){uint32_t n=0;std::memcpy(&n,bytes.data()+at,4);return n;};
  auto u64=[](const auto& bytes,size_t at){uint64_t n=0;std::memcpy(&n,bytes.data()+at,8);return n;};
  uint64_t root=0,machine=0,world=0,cache=0,type=0;
  if(!field(module,0x0592d128,root) || !field(root,0x608,machine) || !field(machine,0x120,world) ||
     !field(world,0x4500,cache) || !field(cache,0,type) || type!=module+0x044eb4d8)
    return fail("current typed sea path cache unavailable");
  std::array<uint8_t,0x28> header{};std::array<uint8_t,24> matrix{};
  if(!field(cache,8,header) || !field(cache,0x30,matrix))return fail("unreadable sea cache index");
  const auto count=u32(header,0x10),mask=u32(header,0x14);const uint32_t probe=header[0x18];
  if(!count || count>65534 || mask>0x1fffff || ((mask+1)&mask) || count>mask+1 || probe==255 ||
     u32(matrix,12)!=count || count>u32(matrix,8) || u32(matrix,8)>65536)
    return fail("invalid sea cache matrix bounds");
  const uint64_t rows=uint64_t(mask)+probe+1,data=u64(header,8),matrixData=u64(matrix,0);
  if(!data || rows*12>UINT64_MAX-data || !matrixData || uint64_t(u32(matrix,8))*24>UINT64_MAX-matrixData)
    return fail("sea cache index address overflow");
  std::vector<uint8_t> indices(size_t(rows)*12);std::set<uint16_t> keys;std::set<uint32_t> slots;
  if(!window.Read(data,indices.data(),indices.size()))return fail("unreadable sea cache index rows");
  for(size_t at=0;at<indices.size();at+=12)if(indices[at]){
    uint16_t key=0;std::memcpy(&key,indices.data()+at+4,2);const auto slot=u32(indices,at+8);
    if(indices[at]==255 || !key || key==UINT16_MAX || slot>=count || !keys.insert(key).second || !slots.insert(slot).second)
      return fail("invalid or duplicate sea cache index identity");
  }
  if(keys.size()!=count)return fail("sea cache index count mismatch");
  auto lookup=[&](uint16_t key)->std::optional<uint32_t>{
    size_t row=key&mask;uint32_t distance=1;
    while(row<rows && distance<=indices[row*12]){
      uint16_t actual=0;std::memcpy(&actual,indices.data()+row*12+4,2);
      if(actual==key)return u32(indices,row*12+8);
      ++row;++distance;
    }
    return {};
  };
  const auto a=lookup(std::min(source,destination)),b=lookup(std::max(source,destination));
  if(!a || !b || *a>=*b)return fail("requested original sea cache indices unavailable");
  std::array<uint8_t,24> row{};
  if(!window.Read(matrixData+uint64_t(*a)*24,row.data(),row.size()) ||
     u32(row,12)!=count-*a-1 || u32(row,12)>u32(row,8) || u32(row,8)>65536)
    return fail("invalid sea cache triangular row");
  const auto column=*b-*a-1;const uint64_t entries=u64(row,0);
  if(!entries || uint64_t(u32(row,8))*32>UINT64_MAX-entries)return fail("sea cache entry address overflow");
  std::array<uint8_t,32> entry{};
  if(!window.Read(entries+uint64_t(column)*32,entry.data(),entry.size()))return fail("unreadable sea cache entry");
  const auto capacity=u32(entry,8),length=u32(entry,12);const uint64_t rangesData=u64(entry,0);
  NativeSeaCacheRoute result;result.source=source;result.destination=destination;
  std::memcpy(&result.distanceFixed,entry.data()+24,8);
  if(length>capacity || length>4096 || capacity>65536 || result.distanceFixed<0 ||
     (capacity && (!rangesData || uint64_t(capacity)*4>UINT64_MAX-rangesData)))return fail("invalid cached sea path bounds");
  std::vector<std::array<uint16_t,2>> ranges(length);std::bitset<65536> members;
  if(length && !window.Read(rangesData,ranges.data(),ranges.size()*4))return fail("unreadable cached sea path");
  for(const auto& range:ranges){
    const uint32_t last=uint32_t(range[0])+range[1];
    if(!range[0] || last>=UINT16_MAX)return fail("invalid cached sea path range");
    for(uint32_t id=range[0];id<=last;++id){if(members[id])return fail("overlapping cached sea path ranges");members.set(id);}
    result.provinces.push_back({range[0],uint16_t(last)});
  }
  if((length && (!members[source] || !members[destination])) || (!length && result.distanceFixed))
    return fail("cached sea path differs from requested endpoints");
  if(!window.Unchanged() || !current())return fail("sea cache changed or retired during copy");
  if(error)error->clear();return result;
}
} // namespace v3
