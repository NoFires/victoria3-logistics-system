#pragma once
#include "native_access.h"
#include "native_economy.h"
#include "native_world_trade.h"
#include "native_splines.h"
#include "native_sea_geography.h"
#include "native_ports.h"
#include "native_domestic.h"
#include "native_lane_access.h"
#include "goods_snapshot.h"
#include <mutex>
#include <atomic>

namespace v3 {
// Only the exact command-runner call site, before its native scope destructor,
// may borrow this range. Observing the same flags elsewhere grants no lease.
inline bool IsNativeCapturePoint(uint64_t caller,const NativeAccessObservation& s) {
  return caller==kRvaCommandScopeReturn && s.phase==4 && s.access==2 && !s.hidden && s.owns==1 && s.context==1;
}
inline bool NativeCaptureDue(bool paused,uint64_t request,uint64_t copiedRequest,uint64_t now,uint64_t copiedAt) {
  return paused || request!=copiedRequest || now-copiedAt>=30000;
}
struct NativeCaptureRequest {
  // revision is the minimum version allocated for this resident request;
  // each completed-command copy receives its own newer snapshot revision.
  uint64_t world=0,revision=0;
  uint32_t marketOverride=0;
};
// Adapter-only owned batch and independent collection witness. The witness
// reads only our collection epoch, never engine objects or native permissions.
// A producer must freeze the batch and retire its epoch on every invalidation;
// deriving generation/epoch from the batch itself would defeat this guard.
// The producer advances this epoch monotonically; it must never restore an old
// value or reuse the token for a different collection lifetime.
struct NativeLaneAccessInput {
  std::shared_ptr<const NativeLaneAccessBatch> batch;
  uint64_t generation=0;
  std::shared_ptr<const std::atomic<uint64_t>> epoch;
  bool IsCurrent(uint64_t world,uint64_t revision) const {
    return batch && world && revision && generation &&
        batch->scope==NativeLaneAccessScope{world,revision,generation} && epoch && epoch->load()==generation;
  }
};
struct NativeCaptureResult {
  std::shared_ptr<const EconomySnapshot> snapshot;
  std::vector<NativeStateEconomy> states;
  GoodsCatalog catalog;
  // Adapter-only identity witnesses; never dereferenced or passed to workers.
  NativeAccessObservation identity;
  uint64_t anchor=0;
  uint32_t anchorId=0;
  // Owned collection witness stays in the adapter/mailbox, outside the worker
  // EconomySnapshot. Retaining the result does not extend the collection epoch.
  std::shared_ptr<const std::atomic<uint64_t>> laneAccessEpoch;
  uint64_t laneAccessGeneration=0;
  bool LaneAccessCurrent() const {
    return laneAccessGeneration?laneAccessEpoch && laneAccessEpoch->load()==laneAccessGeneration:!laneAccessEpoch;
  }
};
inline bool NativeCaptureMatches(const NativeCaptureResult& copy,const NativeAccessObservation& now,
                                uint64_t world,uint64_t revision,uint64_t anchor) {
  return copy.LaneAccessCurrent() && copy.snapshot && now.phase==4 && copy.snapshot->world==world && copy.snapshot->revision==revision &&
      now.machine==copy.identity.machine && now.world==copy.identity.world && now.stateDb==copy.identity.stateDb &&
      anchor && anchor==copy.anchor;
}
class NativeSnapshotMailbox {
 public:
  // Called by the verified native world destructor BEFORE any owned manager
  // is destroyed. Compare owned identity only; never dereference this pointer.
  uint64_t RetireWorld(uint64_t nativeWorld,std::atomic<uint64_t>& world) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(!nativeWorld || !m_sourceWorld || nativeWorld!=m_source.world)return 0;
    uint64_t expected=m_sourceWorld;
    world.compare_exchange_strong(expected,m_sourceWorld+1);
    const auto epoch=world.load();
    // Replace the resident request, so late old-source publications are
    // rejected while the first paused command can still capture the new world.
    if(m_request && m_request->world<epoch) {
      auto next=std::make_shared<NativeCaptureRequest>(*m_request);next->world=epoch;m_request=std::move(next);
    }
    m_result.reset();m_error.clear();m_revision=0;m_source={};m_sourceWorld=0;
    return epoch;
  }
  // Bind before copying a replacement source. A UI-detected change may already
  // have advanced the epoch; CAS avoids advancing it for the same change twice.
  uint64_t BindSource(const NativeAccessObservation& source,std::atomic<uint64_t>& world) {
    if(!source.machine || !source.world || !source.stateDb)return 0;
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_sourceWorld && (m_source.machine!=source.machine || m_source.world!=source.world ||
                        m_source.stateDb!=source.stateDb)) {
      uint64_t expected=m_sourceWorld;
      world.compare_exchange_strong(expected,m_sourceWorld+1);
    }
    m_source=source;m_sourceWorld=world.load();return m_sourceWorld;
  }
  std::shared_ptr<const NativeCaptureRequest> Ensure(std::shared_ptr<const NativeCaptureRequest> request) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(request && (!m_request || m_request->world<request->world)) {
      m_request=std::move(request);m_result.reset();m_error.clear();m_revision=0;
    }
    return m_request;
  }
  uint64_t LatestRevision() const {
    std::lock_guard<std::mutex> lock(m_mutex);return m_revision;
  }
  void Request(std::shared_ptr<const NativeCaptureRequest> request) {
    std::lock_guard<std::mutex> lock(m_mutex);m_request=std::move(request);m_result.reset();m_error.clear();m_revision=0;
  }
  std::shared_ptr<const NativeCaptureRequest> Pending() const {
    std::lock_guard<std::mutex> lock(m_mutex);return m_request;
  }
  bool Ready() const {
    std::lock_guard<std::mutex> lock(m_mutex);return m_result || !m_error.empty();
  }
  bool Requested() const {
    std::lock_guard<std::mutex> lock(m_mutex);return bool(m_request);
  }
  bool Current(const std::shared_ptr<const NativeCaptureRequest>& request) const {
    std::lock_guard<std::mutex> lock(m_mutex);return request && request==m_request;
  }
  bool Publish(const std::shared_ptr<const NativeCaptureRequest>& request,
               std::shared_ptr<const NativeCaptureResult> result) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(!request || request!=m_request || !result || !result->snapshot ||
       result->snapshot->world!=request->world || result->snapshot->revision<request->revision ||
       result->snapshot->revision<=m_revision)return false;
    if(!result->LaneAccessCurrent()){
      m_revision=result->snapshot->revision;m_result.reset();m_error="native lane access retired before publication";return false;
    }
    m_revision=result->snapshot->revision;m_result=std::move(result);m_error.clear();return true;
  }
  void Fail(const std::shared_ptr<const NativeCaptureRequest>& request,uint64_t revision,std::string why) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(request && request==m_request && revision>=request->revision && revision>m_revision){
      m_revision=revision;m_result.reset();m_error=std::move(why);
    }
  }
  std::string TakeError() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_result && !m_result->LaneAccessCurrent()){
      m_result.reset();m_error="native lane access retired before drain";
    }
    auto error=std::move(m_error);m_error.clear();return error;
  }
  std::shared_ptr<const NativeCaptureResult> Take() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if(m_result && !m_result->LaneAccessCurrent()){
      m_result.reset();m_error="native lane access retired before drain";return {};
    }
    return std::move(m_result);
  }
  void Retire(){Request(nullptr);}
 private:
  mutable std::mutex m_mutex;
  std::shared_ptr<const NativeCaptureRequest> m_request;
  std::shared_ptr<const NativeCaptureResult> m_result;
  std::string m_error;
  uint64_t m_revision=0;
  NativeAccessObservation m_source;
  uint64_t m_sourceWorld=0;
};
// Caller already owns the verified completed-command scope. The only engine
// calls are separately injected adapters, after validating their live inputs.
// The optional strait adapter is unbound in production until its ABI and
// access window are validated. No GUI access or allocation solver runs here.
inline std::shared_ptr<const NativeCaptureResult> CopyNativeSnapshot(
    const GoodsMemoryReader& memory,uint64_t module,uint64_t tlsArray,uint64_t caller,
    const NativeCaptureRequest& request,uint64_t revision,const std::function<bool()>& current,std::string* error=nullptr,
    const std::function<uint64_t(uint64_t,uint32_t,uint32_t)>& select={},
    const std::function<std::optional<bool>(uint64_t,uint64_t)>& selectAccess={},
    const NativeLaneAccessInput* laneAccess=nullptr,
    const std::function<std::optional<uint32_t>(uint64_t)>& selectHub={}) {
  auto fail=[&](const std::string& why)->std::shared_ptr<const NativeCaptureResult>{if(error)*error=why;return {};};
  if(!current || !current() || !request.world || !request.revision || revision<request.revision)
    return fail("missing current snapshot request");
  if(laneAccess && (selectAccess || !laneAccess->IsCurrent(request.world,revision)))
    return fail("unavailable or conflicting native lane access input");
  const auto before=ObserveNativeAccess(memory,module,tlsArray);
  if(!before || !IsNativeCapturePoint(caller,*before))return fail("outside completed command scope");
  if(!select)return fail("native trade selector unavailable");
  if(!selectHub)return fail("native current world hub getter unavailable");
  EconomyReadWindow window(memory);
  const GoodsMemoryReader read=[&](uint64_t a,void* p,size_t n){return window.Read(a,p,n);};
  auto catalog=DecodeThreadGoodsCatalog(read,module,tlsArray);
  if(!catalog.ok)return fail(catalog.error);
  std::string why;
  auto states=DecodeNativeTradeStates(read,module,current,&why);
  if(!states)return fail(why);
  auto snapshot=std::make_shared<EconomySnapshot>();snapshot->world=request.world;snapshot->revision=revision;
  if(!PopulateSnapshotGoods(*snapshot,catalog,&why))return fail(why);
  auto trade=DecodeNativeWorldTrade(read,module,*states,uint32_t(catalog.goods.size()),current,select,&why,selectHub);
  if(!trade)return fail(why);
  auto geometry=DecodeNativeSplineNetwork(read,module,current,&why);
  if(!geometry)return fail(why);
  auto domestic=DecodeNativeDomesticSources(read,module,*states,uint32_t(catalog.goods.size()),*geometry,current,&why);
  if(!domestic)return fail(why);
  if(laneAccess){
    if(!current() || !laneAccess->IsCurrent(request.world,revision))
      return fail("native lane access retired before binding");
    std::vector<uint32_t> straits;straits.reserve(trade->straits.size());
    for(const auto& strait:trade->straits)straits.push_back(strait.id);
    auto access=BindNativeLaneAccess(*laneAccess->batch,{request.world,revision,laneAccess->generation},
        domestic->shipping,straits,&why);
    if(!access)return fail(why);
    domestic->laneStraitAccess=std::move(*access);
  }else if(selectAccess){
    auto access=DecodeDomesticStraitAccess(read,module,*domestic,*trade,current,selectAccess,&why);
    if(!access)return fail(why);
    domestic->straitAccess=std::move(*access);
  }
  trade->domestic=std::make_shared<const NativeDomesticSources>(std::move(*domestic));
  auto sea=DecodeNativeSeaGeography(read,module,tlsArray,*states,*trade,current,&why);
  if(!sea)return fail(why);
  auto hubs=DecodeNativePortHubs(read,module,sea->states,current,&why);
  if(!hubs)return fail(why);
  sea->stateHubs=std::move(*hubs);
  trade->seaGeography=std::make_shared<const NativeSeaGeography>(std::move(*sea));
  trade->geometry=std::make_shared<const NativeSplineNetwork>(std::move(*geometry));
  snapshot->nativeTrade=std::make_shared<const NativeWorldTradeSnapshot>(std::move(*trade));
  auto result=std::make_shared<NativeCaptureResult>();
  result->anchorId=states->front().id;
  result->anchor=ReadNativeStateObject(read,module,result->anchorId);
  if(!result->anchor || !window.Unchanged())return fail("combined snapshot changed during copy");
  const auto after=ObserveNativeAccess(memory,module,tlsArray);
  if(!after || !IsNativeCapturePoint(caller,*after) || before->machine!=after->machine ||
     before->world!=after->world || before->stateDb!=after->stateDb || before->date!=after->date || !current())
    return fail("completed command identity changed during copy");
  std::map<uint32_t,size_t> markets;
  for(const auto& state:*states)++markets[state.market];
  size_t largest=0;
  for(const auto& [market,count]:markets)if(count>largest){largest=count;snapshot->primaryMarket=market;}
  if(request.marketOverride && markets.count(request.marketOverride))snapshot->primaryMarket=request.marketOverride;
  // Recheck after all copying/validation and before the owned result escapes.
  // Mailbox request identity still has to be checked separately at publication.
  if(laneAccess && (!current() || !laneAccess->IsCurrent(request.world,revision)))
    return fail("native lane access retired during snapshot copy");
  if(laneAccess){result->laneAccessEpoch=laneAccess->epoch;result->laneAccessGeneration=laneAccess->generation;}
  result->snapshot=std::move(snapshot);result->states=std::move(*states);
  result->catalog=std::move(catalog);result->identity=*after;
  if(error)error->clear();return result;
}
} // namespace v3
