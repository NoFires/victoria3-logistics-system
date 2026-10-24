#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <set>
#include <string>
#include <vector>

namespace v3 {
// Copied-memory decoder for the exact 1.13.11 layout. R72 verified this layout on
// 53 live definitions. This helper does not establish a caller's access window.
// The supplied reader must fail safely; it must never be a raw memcpy of engine
// memory. No pointer, callback or native string escapes in the returned catalog.
struct GoodsMetadata {
  uint32_t id = 0;
  std::string key;
  uint8_t category = 0; // native enum value; display mapping not established
  bool localOnly = false, tradeable = false;
  bool operator==(const GoodsMetadata&) const = default;
};
struct GoodsCatalog {
  bool ok = false;
  std::vector<GoodsMetadata> goods;
  std::string error;
};
struct GoodsCatalogLayout {
  uint64_t databaseVtable = 0, goodVtable = 0; // relocated, exact-build identities
};
using GoodsMemoryReader = std::function<bool(uint64_t, void*, size_t)>;

inline GoodsCatalog DecodeGoodsCatalog(const GoodsMemoryReader& reader, uint64_t database,
                                      GoodsCatalogLayout layout) {
  GoodsCatalog result;
  auto fail = [&](const char* reason) {
    result.ok = false; result.goods.clear(); result.error = reason; return result;
  };
  if (!reader || !database || !layout.databaseVtable || !layout.goodVtable)
    return fail("missing catalog identity or reader");
  auto read = [&](uint64_t address, void* out, size_t bytes) {
    return address && bytes && bytes <= UINT64_MAX-address && reader(address,out,bytes);
  };
  auto field = [](const auto& bytes, size_t offset, auto* out) {
    std::memcpy(out,bytes.data()+offset,sizeof(*out));
  };
  std::array<uint8_t,0x90> header{}, headerAfter{};
  if (!read(database,header.data(),header.size())) return fail("unreadable goods database");
  uint64_t vtable=0, begin=0; uint32_t count=0;
  field(header,0,&vtable); field(header,0x80,&begin); field(header,0x8c,&count);
  if (vtable!=layout.databaseVtable || count==0 || count>128)
    return fail("invalid goods database identity or count");
  std::vector<uint64_t> pointers(count), pointersAfter(count);
  if (!read(begin,pointers.data(),count*sizeof(uint64_t))) return fail("unreadable goods vector");
  std::set<std::string> keys;
  std::vector<std::array<uint8_t,0x70>> definitions(count);
  std::vector<uint64_t> stringAddresses(count,0);
  for (uint32_t id=0; id<count; ++id) {
    auto& bytes=definitions[id];
    if (!read(pointers[id],bytes.data(),bytes.size())) return fail("unreadable good definition");
    uint32_t nativeId=0; uint64_t size=0, capacity=0;
    field(bytes,0,&vtable); field(bytes,0x10,&nativeId);
    field(bytes,0x28,&size); field(bytes,0x30,&capacity);
    if (vtable!=layout.goodVtable || nativeId!=id) return fail("good ID or vtable mismatch");
    if (size==0 || size>128 || size>capacity || (capacity<16 && capacity!=15))
      return fail("invalid good key string bounds");
    std::string key(static_cast<size_t>(size)+1,'\0');
    if (capacity<16) std::memcpy(key.data(),bytes.data()+0x18,key.size());
    else {
      field(bytes,0x18,&stringAddresses[id]);
      if (!read(stringAddresses[id],key.data(),key.size())) return fail("unreadable good key");
    }
    if (key.back()!=0) return fail("unterminated good key");
    key.pop_back();
    for (char c:key)
      if (!((c>='a'&&c<='z') || (c>='A'&&c<='Z') || (c>='0'&&c<='9') || c=='_'))
        return fail("invalid good key characters");
    if (!keys.insert(key).second) return fail("duplicate good key");
    const uint8_t flags=bytes[0x40];
    result.goods.push_back({id,std::move(key),bytes[0x41],bool(flags&1),bool((flags&2)&&!(flags&1))});
  }
  // Detect observed changes; equal rereads are NOT proof of atomicity/lifetime.
  if (!read(database,headerAfter.data(),headerAfter.size()) || header!=headerAfter ||
      !read(begin,pointersAfter.data(),count*sizeof(uint64_t)) || pointers!=pointersAfter)
    return fail("goods database changed during copy");
  for (uint32_t id=0; id<count; ++id) {
    std::array<uint8_t,0x70> after{};
    if (!read(pointers[id],after.data(),after.size()) || definitions[id]!=after)
      return fail("good definition changed during copy");
    if (stringAddresses[id]) {
      std::string afterKey(result.goods[id].key.size()+1,'\0');
      if (!read(stringAddresses[id],afterKey.data(),afterKey.size()) || afterKey.back()!=0 ||
          afterKey.compare(0,afterKey.size()-1,result.goods[id].key)!=0)
        return fail("good key changed during copy");
    }
  }
  result.ok=true; return result;
}
} // namespace v3
