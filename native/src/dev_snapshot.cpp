#include "dev_snapshot.h"
#include <algorithm>

namespace v3 {
EconomySnapshot BuildSyntheticSnapshot(const RouteGeography& geography, size_t maxStates) {
  EconomySnapshot snapshot;
  snapshot.world = 1;
  snapshot.revision = 1;
  snapshot.goods = {{1, "grain", "staple", 1, false, "Grain"},
                    {2, "iron", "industrial", 1, false, "Iron"},
                    {3, "wood", "industrial", 1, false, "Wood"}};
  if (!geography.splines.Valid()) return snapshot;
  std::vector<const StateEntry*> states;
  states.reserve(geography.states.size());
  for (const auto& state : geography.states) {
    if (state.id <= 0 || uint32_t(state.id) > UINT32_MAX / 100) continue;
    // The freight model routes from an authored hub of the region.
    if (!geography.splines.byId.count(uint32_t(state.id) * 100)) continue;
    states.push_back(&state);
  }
  std::sort(states.begin(), states.end(),
            [](const StateEntry* a, const StateEntry* b) { return a->id < b->id; });
  if (states.size() > maxStates) states.resize(maxStates);

  // Three goods; each good's states are split into paired producers/consumers
  // so total supply and demand balance and every good can be fully allocated
  // when the transport network allows it. Eight amount tiers spread the fixed
  // width thresholds so the debug view exercises every native width variant.
  static constexpr int64_t kAmounts[] = {4000,    25000,   80000,    300000,
                                         800000,  3000000, 8000000,  25000000};
  constexpr size_t kTiers = sizeof(kAmounts) / sizeof(kAmounts[0]);
  std::vector<size_t> groups[3];
  for (size_t i = 0; i < states.size(); ++i) groups[i % 3].push_back(i);
  const auto addOrder = [&](size_t index, uint32_t good, bool production, int64_t amount) {
    const uint32_t region = uint32_t(states[index]->id);
    StateOrders orders;
    orders.state = region;  // synthetic one-instance-per-region identity
    orders.region = region;
    orders.market = 1;
    orders.good = good;
    if (production) orders.production = amount;
    else orders.consumption = amount;
    snapshot.orders.push_back(orders);
  };
  for (size_t good = 0; good < 3; ++good) {
    auto& indices = groups[good];
    const size_t half = indices.size() / 2;
    for (size_t pair = 0; pair < half; ++pair) {
      const int64_t amount = kAmounts[pair % kTiers];
      addOrder(indices[pair], uint32_t(good) + 1, true, amount);
      addOrder(indices[half + pair], uint32_t(good) + 1, false, amount);
    }
    // An odd leftover state contributes supply only; it cannot create demand.
    for (size_t i = 2 * half; i < indices.size(); ++i)
      addOrder(indices[i], uint32_t(good) + 1, true, kAmounts[i % kTiers]);
  }
  return snapshot;
}
}  // namespace v3
