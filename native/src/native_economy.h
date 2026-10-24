#pragma once
#include "freight.h"
#include "state_market.h"
#include "state_region.h"
#include <unordered_map>

namespace v3 {
// One candidate may use several decoders. Recheck their combined observations
// after metadata, orders and rail have all been copied, not just each separately.
class EconomyReadWindow {
 public:
  explicit EconomyReadWindow(GoodsMemoryReader reader):m_reader(std::move(reader)){}
  bool Read(uint64_t address,void* out,size_t size) {
    if(!m_reader || !address || !size || size>UINT64_MAX-address || size>32*1024*1024-m_total ||
       !m_reader(address,out,size))return false;
    m_total+=size;m_copies.push_back({address,{(uint8_t*)out,(uint8_t*)out+size}});return true;
  }
  bool Unchanged() const {
    for(const auto& copy:m_copies){std::vector<uint8_t> after(copy.bytes.size());
      if(!m_reader(copy.address,after.data(),after.size()) || after!=copy.bytes)return false;
    }
    return !m_copies.empty();
  }
 private:
  struct Copy {uint64_t address;std::vector<uint8_t> bytes;};
  GoodsMemoryReader m_reader;
  std::vector<Copy> m_copies;
  size_t m_total=0;
};
// Exact typed lookup used by State.Get*. No native call or lifetime guarantee:
// the address is local to the adapter's access window, never a worker input.
inline uint64_t ReadNativeStateObject(const GoodsMemoryReader& read,uint64_t module,uint32_t id) {
  if(!read || !module || module>UINT64_MAX-0x05964ef8 || id==UINT32_MAX)return 0;
  auto field=[&](uint64_t p,uint64_t off,auto& value){
    return p && off<=UINT64_MAX-p && sizeof(value)<=UINT64_MAX-(p+off) &&
        read(p+off,&value,sizeof(value));
  };
  uint64_t db=0,entries=0,object=0,type=0;uint32_t slots=0,actual=UINT32_MAX;
  if(!field(module,0x05964ef0,db) || !field(db,0x20,entries) || !field(db,0x2c,slots) ||
     slots>65536 || (id&0xffffff)>=slots ||
     !field(entries,8+uint64_t(id&0xffffff)*16,object) ||
     !field(object,0,type) || type!=module+0x044de308 || !field(object,8,actual) || actual!=id)return 0;
  return object;
}
struct NativeStateEconomy {uint32_t id=0,region=0,market=0;};
struct NativeEconomyCopy {
  std::vector<NativeStateEconomy> states;
  std::vector<StateOrders> orders;
};
// Safe, bounded owned copies. A stable reread rejects observed mutations; it is
// not proof of an atomic simulation epoch or permission to use another thread.
inline std::optional<NativeEconomyCopy> DecodeNativeEconomy(const GoodsMemoryReader& reader,
    uint64_t module,const std::unordered_map<std::string,uint32_t>& regions,uint32_t goodCount,
    const std::function<bool()>& current,std::string* error=nullptr) {
  auto fail=[&](const char* text)->std::optional<NativeEconomyCopy>{if(error)*error=text;return {};};
  if(!reader || !module || module>UINT64_MAX-0x05964ef8 || regions.empty() ||
     !goodCount || goodCount>128 || !current || !current())return fail("missing economy identity");
  struct Copy {uint64_t address;std::vector<uint8_t> bytes;};
  std::vector<Copy> copies;size_t total=0;
  GoodsMemoryReader read=[&](uint64_t address,void* out,size_t size){
    if(!address || !size || size>UINT64_MAX-address || size>16*1024*1024-total ||
       !reader(address,out,size))return false;
    total+=size;copies.push_back({address,{(uint8_t*)out,(uint8_t*)out+size}});return true;
  };
  auto field=[&](uint64_t p,uint64_t off,auto& value){
    return p && off<=UINT64_MAX-p && read(p+off,&value,sizeof(value));
  };
  auto u64=[](const auto& bytes,size_t at){uint64_t n=0;std::memcpy(&n,bytes.data()+at,8);return n;};
  auto u32=[](const auto& bytes,size_t at){uint32_t n=0;std::memcpy(&n,bytes.data()+at,4);return n;};
  uint64_t db=0,entries=0;uint32_t slots=0;
  if(!field(module,0x05964ef0,db) || !field(db,0x20,entries) || !field(db,0x2c,slots) ||
     !slots || slots>65536)return fail("invalid state database bounds");
  std::vector<uint64_t> roster(size_t(slots)*2);
  if(!read(entries,roster.data(),roster.size()*8))return fail("unreadable state roster");
  NativeEconomyCopy result;
  std::unordered_set<uint64_t> objects;
  auto counter=[&](uint64_t object,uint64_t offset,std::vector<int64_t>& values){
    std::array<uint8_t,0x30> header{};
    if(offset>UINT64_MAX-object || !read(object+offset,header.data(),header.size()))return false;
    const uint64_t data=u64(header,8);const uint32_t cap=u32(header,0x10),count=u32(header,0x14);
    const std::array<uint64_t,2> masks{{u64(header,0x20),u64(header,0x28)}};
    if(count>cap || cap>128 || (count && count!=goodCount))return false;
    for(uint32_t g=goodCount;g<128;++g)if(masks[g/64]&(uint64_t(1)<<(g%64)))return false;
    values.assign(goodCount,0);
    if(!count)return !masks[0] && !masks[1];
    if(!read(data,values.data(),values.size()*8))return false;
    for(uint32_t g=0;g<goodCount;++g){
      if(!(masks[g/64]&(uint64_t(1)<<(g%64))))values[g]=0;
      else if(values[g]<0)return false;
    }
    return true;
  };
  for(uint32_t slot=0;slot<slots;++slot){
    const uint64_t object=roster[size_t(slot)*2+1];if(!object)continue;
    uint32_t id=UINT32_MAX;
    if(!objects.insert(object).second || !field(object,8,id) || id==UINT32_MAX ||
       (id&0xffffff)!=slot || ReadNativeStateObject(read,module,id)!=object)
      return fail("invalid state type or full handle");
    auto key=DecodeNativeStateRegionKey(read,module,object,id);
    auto market=DecodeNativeStateMarket(read,module,object,id);
    if(!key || !regions.count(*key) || !market)return fail("state region or market unavailable");
    const uint32_t region=regions.at(*key);result.states.push_back({id,region,*market});
    std::vector<int64_t> production,consumption;
    if(!counter(object,0xc8,production) || !counter(object,0x78,consumption))
      return fail("invalid goods counter bounds, mask or quantity");
    for(uint32_t g=0;g<goodCount;++g)if(production[g] || consumption[g])
      result.orders.push_back({id,region,*market,g,production[g]/100,consumption[g]/100});
  }
  if(result.states.empty())return fail("empty live state roster");
  if(!current())return fail("economy request retired");
  for(const auto& copy:copies){std::vector<uint8_t> after(copy.bytes.size());
    if(!reader(copy.address,after.data(),after.size()) || after!=copy.bytes)
      return fail("economy changed during copy");
  }
  if(!current())return fail("economy request retired");
  if(error)error->clear();return result;
}
} // namespace v3
