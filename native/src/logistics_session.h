#pragma once
#include "freight_worker.h"
#include "logistics_ui.h"
#include "mutex.h"
#include <map>
#include <memory>

namespace v3 {
// Owner identities double as their priorities; the higher active owner provides
// the map view. A transient hover overrides the base panel and restoring is
// implicit: closing the hover simply re-exposes the next active owner.
inline constexpr uint32_t kOwnerNativePanel = 10;
inline constexpr uint32_t kOwnerNativeState = 15;
inline constexpr uint32_t kOwnerHover = 20;
inline constexpr uint32_t kOwnerDebugPreview = 30;
inline bool IsNativeLogisticsOwner(uint32_t owner) {
  return owner == kOwnerNativePanel || owner == kOwnerNativeState || owner == kOwnerHover;
}

// Observable state for the debug panel and the future native UI bridge. Never
// contains engine pointers; it describes only copied data and cache activity.
struct LogisticsStatus {
  bool snapshotReady = false;
  bool synthetic = false;
  bool haveFrame = false;
  bool active = false;
  uint64_t world = 0, revision = 0, generation = 0, applied = 0;
  uint32_t owner = 0;
  size_t ribbons = 0;
  bool allocationReused = false, viewReused = false;
  uint64_t allocationBuilds = 0, allocationHits = 0, viewBuilds = 0, viewHits = 0;
  double computeMs = 0;
  std::string error;
};

// Connects UI selections to the copied freight pipeline without doing engine
// work on the calling thread. Several owners (native panel, transient hover,
// debug preview) share one snapshot and one worker; only the highest active
// owner builds a view, so one owner closing cannot clear another owner's map.
class LogisticsSession {
 public:
  LogisticsSession(GeographyHandle geography, FreightPolicy policy = {});
  // Replaces the copied snapshot. Stale worlds/revisions are rejected before
  // any work is queued. `synthetic` marks debug-only test input; it can never be
  // presented as a live game reading.
  bool SetSnapshot(std::shared_ptr<const EconomySnapshot> snapshot,
                   std::string* reason = nullptr, bool synthetic = false);
  // A timer capture with identical owned inputs retains the current snapshot,
  // revision and worker request. Failed/retired/synthetic inputs never match.
  bool HasEquivalentSnapshot(const EconomySnapshot& snapshot) const;
  void ResetWorld();
  // Cheap UI-thread update. `priority` selects which active owner owns the map;
  // the worker coalesces rapid hover changes.
  void SetSelection(uint32_t owner, uint32_t priority, LogisticsSelection selection);
  // Owner hidden/closed: it stops contributing immediately. The snapshot stays
  // available so reopening the same view can reuse cached geometry.
  void Close(uint32_t owner);
  void CloseAll();
  // Non-blocking. Submits pending work, consumes one completed generation and
  // returns the copied document when new data arrived.
  std::shared_ptr<const LogisticsDocument> Poll();
  LogisticsStatus Status() const;
  std::shared_ptr<const FreightFrame> Frame() const;
  // Numeric-copy identity also rejects replaced snapshots with equal revisions.
  bool SnapshotIsCurrent(const std::shared_ptr<const EconomySnapshot>& snapshot) const;
  // Render-side publication check, including same-version snapshot replacement
  // and cancellation. Uses only our mailbox lock and immutable numeric data.
  bool FrameIsCurrent(const std::shared_ptr<const FreightFrame>& frame, uint64_t applied) const;
  // Hover changes only map contributions. Statistics for the same snapshot and
  // base filter remain valid while the hover view is being computed.
  bool StatisticsAreCurrent(const std::shared_ptr<const FreightFrame>& frame) const;
 private:
  struct OwnerState {
    uint32_t priority = 0;
    uint64_t serial = 0;
    bool active = false;
    LogisticsSelection selection;
  };
  static uint32_t FirstMarket(const EconomySnapshot& snapshot);
  uint32_t EffectiveOwnerLocked() const;
  GeographyHandle m_geography;
  FreightPolicy m_policy;
  FreightWorker m_worker;
  mutable Mutex m_mutex;
  std::map<uint32_t, OwnerState> m_owners;
  uint64_t m_serial = 0;
  uint32_t m_effectiveOwner = 0;
  std::shared_ptr<const EconomySnapshot> m_snapshot;
  // Validation survives m_snapshot.reset(); a retired world cannot re-enter
  // through a late adapter result while the next world is still loading.
  uint64_t m_lastWorld = 0, m_lastRevision = 0;
  bool m_haveIdentity = false, m_worldRetired = false;
  LogisticsSelection m_selection;
  FreightFilter m_filter;
  bool m_snapshotDirty = false, m_selectionDirty = false;
  bool m_synthetic = false;
  uint64_t m_lastApplied = 0;
  std::shared_ptr<const FreightFrame> m_frame;
  LogisticsStatus m_status;
};
}  // namespace v3
