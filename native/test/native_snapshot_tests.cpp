#include "native_snapshot.h"
#include <cstdio>
#include <cstdlib>
#include <thread>
using namespace v3;
void Check(bool ok,const char* why){if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
int main(){
  Check(!NativeCaptureDue(false,2,2,30999,1000) && NativeCaptureDue(false,2,2,31000,1000),
      "running collection waits 30 seconds, including the exact boundary");
  Check(NativeCaptureDue(true,2,2,1001,1000),"last paused command is captured without waiting for a later command");
  Check(NativeCaptureDue(false,3,2,1001,1000),"new world/request bypasses previous world's timer");
  NativeAccessObservation s;s.phase=4;s.access=2;s.owns=1;s.context=1;
  Check(IsNativeCapturePoint(0xcd2bc1,s),"completed command with native ownership accepted");
  Check(!IsNativeCapturePoint(0xcd2bc2,s),"same scope flags at another caller confer no permission");
  auto good=s;
  for(auto mode:{0u,1u,3u}){s.access=mode;Check(!IsNativeCapturePoint(0xcd2bc1,s),"non-write mode rejected");}s=good;
  s.hidden=1;Check(!IsNativeCapturePoint(0xcd2bc1,s),"nested hidden batch command rejected");s=good;
  s.owns=0;Check(!IsNativeCapturePoint(0xcd2bc1,s),"global mode without thread ownership rejected");s=good;
  s.context=0;Check(!IsNativeCapturePoint(0xcd2bc1,s),"UI context rejected");s=good;
  s.phase=3;Check(!IsNativeCapturePoint(0xcd2bc1,s),"loading phase rejected");
  NativeSnapshotMailbox box;
  auto a=std::make_shared<NativeCaptureRequest>();a->world=1;a->revision=2;
  auto result=std::make_shared<NativeCaptureResult>();auto snapshot=std::make_shared<EconomySnapshot>();
  snapshot->world=1;snapshot->revision=2;result->snapshot=snapshot;
  result->identity=good;result->identity.machine=10;result->identity.world=11;result->identity.stateDb=12;
  result->anchor=42;auto ui=result->identity;ui.access=1;ui.context=0;
  Check(NativeCaptureMatches(*result,ui,1,2,42),"owned copy can be handed to UI without granting UI a native write lease");
  Check(!NativeCaptureMatches(*result,ui,2,2,42) && !NativeCaptureMatches(*result,ui,1,3,42),"stale world/revision handoff rejected");
  ui.phase=3;Check(!NativeCaptureMatches(*result,ui,1,2,42),"phase changed after UI panel observation rejected");ui=result->identity;
  ++ui.machine;Check(!NativeCaptureMatches(*result,ui,1,2,42),"machine replacement rejected");ui=result->identity;
  ++ui.world;Check(!NativeCaptureMatches(*result,ui,1,2,42),"native world replacement rejected");ui=result->identity;
  ++ui.stateDb;Check(!NativeCaptureMatches(*result,ui,1,2,42),"state database replacement rejected");ui=result->identity;
  Check(!NativeCaptureMatches(*result,ui,1,2,0) && !NativeCaptureMatches(*result,ui,1,2,43),"missing or replaced full-handle anchor rejected");
  Check(!box.Pending() && !box.Take() && !box.Publish(a,result),"no unsolicited publication");
  box.Request(a);Check(box.Current(a) && box.Pending()==a,"request owns the pending slot");
  box.Fail(a,2,"metadata unavailable");Check(box.TakeError()=="metadata unavailable" && box.Pending()==a,
      "copy failure reaches UI without losing retry request");
  auto b=std::make_shared<NativeCaptureRequest>(*a);box.Request(b);
  box.Fail(a,2,"late error");Check(box.TakeError().empty(),"retired failure cannot clear newer scope");
  Check(!box.Current(a) && !box.Publish(a,result),"equal tags with replaced request identity reject old result");
  auto wrong=std::make_shared<NativeCaptureResult>(*result);auto changed=std::make_shared<EconomySnapshot>(*snapshot);
  changed->world=2;wrong->snapshot=changed;Check(!box.Publish(b,wrong),"wrong world rejected");
  changed->world=1;changed->revision=1;Check(!box.Publish(b,wrong),"revision predating request rejected");
  Check(box.Publish(b,result) && box.Pending()==b && !box.Publish(b,result),"ready result retains resident capture request and rejects duplicate revision");
  Check(box.Requested(),"undrained result still owns its request and cannot be overwritten by another UI poll");
  auto newer=std::make_shared<NativeCaptureResult>(*result);auto next=std::make_shared<EconomySnapshot>(*snapshot);
  next->revision=3;newer->snapshot=next;
  Check(box.Publish(b,newer) && !box.Publish(b,result),"newer completed command replaces undrained result; older command cannot overwrite it");
  Check(box.Take()==newer && box.Current(b) && box.Pending()==b && !box.Take(),"UI drain retains request for a later paused command");
  Check(!box.Publish(b,result),"old command cannot reappear after UI drain");
  box.Fail(b,2,"old command error");Check(box.TakeError().empty(),"older failure cannot clear newer command data");
  box.Request(a);std::thread late([&]{box.Retire();});late.join();
  Check(!box.Publish(a,result) && !box.Pending(),"world retirement cancels in-flight publication");
  box.Request(a);Check(box.Publish(a,result),"fresh publication after retirement");box.Retire();
  Check(!box.Take(),"retirement also removes undrained result");
  NativeSnapshotMailbox worlds;std::atomic<uint64_t> epoch{1};auto source=result->identity;
  Check(worlds.BindSource(source,epoch)==1,"first source binds without inventing another world");
  worlds.Ensure(a);Check(worlds.Publish(a,result),"first source result ready");
  ++source.world;
  Check(worlds.BindSource(source,epoch)==2 && epoch.load()==2,"native producer advances epoch before copying replacement source");
  auto second=std::make_shared<NativeCaptureRequest>(*a);second->world=2;second->revision=4;
  Check(worlds.Ensure(second)==second,"new source replaces retired request");
  auto secondResult=std::make_shared<NativeCaptureResult>(*result);auto secondSnapshot=std::make_shared<EconomySnapshot>(*snapshot);
  secondSnapshot->world=2;secondSnapshot->revision=5;secondResult->snapshot=secondSnapshot;
  Check(worlds.Publish(second,secondResult),"new source copy arrives before UI notices transition");
  worlds.Ensure(second);
  Check(worlds.Ready() && worlds.Pending()==second && worlds.LatestRevision()==5,
        "late UI world request preserves already copied new-world input while paused");
  auto duplicate=std::make_shared<NativeCaptureRequest>(*second);duplicate->revision=6;
  Check(worlds.Ensure(duplicate)==second && worlds.Take()==secondResult,"racing UI request cannot replace native source request or result");
  Check(!worlds.Publish(a,result),"old source cannot reappear after transition");
  epoch.store(3);++source.stateDb;
  Check(worlds.BindSource(source,epoch)==3 && epoch.load()==3,"UI-first transition is adopted without a second epoch advance");
  auto third=std::make_shared<NativeCaptureRequest>(*second);third->world=3;third->revision=7;
  worlds.Ensure(third);worlds.Ensure(second);
  Check(worlds.Pending()==third,"stale UI cleanup cannot retire a future world request");
  Check(worlds.BindSource(source,epoch)==3,"same native identity does not repeatedly advance epoch");
  ++source.machine;Check(worlds.BindSource(source,epoch)==4,"machine identity replacement also binds a new epoch");
  source.world=0;Check(!worlds.BindSource(source,epoch) && epoch.load()==4,"unavailable identity does not fabricate a world change");
  std::string why;unsigned reads=0;
  Check(!CopyNativeSnapshot([&](auto,auto,auto){++reads;return false;},1,1,0xcd2bc1,*a,a->revision,[]{return false;},&why) && !reads,
        "retired request performs no engine reads");
  std::puts("PASS: completed command gate, owned snapshot mailbox and stale/world rejection");
}
