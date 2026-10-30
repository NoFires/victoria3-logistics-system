#include "frame_timing.h"
#include <cstdio>
#include <cstdlib>
#include <numeric>
using namespace v3;
void Check(bool ok,const char* why){if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
int main(){
  FrameTimingSamples samples;
  Check(!samples.Push(1,2,100),"first observation is an anchor, not a frame");
  uint64_t now=100;
  for(unsigned i=1;i<=64;++i){now+=i;Check(samples.Push(1,2,now)==(i==64),"only complete bounded batches publish");}
  Check(samples.begin==100 && samples.end==now && samples.ticks.front()==1 && samples.ticks.back()==64 &&
      std::accumulate(samples.ticks.begin(),samples.ticks.end(),uint64_t(0))==now-100,"raw intervals retain exact count, order and duration");
  const auto firstEnd=now;
  for(unsigned i=0;i<64;++i){now+=10;Check(samples.Push(1,2,now)==(i==63),"consecutive batches publish once");}
  Check(samples.begin==firstEnd && samples.end==now,"successive batch has no gap or double-count");
  for(unsigned change=0;change<5;++change){
    samples.Reset();Check(!samples.Push(1,2,100),"reset anchor");
    for(unsigned i=1;i<64;++i)Check(!samples.Push(1,2,100+i),"partial old batch held");
    uintptr_t swap=change==0?3:1;uint64_t world=change==1?3:2;
    uint64_t time=change==2?0:(change==3?150:(change==4?163:200));
    Check(!samples.Push(swap,world,time),"swap/world/zero/regressed/repeated clock discards old partial batch");
    if(!time){time=200;Check(!samples.Push(swap,world,time),"invalid timestamp cannot anchor next interval");}
    for(unsigned i=1;i<=64;++i)Check(samples.Push(swap,world,time+i)==(i==64),"new identity requires a complete fresh batch");
    Check(samples.begin==time && samples.end==time+64,"no interval crosses identity or clock reset");
  }
  samples.Reset();Check(!samples.Push(0,2,100),"null swap rejected");
  Check(!samples.Push(1,2,UINT64_MAX-64),"large timestamp anchor");
  for(unsigned i=1;i<=64;++i)Check(samples.Push(1,2,UINT64_MAX-64+i)==(i==64),"full 64-bit counter precision retained");
  unsigned depth=0;
  {FrameTimingScope disabled(depth,false);Check(!disabled.outer && depth==0,"disabled capture touches no depth");}
  {FrameTimingScope outer(depth,true);Check(outer.outer && depth==1,"outer Present measured");
    {FrameTimingScope nested(depth,true);Check(!nested.outer && depth==2,"nested Present1 forwarding excluded");}
    Check(depth==1,"nested return restores depth");}
  Check(depth==0,"outer return restores depth for next frame");
  std::puts("PASS: bounded raw frame timing, exact intervals, identity/clock reset and nested Present guards");
}
