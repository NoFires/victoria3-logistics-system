#pragma once
#include "game.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <memory>
#include <unordered_map>

namespace v3 {
// Immutable, already converted native cubic controls. Create these on a worker;
// hashing/validation and view ordering are independent of native GPU uploads.
struct RibbonGeometry {
  std::string style;
  std::vector<Vec3> points;
  uint64_t hash = 0;
};
using Ribbon = std::shared_ptr<const RibbonGeometry>;
using RibbonBatch = std::vector<Ribbon>;
inline bool SameRibbon(const RibbonGeometry& a, const RibbonGeometry& b) {
  return a.style == b.style && a.points.size() == b.points.size() &&
      std::memcmp(a.points.data(), b.points.data(), a.points.size() * sizeof(Vec3)) == 0;
}
inline Ribbon MakeRibbon(std::string style, std::vector<Vec3> points) {
  if (style.empty() || points.size() < 4 || points.size() > 4096) return {};
  bool distinct = false;
  for (auto& p : points) {
    if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z)) return {};
    // Canonicalise signed zero so equivalent geometry has the same identity.
    if (p.x == 0) p.x = 0; if (p.y == 0) p.y = 0; if (p.z == 0) p.z = 0;
    const auto& first = points.front();
    distinct |= p.x != first.x || p.y != first.y || p.z != first.z;
  }
  if (!distinct) return {};
  // Invalid endpoint controls would silently cut off part of the route.
  const auto equal = [](const Vec3& a, const Vec3& b) {
    return a.x == b.x && a.y == b.y && a.z == b.z;
  };
  if (!equal(points[0], points[1]) || !equal(points.back(), points[points.size()-2])) return {};
  uint64_t hash = 14695981039346656037ull;
  auto feed = [&](const void* data, size_t length) {
    const auto* bytes = static_cast<const uint8_t*>(data);
    for (size_t i=0; i<length; ++i) { hash ^= bytes[i]; hash *= 1099511628211ull; }
  };
  feed(style.data(), style.size()); feed(points.data(), points.size() * sizeof(Vec3));
  return std::make_shared<RibbonGeometry>(RibbonGeometry{std::move(style), std::move(points), hash});
}
inline RibbonBatch PrepareRibbonBatch(RibbonBatch batch) {
  batch.erase(std::remove(batch.begin(), batch.end(), Ribbon{}), batch.end());
  std::sort(batch.begin(), batch.end(), [](const Ribbon& a, const Ribbon& b) {
    if (a->hash != b->hash) return a->hash < b->hash;
    if (a->style != b->style) return a->style < b->style;
    if (a->points.size() != b->points.size()) return a->points.size() < b->points.size();
    return std::memcmp(a->points.data(), b->points.data(), a->points.size()*sizeof(Vec3)) < 0;
  });
  batch.erase(std::unique(batch.begin(), batch.end(), [](const Ribbon& a, const Ribbon& b) {
    return SameRibbon(*a, *b);
  }), batch.end());
  return batch;
}

// Render-thread policy only: no engine pointers, destructors or raw COM release.
// Backend slots retain constructor-time type, path context and game allocation
// for the process lifetime. A recycled slot MUST have the same native style.
class RibbonCache {
 public:
  struct Statistics {
    uint64_t attempts=0, hits=0, failures=0, dropped=0, repeats=0, overrunFrames=0;
    double seconds=0, peakFrameSeconds=0, peakStepSeconds=0;
    unsigned peakFrameAttempts=0;
  };
  explicit RibbonCache(size_t capacity = 8192) : m_capacity(std::min<size_t>(8192, capacity)) {}
  bool Request(RibbonBatch prepared) {
    if (prepared.size() == m_requested.size() && std::equal(prepared.begin(), prepared.end(),
        m_requested.begin(), [](const Ribbon& a, const Ribbon& b) { return SameRibbon(*a,*b); })) {
      ++m_repeats; m_hits+=m_requested.size()-Pending(); return false;
    }
    m_requested = std::move(prepared); ++m_epoch;
    m_active.clear(); m_todo.clear(); m_next = 0;
    // Pin ALL cached entries first. Missing ribbons must not evict a requested
    // cache hit that happens to occur later in the new view's iteration order.
    for (const auto& ribbon : m_requested) {
      const size_t index = Find(*ribbon);
      if (index == npos) m_todo.push_back(ribbon);
      else {
        auto& entry = m_entries[index]; entry.epoch = m_epoch;
        ++m_hits;
        if (entry.valid) m_active.push_back(index);
      }
    }
    return true;
  }
  void Clear() { Request({}); }
  template<class Upload, class Clock> void Pump(uint64_t frame, Upload upload, Clock clock,
                                               const std::function<bool()>& cancelled = {}) {
    if (m_budgetFrame != frame) {
      m_budgetFrame = frame; m_frameAttempts = 0; m_frameSeconds = 0;
    }
    while (m_next < m_todo.size() && m_frameAttempts < 16 && m_frameSeconds < .002) {
      if (cancelled && cancelled()) return;
      const double started = clock();
      auto ribbon = m_todo[m_next++];
      size_t index = npos;
      if (m_entries.size() < m_capacity) {
        index = m_entries.size(); m_entries.push_back({});
      } else {
        uint64_t oldest = UINT64_MAX;
        for (size_t i=0; i<m_entries.size(); ++i) {
          const auto& entry = m_entries[i];
          if (entry.epoch != m_epoch && entry.ribbon->style == ribbon->style && entry.epoch < oldest) {
            oldest = entry.epoch; index = i;
          }
        }
        if (index != npos) EraseIndex(index);
      }
      if (index == npos) {
        ++m_dropped; RecordStep(std::max(0.0, clock() - started)); continue;
      }
      auto& entry = m_entries[index];
      entry.ribbon = std::move(ribbon); entry.epoch = m_epoch; entry.valid = false;
      m_index.emplace(entry.ribbon->hash, index);
      ++m_frameAttempts; ++m_attempts;
      // Store even failed geometry: an unchanged view never retries it every
      // frame. Superseding views can reuse this slot for different geometry.
      entry.valid = upload(index, *entry.ribbon);
      if (entry.valid) m_active.push_back(index); else ++m_failures;
      RecordStep(std::max(0.0, clock() - started));
    }
  }
  const std::vector<size_t>& Active() const { return m_active; }
  size_t Pending() const { return m_todo.size() - m_next; }
  size_t Size() const { return m_entries.size(); }
  uint64_t Hits() const { return m_hits; }
  uint64_t Attempts() const { return m_attempts; }
  uint64_t Failures() const { return m_failures; }
  uint64_t Dropped() const { return m_dropped; }
  double FrameSeconds() const { return m_frameSeconds; }
  size_t Requested() const { return m_requested.size(); }
  Statistics Stats() const {
    return {m_attempts,m_hits,m_failures,m_dropped,m_repeats,m_overrunFrames,
            m_buildSeconds,m_peakFrameSeconds,m_peakStepSeconds,m_peakFrameAttempts};
  }
 private:
  void RecordStep(double seconds) {
    const bool overrun=m_frameSeconds>.002;
    m_frameSeconds+=seconds; m_buildSeconds+=seconds;
    if(!overrun && m_frameSeconds>.002)++m_overrunFrames;
    m_peakFrameSeconds=std::max(m_peakFrameSeconds,m_frameSeconds);
    m_peakStepSeconds=std::max(m_peakStepSeconds,seconds);
    m_peakFrameAttempts=std::max(m_peakFrameAttempts,m_frameAttempts);
  }
  static constexpr size_t npos = size_t(-1);
  struct Entry { Ribbon ribbon; uint64_t epoch = 0; bool valid = false; };
  size_t Find(const RibbonGeometry& ribbon) const {
    const auto range = m_index.equal_range(ribbon.hash);
    for (auto it=range.first; it!=range.second; ++it)
      if (SameRibbon(*m_entries[it->second].ribbon, ribbon)) return it->second;
    return npos;
  }
  void EraseIndex(size_t index) {
    const auto range = m_index.equal_range(m_entries[index].ribbon->hash);
    for (auto it=range.first; it!=range.second; ++it) if (it->second == index) { m_index.erase(it); break; }
  }
  size_t m_capacity, m_next = 0;
  uint64_t m_epoch = 0, m_budgetFrame = UINT64_MAX;
  uint64_t m_hits = 0, m_attempts = 0, m_failures = 0, m_dropped = 0;
  uint64_t m_repeats = 0, m_overrunFrames = 0;
  unsigned m_frameAttempts = 0;
  unsigned m_peakFrameAttempts = 0;
  double m_frameSeconds = 0;
  double m_buildSeconds = 0, m_peakFrameSeconds = 0, m_peakStepSeconds = 0;
  std::vector<Entry> m_entries;
  std::unordered_multimap<uint64_t, size_t> m_index;
  RibbonBatch m_requested, m_todo;
  std::vector<size_t> m_active;
};
} // namespace v3
