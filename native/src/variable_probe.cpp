#include "variable_probe.h"
#include "offsets.h"
#include <windows.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>

namespace v3 {
namespace {
constexpr uint32_t kEntryStride = 0x48;
constexpr uint32_t kMaxBuckets = 100000;
constexpr size_t kMaxString = 256;

bool Readable(const void* pointer, size_t bytes) {
  if (!pointer || !bytes) return false;
  uintptr_t begin = reinterpret_cast<uintptr_t>(pointer);
  uintptr_t end = begin + bytes;
  if (end < begin) return false;
  while (begin < end) {
    MEMORY_BASIC_INFORMATION info{};
    if (!VirtualQuery(reinterpret_cast<LPCVOID>(begin), &info, sizeof(info))) return false;
    if (info.State != MEM_COMMIT) return false;
    if (info.Protect & (PAGE_NOACCESS | PAGE_GUARD)) return false;
    const uintptr_t next = reinterpret_cast<uintptr_t>(info.BaseAddress) + info.RegionSize;
    if (next <= begin) return false;
    begin = next;
  }
  return true;
}

// MSVC std::string layout: data/SSO at +0, size at +0x10, capacity at +0x18.
constexpr size_t kSsoBytes = 16;
struct MsvcStringStorage {
  char data[kSsoBytes];
  uint64_t size;
  uint64_t capacity;
};
static_assert(sizeof(MsvcStringStorage) == 0x20, "MsvcString layout");
struct StringPair {
  MsvcStringStorage key, value;
};
static_assert(sizeof(StringPair) == 0x40, "pair layout");

void InitSso(MsvcStringStorage* storage, const std::string& text) {
  std::memset(storage, 0, sizeof(*storage));
  std::memcpy(storage->data, text.data(), text.size());
  storage->size = text.size();
  storage->capacity = kSsoBytes - 1;
  storage->data[text.size()] = '\0';
}

uint32_t Rotl32(uint32_t value, int bits) {
  return (value << bits) | (value >> (32 - bits));
}

// The engine map uses MurmurHash3 x86_32 with seed 0 (verified in-game against
// 40 real key/hash pairs, 40/40 match; see WORKLOG R12).
uint32_t Murmur3(const void* data, size_t length, uint32_t seed = 0) {
  const uint32_t c1 = 0xcc9e2d51, c2 = 0x1b873593;
  const auto* bytes = static_cast<const uint8_t*>(data);
  uint32_t hash = seed;
  const size_t blocks = length / 4;
  for (size_t i = 0; i < blocks; ++i) {
    uint32_t k;
    std::memcpy(&k, bytes + i * 4, 4);
    k *= c1;
    k = Rotl32(k, 15);
    k *= c2;
    hash ^= k;
    hash = Rotl32(hash, 13);
    hash = hash * 5 + 0xe6546b64;
  }
  const uint8_t* tail = bytes + blocks * 4;
  uint32_t k = 0;
  switch (length & 3) {
    case 3: k ^= uint32_t(tail[2]) << 16; [[fallthrough]];
    case 2: k ^= uint32_t(tail[1]) << 8; [[fallthrough]];
    case 1:
      k ^= uint32_t(tail[0]);
      k *= c1;
      k = Rotl32(k, 15);
      k *= c2;
      hash ^= k;
      break;
    default: break;
  }
  hash ^= uint32_t(length);
  hash ^= hash >> 16;
  hash *= 0x85ebca6b;
  hash ^= hash >> 13;
  hash *= 0xc2b2ae35;
  hash ^= hash >> 16;
  return hash;
}

bool VerifyEntry(const uint8_t* base, uint32_t rva, std::string* error, uint8_t** out) {
  const Sig* signature = nullptr;
  for (const auto& sig : kSigs)
    if (sig.rva == rva) { signature = &sig; break; }
  if (!signature) { if (error) *error = "missing signature"; return false; }
  uint8_t* address = const_cast<uint8_t*>(base) + rva;
  if (!Readable(address, sizeof(signature->bytes)) ||
      std::memcmp(address, signature->bytes, sizeof(signature->bytes)) != 0) {
    if (error) *error = std::string(signature->name) + " signature mismatch";
    return false;
  }
  if (out) *out = address;
  return true;
}

MsvcStringStorage* FindEntry(void* map, uint32_t hash, const std::string& key);
bool ReadMsvcString(uintptr_t address, std::string* out);

MsvcStringStorage* FindEntry(void* map, uint32_t hash, const std::string& key) {
  const auto* bytes = static_cast<const uint8_t*>(map);
  const void* entries = *reinterpret_cast<const void* const*>(bytes + 0x08);
  const auto count = *reinterpret_cast<const int32_t*>(bytes + 0x10);
  const auto mask = *reinterpret_cast<const uint32_t*>(bytes + 0x14);
  if (count < 0 || count > int32_t(kMaxBuckets) || mask > kMaxBuckets) return nullptr;
  const size_t buckets = size_t(mask) + 1;
  if (!Readable(entries, buckets * kEntryStride)) return nullptr;
  const auto* entry = static_cast<const uint8_t*>(entries);
  for (size_t i = 0; i < buckets; ++i, entry += kEntryStride) {
    if (entry[4] == 0) continue;
    if (*reinterpret_cast<const uint32_t*>(entry) != hash) continue;
    std::string stored;
    if (!ReadMsvcString(reinterpret_cast<uintptr_t>(entry) + 0x08, &stored)) continue;
    if (stored != key) continue;
    return reinterpret_cast<MsvcStringStorage*>(const_cast<uint8_t*>(entry) + 0x28);
  }
  return nullptr;
}

// MSVC std::string layout: data/SSO at +0, size at +0x10, capacity at +0x18.
bool ReadMsvcString(uintptr_t address, std::string* out) {
  if (!Readable(reinterpret_cast<const void*>(address), 0x20)) return false;
  const auto size = *reinterpret_cast<const uint64_t*>(address + 0x10);
  const auto capacity = *reinterpret_cast<const uint64_t*>(address + 0x18);
  if (capacity > 0x10000 || size > capacity) return false;
  const char* data = capacity >= 16
                         ? *reinterpret_cast<const char* const*>(address)
                         : reinterpret_cast<const char*>(address);
  if (!Readable(data, size ? size : 1)) return false;
  out->assign(data, static_cast<size_t>(std::min<uint64_t>(size, kMaxString)));
  return true;
}
// The variable system and the research target share this map header/entry
// layout (0x48-byte entries, MsvcStrings for key and value).
bool ReadMapObject(const void* object, GuiVariableProbe* probe,
                   const std::vector<std::string>* keys=nullptr) {
  std::vector<uint32_t> hashes;
  if(keys){
    if(keys->size()>4096){probe->error="too many requested GUI variable keys";return false;}
    hashes.reserve(keys->size());
    for(const auto& key:*keys){
      if(key.empty() || key.size()>kMaxString){probe->error="invalid requested GUI variable key";return false;}
      hashes.push_back(Murmur3(key.data(),key.size()));
    }
    std::sort(hashes.begin(),hashes.end());
  }
  probe->object = const_cast<void*>(object);
  if (!Readable(object, 0x28)) {
    probe->error = "string map header unreadable";
    return false;
  }
  const auto* bytes = static_cast<const uint8_t*>(object);
  const void* entries = *reinterpret_cast<const void* const*>(bytes + 0x08);
  probe->count = *reinterpret_cast<const int32_t*>(bytes + 0x10);
  probe->mask = *reinterpret_cast<const uint32_t*>(bytes + 0x14);
  probe->level = *reinterpret_cast<const uint8_t*>(bytes + 0x18);
  probe->loadFactor = *reinterpret_cast<const float*>(bytes + 0x1C);
  if (probe->count < 0 || probe->count > int32_t(kMaxBuckets) || probe->mask > kMaxBuckets) {
    probe->error = "string map header out of range";
    return false;
  }
  const size_t buckets = size_t(probe->mask) + 1;
  if (!Readable(entries, buckets * kEntryStride)) {
    probe->error = "string map entry array unreadable";
    return false;
  }
  const auto* entry = static_cast<const uint8_t*>(entries);
  for (size_t i = 0; i < buckets; ++i, entry += kEntryStride) {
    if (entry[4] == 0) continue;
    GuiVariableEntry value;
    value.hash = *reinterpret_cast<const uint32_t*>(entry);
    if(keys && !std::binary_search(hashes.begin(),hashes.end(),value.hash))continue;
    // A diagnostic truncated key must never become an exact-key match.
    if(keys && *reinterpret_cast<const uint64_t*>(entry+0x18)>kMaxString)continue;
    if (!ReadMsvcString(reinterpret_cast<uintptr_t>(entry) + 0x08, &value.key)) continue;
    if(keys && std::find(keys->begin(),keys->end(),value.key)==keys->end())continue;
    if (!ReadMsvcString(reinterpret_cast<uintptr_t>(entry) + 0x28, &value.value)) continue;
    probe->entries.push_back(std::move(value));
  }
  probe->ok = true;
  return true;
}
}  // namespace

static GuiVariableProbe ReadGuiVariables(uint8_t* base,const std::vector<std::string>* keys) {
  GuiVariableProbe probe;
  if (!base) {
    probe.error = "no module base";
    return probe;
  }
  uint8_t* getter = base + kRvaVariableSystem;
  const Sig* signature = nullptr;
  for (const auto& sig : kSigs)
    if (sig.rva == kRvaVariableSystem) { signature = &sig; break; }
  if (!signature || !Readable(getter, sizeof(signature->bytes)) ||
      std::memcmp(getter, signature->bytes, sizeof(signature->bytes)) != 0) {
    probe.error = "variable-system getter signature mismatch";
    return probe;
  }
  using Getter = void* (__fastcall*)(void);
  void* object = reinterpret_cast<Getter>(getter)();
  if (!ReadMapObject(object, &probe,keys) && probe.error.empty())
    probe.error = "variable-system object unreadable";
  return probe;
}

GuiVariableProbe ProbeGuiVariables(uint8_t* base) {
  return ReadGuiVariables(base,nullptr);
}

GuiVariableProbe ProbeGuiVariableKeys(uint8_t* base,const std::vector<std::string>& keys) {
  return ReadGuiVariables(base,&keys);
}

GuiVariableProbe ProbeGlobalStringMap(uint8_t* base) {
  GuiVariableProbe probe;
  if (!base) {
    probe.error = "no module base";
    return probe;
  }
  if (!ReadMapObject(base + kRvaGlobalStringMap, &probe) && probe.error.empty())
    probe.error = "global string map unreadable";
  return probe;
}

bool WriteGuiVariableProbe(uint8_t* base, const std::string& key, const std::string& value,
                           std::string* error) {
  if (!base) { if (error) *error = "no module base"; return false; }
  if (key.empty() || key.size() > 4096 || value.size() > 4096) {
    if (error) *error = "key/value empty or too long";
    return false;
  }
  uint8_t* getter = nullptr;
  uint8_t* insert = nullptr;
  uint8_t* assign = nullptr;
  if (!VerifyEntry(base, kRvaVariableSystem, error, &getter)) return false;
  if (!VerifyEntry(base, kRvaStringMapInsert, error, &insert)) return false;
  if (!VerifyEntry(base, kRvaStringAssign, error, &assign)) return false;
  using Getter = void* (__fastcall*)(void);
  void* map = reinterpret_cast<Getter>(getter)();
  if (!Readable(map, 0x28)) { if (error) *error = "string map unreadable"; return false; }

  using Assign = void* (__fastcall*)(void*, const char*, size_t);
  auto assignFn = reinterpret_cast<Assign>(assign);
  const uint32_t hash = Murmur3(key.data(), key.size());
  // Existing UI values need only assign; do not allocate another key/value
  // pair for every row update. FindEntry returns the same validated +0x28 field.
  if (auto* existing=FindEntry(map,hash,key)) {
    assignFn(existing,value.data(),value.size());
    return true;
  }
  StringPair pair{};
  assignFn(&pair.key, key.data(), key.size());
  assignFn(&pair.value, value.data(), value.size());
  uint8_t out[16]{};
  using Insert = void* (__fastcall*)(void*, void*, uint32_t, void*);
  reinterpret_cast<Insert>(insert)(map, out, hash, &pair);
  void* entry = *reinterpret_cast<void* const*>(out);
  // Do NOT clear the pair here: depending on the map's grow path the insert may
  // move the strings (source zeroed) or share/copy them. Freeing a shared key
  // corrupts the entry (observed in R17: dangling key bytes).
  if (!entry || !Readable(entry, kEntryStride)) {
    if (error) *error = "insert returned no readable entry";
    return false;
  }
  // insert-or-find returns the existing or new entry. The engine has already
  // moved the pair strings into it; do not touch the key. For an existing key the
  // insert does not update the value, so assign the new value through the engine
  // string API (post-load heap strings are stable; see D18/R18).
  auto* valueDst = reinterpret_cast<void*>(static_cast<uint8_t*>(entry) + 0x28);
  assignFn(valueDst, value.data(), value.size());
  return true;
}
bool SetGuiVariable(uint8_t* base, const std::string& key, const std::string& value,
                    std::string* error) {
  return WriteGuiVariableProbe(base, key, value, error);
}
}  // namespace v3
