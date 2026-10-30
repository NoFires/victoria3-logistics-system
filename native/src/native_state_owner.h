#pragma once
#include "state_panel.h"
#include "logistics_session.h"
#include "native_world_trade.h"
#include "variable_probe.h"
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
  if(roster.snapshot->nativeTrade){
    for(const auto& state:roster.snapshot->nativeTrade->states)
      if(state.id==UINT32_MAX || state.market==UINT32_MAX || !roster.markets.emplace(state.id,state.market).second)return {};
    return roster;
  }
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
    if(!m_active || m_world!=roster.snapshot->world || m_state!=panel.state || m_market!=found->second){
      m_selection={};
      m_context="ironroad-state-"+std::to_string(++m_epoch)+"-"+std::to_string(roster.snapshot->world)+"-"+std::to_string(panel.state);
    }
    m_selection.kind=FreightViewKind::State;m_selection.state=panel.state;
    m_selection.market=found->second;m_selection.explicitMarket=true;
    session.SetSelection(kOwnerNativeState,kOwnerNativeState,m_selection);
    m_active=true;m_world=roster.snapshot->world;m_revision=roster.snapshot->revision;m_snapshot=roster.snapshot;
    m_state=panel.state;m_market=found->second;return {true,false};
  }
  const std::string& Context() const{return m_context;}
  std::vector<std::string> FilterKeys() const {
    if(!m_active)return {};
    return {"ironroad_state_good_v1","ironroad_state_direction_v1","ironroad_state_transport_v1","ironroad_state_color_v1"};
  }
  bool UpdateFilters(LogisticsSession& session,const GuiVariableProbe& variables,
                     const std::unordered_map<std::string,uint32_t>& goodsByKey) {
    if(!m_active || !variables.ok || !session.SnapshotIsCurrent(m_snapshot))return false;
    std::map<std::string,std::string> values;
    const auto keys=FilterKeys();
    for(const auto& entry:variables.entries){
      if(std::find(keys.begin(),keys.end(),entry.key)==keys.end())continue;
      if(!entry.value.starts_with(m_context+":"))continue;
      const auto value=entry.value.substr(m_context.size()+1);
      if(!values.emplace(entry.key,value).second)return false;
    }
    auto selection=m_selection;selection.selectedGood=-1;selection.mapDirection=0;
    selection.mapTransport=FreightTransport::All;
    selection.colorMode=FreightColorMode::Business;
    if(const auto found=values.find(keys[0]);found!=values.end() && found->second!="all"){
      const auto good=goodsByKey.find(found->second);if(good==goodsByKey.end())return false;
      selection.selectedGood=good->second;
    }
    if(const auto found=values.find(keys[1]);found!=values.end() && found->second!="all"){
      if(found->second=="imports")selection.mapDirection=1;
      else if(found->second=="exports")selection.mapDirection=-1;
      else return false;
    }
    if(const auto found=values.find(keys[2]);found!=values.end() && found->second!="all"){
      if(found->second=="land")selection.mapTransport=FreightTransport::Land;
      else if(found->second=="sea")selection.mapTransport=FreightTransport::Sea;
      else return false;
    }
    if(const auto found=values.find(keys[3]);found!=values.end() && found->second!="all"){
      if(found->second=="goods")selection.colorMode=FreightColorMode::Goods;
      else if(found->second=="native")selection.colorMode=FreightColorMode::Native;
      else return false;
    }
    if(selection.selectedGood==m_selection.selectedGood && selection.mapDirection==m_selection.mapDirection &&
       selection.mapTransport==m_selection.mapTransport && selection.colorMode==m_selection.colorMode)return false;
    m_selection=std::move(selection);session.SetSelection(kOwnerNativeState,kOwnerNativeState,m_selection);return true;
  }
  bool CanPublish(const LogisticsSession& session,const LogisticsDocument& document,
                  const std::shared_ptr<const FreightFrame>& frame,uint64_t) const {
    const auto status=session.Status();
    return m_active && status.owner==kOwnerNativeState && status.snapshotReady && !status.synthetic &&
        !document.synthetic && document.kind==FreightViewKind::State && document.state==m_state &&
        document.market==m_market && document.world==m_world && document.revision==m_revision &&
        status.world==m_world && status.revision==m_revision && frame &&
        session.SnapshotIsCurrent(m_snapshot) &&
        frame->filter.kind==FreightViewKind::State && frame->filter.state==m_state &&
        frame->filter.market==m_market && session.StatisticsAreCurrent(frame);
  }
  std::string ReadyId() const{return m_active?std::to_string(int32_t(m_state)):"ironroad-state-unavailable";}
  void Close(LogisticsSession& session){
    if(m_active)session.Close(kOwnerNativeState);
    m_active=false;m_world=m_revision=0;m_state=m_market=UINT32_MAX;m_snapshot.reset();
    m_context="ironroad-state-inactive";m_selection={};
  }
 private:
  bool m_active=false;
  uint64_t m_world=0,m_revision=0,m_epoch=0;
  uint32_t m_state=UINT32_MAX,m_market=UINT32_MAX;
  std::shared_ptr<const EconomySnapshot> m_snapshot;
  LogisticsSelection m_selection;
  std::string m_context="ironroad-state-inactive";
};
} // namespace v3
