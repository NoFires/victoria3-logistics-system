#pragma once
#include "freight.h"
#include "freight_ribbons.h"
#include "route_geography.h"
#include <functional>
#include <memory>
#include <string>

namespace v3 {
// Immutable result of one market/state view. Produced on a worker, consumed by
// the native-UI bridge and the render update. Holds no engine pointer and no
// mutable native object; the renderer only ever sees `ribbons`.
struct FreightFrame {
  uint64_t world = 0, economyRevision = 0, graphRevision = 0, epoch = 0;
  FreightFilter filter;
  std::shared_ptr<const FreightResult> allocation;
  std::shared_ptr<const FreightView> view;
  RibbonBatch ribbons;
  bool allocationReused = false, viewReused = false;
  double computeMs = 0;
  std::string error;
};

struct FreightEngineStats {
  uint64_t allocations = 0, allocationHits = 0, views = 0, viewHits = 0;
};

// Two cache layers above the native RibbonCache: allocation results and
// view/ribbon aggregates. A cached Dijkstra or allocation never implies cached
// native geometry; that remains the renderer's responsibility.
//
// Single-threaded by contract: use it from the owning worker (or from a test).
// `Build` never mutates the economy snapshot and never calls the game.
class FreightEngine {
 public:
  explicit FreightEngine(GeographyHandle geography, FreightPolicy policy = {});
  // A late adapter result may not replace newer data. Rejects a lower world or
  // a lower economy revision in the same world. Same-revision snapshots whose
  // content changed are accepted and invalidate allocations; identical content
  // keeps the caches warm.
  bool SetSnapshot(EconomySnapshot snapshot, std::string* reason = nullptr);
  bool HasSnapshot() const { return m_snapshot != nullptr; }
  uint64_t World() const { return m_world; }
  uint64_t EconomyRevision() const { return m_revision; }
  // World change: retire the accepted world, drop both caches and advance epoch.
  // The next snapshot must have a strictly newer world (not just a new revision).
  void ResetWorld();
  uint64_t Epoch() const { return m_epoch; }
  std::shared_ptr<const FreightFrame> Build(const FreightFilter& filter,
      float coordScale = 1, float heightScale = 0, float heightOffset = 0,
      const std::function<bool()>& cancelled = {});
  const FreightEngineStats& Stats() const { return m_stats; }
  size_t AllocationCacheSize() const { return m_allocations.size(); }
  size_t ViewCacheSize() const { return m_views.size(); }
  uint64_t SnapshotFingerprint() const { return m_fingerprint; }
  static constexpr size_t kAllocationCapacity = 4;
  static constexpr size_t kViewCapacity = 32;
 private:
  struct AllocationEntry {
    uint64_t world = 0, revision = 0, fingerprint = 0, epoch = 0;
    std::shared_ptr<const FreightResult> value;
  };
  struct ViewEntry {
    uint64_t world = 0, revision = 0, fingerprint = 0, epoch = 0, graphRevision = 0;
    FreightFilter filter;
    float coordScale = 1, heightScale = 0, heightOffset = 0;
    std::shared_ptr<const FreightView> view;
    RibbonBatch ribbons;
  };
  AllocationEntry& TouchAllocation(const AllocationEntry& key);
  ViewEntry& TouchView(const ViewEntry& key);
  GeographyHandle m_geography;
  FreightPolicy m_policy;
  std::shared_ptr<const EconomySnapshot> m_snapshot;
  uint64_t m_world = 0, m_revision = 0, m_fingerprint = 0, m_epoch = 0;
  bool m_haveIdentity = false, m_worldRetired = false;
  std::vector<AllocationEntry> m_allocations;  // most recently used first
  std::vector<ViewEntry> m_views;              // most recently used first
  FreightEngineStats m_stats;
};
}  // namespace v3
