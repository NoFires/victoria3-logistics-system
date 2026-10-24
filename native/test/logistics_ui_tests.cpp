#include "logistics_ui.h"
#include "logistics_panel.h"
#include <imgui.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
using namespace v3;
void Check(bool ok,const char* message) {if(!ok){std::printf("FAIL: %s\n",message);std::exit(1);}}
int main() {
  LogisticsSelection state;state.market=7;state.search="g";
  state.SetCategory(GoodsCategory::Staple);
  Check(state.categories==1 && state.search=="g","category selection preserves search");
  state.SetCategory(GoodsCategory::Industrial);
  Check(state.categories==3,"categories can be combined");
  state.OpenGood(1);state.hoverGood=2;
  Check(state.Filter().hoverGood==2,"hover temporarily overrides persistent good");
  state.hoverGood=-1;Check(state.Filter().hoverGood==1,"leaving hover restores persistent detail");
  state.flowsVisible=false;state.Back();
  Check(!state.flowsVisible && state.categories==3 && state.search=="g","back and hide preserve active filters");
  state.Search("Grain");Check(state.selectedGood==-1,"search returns to filtered goods list");
  state.SetCategory(GoodsCategory::Staple);state.SetCategory(GoodsCategory::Industrial);
  Check(state.categories==0 && state.Filter().restrictCategories && state.Filter().categories.empty(),"no categories selected means no goods, not all goods");
  state.AllCategories();Check(!state.Filter().restrictCategories,"All restores all goods");
  LogisticsGood row;row.id=1;row.key="grain";row.name="Wheat";row.category="staple";
  state.Search("WHEAT");Check(state.Matches(row),"localized display name search is case insensitive");
  state.OpenGood(1);state.SelectMarket(8);
  Check(state.selectedGood==-1 && state.state==0 && state.search=="WHEAT","market switch clears stale identities but keeps search");

  EconomySnapshot snapshot;snapshot.world=1;snapshot.revision=4;
  snapshot.goods={{1,"grain","staple",1,false,"Wheat"},{2,"tools","industrial",1,false,"Tools"}};
  snapshot.orders={{10,1,7,1,5000,0},{20,2,7,1,0,9000},{10,1,7,2,2000,0},{20,2,7,2,0,2000},{30,3,8,1,1000,0}};
  SplineGraph graph;graph.nodes={{100,0,0},{200,10,0},{300,20,0}};
  graph.byId={{100,0},{200,1},{300,2}};graph.edges={{0,0,1,0,0,2,10}};
  const auto allocated=AllocateFreight(snapshot,graph,{});
  Check(allocated.complete,allocated.error.c_str());
  FreightFilter filter;filter.market=7;
  const std::vector<LogisticsQuote> quotes={{7,1,30.,20.},{7,2,{},{}},{8,1,999.,20.}};
  auto document=BuildLogisticsDocument(snapshot,allocated,filter,quotes,{{7,"Test market"}},{{10,7,"North"},{20,7,"South"}});
  Check(FormatFreightWork(&document)=="70.00","native freight work must show 7 orders over 10 map units as 70, without a second milli-order conversion");
  auto expensivePolicy=FreightPolicy{};expensivePolicy.costPerDistance[0]=3;
  auto expensive=AllocateFreight(snapshot,graph,expensivePolicy);
  double expensiveCost=0;
  for(const auto& shipment:expensive.shipments)expensiveCost+=shipment.cost;
  const auto expensiveDocument=BuildLogisticsDocument(snapshot,expensive,filter);
  Check(expensive.complete && std::abs(expensiveCost-210)<1.e-6 && FormatFreightWork(&expensiveDocument)=="70.00",
        "tariffs change cost independently of physical freight work");
  auto weightedSnapshot=snapshot;weightedSnapshot.goods[0].weight=2;
  const auto weighted=AllocateFreight(weightedSnapshot,graph,{});
  const auto weightedDocument=BuildLogisticsDocument(weightedSnapshot,weighted,filter);
  Check(weighted.complete && FormatFreightWork(&weightedDocument)=="120.00" &&
        weightedDocument.intermediaryDemand->target==11 && weightedDocument.intermediaryDemand->fulfilled==7,
        "configured weight affects work independently of intermediary Freight demand");
  Check(FormatFreightWork(nullptr)=="--","missing freight work remains unavailable");
  auto invalidWork=document;invalidWork.world=0;
  Check(FormatFreightWork(&invalidWork)=="--","uninitialized document cannot publish zero freight work");
  invalidWork=document;invalidWork.synthetic=true;
  Check(FormatFreightWork(&invalidWork)=="--","synthetic freight work cannot enter native variables");
  for(double invalid:{-1.,double(INFINITY),double(NAN)}){
    invalidWork=document;invalidWork.allFreight=invalid;
    Check(FormatFreightWork(&invalidWork)=="--","invalid freight work remains unavailable");
  }
  invalidWork=document;invalidWork.allFreight=invalidWork.selectionFreight=0;
  Check(FormatFreightWork(&invalidWork)=="0.00","known zero freight work differs from unavailable");
  for(double invalid:{-1.,double(INFINITY),double(NAN),71.}){
    invalidWork=document;invalidWork.selectionFreight=invalid;
    Check(FormatFreightWork(&invalidWork)=="--","invalid selection work cannot masquerade as a valid card");
  }
  Check(document.world==1 && document.markets.size()==2 && document.states.size()==3,"complete selector roster survives selected market filter");
  Check(document.goods.size()==2 && document.goods[0].id==1,"goods sorted by freight with stable IDs");
  const auto& wheat=document.goods[0];
  Check(wheat.name=="Wheat" && wheat.price==30. && wheat.priceDeviation==.5,"market-specific price and base deviation");
  Check(wheat.sellOrders==5 && wheat.buyOrders==9,"details preserve buy/sell targets, distinct from actual allocation");
  Check(wheat.sellers[0].name=="North" && wheat.buyers[0].name=="South","detail order breakdown has copied state names");
  Check(!document.goods[1].price && !document.goods[1].priceDeviation && !document.railKilometres,"unknown prices and rail calibration never fabricate zero readings");
  Check(document.intermediaryDemand && document.freightDemandRevision==kFreightDemandRevision &&
        document.intermediaryDemand->target==11 && document.intermediaryDemand->fulfilled==7 &&
        document.intermediaryDemand->unmet==4,"copied document keeps Freight intermediary target/fulfilled/unmet distinct");
  Check(wheat.intermediaryDemand && wheat.intermediaryDemand->target==9 &&
        wheat.intermediaryDemand->fulfilled==5 && wheat.intermediaryDemand->unmet==4,
        "per-good document exposes the source contribution to Freight demand");
  LogisticsSelection selection;selection.market=7;selection.SetCategory(GoodsCategory::Staple);
  auto selected=BuildLogisticsDocument(snapshot,allocated,selection.Filter(),quotes);
  Check(FormatFreightWork(&selected)=="50.00" && selected.allFreight==70,
        "reference card follows goods filters while the share denominator stays all-market");
  auto hovering=selection;hovering.hoverGood=2;
  auto hoverDocument=BuildLogisticsDocument(snapshot,allocated,hovering.Filter(),quotes);
  Check(FormatFreightWork(&hoverDocument)=="50.00" && hoverDocument.filteredFreight==0 &&
        hoverDocument.goods[0].share==5./7,
        "hover affects only map contributions, never the selected-filter card or shares");
  const auto nativeGoods=FormatFreightGoods(&document);
  const auto variable=[](const auto& values,const std::string& key){
    const auto found=std::find_if(values.begin(),values.end(),[&](const auto& row){return row.first==key;});
    return found==values.end()?std::string{}:found->second;
  };
  Check(nativeGoods.size()==4 && variable(nativeGoods,"ironroad_good_work_grain")=="50.00" &&
        variable(nativeGoods,"ironroad_good_share_grain")=="71.4%",
        "native row variables use weighted order-distance and a whole-market percentage");
  auto stateFilter=filter;stateFilter.kind=FreightViewKind::State;stateFilter.state=10;
  auto stateDocument=BuildLogisticsDocument(snapshot,allocated,stateFilter);
  const auto stateRows=FormatStateFreightGoods(&stateDocument,10);
  Check(stateDocument.kind==FreightViewKind::State && stateDocument.state==10 && stateRows.size()==2 &&
        variable(stateRows,"ironroad_state_work_10_grain")=="50.00" &&
        variable(stateRows,"ironroad_state_work_10_tools")=="20.00",
        "native state rows publish final input/output work under state and good identity");
  auto largeWork=stateDocument;largeWork.allFreight=2000000;
  largeWork.goods[0].freight=1234.56;largeWork.goods[1].freight=1000000;
  const auto compactRows=FormatStateFreightGoods(&largeWork,10);
  Check(variable(compactRows,"ironroad_state_work_10_grain")=="1.2k" &&
        variable(compactRows,"ironroad_state_work_10_tools")=="1.0M" &&
        variable(FormatFreightGoods(&largeWork),"ironroad_good_work_grain")=="1234.56",
        "narrow state column abbreviates large work without changing market precision or model values");
  Check(FormatStateFreightGoods(&document,10).empty() && FormatStateFreightGoods(&stateDocument,20).empty(),
        "market totals and another state cannot enter the state column");
  stateFilter.hoverGood=2;
  auto stateHover=BuildLogisticsDocument(snapshot,allocated,stateFilter);
  Check(FormatStateFreightGoods(&stateHover,10)==stateRows,"state hover never changes row work");
  auto invalidState=stateDocument;invalidState.states.clear();
  Check(FormatStateFreightGoods(&invalidState,10).empty(),"unknown state cannot fabricate zero rows");
  invalidState=stateDocument;invalidState.states.push_back(invalidState.states[0]);
  Check(FormatStateFreightGoods(&invalidState,10).empty(),"ambiguous state roster is refused");
  invalidState=stateDocument;invalidState.market=8;
  Check(FormatStateFreightGoods(&invalidState,10).empty(),"state must belong to document market");
  invalidState=stateDocument;invalidState.synthetic=true;
  Check(FormatStateFreightGoods(&invalidState,10).empty(),"synthetic state work stays out of native rows");
  invalidState=stateDocument;invalidState.revision=0;
  Check(FormatStateFreightGoods(&invalidState,10).empty(),"unversioned state rows remain unavailable");
  invalidState=stateDocument;invalidState.allFreight=0;
  for(auto& good:invalidState.goods)good.freight=0;
  Check(variable(FormatStateFreightGoods(&invalidState,10),"ironroad_state_work_10_grain")=="0.00",
        "known zero state work stays distinct from unavailable");
  auto zeroState=snapshot;for(auto& orders:zeroState.orders)if(orders.state==10)orders.state=0;
  auto zeroStateResult=AllocateFreight(zeroState,graph,{});stateFilter.hoverGood=-1;stateFilter.state=0;
  auto zeroStateDocument=BuildLogisticsDocument(zeroState,zeroStateResult,stateFilter);
  Check(variable(FormatStateFreightGoods(&zeroStateDocument,0),"ironroad_state_work_0_grain")=="50.00",
        "explicit state zero has its own canonical keys");
  for(auto& orders:zeroState.orders)if(orders.state==0)orders.state=0x80000011;
  zeroStateResult=AllocateFreight(zeroState,graph,{});stateFilter.state=0x80000011;
  zeroStateDocument=BuildLogisticsDocument(zeroState,zeroStateResult,stateFilter);
  Check(variable(FormatStateFreightGoods(&zeroStateDocument,0x80000011),"ironroad_state_work_-2147483631_grain")=="50.00",
        "state keys match native GetIDString signed decimal formatting for high-bit IDs");
  Check(FormatFreightGoods(&selected)==nativeGoods && FormatFreightGoods(&hoverDocument)==nativeGoods,
        "native goods values and shares stay fixed across category selection and temporary hover");
  auto reordered=document;std::reverse(reordered.goods.begin(),reordered.goods.end());
  Check(FormatFreightGoods(&reordered)==nativeGoods,"sorting goods cannot trigger a different native variable payload");
  Check(FormatFreightGoods(nullptr).empty(),"missing native goods input clears rather than fabricates rows");
  auto invalidGoods=document;invalidGoods.synthetic=true;
  Check(FormatFreightGoods(&invalidGoods).empty(),"synthetic rows cannot be published as native game data");
  invalidGoods=document;invalidGoods.goods.push_back(invalidGoods.goods[0]);
  Check(FormatFreightGoods(&invalidGoods).empty(),"ambiguous duplicate keys never select a first goods row");
  invalidGoods=document;invalidGoods.goods[0].key="grain')";
  Check(FormatFreightGoods(&invalidGoods).empty(),"invalid native variable goods keys are rejected");
  invalidGoods=document;invalidGoods.goods[0].freight=INFINITY;
  Check(variable(FormatFreightGoods(&invalidGoods),"ironroad_good_work_grain")=="--" &&
        variable(FormatFreightGoods(&invalidGoods),"ironroad_good_share_grain")=="--",
        "invalid goods work produces unavailable cells, never NaN or a fake zero");
  invalidGoods=document;invalidGoods.allFreight=0;
  for(auto& good:invalidGoods.goods)good.freight=0;
  const auto zeroGoods=FormatFreightGoods(&invalidGoods);
  Check(variable(zeroGoods,"ironroad_good_work_grain")=="0.00" && variable(zeroGoods,"ironroad_good_share_grain")=="--",
        "known zero work differs from an undefined zero-market percentage");
  Check(selected.intermediaryDemand->target==document.intermediaryDemand->target,
        "presentation filters cannot change Freight intermediary demand");
  Check(FormatFreightDemand(&document)=="11.00 / 7.00 / 4.00",
        "native variable displays intermediary units, not order-distance work");
  Check(FormatFreightDemand(nullptr)=="-- / -- / --","missing native document stays unavailable");
  auto hiddenDemand=document;hiddenDemand.synthetic=true;
  Check(FormatFreightDemand(&hiddenDemand)=="-- / -- / --","synthetic Freight cannot enter native variables");
  hiddenDemand=document;hiddenDemand.intermediaryDemand.reset();
  Check(FormatFreightDemand(&hiddenDemand)=="-- / -- / --","legacy documents cannot fabricate zero demand");
  hiddenDemand=document;hiddenDemand.intermediaryDemand=FreightDemand{};
  Check(FormatFreightDemand(&hiddenDemand)=="0.00 / 0.00 / 0.00","known zero demand remains distinguishable from missing");
  hiddenDemand=document;hiddenDemand.intermediaryDemand->fulfilled=INFINITY;
  Check(FormatFreightDemand(&hiddenDemand)=="-- / -- / --","invalid demand is not shown as live data");
  Check(std::abs(selected.allFreight-70)<1.e-6 && std::abs(selected.filteredFreight-50)<1.e-6 &&
        std::abs(selected.goods[0].share-5./7)<1.e-6,"category freight updates while share denominator remains all market freight");
  selection.SetCategory(GoodsCategory::Industrial);
  Check(BuildFreightView(snapshot,allocated,selection.Filter()).segments.size()==1,"combined categories retain shared physical edge");
  auto stale=snapshot;stale.world=2;
  Check(BuildLogisticsDocument(stale,allocated,filter).world==0,"world changes reject stale UI data");
  filter.search="wheat";
  Check(BuildFreightView(snapshot,allocated,filter).filteredFreight==50,"map filter and list use the same localized search");

  ImGui::CreateContext();auto& io=ImGui::GetIO();io.IniFilename=nullptr;
  io.DisplaySize=ImVec2(760,1100);io.DeltaTime=1.f/60;
  unsigned char* pixels;int w,h;io.Fonts->GetTexDataAsRGBA32(&pixels,&w,&h);
  LogisticsPanel panel;
  auto frame=[&](const LogisticsDocument* doc) {
    ImGui::NewFrame();ImGui::SetNextWindowSize(ImVec2(730,1050));ImGui::SetNextWindowPos(ImVec2(5,5));
    ImGui::Begin("UI verification");panel.Draw(doc,FreightViewKind::Market);ImGui::End();ImGui::Render();
    Check(ImGui::GetDrawData()->TotalVtxCount>0,"actual ImGui panel generates draw data");
  };
  frame(nullptr);frame(&document);panel.RequestGood(1);frame(&document);
  Check(panel.Selection().selectedGood==1,"details open by stable goods ID");
  std::reverse(document.goods.begin(),document.goods.end());frame(&document);
  Check(panel.Selection().selectedGood==1,"sorting/refresh cannot select a different good by row index");
  document.world=2;frame(&document);Check(panel.Selection().selectedGood==-1,"world replacement closes stale detail");
  ImGui::DestroyContext();
  std::puts("PASS: logistics categories/search/show-hide/back/hover, copied prices/orders/shares, unknown/stale data and actual ImGui list/detail rendering");
}
