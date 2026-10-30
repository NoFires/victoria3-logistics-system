#include "native_snapshot.h"
#include <cstdio>
#include <cstdlib>
#include <thread>
#include <map>
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
  {
    NativeSnapshotMailbox lifetime;std::atomic<uint64_t> generation{1};const auto same=result->identity;
    lifetime.BindSource(same,generation);lifetime.Ensure(a);Check(lifetime.Publish(a,result),"old source has an undrained result before destruction");
    Check(!lifetime.RetireWorld(0,generation) && !lifetime.RetireWorld(same.world+1,generation) && lifetime.Ready(),
        "foreign or absent native world destruction cannot retire the bound source");
    Check(lifetime.RetireWorld(same.world,generation)==2 && generation.load()==2 && !lifetime.Ready() && !lifetime.Take(),
        "native destruction retires epoch and undrained data before any replacement identity is observed");
    Check(!lifetime.RetireWorld(same.world,generation) && generation.load()==2,"the same destruction event cannot retire twice");
    bool lateAccepted=true;std::thread lateSource([&]{lateAccepted=lifetime.Publish(a,result);});lateSource.join();
    Check(!lateAccepted,"late completed old-world result cannot reappear after the destructor event");
    const auto pending=lifetime.Pending();
    Check(pending && pending!=a && pending->world==2 && pending->revision==a->revision,
        "destruction keeps a new-generation resident request for the first paused replacement command");
    Check(lifetime.BindSource(same,generation)==2,"exactly reused machine/world/database addresses still belong to the new epoch");
    auto fresh=std::make_shared<NativeCaptureResult>(*result);auto freshSnapshot=std::make_shared<EconomySnapshot>(*snapshot);
    freshSnapshot->world=2;freshSnapshot->revision=3;fresh->snapshot=freshSnapshot;
    Check(lifetime.Publish(pending,fresh) && lifetime.Take()==fresh,"first paused replacement result is publishable without waiting for another UI request");
    generation.store(3);
    Check(lifetime.RetireWorld(same.world,generation)==3 && generation.load()==3,
        "UI-first retirement and original destructor event advance the same epoch only once");
    Check(lifetime.BindSource(same,generation)==3 && lifetime.RetireWorld(same.world,generation)==4,
        "a later real destruction after address reuse retires the later object lifetime too");
  }
  std::string why;unsigned reads=0;
  const auto forbidden=[](uint64_t){Check(false,"retired or unleased source cannot invoke native selector");return uint64_t(0);};
  const auto forbiddenAccess=[](uint64_t,uint64_t)->std::optional<bool>{Check(false,"retired or unleased source cannot invoke strait access");return {};};
  Check(!CopyNativeSnapshot([&](auto,auto,auto){++reads;return false;},1,1,0xcd2bc1,*a,a->revision,[]{return false;},&why,forbidden,forbiddenAccess) && !reads,
        "retired request performs no engine reads");
  {
    constexpr uint64_t module=0x140000000,tlsArray=0x10000,tls=0x20000,
        root=0x30000,machine=0x40000,world=0x50000,db=0x60000,idler=0x70000;
    std::map<uint64_t,uint8_t> bytes;
    auto put=[&](uint64_t at,auto value){for(size_t i=0;i<sizeof(value);++i)bytes[at+i]=((uint8_t*)&value)[i];};
    GoodsMemoryReader memory=[&](uint64_t at,void* out,size_t n){
      for(size_t i=0;i<n;++i){auto f=bytes.find(at+i);if(f==bytes.end())return false;((uint8_t*)out)[i]=f->second;}return true;
    };
    put(tlsArray,tls);put(tls+0x5ae0,uint8_t(1));put(tls+0x92c,uint32_t(1));
    put(module+0x0514e580,uint32_t(2));put(module+0x0514e570,uint32_t(0));
    put(module+0x0592d0e0,idler);put(idler,module+0x04431500);put(idler+0x20,uint8_t(1));
    put(module+0x0592d128,root);put(root,module+0x04430800);put(root+0x588,uint32_t(4));put(root+0x608,machine);
    put(machine+8,uint32_t(59883360));put(machine+0x120,world);put(module+0x05964ef0,db);
    // A formal native request needs no embedded map/rail graph. Even its access
    // rejection must be reached without those former prerequisites.
    auto valid=bytes;auto request=*a;
    auto rejected=[&](uint64_t caller){
      Check(!CopyNativeSnapshot(memory,module,tlsArray,caller,request,request.revision,[]{return true;},&why,forbidden,forbiddenAccess) &&
          why=="outside completed command scope","selector cannot run outside the verified completed-command access scope");
    };
    rejected(0xcd2bc2);
    for(auto [at,value]:{std::pair{tls+0x92c,uint32_t(0)},std::pair{module+0x0514e580,uint32_t(1)},
                       std::pair{module+0x0514e570,uint32_t(1)},std::pair{root+0x588,uint32_t(3)}}){
      put(at,value);rejected(0xcd2bc1);bytes=valid;
    }
    put(tls+0x5ae0,uint8_t(0));rejected(0xcd2bc1);bytes=valid;
    Check(!CopyNativeSnapshot(memory,module,tlsArray,0xcd2bc1,request,request.revision,[]{return true;},&why) &&
        why=="native trade selector unavailable","missing native selector is unavailable, never a first-candidate fallback");
  }
  std::puts("PASS: completed command gate, owned snapshot mailbox and stale/world rejection");
}
