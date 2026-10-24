#include "native_gui.h"
#include "stored_zip.h"
#include "util.h"
#include <algorithm>
#include <cstring>
#include <filesystem>
#include <map>
#include <stdexcept>

namespace v3 {
namespace {
std::map<size_t,size_t> Braces(const std::string& text) {
  std::map<size_t,size_t> pairs;std::vector<size_t> stack;
  bool quote=false,comment=false,escaped=false;
  for(size_t i=0;i<text.size();++i) {
    const char c=text[i];
    if(comment){if(c=='\n')comment=false;}
    else if(quote){if(escaped)escaped=false;else if(c=='\\')escaped=true;else if(c=='"')quote=false;}
    else if(c=='#')comment=true;
    else if(c=='"')quote=true;
    else if(c=='{')stack.push_back(i);
    else if(c=='}'){
      if(stack.empty())throw std::runtime_error("unbalanced native GUI brace");
      pairs.emplace(stack.back(),i);stack.pop_back();
    }
  }
  if(quote || !stack.empty())throw std::runtime_error("unbalanced native GUI block/string");
  return pairs;
}
size_t Unique(const std::string& text,const std::string& anchor,size_t begin=0,size_t end=std::string::npos) {
  const auto index=text.find(anchor,begin);
  if(index==std::string::npos || index>=end || text.find(anchor,index+anchor.size())<end)
    throw std::runtime_error("missing/ambiguous native GUI anchor: "+anchor);
  return index;
}
std::pair<size_t,size_t> Block(const std::string& text,const std::string& anchor,size_t begin=0,size_t end=std::string::npos) {
  const auto at=Unique(text,anchor,begin,end),opening=text.find('{',at+anchor.size());
  if(opening==std::string::npos || opening>=end)throw std::runtime_error("native GUI block missing");
  const auto pairs=Braces(text);const auto found=pairs.find(opening);
  if(found==pairs.end() || found->second>=end)throw std::runtime_error("native GUI block escapes parent");
  return {opening,found->second};
}
std::vector<uint8_t> Resource(HMODULE self,int id) {
  const auto resource=FindResourceW(self,MAKEINTRESOURCEW(id),RT_RCDATA);
  const auto loaded=resource?LoadResource(self,resource):nullptr;
  const auto* bytes=loaded?static_cast<const uint8_t*>(LockResource(loaded)):nullptr;
  const auto size=resource?SizeofResource(self,resource):0;
  if(!bytes || !size)throw std::runtime_error("native GUI resource missing: "+std::to_string(id));
  return {bytes,bytes+size};
}
} // namespace

bool PatchNativeMarketGui(const std::vector<uint8_t>& original,const std::string& tabOverride,
                         std::string* patched,std::string* error) {
  if(patched)patched->clear();
  try {
    if(!patched)throw std::runtime_error("native GUI output is missing");
    if(Sha256Bytes(original.data(),original.size())!=kMarketGuiHash)
      throw std::runtime_error("market_panel.gui differs from verified 1.13.11; native UI patch disabled");
    if(tabOverride.empty())throw std::runtime_error("native tab override is empty");
    std::string text(original.begin(),original.end());
    if(text.starts_with("\xef\xbb\xbf"))text.erase(0,3);
    const auto top=Block(text,"blockoverride \"fixed_top\"");
    const auto tabs=Block(text,"tab_buttons =",top.first,top.second);
    text.insert(tabs.second,tabOverride);
    const std::string old="[Or(InformationPanel.IsTabSelected('default'), InformationPanel.IsTabSelected('world_market_trade'))]";
    const auto filter=Unique(text,old);
    text.replace(filter,old.size(),"[Or(InformationPanel.IsTabSelected('ironroad_logistics'), "+old.substr(1,old.size()-2)+")]");
    const auto at=Unique(text,"\t\t\t\tmarket_panel_details_content = {");
    text.insert(at,"                container = {\n"
                   "                    visible = \"[InformationPanel.IsTabSelected('ironroad_logistics')]\"\n"
                   "                    using = default_fade_in_out\n"
                   "                    ironroad_market_logistics_content = {}\n"
                   "                }\n");
    // Keep only the balance column in the logistics tab: the two dependence
    // columns stay in the vanilla tabs but hide while our tab is selected.
    for (const char* key : {"MARKET_GOODS_BALANCE_TOOLTIP", "MARKET_GOODS_IMPORT_DEPENDENCE", "MARKET_GOODS_EXPORT_DEPENDENCE"}) {
      const auto pos=text.find(std::string("tooltip = \"")+key+"\"");
      if(pos==std::string::npos)throw std::runtime_error(std::string("missing native dependence column: ")+key);
      const auto container=text.rfind("\n\t\t\t\tflowcontainer = {",pos);
      if(container==std::string::npos)throw std::runtime_error(std::string("ambiguous native dependence container: ")+key);
      const auto insertAt=text.find('{',container)+1;
      text.insert(insertAt,"\n\t\t\t\t\tvisible = \"[Not(InformationPanel.IsTabSelected('ironroad_logistics'))]\"");
    }
    Braces(text);
    *patched=std::move(text);return true;
  } catch(const std::exception& ex){if(error)*error=ex.what();return false;}
}

bool PatchNativeStateGui(const std::vector<uint8_t>& original,std::string* patched,std::string* error) {
  if(patched)patched->clear();
  try {
    if(!patched)throw std::runtime_error("native state GUI output is missing");
    if(Sha256Bytes(original.data(),original.size())!=kStateGuiHash)
      throw std::runtime_error("states_panel.gui differs from verified 1.13.11; native state UI patch disabled");
    std::string text(original.begin(),original.end());
    if(text.starts_with("\xef\xbb\xbf"))text.erase(0,3);
    const std::string marker="visible =  \"[InformationPanel.IsTabSelected('local_goods')]\"";
    const auto at=Unique(text,marker),start=text.rfind("flowcontainer = {",at);
    if(start==std::string::npos)throw std::runtime_error("state local goods container missing");
    const auto end=Braces(text).at(text.find('{',start));
    std::string section=text.substr(start,end-start+1);
    const auto headerStart=section.find("flowcontainer = {",section.find(marker)+marker.size());
    const auto headerEnd=Braces(section).at(section.find('{',headerStart));
    const auto model=Unique(section,"datamodel = \"[StatesPanel.AccessLocalGoods]\"");
    const auto rowStart=section.find("button = {",model);
    const auto rowEnd=Braces(section).at(section.find('{',rowStart));
    std::string row=section.substr(rowStart,rowEnd-rowStart+1);
    const auto content=row.find("flowcontainer = {");
    const auto contentEnd=Braces(row).at(row.find('{',content));
    row.insert(contentEnd,"ironroad_state_freight_cell = {}\n");
    auto replace=[](std::string& value,const std::string& from,const std::string& to){
      size_t pos=0;
      while((pos=value.find(from,pos))!=std::string::npos){value.replace(pos,from.size(),to);pos+=to.size();}
    };
    // Account for the inherited 55px goods icon and fix auto-sized price cells.
    replace(row,"60 23","50 23");replace(row,"90 52","65 50");
    replace(row,"70 40","80 40");
    replace(row,"margin_right = 3","margin_right = 0");replace(row,"margin_left = 3","margin_left = 0");
    const auto balanceAt=Unique(row,"tooltip = \"STATE_GOODS_BALANCE_TOOLTIP\"");
    const auto balanceStart=row.rfind("flowcontainer = {",balanceAt);
    const auto balanceEnd=Braces(row).at(row.find('{',balanceStart));
    std::string balance=row.substr(balanceStart,balanceEnd-balanceStart+1);
    replace(balance,"autoresize = yes","size = { 65 23 }");
    replace(balance,"margin_right = 15","margin_right = 0");replace(balance,"margin_left = 15","margin_left = 0");
    replace(balance,"align = right|nobaseline","align = hcenter|nobaseline");
    replace(balance,"align = left|nobaseline","align = hcenter|nobaseline");
    row.replace(balanceStart,balanceEnd-balanceStart+1,balance);
    for(const char* anchor:{"tooltip = \"[Goods.GetStatePriceDesc]\"","using = market_price_tooltip_with_graph"}) {
      const auto priceStart=row.rfind("\tcontainer = {",Unique(row,anchor))+1;
      const auto priceEnd=Braces(row).at(row.find('{',priceStart));
      std::string price=row.substr(priceStart,priceEnd-priceStart+1);
      price.replace(0,std::strlen("container = {"),"widget = {");
      replace(price,"minimumsize = { 100 -1 }","size = { 90 50 }");
      replace(price,"autoresize = yes","size = { 90 40 }\nusing = elide_fontsize_min");
      row.replace(priceStart,priceEnd-priceStart+1,price);
    }
    const auto diffStart=row.rfind("widget = {",Unique(row,"text = \"[Goods.GetStateToMarketPriceDiff|%0=-]\""));
    const auto diffEnd=Braces(row).at(row.find('{',diffStart));
    std::string diff=row.substr(diffStart,diffEnd-diffStart+1);
    replace(diff,"align = right|nobaseline","align = center");replace(diff,"margin_right = 10","margin_right = 0");
    row.replace(diffStart,diffEnd-diffStart+1,diff);
    std::string header=section.substr(headerStart,headerEnd-headerStart);
    const int oldWidths[]={55,55,60,90,100,100,80},widths[]={55,50,50,65,90,90,80};
    size_t cursor=0;
    for(size_t i=0;i<7;++i) {
      const auto from="size = { "+std::to_string(oldWidths[i])+" 20 }";
      const auto to="size = { "+std::to_string(widths[i])+" 20 }";
      const auto pos=header.find(from,cursor);
      if(pos==std::string::npos)throw std::runtime_error("state header column missing");
      header.replace(pos,from.size(),to);cursor=pos+to.size();
    }
    replace(header,"margin_right = 7","");
    replace(header,"sort_button = {","sort_button = {\nmargin_left = 0\nmargin_right = 0");
    replace(header,"text = \"concept_local_price\"","text = \"IRONROAD_LOCAL_PRICE\"\ntooltip = \"concept_local_price\"");
    replace(header,"text = \"concept_market_price\"","text = \"IRONROAD_MARKET_PRICE\"");
    replace(header,"text = \"PRICE_DIFFERENCE\"","text = \"IRONROAD_PRICE_DIFFERENCE\"");
    header+="sort_button = { align = hcenter|nobaseline text = \"IRONROAD_FREIGHT_COLUMN\" size = { 60 20 } margin_left = 0 margin_right = 0 tooltip = \"IRONROAD_STATE_WORK_TOOLTIP\" }\n";
    section.replace(rowStart,rowEnd-rowStart+1,row);
    section.replace(headerStart,headerEnd-headerStart,header);
    text.replace(start,end-start+1,section);
    Braces(text);*patched=std::move(text);return true;
  } catch(const std::exception& ex){if(error)*error=ex.what();return false;}
}

bool PrepareNativeGui(HMODULE self,const std::wstring& gameRoot,const std::wstring& cacheRoot,
                      std::wstring* archivePath,std::string* error) {
  if(archivePath)archivePath->clear();
  try {
    if(!archivePath)throw std::runtime_error("native GUI archive output is missing");
    std::vector<uint8_t> original;
    if(!ReadFileBytes(gameRoot+L"\\game\\gui\\market_panel.gui",&original))
      throw std::runtime_error("cannot read installed native market GUI");
    const auto tab=Resource(self,304);
    std::string patched,why;
    if(!PatchNativeMarketGui(original,std::string(tab.begin(),tab.end()),&patched,&why))throw std::runtime_error(why);
    std::vector<uint8_t> stateSource;
    std::string stateGui;
    if(!ReadFileBytes(gameRoot+L"\\game\\gui\\states_panel.gui",&stateSource))
      throw std::runtime_error("cannot read installed native state GUI");
    if(!PatchNativeStateGui(stateSource,&stateGui,&why))throw std::runtime_error(why);
    // Derive the vanilla price/graph block locally; never embed copied base GUI.
    std::string source(original.begin(),original.end());
    const auto row=Block(source,"type goods_entry_button =");
    const auto price=Block(source,"block \"last_column\"",row.first,row.second);
    const auto own=Resource(self,301);
    std::string logistics(own.begin(),own.end());
    const std::string marker="__IRONROAD_NATIVE_MARKET_PRICE__";
    logistics.replace(Unique(logistics,marker),marker.size(),source.substr(price.first+1,price.second-price.first-1));
    Braces(logistics);
    const auto zip=BuildStoredZip({
      {"gui/market_panel.gui",{patched.begin(),patched.end()}},
      {"gui/states_panel.gui",{stateGui.begin(),stateGui.end()}},
      {"gui/ironroad_logistics.gui",{logistics.begin(),logistics.end()}},
      {"localization/english/ironroad_l_english.yml",Resource(self,302)},
      {"localization/simp_chinese/ironroad_l_simp_chinese.yml",Resource(self,303)}});
    std::filesystem::create_directories(cacheRoot);
    const auto path=cacheRoot+L"\\native-gui-"+Widen(Sha256Bytes(zip.data(),zip.size()))+L".zip";
    std::vector<uint8_t> previous;
    if(!ReadFileBytes(path,&previous) || previous!=zip) {
      const auto temporary=path+L".tmp-"+std::to_wstring(GetCurrentProcessId());
      const auto file=CreateFileW(temporary.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
      DWORD written=0;
      bool ok=file!=INVALID_HANDLE_VALUE && WriteFile(file,zip.data(),DWORD(zip.size()),&written,nullptr) && written==zip.size();
      if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);
      if(ok)ok=MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
      if(!ok){DeleteFileW(temporary.c_str());throw std::runtime_error("cannot cache native GUI archive");}
    }
    *archivePath=path;return true;
  } catch(const std::exception& ex){if(error)*error=ex.what();return false;}
}
} // namespace v3
