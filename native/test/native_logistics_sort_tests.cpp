#include "../src/native_logistics_sort.cpp"
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace v3 {
Game& Game::Instance(){static Game game;return game;}
void LogLine(LogLevel,const char*,...){}
}
static bool cleared=false,throwSort=false;
static void __fastcall VanillaSort(void*,const v3::SsoKey*){
  cleared=v3::active==nullptr;
  if(throwSort)throw std::runtime_error("fixture native sort failure");
}

static void Check(bool value,const char* why){if(!value){std::fprintf(stderr,"FAIL %s\n",why);std::exit(1);}}
int main(){
  using namespace v3;
  LogisticsDocument document;document.world=7;document.revision=9;document.market=0;
  document.nativeTrade=NativeTradeSummary{2000000000,2000000000,2000000000,2000000000,2000000000};
  for(auto [id,quantity]:{std::pair{1u,900000000LL},{2u,1000000000LL},{3u,100000000LL},{4u,0LL}}){
    LogisticsGood good;good.id=id;good.tradeQuantity=quantity;document.goods.push_back(good);
  }
  const auto original=document;
  auto values=CopyNativeLogisticsSortValues(&document);
  Check(values && values->world==7 && values->revision==9 && values->market==0 &&
      values->quantities[1]<values->quantities[2] && values->quantities[3]<values->quantities[1] &&
      values->quantities[4]==0 && values->quantities[0]==-1,"numeric values distinguish 1k, 9k, 10k, known zero and absent goods");
  document.goods[0].tradeQuantity=1;
  Check(values->quantities[1]==900000000,"sort payload owns its values independently of publication changes");
  auto invalid=[&](auto change){auto bad=original;change(bad);return !CopyNativeLogisticsSortValues(&bad);};
  Check(!CopyNativeLogisticsSortValues(nullptr) && invalid([](auto& x){x.world=0;}) &&
      invalid([](auto& x){x.revision=0;}) && invalid([](auto& x){x.synthetic=true;}) &&
      invalid([](auto& x){x.nativeTrade.reset();}) && invalid([](auto& x){x.market=UINT32_MAX;}),
      "unknown, synthetic and retired publication contexts cannot supply native sort values");
  Check(invalid([](auto& x){x.goods[0].tradeQuantity.reset();}) &&
      invalid([](auto& x){x.goods[0].tradeQuantity=-1;}) &&
      invalid([](auto& x){x.goods[0].id=128;}) &&
      invalid([](auto& x){x.goods.push_back(x.goods[0]);}) &&
      invalid([](auto& x){x.goods.pop_back();x.goods.pop_back();}) &&
      invalid([](auto& x){x.nativeTrade->scopeQuantity=1;}),"missing, duplicated and inconsistent numeric rows refuse the whole payload");
  document=original;document.kind=FreightViewKind::State;
  Check(!CopyNativeLogisticsSortValues(&document),"state sort requires a full runtime state identity");
  document.state=0x80000011;
  values=CopyNativeLogisticsSortValues(&document);
  Check(values && values->state==0x80000011 && values->kind==FreightViewKind::State,"state zero/high-bit identities are not map keys");
  document.state=0;Check(CopyNativeLogisticsSortValues(&document).has_value(),"runtime state zero remains valid");
  document=original;document.goods.resize(2);
  const int64_t large=int64_t(1)<<54;
  document.goods[0].tradeQuantity=large;document.goods[1].tradeQuantity=large+1;
  document.nativeTrade=NativeTradeSummary{2*large+1,2*large+1,0,0,0};
  values=CopyNativeLogisticsSortValues(&document);
  Check(values && values->quantities[2]-values->quantities[1]==1,"sorting never loses integer precision through shortened labels or doubles");
  document.nativeTrade=NativeTradeSummary{INT64_MAX,INT64_MAX,0,0,0};
  document.goods[0].tradeQuantity=INT64_MAX;document.goods[1].tradeQuantity=1;
  Check(!CopyNativeLogisticsSortValues(&document),"overflow cannot wrap a scope total");
  Scope outer{FreightViewKind::Market,0,{}};active=&outer;originalSort=&VanillaSort;
  char keyText[]="ironroad_work";SsoKey heapKey("");const auto keyPointer=uintptr_t(keyText);
  std::memcpy(heapKey.text,&keyPointer,sizeof(keyPointer));heapKey.size=13;heapKey.capacity=31;
  Check(CustomKey(&heapKey),"native short keys can retain heap storage from an earlier longer selector");
  heapKey.capacity=14;Check(!CustomKey(&heapKey),"invalid native string capacities are rejected");heapKey.capacity=31;
  heapKey.size=4097;Check(!CustomKey(&heapKey),"native selector lengths are bounded before reading");heapKey.size=13;
  const SsoKey price("price");
  SortThunk(nullptr,&price);
  Check(cleared && active==&outer,"nested original selectors cannot inherit custom quantity getters");
  const SsoKey work("ironroad_work");cleared=false;SortThunk(nullptr,&work);
  Check(cleared && active==&outer,"foreign sort owners cannot inherit custom quantity getters");
  throwSort=true;cleared=false;
  try{SortThunk(nullptr,&price);}catch(const std::runtime_error&){}
  Check(cleared && active==&outer,"native selector exceptions restore the previous numeric scope");
  active=nullptr;
  originalMarketQuantity=+[](void*,int64_t* output)->int64_t*{*output=-123;return output;};
  int64_t quantity=0;
  Check(MarketQuantity(nullptr,&quantity)==&quantity && quantity==-123,"original quantity getters remain untouched outside custom sorting");
  module=reinterpret_cast<uint8_t*>(0x140000000);
  struct Definition {uintptr_t type,padding;uint32_t id;};
  struct Row {uintptr_t type,padding,definition;uint32_t state,market;};
  static_assert(offsetof(Definition,id)==0x10 && offsetof(Row,state)==0x18 && sizeof(Row)==0x20);
  Definition a{uintptr_t(module)+kRvaGoodVtable,0,1},b{uintptr_t(module)+kRvaGoodVtable,0,2};
  Row first{uintptr_t(module)+kRvaGuiGoodsVtable,0,uintptr_t(&a),UINT32_MAX,0};
  Row second=first;second.definition=uintptr_t(&b);
  Scope current{FreightViewKind::Market,0,std::make_shared<const NativeLogisticsSortValues>(*CopyNativeLogisticsSortValues(&original))};
  active=&current;
  Check(MarketQuantity(&first,&quantity)==&quantity && quantity==900000000,"only the matching current GUI row receives the exact quantity");
  first.market=1;MarketQuantity(&first,&quantity);
  Check(quantity==0,"foreign-scope rows cannot receive the active market quantity");first.market=0;
  uintptr_t rows[]={uintptr_t(&first),uintptr_t(&second)};
  PdxVector vector{rows,2,2,nullptr};uintptr_t sorter=uintptr_t(&vector);
  Check(CurrentRows(uintptr_t(&sorter),current),"current native row membership validates before native sorting");
  rows[1]=rows[0];Check(!CurrentRows(uintptr_t(&sorter),current),"duplicate native rows reject the payload");rows[1]=uintptr_t(&second);
  b.id=127;Check(!CurrentRows(uintptr_t(&sorter),current),"unpublished native goods reject the whole sort payload");b.id=2;
  vector.count=129;Check(!CurrentRows(uintptr_t(&sorter),current),"native row bounds are checked before reading");
  vector.count=2;
  alignas(8) std::array<uint8_t,0x230> market{};
  auto put=[&](size_t offset,const auto& value){std::memcpy(market.data()+offset,&value,sizeof(value));};
  put(0,uintptr_t(module)+kRvaGuiMarketVtable);put(0x10,uint32_t(0));
  auto* currentSorter=market.data()+0x198;put(0x198,uintptr_t(&vector));
  std::memcpy(currentSorter+8,&work,sizeof(work));
  static unsigned refreshes=0;refreshSort=+[](void*){++refreshes;};
  Check(RefreshCurrentSorter(uintptr_t(currentSorter),current.values) && refreshes==1,
      "newly published values refresh the selected native sort after an unready state switch");
  auto foreign=std::make_shared<NativeLogisticsSortValues>(*current.values);foreign->market=1;
  Check(!RefreshCurrentSorter(uintptr_t(currentSorter),foreign) && refreshes==1,"refresh cannot use another market publication");
  std::memcpy(currentSorter+8,&price,sizeof(price));
  Check(RefreshCurrentSorter(uintptr_t(currentSorter),current.values) && refreshes==1,"publication never refreshes or replaces a stock selected sort");
  PublishNativeLogisticsSort(&original);Check(refreshPending.load(),"ready publication requests a native refresh");
  RefreshNativeLogisticsSort();Check(refreshPending.load() && refreshes==1,"unavailable current UI ownership defers refresh without calling native code");
  PublishNativeLogisticsSort(nullptr);Check(!refreshPending.load() && !published.load(),"retirement clears pending refreshes and copied values");
  active=nullptr;module=nullptr;
  std::string reason;
  Check(!InstallNativeLogisticsSort(nullptr,&reason) && !reason.empty(),"unverified profiles never install native hooks");
  std::puts("PASS native logistics sorting values: exact quantities, fixed-denominator share order, owned scope and invalid publication refusal");
}
