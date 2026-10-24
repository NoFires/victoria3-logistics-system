#pragma once
#include "state_panel.h"
#include "logistics_session.h"
#include <unordered_map>

namespace v3 {
// Built once per accepted copied snapshot, never by scanning native objects on
// the UI/render path. Repeated goods rows must agree on their state's market.
struct NativeStateRoster {
  std::shared_ptr<const EconomySnapshot> snapshot;
  std::unordered_map<uint32_t,uint32_t> markets;
};
inline NativeStateRoster CopyNativeStateRoster(std::shared_ptr<const EconomySnapshot> snapshot) {
  if(!snapshot || !snapshot->world || !snapshot->revision)return {};
  NativeStateRoster roster;roster.snapshot=std::move(snapshot);
  for(const auto& row:roster.snapshot->orders){
    if(row.state==UINT32_MAX || row.market==UINT32_MAX)return {};
    const auto [found,added]=roster.markets.emplace(row.state,row.market);
    if(!added && found->second!=row.market)return {};
  }
  return roster;
}

// Numeric candidate seam only; native decoder/GUI publication are not enabled
// until live state ownership and state-ID correspondence have been verified.
class NativeStateOwner {
 public:
  struct Change {bool changed=false,closed=false;};
  Change Update(LogisticsSession& session,const NativeStatePanelState& panel,
                const NativeStateRoster& roster,bool metadataReady) {
    const auto status=session.Status();
    const auto found=roster.markets.find(panel.state);
    const bool active=metadataReady && status.snapshotReady && !status.synthetic &&
        panel.available && panel.ingame && panel.stateOpen && panel.tab=="local_goods" &&
        session.SnapshotIsCurrent(roster.snapshot) &&
        panel.state!=UINT32_MAX && found!=roster.markets.end();
    if(!active){
      if(!m_active)return {};
      Close(session);return {true,true};
    }
    if(m_active && m_snapshot==roster.snapshot &&
       m_state==panel.state && m_market==found->second)return {};
    LogisticsSelection selection;selection.kind=FreightViewKind::State;
    selection.state=panel.state;selection.market=found->second;selection.explicitMarket=true;
    session.SetSelection(kOwnerNativeState,kOwnerNativeState,std::move(selection));
    m_active=true;m_world=roster.snapshot->world;m_revision=roster.snapshot->revision;m_snapshot=roster.snapshot;
    m_state=panel.state;m_market=found->second;return {true,false};
  }
  bool CanPublish(const LogisticsSession& session,const LogisticsDocument& document,
                  const std::shared_ptr<const FreightFrame>& frame,uint64_t applied) const {
    const auto status=session.Status();
    return m_active && status.owner==kOwnerNativeState && status.snapshotReady && !status.synthetic &&
        !document.synthetic && document.kind==FreightViewKind::State && document.state==m_state &&
        document.market==m_market && document.world==m_world && document.revision==m_revision &&
        status.world==m_world && status.revision==m_revision && frame &&
        session.SnapshotIsCurrent(m_snapshot) &&
        frame->filter.kind==FreightViewKind::State && frame->filter.state==m_state &&
        frame->filter.market==m_market && session.FrameIsCurrent(frame,applied);
  }
  std::string ReadyId() const{return m_active?std::to_string(int32_t(m_state)):"ironroad-state-unavailable";}
  void Close(LogisticsSession& session){
    if(m_active)session.Close(kOwnerNativeState);
    m_active=false;m_world=m_revision=0;m_state=m_market=UINT32_MAX;m_snapshot.reset();
  }
 private:
  bool m_active=false;
  uint64_t m_world=0,m_revision=0;
  uint32_t m_state=UINT32_MAX,m_market=UINT32_MAX;
  std::shared_ptr<const EconomySnapshot> m_snapshot;
};
} // namespace v3
