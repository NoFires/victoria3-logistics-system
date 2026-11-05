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
  Check(IsNativeCapturePoint(kRvaCommandScopeReturn,s),"completed command with native ownership accepted");
  Check(!IsNativeCapturePoint(kRvaCommandScopeReturn+1,s),"same scope flags at another caller confer no permission");
  auto good=s;
  for(auto mode:{0u,1u,3u}){s.access=mode;Check(!IsNativeCapturePoint(kRvaCommandScopeReturn,s),"non-write mode rejected");}s=good;
  s.hidden=1;Check(!IsNativeCapturePoint(kRvaCommandScopeReturn,s),"nested hidden batch command rejected");s=good;
  s.owns=0;Check(!IsNativeCapturePoint(kRvaCommandScopeReturn,s),"global mode without thread ownership rejected");s=good;
  s.context=0;Check(!IsNativeCapturePoint(kRvaCommandScopeReturn,s),"UI context rejected");s=good;
  s.phase=3;Check(!IsNativeCapturePoint(kRvaCommandScopeReturn,s),"loading phase rejected");
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
  const auto forbidden=[](uint64_t,uint32_t,uint32_t){Check(false,"retired or unleased source cannot invoke native selector");return uint64_t(0);};
  const auto forbiddenAccess=[](uint64_t,uint64_t)->std::optional<bool>{Check(false,"retired or unleased source cannot invoke strait access");return {};};
  {
    auto batch=std::make_shared<NativeLaneAccessBatch>();batch->scope={1,2,7};
    auto collection=std::make_shared<std::atomic<uint64_t>>(7);
    NativeLaneAccessInput input{batch,7,collection};
    Check(input.IsCurrent(1,2),"owned lane batch requires an independent current collection witness");
    Check(!input.IsCurrent(2,2) && !input.IsCurrent(1,3),"wrong world/revision rejected");
    input.generation=6;
    Check(!input.IsCurrent(1,2),"old collection generation cannot validate its own batch");
    input.generation=7;collection->store(8);
    Check(!input.IsCurrent(1,2),"collection retirement invalidates a retained immutable batch");
    GoodsMemoryReader unread=[&](auto,auto,auto){++reads;return false;};
    reads=0;
    Check(!CopyNativeSnapshot(unread,1,1,kRvaCommandScopeReturn,*a,a->revision,[]{return true;},&why,forbidden,{},&input) &&
        !reads && why=="unavailable or conflicting native lane access input",
        "retired lane collection prevents snapshot engine reads");
    auto conflicting=input;conflicting.epoch=std::make_shared<std::atomic<uint64_t>>(7);
    Check(!CopyNativeSnapshot(unread,1,1,kRvaCommandScopeReturn,*a,a->revision,[]{return true;},&why,forbidden,forbiddenAccess,&conflicting) &&
        !reads && why=="unavailable or conflicting native lane access input",
        "lane-bound input and live country permission adapter cannot be mixed");
    input=conflicting;auto saved=input;input.epoch.reset();
    Check(!input.IsCurrent(1,2),"missing collection witness is unavailable");input=saved;
    input.generation=0;Check(!input.IsCurrent(1,2),"zero collection generation is unavailable");input=saved;
    input.batch.reset();Check(!input.IsCurrent(1,2),"missing owned batch is unavailable");input=saved;
    batch.reset();Check(input.IsCurrent(1,2),"adapter input owns the batch after producer releases it");
    input=saved;collection.reset();Check(input.IsCurrent(1,2),"collection witness owns no borrowed producer storage");
    auto retired=std::make_shared<std::atomic<uint64_t>>(8);input.epoch=retired;
    Check(!input.IsCurrent(1,2),"ownership alone cannot extend the collection epoch");
    auto invalid=std::make_shared<NativeLaneAccessBatch>();invalid->scope={0,0,7};
    input.batch=invalid;
    Check(!input.IsCurrent(0,0),"zero world/revision cannot identify a current batch");
  }
  {
    NativeSnapshotMailbox lanes;lanes.Request(a);
    auto collection=std::make_shared<std::atomic<uint64_t>>(7);
    auto stamped=std::make_shared<NativeCaptureResult>(*result);
    stamped->laneAccessEpoch=collection;stamped->laneAccessGeneration=7;
    Check(lanes.Publish(a,stamped),"current lane collection can enter resident mailbox");
    collection->store(8);
    Check(lanes.Ready() && lanes.TakeError()=="native lane access retired before drain" && !lanes.Take() && lanes.Current(a),
        "undrained stale collection produces existing UI withdrawal error while retaining retry request");
    auto fresh=std::make_shared<NativeCaptureResult>(*stamped);
    auto updated=std::make_shared<EconomySnapshot>(*snapshot);updated->revision=3;fresh->snapshot=updated;
    fresh->laneAccessGeneration=8;
    Check(lanes.Publish(a,fresh) && !lanes.Publish(a,stamped) && lanes.TakeError().empty(),
        "late stale collection cannot clear a newer current result");
    auto retained=lanes.Take();
    Check(retained==fresh && NativeCaptureMatches(*retained,retained->identity,1,3,retained->anchor),
        "current drained result passes adapter handoff");
    collection->store(9);
    Check(!NativeCaptureMatches(*retained,retained->identity,1,3,retained->anchor),
        "retained result fails handoff after its collection retires");
    updated=std::make_shared<EconomySnapshot>(*snapshot);updated->revision=4;
    auto late=std::make_shared<NativeCaptureResult>(*fresh);late->snapshot=updated;
    Check(!lanes.Publish(a,late) && lanes.TakeError()=="native lane access retired before publication",
        "retirement between copy return and publication rejects result and signals withdrawal");
    late->laneAccessGeneration=9;updated=std::make_shared<EconomySnapshot>(*snapshot);updated->revision=5;late->snapshot=updated;
    Check(lanes.Publish(a,late),"a newer snapshot can retry with the current collection generation");
    collection->store(10);
    Check(!lanes.Take() && lanes.TakeError()=="native lane access retired before drain",
        "retirement racing the UI error check is rejected at drain too");
    auto malformed=*late;malformed.laneAccessEpoch.reset();
    Check(!malformed.LaneAccessCurrent(),"nonzero generation without collection token rejected");
    malformed=*late;malformed.laneAccessGeneration=0;
    Check(!malformed.LaneAccessCurrent(),"token with absent generation cannot bypass collection checks");
    Check(result->LaneAccessCurrent(),"legacy result without per-lane collection witness remains supported");
  }
  Check(!CopyNativeSnapshot([&](auto,auto,auto){++reads;return false;},1,1,kRvaCommandScopeReturn,*a,a->revision,[]{return false;},&why,forbidden,forbiddenAccess) && !reads,
        "retired request performs no engine reads");
  {
    constexpr uint64_t module=0x140000000,tlsArray=0x10000,tls=0x20000,
        root=0x30000,machine=0x40000,world=0x50000,db=0x60000,idler=0x70000;
    std::map<uint64_t,uint8_t> bytes;
    auto put=[&](uint64_t at,auto value){for(size_t i=0;i<sizeof(value);++i)bytes[at+i]=((uint8_t*)&value)[i];};
    GoodsMemoryReader memory=[&](uint64_t at,void* out,size_t n){
      for(size_t i=0;i<n;++i){auto f=bytes.find(at+i);if(f==bytes.end())return false;((uint8_t*)out)[i]=f->second;}return true;
    };
    put(tlsArray,tls);put(tls+0x5b60,uint8_t(1));put(tls+0x34c,uint32_t(1));
    put(module+0x05404748,uint32_t(2));put(module+0x05404738,uint32_t(0));
    put(module+0x05c07c18,idler);put(idler,module+0x04678bc0);put(idler+0x20,uint8_t(1));
    put(module+0x05c07c68,root);put(root,module+0x04660618);put(root+0x588,uint32_t(4));put(root+0x608,machine);
    put(machine+8,uint32_t(59883360));put(machine+0x120,world);put(module+0x05bd69c8,db);
    // A formal native request needs no embedded map/rail graph. Even its access
    // rejection must be reached without those former prerequisites.
    auto valid=bytes;auto request=*a;
    auto rejected=[&](uint64_t caller){
      Check(!CopyNativeSnapshot(memory,module,tlsArray,caller,request,request.revision,[]{return true;},&why,forbidden,forbiddenAccess) &&
          why=="outside completed command scope","selector cannot run outside the verified completed-command access scope");
    };
    rejected(kRvaCommandScopeReturn+1);
    for(auto [at,value]:{std::pair{tls+0x34c,uint32_t(0)},std::pair{module+0x05404748,uint32_t(1)},
                       std::pair{module+0x05404738,uint32_t(1)},std::pair{root+0x588,uint32_t(3)}}){
      put(at,value);rejected(kRvaCommandScopeReturn);bytes=valid;
    }
    put(tls+0x5b60,uint8_t(0));rejected(kRvaCommandScopeReturn);bytes=valid;
  }
  std::puts("PASS: completed command gate, owned snapshot mailbox and stale/world rejection");
}
