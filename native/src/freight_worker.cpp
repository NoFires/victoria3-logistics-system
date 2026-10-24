#include "freight_worker.h"

namespace v3 {
FreightWorker::FreightWorker(GeographyHandle geography, FreightPolicy policy)
    : m_geography(std::move(geography)),
      m_policy(std::move(policy)),
      m_engine(m_geography, m_policy),
      m_worker([this](const FreightJob& job, const Cancelled& cancelled) {
        return Compute(job, cancelled);
      }) {}

FreightWorkerReply FreightWorker::Compute(const FreightJob& job, const Cancelled& cancelled) {
  FreightWorkerReply reply;
  reply.owner = job.owner;
  if (cancelled()) {
    reply.error = "cancelled";
    return reply;
  }
  const auto reset = m_resetRequested.load();
  if (reset != m_resetApplied) {
    m_engine.ResetWorld();
    m_appliedSnapshot.reset();
    m_resetApplied = reset;
  }
  if (job.resetWorld) {
    m_engine.ResetWorld();
    m_appliedSnapshot.reset();
    reply.stats = m_engine.Stats();
    return reply;
  }
  if (job.updateSnapshot) {
    if (!job.snapshot) {
      reply.error = "missing economy snapshot";
      return reply;
    }
    if(job.snapshot!=m_appliedSnapshot) {
      std::string reason;
      if (!m_engine.SetSnapshot(*job.snapshot, &reason)) {
        reply.error = reason.empty() ? "rejected economy snapshot" : reason;
        return reply;
      }
      m_appliedSnapshot=job.snapshot;
    }
  } else if (!m_engine.HasSnapshot()) {
    reply.error = "no economy snapshot";
    return reply;
  }
  reply.frame = m_engine.Build(job.filter, job.coordScale, job.heightScale, job.heightOffset,
                               cancelled);
  reply.stats = m_engine.Stats();
  if (reply.frame && !reply.frame->error.empty()) reply.error = reply.frame->error;
  return reply;
}
}  // namespace v3
