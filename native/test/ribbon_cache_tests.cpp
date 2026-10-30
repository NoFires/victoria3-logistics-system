#include "ribbon_cache.h"
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <unordered_set>
using namespace v3;
void Check(bool ok, const char* label) {
  if (!ok) { std::printf("FAIL: %s\n",label); std::exit(1); }
}
Ribbon Shape(float x, const char* style="green") {
  return MakeRibbon(style, {{x,0,0},{x,0,0},{x+1,0,1},{x+1,0,1}});
}
int main() {
  Check(!MakeRibbon("green",std::vector<Vec3>(4)),"zero-length rejected");
  Check(!MakeRibbon("green",{{0,0,0},{1,0,0},{2,0,0},{2,0,0}}),"missing cubic endpoint control rejected");
  Check(!Shape(std::numeric_limits<float>::infinity()),"nonfinite rejected");
  Check(!MakeRibbon("",{{0,0,0},{0,0,0},{1,0,1},{1,0,1}}),"empty style rejected");
  auto zero=Shape(0); auto negativeZero=Shape(-0.0f);
  Check(zero->hash==negativeZero->hash && SameRibbon(*zero,*negativeZero),"signed zero canonicalised");
  RibbonCache cache;
  unsigned uploads=0; double now=0;
  auto clock=[&] { return now; };
  auto upload=[&](size_t,const RibbonGeometry&) { ++uploads; return true; };
  RibbonBatch batch;
  for (int i=0;i<40;++i) batch.push_back(Shape(float(i)));
  batch.push_back(Shape(0)); batch.push_back({});
  cache.Request(PrepareRibbonBatch(batch));
  cache.Pump(1,upload,clock);
  Check(uploads==16 && cache.Active().size()==16 && cache.Pending()==24,"cold batch capped at sixteen");
  cache.Pump(1,upload,clock);
  Check(uploads==16,"second logical pass cannot reset frame budget");
  Check(cache.Stats().peakFrameAttempts==16 && cache.Stats().overrunFrames==0,
      "recorded count peak covers repeated same-frame pumps without inventing time overruns");
  cache.Pump(2,upload,clock); cache.Pump(3,upload,clock);
  Check(uploads==40 && cache.Active().size()==40,"cold batch completes over frames");
  std::reverse(batch.begin(),batch.end());
  Check(!cache.Request(PrepareRibbonBatch(batch)),"sorting and duplicate order do not change request");
  Check(cache.Stats().repeats==1,"identical request counter does not count normal frame pumps");
  Check(cache.Stats().hits==40,"identical ready request counts every retained geometry as reused");
  cache.Pump(4,upload,clock);
  Check(uploads==40,"unchanged view has no uploads");
  cache.Clear(); Check(cache.Active().empty() && !cache.Pending(),"clear removes active and pending immediately");
  cache.Request(PrepareRibbonBatch(batch)); cache.Pump(5,upload,clock);
  Check(uploads==40 && cache.Active().size()==40,"reopen reactivates all existing geometry");
  auto reverse=MakeRibbon("green",{{1,0,1},{1,0,1},{0,0,0},{0,0,0}});
  cache.Request(PrepareRibbonBatch({reverse,Shape(0,"orange")})); cache.Pump(6,upload,clock);
  Check(uploads==42,"direction and constructor style have distinct identity");
  // Deliberately collide hashes: equality must still compare style and points.
  auto collision=std::make_shared<RibbonGeometry>(*Shape(999)); collision->hash=zero->hash;
  cache.Request(PrepareRibbonBatch({zero,collision})); cache.Pump(7,upload,clock);
  Check(cache.Active().size()==2 && uploads==43,"hash collisions cannot alias geometry");

  RibbonCache bounded(2); std::vector<std::string> slotStyles(2);
  auto typed=[&](size_t index,const RibbonGeometry& geometry) {
    Check(slotStyles[index].empty() || slotStyles[index]==geometry.style,"reuse never changes native type");
    slotStyles[index]=geometry.style; return true;
  };
  bounded.Request(PrepareRibbonBatch({Shape(0),Shape(1)})); bounded.Pump(1,typed,clock);
  bounded.Request(PrepareRibbonBatch({Shape(2),Shape(1)})); bounded.Pump(2,typed,clock);
  Check(bounded.Active().size()==2 && bounded.Size()==2 && bounded.Hits()==1,"pin all current hits before LRU reuse");
  bounded.Request(PrepareRibbonBatch({Shape(3,"orange")})); bounded.Pump(3,typed,clock);
  Check(bounded.Active().empty() && bounded.Dropped()==1 && bounded.Size()==2,"global cap drops incompatible styles");
  bounded.Request(PrepareRibbonBatch({Shape(4),Shape(5),Shape(6)})); bounded.Pump(4,typed,clock);
  Check(bounded.Active().size()==2 && bounded.Dropped()==2,"requested new entries also stay pinned");

  RibbonCache failed(2); unsigned attempts=0;
  auto fail=[&](size_t,const RibbonGeometry&) { ++attempts; return false; };
  failed.Request(PrepareRibbonBatch({Shape(0)})); failed.Pump(1,fail,clock);
  for (unsigned frame=2;frame<20;++frame) failed.Pump(frame,fail,clock);
  failed.Clear(); failed.Request(PrepareRibbonBatch({Shape(0)})); failed.Pump(20,fail,clock);
  Check(attempts==1 && failed.Failures()==1 && failed.Active().empty(),"failed geometry stays suppressed across frames/reopen");
  Check(failed.Stats().attempts==1 && failed.Stats().failures==1 && failed.Stats().seconds==0,
      "failed upload statistics stay suppressed too");

  RibbonCache timed;
  timed.Request(PrepareRibbonBatch({Shape(0),Shape(1),Shape(2)})); now=0;
  unsigned slowCalls=0;
  auto slow=[&](size_t,const RibbonGeometry&) { now+=.003; ++slowCalls; return true; };
  timed.Pump(1,slow,clock); timed.Pump(1,slow,clock);
  Check(slowCalls==1 && timed.Pending()==2,"one indivisible call may overrun soft budget, next pass cannot reset it");
  Check(timed.Stats().overrunFrames==1 && timed.Stats().peakFrameAttempts==1 &&
        timed.Stats().seconds==.003 && timed.Stats().peakFrameSeconds==.003 &&
        timed.Stats().peakStepSeconds==.003,"actual step overrun recorded once, excluding time between frames");
  now+=10; // No build work during this gap between budgeted passes.
  timed.Request(PrepareRibbonBatch({Shape(9)})); timed.Pump(1,slow,clock);
  Check(slowCalls==1,"superseding generation cannot reset same-frame budget");
  Check(timed.Stats().overrunFrames==1 && timed.Stats().seconds==.003,
      "superseding request and repeated pump cannot double-count an old overrun");
  timed.Pump(2,slow,clock);
  Check(slowCalls==2 && timed.Active().size()==1 && !timed.Pending(),"superseding batch cancels old cold work");
  Check(timed.Stats().overrunFrames==2 && std::abs(timed.Stats().seconds-.006)<1e-9,
      "next logical frame records its own overrun and cumulative CPU work");
  RibbonCache cancelled;
  cancelled.Request(PrepareRibbonBatch({Shape(0),Shape(1),Shape(2)}));
  unsigned cancellationCalls=0;
  cancelled.Pump(1,[&](size_t,const RibbonGeometry&){++cancellationCalls; return true;},clock,
      [&]{return cancellationCalls==1;});
  Check(cancellationCalls==1 && cancelled.Pending()==2 && cancelled.Failures()==0,
      "cancellation between native calls stops work without caching a false geometry failure");
  Check(!cancelled.Request(PrepareRibbonBatch({Shape(0),Shape(1),Shape(2)})) && cancelled.Stats().hits==1,
      "identical partial request reuses only completed entries, not unfinished uploads");

  RibbonCache full(99999); batch.clear();
  for (int i=0;i<8200;++i) batch.push_back(Shape(float(i)));
  full.Request(PrepareRibbonBatch(batch));
  for (uint64_t frame=1; full.Pending(); ++frame) full.Pump(frame,upload,clock);
  Check(full.Size()==8192 && full.Active().size()==8192 && full.Dropped()==8,"hard cap is global 8192 even with larger requested capacity");
  Check(full.Stats().attempts==8192 && full.Stats().dropped==8 && full.Stats().peakFrameAttempts==16,
      "pressure statistics count all uploads and rejected entries across the global pool");
  std::puts("PASS: native cache identity/dedup, collision, reopen, 8192 cap, style-safe pinning/LRU, cancellation, failed suppression, 16/frame and 2ms policy");
}
