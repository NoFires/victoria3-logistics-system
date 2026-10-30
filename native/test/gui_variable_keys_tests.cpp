// Exercise the production private map decoder without invoking a game getter.
// Including the implementation keeps this raw-layout test seam out of the API.
#ifndef V3_GUI_PROBE_SOURCE
#define V3_GUI_PROBE_SOURCE "../src/variable_probe.cpp"
#endif
#include V3_GUI_PROBE_SOURCE
#include <array>
#include <chrono>
#include <cstdlib>
using namespace v3;
void Check(bool ok,const char* why){if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
int main(){
  constexpr size_t count=4096;
  std::vector<std::string> names,values;names.reserve(count);values.reserve(count);
  for(size_t i=0;i<count;++i){names.push_back("unrelated_gui_variable_"+std::to_string(i));values.push_back("unrelated_gui_value_"+std::to_string(i));}
  names[5]="ironroad_market_hover_grain";names[50]="ironroad_market_hover_clothes";
  values[5]=values[50]="ironroad-hover-1-9-0";
  std::vector<std::array<uint8_t,0x48>> entries(count);
  std::array<uint8_t,0x28> header{};
  auto put=[](auto& data,size_t offset,auto value){std::memcpy(data.data()+offset,&value,sizeof(value));};
  auto stringAt=[&](size_t index,size_t offset,const std::string& text){
    MsvcStringStorage s{};
    if(text.size()<16)InitSso(&s,text);
    else {const auto p=text.data();std::memcpy(s.data,&p,sizeof(p));s.size=s.capacity=text.size();}
    std::memcpy(entries[index].data()+offset,&s,sizeof(s));
  };
  for(size_t i=0;i<count;++i){
    put(entries[i],0,Murmur3(names[i].data(),names[i].size()));entries[i][4]=1;
    stringAt(i,8,names[i]);stringAt(i,0x28,values[i]);
  }
  put(header,8,entries.data());put(header,0x10,int32_t(count));put(header,0x14,uint32_t(count-1));
  put(header,0x1c,float(.75));
  // Distinct full keys can share a stored hash. Only hash-prefiltering is unsafe.
  put(entries[17],0,Murmur3(names[5].data(),names[5].size()));
  const std::vector<std::string> wanted{names[5],names[50]};
  auto filtered=[&](const std::vector<std::string>& keys){
    GuiVariableProbe p;
#ifdef V3_LEGACY_FULL_GUI_PROBE
    ReadMapObject(header.data(),&p);
#else
    ReadMapObject(header.data(),&p,&keys);
#endif
    return p;
  };
  GuiVariableProbe full;Check(ReadMapObject(header.data(),&full) && full.entries.size()==count,"full diagnostic enumeration retained");
  auto result=filtered(wanted);
  std::printf("GUI probe copied full=%zu selected=%zu\n",full.entries.size(),result.entries.size());
  Check(result.ok && result.count==int32_t(count) && result.mask==count-1 && result.entries.size()==2,
        "formal poll copies only requested exact keys instead of every unrelated variable");
  Check(result.entries[0].key==names[5] && result.entries[1].key==names[50] && result.entries[0].value==values[5],
        "bucket order, complete values and distinct colliding keys preserved");
  Check(filtered({names[5],names[5]}).entries.size()==1,"duplicate requests do not duplicate ambiguous hover rows");
  Check(filtered({}).ok && filtered({}).entries.empty(),"empty request is known empty, not full enumeration");
  Check(filtered({"ironroad_hide_flows"}).ok && filtered({"ironroad_hide_flows"}).entries.empty(),"missing key is known absent");
  Check(!filtered({""}).ok && !filtered({std::string(257,'x')}).ok && !filtered(std::vector<std::string>(4097,"x")).ok,
        "invalid or unbounded requested keys rejected");
  const auto original=entries[5];
  put(entries[5],0x40,uint64_t(0x10001));
  Check(filtered(wanted).entries.size()==1,"unreadable/invalid matching value cannot become a live hover");entries[5]=original;
  std::string longKey(257,'x');stringAt(5,8,longKey);put(entries[5],0,Murmur3(longKey.data(),256));
  Check(filtered({longKey.substr(0,256)}).entries.empty(),"truncated diagnostic key cannot impersonate an exact requested key");entries[5]=original;
  values[5]="short";stringAt(5,0x28,values[5]);
  Check(filtered(wanted).entries[0].value=="short","SSO values use the same verified layout");
  auto invalid=header;
  put(header,0x10,int32_t(-1));Check(!filtered(wanted).ok,"negative map count rejected");header=invalid;
  put(header,0x14,uint32_t(100001));Check(!filtered(wanted).ok,"excess bucket range rejected");header=invalid;
  put(header,8,uintptr_t(1));Check(!filtered(wanted).ok,"unreadable entry array rejected");header=invalid;
#ifndef V3_LEGACY_FULL_GUI_PROBE
  Check(!ProbeGuiVariableKeys(nullptr,wanted).ok,"missing game base does not invoke a getter");
#endif
  const auto beforeHeader=header;
  const auto beforeEntries=entries;
  auto start=std::chrono::steady_clock::now();
  for(unsigned i=0;i<20;++i){GuiVariableProbe p;Check(ReadMapObject(header.data(),&p),"full benchmark decoder");}
  const double fullMs=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/20;
  start=std::chrono::steady_clock::now();
  for(unsigned i=0;i<100;++i)Check(filtered(wanted).entries.size()==2,"filtered benchmark remains exact");
  const double selectedMs=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count()/100;
  Check(header==beforeHeader && entries==beforeEntries,"read-only probe leaves source bytes unchanged");
  std::printf("GUI probe 4096-entry fixture full=%.4f ms filtered=%.4f ms (diagnostic benchmark, not game FPS)\n",fullMs,selectedMs);
  std::puts("PASS: exact GUI key selection, hash collisions, order, missing/SSO/truncated/invalid keys and read-only bounds");
}
