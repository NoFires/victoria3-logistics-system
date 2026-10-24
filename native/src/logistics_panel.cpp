#include "logistics_panel.h"
#include <imgui.h>
#include <algorithm>
#include <cstdio>
#include <cmath>

namespace v3 {
namespace {
constexpr ImVec4 gold{.85f,.72f,.46f,1},muted{.62f,.67f,.7f,1};
constexpr ImVec4 positive{.35f,.75f,.79f,1},negative{1.f,.58f,.37f,1};
std::string Number(double value) {
  char buffer[48];const double magnitude=std::abs(value);
  if (magnitude>=1.e6) std::snprintf(buffer,sizeof(buffer),"%.2fM",value/1.e6);
  else if (magnitude>=1.e3) std::snprintf(buffer,sizeof(buffer),"%.2fk",value/1.e3);
  else std::snprintf(buffer,sizeof(buffer),"%.2f",value);
  return buffer;
}
void Header(const char* title) {
  ImGui::Spacing();
  ImGui::PushStyleColor(ImGuiCol_ChildBg,ImVec4(.09f,.15f,.19f,1));
  ImGui::BeginChild(title,ImVec2(0,29),ImGuiChildFlags_Border);
  ImGui::TextColored(gold,"%s",title);
  ImGui::EndChild();ImGui::PopStyleColor();ImGui::Spacing();
}
bool Choice(const char* label,bool selected,ImVec2 size=ImVec2(0,0)) {
  if (selected) {
    ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(.34f,.29f,.18f,1));
    ImGui::PushStyleColor(ImGuiCol_Border,gold);
  }
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize,1);
  const bool clicked=ImGui::Button(label,size);
  ImGui::PopStyleVar();if (selected) ImGui::PopStyleColor(2);
  return clicked;
}
void Card(const char* label,const std::string& value,float width) {
  ImGui::BeginChild(label,ImVec2(width,78),ImGuiChildFlags_Border);
  ImGui::TextColored(muted,"%s",label);ImGui::Spacing();
  ImGui::TextColored(ImVec4(.95f,.91f,.81f,1),"%s",value.c_str());ImGui::EndChild();
}
void Price(const LogisticsGood& row) {
  if (row.price) ImGui::Text("\xC2\xA3 %.2f",*row.price); else ImGui::TextDisabled("--");
}
void Deviation(const LogisticsGood& row) {
  if (row.priceDeviation) {
    const float fraction=float(std::clamp(std::abs(*row.priceDeviation),0.0,1.0));
    ImGui::TextColored(*row.priceDeviation>0?negative:positive,"%+.1f%%",*row.priceDeviation*100);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram,*row.priceDeviation>0?negative:positive);
    ImGui::ProgressBar(fraction,ImVec2(-1,4),"");ImGui::PopStyleColor();
  } else ImGui::TextDisabled("--");
}
void OrderList(const char* title,double total,const std::vector<LogisticsOrders>& rows) {
  ImGui::TextColored(gold,"%s",title);ImGui::SameLine();ImGui::Text("%s",Number(total).c_str());
  ImGui::Separator();
  for (const auto& row:rows) {
    ImGui::TextUnformatted(row.name.c_str());
    ImGui::SameLine(std::max(100.f,ImGui::GetContentRegionAvail().x-72));
    ImGui::Text("%s",Number(row.quantity).c_str());
  }
  if (rows.empty()) ImGui::TextDisabled("No orders");
}
} // namespace

void LogisticsPanel::DrawFilters() {
  Header("Filters");
  ImGui::SetNextItemWidth(-1);
  if (ImGui::InputTextWithHint("##GoodsSearch","Search goods...",m_search,sizeof(m_search))) {
    m_selection.Search(m_search);m_changed=true;
  }
  ImGui::Spacing();ImGui::TextColored(muted,"Goods category");ImGui::SameLine();
  if (ImGui::SmallButton("All goods")) {m_selection.AllCategories();m_changed=true;}
  static constexpr const char* labels[]={"Staple","Industrial","Luxury","Military"};
  const float width=(ImGui::GetContentRegionAvail().x-ImGui::GetStyle().ItemSpacing.x*3)/4;
  for (unsigned i=0;i<4;++i) {
    if (i) ImGui::SameLine();
    if (Choice(labels[i],m_selection.categories&(1<<i),ImVec2(width,36))) {
      m_selection.SetCategory(static_cast<GoodsCategory>(1<<i));m_changed=true;
    }
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show/hide %s goods. First choice selects that category; combine with other categories.",labels[i]);
  }
  ImGui::Spacing();ImGui::TextColored(muted,"Freight flows");
  const float half=(ImGui::GetContentRegionAvail().x-ImGui::GetStyle().ItemSpacing.x)/2;
  if (Choice("Show",m_selection.flowsVisible,ImVec2(half,31))) {m_selection.flowsVisible=true;m_changed=true;}
  ImGui::SameLine();
  if (Choice("Hide",!m_selection.flowsVisible,ImVec2(half,31))) {m_selection.flowsVisible=false;m_changed=true;}
}

void LogisticsPanel::DrawGoods(const LogisticsDocument& doc) {
  Header("Goods");
  auto flags=ImGuiTableFlags_RowBg|ImGuiTableFlags_BordersInnerH|ImGuiTableFlags_Resizable|
      ImGuiTableFlags_Sortable|ImGuiTableFlags_ScrollY;
  if (!ImGui::BeginTable("##LogisticsGoods",5,flags,ImVec2(0,std::max(160.f,ImGui::GetContentRegionAvail().y-70)))) return;
  ImGui::TableSetupColumn("Goods",ImGuiTableColumnFlags_WidthStretch,1.5f,0);
  ImGui::TableSetupColumn("Market price",ImGuiTableColumnFlags_WidthFixed,90,1);
  ImGui::TableSetupColumn("vs base",ImGuiTableColumnFlags_WidthFixed,65,2);
  ImGui::TableSetupColumn("Freight",ImGuiTableColumnFlags_WidthFixed|ImGuiTableColumnFlags_DefaultSort|ImGuiTableColumnFlags_PreferSortDescending,80,3);
  ImGui::TableSetupColumn("% all freight",ImGuiTableColumnFlags_WidthFixed,85,4);
  ImGui::TableSetupScrollFreeze(0,1);ImGui::TableHeadersRow();
  if (auto* sort=ImGui::TableGetSortSpecs();sort && sort->SpecsCount) {
    m_sortColumn=sort->Specs[0].ColumnUserID;
    m_sortDescending=sort->Specs[0].SortDirection==ImGuiSortDirection_Descending;
    sort->SpecsDirty=false;
  }
  std::vector<const LogisticsGood*> rows;
  for (const auto& row:doc.goods) if (m_selection.Matches(row)) rows.push_back(&row);
  std::stable_sort(rows.begin(),rows.end(),[this](const auto* a,const auto* b) {
    if (m_sortColumn==0) return m_sortDescending?a->name>b->name:a->name<b->name;
    auto value=[this](const LogisticsGood* row)->std::optional<double> {
      if (m_sortColumn==1) return row->price;
      if (m_sortColumn==2) return row->priceDeviation;
      return m_sortColumn==4?row->share:row->freight;
    };
    const auto av=value(a),bv=value(b);
    if (av.has_value()!=bv.has_value()) return av.has_value();
    if (av && bv && *av!=*bv) return m_sortDescending?*av>*bv:*av<*bv;
    return a->id<b->id;
  });
  for (const auto* row:rows) {
    ImGui::PushID(int(row->id));ImGui::TableNextRow(0,42);ImGui::TableSetColumnIndex(0);
    if (ImGui::Selectable(row->name.c_str(),false,ImGuiSelectableFlags_SpanAllColumns,ImVec2(0,30))) {
      m_selection.OpenGood(row->id);m_changed=true;
    }
    if (ImGui::IsItemHovered()) {
      m_hover=row->id;
      ImGui::SetTooltip("%s\nFreight %s (%.2f%% of all market freight)\nHover: show this good's routes. Click: open details.",
          row->name.c_str(),Number(row->freight).c_str(),row->share*100);
    }
    ImGui::TableSetColumnIndex(1);Price(*row);
    ImGui::TableSetColumnIndex(2);Deviation(*row);
    ImGui::TableSetColumnIndex(3);ImGui::TextUnformatted(Number(row->freight).c_str());
    ImGui::TableSetColumnIndex(4);ImGui::Text("%.2f%%",row->share*100);ImGui::PopID();
  }
  ImGui::EndTable();
  if (rows.empty()) ImGui::TextDisabled("No goods match these filters.");
}

void LogisticsPanel::DrawDetail(const LogisticsDocument&,const LogisticsGood& row) {
  if (ImGui::Button("< Logistics")) {m_selection.Back();m_changed=true;}
  ImGui::SameLine();ImGui::TextColored(gold,"%s",row.name.c_str());
  Header("Market details");
  Price(row);ImGui::SameLine(150);Deviation(row);ImGui::Spacing();
  ImGui::Text("Freight %s    %.2f%% of all market freight",Number(row.freight).c_str(),row.share*100);
  ImGui::Text("Sell orders %s    Buy orders %s",Number(row.sellOrders).c_str(),Number(row.buyOrders).c_str());
  ImGui::TextDisabled("Orders are game quantities. Routed quantities are model allocations.");
  Header("Supply and consumption by state");
  if (ImGui::BeginTable("##GoodOrders",2,ImGuiTableFlags_Resizable|ImGuiTableFlags_BordersInnerV)) {
    ImGui::TableNextColumn();OrderList("Sold by",row.sellOrders,row.sellers);
    ImGui::TableNextColumn();OrderList("Bought by",row.buyOrders,row.buyers);ImGui::EndTable();
  }
}

bool LogisticsPanel::Draw(const LogisticsDocument* doc,FreightViewKind kind) {
  m_changed=false;m_hover=-1;
  if (m_selection.kind!=kind) {m_selection.kind=kind;m_changed=true;m_selection.Back();}
  const bool ready=doc && doc->world;
  if (ready && doc->world!=m_world) {
    m_world=doc->world;m_selection.Back();m_selection.market=doc->market;m_selection.state=0;m_changed=true;
  }
  if (ready && m_pendingGood) {
    m_selection.OpenGood(*m_pendingGood);m_pendingGood.reset();m_changed=true;
  }
  ImGui::TextColored(gold,kind==FreightViewKind::Market?"MARKET LOGISTICS":"STATE IMPORTS / EXPORTS");
  if (ready && doc->synthetic) {
    ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(1.f,.55f,.35f,1));
    ImGui::TextWrapped("TEST INPUT: synthetic snapshot generated by the debug panel, not a live game reading.");
    ImGui::PopStyleColor();
  }
  const LogisticsMarket* market=nullptr;
  if (ready) for (const auto& candidate:doc->markets) if (candidate.id==m_selection.market) market=&candidate;
  ImGui::SetNextItemWidth(-1);ImGui::BeginDisabled(!ready);
  if (ImGui::BeginCombo("##LogisticsMarket",market?market->name.c_str():"Waiting for market snapshot")) {
    for (const auto& candidate:doc->markets)
      if (ImGui::Selectable(candidate.name.c_str(),candidate.id==m_selection.market)) {
        m_selection.SelectMarket(candidate.id);m_changed=true;
      }
    ImGui::EndCombo();
  }
  if (kind==FreightViewKind::State) {
    const LogisticsState* state=nullptr;
    if (ready) for (const auto& candidate:doc->states) if (candidate.id==m_selection.state && candidate.market==m_selection.market) state=&candidate;
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##LogisticsState",state?state->name.c_str():"Select state")) {
      for (const auto& candidate:doc->states) if (candidate.market==m_selection.market)
        if (ImGui::Selectable(candidate.name.c_str(),candidate.id==m_selection.state)) {
          m_selection.SelectState(candidate.id);m_changed=true;
        }
      ImGui::EndCombo();
    }
  }
  ImGui::EndDisabled();
  const LogisticsGood* detail=nullptr;
  if (ready) for (const auto& row:doc->goods) if (row.id==m_selection.selectedGood) detail=&row;
  if (detail) DrawDetail(*doc,*detail);
  else {
    if (m_selection.selectedGood>=0) {m_selection.Back();m_changed=true;}
    DrawFilters();Header("Logistics");
    const float half=(ImGui::GetContentRegionAvail().x-ImGui::GetStyle().ItemSpacing.x)/2;
    Card("Freight",ready?Number(doc->filteredFreight):"--",half);ImGui::SameLine();
    Card("Laid track",!ready?"--":(doc->railKilometres?Number(*doc->railKilometres)+" km":Number(doc->builtRailLength)+" map units"),half);
    if (ready) {
      ImGui::Text("Target import %s   Allocated %s   Unmet %s  (orders, model)",
                  Number(doc->targetImport).c_str(),Number(doc->allocatedImport).c_str(),
                  Number(doc->unmetDemand).c_str());
      if (doc->unmetDemand>0)
        ImGui::TextDisabled("Unmet demand is shown separately, not drawn as transported freight.");
    }
    if (ready) DrawGoods(*doc);
    else {
      Header("Goods");
      ImGui::TextWrapped("Waiting for a verified economy snapshot. Prices, quantities and routes will appear here when live data is connected.");
      ImGui::TextDisabled("Freight  |  Market price  |  %% of all market freight");
    }
  }
  if (m_selection.hoverGood!=m_hover) {m_selection.hoverGood=m_hover;m_changed=true;}
  ImGui::Spacing();
  ImGui::TextDisabled("Width: allocated quantity. Share: all market freight.");
  if (kind==FreightViewKind::Market) {
    const ImVec4 colours[]={{.08f,.85f,.35f,1},{.55f,.9f,.12f,1},{1,.9f,.05f,1},{1,.53f,.03f,1},{.95f,.04f,.12f,1}};
    for (int i=0;i<5;++i) {if(i) ImGui::SameLine();ImGui::ColorButton(("##Intensity"+std::to_string(i)).c_str(),colours[i],ImGuiColorEditFlags_NoTooltip,ImVec2(25,9));}
    ImGui::SameLine();ImGui::TextDisabled("Low -> high throughput");
    if (ready) {
      if (doc->heatReference>0)
        ImGui::TextDisabled("Reference %.0f weighted orders (heat statistic rev %u)",
                            doc->heatReference,doc->heatScaleRevision);
      else
        ImGui::TextDisabled("No freight in this market; no heat scale.");
    }
  } else ImGui::TextDisabled("Green: incoming. Orange: outgoing.");
  return m_changed;
}
} // namespace v3
