#pragma once
#include "logistics_ui.h"
#include <array>

namespace v3 {
struct NativeLogisticsSortValues {
  uint64_t world=0,revision=0;
  FreightViewKind kind=FreightViewKind::Market;
  uint32_t market=UINT32_MAX,state=UINT32_MAX;
  std::array<int64_t,128> quantities;
};
inline std::optional<NativeLogisticsSortValues> CopyNativeLogisticsSortValues(const LogisticsDocument* document) {
  if(!document || !document->world || !document->revision || document->synthetic || !document->nativeTrade ||
      document->goods.empty() || document->goods.size()>128 || document->market==UINT32_MAX ||
      (document->kind!=FreightViewKind::Market && document->kind!=FreightViewKind::State) ||
      (document->kind==FreightViewKind::State && document->state==UINT32_MAX))return {};
  const auto& summary=*document->nativeTrade;
  if(summary.marketQuantity<0 || summary.scopeQuantity<0 || summary.scopeQuantity>summary.marketQuantity ||
      summary.scopeDrawable<0 || summary.scopeDrawable>summary.scopeQuantity || summary.selectionQuantity<0 ||
      summary.selectionQuantity>summary.scopeQuantity)return {};
  NativeLogisticsSortValues result;result.world=document->world;result.revision=document->revision;
  result.kind=document->kind;result.market=document->market;result.state=document->state;
  result.quantities.fill(-1);
  int64_t total=0;
  for(const auto& good:document->goods){
    if(good.id>=result.quantities.size() || result.quantities[good.id]>=0 || !good.tradeQuantity ||
        *good.tradeQuantity<0 || *good.tradeQuantity>summary.scopeQuantity)return {};
    if(*good.tradeQuantity>INT64_MAX-total)return {};
    total+=*good.tradeQuantity;result.quantities[good.id]=*good.tradeQuantity;
  }
  if(total!=summary.scopeQuantity)return {};
  return result;
}
bool InstallNativeLogisticsSort(uint8_t* base,std::string* error);
void PublishNativeLogisticsSort(const LogisticsDocument* document);
void RefreshNativeLogisticsSort();
} // namespace v3
