#include "native_vfs.h"
#include <cstdio>
#include <cstdlib>
#include <map>

void require(bool value,const char* message){if(!value){std::printf("FAIL: %s\n",message);std::exit(1);}}
struct Memory {
  std::map<uint64_t,uint8_t> bytes;bool current=true,mutate=false;int headReads=0;
  uint64_t changeAt=0,changeTarget=0;bool changed=false;
  template<class T>void Put(uint64_t address,T value){
    auto* p=(uint8_t*)&value;for(size_t i=0;i<sizeof(value);++i)bytes[address+i]=p[i];
  }
  void String(uint64_t address,const std::string& value){
    for(size_t i=0;i<=value.size();++i)Put<uint8_t>(address+i,i==value.size()?0:value[i]);
  }
  void NativeString(uint64_t address,const std::string& value){
    for(unsigned i=0;i<32;++i)Put<uint8_t>(address+i,0);
    const bool heap=value.size()>15;const uint64_t text=heap?address+0x1000:address;
    if(heap)Put<uint64_t>(address,text);
    String(text,value);Put<uint64_t>(address+0x10,value.size());
    Put<uint64_t>(address+0x18,heap?value.size():15);
  }
  bool Read(uint64_t address,void* out,size_t size){
    for(size_t i=0;i<size;++i){auto it=bytes.find(address+i);if(it==bytes.end())return false;((uint8_t*)out)[i]=it->second;}
    if(address==0x20008 && ++headReads==1 && mutate)Put<uint64_t>(address,0x9999);
    if(address==changeAt && !changed){changed=true;Put<uint8_t>(changeTarget,bytes[changeTarget]^1);}
    return true;
  }
};
int main(){
  constexpr uint64_t base=0x10000000,fs=base+0x05144588;
  Memory good;good.Put<uint8_t>(fs+0x120,1);good.Put<uint64_t>(fs+0xb8,0x20000);
  good.Put<uint8_t>(0x200fa,1);good.Put<uint64_t>(0x20008,0x30000);
  good.Put<uint64_t>(fs+0x28,0x40000);
  for(unsigned offset:{0x48u,0x60u}){
    good.Put<uint64_t>(fs+offset,0);good.Put<uint32_t>(fs+offset+8,0);good.Put<uint32_t>(fs+offset+12,0);
  }
  for(unsigned n=0;n<2;++n){
    uint64_t node=0x30000+n*0x100,backend=0x50000+n*0x100,root=0x60000+n*0x100;
    std::array<uint64_t,7> item{0x90000+n*0x100,root,0,0,0,backend,n==0?0x30100u:0u};
    for(size_t j=0;j<item.size();++j)good.Put<uint64_t>(node+j*8,item[j]);
    good.String(root,n==0?"E:\\Mods\\改图":"E:/Game/base.zip");
    good.Put<uint64_t>(0x40000+n*8,root);
    good.Put<uint64_t>(backend+0x38,base+(n==0?0x03b33d80:0x03b40a50));
    good.Put<uint64_t>(backend+0x40,base+(n==0?0x03b33fe0:0x03b40cc0));
    good.Put<uint64_t>(backend+0x68,base+(n==0?0x03b34220:0x03b410e0));
  }
  good.Put<uint64_t>(0x40010,0);
  auto check=[&](Memory& memory,uint32_t context=0,uint64_t module=0x10000000){
    return v3::DecodeNativeVfsConfiguration([&](auto a,auto b,auto c){return memory.Read(a,b,c);},module,context,[&]{return memory.current;});
  };
  auto result=check(good);require(result && result->mounts.size()==2,"current frozen mounts copied");
  require(result->mounts[0].kind==v3::NativeVfsKind::Directory && result->mounts[1].kind==v3::NativeVfsKind::Zip,"backend functions identify directory and ZIP");
  require(result->mounts[0].root=="E:\\Mods\\改图","UTF-8 root retained in search order");
  require(result->replacements.empty() && result->exclusions.empty(),"empty rule tables are explicit");
  auto slashes=good;slashes.String(0x72000,"E:/Mods/改图");slashes.Put<uint64_t>(0x40000,0x72000);
  auto slashResult=check(slashes);
  require(slashResult && slashResult->searchPaths[0]=="E:/Mods/改图" && slashResult->mounts[0].root=="E:\\Mods\\改图",
      "native search-path bytes retained independently of normalized mount reconciliation");
  auto test=[&](auto change,const char* message){auto memory=good;change(memory);require(!check(memory),message);};
  test([&](auto& m){m.template Put<uint8_t>(fs+0x120,0);},"uninitialized context rejected");
  test([&](auto& m){m.template Put<uint8_t>(0x200fa,0);},"mutable mounts rejected");
  test([&](auto& m){m.template Put<uint64_t>(0x30130,0x30000);},"mount cycle rejected");
  test([&](auto& m){m.template Put<uint64_t>(0x50040,base);},"unknown backend rejected");
  test([&](auto& m){m.template Put<uint64_t>(0x40000,0x60100);},"wrong search priority rejected");
  test([&](auto& m){m.template Put<uint64_t>(0x40010,0x60100);},"extra search path rejected");
  test([&](auto& m){m.bytes.erase(0x60000);},"unreadable string rejected");
  test([&](auto& m){m.template Put<uint64_t>(0x30008,UINT64_MAX);m.template Put<uint8_t>(UINT64_MAX,'X');},"string pointer overflow rejected");
  test([&](auto& m){for(unsigned i=0;i<4096;++i)m.template Put<uint8_t>(0x60000+i,'X');},"unterminated path rejected");
  test([&](auto& m){m.template Put<uint64_t>(0x30020,1);},"null source prefix length rejected");
  test([&](auto& m){m.mutate=true;m.headReads=0;},"head mutation during capture rejected");
  test([&](auto& m){m.current=false;},"retired capture rejected");
  auto prefixes=good;prefixes.Put<uint64_t>(0x30010,0x70000);prefixes.String(0x70000,"map_data/");
  prefixes.Put<uint64_t>(0x30018,0x70100);prefixes.String(0x70100,"alternate/");prefixes.Put<uint64_t>(0x30020,11);
  require(bool(check(prefixes)),"mount point and source prefix copied");
  prefixes.String(0x70000,"map_data");require(!check(prefixes),"malformed mount point rejected");
  require(!check(good,16) && !check(good,0,UINT64_MAX-1),"context and module bounds enforced");
  auto stale=good;int checks=0;
  require(!v3::DecodeNativeVfsConfiguration([&](auto a,auto b,auto c){return stale.Read(a,b,c);},base,0,[&]{return ++checks==1;}),"retirement before publication rejected");
  auto rules=good;
  rules.Put<uint64_t>(fs+0x48,0x80000);rules.Put<uint32_t>(fs+0x50,3);rules.Put<uint32_t>(fs+0x54,2);
  rules.NativeString(0x80000,"E:/Mods/修改地图");rules.NativeString(0x80020,"map_data/state_regions");
  rules.NativeString(0x80040,"base.zip");rules.NativeString(0x80060,"gfx/map");
  rules.Put<uint64_t>(fs+0x60,0x90000);rules.Put<uint32_t>(fs+0x68,2);rules.Put<uint32_t>(fs+0x6c,2);
  rules.NativeString(0x90000,"map_data/old");rules.NativeString(0x90020,"map_data/旧地区");
  auto decoded=check(rules);
  require(decoded && decoded->mounts.size()==2 && decoded->replacements.size()==2 && decoded->exclusions.size()==2,
      "mounts and both rule tables copied as one configuration");
  require(decoded->replacements[0].root=="E:/Mods/修改地图" && decoded->replacements[0].path=="map_data/state_regions" &&
      decoded->replacements[1].root=="base.zip" && decoded->replacements[1].path=="gfx/map" &&
      decoded->exclusions[1]=="map_data/旧地区","inline/heap UTF-8 rules retain native order and exact bytes");
  auto ruleTest=[&](auto change,const char* message){auto memory=rules;change(memory);require(!check(memory),message);};
  for(unsigned offset:{0x48u,0x60u}){
    ruleTest([&](auto& m){m.template Put<uint32_t>(fs+offset+8,4097);m.template Put<uint32_t>(fs+offset+12,4097);},"unbounded rule count rejected");
    ruleTest([&](auto& m){m.template Put<uint32_t>(fs+offset+8,1);},"rule count exceeds capacity rejected");
    ruleTest([&](auto& m){m.template Put<uint32_t>(fs+offset+8,1048577);},"unbounded rule capacity rejected");
    ruleTest([&](auto& m){m.template Put<uint64_t>(fs+offset,0);},"missing nonempty rule table rejected");
    ruleTest([&](auto& m){m.template Put<uint64_t>(fs+offset,UINT64_MAX-15);},"rule table extent overflow rejected");
    ruleTest([&](auto& m){m.bytes.erase(fs+offset+12);},"unreadable rule header rejected");
  }
  ruleTest([&](auto& m){m.template Put<uint64_t>(0x80010,4096);},"oversized native string rejected");
  ruleTest([&](auto& m){m.template Put<uint64_t>(0x80018,16);},"string length exceeds capacity rejected");
  ruleTest([&](auto& m){m.template Put<uint64_t>(0x80058,14);},"invalid inline string capacity rejected");
  ruleTest([&](auto& m){m.template Put<uint64_t>(0x80018,1048577);},"unbounded string capacity rejected");
  ruleTest([&](auto& m){m.template Put<uint64_t>(0x80000,0);},"missing heap string rejected");
  ruleTest([&](auto& m){m.template Put<uint64_t>(0x80000,UINT64_MAX);m.template Put<uint8_t>(UINT64_MAX,'x');},"heap string overflow rejected");
  ruleTest([&](auto& m){m.template Put<uint8_t>(0x81000,0);},"embedded string terminator disagrees with length rejected");
  ruleTest([&](auto& m){m.bytes.erase(0x90000);},"unreadable inline exclusion rejected");
  ruleTest([&](auto& m){m.template Put<uint8_t>(0x91020,0);},"unreadable or truncated heap exclusion rejected");
  for(auto change:std::vector<std::pair<uint64_t,uint64_t>>{{fs+0x48,fs+0x54},{fs+0x60,fs+0x6c},
       {0x80000,0x80018},{0x81000,0x81000},{0x90000,0x90000},{0x91020,0x91020}})
    ruleTest([&](auto& m){m.changeAt=change.first;m.changeTarget=change.second;},"rule mutation during combined copy rejected");
  auto originalRoot=decoded->replacements[0].root;rules.NativeString(0x80000,"later");
  require(decoded->replacements[0].root==originalRoot,"owned rule strings outlive native source changes");
  auto empty=good;empty.Put<uint64_t>(fs+0x60,0x90000);empty.Put<uint32_t>(fs+0x68,1);empty.Put<uint32_t>(fs+0x6c,1);
  empty.NativeString(0x90000,"");auto emptyResult=check(empty);
  require(emptyResult && emptyResult->exclusions.size()==1 && emptyResult->exclusions[0].empty(),
      "explicit empty native rule retained without inventing interpretation");
  v3::NativeVfsConfiguration policy;policy.searchPaths={"E:/Mods/high","E:/Mods/改图","E:/base"};
  policy.replacements={{"E:/Mods/改图","map_data"},{"E:/Mods/high","map_data/state_regions"},
      {"missing owner","gfx/map"}};
  policy.exclusions={"gui/gui_editor"};
  require(v3::NativeVfsDirectoryRootCount(policy,"map_data")==2 &&
      v3::NativeVfsDirectoryRootCount(policy,"map_data/state_regions")==1,
      "replacement allows owning root and higher priorities, including nested directory rules");
  require(v3::NativeVfsDirectoryRootCount(policy,"map_data/state_regions/nested")==3 &&
      v3::NativeVfsDirectoryRootCount(policy,"map_data_extra")==3,
      "replacement itself is exact-directory equality; recursive walking must apply it at its parent");
  require(v3::NativeVfsDirectoryRootCount(policy,"gui/gui_editor")==0 &&
      v3::NativeVfsDirectoryRootCount(policy,"gui/gui_editor/child")==0 &&
      v3::NativeVfsDirectoryRootCount(policy,"gui/gui_editor_extra")==0,
      "native exclusions are literal prefixes, including a same-prefix neighbouring name");
  require(v3::NativeVfsDirectoryRootCount(policy,"GUI/gui_editor")==3 &&
      v3::NativeVfsDirectoryRootCount(policy,"Map_data")==3 &&
      v3::NativeVfsDirectoryRootCount(policy,"gfx/map")==3,
      "native byte comparisons do not fold case and an unmounted replacement owner gives no cutoff");
  policy.replacements.push_back({"E:/Mods/high","map_data"});
  require(v3::NativeVfsDirectoryRootCount(policy,"map_data")==1,"multiple rules choose highest priority cutoff");
  policy.replacements={{"E:\\Mods\\high","map_data"}};
  require(v3::NativeVfsDirectoryRootCount(policy,"map_data")==3,"root lookup uses actual native search-path bytes");
  policy.exclusions={""};require(!v3::NativeVfsDirectoryRootCount(policy,"map_data"),"empty exclusion retains native exclude-all behavior");
  std::puts("PASS: frozen VFS mounts and replacement/exclusion rules, inline/heap strings, bounds and combined copy retirement");
}
