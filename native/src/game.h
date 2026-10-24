#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace v3 {

struct Vec3 {
  float x = 0, y = 0, z = 0;
};

// CPdxVector<T> as used by the arrow path and by the renderer's buckets.
// data @0x00, capacity @0x08, count @0x0C, allocator @0x10.
struct PdxVector {
  void* data = nullptr;
  uint32_t capacity = 0;
  uint32_t count = 0;
  void* allocator = nullptr;
};
static_assert(sizeof(PdxVector) == 0x18, "PdxVector layout");
static_assert(offsetof(PdxVector, count) == 0xC, "PdxVector count");

// The object retained at CArrowObject+0x70 and read by the render pass through
// the arrow subsystem's path interface (0x2674100 / 0x2674120).
//   +0x00 MoveLength, +0x04 AnimationLength, +0x08 ClipLength
//   +0x10 CPdxVector<SVector3> authored/dense polyline
struct ArrowPath {
  float moveLength = 1.0e9f;
  float animationLength = 0.0f;
  float clipLength = 0.0f;
  float reserved = 0.0f;
  PdxVector points;
};
static_assert(sizeof(ArrowPath) == 0x28, "ArrowPath layout");
static_assert(offsetof(ArrowPath, points) == 0x10, "ArrowPath points");

class Game {
 public:
  static Game& Instance();

  // Verifies the executable hash and entry-point bytes. Never calls game code.
  bool Initialize(const std::wstring& exePath, std::string* error);
  bool Ready() const { return m_ready; }
  const std::string& Error() const { return m_error; }
  const std::string& Hash() const { return m_hash; }
  uint8_t* Base() const { return m_base; }

  void* ResolveArrowType(const char* name, std::string* error);
  void ForgetArrowTypes() { m_typeCache.clear(); } // render side, after verified world recovery
  bool TypeSane(void* type, std::string* error);
  void* CreateSupplyArrow(void* type);
  bool SetPath(void* arrow, ArrowPath* path);
  void EnsureConstants(void* arrow);
  void Submit(void* arrow, void* passContext);
  bool IsArrowSubmissionPass(const void* passContext) const;
  void ArrowSubmissionScope(int32_t* domain, int32_t* stage) const;
  void* AllocateGameMemory(size_t bytes, size_t alignment);
  void* GameAllocator() const { return m_allocator; }

  uint64_t Frame() const { return m_frame; }
  void NextFrame() { ++m_frame; }

  // Diagnostics counters (render thread only).
  uint64_t uploads = 0, builds = 0, submitCalls = 0, failures = 0;

 private:
  template <typename T>
  T Fn(uint32_t rva) const { return reinterpret_cast<T>(m_base + rva); }

  uint8_t* m_base = nullptr;
  void* m_allocator = nullptr;
  bool m_ready = false;
  uint64_t m_frame = 0;
  std::string m_error;
  std::string m_hash;
  std::unordered_map<std::string, void*> m_typeCache;
};

}  // namespace v3
