#pragma once
#include "freight.h"
#include <optional>

namespace v3 {
enum class GoodsCategory : uint8_t { Staple=1, Industrial=2, Luxury=4, Military=8 };
struct LogisticsOrders { std::string name; double quantity=0; };
struct LogisticsGood {
  uint32_t id=0;
  std::string name,key,category;
  std::optional<double> price, priceDeviation;
  double freight=0,share=0,sellOrders=0,buyOrders=0;
  std::vector<LogisticsOrders> sellers,buyers;
  std::optional<FreightDemand> intermediaryDemand;
};
struct LogisticsMarket { uint32_t id=0; std::string name; };
struct LogisticsState { uint32_t id=0,market=0; std::string name; };
struct LogisticsQuote {
  uint32_t market=0,good=0;
  std::optional<double> price,basePrice;
};
// Copied presentation data only. Missing prices/calibration remain unknown,
// never zero or invented game readings. Build it outside the render callback.
struct LogisticsDocument {
  uint64_t world=0,revision=0;
  uint32_t market=0;
  FreightViewKind kind=FreightViewKind::Market;
  uint32_t state=UINT32_MAX;
  std::vector<LogisticsMarket> markets;
  std::vector<LogisticsState> states;
  std::vector<LogisticsGood> goods;
  double allFreight=0,filteredFreight=0,builtRailLength=0;
  double selectionFreight=0; // card total before transient map-only hover
  // Model demand for the selected scope, in orders. Allocated is bounded by
  // target; unmet is the difference and is never folded into drawn freight.
  double targetImport=0,allocatedImport=0,unmetDemand=0;
  std::optional<FreightDemand> intermediaryDemand;
  uint32_t freightDemandRevision=0;
  // True only for the explicit debug test input; the UI must label it and the
  // native game panel must never receive such a document.
  bool synthetic=false;
  // Market heat reference from the immutable FreightView. Zero means the market
  // has no freight; the legend must not invent a full scale.
  double heatReference=0;
  uint32_t heatScaleRevision=0;
  std::optional<double> railKilometres;
};
LogisticsDocument BuildLogisticsDocument(const EconomySnapshot&, const FreightResult&,
    const FreightFilter&, const std::vector<LogisticsQuote>& quotes={},
    const std::vector<LogisticsMarket>& markets={}, const std::vector<LogisticsState>& states={},
    std::optional<double> mapUnitsPerKilometre={});
// Player-facing variable value. Missing, synthetic or invalid data stays --.
std::string FormatFreightDemand(const LogisticsDocument*);
// Native card work for the current filters; hover does not change this total.
std::string FormatFreightWork(const LogisticsDocument*);
// Canonical per-good native variable payload; missing/synthetic input clears it.
std::vector<std::pair<std::string,std::string>> FormatFreightGoods(const LogisticsDocument*);
// State work uses separate keys; a market document or different state is refused.
std::vector<std::pair<std::string,std::string>> FormatStateFreightGoods(const LogisticsDocument*,uint32_t state);
// Overload for callers that already own the cached aggregate: avoids rebuilding
// the view on the UI thread.
LogisticsDocument BuildLogisticsDocument(const EconomySnapshot&, const FreightResult&,
    const FreightView&, const FreightFilter&, const std::vector<LogisticsQuote>& quotes={},
    const std::vector<LogisticsMarket>& markets={}, const std::vector<LogisticsState>& states={},
    std::optional<double> mapUnitsPerKilometre={});

// View navigation is separate from the freight filter. A row hover temporarily
// overrides only map selection; it cannot erase category/search/detail state.
struct LogisticsSelection {
  uint32_t market=0,state=0;
  uint8_t categories=15;
  std::string search;
  bool flowsVisible=true;
  int64_t selectedGood=-1,hoverGood=-1;
  FreightViewKind kind=FreightViewKind::Market;
  bool restrictGoods=false;
  std::vector<uint32_t> allowedGoods;
  bool explicitMarket=false; // native ID 0 differs from an unspecified legacy selection
  void SetCategory(GoodsCategory category);
  void AllCategories();
  void Search(std::string value);
  void OpenGood(uint32_t good);
  void Back();
  void SelectMarket(uint32_t id);
  void SelectState(uint32_t id);
  bool Matches(const LogisticsGood&) const;
  FreightFilter Filter() const;
};
} // namespace v3
