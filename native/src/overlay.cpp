#include "overlay.h"
#include "native_trade_view.h"
#include "state_market.h"
#include "arrow_lease.h"
#include "native_rail.h"
#include "native_economy.h"
#include "native_access.h"
#include "native_strait_probe.h"
#include "frame_timing.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include <MinHook.h>
#include <dxgi1_2.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <set>
#include <tuple>
#include <unordered_set>

#include "log.h"
#include "offsets.h"
#include "arrow_geometry.h"
#include "dev_snapshot.h"
#include "variable_probe.h"
#include "goods_snapshot.h"
#include <intrin.h>
#include "embedded_assets.h"
#include "util.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam,
                                                             LPARAM lParam);

namespace v3 {
namespace {

using FnPresent = HRESULT(WINAPI*)(IDXGISwapChain*, UINT, UINT);
using FnPresent1 = HRESULT(WINAPI*)(IDXGISwapChain1*, UINT, UINT, const DXGI_PRESENT_PARAMETERS*);
thread_local unsigned g_frameTimingDepth=0;
using FnSetPath = void(__fastcall*)(void*, void*);
using FnAppend = void(__fastcall*)(void*, void*);
using FnEconomyCallback = void(__fastcall*)(void*, void*, void*);
FnEconomyCallback g_origEconomyCallback = nullptr;
using FnPartialDailyTick = void(__fastcall*)(void*);
FnPartialDailyTick g_origPartialDailyTick = nullptr;
std::atomic<unsigned> g_accessSamples{0};
void __fastcall PartialDailyTickThunk(void* command) {
  const unsigned sample=g_accessSamples.fetch_add(1);
  const auto observe=[] {
    const GoodsMemoryReader read=[](uint64_t address,void* out,size_t size){
      SIZE_T copied=0;
      return address && size<=UINT64_MAX-address &&
          ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(address),
                            out,size,&copied) && copied==size;
    };
    return ObserveNativeAccess(read,reinterpret_cast<uint64_t>(Game::Instance().Base()),
                               __readgsqword(0x58));
  };
  const auto before=sample<16?observe():std::optional<NativeAccessObservation>{};
  g_origPartialDailyTick(command);
  if(sample>=16)return;
  const auto after=observe();
  if(!before || !after){V3_WARN("native access tick probe sample=%u unreadable",sample);return;}
  V3_INFO("native access tick probe sample=%u thread=%lu access=%u/%u hidden=%u/%u owns=%u/%u context=%u/%u date=%u/%u machine=%llx/%llx world=%llx/%llx db=%llx/%llx",
      sample,GetCurrentThreadId(),before->access,after->access,before->hidden,after->hidden,
      unsigned(before->owns),unsigned(after->owns),before->context,after->context,before->date,after->date,
      (unsigned long long)before->machine,(unsigned long long)after->machine,
      (unsigned long long)before->world,(unsigned long long)after->world,
      (unsigned long long)before->stateDb,(unsigned long long)after->stateDb);
  // No GUI effect, selection, database scan, worker request, engine-owned
  // pointer retention or economic write is performed from this observation.
}
// Diagnostic only: observe calls the game already makes; never request one.
using FnStraitPermission = uint8_t(__fastcall*)(void*, void*, uint8_t);
FnStraitPermission g_origStraitPermission = nullptr;
std::atomic<unsigned> g_straitPermissionSamples{0};
std::mutex g_straitCoverageMutex;
unsigned g_straitCoverageEvents=0;
std::set<std::tuple<uint32_t,uint32_t,uint8_t,uint8_t>> g_straitCoverage;
struct StraitClassificationTrace {
  uint32_t country=UINT32_MAX;
  uint8_t mode=0;
  bool invalid=false;
  NativeStraitClassification observed;
};
thread_local StraitClassificationTrace* g_straitClassificationTrace=nullptr;
struct DomesticLaneBuildTrace {
  uint32_t country=UINT32_MAX;
  unsigned generation=0,classifications=0;
  bool invalid=false;
  std::optional<NativeStraitClassification> access;
  std::vector<std::pair<uint32_t,NativeLaneSelection>> lanes;
};
thread_local DomesticLaneBuildTrace* g_domesticLaneBuildTrace=nullptr;
uint8_t __fastcall StraitPermissionProbe(void* strait, void* country, uint8_t mode) {
  const auto base=reinterpret_cast<uint64_t>(Game::Instance().Base());
  const auto caller=reinterpret_cast<uint64_t>(__builtin_return_address(0))-base;
  const unsigned sample=g_straitPermissionSamples.fetch_add(1);
  const GoodsMemoryReader read=[](uint64_t address,void* out,size_t size){
    SIZE_T copied=0;return address && size<=UINT64_MAX-address &&
      ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(address),out,size,&copied) && copied==size;
  };
  uint32_t sid=UINT32_MAX,cid=UINT32_MAX;uint64_t definition=0;uint8_t marker=255;
  std::array<uint8_t,4> controls{};
  const auto sp=reinterpret_cast<uint64_t>(strait),cp=reinterpret_cast<uint64_t>(country);
  const auto before=sample<32?ObserveNativeAccess(read,base,__readgsqword(0x58)):std::optional<NativeAccessObservation>{};
  const bool fields=sample<32 && sp && cp && read(sp+8,&sid,4) && read(cp+0x18,&cid,4) &&
      read(sp+0xa0,&definition,8) && definition && read(definition+0x4b,&marker,1) && read(sp+0xa8,controls.data(),4);
  const uint8_t result=g_origStraitPermission(strait,country,mode);
  if(auto* trace=g_straitClassificationTrace) {
    uint32_t actualCountry=UINT32_MAX,actualStrait=UINT32_MAX;
    if(!sp || !cp || cp>UINT64_MAX-0x1c || sp>UINT64_MAX-0xc ||
       !read(cp+0x18,&actualCountry,4) || !read(sp+8,&actualStrait,4) ||
       actualCountry!=trace->country || mode!=trace->mode ||
       trace->observed.allowed.size()+trace->observed.denied.size()>=4096)trace->invalid=true;
    else try {
      (result?trace->observed.allowed:trace->observed.denied).push_back(actualStrait);
    } catch(...) {trace->invalid=true;}
  }
  if(sample<32){
    const auto after=ObserveNativeAccess(read,base,__readgsqword(0x58));
    V3_INFO("native strait probe sample=%u thread=%lu caller=%llx mode=%u result=%u fields=%u country=%u strait=%u marker=%u controls=%u,%u,%u,%u observation=%u/%u access=%u/%u owns=%u/%u context=%u/%u phase=%u/%u world=%llx/%llx date=%u/%u",
      sample,GetCurrentThreadId(),(unsigned long long)caller,unsigned(mode),unsigned(result),unsigned(fields),cid,sid,unsigned(marker),
      unsigned(controls[0]),unsigned(controls[1]),unsigned(controls[2]),unsigned(controls[3]),unsigned(bool(before)),unsigned(bool(after)),
      before?before->access:999,after?after->access:999,before?unsigned(before->owns):999,after?unsigned(after->owns):999,
      before?before->context:999,after?after->context:999,before?before->phase:999,after?after->phase:999,
      before?(unsigned long long)before->world:0,after?(unsigned long long)after->world:0,before?before->date:0,after?after->date:0);
  }
  // Bounded diagnostic coverage only; never publish these potentially stale
  // observations to a snapshot. A busy observer drops samples rather than wait.
  if(sample<150000 && sp && cp && mode==1 && (fields || (read(sp+8,&sid,4) && read(cp+0x18,&cid,4)))) {
    bool first=false;
    try {
      std::unique_lock<std::mutex> lock(g_straitCoverageMutex,std::try_to_lock);
      if(lock.owns_lock() && g_straitCoverage.size()<4096 && g_straitCoverageEvents<8192) {
        first=g_straitCoverage.emplace(cid,sid,mode,result).second;
        if(first) {
          g_straitCoverage.erase({cid,sid,mode,uint8_t(result?0:1)});
          ++g_straitCoverageEvents;
        }
      }
    } catch(...) {}
    if(first) V3_INFO("native strait coverage sample=%u country=%u strait=%u mode=%u result=%u",sample,cid,sid,unsigned(mode),unsigned(result));
  }
  return result;
}
using FnStraitClassification = void(__fastcall*)(void*,uint8_t,void*,void*);
FnStraitClassification g_origStraitClassification=nullptr;
std::atomic<unsigned> g_straitClassificationSamples{0},g_straitClassificationFailures{0};
std::mutex g_straitClassificationMutex;
unsigned g_straitClassificationEvents=0;
std::map<std::pair<uint32_t,uint8_t>,NativeStraitClassification> g_straitClassifications;
void __fastcall StraitClassificationProbe(void* country,uint8_t mode,void* allowed,void* denied) {
  const unsigned sample=g_straitClassificationSamples.fetch_add(1);
  if(sample>=20000){
    if(g_domesticLaneBuildTrace)g_domesticLaneBuildTrace->invalid=true;
    g_origStraitClassification(country,mode,allowed,denied);return;
  }
  const auto base=reinterpret_cast<uint64_t>(Game::Instance().Base());
  const auto caller=reinterpret_cast<uint64_t>(__builtin_return_address(0))-base;
  const auto cp=reinterpret_cast<uint64_t>(country),ap=reinterpret_cast<uint64_t>(allowed),dp=reinterpret_cast<uint64_t>(denied);
  const GoodsMemoryReader read=[](uint64_t a,void* p,size_t n){SIZE_T copied=0;
    return a && n<=UINT64_MAX-a && ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(a),p,n,&copied) && copied==n;};
  StraitClassificationTrace trace;trace.mode=mode;
  std::optional<NativeAccessObservation> before;
  std::optional<NativeStraitOutputHeader> a,d;
  bool prepared=false;
  try {
    before=ObserveNativeAccess(read,base,__readgsqword(0x58));
    a=ReadNativeStraitOutputHeader(read,ap);d=ReadNativeStraitOutputHeader(read,dp);
    prepared=before && a && d && cp && cp<=UINT64_MAX-0x1c &&
        read(cp+0x18,&trace.country,4) && trace.country!=UINT32_MAX;
  } catch(...) {}
  {
    struct RestoreTrace {
      StraitClassificationTrace* previous;
      ~RestoreTrace(){g_straitClassificationTrace=previous;}
    } restore{g_straitClassificationTrace};
    g_straitClassificationTrace=prepared?&trace:nullptr;
    g_origStraitClassification(country,mode,allowed,denied);
  }
  try {
    const auto after=ObserveNativeAccess(read,base,__readgsqword(0x58));
    auto output=prepared?CopyNativeStraitClassification(read,ap,dp,a->count,d->count):std::optional<NativeStraitClassification>{};
    uint32_t actualCountry=UINT32_MAX;
    const bool same=before && after && before->world==after->world && before->machine==after->machine &&
        before->stateDb==after->stateDb && before->date==after->date && before->phase==after->phase &&
        before->access==after->access && before->hidden==after->hidden &&
        before->owns==after->owns && before->context==after->context &&
        cp && cp<=UINT64_MAX-0x1c && read(cp+0x18,&actualCountry,4) && actualCountry==trace.country;
    if(!same || !output || trace.invalid || *output!=trace.observed) {
      if(g_domesticLaneBuildTrace)g_domesticLaneBuildTrace->invalid=true;
      if(g_straitClassificationFailures.fetch_add(1)<32)
        V3_WARN("native strait classification refused sample=%u country=%u prepared=%u scope=%u copied=%u trace=%u",sample,trace.country,unsigned(prepared),unsigned(same),unsigned(bool(output)),unsigned(!trace.invalid));
      return;
    }
    if(auto* build=g_domesticLaneBuildTrace) {
      if(mode!=1 || trace.country!=build->country || (build->access && *build->access!=*output))build->invalid=true;
      else {build->access=*output;++build->classifications;}
    }
    bool changed=false;
    {
      std::unique_lock<std::mutex> lock(g_straitClassificationMutex,std::try_to_lock);
      if(lock.owns_lock() && g_straitClassifications.size()<512 && g_straitClassificationEvents<1024) {
        const auto key=std::make_pair(trace.country,mode);auto found=g_straitClassifications.find(key);
        if(found==g_straitClassifications.end() || found->second!=*output) {
          g_straitClassifications[key]=*output;++g_straitClassificationEvents;changed=true;
        }
      }
    }
    if(changed) {
      auto list=[](const auto& values){std::string s;for(auto id:values){if(!s.empty())s+=',';s+=std::to_string(id);}return s;};
      const auto yes=list(output->allowed),no=list(output->denied);
      V3_INFO("native strait classification sample=%u thread=%lu caller=%llx country=%u mode=%u world=%llx date=%u access=%u owns=%u context=%u prefixes=%u/%u matched=1 allowed=[%s] denied=[%s]",
          sample,GetCurrentThreadId(),(unsigned long long)caller,trace.country,unsigned(mode),
          (unsigned long long)after->world,after->date,after->access,unsigned(after->owns),after->context,a->count,d->count,yes.c_str(),no.c_str());
    }
  } catch(...) {if(g_domesticLaneBuildTrace)g_domesticLaneBuildTrace->invalid=true;}
  // Diagnostic only: no result or borrowed pointer is handed to the worker.
}
bool ReadProbeMemory(uint64_t a,void* p,size_t n) {
  SIZE_T copied=0;return a && n<=UINT64_MAX-a &&
      ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(a),p,n,&copied) && copied==n;
}
using FnShippingConstructor=void*(__fastcall*)(void*,void*,void*,void*,void*,void*,void*,void*,void*);
FnShippingConstructor g_origShippingConstructor=nullptr;
void* __fastcall ShippingConstructorProbe(void* db,void* kind,void* country,void* treaty,
    void* from,void* to,void* provinces,void* extra,void* distance) {
  auto* build=g_domesticLaneBuildTrace;
  if(!build)return g_origShippingConstructor(db,kind,country,treaty,from,to,provinces,extra,distance);
  const GoodsMemoryReader read=ReadProbeMemory;
  std::optional<NativeLaneSelection> input;uint32_t category=UINT32_MAX;
  try {
    if(!read(reinterpret_cast<uint64_t>(kind),&category,4))build->invalid=true;
    else if(category==0)
      input=CopyNativeLaneSelection(read,reinterpret_cast<uint64_t>(country),reinterpret_cast<uint64_t>(from),
          reinterpret_cast<uint64_t>(to),reinterpret_cast<uint64_t>(provinces),reinterpret_cast<uint64_t>(distance));
    if(category==0 && (!input || input->country!=build->country || !build->access || build->lanes.size()>=512))build->invalid=true;
  } catch(...) {build->invalid=true;}
  void* result=g_origShippingConstructor(db,kind,country,treaty,from,to,provinces,extra,distance);
  if(category!=0 || build->lanes.size()>=512)return result;
  try {
    const auto base=reinterpret_cast<uint64_t>(Game::Instance().Base()),object=reinterpret_cast<uint64_t>(result);
    uint64_t type=0,secondary=0;uint32_t id=UINT32_MAX,actualKind=UINT32_MAX;
    auto output=input && object && object<=UINT64_MAX-0x68 && read(object,&type,8) && type==base+0x04427d98 &&
        read(object+0x10,&secondary,8) && secondary==base+0x04427d60 && read(object+8,&id,4) && id!=UINT32_MAX &&
        read(object+0x1c,&actualKind,4) && actualKind==0?
        CopyNativeLaneSelection(read,object+0x18,object+0x48,object+0x4c,object+0x30,object+0x60):std::optional<NativeLaneSelection>{};
    if(!output || *output!=*input)build->invalid=true;
    else build->lanes.emplace_back(id,std::move(*output));
  } catch(...) {build->invalid=true;}
  return result;
}
using FnDomesticLaneBuild=void(__fastcall*)(void*,void*);
FnDomesticLaneBuild g_origDomesticLaneBuild=nullptr;
std::atomic<unsigned> g_domesticLaneBuildSamples{0},g_domesticLaneBuildEvents{0};
void __fastcall DomesticLaneBuildProbe(void* manager,void* country) {
  const unsigned sample=g_domesticLaneBuildSamples.fetch_add(1);
  if(sample>=512){g_origDomesticLaneBuild(manager,country);return;}
  const auto base=reinterpret_cast<uint64_t>(Game::Instance().Base()),cp=reinterpret_cast<uint64_t>(country);
  const GoodsMemoryReader read=ReadProbeMemory;
  DomesticLaneBuildTrace trace;trace.generation=sample+1;
  std::optional<NativeAccessObservation> before;
  bool prepared=false;
  try {
    before=ObserveNativeAccess(read,base,__readgsqword(0x58));
    prepared=before && cp && cp<=UINT64_MAX-0x1c && read(cp+0x18,&trace.country,4) && trace.country!=UINT32_MAX;
  } catch(...) {}
  {
    struct RestoreBuildTrace {
      DomesticLaneBuildTrace* previous;
      ~RestoreBuildTrace(){g_domesticLaneBuildTrace=previous;}
    } restore{g_domesticLaneBuildTrace};
    g_domesticLaneBuildTrace=prepared?&trace:nullptr;
    g_origDomesticLaneBuild(manager,country);
  }
  if(!prepared || trace.lanes.empty())return;
  try {
    const auto after=ObserveNativeAccess(read,base,__readgsqword(0x58));
    bool valid=!trace.invalid && trace.access && trace.classifications && after &&
        before->world==after->world && before->machine==after->machine && before->stateDb==after->stateDb &&
        before->date==after->date && before->phase==after->phase && before->access==after->access &&
        before->hidden==after->hidden && before->owns==after->owns && before->context==after->context;
    uint64_t db=0,type=0,entries=0;uint32_t slots=0,actualCountry=UINT32_MAX;
    valid=valid && read(cp+0x18,&actualCountry,4) && actualCountry==trace.country &&
        read(base+0x05966760,&db,8) && db && db<=UINT64_MAX-0x30 && read(db,&type,8) && type==base+0x04439410 &&
        read(db+0x20,&entries,8) && read(db+0x2c,&slots,4) && slots<=131072;
    for(const auto& [id,selected]:trace.lanes) {
      uint64_t object=0,secondary=0;uint32_t actual=UINT32_MAX,kind=UINT32_MAX;
      valid=valid && (id&0xffffff)<slots && entries && entries<=UINT64_MAX-uint64_t(slots)*16 &&
          read(entries+uint64_t(id&0xffffff)*16+8,&object,8) && object && object<=UINT64_MAX-0x68 &&
          read(object+8,&actual,4) && actual==id && read(object,&type,8) && type==base+0x04427d98 &&
          read(object+0x10,&secondary,8) && secondary==base+0x04427d60 && read(object+0x1c,&kind,4) && kind==0;
      auto current=valid?CopyNativeLaneSelection(read,object+0x18,object+0x48,object+0x4c,object+0x30,object+0x60):std::optional<NativeLaneSelection>{};
      if(!current || *current!=selected){valid=false;break;}
    }
    if(!valid){V3_WARN("native domestic lane binding refused generation=%u country=%u lanes=%zu classifications=%u",trace.generation,trace.country,trace.lanes.size(),trace.classifications);return;}
    auto ids=[](const auto& values){std::string s;for(auto id:values){if(!s.empty())s+=',';s+=std::to_string(id);}return s;};
    const auto allowed=ids(trace.access->allowed),denied=ids(trace.access->denied);
    for(const auto& [id,selected]:trace.lanes)if(g_domesticLaneBuildEvents.fetch_add(1)<256) {
      std::string ranges;
      for(const auto& r:selected.provinces){if(!ranges.empty())ranges+=',';ranges+=std::to_string(r.first)+':'+std::to_string(r.last);}
      V3_INFO("native domestic lane binding generation=%u thread=%lu world=%llx date=%u country=%u lane=%u from=%u to=%u distance=%lld classifications=%u matched=1 allowed=[%s] denied=[%s] provinces=[%s]",
          trace.generation,GetCurrentThreadId(),(unsigned long long)after->world,after->date,trace.country,id,
          selected.fromState,selected.toState,(long long)selected.distance,trace.classifications,allowed.c_str(),denied.c_str(),ranges.c_str());
    }
  } catch(...) {}
  // All results remain diagnostic, owned and confined to this natural call.
}
std::atomic<uint64_t> g_economyCallbackCalls{0};
using FnContainerRead = void*(__fastcall*)(void*, void*, void*);
FnContainerRead g_origContainerKeyed = nullptr;
FnContainerRead g_origContainerIndexed = nullptr;
std::atomic<uint64_t> g_containerCalls{0};
using FnProductionReader = void*(__fastcall*)(void*, void*);
FnProductionReader g_origProductionReader = nullptr;
std::atomic<uint64_t> g_productionReaderCalls{0};
using FnScriptRegister = void(__fastcall*)(const char*, uintptr_t, void*, uintptr_t);
FnScriptRegister g_origScriptRegister = nullptr;
using FnPanelMarket = void*(__fastcall*)(void*, void*);
FnPanelMarket g_origPanelMarket = nullptr;
std::atomic<unsigned> g_panelMarketSamples{0};
void* __fastcall PanelMarketThunk(void* panel, void* out) {
  // The caller owns the returned reference. Forward exactly once and preserve
  // both the output buffer and RAX; never retain/release the wrapper ourselves.
  void* result = g_origPanelMarket(panel, out);
  if (g_panelMarketSamples.fetch_add(1) >= 16) return result;
  uint32_t panelId = UINT32_MAX;
  void* wrapper = nullptr;
  std::array<uint8_t, 24> bytes{};
  SIZE_T copied = 0;
  bool ok = panel && out && result == out &&
      ReadProcessMemory(GetCurrentProcess(), static_cast<uint8_t*>(panel) + 0x70,
                        &panelId, sizeof(panelId), &copied) && copied == sizeof(panelId) &&
      ReadProcessMemory(GetCurrentProcess(), out, &wrapper, sizeof(wrapper), &copied) &&
      copied == sizeof(wrapper) && wrapper &&
      ReadProcessMemory(GetCurrentProcess(), wrapper, bytes.data(), bytes.size(), &copied) &&
      copied == bytes.size();
  uintptr_t vtable = 0;
  uint32_t wrapperId = UINT32_MAX;
  if (ok) {
    std::memcpy(&vtable, bytes.data(), sizeof(vtable));
    std::memcpy(&wrapperId, bytes.data() + 0x10, sizeof(wrapperId));
    ok = vtable == reinterpret_cast<uintptr_t>(Game::Instance().Base()) + 0x045850B0 &&
         wrapperId == panelId && panelId != UINT32_MAX;
  }
  V3_INFO("panel market probe thread=%lu panelId=%u wrapperId=%u match=%d",
          GetCurrentThreadId(), panelId, wrapperId, int(ok));
  // Diagnostic only: no selection change, GUI effect, database scan or worker
  // request is performed from this callback.
  return result;
}
bool PointerReadable(const void* p, size_t bytes) {
  MEMORY_BASIC_INFORMATION mbi{};
  if (VirtualQuery(p, &mbi, sizeof(mbi)) != sizeof(mbi)) return false;
  if (mbi.State != MEM_COMMIT) return false;
  const auto start = reinterpret_cast<uintptr_t>(p);
  const auto end = start + bytes;
  return end <= reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
}
void __fastcall ScriptRegisterThunk(const char* name, uintptr_t flags, void* callback,
                                    uintptr_t extra) {
  if (name && PointerReadable(name, 4)) {
    char text[96]{};
    std::memcpy(text, name, sizeof(text) - 1);
    if (std::strstr(text, "arket") || std::strstr(text, "market"))
      V3_INFO("script register '%s' cb=%p", text, callback);
  }
  if (g_origScriptRegister) g_origScriptRegister(name, flags, callback, extra);
}

void LogContainerRead(const char* tag, void* container, void* out, void* arg, void* ret) {
  const uint64_t n = g_containerCalls.fetch_add(1);
  if (n >= 8 || !out) return;
  uint64_t words[6]{};
  std::memcpy(words, out, sizeof(words));
  V3_INFO("eco %s container=%p out=%p arg=%p ret=%p words=%016llx %016llx %016llx %016llx "
          "%016llx %016llx",
          tag, container, out, arg, ret, (unsigned long long)words[0],
          (unsigned long long)words[1], (unsigned long long)words[2],
          (unsigned long long)words[3], (unsigned long long)words[4],
          (unsigned long long)words[5]);
}
void* __fastcall ContainerKeyedThunk(void* container, void* out, void* arg) {
  void* ret = g_origContainerKeyed ? g_origContainerKeyed(container, out, arg) : nullptr;
  LogContainerRead("keyed", container, out, arg, ret);
  return ret;
}
void* __fastcall ContainerIndexedThunk(void* container, void* out, void* arg) {
  void* ret = g_origContainerIndexed ? g_origContainerIndexed(container, out, arg) : nullptr;
  LogContainerRead("indexed", container, out, arg, ret);
  const uint64_t n = g_containerCalls.load();
  if (n < 400 && container && arg) {
    const int goodId = *reinterpret_cast<int*>(static_cast<uint8_t*>(arg) + 0x10);
    if (goodId >= 0 && goodId < 4096) {
      const auto* c = static_cast<const uint8_t*>(container);
      const uint64_t mask =
          *reinterpret_cast<const uint64_t*>(c + 0x20 + (int64_t)(goodId >> 6) * 8);
      if ((mask >> (goodId & 63)) & 1) {
        const auto base = *reinterpret_cast<const uint64_t*>(c + 8);
        const uint64_t value =
            *reinterpret_cast<const uint64_t*>(base + (int64_t)goodId * 8);
        if (value)
          V3_INFO("eco value container=%p good=%d value=%llu (0x%llx)", container, goodId,
                  (unsigned long long)value, (unsigned long long)value);
      }
    }
  }
  return ret;
}
void* __fastcall ProductionReaderThunk(void* ctx, void* out) {
  const uint64_t n = g_productionReaderCalls.fetch_add(1);
  struct EconStateDump {
    int id = 0;
    std::array<uint64_t, 256> prod{}, cons{};
  };
  static std::mutex econMutex;
  static std::vector<EconStateDump> econStates;
  static std::unordered_set<int> collectedStates;
  const auto writeDump = [&]() {
    const std::wstring path = DirectoryOf(RuntimeDataDirectory()) + L"\\economy_dump.txt";
    std::ofstream out(std::filesystem::path(path), std::ios::binary | std::ios::trunc);
    if (!out) return;
    uint64_t totalProd = 0, totalCons = 0;
    out << "# states=" << econStates.size() << " scale=100000\n";
    for (const auto& s : econStates) {
      for (int g = 0; g < 256; ++g) {
        if (s.prod[g]) {
          out << "P " << s.id << ' ' << g << ' ' << s.prod[g] << '\n';
          totalProd += s.prod[g];
        }
        if (s.cons[g]) {
          out << "C " << s.id << ' ' << g << ' ' << s.cons[g] << '\n';
          totalCons += s.cons[g];
        }
      }
    }
    out << "# totalProd=" << totalProd << " totalCons=" << totalCons << '\n';
    // Legacy reader samples are unversioned and accumulate across callbacks.
    // They remain diagnostic dumps only; the versioned UI collector is the
    // sole live snapshot source, avoiding old-world or market=1 publication.
    V3_INFO("eco dump states=%zu totalProd=%llu totalCons=%llu", econStates.size(),
            (unsigned long long)totalProd, (unsigned long long)totalCons);
  };
  static uint64_t sumProd = 0, sumCons = 0;
  static int sumGood = -1, sumCount = 0;
  static std::unordered_set<int> seenStates;
  if (n < 3 && ctx && out) {
    const int stateId = *reinterpret_cast<int*>(static_cast<uint8_t*>(ctx) + 0x18);
    uint64_t pre[6]{};
    std::memcpy(pre, out, sizeof(pre));
    V3_INFO("eco reader pre  state=%d out=%p words=%016llx %016llx %016llx %016llx %016llx "
            "%016llx",
            stateId, out, (unsigned long long)pre[0], (unsigned long long)pre[1],
            (unsigned long long)pre[2], (unsigned long long)pre[3],
            (unsigned long long)pre[4], (unsigned long long)pre[5]);
  }
  void* r = g_origProductionReader ? g_origProductionReader(ctx, out) : nullptr;
  if (n < 4000 && ctx) {
    int stateId = *reinterpret_cast<int*>(static_cast<uint8_t*>(ctx) + 0x18);
    using Lookup = void*(__fastcall*)(int*);
    auto lookup = reinterpret_cast<Lookup>(Game::Instance().Base() + 0x007D40C0);
    void* state = lookup(&stateId);
    if (state && *reinterpret_cast<int*>(static_cast<uint8_t*>(state) + 8) == stateId) {
      static std::unordered_set<int> dumpedStateWords;
      if ((stateId == 16 || stateId == 17 || stateId == 18 || stateId == 19 ||
           stateId == 21 || stateId == 23 || stateId == 201 || stateId == 205 ||
           stateId == 132 || stateId == 350 || stateId == 432 || stateId == 436) &&
          dumpedStateWords.insert(stateId).second) {
        uint64_t w[64]{};
        std::memcpy(w, state, sizeof(w));
        for (int i = 0; i < 64; i += 4)
          V3_INFO("eco sw id=%d off=0x%02x %016llx %016llx %016llx %016llx", stateId, i * 8,
                  (unsigned long long)w[i], (unsigned long long)w[i + 1],
                  (unsigned long long)w[i + 2], (unsigned long long)w[i + 3]);
      }
      static std::atomic<bool> scannedState{false};
      if (!scannedState.exchange(true)) {
        const auto* s = static_cast<const uint8_t*>(state);
        for (int off = 0; off < 0x100; off += 8) {
          void* p = *reinterpret_cast<void* const*>(s + off);
          if (!p) continue;
          MEMORY_BASIC_INFORMATION mbi{};
          if (!VirtualQuery(p, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT ||
              (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)))
            continue;
          char buf[0x81]{};
          std::memcpy(buf, p, 0x80);
          for (int i = 0; i + 6 < 0x80; ++i) {
            if (std::memcmp(buf + i, "STATE_", 6) == 0) {
              V3_INFO("eco state key off=0x%x ptr=%p text=%.64s", off, p, buf + i);
              break;
            }
          }
          // Also treat the pointer as an MSVC string (data, size@0x10, cap@0x18).
          const size_t len = *reinterpret_cast<size_t*>(static_cast<uint8_t*>(p) + 0x10);
          const size_t cap = *reinterpret_cast<size_t*>(static_cast<uint8_t*>(p) + 0x18);
          if (len >= 6 && len < 64 && cap >= len && cap < 128) {
            char text[65]{};
            std::memcpy(text, p, len);
            if (std::memcmp(text, "STATE_", 6) == 0)
              V3_INFO("eco state cstr off=0x%x ptr=%p text=%.64s", off, p, text);
          }
        }
      }
    }
    static std::atomic<bool> enumerated{false};
    if (state && *reinterpret_cast<int*>(static_cast<uint8_t*>(state) + 8) == stateId &&
        !enumerated.exchange(true)) {
      void* expectedVtable = *reinterpret_cast<void**>(state);
      int stateCount = 0, prodGoods = 0, consGoods = 0;
      uint64_t totalProd = 0, totalCons = 0;
      for (int id = 0; id < 2048; ++id) {
        int value = id;
        void* s = lookup(&value);
        if (!s || *reinterpret_cast<int*>(static_cast<uint8_t*>(s) + 8) != id ||
            *reinterpret_cast<void**>(s) != expectedVtable)
          continue;
        ++stateCount;
        for (int off : {0xC8, 0x78}) {
          auto* c = static_cast<uint8_t*>(s) + off;
          for (int g = 0; g < 256; ++g) {
            const uint64_t mask =
                *reinterpret_cast<uint64_t*>(c + 0x20 + (int64_t)(g >> 6) * 8);
            if (((mask >> (g & 63)) & 1) == 0) continue;
            const auto base = *reinterpret_cast<uint64_t*>(c + 8);
            const uint64_t v = *reinterpret_cast<uint64_t*>(base + (int64_t)g * 8);
            if (!v) continue;
            if (off == 0xC8) {
              ++prodGoods;
              totalProd += v;
            } else {
              ++consGoods;
              totalCons += v;
            }
          }
        }
      }
      V3_INFO("eco enumerate states=%d prodGoods=%d consGoods=%d totalProd=%llu totalCons=%llu",
              stateCount, prodGoods, consGoods, (unsigned long long)totalProd,
              (unsigned long long)totalCons);
    }
    if (state) {
      if (n < 3) {
        for (int off = 0; off < 0x100; off += 4) {
          if (*reinterpret_cast<int*>(static_cast<uint8_t*>(state) + off) == stateId) {
            V3_INFO("eco state id at +0x%x (state=%p id=%d)", off, state, stateId);
            break;
          }
        }
      }
      void* good = *reinterpret_cast<void**>(static_cast<uint8_t*>(ctx) + 0x10);
      const int goodId = good ? *reinterpret_cast<int*>(static_cast<uint8_t*>(good) + 0x10) : -1;
      uint64_t prod = 0, cons = 0;
      for (int off : {0xC8, 0x78}) {
        auto* c = static_cast<uint8_t*>(state) + off;
        uint64_t mask = 0;
        uint64_t value = 0;
        if (goodId >= 0 && goodId < 4096) {
          mask = *reinterpret_cast<uint64_t*>(c + 0x20 + (int64_t)(goodId >> 6) * 8);
          if ((mask >> (goodId & 63)) & 1) {
            const auto base = *reinterpret_cast<uint64_t*>(c + 8);
            value = *reinterpret_cast<uint64_t*>(base + (int64_t)goodId * 8);
          }
        }
        if (off == 0xC8) prod = value; else cons = value;
        if (n < 3)
          V3_INFO("eco direct state=%d good=%d off=0x%x valid=%d value=%llu (0x%llx)", stateId,
                  goodId, off, int((mask >> (goodId & 63)) & 1), (unsigned long long)value,
                  (unsigned long long)value);
      }
      if (goodId != sumGood) {
        sumGood = goodId;
        sumProd = sumCons = 0;
        sumCount = 0;
        seenStates.clear();
      }
      if (seenStates.insert(stateId).second) {
        sumProd += prod;
        sumCons += cons;
        if (collectedStates.insert(stateId).second) {
          EconStateDump d;
          d.id = stateId;
          for (int g = 0; g < 128; ++g) {
            for (int off : {0xC8, 0x78}) {
              auto* c = static_cast<uint8_t*>(state) + off;
              const uint64_t mask =
                  *reinterpret_cast<uint64_t*>(c + 0x20 + (int64_t)(g >> 6) * 8);
              if (((mask >> (g & 63)) & 1) == 0) continue;
              const auto base = *reinterpret_cast<uint64_t*>(c + 8);
              const uint64_t v = *reinterpret_cast<uint64_t*>(base + (int64_t)g * 8);
              if (!v || v >= 1000000000000ull) continue;
              (off == 0xC8 ? d.prod[g] : d.cons[g]) = v;
            }
          }
          {
            std::lock_guard<std::mutex> lock(econMutex);
            econStates.push_back(std::move(d));
            if (econStates.size() % 25 == 0) writeDump();
          }
        }
        if (++sumCount % 100 == 0)
          V3_INFO("eco sum good=%d states=%d prod=%llu cons=%llu", goodId, sumCount,
                  (unsigned long long)sumProd, (unsigned long long)sumCons);
        // Enumerate every real state once by vtable+id, so the snapshot is not
        // limited to the visible page of the local-prices list.
        static std::atomic<bool> allStatesEnumerated{false};
        if (!allStatesEnumerated.exchange(true)) {
          void* vt = *reinterpret_cast<void**>(state);
          int found = 0;
          for (int id = 0; id < 2048; ++id) {
            int sid = id;
            void* s = lookup(&sid);
            if (!s || *reinterpret_cast<void**>(s) != vt) continue;
            if (*reinterpret_cast<int*>(static_cast<uint8_t*>(s) + 8) != id) continue;
            EconStateDump d;
            d.id = id;
            bool any = false;
            for (int g = 0; g < 128; ++g) {
              for (int off : {0xC8, 0x78}) {
                auto* c = static_cast<uint8_t*>(s) + off;
                const uint64_t mask =
                    *reinterpret_cast<uint64_t*>(c + 0x20 + (int64_t)(g >> 6) * 8);
                if (((mask >> (g & 63)) & 1) == 0) continue;
                const auto base = *reinterpret_cast<uint64_t*>(c + 8);
                if (!base) continue;
                const uint64_t v = *reinterpret_cast<uint64_t*>(base + (int64_t)g * 8);
                if (!v || v >= 1000000000000ull) continue;
                (off == 0xC8 ? d.prod[g] : d.cons[g]) = v;
                any = true;
              }
            }
            if (!any) continue;
            std::lock_guard<std::mutex> lock(econMutex);
            if (collectedStates.insert(id).second) {
              econStates.push_back(std::move(d));
              ++found;
            }
          }
          {
            std::lock_guard<std::mutex> lock(econMutex);
            writeDump();
          }
          V3_INFO("eco all states found=%d total=%zu", found, econStates.size());
          static std::atomic<bool> dumpedSameMarket{false};
          if (!dumpedSameMarket.exchange(true)) {
            for (int id : {16, 17, 18, 132, 432, 205}) {
              int sid = id;
              void* s = lookup(&sid);
              if (!s || *reinterpret_cast<void**>(s) != vt) continue;
              uint64_t w[64]{};
              std::memcpy(w, s, sizeof(w));
              for (int i = 0; i < 64; i += 4)
                V3_INFO("eco sm id=%d off=0x%02x %016llx %016llx %016llx %016llx", id, i * 8,
                        (unsigned long long)w[i], (unsigned long long)w[i + 1],
                        (unsigned long long)w[i + 2], (unsigned long long)w[i + 3]);
              for (int poff : {0x80, 0xa8, 0xd0}) {
                void* p = *reinterpret_cast<void**>(static_cast<uint8_t*>(s) + poff);
                if (!p) continue;
                MEMORY_BASIC_INFORMATION mbi{};
                if (!VirtualQuery(p, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT ||
                    (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)))
                  continue;
                uint64_t pw[16]{};
                std::memcpy(pw, p, sizeof(pw));
                for (int i = 0; i < 16; i += 4)
                  V3_INFO("eco smp id=%d base=0x%02x off=0x%02x %016llx %016llx %016llx %016llx",
                          id, poff, i * 8, (unsigned long long)pw[i],
                          (unsigned long long)pw[i + 1], (unsigned long long)pw[i + 2],
                          (unsigned long long)pw[i + 3]);
              }
            }
          }
        }
      }
    }
  }
  if (n < 8 && out) {
    uint64_t post[8]{};
    std::memcpy(post, out, sizeof(post));
    V3_INFO("eco reader post ret=%p out=%p words=%016llx %016llx %016llx %016llx %016llx "
            "%016llx %016llx %016llx",
            r, out, (unsigned long long)post[0], (unsigned long long)post[1],
            (unsigned long long)post[2], (unsigned long long)post[3],
            (unsigned long long)post[4], (unsigned long long)post[5],
            (unsigned long long)post[6], (unsigned long long)post[7]);
    if (post[1]) {
      uint64_t obj[8]{};
      std::memcpy(obj, reinterpret_cast<const void*>(post[1]), sizeof(obj));
      V3_INFO("eco reader obj  %p words=%016llx %016llx %016llx %016llx %016llx %016llx "
              "%016llx %016llx",
              reinterpret_cast<const void*>(post[1]), (unsigned long long)obj[0],
              (unsigned long long)obj[1], (unsigned long long)obj[2],
              (unsigned long long)obj[3], (unsigned long long)obj[4],
              (unsigned long long)obj[5], (unsigned long long)obj[6],
              (unsigned long long)obj[7]);
    }
  }
  return r;
}
void __fastcall EconomyCallbackThunk(void* ctx, void* result, void* args) {
  const uint64_t n = g_economyCallbackCalls.fetch_add(1);
  void* data = args ? *reinterpret_cast<void**>(args) : nullptr;
  if (n < 5 && ctx) {
    const auto* c = static_cast<const uint8_t*>(ctx);
    uint64_t w[6]{};
    std::memcpy(w, c, sizeof(w));
    V3_INFO("eco ctx words=%016llx %016llx %016llx %016llx %016llx %016llx",
            (unsigned long long)w[0], (unsigned long long)w[1], (unsigned long long)w[2],
            (unsigned long long)w[3], (unsigned long long)w[4], (unsigned long long)w[5]);
    if (data) {
      const auto* d = static_cast<const uint8_t*>(data);
      uint64_t dw[10]{};
      std::memcpy(dw, d, sizeof(dw));
      V3_INFO("eco args data=%p words=%016llx %016llx %016llx %016llx %016llx %016llx "
              "%016llx %016llx %016llx %016llx",
              data, (unsigned long long)dw[0], (unsigned long long)dw[1],
              (unsigned long long)dw[2], (unsigned long long)dw[3],
              (unsigned long long)dw[4], (unsigned long long)dw[5],
              (unsigned long long)dw[6], (unsigned long long)dw[7],
              (unsigned long long)dw[8], (unsigned long long)dw[9]);
    }
  }
  if (g_origEconomyCallback) g_origEconomyCallback(ctx, result, args);
  if (n < 5 && result) {
    uint64_t rw[4]{};
    std::memcpy(rw, result, sizeof(rw));
    V3_INFO("eco result=%p words=%016llx %016llx %016llx %016llx", result,
            (unsigned long long)rw[0], (unsigned long long)rw[1], (unsigned long long)rw[2],
            (unsigned long long)rw[3]);
  }
}

constexpr const char* kWindowTitle = "Ironroad debug - Victoria 3";

bool IsMouseMessage(UINT msg) {
  switch (msg) {
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
    case WM_XBUTTONDOWN: case WM_XBUTTONUP: case WM_XBUTTONDBLCLK:
    case WM_MOUSEWHEEL: case WM_MOUSEHWHEEL:
    case WM_SETCURSOR:
      return true;
    default:
      return false;
  }
}

}  // namespace

Overlay& Overlay::Instance() {
  // Hooks and registered native contexts are process-lifetime. In particular,
  // do not join a background thread from a CRT destructor under loader lock.
  static Overlay* o = new Overlay;
  return *o;
}

bool Overlay::Install(std::string* error) {
  Game& game = Game::Instance();

  HMODULE self = nullptr;
  GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                     (LPCWSTR)&Overlay::WndProcThunk, &self);
  m_dataDir = DirectoryOf(ModulePath(self));
  if (m_dataDir.empty()) m_dataDir = DirectoryOf(ModulePath(nullptr));
  m_dataDir += L"\\ironroad";
  if (!RuntimeDataDirectory().empty()) m_dataDir = RuntimeDataDirectory();
  m_settingsPath = m_dataDir + L"\\freight.ini";
  if (!RuntimeDataDirectory().empty()) {
    const auto root = DirectoryOf(RuntimeDataDirectory());
    if (!root.empty()) m_settingsPath = root + L"\\freight.ini";
  }
  LoadSettings();
  wchar_t timing[2]{};
  LARGE_INTEGER frequency{};
  m_frameTiming=GetEnvironmentVariableW(L"IRONROAD_FRAME_TIMING",timing,2)==1 && timing[0]==L'1' &&
      QueryPerformanceFrequency(&frequency) && frequency.QuadPart>0;
  if(m_frameTiming){
    m_frameTimingFrequency=uint64_t(frequency.QuadPart);
    V3_INFO("FRAME PERF enabled frequency=%llu batch=64 metric=Present-entry-interval",
        (unsigned long long)m_frameTimingFrequency);
  }
  auto geography = std::make_shared<RouteGeography>();
  { std::string why; if (!geography->splines.Load(m_dataDir, &why)) V3_WARN("authored routing unavailable: %s", why.c_str()); }

  if (LoadRouteData(m_dataDir, &geography->states, &geography->heights, &geography->provinces, &m_stateError)) {
    m_stateDataLoaded = true;
    m_originState = std::min<int>(m_originState, (int)geography->states.size() - 1);
    m_destState = std::min<int>(std::max(1, m_destState), (int)geography->states.size() - 1);
  } else {
    V3_ERR("route data unavailable: %s", m_stateError.c_str());
  }
  m_geography = std::move(geography);
  if (m_stateDataLoaded) {
    try { m_preview = std::make_unique<PreviewWorker>(m_geography); }
    catch (const std::exception& e) { m_stateError = e.what(); m_stateDataLoaded = false; }
  }
  if (m_stateDataLoaded) {
    // The session owns the freight worker and caches. It has no live snapshot
    // source until a verified economy access window exists; until then the UI
    // must keep showing unavailable values instead of invented numbers.
    try { m_logisticsSession = std::make_unique<LogisticsSession>(m_geography, FreightPolicy{}); }
    catch (const std::exception& e) { V3_WARN("freight session unavailable: %s", e.what()); }
    if (m_logisticsSession && m_settings.testSnapshot) {
      // Restored debug setting: re-create the same synthetic input, still
      // labelled as a test value everywhere it is shown.
      std::string reason;
      auto testInput = BuildSyntheticSnapshot(*m_geography);
      testInput.world = m_economyWorld.load();
      testInput.revision = ++m_economyRevision;
      auto snapshot = std::make_shared<const EconomySnapshot>(std::move(testInput));
      if (!m_logisticsSession->SetSnapshot(snapshot, &reason, true))
        V3_WARN("synthetic snapshot rejected: %s", reason.c_str());
      else
        V3_INFO("synthetic freight test snapshot restored (%zu orders)", snapshot->orders.size());
    }
  }
  auto hookInit = MH_Initialize();
  if (hookInit != MH_OK && hookInit != MH_ERROR_ALREADY_INITIALIZED) {
    if (error) *error = "MinHook initialization failed";
    return false;
  }

  if (game.Ready()) {
    uint8_t* base = game.Base();
    if (m_settings.nativeVariables && m_stateDataLoaded) {
      void* end=base+kRvaAccessScopeEnd;
      void* retire=base+kRvaDestroyNativeWorld;
      if (MH_CreateHook(retire,(LPVOID)&Overlay::DestroyNativeWorldThunk,&m_origDestroyNativeWorld)==MH_OK &&
          MH_EnableHook(retire)==MH_OK &&
          MH_CreateHook(end,(LPVOID)&Overlay::AccessScopeEndThunk,&m_origAccessScopeEnd)==MH_OK &&
          MH_EnableHook(end)==MH_OK) {
        m_nativeCaptureHooked.store(true);
        RequestNativeSnapshot();
        V3_INFO("native world retirement and completed command snapshot hooks installed");
      } else V3_WARN("native world retirement/capture hook failed; economy unavailable");
    }
    if (m_settings.probeAccess) {
      void* tick=base+kRvaPartialDailyTick;
      if (MH_CreateHook(tick,(LPVOID)&PartialDailyTickThunk,
                        (LPVOID*)&g_origPartialDailyTick)==MH_OK && MH_EnableHook(tick)==MH_OK)
        V3_INFO("native access tick probe installed (16 observations only)");
      else V3_WARN("native access tick probe hook failed");
      constexpr uint8_t straitSignature[] = {0x40,0x53,0x55,0x57,0x41,0x56,0x48,0x83};
      void* permission=base+0x00BE7C10;
      if(std::memcmp(permission,straitSignature,sizeof(straitSignature))!=0)
        V3_WARN("native strait probe refused: signature mismatch");
      else if(MH_CreateHook(permission,(LPVOID)&StraitPermissionProbe,(LPVOID*)&g_origStraitPermission)==MH_OK &&
              MH_EnableHook(permission)==MH_OK)
        V3_INFO("native strait probe installed (32 observations only)");
      else V3_WARN("native strait probe hook failed");
      constexpr uint8_t classificationSignature[]={0x48,0x89,0x4c,0x24,0x08,0x56,0x57,0x41,0x54,0x41,0x55,0x41,0x56};
      void* classification=base+0x0141FDA0;
      if(std::memcmp(classification,classificationSignature,sizeof(classificationSignature))!=0)
        V3_WARN("native strait classification probe refused: signature mismatch");
      else if(g_origStraitPermission &&
              MH_CreateHook(classification,(LPVOID)&StraitClassificationProbe,(LPVOID*)&g_origStraitClassification)==MH_OK &&
              MH_EnableHook(classification)==MH_OK)
        V3_INFO("native strait classification probe installed (owned outputs, diagnostic only)");
      else V3_WARN("native strait classification probe hook failed");
      constexpr uint8_t buildSignature[]={0x48,0x89,0x4c,0x24,0x08,0x55,0x53,0x56,0x57,0x41,0x54,0x41,0x55};
      constexpr uint8_t constructorSignature[]={0x4c,0x89,0x44,0x24,0x18,0x48,0x89,0x54,0x24,0x10,0x53,0x41,0x54};
      void* build=base+0x0140D830;void* constructor=base+0x0140FAA0;
      if(std::memcmp(build,buildSignature,sizeof(buildSignature))!=0 ||
         std::memcmp(constructor,constructorSignature,sizeof(constructorSignature))!=0)
        V3_WARN("native domestic lane probe refused: signature mismatch");
      else if(g_origStraitClassification &&
              MH_CreateHook(constructor,(LPVOID)&ShippingConstructorProbe,(LPVOID*)&g_origShippingConstructor)==MH_OK &&
              MH_CreateHook(build,(LPVOID)&DomesticLaneBuildProbe,(LPVOID*)&g_origDomesticLaneBuild)==MH_OK &&
              MH_EnableHook(constructor)==MH_OK && MH_EnableHook(build)==MH_OK)
        V3_INFO("native domestic lane probe installed (natural builds, diagnostic only)");
      else V3_WARN("native domestic lane probe hook failed");
    }
    if (m_settings.probePanelMarket) {
      constexpr uint8_t expected[] = {0x40,0x53,0x48,0x83,0xEC,0x30,0x48,0x83,0xC1,0x70};
      void* getter = base + 0x0225AC90;
      if (std::memcmp(getter, expected, sizeof(expected)) != 0) {
        V3_WARN("panel market probe refused: getter signature mismatch");
      } else if (MH_CreateHook(getter, (LPVOID)&PanelMarketThunk,
                               (LPVOID*)&g_origPanelMarket) == MH_OK &&
                 MH_EnableHook(getter) == MH_OK) {
        V3_INFO("panel market getter probe installed (observation only)");
      } else {
        V3_WARN("panel market getter probe hook failed");
      }
    }
    if (m_settings.probeRegister) {
      void* reg = base + kRvaScriptRegister;
      if (MH_CreateHook(reg, (LPVOID)&ScriptRegisterThunk,
                        (LPVOID*)&g_origScriptRegister) == MH_OK &&
          MH_EnableHook(reg) == MH_OK) {
        V3_INFO("script register probe installed at %p", reg);
      } else {
        V3_WARN("script register probe hook failed");
      }
    }
    if (MH_CreateHook(base + kRvaRenderPass, (LPVOID)&Overlay::PassThunk, &m_origPass) == MH_OK &&
        MH_CreateHook(base + kRvaSetPath, (LPVOID)&Overlay::SetPathThunk, &m_origSetPath) == MH_OK &&
        MH_CreateHook(base + kRvaBucketAppend, (LPVOID)&Overlay::AppendThunk, &m_origAppend) == MH_OK) {
      m_nativeHooks = MH_EnableHook(MH_ALL_HOOKS) == MH_OK;      V3_INFO("native hooks installed (render pass %p, set path %p, append %p)",
              base + kRvaRenderPass, base + kRvaSetPath, base + kRvaBucketAppend);
    } else {
      V3_ERR("native hook creation failed");
    }
    if (m_settings.probeEconomy) {
      void* target = base + 0x019791F0;
      if (MH_CreateHook(target, (LPVOID)&EconomyCallbackThunk,
                        (LPVOID*)&g_origEconomyCallback) == MH_OK &&
          MH_EnableHook(target) == MH_OK) {
        V3_INFO("economy production probe installed at %p", target);
        void* reader = base + 0x0195BA00;
        if (MH_CreateHook(reader, (LPVOID)&ProductionReaderThunk,
                          (LPVOID*)&g_origProductionReader) == MH_OK &&
            MH_EnableHook(reader) == MH_OK) {
          V3_INFO("economy reader probe installed at %p", reader);
        } else {
          V3_WARN("economy reader probe hook failed");
        }
        void* keyed = base + 0x0103F3C0;
        void* indexed = base + 0x0103F440;
        if (MH_CreateHook(keyed, (LPVOID)&ContainerKeyedThunk,
                          (LPVOID*)&g_origContainerKeyed) == MH_OK &&
            MH_EnableHook(keyed) == MH_OK) {
          V3_INFO("economy keyed container probe installed at %p", keyed);
        } else {
          V3_WARN("economy keyed container hook failed");
        }
        if (MH_CreateHook(indexed, (LPVOID)&ContainerIndexedThunk,
                          (LPVOID*)&g_origContainerIndexed) == MH_OK &&
            MH_EnableHook(indexed) == MH_OK) {
          V3_INFO("economy indexed container probe installed at %p", indexed);
        } else {
          V3_WARN("economy indexed container hook failed");
        }
      } else {
        V3_WARN("economy production probe hook failed");
      }
    }
  } else {
    V3_WARN("native profile not ready; arrow hooks disabled");
  }

  // Build a throwaway swap chain to resolve IDXGISwapChain::Present. This runs on
  // the bootstrap thread, never under the loader lock.
  WNDCLASSEXW wc{};
  wc.cbSize = sizeof(wc);
  wc.lpfnWndProc = DefWindowProcW;
  wc.hInstance = GetModuleHandleW(nullptr);
  wc.lpszClassName = L"IronroadProbeWindow";
  RegisterClassExW(&wc);
  HWND probe = CreateWindowExW(0, L"IronroadProbeWindow", L"", WS_POPUP, 0, 0, 8, 8, nullptr,
                               nullptr, wc.hInstance, nullptr);
  if (!probe) {
    if (error) *error = "probe window creation failed";
    return false;
  }

  DXGI_SWAP_CHAIN_DESC desc{};
  desc.BufferCount = 1;
  desc.BufferDesc.Width = 8;
  desc.BufferDesc.Height = 8;
  desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  desc.OutputWindow = probe;
  desc.SampleDesc.Count = 1;
  desc.Windowed = TRUE;
  desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  IDXGISwapChain* probeSwap = nullptr;
  ID3D11Device* probeDevice = nullptr;
  ID3D11DeviceContext* probeContext = nullptr;
  D3D_FEATURE_LEVEL level{};
  HRESULT hr = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, &desc,
      &probeSwap, &probeDevice, &level, &probeContext);
  if (FAILED(hr) || !probeSwap) {
    DestroyWindow(probe);
    UnregisterClassW(L"IronroadProbeWindow", wc.hInstance);
    if (error) *error = "probe D3D11 device creation failed";
    return false;
  }

  void** vt = *(void***)probeSwap;
  void* presentAddr = vt[8];
  void* present1Addr = vt[22];
  MH_STATUS s1 = MH_CreateHook(presentAddr, (LPVOID)&Overlay::PresentThunk, &m_origPresent);
  MH_STATUS s2 = MH_CreateHook(present1Addr, (LPVOID)&Overlay::Present1Thunk, &m_origPresent1);
  if (s1 == MH_OK) s1 = MH_EnableHook(presentAddr);
  if (s2 == MH_OK) s2 = MH_EnableHook(present1Addr);
  m_presentHooked = (s1 == MH_OK) || (s2 == MH_OK);

  probeSwap->Release();
  probeContext->Release();
  probeDevice->Release();
  DestroyWindow(probe);
  UnregisterClassW(L"IronroadProbeWindow", wc.hInstance);

  if (!m_presentHooked) {
    if (error) *error = "Present hook installation failed";
    return false;
  }
  V3_INFO("present hook installed (present=%p present1=%p)", presentAddr, present1Addr);
  return true;
}

void Overlay::LoadSettings() {
  std::string text;
  std::vector<uint8_t> bytes;
  // Canonical root first; fall back to the legacy map-data copy for a smooth
  // migration after the path fix.
  if (!ReadFileBytes(m_settingsPath, &bytes) &&
      !ReadFileBytes(m_dataDir + L"\\freight.ini", &bytes)) {
    bytes.clear();
  }
  if (!bytes.empty()) text.assign((const char*)bytes.data(), bytes.size());
  if (!text.empty() && m_settingsPath.empty()) V3_WARN("settings path unavailable");
  std::istringstream in(text);
  std::string line;
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    size_t eq = line.find('=');
    if (eq == std::string::npos) continue;
    std::string k = line.substr(0, eq), v = line.substr(eq + 1);
    if (k == "arrow_type") m_settings.arrowType = v;
    else if (k == "height_scale") m_settings.heightScale = (float)atof(v.c_str());
    else if (k == "height_offset") m_settings.heightOffset = (float)atof(v.c_str());
    else if (k == "coord_scale") m_settings.coordScale = (float)atof(v.c_str());
    else if (k == "native_enabled") m_settings.nativeEnabled = atoi(v.c_str()) != 0;
    else if (k == "route_mode") m_settings.routeMode = atoi(v.c_str());
    else if (k == "auto_test") m_settings.autoTest = atoi(v.c_str()) != 0;
    else if (k == "preview_consumption") m_settings.previewConsumption = (float)atof(v.c_str());
    else if (k == "test_snapshot") m_settings.testSnapshot = atoi(v.c_str()) != 0;
    else if (k == "probe_variables") m_settings.probeVariables = atoi(v.c_str()) != 0;
    else if (k == "probe_write") m_settings.probeWrite = atoi(v.c_str()) != 0;
    else if (k == "native_variables") m_settings.nativeVariables = atoi(v.c_str()) != 0;
    else if (k == "test_hide_variable") m_settings.testHideVariable = atoi(v.c_str()) != 0;
    else if (k == "probe_economy") m_settings.probeEconomy = atoi(v.c_str()) != 0;
    else if (k == "probe_access") m_settings.probeAccess = atoi(v.c_str()) != 0;
    else if (k == "test_world_change") m_settings.testWorldChange = atoi(v.c_str()) != 0;
    else if (k == "probe_market") m_settings.probeMarket = atoi(v.c_str()) != 0;
    else if (k == "market_override") m_settings.marketOverride = atoi(v.c_str());
    else if (k == "probe_register") m_settings.probeRegister = atoi(v.c_str()) != 0;
    else if (k == "probe_registry") m_settings.probeRegistry = atoi(v.c_str()) != 0;
    else if (k == "probe_panel_market") m_settings.probePanelMarket = atoi(v.c_str()) != 0;
    else if (k == "piggyback") m_piggyback = atoi(v.c_str()) != 0;
  }
  if (m_settings.routeMode < 0 || m_settings.routeMode > 2) m_settings.routeMode = 2;
  if (m_settings.arrowType == "supplyRouteArrowFlatMap") m_settings.arrowType = "ironroadBuildingOutput";
  if (m_settings.arrowType == "ironroadGreen") m_settings.arrowType = "ironroadBuildingOutput";
  if (m_settings.arrowType == "ironroadRed") m_settings.arrowType = "ironroadBuildingInput";
  if (m_settings.arrowType == "ironroadYellow") m_settings.arrowType = "ironroadBuildingYellow";
  const char* allowed[] = {"supplyRouteArrow", "supplyRouteArrowFlatMap", "ironroadBuildingOutput", "ironroadBuildingInput", "ironroadBuildingYellow", "ironroadGreen", "ironroadRed", "ironroadYellow", "ironroadBlue", "ironroadTrafficGreen", "ironroadTrafficLime", "ironroadTrafficYellow", "ironroadTrafficOrange", "ironroadTrafficRed"};
  if (std::none_of(std::begin(allowed), std::end(allowed), [&](const char* s) { return m_settings.arrowType == s; }))
    m_settings.arrowType = "ironroadBuildingOutput";
  if (!(m_settings.coordScale > 0.01f && m_settings.coordScale < 100.0f)) m_settings.coordScale = 1.0f;
  if (!(m_settings.heightScale >= 0.0f && m_settings.heightScale < 100.0f)) m_settings.heightScale = 0.0f;
  if (!(m_settings.previewConsumption > 0 && m_settings.previewConsumption <= 1.e9f)) m_settings.previewConsumption = 100;
  V3_INFO("settings: type='%s' coordScale=%.3f heightScale=%.4f heightOffset=%.2f mode=%d",
          m_settings.arrowType.c_str(), m_settings.coordScale, m_settings.heightScale,
          m_settings.heightOffset, m_settings.routeMode);
  V3_INFO("settings flags: native=%d world_change=%d probe_econ=%d test_snapshot=%d",
          int(m_settings.nativeVariables), int(m_settings.testWorldChange),
          int(m_settings.probeEconomy), int(m_settings.testSnapshot));
}

void Overlay::SaveSettings() {
  std::ostringstream out;
  out << "building_flow_version=1\n"
      << "arrow_type=" << m_settings.arrowType << "\n"
      << "height_scale=" << m_settings.heightScale << "\n"
      << "height_offset=" << m_settings.heightOffset << "\n"
      << "coord_scale=" << m_settings.coordScale << "\n"
      << "native_enabled=" << (m_settings.nativeEnabled ? 1 : 0) << "\n"
      << "route_mode=" << m_settings.routeMode << "\n"
      << "auto_test=" << (m_settings.autoTest ? 1 : 0) << "\n"
      << "preview_consumption=" << m_settings.previewConsumption << "\n"
      << "test_snapshot=" << (m_settings.testSnapshot ? 1 : 0) << "\n"
      << "probe_variables=" << (m_settings.probeVariables ? 1 : 0) << "\n"
      << "probe_write=" << (m_settings.probeWrite ? 1 : 0) << "\n"
      << "native_variables=" << (m_settings.nativeVariables ? 1 : 0) << "\n"
      << "test_hide_variable=" << (m_settings.testHideVariable ? 1 : 0) << "\n"
      << "probe_economy=" << (m_settings.probeEconomy ? 1 : 0) << "\n"
      << "probe_access=" << (m_settings.probeAccess ? 1 : 0) << "\n"
      << "test_world_change=" << (m_settings.testWorldChange ? 1 : 0) << "\n"
      << "probe_market=" << (m_settings.probeMarket ? 1 : 0) << "\n"
      << "probe_registry=" << (m_settings.probeRegistry ? 1 : 0) << "\n"
      << "probe_panel_market=" << (m_settings.probePanelMarket ? 1 : 0) << "\n"
      << "market_override=" << m_settings.marketOverride << "\n"
      << "piggyback=" << (m_piggyback ? 1 : 0) << "\n";
  std::string text = out.str();
  const auto parent = DirectoryOf(m_settingsPath);
  if (!parent.empty()) {
    std::error_code ignored;
    std::filesystem::create_directories(parent, ignored);
  }
  const auto temporary = m_settingsPath + L".tmp-" + std::to_wstring(GetCurrentProcessId());
  HANDLE f = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr,
                         CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  DWORD wrote = 0;
  bool ok = f != INVALID_HANDLE_VALUE &&
      WriteFile(f, text.data(), (DWORD)text.size(), &wrote, nullptr) && wrote == text.size();
  if (f != INVALID_HANDLE_VALUE) CloseHandle(f);
  if (ok) ok = MoveFileExW(temporary.c_str(), m_settingsPath.c_str(),
      MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
  if (!ok) {
    DeleteFileW(temporary.c_str());
    V3_ERR("settings save failed: %ls", m_settingsPath.c_str());
    return;
  }
  V3_INFO("settings saved");
}

void Overlay::RequestRoute(int from, int to) {
  if (!m_preview || from < 0 || to < 0 ||
      from >= int(m_geography->states.size()) || to >= int(m_geography->states.size())) return;
  PreviewRequest request;
  request.from = m_geography->states[from].id;
  request.to = m_geography->states[to].id;
  request.mode = m_settings.routeMode; request.style = m_settings.arrowType;
  request.coordScale = m_settings.coordScale; request.heightScale = m_settings.heightScale;
  request.heightOffset = m_settings.heightOffset;
  request.previewConsumption = m_settings.previewConsumption;
  m_generation = m_preview->Request(std::move(request));
  m_wantsRoute = true;
  m_from = from; m_to = to;
}

void Overlay::ClearRoute() {
  if (m_preview) m_generation = m_preview->Cancel();
  m_wantsRoute = false;
  // Any freight ribbons that were requested into the shared pool stop being
  // submitted; their slots stay cached for a later identical view.
  m_freightKey = 0;
}

void Overlay::SetArrowType(const char* name) {
  if (!name || m_settings.arrowType == name) return;
  static const char* allowed[] = {"supplyRouteArrow", "supplyRouteArrowFlatMap", "ironroadBuildingOutput", "ironroadBuildingInput", "ironroadBuildingYellow", "ironroadGreen", "ironroadRed", "ironroadYellow", "ironroadBlue", "ironroadTrafficGreen", "ironroadTrafficLime", "ironroadTrafficYellow", "ironroadTrafficOrange", "ironroadTrafficRed"};
  if (std::none_of(std::begin(allowed), std::end(allowed), [&](const char* s) { return std::strcmp(s, name) == 0; })) return;
  V3_INFO("switch arrow type '%s' -> '%s'", m_settings.arrowType.c_str(), name);
  const bool wasActive = m_wantsRoute.load();
  ClearRoute();
  m_settings.arrowType = name;
  SaveSettings();
  if (wasActive) RequestRoute(m_from, m_to);
}

int Overlay::FindState(const char* key) const {
  for (int i = 0; i < (int)m_geography->states.size(); ++i)
    if (m_geography->states[i].key == key) return i;
  return -1;
}

void Overlay::DeactivateRibbons() {
  if (NativeWorldRetired()) { m_routeActive = false; return; }
  for (size_t index : m_ribbons.Active()) {
    if (index < m_slots.size() && m_slots[index] && m_slots[index]->arrow)
      *(uint8_t*)((uint8_t*)m_slots[index]->arrow + kOffFlags) &= (uint8_t)~1u;
  }
  m_routeActive = false;
}

void Overlay::ApplyPendingRoute() {
  if (NativeWorldRetired()) return;
  if (!m_visible && m_logisticsSession &&
      IsNativeLogisticsOwner(m_logisticsSession->Status().owner)) {
    ApplyPendingFreight(); return;
  }
  if (m_viewMode != 0) { ApplyPendingFreight(); return; }
  ReportRibbonBatch("cancelled");
  if (!m_preview) return;
  const uint64_t generation = m_preview->Generation();
  if (generation != m_seenGeneration) {
    DeactivateRibbons(); m_ribbons.Clear();
    m_arrow = nullptr; m_slot = nullptr;
    m_seenGeneration = generation;
  }
  if (!m_visible || !m_settings.nativeEnabled || m_viewMode != 0) return;
  const auto reply = m_preview->Poll();
  if (reply && reply->generation == generation && generation == m_preview->Generation() &&
      reply->generation != m_builtGeneration) {
    const auto& result = reply->value;
    // A style lookup may become available after the worker finishes. Keep the
    // completed request without constructing failed objects every frame.
    if (reply->error.empty() && !result.ribbons.empty() && !m_type) return;
    DeactivateRibbons();
    m_ribbons.Request(reply->error.empty() ? result.ribbons : RibbonBatch{});
    m_builtGeneration = reply->generation;
    m_lastBuildMs = result.computeMs;
    const std::string& error = reply->error.empty() ? result.error : reply->error;
    if (!error.empty()) std::snprintf(m_lastResult, sizeof(m_lastResult), "%s", error.c_str());
    else std::snprintf(m_lastResult, sizeof(m_lastResult), "worker: %zu points, %d nodes, %.2f ms; route cache %s",
        result.rawPoints, result.usedNodes, result.computeMs, result.routeCacheHit ? "hit" : "miss");
    V3_INFO("preview generation=%llu %d -> %d mode=%d points=%zu compute=%.3f ms cache=%d error=%s",
        (unsigned long long)reply->generation, result.from, result.to, result.mode,
        result.rawPoints, result.computeMs, int(result.routeCacheHit), error.c_str());
  }
  if (m_preview->Generation() != m_builtGeneration) return;
  LARGE_INTEGER frequency; QueryPerformanceFrequency(&frequency);
  m_ribbons.Pump(Game::Instance().Frame(), [this](size_t index, const RibbonGeometry& ribbon) {
    return UploadRibbon(index, ribbon);
  }, [&frequency] {
    LARGE_INTEGER now; QueryPerformanceCounter(&now);
    return double(now.QuadPart) / double(frequency.QuadPart);
  }, [this, generation] { return generation != m_preview->Generation(); });
  if (generation != m_preview->Generation()) {
    DeactivateRibbons(); m_ribbons.Clear(); return;
  }
  m_cacheHits = m_ribbons.Hits();
  m_routeActive = !m_ribbons.Active().empty();
  for (size_t index : m_ribbons.Active()) {
    auto& slot = *m_slots[index];
    *(uint8_t*)((uint8_t*)slot.arrow + kOffFlags) |= 1;
  }
  if (m_routeActive) {
    m_slot = m_slots[m_ribbons.Active().front()].get(); m_arrow = m_slot->arrow;
  }
}

void Overlay::ReportRibbonBatch(const char* state) {
  if (!m_ribbonBatchOpen) return;
  const auto stats=m_ribbons.Stats();
  const auto failures=stats.failures-m_ribbonBatchStart.failures;
  V3_INFO("DRAW PERF paths=%zu uploads=%llu reused=%llu failed=%llu pool=%zu cpu_us=%llu repeats_skipped=%llu "
          "state=%s gen=%llu world=%llu rev=%llu pending=%zu visible=%zu dropped=%llu native_slots=%zu "
          "peak_frame_us=%llu peak_step_us=%llu peak_attempts=%u overrun_frames=%llu",
          m_ribbons.Requested(),
          (unsigned long long)(stats.attempts-m_ribbonBatchStart.attempts-failures),
          (unsigned long long)(stats.hits-m_ribbonBatchStart.hits),
          (unsigned long long)failures,m_ribbons.Size(),
          (unsigned long long)((stats.seconds-m_ribbonBatchStart.seconds)*1e6),
          (unsigned long long)stats.repeats,state,(unsigned long long)m_ribbonBatchKey,
          (unsigned long long)m_ribbonBatchWorld,(unsigned long long)m_ribbonBatchRevision,
          m_ribbons.Pending(),m_ribbons.Active().size(),
          (unsigned long long)(stats.dropped-m_ribbonBatchStart.dropped),m_slots.size(),
          (unsigned long long)(stats.peakFrameSeconds*1e6),
          (unsigned long long)(stats.peakStepSeconds*1e6),stats.peakFrameAttempts,
          (unsigned long long)stats.overrunFrames);
  m_ribbonBatchOpen=false;
}

void Overlay::ApplyPendingFreight() {
  if (NativeWorldRetired()) return;
  if (!m_logisticsSession) return;
  const auto status = m_logisticsSession->Status();
  auto frame = m_logisticsSession->Frame();
  const bool nativeOwner = IsNativeLogisticsOwner(status.owner);
  if ((!m_visible && !nativeOwner) || !m_settings.nativeEnabled || m_hideFlows.load() || !frame ||
      !m_logisticsSession->FrameIsCurrent(frame, status.applied)) {
    if (m_freightKey) {
      ReportRibbonBatch("cancelled");
      DeactivateRibbons();
      m_freightKey = 0;
    }
    return;
  }
  if (status.applied != m_freightKey) {
    ReportRibbonBatch("superseded");
    DeactivateRibbons();
    m_ribbonBatchStart=m_ribbons.Stats(); m_ribbonBatchOpen=true;
    m_ribbonBatchKey=status.applied; m_ribbonBatchWorld=frame->world;
    m_ribbonBatchRevision=frame->economyRevision;
    m_ribbons.Request(frame->ribbons);
    m_freightKey = status.applied;
    m_lastBuildMs = frame->computeMs;
    std::snprintf(m_lastResult, sizeof(m_lastResult),
                  "%s %zu ribbons, %zu records, %.2f ms%s",frame->nativeFacts?"native trade":"model freight",
                  frame->ribbons.size(),
                  frame->nativeFacts?frame->nativeFacts->bindings.size():frame->allocation?frame->allocation->shipments.size():size_t(0),
                  frame->computeMs, status.synthetic ? " (TEST INPUT)" : "");
    V3_INFO("freight view gen=%llu world=%llu rev=%llu ribbons=%zu compute=%.3f ms synthetic=%d",
            (unsigned long long)status.applied, (unsigned long long)frame->world,
            (unsigned long long)frame->economyRevision, frame->ribbons.size(),
            frame->computeMs, int(status.synthetic));
  }
  LARGE_INTEGER frequency;
  QueryPerformanceFrequency(&frequency);
  const uint64_t key = m_freightKey;
  m_ribbons.Pump(Game::Instance().Frame(), [this](size_t index, const RibbonGeometry& ribbon) {
    return UploadRibbon(index, ribbon);
  }, [&frequency] {
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return double(now.QuadPart) / double(frequency.QuadPart);
  }, [this, key, nativeOwner, frame] {
    return (!m_visible && !nativeOwner) || m_freightKey != key ||
           !m_logisticsSession->FrameIsCurrent(frame, key);
  });
  if (m_freightKey != key || !m_logisticsSession->FrameIsCurrent(frame, key)) {
    ReportRibbonBatch("cancelled");
    DeactivateRibbons();
    m_freightKey = 0;
    return;
  }
  if (!m_ribbons.Pending()) ReportRibbonBatch("complete");
  m_cacheHits = m_ribbons.Hits();
  m_routeActive = !m_ribbons.Active().empty();
  for (size_t index : m_ribbons.Active()) {
    auto& slot = *m_slots[index];
    if (!slot.arrow) continue;
    *(uint8_t*)((uint8_t*)slot.arrow + kOffFlags) |= 1;
  }
  if (m_routeActive) {
    m_slot = m_slots[m_ribbons.Active().front()].get();
    m_arrow = m_slot->arrow;
  } else {
    m_slot = nullptr;
    m_arrow = nullptr;
  }
}

void Overlay::ProbeVariablesIfEnabled(bool force) {
  if (!m_settings.probeVariables && !force) return;
  Game& game = Game::Instance();
  if (!game.Ready()) return;
  const uint64_t now = GetTickCount64();
  if (!force && now - m_lastVarProbe < 2000) return;
  m_lastVarProbe = now;
  const auto sanitize = [](std::string text) {
    for (char& c : text)
      if (static_cast<unsigned char>(c) < 32 || static_cast<unsigned char>(c) > 126) c = '.';
    if (text.size() > 80) text.resize(80);
    return text;
  };
  const auto report = [&](const GuiVariableProbe& probe, const char* label,
                          std::set<uint32_t>& seen, std::string& summary, size_t& lastCount,
                          size_t& logged) {
    if (!probe.ok) {
      if (summary != probe.error) {
        summary = probe.error;
        V3_WARN("%s probe failed: %s", label, probe.error.c_str());
      }
      return;
    }
    bool changed = false;
    for (const auto& entry : probe.entries) {
      if (!seen.insert(entry.hash).second) continue;
      changed = true;
      if (logged < 40) {
        V3_INFO("%s key='%s' hash=%08x value='%s'", label, sanitize(entry.key).c_str(),
                entry.hash, sanitize(entry.value).c_str());
        ++logged;
      }
    }
    if (changed || probe.entries.size() != lastCount) {
      lastCount = probe.entries.size();
      char text[192];
      std::snprintf(text, sizeof(text), "obj=%p buckets=%u count=%d level=%u load=%.2f entries=%zu",
                    probe.object, probe.mask + 1, probe.count, probe.level, probe.loadFactor,
                    probe.entries.size());
      summary = text;
      V3_INFO("%s %s", label, summary.c_str());
    }
  };
  report(ProbeGuiVariables(game.Base()), "GUI variable map", m_varProbeSeen, m_varProbeSummary,
         m_varProbeCount, m_varProbeLogged);
  report(ProbeGlobalStringMap(game.Base()), "global string map", m_globalProbeSeen,
         m_globalProbeSummary, m_globalProbeCount, m_globalProbeLogged);
  if (m_settings.probeWrite && !m_writeTestDone) {
    m_writeTestDone = true;
    std::string error;
    const bool first = WriteGuiVariableProbe(game.Base(), "ironroad_probe_long_key", "long-value-1", &error);
    V3_INFO("GUI variable write test #1: %s%s%s", first ? "ok" : "failed",
            error.empty() ? "" : " - ", error.c_str());
    if (first) {
      error.clear();
      const bool second = WriteGuiVariableProbe(game.Base(), "ironroad_probe_long_key", "long-value-2", &error);
      V3_INFO("GUI variable write test #2 (update): %s%s%s", second ? "ok" : "failed",
              error.empty() ? "" : " - ", error.c_str());
    }
    m_lastVarProbe = 0;  // read the new entry back on the next tick
  }
}

bool Overlay::UploadRibbon(size_t index, const RibbonGeometry& geometry) {
  if (NativeWorldRetired()) return false;
  Game& game = Game::Instance();
  if (m_nativePoolDeviceChanged || !m_device || FAILED(m_device->GetDeviceRemovedReason())) return false;
  if (!m_nativePoolDevice) {
    SIZE_T copied=0;
    if (!ReadProcessMemory(GetCurrentProcess(),game.Base()+0x0592d5c8,&m_nativePoolGraphics,
                           sizeof(m_nativePoolGraphics),&copied) ||
        copied!=sizeof(m_nativePoolGraphics) || !m_nativePoolGraphics) return false;
    m_nativePoolDevice=m_device;
  }
  if (m_nativePoolDevice!=m_device) return false;
  if (index >= 8192) return false;
  if (index >= m_slots.size()) m_slots.resize(index + 1);
  if (!m_slots[index]) m_slots[index] = std::make_unique<NativeSlot>();
  auto& slot = *m_slots[index];
  if (!slot.style.empty() && slot.style != geometry.style) return false;
  if (!slot.arrow) {
    std::string error;
    void* type = game.ResolveArrowType(geometry.style.c_str(), &error);
    if (!type) { V3_WARN("ribbon type unavailable: %s", error.c_str()); return false; }
    slot.style = geometry.style;
    slot.arrow = game.CreateSupplyArrow(type);
    if (!slot.arrow) return false;
    slot.type = type;
    float* matrix = (float*)((uint8_t*)slot.arrow + kOffMatrix);
    std::memset(matrix, 0, 0x40);
    matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
    *(uint8_t*)((uint8_t*)slot.arrow + kOffFlags) |= 6;
  }
  if (!slot.pointBuffer) {
    slot.pointBuffer = game.AllocateGameMemory(4096 * sizeof(Vec3), 4);
    if (!slot.pointBuffer) return false;
  }
  // Path and point storage outlive native registration. Compatible slot reuse
  // relies on the verified SetPath VB/CB replacement paths, never a constructor.
  std::memcpy(slot.pointBuffer, geometry.points.data(), geometry.points.size() * sizeof(Vec3));
  slot.path.moveLength = 1.0e9f;
  slot.path.animationLength = slot.path.clipLength = slot.path.reserved = 0;
  slot.path.points = {slot.pointBuffer, 4096, (uint32_t)geometry.points.size(), game.GameAllocator()};
  m_inSetPath = true;
  const bool set = game.SetPath(slot.arrow, &slot.path);
  m_inSetPath = false;
  if (!set) return false;
  game.EnsureConstants(slot.arrow);
  ++m_uploadedRoutes;
  slot.vertex=*(void**)((uint8_t*)slot.arrow+kOffVertexBuffer);
  slot.constant=*(void**)((uint8_t*)slot.arrow+kOffConstantBuffer);
  const bool valid = slot.vertex && slot.constant;
  if (!valid) V3_ERR("ribbon slot %zu failed VB/CB creation; cached failure will not retry each frame", index);
  return valid;
}

bool Overlay::RecoverNativeWorld() {
  if (m_nativePoolDeviceChanged || !m_logisticsSession || !m_device ||
      FAILED(m_device->GetDeviceRemovedReason())) return false;
  const uint64_t world=m_economyWorld.load();
  const auto status=m_logisticsSession->Status();
  const auto frame=m_logisticsSession->Frame();
  const auto current=[&]{return world==m_economyWorld.load() && frame && !status.synthetic &&
      status.world==world && frame->world==world &&
      m_logisticsSession->FrameIsCurrent(frame,status.applied);};
  if (!current()) return false;
  const uint64_t now=GetTickCount64();
  if (now-m_nativeRecoveryAttempt<1000) return false;
  m_nativeRecoveryAttempt=now;
  const auto reader=[](uint64_t address,void* out,size_t bytes){
    SIZE_T copied=0;
    return address && bytes<=UINT64_MAX-address &&
        ReadProcessMemory(GetCurrentProcess(),(const void*)address,out,bytes,&copied) && copied==bytes;
  };
  if (m_slots.empty()) {
    if (!reader((uint64_t)Game::Instance().Base()+0x0592d5c8,&m_nativePoolGraphics,8)) return false;
    m_nativePoolDevice=m_device;
  }
  std::vector<NativeArrowLease> leases;
  for (const auto& slot:m_slots) if (slot && slot->arrow)
    leases.push_back({slot->style,(uint64_t)slot->arrow,(uint64_t)slot->type,
        (uint64_t)&slot->path,(uint64_t)slot->pointBuffer,(uint64_t)slot->vertex,(uint64_t)slot->constant});
  std::string error;
  const bool valid=m_nativePoolDevice==m_device && ValidateNativeArrowLeases(reader,
      (uint64_t)Game::Instance().Base(),m_nativePoolGraphics,leases,current,&error);
  if (!valid) {
    if (error.empty()) error="overlay graphics device changed";
    if (error!=m_nativeRecoveryError) {
      m_nativeRecoveryError=error;V3_WARN("native world recovery pending: %s",error.c_str());
    }
    return false;
  }
  // Only copied numeric work from this world may activate the retained pool.
  // No constructor, destructor, native lock or resource replacement is needed.
  m_ribbons.Clear();m_freightKey=0;m_routeActive=false;m_arrow=nullptr;m_slot=nullptr;
  m_type=nullptr;m_renderStyle.clear();m_submitGuard={};
  Game::Instance().ForgetArrowTypes();
  m_nativePoolWorld.store(world);
  if (NativeWorldRetired() || !current()) return false;
  m_nativeRecoveryError.clear();m_nativeRecoveryStatusDirty.store(true);
  V3_INFO("native pool recovered world=%llu slots=%zu; old requests cleared",
          (unsigned long long)world,leases.size());
  return true;
}

void __fastcall Overlay::PassThunk(void* ctx) {
  Overlay& o = Instance();
  if (o.m_inPass) {
    ((void(__fastcall*)(void*))o.m_origPass)(ctx);
    return;
  }
  o.m_inPass = true;
  o.OnRenderPass(ctx);
  ((void(__fastcall*)(void*))o.m_origPass)(ctx);
  o.m_inPass = false;
}

void Overlay::OnRenderPass(void* ctx) {
  ++m_passCalls;
  Game& game = Game::Instance();
  if (!game.Ready() || !ctx) return;
  const int32_t* passWords = (const int32_t*)ctx;
  if (m_passCalls <= 12) {
    int32_t domain, stage; game.ArrowSubmissionScope(&domain, &stage);
    V3_INFO("arrow pass #%llu scope=%d/%d/%d native-scope=%d/%d eligible=%d frame=%llu",
            (unsigned long long)m_passCalls, passWords[0], passWords[2], passWords[4],
            domain, stage, (int)game.IsArrowSubmissionPass(ctx), (unsigned long long)game.Frame());
  }
  // A frame includes other stages (e.g. 0/3/0) with their own camera/render
  // state. Selecting the first callback consumed the frame's only submission
  // before the vanilla arrow stage (observed 0/1/0) ever ran.
  if (!game.IsArrowSubmissionPass(ctx)) return;
  uint64_t graphics=0;SIZE_T graphicsBytes=0;
  const bool graphicsCurrent=!m_nativePoolDevice ||
      (ReadProcessMemory(GetCurrentProcess(),game.Base()+0x0592d5c8,&graphics,sizeof(graphics),&graphicsBytes) &&
       graphicsBytes==sizeof(graphics) && graphics==m_nativePoolGraphics);
  // Cached geometry also needs this check when the world itself did not change.
  // A new/lost device must never receive old native buffers through cache hits.
  if (m_nativePoolDevice && (!graphicsCurrent || m_nativePoolDeviceChanged || !m_device || m_nativePoolDevice!=m_device ||
                            FAILED(m_device->GetDeviceRemovedReason()))) {
    if (m_nativePoolWorld.exchange(0)!=0) {
      m_nativeRecoveryStatusDirty.store(true);
      V3_WARN("native pool retired: graphics identity changed, removed or unreadable");
    }
    return;
  }
  if (NativeWorldRetired() && !RecoverNativeWorld()) return;
  const bool nativeOwner = m_logisticsSession &&
      IsNativeLogisticsOwner(m_logisticsSession->Status().owner);
  if (!m_settings.nativeEnabled || (!m_visible && !nativeOwner)) { ApplyPendingRoute(); return; }
  ++m_matchingPassCalls;
  if (m_renderStyle != m_settings.arrowType) {
    DeactivateRibbons();
    m_renderStyle = m_settings.arrowType;
    m_slot = nullptr; m_arrow = nullptr;
    m_type = nullptr; m_lastTypeAttempt = 0; m_typeFailedFor.clear();
  }
  if (m_passCalls <= 3) {
    const uint32_t* tri = (const uint32_t*)ctx;
    V3_INFO("arrow pass #%llu entered ctx=%p tri=%u/%u/%u (frame %llu)",
            (unsigned long long)m_passCalls, ctx, tri[0], tri[2], tri[4],
            (unsigned long long)game.Frame());
  }

  // Resolve the arrow type as soon as the renderer is alive so the panel can
  // report whether the build's asset registry answers before any object is made.
  if (!m_type) {
    uint64_t now = GetTickCount64();
    if (!m_lastTypeAttempt || now - m_lastTypeAttempt >= 15000) {
      m_lastTypeAttempt = now;
      std::string err;
      m_type = game.ResolveArrowType(m_settings.arrowType.c_str(), &err);
      if (!m_type && m_typeFailedFor != m_settings.arrowType) {
        m_typeFailedFor = m_settings.arrowType;
        V3_WARN("arrow type '%s' unavailable (%s); retrying every 15 s. Check embedded "
                "asset startup registration and game shader errors.",
                m_settings.arrowType.c_str(), err.c_str());
      }
    }
  }

  // Auto-test: build one known short route after the renderer settles, so the
  // native pipeline is exercised even if nobody touches the panel.
  if (m_visible && m_viewMode == 0 && m_settings.autoTest && !m_autoTestDone && m_type && m_passCalls > 30) {
    m_autoTestDone = true;
    int a = FindState("STATE_HOME_COUNTIES");
    int b = FindState("STATE_LANCASHIRE");
    if (a < 0 || b < 0) {
      a = FindState("STATE_SVEALAND");
      b = FindState("STATE_GOTALAND");
    }
    if (a >= 0 && b >= 0) {
      m_originState = a;
      m_destState = b;
      V3_INFO("auto-test route %s -> %s (disable with auto_test=0)",
              m_geography->states[a].name.c_str(), m_geography->states[b].name.c_str());
      RequestRoute(a, b);
      // Capture the frame a few seconds later so the result is verifiable from
      // the log directory even without operator interaction.
      m_autoCaptureAt = GetTickCount64() + 3000;
    }
  }

  ApplyPendingRoute();
  if ((!m_visible && !nativeOwner) || !m_routeActive || !m_arrow) return;
  if (m_viewMode == 0 && (m_visible || !nativeOwner)) {
    if (!m_preview || m_preview->Generation() != m_builtGeneration) return;
  } else if (m_freightKey == 0) {
    return;
  } else if (!m_logisticsSession ||
             !m_logisticsSession->FrameIsCurrent(m_logisticsSession->Frame(), m_freightKey)) {
    return;
  }
  int32_t domain, stage; game.ArrowSubmissionScope(&domain, &stage);
  if (!m_submitGuard.Allow(game.Frame(), *(const ArrowPassContext*)ctx, domain, stage)) {
    ++m_skippedDuplicate;
    return;
  }
  m_submittingOwn = true;
  size_t submitted=0;
  for (size_t index : m_ribbons.Active()) {
    if (NativeWorldRetired()) break;
    game.Submit(m_slots[index]->arrow, ctx);++submitted;
  }
  m_submittingOwn = false;
  m_lastSubmitFrame = game.Frame();
  m_lastSubmitCtx = ctx;
  m_submitted += submitted;
  uint64_t now = GetTickCount64();
  if (now - m_lastPassSummary >= 5000) {
    m_lastPassSummary = now;
    V3_INFO("SUBMIT scope=%d/%d/%d frame=%llu pass=%llu matching=%llu submitted=%llu uploads=%llu cache=%llu",
            passWords[0], passWords[2], passWords[4], (unsigned long long)game.Frame(),
            (unsigned long long)m_passCalls, (unsigned long long)m_matchingPassCalls,
            (unsigned long long)m_submitted, (unsigned long long)m_uploadedRoutes,
            (unsigned long long)m_cacheHits);
  }
}

void __fastcall Overlay::SetPathThunk(void* arrow, void* path) {
  Overlay& o = Instance();
  if (!o.m_inSetPath && o.m_captureRemaining && arrow && path && arrow != o.m_arrow) {
    o.m_inSetPath = true;
    ArrowPath* p = (ArrowPath*)path;
    uint32_t n = p->points.count;
    if (n >= 2 && p->points.data) {
      const Vec3* pts = (const Vec3*)p->points.data;
      const Vec3& a = pts[0];
      const Vec3& b = pts[n - 1];
      void* type = *(void**)((uint8_t*)arrow + kOffType);
      float h0 = o.m_geography->heights.Valid() ? o.m_geography->heights.Sample(a.x, a.z) : 0.0f;
      float h1 = o.m_geography->heights.Valid() ? o.m_geography->heights.Sample(b.x, b.z) : 0.0f;
      V3_INFO("CAPTURE setpath obj=%p type=%p n=%u mv=%.1f anim=%.1f clip=%.1f "
              "first=(%.2f %.2f %.2f) last=(%.2f %.2f %.2f) heightmap=(%.1f %.1f) flags=%02x",
              arrow, type, n, p->moveLength, p->animationLength, p->clipLength, a.x, a.y, a.z,
              b.x, b.y, b.z, h0, h1, (unsigned)*(uint8_t*)((uint8_t*)arrow + kOffFlags));
      --o.m_captureRemaining;
    }
    o.m_inSetPath = false;
  }
  ((FnSetPath)o.m_origSetPath)(arrow, path);
}

// Diagnostic hook: observes the game's own append calls (and can piggyback our
// arrow on a verified native append context). The piggyback is a diagnostic only:
// correct submission happens from the render-pass hook with the pass's own context.
void __fastcall Overlay::AppendThunk(void* arrow, void* ctx) {
  Overlay& o = Instance();
  if (o.m_inAppend || o.m_submittingOwn) {
    ((FnAppend)o.m_origAppend)(arrow, ctx);
    return;
  }
  o.m_inAppend = true;
  if (o.m_appendLog < 40 && arrow != o.m_arrow) {
    uint32_t* tri = (uint32_t*)ctx;
    void* type = arrow ? *(void**)((uint8_t*)arrow + kOffType) : nullptr;
    V3_INFO("APPEND native arrow=%p type=%p bucket=%d ctx=%u/%u/%u", arrow, type,
            type ? (int)*(int16_t*)((uint8_t*)type + 0x30) : -1, tri ? tri[0] : 0,
            tri ? tri[2] : 0, tri ? tri[4] : 0);
    ++o.m_appendLog;
  }
  if (!o.NativeWorldRetired() && o.m_viewMode == 0 && o.m_piggyback && o.m_visible && o.m_settings.nativeEnabled && o.m_routeActive && o.m_arrow &&
      o.m_preview && o.m_preview->Generation() == o.m_builtGeneration && arrow != o.m_arrow && ctx &&
      o.m_piggyFrame != Game::Instance().Frame()) {
    o.m_piggyFrame = Game::Instance().Frame();
    ((FnAppend)o.m_origAppend)(arrow, ctx);
    // Diagnostic selection uses the same once-per-frame guard as the pass.
    int32_t domain, stage; Game::Instance().ArrowSubmissionScope(&domain, &stage);
    if (o.m_submitGuard.Allow(Game::Instance().Frame(), *(const ArrowPassContext*)ctx, domain, stage)) {
      for (size_t index : o.m_ribbons.Active()) ((FnAppend)o.m_origAppend)(o.m_slots[index]->arrow, ctx);
      o.m_lastSubmitFrame = Game::Instance().Frame();
      o.m_submitted += o.m_ribbons.Active().size();
    }
    o.m_inAppend = false;
    return;
  }
  o.m_inAppend = false;
  ((FnAppend)o.m_origAppend)(arrow, ctx);
}

bool Overlay::IsMainSwapchain(IDXGISwapChain* swap) {
  if (!swap) return false;
  DXGI_SWAP_CHAIN_DESC desc{};
  if (FAILED(swap->GetDesc(&desc)) || !desc.OutputWindow) return false;
  DWORD pid = 0;
  GetWindowThreadProcessId(desc.OutputWindow, &pid);
  if (pid != GetCurrentProcessId()) return false;
  RECT rc{};
  if (!GetClientRect(desc.OutputWindow, &rc)) return false;
  if (rc.right - rc.left < 320 || rc.bottom - rc.top < 240) return false;
  return true;
}

HRESULT WINAPI Overlay::PresentThunk(IDXGISwapChain* self, UINT sync, UINT flags) {
  Overlay& o = Instance();
  FrameTimingScope timing(g_frameTimingDepth,o.m_frameTiming);
  if(timing.outer)o.RecordFrameTiming(self,flags);
  if (!o.m_imguiReady && !o.m_initializing && !o.m_imguiDisabled) {
    if (!o.IsMainSwapchain(self)) return ((FnPresent)o.m_origPresent)(self, sync, flags);
    o.m_initializing = true;
    o.m_imguiDisabled = !o.InitImGui(self);
    o.m_initializing = false;
  } else if (o.m_imguiReady) {
    o.OnPresent(self);
  }
  HRESULT hr = ((FnPresent)o.m_origPresent)(self, sync, flags);
  if (o.m_presentLog < 6) {
    ++o.m_presentLog;
    ImDrawData* dd = ImGui::GetCurrentContext() ? ImGui::GetDrawData() : nullptr;
    V3_INFO("present #%u hr=0x%08lx imgui=%d vtx=%d", o.m_presentLog, (unsigned long)hr,
            (int)o.m_imguiReady, dd ? dd->TotalVtxCount : -1);
  }
  return hr;
}

HRESULT WINAPI Overlay::Present1Thunk(IDXGISwapChain1* self, UINT sync, UINT flags,
                                      const DXGI_PRESENT_PARAMETERS* params) {
  Overlay& o = Instance();
  FrameTimingScope timing(g_frameTimingDepth,o.m_frameTiming);
  if(timing.outer)o.RecordFrameTiming(self,flags);
  if (!o.m_imguiReady && !o.m_initializing && !o.m_imguiDisabled) {
    if (!o.IsMainSwapchain(self)) return ((FnPresent1)o.m_origPresent1)(self, sync, flags, params);
    o.m_initializing = true;
    o.m_imguiDisabled = !o.InitImGui(self);
    o.m_initializing = false;
  } else if (o.m_imguiReady) {
    o.OnPresent(self);
  }
  return ((FnPresent1)o.m_origPresent1)(self, sync, flags, params);
}

void Overlay::RecordFrameTiming(IDXGISwapChain* swap,UINT flags) {
  if((flags&DXGI_PRESENT_TEST) || !m_imguiReady || swap!=m_swap)return;
  static thread_local FrameTimingSamples samples;
  LARGE_INTEGER now{};
  if(!QueryPerformanceCounter(&now) || now.QuadPart<=0){samples.Reset();return;}
  const auto world=m_economyWorld.load();
  if(!samples.Push(reinterpret_cast<uintptr_t>(swap),world,uint64_t(now.QuadPart)))return;
  std::string raw;raw.reserve(1408);
  for(uint64_t tick:samples.ticks){if(!raw.empty())raw+=',';raw+=std::to_string(tick);}
  V3_INFO("FRAME PERF thread=%lu world=%llu swap=%p frequency=%llu begin=%llu end=%llu count=64 ticks=%s",
      GetCurrentThreadId(),(unsigned long long)world,swap,(unsigned long long)m_frameTimingFrequency,
      (unsigned long long)samples.begin,(unsigned long long)samples.end,raw.c_str());
}

void Overlay::ReinitForSwapchain(IDXGISwapChain* swap) {
  V3_INFO("swap chain changed; reinitializing overlay");
  if (m_nativePoolDevice) {
    // Compare while the old device is still referenced, so a recycled COM
    // address after Release cannot masquerade as the original device.
    ID3D11Device* nextDevice=nullptr;
    const bool sameDevice=m_device && swap && SUCCEEDED(swap->GetDevice(__uuidof(ID3D11Device),(void**)&nextDevice)) &&
        nextDevice==m_device && SUCCEEDED(m_device->GetDeviceRemovedReason());
    if (nextDevice) nextDevice->Release();
    m_nativePoolDeviceChanged|=!sameDevice;
    m_nativePoolWorld.store(0);m_nativeRecoveryStatusDirty.store(true);
  }
  if (m_hwnd && m_origWndProc) {
    KillTimer(m_hwnd, 0x4952);
    KillTimer(m_hwnd, 0x4953);
    SetWindowLongPtrW(m_hwnd, GWLP_WNDPROC, (LONG_PTR)m_origWndProc);
    m_origWndProc = nullptr;
  }
  if (m_rtv) { m_rtv->Release(); m_rtv = nullptr; }
  if (m_backbuffer) { m_backbuffer->Release(); m_backbuffer = nullptr; }
  if (m_imguiReady) {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
  }
  if (m_context) { m_context->Release(); m_context = nullptr; }
  if (m_device) { m_device->Release(); m_device = nullptr; }
  m_swap = nullptr;
  m_hwnd = nullptr;
  m_imguiReady = false;
  m_readbackDone = false;
  m_imguiDisabled = !InitImGui(swap);
}

bool Overlay::InitImGui(IDXGISwapChain* swap) {
  if (!swap) return false;
  wchar_t envbuf[8]{};
  if (GetEnvironmentVariableW(L"IRONROAD_NO_IMGUI", envbuf, 8) > 0) {
    V3_INFO("ImGui rendering disabled by IRONROAD_NO_IMGUI");
    return false;
  }
  ID3D11Device* device = nullptr;
  if (FAILED(swap->GetDevice(__uuidof(ID3D11Device), (void**)&device)) || !device) {
    V3_ERR("swap chain has no D3D11 device");
    return false;
  }
  device->GetImmediateContext(&m_context);
  m_device = device;
  m_swap = swap;

  DXGI_SWAP_CHAIN_DESC desc{};
  swap->GetDesc(&desc);
  m_hwnd = desc.OutputWindow;
  if (!m_hwnd) {
    V3_ERR("swap chain has no window");
    return false;
  }

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  io.IniFilename = nullptr;
  io.LogFilename = nullptr;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();
  ImGuiStyle& style = ImGui::GetStyle();
  style.WindowRounding = 4.0f;
  style.FrameRounding = 3.0f;
  style.Colors[ImGuiCol_WindowBg].w = 0.94f;

  // Prefer a crisp UI font; fall back to the built-in atlas.
  ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
  if (!font) {
    V3_WARN("Segoe UI unavailable, using ImGui default font");
    io.Fonts->AddFontDefault();
  } else {
    io.FontDefault = font;
  }

  if (!ImGui_ImplWin32_Init(m_hwnd)) {
    V3_ERR("ImGui Win32 backend init failed");
    return false;
  }
  if (!ImGui_ImplDX11_Init(m_device, m_context)) {
    V3_ERR("ImGui D3D11 backend init failed");
    return false;
  }
  m_emptyUi = GetEnvironmentVariableW(L"IRONROAD_EMPTY_UI", nullptr, 0) > 0;
  m_noRender = GetEnvironmentVariableW(L"IRONROAD_NO_RENDER", nullptr, 0) > 0;
  m_readbackWanted = GetEnvironmentVariableW(L"IRONROAD_READBACK", nullptr, 0) > 0;
  // Product controls belong to the native game UI. This optional ImGui window
  // is debug-only; the standalone harness explicitly enables it for readback.
  m_visible = GetEnvironmentVariableW(L"IRONROAD_DEBUG_UI", nullptr, 0) > 0 || m_readbackWanted;
  m_origWndProc = (WNDPROC)SetWindowLongPtrW(m_hwnd, GWLP_WNDPROC, (LONG_PTR)&Overlay::WndProcThunk);
  SetTimer(m_hwnd, 0x4952, 500, nullptr);
  SetTimer(m_hwnd, 0x4953, 25, nullptr);
  m_imguiReady = true;
  V3_INFO("ImGui ready (hwnd %p, %ux%u)", m_hwnd, desc.BufferDesc.Width, desc.BufferDesc.Height);
  return true;
}

void Overlay::EnsureBackbufferRtv(IDXGISwapChain* swap) {
  if (!m_device || !swap) return;
  ID3D11Texture2D* back = nullptr;
  if (FAILED(swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back)) || !back) return;
  if (back != m_backbuffer) {
    if (m_rtv) { m_rtv->Release(); m_rtv = nullptr; }
    if (m_backbuffer) { m_backbuffer->Release(); m_backbuffer = nullptr; }
    if (SUCCEEDED(m_device->CreateRenderTargetView(back, nullptr, &m_rtv))) {
      m_backbuffer = back;
      D3D11_TEXTURE2D_DESC td{};
      back->GetDesc(&td);
      m_rtvWidth = td.Width;
      m_rtvHeight = td.Height;
      m_viewport = D3D11_VIEWPORT{0.f, 0.f, (float)td.Width, (float)td.Height, 0.f, 1.f};
    } else {
      back->Release();
    }
  } else {
    back->Release();
  }
}

void Overlay::DebugReadbackOnce() {
  if (!m_device || !m_swap || !m_context) return;
  ID3D11Texture2D* bb = nullptr;
  if (FAILED(m_swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&bb)) || !bb) return;
  D3D11_TEXTURE2D_DESC td{};
  bb->GetDesc(&td);
  td.Usage = D3D11_USAGE_STAGING;
  td.BindFlags = 0;
  td.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
  td.MiscFlags = 0;
  ID3D11Texture2D* staging = nullptr;
  if (FAILED(m_device->CreateTexture2D(&td, nullptr, &staging))) {
    bb->Release();
    return;
  }
  m_context->CopyResource(staging, bb);
  D3D11_MAPPED_SUBRESOURCE mapped{};
  if (SUCCEEDED(m_context->Map(staging, 0, D3D11_MAP_READ, 0, &mapped))) {
    std::wstring path = DirectoryOf(RuntimeDataDirectory()) + L"\\readback.ppm";
    FILE* f = _wfopen(path.c_str(), L"wb");
    if (f) {
      std::fprintf(f, "P6\n%u %u\n255\n", td.Width, td.Height);
      unsigned changed = 0;
      for (UINT y = 0; y < td.Height; ++y) {
        const unsigned char* row = (const unsigned char*)mapped.pData + (size_t)y * mapped.RowPitch;
        for (UINT x = 0; x < td.Width; ++x) {
          if (row[x * 4] > 40 || row[x * 4 + 1] > 40 || row[x * 4 + 2] > 40) ++changed;
          std::fwrite(row + x * 4, 1, 3, f);
        }
      }
      std::fclose(f);
      V3_INFO("readback written: %ux%u, %u bright pixels -> %s", td.Width, td.Height, changed,
              Narrow(path).c_str());
    }
    m_context->Unmap(staging, 0);
  }
  staging->Release();
  bb->Release();
}

void Overlay::OnPresent(IDXGISwapChain* swap) {
  if (!m_imguiReady) return;
  if (swap != m_swap) {
    if (!IsMainSwapchain(swap)) return;
    ReinitForSwapchain(swap);
    if (!m_imguiReady) return;
  }
  if (!m_passWarned && Game::Instance().Ready() && Game::Instance().Frame() > 300 &&
      m_passCalls == 0) {
    m_passWarned = true;
    V3_INFO("no arrow render pass observed after %llu frames; waiting for a map "
            "(main menu/loading can be normal)",
            (unsigned long long)Game::Instance().Frame());
  }
  ProbeVariablesIfEnabled();
  // R17: UI-thread write experiment. After the map has been stable for a few
  // seconds, queue one publish; the window subclass handles it on the UI thread.
  if (m_settings.nativeVariables && Game::Instance().Ready()) {
    if (m_passCalls > 0) {
      if (!m_passStableSince) m_passStableSince = GetTickCount64();
      if (!m_postLoadWriteDone && GetTickCount64() - m_passStableSince > 5000) {
        m_postLoadWriteDone = true;
        std::shared_ptr<const LogisticsDocument> document;
        {
          LockGuard lock(m_logisticsMutex);
          document = m_logisticsDocument;
        }
        QueueLogisticsVariables(document);
      }
    } else {
      m_passStableSince = 0;
    }
  }
  EnsureBackbufferRtv(swap);
  if (!m_rtv) return;

  wchar_t testbuf[8]{};
  if (GetEnvironmentVariableW(L"IRONROAD_CLEAR_TEST", testbuf, 8) > 0) {
    const float magenta[4] = {1.0f, 0.0f, 1.0f, 1.0f};
    m_context->OMSetRenderTargets(1, &m_rtv, nullptr);
    m_context->ClearRenderTargetView(m_rtv, magenta);
    Game::Instance().NextFrame();
    return;
  }

  if (m_autoCaptureAt && GetTickCount64() >= m_autoCaptureAt) {
    m_autoCaptureAt = 0;
    if (m_routeActive) {
      m_captureFrameRequested = true;
      V3_INFO("auto-capture: saving the next frame with the test route");
    } else {
      m_autoCaptureAt = GetTickCount64() + 1000;  // route not built yet; retry
    }
  }

  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();
  if (!m_emptyUi) DrawUi();
  ImGui::Render();

  ImDrawData* dd = ImGui::GetDrawData();
  if (m_debugFrames < 4) {
    ++m_debugFrames;
    if (dd) {
      V3_INFO("drawdata display=%.0fx%.0f fb=%.2fx%.2f lists=%d vtx=%d idx=%d",
              dd->DisplaySize.x, dd->DisplaySize.y, dd->FramebufferScale.x, dd->FramebufferScale.y,
              dd->CmdListsCount, dd->TotalVtxCount, dd->TotalIdxCount);
      for (int i = 0; i < dd->CmdListsCount && i < 4; ++i) {
        const ImDrawList* dl = dd->CmdLists[i];
        V3_INFO("  list %d cmds=%d vtx=%d idx=%d clip=(%.0f,%.0f)-(%.0f,%.0f)", i,
                dl->CmdBuffer.Size, dl->VtxBuffer.Size, dl->IdxBuffer.Size, dl->CmdBuffer[0].ClipRect.x,
                dl->CmdBuffer[0].ClipRect.y, dl->CmdBuffer[0].ClipRect.z, dl->CmdBuffer[0].ClipRect.w);
        for (int c = 0; c < dl->CmdBuffer.Size && c < 3; ++c)
          V3_INFO("    cmd %d elems=%u", c, dl->CmdBuffer[c].ElemCount);
      }
    }
  }

  m_context->OMSetRenderTargets(1, &m_rtv, nullptr);
  m_context->RSSetViewports(1, &m_viewport);
  if (!m_noRender) ImGui_ImplDX11_RenderDrawData(dd);

  if (m_readbackWanted && !m_readbackDone) {
    m_readbackDone = true;
    DebugReadbackOnce();
  }
  if (m_captureFrameRequested) {
    m_captureFrameRequested = false;
    DebugReadbackOnce();
  }

  Game::Instance().NextFrame();
  LogFlush(false);
}

bool Overlay::PickState(const char* label, int* value, const char* filterId) {
  bool changed = false;
  const char* preview = (*value >= 0 && *value < (int)m_geography->states.size())
                            ? m_geography->states[*value].name.c_str()
                            : "(none)";
  ImGui::TextUnformatted(label);
  ImGui::SetNextItemWidth(-1);
  char comboId[48];
  std::snprintf(comboId, sizeof(comboId), "##%s", label);
  if (ImGui::BeginCombo(comboId, preview)) {
    char* filter = (filterId && filterId[0] == 'd') ? m_filterDest : m_filter;
    ImGui::SetNextItemWidth(-1);
    ImGui::InputTextWithHint("##filter", "search state...", filter, 64);
    ImGui::BeginChild("##states", ImVec2(0, 280), ImGuiChildFlags_Border);
    std::string f = LowerAscii(filter);
    for (int i = 0; i < (int)m_geography->states.size(); ++i) {
      if (!f.empty() && LowerAscii(m_geography->states[i].name).find(f) == std::string::npos) continue;
      bool selected = (*value == i);
      ImGui::PushID(i);
      if (ImGui::Selectable(m_geography->states[i].name.c_str(), selected)) {
        *value = i;
        changed = true;
      }
      if (selected) ImGui::SetItemDefaultFocus();
      ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::EndCombo();
  }
  return changed;
}

void Overlay::DrawStatus() {
  Game& game = Game::Instance();
  auto row = [](const char* k, const char* v) {
    ImGui::TextUnformatted(k);
    ImGui::SameLine(180);
    ImGui::TextUnformatted(v);
  };
  row("Loader", "version.dll -> freight/freight.dll");
  row("Build", game.Ready() ? kGameVersion : "unsupported");
  if (!game.Ready()) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.55f, 0.45f, 1.0f));
    ImGui::TextWrapped("%s", game.Error().c_str());
    ImGui::PopStyleColor();
  }
  row("Native hooks", m_nativeHooks ? "installed" : "disabled");
  row("Style resources", EmbeddedAssetsMounted() ? "embedded archive mounted" : "waiting for embedded assets");
  row("Arrow type", m_type ? m_settings.arrowType.c_str() : "not resolved");
  char passText[64];
  std::snprintf(passText, sizeof(passText), "%llu calls, %llu submitted",
                (unsigned long long)m_passCalls, (unsigned long long)m_submitted);
  row("Arrow pass", passText);
  const char* autoText = !m_settings.autoTest ? "off" : (m_autoTestDone ? "done" : "pending");
  row("Auto-test", autoText);
  row("Route", m_routeActive ? "active" : "none");
  row("ImGui", m_imguiReady ? "ready" : "pending");
}

namespace {
// Slider plus a numeric entry field, so exact calibration values can be typed.
bool SliderFloatWithInput(const char* label, float* value, float min, float max,
                          const char* fmt, float step) {
  bool changed = false;
  ImGui::SetNextItemWidth(190);
  if (ImGui::SliderFloat(label, value, min, max, fmt)) changed = true;
  ImGui::SameLine();
  ImGui::SetNextItemWidth(95);
  char id[96];
  std::snprintf(id, sizeof(id), "##input_%s", label);
  if (ImGui::InputFloat(id, value, step, step * 10.0f, fmt)) {
    if (*value < min) *value = min;
    if (*value > max) *value = max;
    changed = true;
  }
  return changed;
}
}  // namespace

void Overlay::PublishLogisticsDocument(std::shared_ptr<const LogisticsDocument> document) {
  // The GUI variables are a pure function of the document plus the scope-bearing
  // status fields below, so republishing an identical state would only rewrite
  // the same strings again.
  const auto status = m_logisticsSession ? m_logisticsSession->Status() : LogisticsStatus{};
  PublishedKey key;
  key.document = document.get();
  key.owner = status.owner;
  key.world = status.world;
  key.revision = status.revision;
  key.panelUnavailable = m_nativePanelUnavailable.load();
  key.goodsUnavailable = m_goodsMetadataUnavailable.load();
  key.worldRetired = NativeWorldRetired();
  key.valid = true;
  {
    LockGuard lock(m_logisticsMutex);
    if (m_publishedKey == key) return;
    m_publishedKey = key;
    m_logisticsDocument = document;
  }
  V3_INFO("publish document=%d synthetic=%d", int(document != nullptr),
          document ? int(document->synthetic) : -1);
  PublishLogisticsVariables(document);
}

void Overlay::PublishRealEconomy(std::shared_ptr<const EconomySnapshot> snapshot) {
  if (!snapshot || !m_logisticsSession) return;
  if (m_logisticsSession->SnapshotIsCurrent(m_nativeStateRoster.snapshot) &&
      m_logisticsSession->HasEquivalentSnapshot(*snapshot)) {
    V3_INFO("real economy snapshot unchanged: retaining current revision and view");
    return;
  }
  std::string reason;
  if (!m_logisticsSession->SetSnapshot(snapshot, &reason, false)) {
    V3_WARN("real economy snapshot rejected: %s", reason.c_str());
  } else {
    m_nativeStateRoster=CopyNativeStateRoster(snapshot);
    // Snapshot availability does not activate a panel. Only the observed open
    // native market/state panel may own the formal view (PollNativeMarketPanel).
    PublishLogisticsDocument(nullptr);
    V3_INFO("real economy snapshot accepted: orders=%zu goods=%zu primary=%u (diagnostic)",
            snapshot->orders.size(), snapshot->goods.size(), snapshot->primaryMarket);
  }
}

void Overlay::PublishLogisticsVariables(const std::shared_ptr<const LogisticsDocument>& document) {
  QueueLogisticsVariables(document);
}

void Overlay::QueueLogisticsVariables(const std::shared_ptr<const LogisticsDocument>& document) {
  if (!m_settings.nativeVariables) return;
  if (!Game::Instance().Ready()) return;
  const auto status = m_logisticsSession ? m_logisticsSession->Status() : LogisticsStatus{};
  const auto frame = m_logisticsSession ? m_logisticsSession->Frame() : nullptr;
  const bool live = document && !document->synthetic && !m_goodsMetadataUnavailable.load() &&
      document->nativeTrade && frame && frame->nativeFacts && frame->nativeView &&
      status.world==m_economyWorld.load() &&
      document->kind==FreightViewKind::Market &&
      (status.owner==kOwnerNativePanel || status.owner==kOwnerHover) && document->world == status.world &&
      document->revision == status.revision && frame && frame->filter.market == document->market &&
      m_logisticsSession->StatisticsAreCurrent(frame);
  const bool stateLive=document && !document->synthetic && !m_goodsMetadataUnavailable.load() &&
      document->nativeTrade && frame && frame->nativeFacts && frame->nativeView &&
      status.world==m_economyWorld.load() &&
      status.owner==kOwnerNativeState && document->kind==FreightViewKind::State &&
      document->world==status.world && document->revision==status.revision && frame &&
      frame->filter.kind==FreightViewKind::State && frame->filter.state==document->state &&
      frame->filter.market==document->market && m_logisticsSession->StatisticsAreCurrent(frame);
  if(document && document->nativeTrade)
    V3_INFO("queue native trade vars live=%d selection_fixed=%lld scope_fixed=%lld drawable_fixed=%lld scale=%lld",int(live),
        (long long)document->nativeTrade->selectionQuantity,(long long)document->nativeTrade->scopeQuantity,
        (long long)document->nativeTrade->scopeDrawable,(long long)kNativeTradeQuantityScale);
  else if (document)
    V3_INFO("queue vars live=%d freight=%.2f target=%.2f allocated=%.2f unmet=%.2f", int(live),
            document->allFreight, document->targetImport, document->allocatedImport,
            document->unmetDemand);
  char buffer[64];
  const auto number = [&](double value) {
    std::snprintf(buffer, sizeof(buffer), "%.2f", value);
    return std::string(buffer);
  };
  std::array<std::string, 9> values;
  std::string scope = document && document->nativeTrade?"native trade (market ":"model allocation (market ";
  if (document) scope += std::to_string(document->market);
  scope += m_settings.marketOverride > 0 ? ", override)" : ", panel)";
  if (NativeWorldRetired()) scope += " - native map validation pending";
  values[0] = live ? scope : (m_goodsMetadataUnavailable.load()
                                ? "waiting for goods metadata" :
                                m_nativePanelUnavailable.load() ? "native market panel unavailable" :
                                m_nativeFiltersPending.load() ? "waiting for native goods filters" :
                                "waiting for trade view");
  values[1] = FormatFreightWork(live ? document.get() : nullptr);
  values[2] = live ? number(document->targetImport) : "--";
  values[3] = live ? number(document->allocatedImport) : "--";
  values[4] = live ? number(document->unmetDemand) : "--";
  if(live && document->nativeTrade){
    const auto& quantity=*document->nativeTrade;
    values[2]=FormatNativeTradeQuantity(quantity.scopeQuantity);
    values[3]=FormatNativeTradeQuantity(quantity.scopeQuantity-quantity.domesticScopeQuantity);
    values[4]=FormatNativeTradeQuantity(quantity.domesticScopeQuantity);
  }
  values[5] = FormatLaidTrack(live ? document.get() : nullptr);
  if(document && document->nativeTrade)
    V3_INFO("queue railway vars live=%d value=%s market=%u world=%llu revision=%llu",int(live),values[5].c_str(),
        document->market,(unsigned long long)document->world,(unsigned long long)document->revision);
  values[6] = document && document->synthetic ? "1" : "0";
  {
    std::lock_guard<std::mutex> lock(m_marketSummaryMutex);
    values[7] = m_marketSummary.empty() ? "--" : m_marketSummary;
  }
  values[8] = FormatFreightDemand(live ? document.get() : nullptr);
  auto goods=FormatFreightGoods(live ? document.get() : nullptr);
  const std::string stateId=stateLive?std::to_string(int32_t(document->state)):"ironroad-state-unavailable";
  if(stateLive)goods=FormatStateFreightGoods(document.get(),document->state);
  // The post-load map move path is only reliable for heap strings (>=16 chars);
  // short SSO values arrived corrupted (R18). Pad short values with spaces.
  for (auto& value : values)
    if (value.size() < 16) value.resize(16, ' ');
  {
    LockGuard lock(m_publishMutex);
    m_pendingPublishFrame=(live || stateLive)?frame:nullptr;
    m_pendingPublishApplied=status.applied;
    if (values == m_lastPublished && goods == m_lastGoods && stateId==m_lastStateId) return;
    m_lastStateId=stateId;m_pendingStateId=stateId;
    m_pendingStateDocument=stateLive?document:nullptr;
    m_lastPublished = values;
    m_lastGoods=goods;
    m_pendingGoods=std::move(goods);
    m_pendingPublish = values;
    if (m_publishPending) return;
    m_publishPending = true;
  }
  if (m_hwnd) PostMessageW(m_hwnd, WM_APP + 0x51, 0, 0);
}

void __fastcall Overlay::DestroyNativeWorldThunk(void* nativeWorld) {
  Overlay& o=Instance();
  try {
    const auto world=o.m_snapshotMailbox.RetireWorld(reinterpret_cast<uint64_t>(nativeWorld),o.m_economyWorld);
    if(world) {
      o.m_nativeRecoveryStatusDirty.store(true);
      V3_INFO("native world retirement event: thread=%lu next world=%llu; source retired before original destruction",
              GetCurrentThreadId(),(unsigned long long)world);
    }
  } catch(...) {
    ++o.m_economyWorld;o.m_snapshotMailbox.Retire();
    V3_WARN("native world retirement handoff failed; old requests discarded");
  }
  reinterpret_cast<void(__fastcall*)(void*)>(o.m_origDestroyNativeWorld)(nativeWorld);
}

void __fastcall Overlay::AccessScopeEndThunk(void* scope) {
  auto& o=Instance();
  const uint64_t caller=reinterpret_cast<uint64_t>(__builtin_return_address(0))-
      reinterpret_cast<uint64_t>(Game::Instance().Base());
  if(caller==kRvaCommandScopeReturn && o.m_nativeCaptureHooked.load()) {
    try {o.CopyAtNativeScopeEnd(caller);}
    catch(const std::exception& e){V3_WARN("completed command copy failed: %s",e.what());}
    catch(...){V3_WARN("completed command copy failed");}
  }
  reinterpret_cast<void(__fastcall*)(void*)>(o.m_origAccessScopeEnd)(scope);
}

void Overlay::RequestNativeSnapshot() {
  if(!m_nativeCaptureHooked.load())return;
  const auto requested=m_snapshotMailbox.Pending();
  if(requested && requested->world>=m_economyWorld.load())return;
  auto request=std::make_shared<NativeCaptureRequest>();
  request->world=m_economyWorld.load();request->revision=++m_economyRevision;
  request->marketOverride=m_settings.marketOverride>0?uint32_t(m_settings.marketOverride):0;
  m_snapshotMailbox.Ensure(std::move(request));
}

void Overlay::CopyAtNativeScopeEnd(uint64_t caller) {
  auto request=m_snapshotMailbox.Pending();if(!request)return;
  const auto base=reinterpret_cast<uint64_t>(Game::Instance().Base());
  const GoodsMemoryReader memory=[](uint64_t address,void* out,size_t size){
    SIZE_T copied=0;return address && size<=UINT64_MAX-address &&
        ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(address),out,size,&copied) && copied==size;
  };
  const uint64_t tls=__readgsqword(0x58);
  const auto access=ObserveNativeAccess(memory,base,tls);
  if(!access || !IsNativeCapturePoint(caller,*access))return;
  const uint64_t world=m_snapshotMailbox.BindSource(*access,m_economyWorld);
  if(!world)return;
  if(request->world!=world) {
    auto replacement=std::make_shared<NativeCaptureRequest>(*request);
    replacement->world=world;replacement->revision=++m_economyRevision;
    request=m_snapshotMailbox.Ensure(std::move(replacement));
    if(!request || request->world!=m_economyWorld.load())return;
    V3_INFO("native snapshot source bound before copy: world=%llu",(unsigned long long)request->world);
  }
  const auto start=GetTickCount64();std::string why;
  // Paused commands are sparse but may be the last write before a long idle.
  // Always retain their latest input. Running simulation copies at most once
  // per 30 seconds; rendering never triggers a database scan.
  if(!NativeCaptureDue(access->paused,request->revision,m_lastNativeRequestRevision.load(),
                       start,m_lastNativeSnapshotAt.load()))return;
  const uint64_t revision=++m_economyRevision;
  auto copy=CopyNativeSnapshot(memory,base,tls,caller,*request,revision,[&]{
    return m_snapshotMailbox.Current(request) && request->world==m_economyWorld.load();
  },&why,[](uint64_t candidates){return Game::Instance().SelectTradePath(candidates);});
  if(!copy){
    m_snapshotMailbox.Fail(request,revision,why);
    static std::string lastError;
    if(m_snapshotMailbox.Current(request) && why!=lastError){
      lastError=why;V3_WARN("completed command snapshot pending: %s",why.c_str());
    }
    return;
  }
  if(m_snapshotMailbox.Publish(request,copy)) {
    m_lastNativeSnapshotAt.store(GetTickCount64());m_lastNativeRequestRevision.store(request->revision);
    V3_INFO("native snapshot copied: thread=%lu world=%llu revision=%llu date=%u states=%zu goods=%zu legacy_orders/rails=not_collected total=%llu ms",
        GetCurrentThreadId(),(unsigned long long)request->world,(unsigned long long)revision,
        copy->identity.date,copy->states.size(),copy->catalog.goods.size(),(unsigned long long)(GetTickCount64()-start));
    const auto& trade=*copy->snapshot->nativeTrade;
    size_t destinations=0,alternatives=0,selected=0;
    for(const auto& group:trade.groups)for(const auto& dest:group.destinations){
      ++destinations;alternatives+=dest.candidates.size();selected+=dest.selected.has_value();
    }
    V3_INFO("native trade copied: world=%llu revision=%llu groups=%zu destinations=%zu alternatives=%zu selected=%zu pairs=%zu straits=%zu",
        (unsigned long long)request->world,(unsigned long long)revision,trade.groups.size(),destinations,
        alternatives,selected,trade.statePairs.size(),trade.straits.size());
    const auto& geometry=*trade.geometry;
    V3_INFO("native spline geometry copied: map=%ux%u points=%zu segments=%zu connections=%zu",
        geometry.width,geometry.height,geometry.points.size(),geometry.strips.size(),geometry.connections.size());
    const auto& sea=*trade.seaGeography;
    V3_INFO("native sea geography copied: definitions=%zu referenced_provinces=%zu states=%zu",
        sea.definitions.size(),sea.provinces.size(),sea.states.size());
  }
}

void Overlay::CollectRealEconomy() {
  if(!m_settings.nativeVariables || !Game::Instance().Ready() ||
     !m_nativeCaptureHooked.load() || !m_hwnd ||
     GetWindowThreadProcessId(m_hwnd,nullptr)!=GetCurrentThreadId())return;
  CheckWorldLifecycle(true);
  const auto base=Game::Instance().Base();if(!base)return;
  const GoodsMemoryReader memory=[](uint64_t address,void* out,size_t size){
    SIZE_T copied=0;return address && size<=UINT64_MAX-address &&
        ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(address),out,size,&copied) && copied==size;
  };
  const auto phase=DecodeNativeMarketPanel(memory,reinterpret_cast<uint64_t>(base));
  if(!phase.available || !phase.ingame)return;
  const auto error=m_snapshotMailbox.TakeError();
  if(!error.empty()) {
    m_goodsMetadataUnavailable.store(true);
    if(m_logisticsSession)m_nativeMarketOwner.Close(*m_logisticsSession);
    if(m_logisticsSession)m_nativeStateOwner.Close(*m_logisticsSession);
    m_nativeStateRoster={};m_nativeMarketOwner.Reset();
    m_realMarketIds.clear();m_realGoodsByKey.clear();m_hoverVariableKeys.clear();PublishLogisticsDocument(nullptr);
  }
  auto capture=m_snapshotMailbox.Take();
  if(!capture){RequestNativeSnapshot();return;}
  const auto identity=ObserveNativeAccess(memory,reinterpret_cast<uint64_t>(base),__readgsqword(0x58));
  const auto snapshot=capture->snapshot;
  const auto anchor=ReadNativeStateObject(memory,reinterpret_cast<uint64_t>(base),capture->anchorId);
  if(!identity || !NativeCaptureMatches(*capture,*identity,m_economyWorld.load(),m_snapshotMailbox.LatestRevision(),anchor)){
    V3_INFO("native snapshot handoff rejected: retired source identity");RequestNativeSnapshot();return;
  }
  const auto& catalog=capture->catalog;
  const int states=int(capture->states.size());
  std::map<uint32_t,int> marketCounts;
  for(const auto& state:capture->states)++marketCounts[state.market];
  m_economyAnchorId=capture->anchorId;m_economyAnchor=reinterpret_cast<void*>(capture->anchor);
  V3_INFO("native snapshot UI handoff: thread=%lu world=%llu revision=%llu states=%d orders=%zu",
      GetCurrentThreadId(),(unsigned long long)snapshot->world,(unsigned long long)snapshot->revision,
      states,snapshot->orders.size());
  m_goodsMetadataUnavailable.store(false);
  const auto localGoods = std::count_if(snapshot->goods.begin(), snapshot->goods.end(),
                                      [](const auto& g) { return g.localOnly; });
  const auto nontradeable = std::count_if(snapshot->goods.begin(), snapshot->goods.end(),
                                        [](const auto& g) { return !g.tradeable && !g.localOnly; });
  V3_INFO("real goods metadata catalog=%zu observed=%zu local=%zu nontradeable=%zu thread=%lu",
          catalog.goods.size(), snapshot->goods.size(), size_t(localGoods), size_t(nontradeable),
          GetCurrentThreadId());
  m_economyCollected = true;
  V3_INFO("real economy collected states=%d orders=%zu goods=%zu", states,
          snapshot->orders.size(), snapshot->goods.size());
  uint32_t primaryMarket = 0;
  m_realMarketIds.clear();
  for (const auto& [market, count] : marketCounts) m_realMarketIds.insert(market);
  m_realGoodsByKey.clear();
  m_hoverVariableKeys.clear();
  for (const auto& good : snapshot->goods) {
    m_realGoodsByKey.emplace(good.key,good.id);
    m_hoverVariableKeys.push_back("ironroad_market_hover_"+good.key);
  }
  int primaryCount = 0;
  for (const auto& [market, count] : marketCounts)
    if (count > primaryCount) {
      primaryCount = count;
      primaryMarket = market;
    }
  if (m_settings.marketOverride > 0 && marketCounts.count(m_settings.marketOverride)) {
    primaryMarket = static_cast<uint32_t>(m_settings.marketOverride);
    primaryCount = marketCounts[primaryMarket];
  }
  std::vector<std::pair<int, uint32_t>> topMarkets;
  for (const auto& [market, count] : marketCounts)
    topMarkets.push_back({count, market});
  std::sort(topMarkets.rbegin(), topMarkets.rend());
  std::string topText;
  for (size_t i = 0; i < topMarkets.size() && i < 3; ++i)
    topText += std::to_string(topMarkets[i].second) + ":" +
               std::to_string(topMarkets[i].first) + " ";
  {
    std::string summary;
    for (size_t i = 0; i < topMarkets.size() && i < 5; ++i) {
      summary += std::to_string(topMarkets[i].second) + ":" +
                 std::to_string(topMarkets[i].first);
      summary += (i + 1 < topMarkets.size() && i + 1 < 5) ? " " : "";
    }
    std::lock_guard<std::mutex> lock(m_marketSummaryMutex);
    m_marketSummary = std::move(summary);
  }
  V3_INFO("real economy markets=%zu primary=%u states=%d top=%s", marketCounts.size(),
          primaryMarket, primaryCount, topText.c_str());
  {
    // Full id list so the snapshot side of the market-identity question is on
    // record in the log instead of only the top three.
    std::vector<uint32_t> ids;
    ids.reserve(marketCounts.size());
    for (const auto& [market, count] : marketCounts) ids.push_back(market);
    std::sort(ids.begin(), ids.end());
    std::string list;
    for (size_t i = 0; i < ids.size(); ++i) {
      list += std::to_string(ids[i]);
      if (i + 1 < ids.size()) list += ' ';
    }
    V3_INFO("real economy market ids: %s", list.c_str());
  }
  if (m_settings.probeRegistry) {
    static std::atomic<bool> dumpedRegistry{false};
    if (!dumpedRegistry.exchange(true)) {
      const uintptr_t registry = reinterpret_cast<uintptr_t>(base) + 0x59FD648;
      if (PointerReadable(reinterpret_cast<const void*>(registry), 0x10)) {
        auto* data = *reinterpret_cast<uint8_t**>(registry);
        const int sizeA = *reinterpret_cast<int*>(registry + 0x08);
        const int sizeB = *reinterpret_cast<int*>(registry + 0x0C);
        uint64_t h[4]{};
        std::memcpy(h, reinterpret_cast<const void*>(registry), sizeof(h));
        V3_INFO("script registry hdr=%llx %llx %llx %llx data=%p sizeA=%d sizeB=%d",
                (unsigned long long)h[0], (unsigned long long)h[1],
                (unsigned long long)h[2], (unsigned long long)h[3], data, sizeA, sizeB);
        const int count = sizeA > 0 && sizeA < 2000 ? sizeA : sizeB;
        for (int i = 0; i < count && i < 400; ++i) {
          auto* entry = data + size_t(i) * 0x78;
          if (!PointerReadable(entry, 0x78)) break;
          auto* np = *reinterpret_cast<const char**>(entry + 0x40);
          const uint64_t ns = *reinterpret_cast<uint64_t*>(entry + 0x50);
          if (!np || ns == 0 || ns >= 64 || !PointerReadable(np, static_cast<size_t>(ns)))
            continue;
          std::string name(np, static_cast<size_t>(ns));
          if (i < 10)
            V3_INFO("script registry[%d] '%s' q20=%llx q30=%llx", i, name.c_str(),
                    (unsigned long long)*reinterpret_cast<uint64_t*>(entry + 0x20),
                    (unsigned long long)*reinterpret_cast<uint64_t*>(entry + 0x30));
          if (name.find("arket") == std::string::npos) continue;
          V3_INFO("script registry '%s' q20=%llx q28=%llx q30=%llx q38=%llx",
                  name.c_str(),
                  (unsigned long long)*reinterpret_cast<uint64_t*>(entry + 0x20),
                  (unsigned long long)*reinterpret_cast<uint64_t*>(entry + 0x28),
                  (unsigned long long)*reinterpret_cast<uint64_t*>(entry + 0x30),
                  (unsigned long long)*reinterpret_cast<uint64_t*>(entry + 0x38));
        }
      }
    }
  }
  if (m_settings.probeMarket) {
    const auto lookup=[&](uint32_t id){return reinterpret_cast<void*>(
        ReadNativeStateObject(memory,reinterpret_cast<uint64_t>(base),id));};
    // Historical field diagnostic only; field48 never selects a market.
    {
      std::map<uint64_t, int> counts;
      for (const auto& state : capture->states) {
        const auto id=state.id;
        void* s = lookup(id);
        if (!s) continue;
        uint64_t value=0;if(!memory(reinterpret_cast<uint64_t>(s)+0x48,&value,8))continue;
        ++counts[value];
      }
      std::vector<std::pair<int, uint64_t>> top;
      for (const auto& [v, n] : counts) top.push_back({n, v});
      std::sort(top.rbegin(), top.rend());
      std::string text;
      for (size_t i = 0; i < top.size() && i < 12; ++i)
        text += std::to_string(top[i].second) + ":" + std::to_string(top[i].first) + " ";
      V3_INFO("eco field48 distinct=%zu top=%s", counts.size(), text.c_str());
    }
    for (int id : {17, 19, 132}) {
      void* s = lookup(uint32_t(id));
      if (!s) continue;
      uint64_t w[64]{};
      if(!memory(reinterpret_cast<uint64_t>(s),w,sizeof(w)))continue;
      for (int i = 0; i < 64; i += 4)
        V3_INFO("eco state id=%d off=0x%02x %016llx %016llx %016llx %016llx", id, i * 8,
                (unsigned long long)w[i], (unsigned long long)w[i + 1],
                (unsigned long long)w[i + 2], (unsigned long long)w[i + 3]);
    }
    for (int id : {17, 19, 132, 350, 201, 205, 432, 436}) {
      void* s = lookup(uint32_t(id));
      if (!s) continue;
      uint64_t value=0;if(!memory(reinterpret_cast<uint64_t>(s)+0x48,&value,8))continue;
      V3_INFO("eco market id=%d market=%llu", id,
              (unsigned long long)value);
    }
    for (int id : {16, 17, 18, 132, 432, 205, 201, 350}) {
      void* s = lookup(uint32_t(id));
      if (!s) continue;
      for (int off : {0xC8, 0x78}) {
        uint64_t h[8]{};
        if(!memory(reinterpret_cast<uint64_t>(s)+off,h,sizeof(h)))continue;
        V3_INFO("eco container id=%d off=0x%x %016llx %016llx %016llx %016llx %016llx %016llx %016llx %016llx",
                id, off, (unsigned long long)h[0], (unsigned long long)h[1],
                (unsigned long long)h[2], (unsigned long long)h[3],
                (unsigned long long)h[4], (unsigned long long)h[5],
                (unsigned long long)h[6], (unsigned long long)h[7]);
      }
    }
  }
  PublishRealEconomy(snapshot);
}

void Overlay::CheckWorldLifecycle(bool force) {
  const auto world=m_economyWorld.load();
  if(m_uiEconomyWorld!=world){ApplyEconomyWorld(world,"native source changed");return;}
  if (!m_economyAnchor) return;
  if (!force && ++m_lifecycleTicks % 4) return;
  const auto base = Game::Instance().Base();
  if (!base) return;
  const GoodsMemoryReader read=[](uint64_t address,void* out,size_t size){
    SIZE_T copied=0;return address && size<=UINT64_MAX-address &&
      ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(address),out,size,&copied) && copied==size;
  };
  const auto state=ReadNativeStateObject(read,reinterpret_cast<uint64_t>(base),m_economyAnchorId);
  if(state && reinterpret_cast<void*>(state)==m_economyAnchor)return;
  InvalidateEconomy("world changed");
}

void Overlay::InvalidateEconomy(const char* why) {
  uint64_t expected=m_uiEconomyWorld;
  m_economyWorld.compare_exchange_strong(expected,m_uiEconomyWorld+1);
  ApplyEconomyWorld(m_economyWorld.load(),why);
}

void Overlay::ApplyEconomyWorld(uint64_t world,const char* why) {
  // The pool is no longer ready for this world. Its render-side recovery must
  // verify retained registration/resources; never destruct or reconstruct it.
  m_uiEconomyWorld=world;
  RequestNativeSnapshot();
  V3_INFO("%s: invalidating economy snapshot, next world=%llu", why, (unsigned long long)world);
  m_economyCollected = false;
  m_economyAnchor = nullptr;
  if (m_logisticsSession) m_logisticsSession->ResetWorld();
  if (m_logisticsSession) m_nativeStateOwner.Close(*m_logisticsSession);
  m_nativeStateRoster={};
  m_nativeMarketOwner.Reset();
  m_realMarketIds.clear();
  m_realGoodsByKey.clear();
  m_hoverVariableKeys.clear();
  PublishLogisticsDocument(nullptr);
}

void Overlay::PollNativeMarketPanel() {
  if (!m_settings.nativeVariables || !m_logisticsSession || !m_hwnd || !Game::Instance().Ready() ||
      GetWindowThreadProcessId(m_hwnd,nullptr)!=GetCurrentThreadId()) return;
  if(m_uiEconomyWorld!=m_economyWorld.load())CheckWorldLifecycle(true);
  const auto reader=[](uint64_t address,void* out,size_t bytes) {
    if (!address || bytes>UINT64_MAX-address) return false;
    SIZE_T copied=0;
    return ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(address),
                             out,bytes,&copied) && copied==bytes;
  };
  const auto panel=DecodeNativeMarketPanel(reader,reinterpret_cast<uint64_t>(Game::Instance().Base()));
  if (panel.available && !panel.ingame && m_economyAnchor)
    InvalidateEconomy("left game phase");
  const bool availabilityChanged=m_nativePanelUnavailable.exchange(!panel.available)!=!panel.available;
  const bool pending=panel.available && panel.marketOpen && panel.tab=="ironroad_logistics" && !panel.visibleGoods;
  const bool filterPendingChanged=m_nativeFiltersPending.exchange(pending)!=pending;
  if (panel.error!=m_nativePanelError) {
    m_nativePanelError=panel.error;
    if (!panel.error.empty()) V3_WARN("native market panel/filter unavailable: %s",panel.error.c_str());
  }
  const auto change=m_nativeMarketOwner.Update(*m_logisticsSession,panel,m_realMarketIds,
        !m_goodsMetadataUnavailable.load(),m_settings.marketOverride);
  const auto statePanel=DecodeNativeStatePanel(reader,reinterpret_cast<uint64_t>(Game::Instance().Base()));
  const auto previousStateContext=m_nativeStateOwner.Context();
  const auto stateChange=m_nativeStateOwner.Update(*m_logisticsSession,statePanel,m_nativeStateRoster,
        !m_goodsMetadataUnavailable.load());
  if(previousStateContext!=m_nativeStateOwner.Context()){
    std::string error;
    bool ready=SetGuiVariable(Game::Instance().Base(),"ironroad_state_controls_id_v1","ironroad-state-unavailable",&error) &&
       SetGuiVariable(Game::Instance().Base(),"ironroad_state_context_v1",m_nativeStateOwner.Context(),&error);
    for(const auto& key:m_nativeStateOwner.FilterKeys())
      if(ready)ready=SetGuiVariable(Game::Instance().Base(),key,m_nativeStateOwner.Context()+":all",&error);
    if(ready)
      SetGuiVariable(Game::Instance().Base(),"ironroad_state_controls_id_v1",m_nativeStateOwner.ReadyId(),&error);
    if(!error.empty())V3_WARN("native state controls unavailable: %s",error.c_str());
  }
  // Diagnostic only (no view authorisation): the vanilla panel resolves its market
  // through the GUI registry, the snapshot groups states by state+0x48. Log both so
  // the two id spaces can be reconciled from evidence instead of assumption.
  {
    static uint32_t lastGuiMarket=UINT32_MAX, lastCandidate=UINT32_MAX;
    if (panel.available && panel.marketOpen &&
        (panel.market!=lastGuiMarket || panel.simulationMarket!=lastCandidate)) {
      lastGuiMarket=panel.market;lastCandidate=panel.simulationMarket;
      V3_INFO("native market identity gui=%u sim_candidate=%u in_snapshot=%d snapshot_markets=%zu",
              panel.market, panel.simulationMarket,
              int(m_realMarketIds.count(panel.simulationMarket)!=0), m_realMarketIds.size());
    }
  }
  // Diagnostic: what does the engine itself say the panel market is? The GUI writes
  // these variables from MarketPanel.GetMarket; reading them tells us whether the
  // GUI->variable channel accepts computed values at all. Only sampled while our
  // tab is the active one, because that is where the probe widgets exist.
  {
    static int sample = 0;
    static std::string lastOwner, lastMame;
    if (panel.available && panel.marketOpen && panel.tab == "ironroad_logistics" && (sample++ % 60) == 0) {
      std::string error;
      const v3::GuiVariableProbe probe = v3::ProbeGuiVariableKeys(Game::Instance().Base(),
          {"ironroad_probe_market_owner_v1","ironroad_probe_market_name_v1"});
      if (!probe.ok) {
        V3_WARN("gui market identity probe failed: %s", probe.error.c_str());
      } else {
        std::string owner, name;
        for (const auto& entry : probe.entries) {
          if (entry.key == "ironroad_probe_market_owner_v1") owner = entry.value;
          else if (entry.key == "ironroad_probe_market_name_v1") name = entry.value;
        }
        if (owner != lastOwner || name != lastMame) {
          lastOwner = owner;
          lastMame = name;
          V3_INFO("gui market identity owner=\"%s\" name=\"%s\" entries=%d",
                  owner.c_str(), name.c_str(), probe.count);
        }
      }
    }
  }
  const auto& context=m_nativeMarketOwner.Context();
  if (m_hoverContext!=context && (!m_hoverContext.empty() || context!="ironroad-hover-inactive")) {
    m_hoverContext=context;
    std::string error;
    m_hoverContextReady=SetGuiVariable(Game::Instance().Base(),"ironroad_hover_context_v1",context,&error);
    if(m_hoverContextReady)m_hoverContextReady=SetGuiVariable(Game::Instance().Base(),"ironroad_market_color_v1",context+":all",&error);
    if (!m_hoverContextReady) V3_WARN("native hover context unavailable: %s",error.c_str());
  }
  auto variableKeys=m_nativeStateOwner.FilterKeys();
  if(m_hoverContextReady && context!="ironroad-hover-inactive"){
    variableKeys.insert(variableKeys.end(),m_hoverVariableKeys.begin(),m_hoverVariableKeys.end());
    variableKeys.push_back("ironroad_market_color_v1");
  }
  const auto variables=!variableKeys.empty()?ProbeGuiVariableKeys(Game::Instance().Base(),variableKeys):GuiVariableProbe{};
  const bool colorChanged=m_nativeMarketOwner.UpdateColor(*m_logisticsSession,variables);
  const bool hoverChanged=m_nativeMarketOwner.UpdateHover(*m_logisticsSession,variables,m_realGoodsByKey);
  const bool stateFilterChanged=m_nativeStateOwner.UpdateFilters(*m_logisticsSession,variables,m_realGoodsByKey);
  if (change.changed || stateChange.changed || stateFilterChanged || hoverChanged || colorChanged || availabilityChanged || filterPendingChanged) {
    const auto status=m_logisticsSession->Status();
    // A lower-priority panel change must not erase another owner's document.
    if ((change.changed || stateChange.changed || availabilityChanged || filterPendingChanged) &&
        (!status.active || IsNativeLogisticsOwner(status.owner))) PublishLogisticsDocument(nullptr);
    V3_INFO("native market owner %s panel=%u tab=%s reason=%s",
            change.changed ? (change.closed?"closed":"selected") : (hoverChanged?"hover":"state ready"),
            panel.market,panel.tab.c_str(),panel.error.c_str());
    if(stateChange.changed)V3_INFO("native state owner %s state=%u tab=%s reason=%s",
        stateChange.closed?"closed":"selected",statePanel.state,statePanel.tab.c_str(),statePanel.error.c_str());
    if(stateFilterChanged)V3_INFO("native state map filter changed state=%u",statePanel.state);
    if(colorChanged)V3_INFO("native market color mode changed market=%u",panel.market);
  }
  if (auto document=m_logisticsSession->Poll()) PublishLogisticsDocument(std::move(document));
}

void Overlay::PollHideFlows() {
  if (!m_settings.nativeVariables || !Game::Instance().Ready()) return;
  const auto base = Game::Instance().Base();
  if (!base) return;
  const auto probe = ProbeGuiVariableKeys(base,{"ironroad_hide_flows"});
  bool hide = false;
  for (const auto& entry : probe.entries) {
    if (entry.key == "ironroad_hide_flows") {
      hide = true;
      break;
    }
  }
  const bool previous = m_hideFlows.exchange(hide);
  if (previous != hide) V3_INFO("native panel hide flows=%d", int(hide));
  if (m_settings.testWorldChange && m_economyCollected) {
    m_settings.testWorldChange = false;
    InvalidateEconomy("TEST world-change");
  }
  CheckWorldLifecycle();
  // Completed commands maintain the latest owned input. UI consumes it without
  // scanning an idle paused world's databases or waiting for another command.
  if (m_snapshotMailbox.Ready() || (m_economyAttempts++ % 10 == 0)) CollectRealEconomy();
  if (m_logisticsSession) {
    if (auto document = m_logisticsSession->Poll())
      PublishLogisticsDocument(std::move(document));
    if (m_nativeRecoveryStatusDirty.exchange(false))
      PublishLogisticsDocument(m_logisticsDocument);
    static uint64_t lastGen = 0;
    static uint64_t statusTicks = 0;
    const auto st = m_logisticsSession->Status();
    if (++statusTicks % 10 == 0)
      V3_INFO("native session tick owner=%u gen=%llu ready=%d frame=%d ribbons=%zu err=%s",
              st.owner, (unsigned long long)st.generation, int(st.snapshotReady),
              int(st.haveFrame), st.ribbons, st.error.c_str());
    if (st.generation != lastGen) {
      lastGen = st.generation;
      V3_INFO("native session gen=%llu ready=%d frame=%d owner=%u ribbons=%zu err=%s",
              (unsigned long long)st.generation, int(st.snapshotReady), int(st.haveFrame),
              st.owner, st.ribbons, st.error.c_str());
    }
  }
}

void Overlay::OnUiThreadPublish() {  std::array<std::string, 9> values;
  std::vector<std::pair<std::string,std::string>> goods;
  std::shared_ptr<const FreightFrame> frame;
  std::shared_ptr<const LogisticsDocument> stateDocument;
  std::string stateId;
  uint64_t applied=0;
  {
    LockGuard lock(m_publishMutex);
    if (!m_publishPending) return;
    values = m_pendingPublish;
    goods=std::move(m_pendingGoods);
    frame=std::move(m_pendingPublishFrame);applied=m_pendingPublishApplied;
    stateDocument=std::move(m_pendingStateDocument);stateId=std::move(m_pendingStateId);
    m_publishPending = false;
  }
  const auto current=[&]{return frame && m_logisticsSession && m_logisticsSession->Status().world==m_economyWorld.load() &&
      m_logisticsSession->StatisticsAreCurrent(frame);};
  const auto stateCurrent=[&]{return stateDocument && m_logisticsSession &&
      stateDocument->world==m_economyWorld.load() &&
      m_nativeStateOwner.CanPublish(*m_logisticsSession,*stateDocument,frame,applied) &&
      stateId==m_nativeStateOwner.ReadyId();};
  if (frame && !current()) {
    values[0]="waiting for model view";
    for (size_t i=1;i<=5;++i)values[i]="--              ";
    values[8]="-- / -- / --    ";goods.clear();
    LockGuard lock(m_publishMutex);m_lastPublished={};m_lastGoods.clear();
  }
  std::string gateError;
  if (!SetGuiVariable(Game::Instance().Base(),"ironroad_state_rows_id_v1","ironroad-state-unavailable",&gateError)) {
    V3_WARN("native state publication unavailable: %s",gateError.c_str());return;
  }
  if (!SetGuiVariable(Game::Instance().Base(),"ironroad_goods_context_v1","ironroad-rows-unavailable",&gateError)) {
    V3_WARN("native goods publication unavailable: %s",gateError.c_str());return;
  }
  bool published=true;
  if (!SetGuiVariable(Game::Instance().Base(),"ironroad_map_status_v1",
                      NativeWorldRetired()?"map-retired-state-v1":"map-available-state-v1",&gateError)) {
    published=false;
    V3_WARN("native map status publication unavailable: %s",gateError.c_str());
  }
  static const char* const keys[9] = {"ironroad_status_v1",    "ironroad_freight_v1",
                                      "ironroad_target_v1",    "ironroad_allocated_v1",
                                      "ironroad_unmet_v1",     "ironroad_track_v1",
                                      "ironroad_synthetic_v1", "ironroad_markets_v1",
                                      "ironroad_demand_v1"};
  for (size_t i = 0; i < values.size(); ++i) {
    std::string error;
    if (!SetGuiVariable(Game::Instance().Base(), keys[i], values[i], &error)) {
      published=false;
      V3_WARN("UI-thread variable %s failed: %s", keys[i], error.c_str());
    }
  }
  std::map<std::string,std::string> rows;
  for (const auto& key:m_writtenGoodsKeys)rows[key]="--";
  m_writtenGoodsKeys.clear();
  for (auto& [key,value]:goods){rows[key]=value;m_writtenGoodsKeys.push_back(key);}
  for (auto& [key,value]:rows) {
    const std::string stateValue=value;
    if(value.size()<16)value.resize(16,' ');
    std::string error;
    bool written=SetGuiVariable(Game::Instance().Base(),key,value,&error);
    // Prime short strings through the verified heap path, then assign the
    // unpadded text to the existing entry. Padding otherwise elides as "...".
    if(written && key.starts_with("ironroad_state_work_") && stateValue.size()<16)
      written=SetGuiVariable(Game::Instance().Base(),key,stateValue,&error);
    if(!written) {
      published=false;V3_WARN("UI-thread goods variable %s failed: %s",key.c_str(),error.c_str());
    }
  }
  if (published && stateCurrent()) {
    std::string error;
    if(!SetGuiVariable(Game::Instance().Base(),"ironroad_state_rows_id_v1",stateId,&error))
      V3_WARN("native state ready ID failed: %s",error.c_str());
  }
  if (published && !stateDocument && current() && !goods.empty() && m_nativeMarketOwner.Context()!="ironroad-hover-inactive") {
    std::string error;
    if(!SetGuiVariable(Game::Instance().Base(),"ironroad_goods_context_v1",m_nativeMarketOwner.Context(),&error))
      V3_WARN("native goods ready context failed: %s",error.c_str());
  }
  if (frame && !current()) {
    for (size_t i=0;i<values.size();++i) {
      if (i==6 || i==7)continue;
      std::string error;
      const std::string unavailable=i==0?"waiting for model view":(i==8?"-- / -- / --    ":"--              ");
      if(!SetGuiVariable(Game::Instance().Base(),keys[i],unavailable,&error))
        V3_WARN("UI-thread stale summary clear failed: %s",error.c_str());
    }
    std::string error;
    SetGuiVariable(Game::Instance().Base(),"ironroad_goods_context_v1","ironroad-rows-unavailable",&error);
    SetGuiVariable(Game::Instance().Base(),"ironroad_state_rows_id_v1","ironroad-state-unavailable",&error);
    LockGuard lock(m_publishMutex);m_lastPublished={};m_lastGoods.clear();
  }
  V3_INFO("UI-thread variables published");
  if (m_settings.testHideVariable && !m_hideTestWritten) {
    std::string error;
    if (SetGuiVariable(Game::Instance().Base(), "ironroad_hide_flows",
                       "1               ", &error)) {
      m_hideTestWritten = true;
      V3_INFO("TEST ironroad_hide_flows written from UI thread");
    } else {
      V3_WARN("TEST ironroad_hide_flows failed: %s", error.c_str());
    }
  }
}

std::optional<LogisticsSelection> Overlay::TakeLogisticsRequest() {
  LockGuard lock(m_logisticsMutex);
  auto request = std::move(m_logisticsRequest);
  m_logisticsRequest.reset();
  return request;
}

void Overlay::DrawPreviewControls() {
    ImGui::TextUnformatted("Route preview");
    if (!m_stateDataLoaded) {
      ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Route data missing: %s", m_stateError.c_str());
    } else {
      PickState("Origin", &m_originState, "origin");
      PickState("Destination", &m_destState, "dest");
      ImGui::TextUnformatted("Geometry");
      int mode = m_settings.routeMode;
      if (ImGui::RadioButton("Authored splines", &mode, 2)) {
        m_settings.routeMode = mode; SaveSettings();
        if (m_wantsRoute.load()) RequestRoute(m_from, m_to);
      }
      ImGui::SameLine();
      if (ImGui::RadioButton("Province land path", &mode, 1)) {
        m_settings.routeMode = mode;
        SaveSettings();
        if (m_wantsRoute.load()) RequestRoute(m_from, m_to);
      }
      ImGui::SameLine();
      if (ImGui::RadioButton("Straight chord", &mode, 0)) {
        m_settings.routeMode = mode;
        SaveSettings();
        if (m_wantsRoute.load()) RequestRoute(m_from, m_to);
      }
      if (!m_geography->provinces.Valid())
        ImGui::TextDisabled("province graph missing: straight chord only");
      static const char* kTypeNames[] = {"ironroadBuildingOutput", "ironroadBuildingInput",
                                         "ironroadBuildingYellow", "ironroadBlue", "supplyRouteArrow",
                                         "ironroadTrafficGreen", "ironroadTrafficLime", "ironroadTrafficYellow",
                                         "ironroadTrafficOrange", "ironroadTrafficRed"};
      static const char* kTypeLabels[] = {"Building flow (green)", "Building flow (orange)",
                                          "Building flow (yellow)", "Building flow (blue)", "Vanilla (tan)",
                                          "Traffic (green)", "Traffic (lime)", "Traffic (yellow)",
                                          "Traffic (orange)", "Traffic (red)"};
      int typeIndex = 0;
      for (int i = 0; i < 10; ++i)
        if (m_settings.arrowType == kTypeNames[i]) typeIndex = i;
      ImGui::TextUnformatted("Colour");
      ImGui::SetNextItemWidth(-1);
      if (ImGui::BeginCombo("##colour", kTypeLabels[typeIndex])) {
        for (int i = 0; i < 10; ++i) {
          if (ImGui::Selectable(kTypeLabels[i], i == typeIndex)) SetArrowType(kTypeNames[i]);
        }
        ImGui::EndCombo();
      }
      float consumption = m_settings.previewConsumption;
      if (ImGui::SliderFloat("Preview consumption", &consumption, 1, 100000, "%.0f units", ImGuiSliderFlags_Logarithmic)) {
        m_settings.previewConsumption = consumption;
        if (m_wantsRoute.load()) RequestRoute(m_from, m_to);
      }
      ImGui::TextDisabled("Manual preview amount; allocated freight determines width in freight views.");
      if (typeIndex < 4 && !m_type)
        ImGui::TextDisabled("Embedded styles not registered. Restart with the latest DLL and check the log.");
      if (ImGui::Button("Draw route", ImVec2(140, 0))) {
        RequestRoute(m_originState, m_destState);
      }
      ImGui::SameLine();
      if (ImGui::Button("Clear", ImVec2(90, 0))) ClearRoute();
      ImGui::SameLine();
      ImGui::TextDisabled("generation %llu", (unsigned long long)m_generation.load());
    }

}

void Overlay::DrawDiagnostics() {
    if (ImGui::CollapsingHeader("Status", ImGuiTreeNodeFlags_DefaultOpen)) {
      DrawStatus();
      bool enabled = m_settings.nativeEnabled;
      if (ImGui::Checkbox("Enable native arrow submission", &enabled)) {
        m_settings.nativeEnabled = enabled;
        if (!enabled) ClearRoute();
        SaveSettings();
      }
      ImGui::TextDisabled("Turn off to use the panel without calling the arrow renderer.");
      bool piggy = m_piggyback;
      if (ImGui::Checkbox("Diagnostic: piggyback on a native append", &piggy)) {
        m_piggyback = piggy;
        SaveSettings();
      }
      bool autoTest = m_settings.autoTest;
      if (ImGui::Checkbox("Auto-test route on first load (Home Counties -> Lancashire)", &autoTest)) {
        m_settings.autoTest = autoTest;
        SaveSettings();
      }
    }

    if (m_logisticsSession) {
      const auto status = m_logisticsSession->Status();
      if (ImGui::CollapsingHeader("Freight session (model worker)")) {        ImGui::Text("snapshot %s   world %llu   revision %llu   owner %u",
                    status.synthetic ? "TEST INPUT"
                                     : (status.snapshotReady ? "ready" : "unavailable"),
                    (unsigned long long)status.world, (unsigned long long)status.revision,
                    status.owner);
        ImGui::Text("frame %s   ribbons %zu   generation %llu",
                    status.haveFrame ? "ready" : "--", status.ribbons,
                    (unsigned long long)status.generation);
        ImGui::Text("allocation %llu build / %llu hit   view %llu build / %llu hit",
                    (unsigned long long)status.allocationBuilds,
                    (unsigned long long)status.allocationHits,
                    (unsigned long long)status.viewBuilds,
                    (unsigned long long)status.viewHits);
        ImGui::Text("last compute %.2f ms   %s", status.computeMs,
                    status.error.empty() ? "ok" : status.error.c_str());
        bool testSnapshot = m_settings.testSnapshot;
        if (ImGui::Checkbox("Synthetic test snapshot (NOT live)", &testSnapshot)) {
          m_settings.testSnapshot = testSnapshot;
          if (testSnapshot) {
            auto testInput = BuildSyntheticSnapshot(*m_geography);
            testInput.world = ++m_economyWorld;
            testInput.revision = ++m_economyRevision;
            auto snapshot = std::make_shared<const EconomySnapshot>(std::move(testInput));
            std::string reason;
            if (m_logisticsSession->SetSnapshot(snapshot, &reason, true))
              V3_INFO("synthetic freight test snapshot: %zu states/orders (debug input)",
                      snapshot->orders.size());
            else
              V3_WARN("synthetic snapshot rejected: %s", reason.c_str());
          } else {
            ++m_economyWorld;
            m_logisticsSession->ResetWorld();
            m_freightKey = 0;
          }
          SaveSettings();
        }
        ImGui::TextDisabled("Generated debug input; values are not game readings.");
        ImGui::TextWrapped(
            "No verified live economy access window: values stay unavailable. The worker only "
            "runs after a copied snapshot is supplied; it never scans the game on the render thread.");
      }
    }

    if (ImGui::CollapsingHeader("GUI variable probe (read-only)")) {
      bool enabled = m_settings.probeVariables;
      if (ImGui::Checkbox("Probe every 2 s (debug)", &enabled)) {
        m_settings.probeVariables = enabled;
        SaveSettings();
      }
      ImGui::SameLine();
      if (ImGui::Button("Probe now")) ProbeVariablesIfEnabled(true);
      ImGui::TextWrapped("GUI: %s", m_varProbeSummary.c_str());
      ImGui::TextWrapped("Global: %s", m_globalProbeSummary.c_str());
      bool write = m_settings.probeWrite;
      if (ImGui::Checkbox("Write test (debug, SSO only)", &write)) {
        m_settings.probeWrite = write;
        m_writeTestDone = false;
        SaveSettings();
      }
      ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.3f, 1.0f),
                         "Writes one debug key into the GUI variable map; never live data.");
      ImGui::TextDisabled("Read-only default: verifies the map layout; writes only when explicitly enabled (DECISIONS D17).");
    }

    if (ImGui::CollapsingHeader("Calibration", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::TextWrapped(
          "Authored mode preserves the game's stored path points and direction. "
          "Province and straight modes are explicit approximations. Type exact values in the fields, "
          "then Save.");
      float scale = m_settings.coordScale;
      if (SliderFloatWithInput("Coordinate scale", &scale, 0.25f, 4.0f, "%.3f", 0.01f)) {
        m_settings.coordScale = scale;
        if (m_wantsRoute.load()) RequestRoute(m_from, m_to);
      }
      float hs = m_settings.heightScale;
      if (SliderFloatWithInput("Height scale", &hs, 0.0f, 0.1f, "%.5f", 0.0005f)) {
        m_settings.heightScale = hs;
        if (m_wantsRoute.load()) RequestRoute(m_from, m_to);
      }
      float ho = m_settings.heightOffset;
      if (SliderFloatWithInput("Height offset", &ho, -2000.0f, 2000.0f, "%.1f", 1.0f)) {
        m_settings.heightOffset = ho;
        if (m_wantsRoute.load()) RequestRoute(m_from, m_to);
      }
      if (ImGui::Button("Reset calibration")) {
        m_settings.coordScale = 1.0f;
        m_settings.heightScale = 0.0f;
        m_settings.heightOffset = 0.0f;
        SaveSettings();
        if (m_wantsRoute.load()) RequestRoute(m_from, m_to);
      }
      ImGui::SameLine();
      if (ImGui::Button("Save settings")) SaveSettings();
      ImGui::SameLine();
      ImGui::TextDisabled("%s", Narrow(m_dataDir).c_str());
    }

    ImGui::Separator();
    if (ImGui::CollapsingHeader("Diagnostics", ImGuiTreeNodeFlags_DefaultOpen)) {
      ImGui::Text("frame %llu   pass calls %llu   submitted %llu   dup-skipped %llu",
                  (unsigned long long)Game::Instance().Frame(), (unsigned long long)m_passCalls,
                  (unsigned long long)m_submitted, (unsigned long long)m_skippedDuplicate);
      ImGui::Text("route uploads %llu   build %.2f ms   capture left %u",
                  (unsigned long long)m_uploadedRoutes, m_lastBuildMs, m_captureRemaining);
      ImGui::Text("cache hits %llu; authored graph %zu junctions / %zu strips",
                  (unsigned long long)m_cacheHits, m_geography->splines.nodes.size(), m_geography->splines.edges.size());
      ImGui::Text("path points %u   visible %s   native pool %zu / 8192",
                  m_slot ? m_slot->path.points.count : 0, m_routeActive ? "yes" : "no", m_slots.size());
      ImGui::Text("ribbons %zu ready / %zu pending; %llu failed / %llu dropped; upload %.3f ms",
                  m_ribbons.Active().size(), m_ribbons.Pending(),
                  (unsigned long long)m_ribbons.Failures(), (unsigned long long)m_ribbons.Dropped(),
                  m_ribbons.FrameSeconds() * 1000.0);
      ImGui::Text("graph %zu nodes / %zu edges   mode %s", m_geography->provinces.nodes.size(),
                  m_geography->provinces.edges.size(), m_settings.routeMode == 2 ? "authored" : (m_settings.routeMode == 1 ? "province" : "straight"));
      ImGui::TextWrapped("last: %s", m_lastResult);
      ImGui::Text("log: %s", Narrow(LogPath()).c_str());
      if (ImGui::Button("Capture frame (readback.ppm)")) m_captureFrameRequested = true;
      ImGui::SameLine();
      if (ImGui::Button("Write diagnostics to log")) {
        V3_INFO("DIAG build=%s ready=%d hooks=%d type=%p enabled=%d mode=%d pass=%llu "
                "submitted=%llu uploads=%llu active=%d auto=%d last=%s",
                kGameVersion, (int)Game::Instance().Ready(), (int)m_nativeHooks, m_type,
                (int)m_settings.nativeEnabled, m_settings.routeMode,
                (unsigned long long)m_passCalls, (unsigned long long)m_submitted,
                (unsigned long long)m_uploadedRoutes, (int)m_routeActive,
                (int)m_settings.autoTest, m_lastResult);
        LogFlush(true);
      }
    }
    ImGui::Separator();
}

void Overlay::QueueLogisticsSelection(bool active) {
  auto selection=m_logisticsPanel.Selection();
  selection.flowsVisible=active && selection.flowsVisible;
  if (!active) selection.hoverGood=-1;
  LockGuard lock(m_logisticsMutex);m_logisticsRequest=std::move(selection);
}

void Overlay::DrawUi() {
  const auto nativeActive = [this] {
    return m_logisticsSession && IsNativeLogisticsOwner(m_logisticsSession->Status().owner);
  };
  if (!m_visible) {
    if (m_logisticsWasActive) QueueLogisticsSelection(false);
    m_logisticsWasActive=false;
    if (m_logisticsSession) {
      m_logisticsSession->Close(kOwnerDebugPreview);
      if (!nativeActive() && !m_settings.probeEconomy) PublishLogisticsDocument(nullptr);
    }
    return;
  }
  if (m_logisticsSession) {
    if (m_viewMode != 1 && m_viewMode != 2) {
      m_logisticsSession->Close(kOwnerDebugPreview);
      if (!nativeActive() && !m_settings.probeEconomy) PublishLogisticsDocument(nullptr);
    } else {
      if (auto request = TakeLogisticsRequest())
        m_logisticsSession->SetSelection(kOwnerDebugPreview, kOwnerDebugPreview, *request);
      if (auto document = m_logisticsSession->Poll()) PublishLogisticsDocument(std::move(document));
    }
  }
  const auto display = ImGui::GetIO().DisplaySize;
  ImGui::SetNextWindowSize(ImVec2(std::min(620.f, display.x - 30), std::min(850.f, display.y - 45)), ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowPos(ImVec2(20, 25), ImGuiCond_FirstUseEver);
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(.10f,.12f,.135f,.97f));
  ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(.16f,.23f,.28f,1));
  if (ImGui::Begin(kWindowTitle, &m_visible)) {
    static constexpr const char* modes[] = {"UI prototype", "State prototype", "Route preview", "Diagnostics"};
    static constexpr int values[] = {2,1,0,3};
    const float width=(ImGui::GetContentRegionAvail().x-ImGui::GetStyle().ItemSpacing.x*3)/4;
    for (int i=0;i<4;++i) {
      if (i) ImGui::SameLine();
      const bool selected=m_viewMode==values[i];
      if (selected) ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(.35f,.30f,.19f,1));
      const bool clicked=ImGui::Button(modes[i],ImVec2(width,30));
      if (selected) ImGui::PopStyleColor();
      if (clicked && !selected) { ClearRoute(); m_viewMode=values[i]; }
    }
    ImGui::Separator();
    if (m_viewMode==1 || m_viewMode==2) {
      std::shared_ptr<const LogisticsDocument> document;
      { LockGuard lock(m_logisticsMutex); document=m_logisticsDocument; }
      if (m_logisticsPanel.Draw(document.get(),m_viewMode==1?FreightViewKind::State:FreightViewKind::Market))
        QueueLogisticsSelection(m_visible);
    } else if (m_viewMode==0) DrawPreviewControls();
    else DrawDiagnostics();
    ImGui::Spacing();ImGui::TextDisabled("Insert toggles this panel.");
  }
  ImGui::End();ImGui::PopStyleColor(2);
  const bool active=m_visible && (m_viewMode==1 || m_viewMode==2);
  if (active!=m_logisticsWasActive) QueueLogisticsSelection(active);
  m_logisticsWasActive=active;
  if (!m_visible) {
    ClearRoute();
  }
}

LRESULT CALLBACK Overlay::WndProcThunk(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
  Overlay& o = Instance();
  if (msg == WM_APP + 0x51) {
    o.OnUiThreadPublish();
    return 0;
  }
  if (msg == WM_TIMER && wp == 0x4952) {
    o.PollHideFlows();
    return 0;
  }
  if (msg == WM_TIMER && wp == 0x4953) {
    o.PollNativeMarketPanel();
    return 0;
  }
  if (o.m_imguiReady) {
    ImGui_ImplWin32_WndProcHandler(hwnd, msg, wp, lp);
  }
  if (msg == WM_KEYDOWN && wp == VK_INSERT) {
    o.m_visible = !o.m_visible;
    if (!o.m_visible) o.ClearRoute();
    return 0;
  }
  if (o.m_visible && o.m_imguiReady) {
    ImGuiIO& io = ImGui::GetIO();
    if (IsMouseMessage(msg) && io.WantCaptureMouse) return 0;
    if ((msg == WM_KEYDOWN || msg == WM_KEYUP || msg == WM_CHAR) && io.WantTextInput) return 0;
  }
  return CallWindowProcW(o.m_origWndProc, hwnd, msg, wp, lp);
}

}  // namespace v3









