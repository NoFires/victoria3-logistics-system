#pragma once
#include "freight_engine.h"
#include "latest_worker.h"

namespace v3 {
// A request is either a new copied economy snapshot (adapter result) or a new
// view of the snapshot already owned by the worker. The snapshot is an
// immutable numeric copy: no engine pointer ever crosses this boundary.
struct FreightJob {
  std::shared_ptr<const EconomySnapshot> snapshot;
  bool updateSnapshot = true;
  // World/device change: drop the worker-side snapshot and both caches before
  // any new data is accepted.
  bool resetWorld = false;
  FreightFilter filter;
  float coordScale = 1, heightScale = 0, heightOffset = 0;
  // Who asked for this view (native market panel, state panel, debug preview).
  // The reply carries it so a late result cannot be applied to another owner.
  uint32_t owner = 0;
};

struct FreightWorkerReply {
  uint32_t owner = 0;
  std::shared_ptr<const FreightFrame> frame;
  FreightEngineStats stats;
  std::string error;
};

// Routes allocation and view assembly off the render/UI thread. Uses the same
// settle/max-wait and generation cancellation as the route preview worker.
class FreightWorker {
 public:
  using Worker = LatestWorker<FreightJob, FreightWorkerReply>;
  using Cancelled = Worker::Cancelled;
  FreightWorker(GeographyHandle geography, FreightPolicy policy = {});
  uint64_t Request(FreightJob job) {
    LockGuard lock(m_requestMutex);
    if(job.resetWorld)m_requestedSnapshot.reset();
    else if(job.updateSnapshot) {
      if(job.snapshot)m_requestedSnapshot=job.snapshot;
    } else if(m_requestedSnapshot) {
      job.snapshot=m_requestedSnapshot;job.updateSnapshot=true;
    }
    return m_worker.Submit(std::move(job));
  }
  uint64_t Cancel() { return m_worker.Cancel(); }
  // Latch invalidation independently of the coalescing mailbox. A newer
  // request/Cancel must not overwrite a reset before the worker consumes it.
  uint64_t Reset() {
    LockGuard lock(m_requestMutex);
    m_requestedSnapshot.reset();
    ++m_resetRequested;
    return m_worker.Cancel();
  }
  uint64_t Generation() const { return m_worker.Generation(); }
  std::shared_ptr<const Worker::Reply> Poll() { return m_worker.Poll(); }
  void Stop() { m_worker.Stop(); }
 private:
  FreightWorkerReply Compute(const FreightJob& job, const Cancelled& cancelled);
  // Declared before the worker: the worker thread must stop before the engine
  // and geography it uses are destroyed.
  GeographyHandle m_geography;
  FreightPolicy m_policy;
  FreightEngine m_engine;
  // Snapshot delivery must survive a newer filter replacing the mailbox job.
  Mutex m_requestMutex;
  std::shared_ptr<const EconomySnapshot> m_requestedSnapshot;
  std::shared_ptr<const EconomySnapshot> m_appliedSnapshot; // worker thread only
  std::atomic<uint64_t> m_resetRequested{0};
  uint64_t m_resetApplied = 0; // owning worker thread only
  Worker m_worker;
};
}  // namespace v3
