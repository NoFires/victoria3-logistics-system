#pragma once
#include "market_panel.h"
#include "logistics_session.h"
#include "variable_probe.h"
#include <unordered_map>
namespace v3 {
// Owned numeric UI state -> existing arbitration API. No engine pointers or
// economy writes; repeated timer samples do not replace settled worker requests.
class NativeMarketOwner {
 public:
  struct Change {bool changed=false, closed=false;};
  Change Update(LogisticsSession& session,const NativeMarketPanelState& panel,
                const std::unordered_set<uint32_t>& markets,bool metadataReady,int overrideMarket=0) {
    const auto status=session.Status();
    const uint32_t market=overrideMarket>0?uint32_t(overrideMarket):panel.market;
    const bool active=metadataReady && status.snapshotReady && !status.synthetic &&
        panel.available && panel.ingame && panel.marketOpen && panel.tab=="ironroad_logistics" &&
        panel.visibleGoods.has_value() && markets.count(market);
    if(!active){
      if(!m_active)return {};
      Close(session);return {true,true};
    }
    auto goods=*panel.visibleGoods;
    std::sort(goods.begin(),goods.end());goods.erase(std::unique(goods.begin(),goods.end()),goods.end());
    const bool contextChanged=!m_active || market!=m_market || panel.market!=m_panelMarket || status.world!=m_world;
    if(!contextChanged && goods==m_goods)return {};
    if(m_hover>=0){session.Close(kOwnerHover);m_hover=-1;}
    if(contextChanged)m_color=FreightColorMode::Business;
    if(contextChanged)m_context="ironroad-hover-"+std::to_string(++m_epoch)+"-"+
                                  std::to_string(status.world)+"-"+std::to_string(panel.market);
    LogisticsSelection selection;selection.kind=FreightViewKind::Market;
    selection.market=market;selection.flowsVisible=true;
    selection.explicitMarket=true;
    selection.colorMode=m_color;
    selection.restrictGoods=true;selection.allowedGoods=goods;
    session.SetSelection(kOwnerNativePanel,kOwnerNativePanel,std::move(selection));
    m_active=true;m_market=market;m_panelMarket=panel.market;m_world=status.world;m_goods=std::move(goods);return {true,false};
  }
  bool UpdateColor(LogisticsSession& session,const GuiVariableProbe& variables) {
    if(!m_active || !variables.ok || session.Status().world!=m_world)return false;
    FreightColorMode color=FreightColorMode::Business;bool seen=false;
    for(const auto& entry:variables.entries){
      if(entry.key!="ironroad_market_color_v1" || !entry.value.starts_with(m_context+":"))continue;
      if(seen)return false;seen=true;
      const auto value=entry.value.substr(m_context.size()+1);
      if(value=="goods")color=FreightColorMode::Goods;
      else if(value=="native")color=FreightColorMode::Native;
      else if(value!="all")return false;
    }
    if(color==m_color)return false;
    m_color=color;LogisticsSelection selection;selection.kind=FreightViewKind::Market;
    selection.market=m_market;selection.explicitMarket=true;selection.restrictGoods=true;
    selection.allowedGoods=m_goods;selection.colorMode=m_color;
    session.SetSelection(kOwnerNativePanel,kOwnerNativePanel,selection);
    if(m_hover>=0){selection.hoverGood=m_hover;session.SetSelection(kOwnerHover,kOwnerHover,std::move(selection));}
    return true;
  }
  // Each row has its own key, so a late leave from A cannot erase an enter on B.
  // Context changes reject entries retained after panel destruction/reopening.
  bool UpdateHover(LogisticsSession& session,const GuiVariableProbe& variables,
                   const std::unordered_map<std::string,uint32_t>& goodsByKey) {
    int64_t good=-1;
    const auto status=session.Status();
    if(m_active && variables.ok && status.snapshotReady && !status.synthetic && status.world==m_world){
      static const std::string prefix="ironroad_market_hover_";
      for(const auto& entry:variables.entries){
        if(!entry.key.starts_with(prefix) || entry.value!=m_context)continue;
        const auto found=goodsByKey.find(entry.key.substr(prefix.size()));
        if(found==goodsByKey.end() || !std::binary_search(m_goods.begin(),m_goods.end(),found->second))continue;
        if(good>=0){good=-1;break;} // ambiguous rows never guess the first
        good=found->second;
      }
    }
    if(good==m_hover)return false;
    if(good<0)session.Close(kOwnerHover);
    else {
      LogisticsSelection selection;selection.kind=FreightViewKind::Market;
      selection.market=m_market;selection.explicitMarket=true;selection.hoverGood=good;
      selection.colorMode=m_color;
      selection.restrictGoods=true;selection.allowedGoods=m_goods;
      session.SetSelection(kOwnerHover,kOwnerHover,std::move(selection));
    }
    m_hover=good;return true;
  }
  const std::string& Context() const{return m_context;}
  void Close(LogisticsSession& session){
    if(m_hover>=0)session.Close(kOwnerHover);
    if(m_active)session.Close(kOwnerNativePanel);
    Reset();
  }
  void Reset(){m_active=false;m_market=0;m_world=0;m_goods.clear();m_hover=-1;m_context="ironroad-hover-inactive";m_color=FreightColorMode::Business;}
 private:
  bool m_active=false;
  uint32_t m_market=0,m_panelMarket=UINT32_MAX;
  uint64_t m_world=0,m_epoch=0;
  int64_t m_hover=-1;
  std::string m_context="ironroad-hover-inactive";
  std::vector<uint32_t> m_goods;
  FreightColorMode m_color=FreightColorMode::Business;
};
} // namespace v3
