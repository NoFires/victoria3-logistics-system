#pragma once
#include "logistics_ui.h"
#include <memory>

namespace v3 {
class LogisticsPanel {
 public:
  // Returns only small selection changes. No game lookups, allocation solver,
  // graph search or native rendering runs in an ImGui callback.
  bool Draw(const LogisticsDocument* document, FreightViewKind kind);
  const LogisticsSelection& Selection() const { return m_selection; }
  // UI-thread entry for a future game-side goods-selection event. Stable ID only.
  void RequestGood(uint32_t good) { m_pendingGood=good; }
 private:
  void DrawFilters();
  void DrawGoods(const LogisticsDocument&);
  void DrawDetail(const LogisticsDocument&,const LogisticsGood&);
  LogisticsSelection m_selection;
  char m_search[128]{};
  int m_sortColumn=3;
  bool m_sortDescending=true,m_changed=false;
  int64_t m_hover=-1;
  uint64_t m_world=0;
  std::optional<uint32_t> m_pendingGood;
};
} // namespace v3
