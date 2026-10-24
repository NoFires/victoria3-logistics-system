#include "logistics_session.h"
#include <algorithm>

namespace v3 {
namespace {
bool SameFilter(const FreightFilter& a, const FreightFilter& b, bool includeHover = true) {
  return a.kind == b.kind && a.market == b.market && a.state == b.state &&
      a.search == b.search && a.category == b.category && (!includeHover || a.hoverGood == b.hoverGood) &&
      a.restrictCategories == b.restrictCategories && a.categories == b.categories &&
      a.restrictGoods == b.restrictGoods && a.allowedGoods == b.allowedGoods;
}
}  // namespace

LogisticsSession::LogisticsSession(GeographyHandle geography, FreightPolicy policy)
    : m_geography(std::move(geography)),
      m_policy(std::move(policy)),
      m_worker(m_geography, m_policy) {}

uint32_t LogisticsSession::EffectiveOwnerLocked() const {
  uint32_t best = 0, bestPriority = 0;
  uint64_t bestSerial = 0;
  for (const auto& [owner, state] : m_owners) {
    if (!state.active) continue;
    if (!best || state.priority > bestPriority ||
        (state.priority == bestPriority && state.serial > bestSerial)) {
      best = owner;
      bestPriority = state.priority;
      bestSerial = state.serial;
    }
  }
  return best;
}

bool LogisticsSession::SetSnapshot(std::shared_ptr<const EconomySnapshot> snapshot,
                                   std::string* reason, bool synthetic) {
  if (!snapshot) {
    if (reason) *reason = "missing economy snapshot";
    return false;
  }
  LockGuard lock(m_mutex);
  if (m_haveIdentity) {
    if (snapshot->world < m_lastWorld || (m_worldRetired && snapshot->world == m_lastWorld)) {
      if (reason) *reason = "stale world generation";
      return false;
    }
    if (snapshot->world == m_lastWorld && snapshot->revision < m_lastRevision) {
      if (reason) *reason = "stale economy revision";
      return false;
    }
  }
  m_snapshot = std::move(snapshot);
  m_lastWorld = m_snapshot->world;
  m_lastRevision = m_snapshot->revision;
  m_haveIdentity = true;
  m_worldRetired = false;
  m_worker.Cancel();
  m_frame.reset();
  m_status.haveFrame = m_status.active = false;
  m_status.ribbons = 0;
  m_status.applied = 0;
  m_synthetic = synthetic;
  m_snapshotDirty = true;
  m_status.snapshotReady = true;
  m_status.synthetic = synthetic;
  m_status.world = m_snapshot->world;
  m_status.revision = m_snapshot->revision;
  return true;
}

bool LogisticsSession::HasEquivalentSnapshot(const EconomySnapshot& snapshot) const {
  LockGuard lock(m_mutex);
  // Compare values, not a hash: revision counts captures, whereas the owned
  // snapshot identity changes only when real inputs change. Vector ordering is
  // the adapter's deterministic state/good order; rail insertion order is irrelevant.
  return m_snapshot && !m_worldRetired && !m_synthetic &&
      snapshot.world==m_snapshot->world && snapshot.revision>=m_lastRevision &&
      snapshot.primaryMarket==m_snapshot->primaryMarket && snapshot.goods==m_snapshot->goods &&
      snapshot.orders==m_snapshot->orders && snapshot.builtRailEdges==m_snapshot->builtRailEdges;
}

bool LogisticsSession::StatisticsAreCurrent(const std::shared_ptr<const FreightFrame>& frame) const {
  LockGuard lock(m_mutex);
  if (!frame || !m_frame || !frame->allocation || frame->allocation!=m_frame->allocation ||
      !m_snapshot || m_snapshotDirty || m_synthetic ||
      frame->world!=m_snapshot->world || frame->economyRevision!=m_snapshot->revision) return false;
  const auto owner=m_owners.find(m_effectiveOwner);
  if (owner==m_owners.end() || !owner->second.active) return false;
  auto selection=owner->second.selection;
  if (!selection.market && !selection.explicitMarket) selection.market=FirstMarket(*m_snapshot);
  return SameFilter(frame->filter,selection.Filter(),false);
}

void LogisticsSession::ResetWorld() {
  LockGuard lock(m_mutex);
  for (auto& [owner, state] : m_owners) state.active = false;
  m_effectiveOwner = 0;
  m_snapshot.reset();
  m_worldRetired = m_haveIdentity;
  m_frame.reset();
  m_snapshotDirty = m_selectionDirty = false;
  m_synthetic = false;
  m_status = LogisticsStatus{};
  m_worker.Reset();
}

void LogisticsSession::SetSelection(uint32_t owner, uint32_t priority,
                                    LogisticsSelection selection) {
  LockGuard lock(m_mutex);
  auto& state = m_owners[owner];
  state.priority = priority;
  state.active = true;
  state.selection = std::move(selection);
  state.serial = ++m_serial;
  const uint32_t effective = EffectiveOwnerLocked();
  if (effective != m_effectiveOwner) {
    m_effectiveOwner = effective;
    m_selectionDirty = true;
  } else if (effective == owner) {
    m_selectionDirty = true;
  }
}

void LogisticsSession::Close(uint32_t owner) {
  LockGuard lock(m_mutex);
  auto it = m_owners.find(owner);
  if (it == m_owners.end() || !it->second.active) return;
  it->second.active = false;
  it->second.serial = ++m_serial;
  const uint32_t effective = EffectiveOwnerLocked();
  if (effective == 0) {
    m_effectiveOwner = 0;
    m_selectionDirty = false;
    m_frame.reset();
    m_worker.Cancel();
    m_status.haveFrame = false;
    m_status.active = false;
    m_status.ribbons = 0;
    m_status.applied = 0;
    m_status.allocationReused = m_status.viewReused = false;
  } else if (effective != m_effectiveOwner) {
    m_effectiveOwner = effective;
    m_selectionDirty = true;
  }
}

void LogisticsSession::CloseAll() {
  LockGuard lock(m_mutex);
  for (auto& [owner, state] : m_owners) state.active = false;
  m_effectiveOwner = 0;
  m_selectionDirty = false;
  m_frame.reset();
  m_worker.Cancel();
  m_status.haveFrame = false;
  m_status.active = false;
  m_status.ribbons = 0;
  m_status.applied = 0;
  m_status.allocationReused = m_status.viewReused = false;
}

uint32_t LogisticsSession::FirstMarket(const EconomySnapshot& snapshot) {
  if (snapshot.primaryMarket) return snapshot.primaryMarket;
  uint32_t first = 0;
  bool found = false;
  for (const auto& orders : snapshot.orders) {
    if (!found || orders.market < first) {
      first = orders.market;
      found = true;
    }
  }
  return first;
}

LogisticsStatus LogisticsSession::Status() const {
  LockGuard lock(m_mutex);
  return m_status;
}

std::shared_ptr<const FreightFrame> LogisticsSession::Frame() const {
  LockGuard lock(m_mutex);
  return m_frame;
}

bool LogisticsSession::SnapshotIsCurrent(const std::shared_ptr<const EconomySnapshot>& snapshot) const {
  LockGuard lock(m_mutex);
  return snapshot && snapshot==m_snapshot;
}

bool LogisticsSession::FrameIsCurrent(const std::shared_ptr<const FreightFrame>& frame,
                                      uint64_t applied) const {
  LockGuard lock(m_mutex);
  return frame && frame == m_frame && m_snapshot && m_effectiveOwner &&
      !m_snapshotDirty && !m_selectionDirty && m_status.active && m_status.haveFrame &&
      applied && applied == m_status.applied && applied == m_worker.Generation() &&
      frame->world == m_snapshot->world && frame->economyRevision == m_snapshot->revision;
}

std::shared_ptr<const LogisticsDocument> LogisticsSession::Poll() {
  FreightJob job;
  {
    LockGuard lock(m_mutex);
    if (m_effectiveOwner && m_snapshot && (m_snapshotDirty || m_selectionDirty)) {
      auto& owner = m_owners[m_effectiveOwner];
      if (owner.selection.market == 0 && !owner.selection.explicitMarket)
        owner.selection.market = FirstMarket(*m_snapshot);
      m_selection = owner.selection;
      m_filter = m_selection.Filter();
      job.updateSnapshot = m_snapshotDirty;
      job.snapshot = m_snapshotDirty ? m_snapshot : nullptr;
      job.filter = m_filter;
      job.owner = m_effectiveOwner;
      m_snapshotDirty = m_selectionDirty = false;
      // Serialize mailbox submission with ResetWorld/SetSnapshot. Otherwise a
      // previously captured old job could be enqueued after reset cancelled it.
      m_status.generation = m_worker.Request(std::move(job));
    }
  }
  auto reply = m_worker.Poll();
  if (!reply || reply->generation != m_worker.Generation()) return {};
  std::shared_ptr<const EconomySnapshot> snapshot;
  FreightFilter filter;
  uint32_t effective = 0;
  bool synthetic = false;
  {
    LockGuard lock(m_mutex);
    // LatestWorker::Poll is a retained mailbox, not a consuming read. A stable
    // generation must not rebuild/re-publish its document every UI timer tick.
    if (reply->generation == m_lastApplied) return {};
    snapshot = m_snapshot;
    filter = m_filter;
    effective = m_effectiveOwner;
    synthetic = m_synthetic;
  }
  if (!snapshot || !effective) return {};
  if (!reply->error.empty() || !reply->value.error.empty()) {
    LockGuard lock(m_mutex);
    if (m_snapshot != snapshot || m_effectiveOwner != effective ||
        reply->generation != m_worker.Generation()) return {};
    m_status.error = reply->error.empty() ? reply->value.error : reply->error;
    m_status.allocationBuilds = reply->value.stats.allocations;
    m_status.allocationHits = reply->value.stats.allocationHits;
    m_status.viewBuilds = reply->value.stats.views;
    m_status.viewHits = reply->value.stats.viewHits;
    return {};
  }
  auto frame = reply->value.frame;
  if (!frame || !frame->allocation || !frame->view) return {};
  // Discard a generation produced for a superseded snapshot, filter or owner.
  if (reply->value.owner != effective || frame->world != snapshot->world ||
      frame->economyRevision != snapshot->revision || !SameFilter(frame->filter, filter))
    return {};
  auto document = std::make_shared<LogisticsDocument>(
      BuildLogisticsDocument(*snapshot, *frame->allocation, *frame->view, frame->filter));
  document->synthetic = synthetic;
  {
    LockGuard lock(m_mutex);
    if (m_effectiveOwner != effective || m_snapshot != snapshot || m_snapshotDirty ||
        m_selectionDirty || reply->generation != m_worker.Generation() ||
        reply->generation == m_lastApplied || !SameFilter(m_filter, filter)) return {};
    m_frame = std::move(frame);
    m_status.snapshotReady = true;
    m_status.synthetic = synthetic;
    m_status.haveFrame = true;
    m_status.active = true;
    m_status.owner = effective;
    m_status.applied = reply->generation;
    m_status.world = m_frame->world;
    m_status.revision = m_frame->economyRevision;
    m_status.ribbons = m_frame->ribbons.size();
    m_status.allocationReused = m_frame->allocationReused;
    m_status.viewReused = m_frame->viewReused;
    m_status.allocationBuilds = reply->value.stats.allocations;
    m_status.allocationHits = reply->value.stats.allocationHits;
    m_status.viewBuilds = reply->value.stats.views;
    m_status.viewHits = reply->value.stats.viewHits;
    m_status.computeMs = m_frame->computeMs;
    m_status.error.clear();
    m_lastApplied = reply->generation;
  }
  return document;
}
}  // namespace v3
