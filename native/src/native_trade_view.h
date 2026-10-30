#pragma once
#include "native_trade_routes.h"
#include "native_trade_geometry.h"
#include "freight.h"
#include "ribbon_cache.h"
#include "native_domestic_model.h"

namespace v3 {
struct NativeTradeShape {
  std::optional<NativeTradeGeometry> geometry;
  std::string unavailable;
};
struct NativeTradeFacts {
  uint64_t world=0,revision=0;
  std::shared_ptr<const NativeWorldTradeSnapshot> source;
  std::vector<NativeTradeBinding> bindings;
  std::vector<std::optional<size_t>> shapeForBinding;
  std::vector<NativeTradeShape> shapes;
  std::optional<NativeDomesticModel> domestic;
  std::optional<std::map<uint32_t,double>> builtRailMapLengths; // unique strips, both endpoints in market
};
struct NativeGoodQuantity {uint32_t good=0;int64_t quantity=0;};
struct NativeSegmentQuantity {
  uint64_t strip=0;
  int direction=0; // state imports/exports; market aggregate0
  int64_t quantity=0,netQuantity=0;
  uint8_t band=0;
};
// Original quantities retain1e5 precision. Domestic final milli-order model
// quantities are scaled exactly into that common unit and separately retained.
// Demand fields remain milli-orders; neither kind is distance work or capacity.
struct NativeTradeView {
  uint64_t world=0,revision=0;
  FreightColorMode colorMode=FreightColorMode::Business;
  int64_t highlightGood=-1;
  int64_t marketQuantity=0,scopeQuantity=0,scopeDrawable=0;
  int64_t selectionQuantity=0,filteredQuantity=0,heatReference=0;
  int64_t domesticMarketQuantity=0,domesticScopeQuantity=0,domesticSelectionQuantity=0;
  int64_t domesticTarget=0,domesticAllocated=0,domesticUnmet=0; // milli-orders
  std::optional<FreightDemand> domesticDemand;
  std::optional<double> builtRailMapLength; // current market, independent of goods/state/hover
  std::vector<NativeGoodQuantity> goods;
  std::vector<NativeSegmentQuantity> segments;
};
std::optional<NativeTradeFacts> BuildNativeTradeFacts(const EconomySnapshot&,
    const std::function<bool()>& cancelled={},std::string* error=nullptr);
std::optional<NativeTradeView> BuildNativeTradeView(const EconomySnapshot&,const NativeTradeFacts&,
    const FreightFilter&,const std::function<bool()>& cancelled={},std::string* error=nullptr);
std::optional<RibbonBatch> BuildNativeTradeRibbons(const NativeTradeFacts&,const NativeTradeView&,
    float coordScale=1,float heightOffset=0,const std::function<bool()>& cancelled={},std::string* error=nullptr);
} // namespace v3
