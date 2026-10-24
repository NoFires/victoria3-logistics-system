#pragma once
#include <cstdint>
#include <cstddef>
#include <array>

namespace v3 {
// Native submission/render functions read indices at byte offsets 0, 8, 16.
struct RenderHandle { int32_t index; uint32_t generation; };
struct ArrowPassContext { RenderHandle domain, stage, view; };
static_assert(offsetof(ArrowPassContext, stage) == 8);
static_assert(offsetof(ArrowPassContext, view) == 16);

inline bool MatchesArrowScope(const ArrowPassContext& context, int32_t domain, int32_t stage) {
  return domain >= 0 && stage >= 0 && context.domain.index == domain && context.stage.index == stage;
}

// Native updaters submit for every active view (the third context index),
// including wrapped-map views. Deduplicate logical passes, never stack addresses.
class ArrowSubmissionGuard {
 public:
  bool Allow(uint64_t frame, const ArrowPassContext& context, int32_t domain, int32_t stage) {
    if (!MatchesArrowScope(context, domain, stage) || context.view.index < 0) return false;
    if (frame != m_frame) { m_frame = frame; m_count = 0; }
    for (size_t i = 0; i < m_count; ++i) if (m_views[i] == context.view.index) return false;
    if (m_count == m_views.size()) return false;
    m_views[m_count++] = context.view.index;
    return true;
  }
 private:
  uint64_t m_frame = ~uint64_t(0);
  size_t m_count = 0;
  std::array<int32_t, 64> m_views{};
};
}  // namespace v3
