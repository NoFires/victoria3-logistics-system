#pragma once
#include "goods_catalog.h"
#include "offsets.h"
#include <algorithm>
#include <optional>
namespace v3 {
struct NativeGoodsFilter {
  std::optional<std::vector<uint32_t>> goods; // null=unavailable, empty=known no matches
  std::string error;
};
// Read the SAME cached row IDs returned by Market.AccessMarketGoods, after its
// original localization/search/category filtering. No getter/lock/write is used.
// Raw pointers stay local to this UI-thread copy. Rereads reject observed change,
// but do not establish atomicity or all native cache lifetimes.
inline NativeGoodsFilter DecodeNativeGoodsFilter(const GoodsMemoryReader& reader,uint64_t module,
    uint32_t market,const std::string& query,const std::vector<uint8_t>& exclusions) {
  auto fail=[](const char* why){return NativeGoodsFilter{std::nullopt,why};};
  if(!reader || !module || module>UINT64_MAX-kRvaGuiMarketRegistry-8 || market==UINT32_MAX)
    return fail("native goods context unavailable");
  struct Copy {uint64_t address;std::vector<uint8_t> bytes;};std::vector<Copy> copies;
  auto read=[&](uint64_t a,void* out,size_t n){
    if(!a || !n || n>UINT64_MAX-a || !reader(a,out,n))return false;
    copies.push_back({a,std::vector<uint8_t>(static_cast<uint8_t*>(out),static_cast<uint8_t*>(out)+n)});return true;
  };
  auto field=[&](uint64_t a,uint64_t off,auto& out){return a && a<=UINT64_MAX-off && read(a+off,&out,sizeof(out));};
  uint64_t registry=0,entries=0;int32_t count=0,capacity=0;uint32_t lock=0;
  if(!field(module,kRvaGuiMarketRegistry,registry) || !field(registry,0x18,lock) || (lock&1) ||
     !field(registry,0,entries) || !field(registry,8,capacity) || !field(registry,12,count) ||
     count<0 || count>2048 || capacity<count)return fail("native market registry unavailable or busy");
  uint64_t match=0;
  for(int32_t i=0;i<count;++i){
    uint64_t object=0,vt=0;uint32_t id=0;
    if(!field(entries,uint64_t(i)*8,object) || !field(object,0,vt) || vt!=module+kRvaGuiMarketVtable ||
       !field(object,0x10,id))return fail("invalid native market entry");
    if(id==market){if(match)return fail("ambiguous native market identity");match=object;}
  }
  if(!match)return fail("native market rows not created yet");
  uint32_t cacheLock=0;uint8_t dirty=1;
  if(!field(match,0x54,cacheLock) || (cacheLock&1) || !field(match,0x320,dirty) || dirty)
    return fail("native goods rows pending refresh");
  std::array<uint8_t,32> text{};
  if(match>UINT64_MAX-0x328 || !read(match+0x328,text.data(),text.size()))return fail("native goods query unavailable");
  uint64_t size=0,cap=0;std::memcpy(&size,text.data()+16,8);std::memcpy(&cap,text.data()+24,8);
  if(size>4096 || size>cap || (cap<16 && cap!=15))return fail("invalid native goods query");
  std::string cached(size+1,'\0');
  if(cap<16)std::memcpy(cached.data(),text.data(),cached.size());
  else {uint64_t p=0;std::memcpy(&p,text.data(),8);if(!read(p,cached.data(),cached.size()))return fail("unreadable native goods query");}
  if(cached.back()!=0)return fail("unterminated native goods query");
  cached.pop_back();if(cached.find('\0')!=std::string::npos || cached!=query)return fail("native goods query not settled");
  uint64_t rows=0;int32_t rowCount=0,rowCapacity=0;
  if(!field(match,0x60,rows) || !field(match,0x68,rowCapacity) || !field(match,0x6c,rowCount) ||
     rowCount<0 || rowCount>128 || rowCapacity<rowCount)return fail("invalid native goods row count");
  std::vector<uint32_t> goods;
  for(int32_t i=0;i<rowCount;++i){
    uint64_t row=0,vt=0,definition=0;uint32_t state=0,rowMarket=0,id=0;uint8_t category=0,flags=0;
    if(!field(rows,uint64_t(i)*8,row) || !field(row,0,vt) || vt!=module+kRvaGuiGoodsVtable ||
       !field(row,0x18,state) || state!=UINT32_MAX || !field(row,0x1c,rowMarket) || rowMarket!=market ||
       !field(row,0x10,definition) || !field(definition,0,vt) || vt!=module+kRvaGoodVtable ||
       !field(definition,0x10,id) || id>=128 || !field(definition,0x41,category) || category>=4 ||
       !field(definition,0x40,flags) || (flags&1) || !(flags&2))return fail("invalid native goods row identity");
    if(std::find(exclusions.begin(),exclusions.end(),category)!=exclusions.end())
      return fail("native category rows not settled");
    goods.push_back(id);
  }
  std::sort(goods.begin(),goods.end());
  if(std::adjacent_find(goods.begin(),goods.end())!=goods.end())return fail("duplicate native goods row");
  for(const auto& c:copies){std::vector<uint8_t> after(c.bytes.size());
    if(!reader(c.address,after.data(),after.size()) || after!=c.bytes)return fail("native goods rows changed during copy");}
  return {std::move(goods),{}};
}
} // namespace v3
