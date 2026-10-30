#include "logistics_ui.h"
#include "native_trade_view.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <map>

namespace v3 {
namespace {
std::string Lower(std::string value) {
  for (char& c:value) if (c>='A' && c<='Z') c+=32;
  return value;
}
uint8_t Category(const std::string& name) {
  if (name=="staple") return 1;
  if (name=="industrial") return 2;
  if (name=="luxury") return 4;
  if (name=="military") return 8;
  return 0;
}
} // namespace
std::string FormatNativeTradeQuantity(int64_t quantity,bool compact) {
  if(quantity<0)return "--";
  if(compact && quantity>=1000*kNativeTradeQuantityScale){
    char text[64];const bool millions=quantity>=1000000*kNativeTradeQuantityScale;
    std::snprintf(text,sizeof(text),"%.1f%s",double(quantity)/kNativeTradeQuantityScale/(millions?1000000:1000),millions?"M":"k");
    return text;
  }
  char fraction[8];std::snprintf(fraction,sizeof(fraction),"%05lld",(long long)(quantity%kNativeTradeQuantityScale));
  std::string value=std::to_string(quantity/kNativeTradeQuantityScale)+"."+fraction;
  while(value.back()=='0' && value.size()-value.find('.')>3)value.pop_back();
  return value;
}
std::string FormatFreightDemand(const LogisticsDocument* document) {
  if (!document || !document->world || document->synthetic ||
      document->freightDemandRevision != kFreightDemandRevision || !document->intermediaryDemand)
    return "-- / -- / --";
  const auto& d = *document->intermediaryDemand;
  if (!std::isfinite(d.target) || !std::isfinite(d.fulfilled) || !std::isfinite(d.unmet) ||
      d.target < 0 || d.fulfilled < 0 || d.unmet < 0 || d.fulfilled > d.target)
    return "-- / -- / --";
  char buffer[192];
  std::snprintf(buffer, sizeof(buffer), "%.2f / %.2f / %.2f", d.target, d.fulfilled, d.unmet);
  return buffer;
}
std::string FormatFreightWork(const LogisticsDocument* document) {
  if(document && document->world && !document->synthetic && document->nativeTrade){
    const auto& value=*document->nativeTrade;
    if(value.marketQuantity<0 || value.scopeQuantity<0 || value.scopeDrawable<0 || value.selectionQuantity<0 ||
       value.scopeQuantity>value.marketQuantity || value.scopeDrawable>value.scopeQuantity || value.selectionQuantity>value.scopeQuantity)return "--";
    return FormatNativeTradeQuantity(value.selectionQuantity);
  }
  if (!document || !document->world || document->synthetic ||
      !std::isfinite(document->allFreight) || document->allFreight < 0 ||
      !std::isfinite(document->selectionFreight) || document->selectionFreight < 0 ||
      document->selectionFreight > document->allFreight) return "--";
  char buffer[64];
  std::snprintf(buffer, sizeof(buffer), "%.2f", document->selectionFreight);
  return buffer;
}
std::string FormatLaidTrack(const LogisticsDocument* document) {
  if(!document || !document->world || document->synthetic)return "--";
  const auto value=document->railKilometres?document->railKilometres:document->railMapUnits;
  if(!value || !std::isfinite(*value) || *value<0)return "--";
  char buffer[64];std::snprintf(buffer,sizeof(buffer),"%.2f %s",*value,document->railKilometres?"km":"MU");
  return buffer;
}
static std::vector<std::pair<std::string,std::string>> FreightGoodsVariables(const LogisticsDocument* document,bool compact) {
  if (!document || !document->world || document->synthetic || document->goods.size()>128 ||
      !std::isfinite(document->allFreight) || document->allFreight<0) return {};
  std::map<std::string,std::string> values;
  for (const auto& good:document->goods) {
    if (good.key.empty() || good.key.size()>128 || std::any_of(good.key.begin(),good.key.end(),[](char c){
        return !((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_');})) return {};
    if(document->nativeTrade){
      const int64_t total=document->nativeTrade->marketQuantity;
      const bool valid=total>=0 && good.tradeQuantity && *good.tradeQuantity>=0 && *good.tradeQuantity<=total;
      const auto quantity=valid?FormatNativeTradeQuantity(*good.tradeQuantity,compact):"--";
      char share[64];
      if(valid && total>0)std::snprintf(share,sizeof(share),"%.1f%%",double(*good.tradeQuantity)/double(total)*100);
      else std::strcpy(share,"--");
      if(!values.emplace("ironroad_good_work_"+good.key,quantity).second)return {};
      values.emplace("ironroad_good_share_"+good.key,share);continue;
    }
    const bool valid=std::isfinite(good.freight) && good.freight>=0 && good.freight<=document->allFreight;
    char work[64],share[64];
    if (valid && compact && good.freight>=1000000) std::snprintf(work,sizeof(work),"%.1fM",good.freight/1000000);
    else if (valid && compact && good.freight>=1000) std::snprintf(work,sizeof(work),"%.1fk",good.freight/1000);
    else if (valid) std::snprintf(work,sizeof(work),"%.2f",good.freight);
    else std::strcpy(work,"--");
    if (valid && document->allFreight>0)
      std::snprintf(share,sizeof(share),"%.1f%%",good.freight/document->allFreight*100);
    else std::strcpy(share,"--");
    if (!values.emplace("ironroad_good_work_"+good.key,work).second) return {};
    values.emplace("ironroad_good_share_"+good.key,share);
  }
  return {values.begin(),values.end()};
}

LogisticsDocument BuildLogisticsDocument(const EconomySnapshot& snapshot,const NativeTradeView& view,const FreightFilter& filter) {
  LogisticsDocument document;
  if(!snapshot.nativeTrade || !snapshot.world || view.world!=snapshot.world || view.revision!=snapshot.revision)return document;
  document.world=snapshot.world;document.revision=snapshot.revision;document.market=filter.market;
  document.kind=filter.kind;document.state=filter.kind==FreightViewKind::State?filter.state:UINT32_MAX;
  document.nativeTrade=NativeTradeSummary{view.marketQuantity,view.scopeQuantity,view.scopeDrawable,view.selectionQuantity,view.filteredQuantity};
  document.nativeTrade->domesticMarketQuantity=view.domesticMarketQuantity;
  document.nativeTrade->domesticScopeQuantity=view.domesticScopeQuantity;
  document.nativeTrade->domesticSelectionQuantity=view.domesticSelectionQuantity;
  document.railMapUnits=view.builtRailMapLength;
  if(document.railMapUnits)document.builtRailLength=*document.railMapUnits;
  document.targetImport=double(view.domesticTarget)/kQuantityScale;
  document.allocatedImport=double(view.domesticAllocated)/kQuantityScale;
  document.unmetDemand=double(view.domesticUnmet)/kQuantityScale;
  document.intermediaryDemand=view.domesticDemand;
  if(view.domesticDemand)document.freightDemandRevision=kFreightDemandRevision;
  document.heatReference=double(view.heatReference)/kNativeTradeQuantityScale;document.heatScaleRevision=kHeatScaleRevision;
  std::set<uint32_t> markets;
  for(const auto& state:snapshot.nativeTrade->states){
    document.states.push_back({state.id,state.market,"State "+std::to_string(state.id)});markets.insert(state.market);
  }
  for(uint32_t market:markets)document.markets.push_back({market,"Market "+std::to_string(market)});
  std::map<uint32_t,int64_t> quantities;
  for(const auto& row:view.goods)quantities.emplace(row.good,row.quantity);
  for(const auto& good:snapshot.goods){
    LogisticsGood row;row.id=good.id;row.key=good.key;row.name=good.name.empty()?good.key:good.name;row.category=good.category;
    const auto quantity=quantities.find(good.id);
    if(quantity!=quantities.end())row.tradeQuantity=quantity->second;
    if(row.tradeQuantity && view.marketQuantity>0)row.share=double(*row.tradeQuantity)/double(view.marketQuantity);
    document.goods.push_back(std::move(row));
  }
  return document;
}
std::vector<std::pair<std::string,std::string>> FormatFreightGoods(const LogisticsDocument* document) {
  return FreightGoodsVariables(document,false);
}
std::vector<std::pair<std::string,std::string>> FormatStateFreightGoods(const LogisticsDocument* document,uint32_t state) {
  if (!document || state==UINT32_MAX || document->kind!=FreightViewKind::State ||
      document->state!=state || !document->revision) return {};
  if (std::count_if(document->states.begin(),document->states.end(),[&](const auto& s){return s.id==state;})!=1 ||
      std::none_of(document->states.begin(),document->states.end(),[&](const auto& s){return s.id==state && s.market==document->market;})) return {};
  std::vector<std::pair<std::string,std::string>> values;
  const std::string prefix="ironroad_good_work_";
  for (auto& [key,value]:FreightGoodsVariables(document,true))
    if (key.starts_with(prefix)) values.emplace_back("ironroad_state_work_"+std::to_string(int32_t(state))+"_"+key.substr(prefix.size()),std::move(value));
  return values;
}
void LogisticsSelection::SetCategory(GoodsCategory category) {
  const auto bit=static_cast<uint8_t>(category);
  categories=categories==15?bit:categories^bit;
  selectedGood=hoverGood=-1;
}
void LogisticsSelection::AllCategories() { categories=15; selectedGood=hoverGood=-1; }
void LogisticsSelection::Search(std::string value) { search=std::move(value); selectedGood=hoverGood=-1; }
void LogisticsSelection::OpenGood(uint32_t good) { selectedGood=good; hoverGood=-1; }
void LogisticsSelection::Back() { selectedGood=hoverGood=-1; }
void LogisticsSelection::SelectMarket(uint32_t id) {
  explicitMarket=true;
  if (market==id) return;
  market=id; state=0; selectedGood=hoverGood=-1;
}
void LogisticsSelection::SelectState(uint32_t id) {
  if (state==id) return;
  state=id; selectedGood=hoverGood=-1;
}
bool LogisticsSelection::Matches(const LogisticsGood& good) const {
  if (restrictGoods && std::find(allowedGoods.begin(),allowedGoods.end(),good.id)==allowedGoods.end()) return false;
  if (categories!=15 && !(categories & Category(good.category))) return false;
  return search.empty() || Lower(good.name+" "+good.key).find(Lower(search))!=std::string::npos;
}
FreightFilter LogisticsSelection::Filter() const {
  FreightFilter filter;
  filter.kind=kind;filter.market=market;filter.state=state;filter.search=search;
  filter.hoverGood=hoverGood>=0?hoverGood:selectedGood;
  filter.mapDirection=mapDirection;filter.mapTransport=mapTransport;
  filter.colorMode=colorMode;
  filter.restrictCategories=categories!=15;
  filter.restrictGoods=restrictGoods;
  if(restrictGoods){
    filter.allowedGoods=allowedGoods;
    std::sort(filter.allowedGoods.begin(),filter.allowedGoods.end());
    filter.allowedGoods.erase(std::unique(filter.allowedGoods.begin(),filter.allowedGoods.end()),filter.allowedGoods.end());
  }
  for (const auto* name:{"staple","industrial","luxury","military"})
    if (categories & Category(name)) filter.categories.emplace_back(name);
  return filter;
}

LogisticsDocument BuildLogisticsDocument(const EconomySnapshot& snapshot,const FreightResult& result,
    const FreightFilter& filter,const std::vector<LogisticsQuote>& quotes,
    const std::vector<LogisticsMarket>& marketNames,const std::vector<LogisticsState>& stateNames,
    std::optional<double> mapUnitsPerKilometre) {
  if (!result.complete || result.world!=snapshot.world || result.snapshotRevision!=snapshot.revision)
    return {};
  return BuildLogisticsDocument(snapshot,result,BuildFreightView(snapshot,result,filter),filter,
                                quotes,marketNames,stateNames,mapUnitsPerKilometre);
}

LogisticsDocument BuildLogisticsDocument(const EconomySnapshot& snapshot,const FreightResult& result,
    const FreightView& view,const FreightFilter& filter,const std::vector<LogisticsQuote>& quotes,
    const std::vector<LogisticsMarket>& marketNames,const std::vector<LogisticsState>& stateNames,
    std::optional<double> mapUnitsPerKilometre) {
  LogisticsDocument doc;
  if (!result.complete || result.world!=snapshot.world || result.snapshotRevision!=snapshot.revision) return doc;
  doc.world=snapshot.world;doc.revision=snapshot.revision;doc.market=filter.market;
  doc.kind=filter.kind;doc.state=filter.kind==FreightViewKind::State?filter.state:UINT32_MAX;
  doc.markets=marketNames;doc.states=stateNames;
  doc.allFreight=view.marketFreight;doc.filteredFreight=view.filteredFreight;doc.builtRailLength=view.builtRailLength;
  doc.selectionFreight=view.selectionFreight;
  doc.targetImport=view.targetImport;doc.allocatedImport=view.allocatedImport;doc.unmetDemand=view.unmetDemand;
  doc.intermediaryDemand=view.intermediaryDemand;doc.freightDemandRevision=result.freightDemandRevision;
  doc.heatReference=view.heatReference;doc.heatScaleRevision=view.heatScaleRevision;
  if (mapUnitsPerKilometre && std::isfinite(*mapUnitsPerKilometre) && *mapUnitsPerKilometre>0)
    doc.railKilometres=doc.builtRailLength/ *mapUnitsPerKilometre;
  std::map<uint32_t,std::string> names;
  for (const auto& state:stateNames) names[state.id]=state.name;
  std::map<uint32_t,LogisticsGood> goods;
  for (const auto& good:snapshot.goods) {
    auto& row=goods[good.id];row.id=good.id;row.key=good.key;row.name=good.name.empty()?good.key:good.name;row.category=good.category;
    if (doc.intermediaryDemand) row.intermediaryDemand = FreightDemand{};
  }
  for (const auto& balance : result.balances) {
    if (balance.market != filter.market || !goods.count(balance.good) ||
        (filter.kind == FreightViewKind::State && balance.state != filter.state)) continue;
    auto& demand = goods.at(balance.good).intermediaryDemand;
    if (!demand) continue;
    demand->target += balance.freightDemand.target;
    demand->fulfilled += balance.freightDemand.fulfilled;
    demand->unmet += balance.freightDemand.unmet;
  }
  std::map<uint32_t,bool> present;
  for (const auto& orders:snapshot.orders) {
    const auto name=names.count(orders.state)?names.at(orders.state):"State "+std::to_string(orders.state);
    if (std::none_of(doc.markets.begin(),doc.markets.end(),[&](const auto& market){return market.id==orders.market;}))
      doc.markets.push_back({orders.market,"Market "+std::to_string(orders.market)});
    if (std::none_of(doc.states.begin(),doc.states.end(),[&](const auto& state){return state.id==orders.state;}))
      doc.states.push_back({orders.state,orders.market,name});
    if (orders.market!=filter.market || !goods.count(orders.good)) continue;
    present[orders.good]=true;
    auto& row=goods.at(orders.good);
    const double production=double(orders.production)/kQuantityScale,consumption=double(orders.consumption)/kQuantityScale;
    row.sellOrders+=production;row.buyOrders+=consumption;
    if (production>0) row.sellers.push_back({name,production});
    if (consumption>0) row.buyers.push_back({name,consumption});
  }
  for (const auto& total:view.goods) if (goods.count(total.good)) {
    goods.at(total.good).freight=total.freight;goods.at(total.good).share=total.share;
  }
  for (const auto& quote:quotes) if (quote.market==filter.market && goods.count(quote.good) &&
      quote.price && std::isfinite(*quote.price) && *quote.price>=0) {
    auto& row=goods.at(quote.good);row.price=quote.price;
    if (quote.basePrice && std::isfinite(*quote.basePrice) && *quote.basePrice>0)
      row.priceDeviation=*quote.price/ *quote.basePrice-1;
  }
  const auto descending=[](const auto& a,const auto& b){return a.quantity>b.quantity;};
  for (auto& [id,row]:goods) if (present[id]) {
    std::stable_sort(row.sellers.begin(),row.sellers.end(),descending);
    std::stable_sort(row.buyers.begin(),row.buyers.end(),descending);
    doc.goods.push_back(std::move(row));
  }
  std::stable_sort(doc.goods.begin(),doc.goods.end(),[](const auto& a,const auto& b){
    return a.freight!=b.freight?a.freight>b.freight:a.id<b.id;
  });
  return doc;
}
} // namespace v3
