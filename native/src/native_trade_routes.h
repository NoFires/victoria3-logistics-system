#pragma once
#include "native_world_trade.h"
#include <map>
#include <set>
#include <tuple>

namespace v3 {
struct NativeTradeBinding {
  size_t pair=0;
  uint32_t sourceCenter=UINT32_MAX,destinationCenter=UINT32_MAX;
  std::optional<size_t> sourceGroup;
  // Empty for same-group trade or absent native destination. A retained route
  // may itself be known empty or unselected: neither permits a model fallback.
  std::optional<size_t> route;
};
// Original01221F180 uses each state's current CMarketArea+48 and013C4E70
// looks those centers up in the native cache. Aggregate detailed890 exports by
// current destination center and require exact agreement with independent8B8.
// Old group membership never substitutes for a missing current route. This
// establishes quantity/path association, not port endpoints or travel order.
inline std::optional<std::vector<NativeTradeBinding>> BindNativeTradeRoutes(
    const NativeWorldTradeSnapshot& trade,std::string* error=nullptr) {
  auto fail=[&](const char* why)->std::optional<std::vector<NativeTradeBinding>>{if(error)*error=why;return {};};
  std::set<uint32_t> states;
  for(const auto& state:trade.states)if(!states.insert(state.id).second)return fail("duplicate native trade state");
  std::map<uint32_t,size_t> members,anchors;
  for(size_t i=0;i<trade.groups.size();++i){
    const auto& group=trade.groups[i];
    if(!states.count(group.anchor) || !anchors.emplace(group.anchor,i).second)return fail("invalid native trade representative");
    for(uint32_t state:group.states)if(!states.count(state) || !members.emplace(state,i).second)
      return fail("invalid native trade membership");
  }
  std::vector<std::map<uint32_t,size_t>> destinations(trade.groups.size());
  for(size_t i=0;i<trade.groups.size();++i)for(size_t j=0;j<trade.groups[i].destinations.size();++j){
    const auto& route=trade.groups[i].destinations[j];
    if(!anchors.count(route.state) || route.state==trade.groups[i].anchor ||
       !destinations[i].emplace(route.state,j).second ||
       (route.selected && *route.selected>=route.candidates.size()))return fail("invalid native trade destination");
  }
  using Key=std::tuple<uint32_t,uint32_t,uint32_t>; // source state, destination representative, good
  std::map<Key,int64_t> detailed,grouped;
  auto add=[](auto& totals,Key key,int64_t quantity){
    auto& total=totals[key];if(quantity>INT64_MAX-total)return false;total+=quantity;return true;
  };
  std::vector<NativeTradeBinding> result;
  for(unsigned aggregate=0;aggregate<2;++aggregate){
    const auto& pairs=aggregate?trade.groupPairs:trade.statePairs;
    std::set<std::pair<uint32_t,uint32_t>> seen;
    for(size_t i=0;i<pairs.size();++i){
      const auto& pair=pairs[i];
      if(!states.count(pair.source) || !states.count(pair.destination) ||
         !seen.emplace(pair.source,pair.destination).second)return fail("invalid native trade peer identity");
      std::set<uint32_t> goods;
      for(const auto& amount:pair.goods)if(amount.valueFixed<=0 || !goods.insert(amount.good).second)
        return fail("invalid native trade amount");
      if(pair.goods.empty())continue;
      const auto source=trade.centers.find(pair.source),target=trade.centers.find(pair.destination);
      if(source==trade.centers.end() || source->second==UINT32_MAX || !states.count(source->second) ||
         (!aggregate && (target==trade.centers.end() || target->second==UINT32_MAX || !states.count(target->second))))
        return fail("native export current trade center unavailable");
      const uint32_t destination=aggregate?pair.destination:target->second;
      for(const auto& amount:pair.goods)if(!add(aggregate?grouped:detailed,{pair.source,destination,amount.good},amount.valueFixed))
        return fail("native grouped quantity overflow");
      if(!aggregate){
        NativeTradeBinding binding;binding.pair=i;binding.sourceCenter=source->second;binding.destinationCenter=destination;
        const auto group=anchors.find(source->second);
        if(group!=anchors.end()){
          binding.sourceGroup=group->second;
          const auto route=destinations[group->second].find(destination);
          if(route!=destinations[group->second].end())binding.route=route->second;
        }
        result.push_back(binding);
      }
    }
  }
  if(detailed!=grouped)return fail("native detailed and grouped exports differ");
  if(error)error->clear();return result;
}
} // namespace v3
