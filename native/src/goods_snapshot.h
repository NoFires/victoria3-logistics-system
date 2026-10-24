#pragma once
#include "goods_catalog.h"
#include "freight.h"
#include "offsets.h"
#include <map>

namespace v3 {
// Observe this thread's existing engine context; do NOT initialize TLS, acquire
// the engine's lock, call a getter, or choose a populated slot from another context.
// Exact-build, current verified native access thread only. R101 also uses the
// completed-command thread's own TLS; never borrow another thread's context.
inline GoodsCatalog DecodeThreadGoodsCatalog(const GoodsMemoryReader& reader,
                                             uint64_t moduleBase, uint64_t tlsArray) {
  auto fail=[](const char* reason){return GoodsCatalog{false,{},reason};};
  if(!reader || !moduleBase || moduleBase>UINT64_MAX-kRvaGoodsDatabaseSlots-256)
    return fail("missing or invalid module identity");
  struct Context {
    uint64_t storage=0, token=0, database=0;
    bool operator==(const Context&)const=default;
  };
  auto read=[&](uint64_t a,void* out,size_t n){return a && n<=UINT64_MAX-a && reader(a,out,n);};
  auto context=[&](Context& c){
    uint32_t initialized=0; uint8_t active=0;
    if(!read(tlsArray,&c.storage,8) || !c.storage || c.storage>UINT64_MAX-0x8650 ||
       !read(c.storage+0x8600,&initialized,4) || !(initialized&1) ||
       !read(c.storage+0x8648,&c.token,8))return false;
    const uint32_t index=uint32_t(c.token);
    if(index>=16)return false;
    uint64_t slot=moduleBase+kRvaGoodsDatabaseSlots+16*index;
    return read(slot,&c.database,8) && c.database && read(slot+8,&active,1) && active==1;
  };
  Context before,after;
  if(!context(before))return fail("goods context not initialized");
  auto catalog=DecodeGoodsCatalog(reader,before.database,
      {moduleBase+kRvaGoodsDatabaseVtable,moduleBase+kRvaGoodVtable});
  if(!catalog.ok)return catalog;
  if(!context(after) || before!=after)return fail("goods context changed during copy");
  return catalog;
}

// Populate observed goods by verified native ID. No file-ordinal lookup or goodN
// fallback. An unknown good rejects the whole candidate snapshot, never zeroes
// its orders. Weight/Freight conversion retain explicit model defaults (1).
inline bool PopulateSnapshotGoods(EconomySnapshot& snapshot, const GoodsCatalog& catalog,
                                  std::string* error=nullptr) {
  snapshot.goods.clear();
  auto fail=[&](const char* why){if(error)*error=why;return false;};
  if(error)error->clear();
  if(!catalog.ok || catalog.goods.empty() || catalog.goods.size()>128)
    return fail("goods catalog unavailable");
  std::map<uint32_t,const GoodsMetadata*> byId;
  std::set<std::string> keys;
  static constexpr const char* categories[]={"military","staple","industrial","luxury"};
  for(const auto& g:catalog.goods)
    if(g.key.empty() || g.category>=4 || !byId.emplace(g.id,&g).second || !keys.insert(g.key).second)
      return fail("invalid goods metadata");
  std::set<uint32_t> observed;
  for(const auto& order:snapshot.orders){
    if(!byId.count(order.good))return fail("economy order has unknown good ID");
    observed.insert(order.good);
  }
  if(observed.empty())return fail("economy orders unavailable");
  std::vector<FreightGood> goods;
  for(uint32_t id:observed){
    const auto& m=*byId.at(id); FreightGood g;
    g.id=id; g.key=m.key; g.name=m.key; g.category=categories[m.category];
    g.localOnly=m.localOnly; g.tradeable=m.tradeable;
    goods.push_back(std::move(g));
  }
  snapshot.goods=std::move(goods);return true;
}
} // namespace v3
