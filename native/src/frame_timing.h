#pragma once
#include <array>
#include <cstdint>

namespace v3 {
// Diagnostic raw Present-entry intervals, not GPU execution/display latency.
// One render thread owns each sampler. No allocation or engine access per frame.
struct FrameTimingSamples {
  std::array<uint64_t,64> ticks{};
  uint64_t begin=0,end=0;
  bool Push(uintptr_t swap,uint64_t world,uint64_t now) {
    if(!swap || !now){Reset();return false;}
    if(swap!=m_swap || world!=m_world || now<=m_last){
      m_swap=swap;m_world=world;m_last=now;m_count=0;return false;
    }
    if(!m_count)begin=m_last;
    ticks[m_count++]=now-m_last;m_last=now;end=now;
    if(m_count<ticks.size())return false;
    m_count=0;return true;
  }
  void Reset(){m_swap=0;m_world=0;m_last=0;m_count=0;begin=0;end=0;}
 private:
  uintptr_t m_swap=0;
  uint64_t m_world=0,m_last=0;
  size_t m_count=0;
};

// Both Present entry points share the caller's thread-local depth. Nested
// forwarding must not create a second near-zero interval for one outer call.
struct FrameTimingScope {
  bool outer=false;
  FrameTimingScope(unsigned& depth,bool enabled) {
    if(enabled){m_depth=&depth;outer=depth++==0;}
  }
  ~FrameTimingScope(){if(m_depth)--*m_depth;}
  FrameTimingScope(const FrameTimingScope&)=delete;
  FrameTimingScope& operator=(const FrameTimingScope&)=delete;
 private:
  unsigned* m_depth=nullptr;
};
} // namespace v3
