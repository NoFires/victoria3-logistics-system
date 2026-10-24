#include "freight_engine.h"
#include "log.h"
#include <algorithm>
#include <chrono>
#include <cstring>
#include <tuple>

namespace v3 {
namespace {
uint64_t HashBytes(uint64_t hash, const void* data, size_t length) {
  const auto* bytes = static_cast<const uint8_t*>(data);
  for (size_t i = 0; i < length; ++i) {
    hash ^= bytes[i];
    hash *= 1099511628211ull;
  }
  return hash;
}
template <class T> uint64_t HashValue(uint64_t hash, const T& value) {
  return HashBytes(hash, &value, sizeof(T));
}
bool SameFilter(const FreightFilter& a, const FreightFilter& b) {
  return a.kind == b.kind && a.market == b.market && a.state == b.state &&
      a.search == b.search && a.category == b.category && a.hoverGood == b.hoverGood &&
      a.restrictCategories == b.restrictCategories && a.categories == b.categories &&
      a.restrictGoods == b.restrictGoods && a.allowedGoods == b.allowedGoods;
}
// Content identity for the caches. Order-independent and independent of the
// adapter's iteration order; a same-revision snapshot whose numbers changed is
// therefore a different snapshot, not a cache hit.
uint64_t Fingerprint(const EconomySnapshot& snapshot) {
  uint64_t hash = 14695981039346656037ull;
  std::vector<const FreightGood*> goods;
  goods.reserve(snapshot.goods.size());
  for (const auto& good : snapshot.goods) goods.push_back(&good);
  std::sort(goods.begin(), goods.end(),
            [](const FreightGood* a, const FreightGood* b) { return a->id < b->id; });
  hash = HashValue(hash, goods.size());
  for (const auto* good : goods) {
    hash = HashValue(hash, good->id);
    hash = HashBytes(hash, good->key.data(), good->key.size());
    hash = HashBytes(hash, good->category.data(), good->category.size());
    hash = HashValue(hash, good->weight);
    hash = HashValue(hash, good->localOnly);
    hash = HashValue(hash, good->tradeable);
    hash = HashValue(hash, good->freightPerOrder);
    hash = HashValue(hash, good->freightIntermediary);
  }
  std::vector<StateOrders> orders = snapshot.orders;
  std::sort(orders.begin(), orders.end(), [](const StateOrders& a, const StateOrders& b) {
    return std::tie(a.state, a.good) < std::tie(b.state, b.good);
  });
  hash = HashValue(hash, orders.size());
  for (const auto& order : orders) {
    hash = HashValue(hash, order.state);
    hash = HashValue(hash, order.region);
    hash = HashValue(hash, order.market);
    hash = HashValue(hash, order.good);
    hash = HashValue(hash, order.production);
    hash = HashValue(hash, order.consumption);
  }
  std::vector<uint32_t> rail(snapshot.builtRailEdges.begin(), snapshot.builtRailEdges.end());
  std::sort(rail.begin(), rail.end());
  hash = HashValue(hash, rail.size());
  for (uint32_t edge : rail) hash = HashValue(hash, edge);
  return hash;
}
}  // namespace

FreightEngine::FreightEngine(GeographyHandle geography, FreightPolicy policy)
    : m_geography(std::move(geography)), m_policy(std::move(policy)) {}

bool FreightEngine::SetSnapshot(EconomySnapshot snapshot, std::string* reason) {
  if (m_haveIdentity) {
    if (snapshot.world < m_world || (m_worldRetired && snapshot.world == m_world)) {
      if (reason) *reason = "stale world generation";
      return false;
    }
    if (snapshot.world == m_world && snapshot.revision < m_revision) {
      if (reason) *reason = "stale economy revision";
      return false;
    }
  }
  const uint64_t fingerprint = Fingerprint(snapshot);
  const bool crossWorld = m_snapshot && snapshot.world != m_world;
  const bool changed = !m_snapshot || snapshot.revision != m_revision || fingerprint != m_fingerprint;
  if (crossWorld || changed) {
    // Cached allocations/views belong to one economy revision. They are never
    // reused across worlds or after the numbers changed.
    m_allocations.clear();
    m_views.clear();
  }
  m_snapshot = std::make_shared<const EconomySnapshot>(std::move(snapshot));
  m_world = m_snapshot->world;
  m_revision = m_snapshot->revision;
  m_haveIdentity = true;
  m_worldRetired = false;
  m_fingerprint = fingerprint;
  return true;
}

void FreightEngine::ResetWorld() {
  m_snapshot.reset();
  m_allocations.clear();
  m_views.clear();
  ++m_epoch;
  m_worldRetired = m_haveIdentity;
  // m_world/m_revision keep the last accepted identities so a late adapter
  // result from the old world is still rejected by SetSnapshot.
}

FreightEngine::AllocationEntry& FreightEngine::TouchAllocation(const AllocationEntry& key) {
  const auto same = [&](const AllocationEntry& entry) {
    return entry.world == key.world && entry.revision == key.revision &&
        entry.fingerprint == key.fingerprint && entry.epoch == key.epoch;
  };
  const auto found = std::find_if(m_allocations.begin(), m_allocations.end(), same);
  if (found != m_allocations.end()) {
    if (found != m_allocations.begin()) {
      AllocationEntry moved = std::move(*found);
      m_allocations.erase(found);
      m_allocations.insert(m_allocations.begin(), std::move(moved));
    }
    return m_allocations.front();
  }
  m_allocations.insert(m_allocations.begin(), key);
  if (m_allocations.size() > kAllocationCapacity) m_allocations.pop_back();
  return m_allocations.front();
}

FreightEngine::ViewEntry& FreightEngine::TouchView(const ViewEntry& key) {
  const auto same = [&](const ViewEntry& entry) {
    return entry.world == key.world && entry.revision == key.revision &&
        entry.fingerprint == key.fingerprint && entry.epoch == key.epoch &&
        entry.graphRevision == key.graphRevision && SameFilter(entry.filter, key.filter) &&
        entry.coordScale == key.coordScale && entry.heightScale == key.heightScale &&
        entry.heightOffset == key.heightOffset;
  };
  const auto found = std::find_if(m_views.begin(), m_views.end(), same);
  if (found != m_views.end()) {
    if (found != m_views.begin()) {
      ViewEntry moved = std::move(*found);
      m_views.erase(found);
      m_views.insert(m_views.begin(), std::move(moved));
    }
    return m_views.front();
  }
  m_views.insert(m_views.begin(), key);
  if (m_views.size() > kViewCapacity) m_views.pop_back();
  return m_views.front();
}

std::shared_ptr<const FreightFrame> FreightEngine::Build(const FreightFilter& inputFilter,
    float coordScale, float heightScale, float heightOffset,
    const std::function<bool()>& cancelled) {
  const auto started = std::chrono::steady_clock::now();
  FreightFilter filter=inputFilter;
  if(filter.restrictGoods) {
    std::sort(filter.allowedGoods.begin(),filter.allowedGoods.end());
    filter.allowedGoods.erase(std::unique(filter.allowedGoods.begin(),filter.allowedGoods.end()),filter.allowedGoods.end());
  } else filter.allowedGoods.clear();
  double allocationMs = 0;
  double viewMs = 0;
  auto frame = std::make_shared<FreightFrame>();
  frame->world = m_world;
  frame->economyRevision = m_revision;
  frame->graphRevision = m_policy.graphRevision;
  frame->epoch = m_epoch;
  frame->filter = filter;
  const auto finish = [&]() {
    frame->computeMs = std::chrono::duration<double, std::milli>(
        std::chrono::steady_clock::now() - started).count();
    V3_INFO("freight build allocation=%.2f ms view=%.2f ms total=%.2f ms (allocReuse=%d viewReuse=%d)",
            allocationMs, viewMs, frame->computeMs, int(frame->allocationReused),
            int(frame->viewReused));
    return frame;
  };
  if (!m_snapshot) {
    frame->error = "no economy snapshot";
    return finish();
  }
  if (!m_geography || !m_geography->splines.Valid()) {
    frame->error = "transport graph unavailable";
    return finish();
  }
  if (cancelled && cancelled()) {
    frame->error = "cancelled";
    return finish();
  }
  AllocationEntry allocationKey;
  allocationKey.world = m_world;
  allocationKey.revision = m_revision;
  allocationKey.fingerprint = m_fingerprint;
  allocationKey.epoch = m_epoch;
  auto& allocation = TouchAllocation(allocationKey);
  if (allocation.value) {
    frame->allocationReused = true;
    ++m_stats.allocationHits;
  } else {
    auto result = AllocateFreight(*m_snapshot, m_geography->splines, m_policy, cancelled);
    if (!result.complete) {
      frame->error = result.cancelled ? "cancelled" : result.error;
      return finish();
    }
    allocation.value = std::make_shared<const FreightResult>(std::move(result));
    ++m_stats.allocations;
  }
  frame->allocation = allocation.value;
  allocationMs = std::chrono::duration<double, std::milli>(
      std::chrono::steady_clock::now() - started).count();
  if (cancelled && cancelled()) {
    frame->error = "cancelled";
    return finish();
  }
  ViewEntry viewKey;
  viewKey.world = m_world;
  viewKey.revision = m_revision;
  viewKey.fingerprint = m_fingerprint;
  viewKey.epoch = m_epoch;
  viewKey.graphRevision = m_policy.graphRevision;
  viewKey.filter = filter;
  viewKey.coordScale = coordScale;
  viewKey.heightScale = heightScale;
  viewKey.heightOffset = heightOffset;
  auto& cached = TouchView(viewKey);
  if (cached.view) {
    frame->viewReused = true;
    ++m_stats.viewHits;
  } else {
    auto view = BuildFreightView(*m_snapshot, *frame->allocation, filter);
    cached.view = std::make_shared<const FreightView>(std::move(view));
    cached.ribbons = BuildFreightRibbons(*cached.view, m_geography->splines, m_geography->heights,
                                         coordScale, heightScale, heightOffset);
    ++m_stats.views;
  }
  if (cancelled && cancelled()) {
    frame->error = "cancelled";
    return finish();
  }
  frame->view = cached.view;
  frame->ribbons = cached.ribbons;
  viewMs = std::chrono::duration<double, std::milli>(
      std::chrono::steady_clock::now() - started).count() - allocationMs;
  return finish();
}
}  // namespace v3
