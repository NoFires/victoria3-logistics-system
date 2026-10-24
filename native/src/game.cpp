#include "game.h"
#include "arrow_scope.h"
#include "log.h"
#include "offsets.h"
#include "util.h"

#include <windows.h>
#include <cstring>

namespace v3 {
namespace {

// MSVC std::string layout (x64, 32-byte object): inline buffer/pointer at +0,
// size at +0x10, capacity at +0x18. The game's name lookup reads exactly these
// fields (cmp qword ptr [rdx+0x18],0xF; mov ebx,[rdx+0x10]).
struct MsvcString {
  union {
    char buf[16];
    char* ptr;
  };
  size_t size;
  size_t capacity;
};
static_assert(sizeof(MsvcString) == 0x20);

void BuildName(MsvcString* s, const char* name, size_t len) {
  std::memset(s, 0, sizeof(*s));
  if (len <= 15) {
    std::memcpy(s->buf, name, len);
    s->size = len;
    s->capacity = 15;
  } else {
    s->ptr = (char*)std::malloc(len + 1);
    std::memcpy(s->ptr, name, len);
    s->ptr[len] = 0;
    s->size = len;
    s->capacity = len;
  }
}

void FreeName(MsvcString* s) {
  if (!s) return;
  if (s->capacity > 15) std::free(s->ptr);
  s->ptr = nullptr;
  s->size = s->capacity = 0;
}

using FnGetType = void*(__fastcall*)(void*, MsvcString*);
using FnCreate = void*(__fastcall*)(void*);
using FnSetPath = void(__fastcall*)(void*, ArrowPath*);
using FnOne = void(__fastcall*)(void*);
using FnSubmit = void(__fastcall*)(void*, void*);
using FnAlloc = void*(__fastcall*)(void*, size_t, size_t);

}  // namespace

Game& Game::Instance() {
  static Game g;
  return g;
}

bool Game::Initialize(const std::wstring& exePath, std::string* error) {
  HMODULE exe = GetModuleHandleW(nullptr);
  if (!exe) {
    m_error = "no main module";
    if (error) *error = m_error;
    return false;
  }
  m_base = (uint8_t*)exe;

  std::string hashErr;
  m_hash = Sha256File(exePath, &hashErr);
  if (m_hash.empty()) {
    m_error = "cannot hash executable: " + hashErr;
    if (error) *error = m_error;
    return false;
  }
  if (m_hash != kExpectedExeSha256) {
    m_error = "unsupported Victoria 3 build (exe sha256 " + m_hash.substr(0, 16) + "...)";
    if (error) *error = m_error;
    return false;
  }
  if (*(uint16_t*)(m_base + 0) != 0x5A4D) {  // sanity: MZ
    m_error = "main module is not a PE image";
    if (error) *error = m_error;
    return false;
  }

  for (const Sig& s : kSigs) {
    if (std::memcmp(m_base + s.rva, s.bytes, sizeof(s.bytes)) != 0) {
      m_error = std::string("entry-point signature mismatch: ") + s.name;
      if (error) *error = m_error;
      return false;
    }
  }

  m_allocator = m_base + kRvaAllocator;
  void** allocVt = *(void***)m_allocator;
  if (!allocVt) {
    m_error = "allocator singleton has no vtable";
    if (error) *error = m_error;
    return false;
  }

  m_ready = true;
  V3_INFO("native profile ready: Victoria 3 %s sha256=%s", kGameVersion, m_hash.c_str());
  return true;
}

void* Game::ResolveArrowType(const char* name, std::string* error) {
  if (!m_ready) {
    if (error) *error = "native profile not ready";
    return nullptr;
  }
  auto it = m_typeCache.find(name);
  if (it != m_typeCache.end()) return it->second;

  MsvcString s;
  BuildName(&s, name, std::strlen(name));
  // 0x37E7D60 obtains the asset factory singleton internally and only reads rdx
  // as an MSVC std::string; rcx is not consumed (verified against its prologue).
  void* type = ((FnGetType)Fn<FnGetType>(kRvaGetArrowType))(nullptr, &s);
  FreeName(&s);

  std::string why;
  if (!type || !TypeSane(type, &why)) {
    if (error) *error = "arrow type '" + std::string(name) + "' unresolved" +
                        (why.empty() ? "" : (": " + why));
    V3_WARN("arrow type '%s' unresolved (%s)", name, why.c_str());
    return nullptr;
  }
  m_typeCache[name] = type;
  V3_INFO("resolved arrow type '%s' -> %p (width=%.3f subdivisions=%d smoothing=%d bucket=%d)",
          name, type, *(float*)((uint8_t*)type + 0x68), *(int*)((uint8_t*)type + 0x6C),
          (int)*(uint8_t*)((uint8_t*)type + 0x70), (int)*(int16_t*)((uint8_t*)type + 0x30));
  return type;
}

bool Game::TypeSane(void* type, std::string* error) {
  uint8_t* p = (uint8_t*)type;
  if ((uintptr_t)p < 0x10000) {
    if (error) *error = "null type";
    return false;
  }
  float width = *(float*)(p + 0x68);
  int subdivisions = *(int*)(p + 0x6C);
  uint8_t smoothing = *(uint8_t*)(p + 0x70);
  if (!(width > 0.0f && width < 4096.0f)) {
    if (error) *error = "implausible width";
    return false;
  }
  if (subdivisions < 1 || subdivisions > 64) {
    if (error) *error = "implausible subdivisions";
    return false;
  }
  if (smoothing > 1) {
    if (error) *error = "implausible smoothing flag";
    return false;
  }
  return true;
}

void* Game::CreateSupplyArrow(void* type) {
  if (!m_ready || !type) return nullptr;
  void* arrow = ((FnCreate)Fn<FnCreate>(kRvaCreateSupplyArrow))(type);
  if (!arrow) {
    ++failures;
    V3_ERR("supply arrow construction failed for type %p", type);
    return nullptr;
  }
  ++builds;
  return arrow;
}

bool Game::SetPath(void* arrow, ArrowPath* path) {
  if (!m_ready || !arrow || !path || !path->points.data || path->points.count < 4 ||
      path->points.count > 4096 || path->points.count > path->points.capacity) return false;
  ((FnSetPath)Fn<FnSetPath>(kRvaSetPath))(arrow, path);
  ++uploads;
  return true;
}

void Game::EnsureConstants(void* arrow) {
  if (!m_ready || !arrow) return;
  ((FnOne)Fn<FnOne>(kRvaEnsureConstants))(arrow);
}

void Game::Submit(void* arrow, void* passContext) {
  if (!m_ready || !arrow || !passContext) return;
  ((FnSubmit)Fn<FnSubmit>(kRvaBucketAppend))(arrow, passContext);
  ++submitCalls;
}

void Game::ArrowSubmissionScope(int32_t* domain, int32_t* stage) const {
  *domain = m_ready ? *(const int32_t*)(m_base + kRvaArrowSubmitDomain) : -1;
  *stage = m_ready ? *(const int32_t*)(m_base + kRvaArrowSubmitStage) : -1;
}

bool Game::IsArrowSubmissionPass(const void* passContext) const {
  if (!m_ready || !passContext) return false;
  int32_t domain, stage; ArrowSubmissionScope(&domain, &stage);
  // The native pass and append routines use indices at +0, +8, +16. The
  // intervening words belong to the handles; they are not extra pass indices.
  return MatchesArrowScope(*(const ArrowPassContext*)passContext, domain, stage);
}

void* Game::AllocateGameMemory(size_t bytes, size_t alignment) {
  if (!m_allocator) return nullptr;
  void** vt = *(void***)m_allocator;
  if (!vt || !vt[1]) return nullptr;
  return ((FnAlloc)vt[1])(m_allocator, bytes, alignment);
}

}  // namespace v3

