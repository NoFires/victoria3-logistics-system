// Exact-build read-only diagnostic. Verify the EXE hash in PROFILE.md first.
// No attach, writes, suspension, native getter, input or game launch.
#include "native_shipping.h"
#include "native_world_trade.h"
#include "native_splines.h"
#include "native_sea_geography.h"
#include "native_ports.h"
#include "native_trade_routes.h"
#include "native_trade_geometry.h"
#include "native_domestic.h"
#include "native_domestic_model.h"
#include "goods_snapshot.h"
#include <windows.h>
#include <tlhelp32.h>
#include <cstdio>

namespace {
HANDLE process=nullptr;
bool Read(uint64_t address,void* out,size_t size){
  SIZE_T got=0;
  return address && size<=UINT64_MAX-address &&
      ReadProcessMemory(process,(LPCVOID)address,out,size,&got) && got==size;
}
struct Identity {
  uint64_t root=0,machine=0,world=0,states=0;
  uint32_t phase=0;
  bool operator==(const Identity&)const=default;
};
bool Observe(uint64_t module,Identity& id){
  return Read(module+0x0592d128,&id.root,8) && Read(id.root+0x588,&id.phase,4) && id.phase==4 &&
      Read(id.root+0x608,&id.machine,8) && id.machine && Read(id.machine+0x120,&id.world,8) && id.world &&
      Read(module+0x05964ef0,&id.states,8) && id.states;
}
struct Window {DWORD pid=0,thread=0;unsigned count=0;};
BOOL CALLBACK FindWindow(HWND window,LPARAM parameter){
  auto& result=*(Window*)parameter;DWORD pid=0;wchar_t title[256]{},kind[64]{};
  const DWORD thread=GetWindowThreadProcessId(window,&pid);
  if(pid==result.pid && GetWindowTextW(window,title,256) && !wcscmp(title,L"Victoria 3") &&
     GetClassNameW(window,kind,64) && !wcscmp(kind,L"SDL_app")){
    result.thread=thread;++result.count;
  }
  return TRUE;
}
struct ThreadBasicInfo {LONG status;void* teb;ULONG_PTR processId,threadId,affinity;LONG priority,basePriority;};
using QueryThread=LONG(NTAPI*)(HANDLE,ULONG,void*,ULONG,ULONG*);
struct ThreadContext {
  uint64_t teb=0,array=0,storage=0,token=0;
  bool operator==(const ThreadContext&)const=default;
};
bool ObserveContext(HANDLE thread,QueryThread query,DWORD pid,ThreadContext& result){
  ThreadBasicInfo info{};DWORD exit=0;uint32_t initialized=0;
  if(!thread || !query || !GetExitCodeThread(thread,&exit) || exit!=STILL_ACTIVE ||
     query(thread,0,&info,sizeof(info),nullptr)<0 || info.processId!=pid || !info.teb)return false;
  result.teb=(uint64_t)info.teb;
  return result.teb<=UINT64_MAX-0x60 && Read(result.teb+0x58,&result.array,8) &&
      Read(result.array,&result.storage,8) && result.storage<=UINT64_MAX-0x8650 &&
      Read(result.storage+0x8600,&initialized,4) && (initialized&1) &&
      Read(result.storage+0x8648,&result.token,8) && uint32_t(result.token)<16;
}
}
int main(int argc,char** argv){
  if((argc!=2 && argc!=4 && argc!=5) || (argc==5 && std::string(argv[4])!="--sea-geography")){
    std::puts("usage: live_shipping_probe <Victoria3 PID> [--world-trade <verified goods count> [--sea-geography]]; verify exact EXE hash first");return 2;
  }
  DWORD pid=0;uint32_t goodCount=0;
  try{const std::string arg=argv[1];size_t used=0;const auto value=std::stoull(arg,&used);
    if(used!=arg.size() || !value || value>MAXDWORD)return 2;pid=DWORD(value);
    if(argc>=4){
      const std::string count=argv[3];used=0;const auto number=std::stoull(count,&used);
      if(std::string(argv[2])!="--world-trade" || used!=count.size() || !number || number>128)return 2;
      goodCount=uint32_t(number);
    }
  }catch(...){return 2;}
  process=OpenProcess(PROCESS_VM_READ|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
  if(!process)return 3;
  HANDLE modules=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);
  MODULEENTRY32W entry{};entry.dwSize=sizeof(entry);uint64_t module=0;
  if(Module32FirstW(modules,&entry))do{
    if(!_wcsicmp(entry.szModule,L"victoria3.exe")){module=(uint64_t)entry.modBaseAddr;break;}
  }while(Module32NextW(modules,&entry));
  CloseHandle(modules);Identity before;
  if(!module || !Observe(module,before)){CloseHandle(process);return 4;}
  Window window{pid};HANDLE thread=nullptr;ThreadContext context;
  auto query=(QueryThread)GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"NtQueryInformationThread");
  if(argc==5){
    EnumWindows(FindWindow,(LPARAM)&window);
    if(window.count==1)thread=OpenThread(THREAD_QUERY_INFORMATION,FALSE,window.thread);
    if(!ObserveContext(thread,query,pid,context)){
      std::puts("current game window definition context unavailable");
      if(thread)CloseHandle(thread);CloseHandle(process);return 4;
    }
  }
  std::string error;
  v3::GoodsCatalog copiedCatalog;
  auto current=[&]{
    Identity now;DWORD exit=0;ThreadContext after;
    return GetExitCodeProcess(process,&exit) && exit==STILL_ACTIVE && Observe(module,now) && now==before &&
        (!thread || (ObserveContext(thread,query,pid,after) && after==context));
  };
  auto copy=v3::DecodeNativeShipping(Read,module,current,&error);
  std::optional<v3::NativeWorldTradeSnapshot> trade;
  if(copy && goodCount){
    v3::EconomyReadWindow window(Read);
    v3::GoodsMemoryReader read=[&](uint64_t a,void* p,size_t n){return window.Read(a,p,n);};
    auto roster=v3::DecodeNativeTradeStates(read,module,current,&error);
    std::vector<v3::NativeStateEconomy> states=roster?std::move(*roster):std::vector<v3::NativeStateEconomy>{};
    if(thread){
      const auto catalog=v3::DecodeThreadGoodsCatalog(read,module,context.array);
      copiedCatalog=catalog;
      if(!catalog.ok || catalog.goods.size()!=goodCount){states.clear();error="current native goods catalog/count mismatch";}
    }
    if(!states.empty())trade=v3::DecodeNativeWorldTrade(read,module,states,goodCount,current,{},&error);
    else if(error.empty())error="current state roster unavailable";
    if(trade){
      auto geometry=v3::DecodeNativeSplineNetwork(read,module,current,&error);
      if(geometry){
        auto domestic=v3::DecodeNativeDomesticSources(read,module,states,goodCount,*geometry,current,&error);
        if(domestic){
          trade->domestic=std::make_shared<const v3::NativeDomesticSources>(std::move(*domestic));
          trade->geometry=std::make_shared<const v3::NativeSplineNetwork>(std::move(*geometry));
        }else trade.reset();
      }else trade.reset();
    }
    if(trade && thread){
      auto sea=v3::DecodeNativeSeaGeography(read,module,context.array,states,*trade,current,&error);
      if(sea){
        auto hubs=v3::DecodeNativePortHubs(read,module,sea->states,current,&error);
        if(hubs){sea->stateHubs=std::move(*hubs);trade->seaGeography=std::make_shared<const v3::NativeSeaGeography>(std::move(*sea));}
        else trade.reset();
      }else trade.reset();
    }
    if(trade && (!window.Unchanged() || !current())){trade.reset();error="combined world trade source changed";}
  }
  if(thread)CloseHandle(thread);CloseHandle(process);process=nullptr;
  std::printf("pid=%lu module=%llx phase=%u (external sequential observation, not an atomic game scope)\n",
      pid,(unsigned long long)module,before.phase);
  if(!copy){std::printf("FAIL: %s\n",error.c_str());return 5;}
  if(goodCount && !trade){std::printf("FAIL world trade: %s\n",error.c_str());return 6;}
  size_t ports=0,transfers=0;
  for(size_t i=0;i<copy->lanes.size();++i){
    const auto& lane=copy->lanes[i];if(lane.kind==0)++ports;else ++transfers;
    size_t provinces=0;for(const auto& range:lane.provinces)provinces+=uint32_t(range.last)-range.first+1;
    std::printf("lane=%u kind=%s country=%u from=%u to=%u treaty=%u ranges=%zu provinces=%zu merchantMarineFixed=%lld travelDistanceFixed=%lld\n",
        lane.id,lane.kind==0?"port_connection":"goods_transfer",lane.country,lane.fromState,lane.toState,
        lane.treaty,lane.provinces.size(),provinces,(long long)lane.merchantMarineDemandFixed,(long long)lane.travelDistanceFixed);
  }
  std::printf("PASS owned shipping copy lanes=%zu port_connections=%zu goods_transfers=%zu; goods/actual delivery/geometry not decoded\n",
      copy->lanes.size(),ports,transfers);
  if(trade){
    if(trade->domestic){
      std::map<uint32_t,size_t> types;
      for(const auto& link:trade->domestic->land)++types[link.type];
      std::printf("PASS current domestic sources local_orders=%zu country_port_lanes=%zu land=%zu category2=%zu category3=%zu category4=%zu\n",
          trade->domestic->orders.size(),trade->domestic->shipping.lanes.size(),trade->domestic->land.size(),types[2],types[3],types[4]);
      if(copiedCatalog.ok && trade->seaGeography){
        v3::EconomySnapshot snapshot;snapshot.world=1;snapshot.revision=1;
        if(!v3::PopulateSnapshotGoods(snapshot,copiedCatalog,&error)){std::printf("FAIL domestic catalog: %s\n",error.c_str());return 10;}
        snapshot.nativeTrade=std::make_shared<const v3::NativeWorldTradeSnapshot>(*trade);
        auto model=v3::BuildNativeDomesticModel(snapshot,{},&error);
        if(!model){std::printf("FAIL domestic model: %s\n",error.c_str());return 10;}
        size_t seaEdge=model->landConnections;
        for(const auto& lane:trade->domestic->shipping.lanes){
          const auto from=std::find_if(trade->states.begin(),trade->states.end(),[&](const auto& state){return state.id==lane.fromState;});
          const auto to=std::find_if(trade->states.begin(),trade->states.end(),[&](const auto& state){return state.id==lane.toState;});
          if(from==trade->states.end() || to==trade->states.end() || from->market!=to->market || lane.provinces.empty())continue;
          const size_t edgeId=seaEdge++;
          if(!model->edges.at(edgeId).complete){
            int64_t quantity=0;size_t count=0;
            for(const auto& shipment:model->allocation.shipments)for(const auto& edge:shipment.route)if(edge.id==edgeId){quantity+=shipment.quantity;++count;}
            std::printf("domestic partial lane id=%u from=%u to=%u nativeDistanceFixed=%lld allocated_milli=%lld shipments=%zu drawn_strips=%zu\n",
                lane.id,lane.fromState,lane.toState,(long long)lane.travelDistanceFixed,(long long)quantity,count,model->edges.at(edgeId).strips.size());
          }
          v3::NativeTradeDestination destination;destination.state=lane.toState;destination.selected=0;
          v3::NativeTradePath path;path.provinces=lane.provinces;for(const auto& strait:trade->straits)path.straits.push_back(strait.id);
          destination.candidates.push_back(std::move(path));std::string why;
          if(!v3::BuildNativeTradeGeometry(*trade,lane.fromState,destination,&why)){
            const auto& sea=*trade->seaGeography;
            auto links=v3::SelectNativeRouteConnections(*trade,sea,lane.fromState,destination);
            std::printf("domestic lane full geometry unavailable id=%u from=%u to=%u reason=%s geo=%u/%u hubs=%d/%d ports=%d/%d\n",
                lane.id,lane.fromState,lane.toState,why.c_str(),sea.states.at(lane.fromState).geo,sea.states.at(lane.toState).geo,
                int(sea.stateHubs.at(lane.fromState)),int(sea.stateHubs.at(lane.toState)),int(links && links->ports[0]),int(links && links->ports[1]));
          }
        }
        size_t landShipments=0,seaShipments=0;int64_t marketQuantity=0;
        for(const auto& shipment:model->allocation.shipments){
          bool sea=false,land=false;
          for(const auto& edge:shipment.route)for(const auto& strip:model->edges.at(edge.id).strips){
            sea|=(strip.id&63)==2 || (strip.id&63)==3;land|=(strip.id&63)<=1;
          }
          landShipments+=land;seaShipments+=sea;
          if(shipment.market==0)marketQuantity+=shipment.quantity;
        }
        std::printf("PASS owned domestic replay AFTER PROCESS HANDLE CLOSED landLinks=%u seaLinks=%u unavailableSea=%u partialSea=%u shipments=%zu landShipments=%zu seaShipments=%zu market0_quantity_milli=%lld\n",
            model->landConnections,model->seaConnections,model->unavailableSea,model->partialSea,model->allocation.shipments.size(),landShipments,seaShipments,(long long)marketQuantity);
      }
    }
    size_t destinations=0,candidates=0,selected=0,amounts=0;
    for(const auto& group:trade->groups)for(const auto& dest:group.destinations){
      ++destinations;candidates+=dest.candidates.size();if(dest.selected)++selected;
    }
    for(const auto& pair:trade->statePairs)amounts+=pair.goods.size();
    std::printf("PASS owned world trade copy groups=%zu destinations=%zu candidates=%zu selected=%zu statePairs=%zu goodsRecords=%zu straits=%zu\n",
        trade->groups.size(),destinations,candidates,selected,trade->statePairs.size(),amounts,trade->straits.size());
    std::map<uint32_t,uint32_t> membership;
    for(const auto& group:trade->groups)for(uint32_t state:group.states)membership.emplace(state,group.anchor);
    size_t nonempty=0,unknownSource=0,unknownDestination=0,sameGroup=0,crossGroup=0;
    for(const auto& pair:trade->statePairs)if(!pair.goods.empty()){
      ++nonempty;auto from=membership.find(pair.source),to=membership.find(pair.destination);
      if(from==membership.end())++unknownSource;
      if(to==membership.end())++unknownDestination;
      if(from!=membership.end() && to!=membership.end()){
        if(from->second==to->second)++sameGroup;else ++crossGroup;
      }
    }
    std::printf("native group membership positivePairs=%zu missingSource=%zu missingDestination=%zu sameGroup=%zu crossGroup=%zu\n",
        nonempty,unknownSource,unknownDestination,sameGroup,crossGroup);
    auto bindings=v3::BindNativeTradeRoutes(*trade,&error);
    if(!bindings){std::printf("FAIL native trade association: %s\n",error.c_str());return 9;}
    size_t groupedAmounts=0,boundSelected=0,boundMissing=0,boundEmpty=0,boundUnselected=0;
    for(const auto& pair:trade->groupPairs)groupedAmounts+=pair.goods.size();
    for(const auto& binding:*bindings)if(binding.sourceCenter!=binding.destinationCenter){
      if(!binding.sourceGroup || !binding.route){++boundMissing;continue;}
      const auto& route=trade->groups[*binding.sourceGroup].destinations[*binding.route];
      if(route.candidates.empty())++boundEmpty;else if(route.selected)++boundSelected;else ++boundUnselected;
    }
    std::printf("PASS native exact detailed/grouped exports groupPairs=%zu groupedGoods=%zu bindings=%zu crossSelected=%zu crossMissing=%zu crossEmpty=%zu crossUnselected=%zu\n",
        trade->groupPairs.size(),groupedAmounts,bindings->size(),boundSelected,boundMissing,boundEmpty,boundUnselected);
    if(trade->seaGeography){
      std::map<std::pair<size_t,size_t>,bool> checked;
      std::map<std::string,size_t> unavailable;
      size_t drawableFlows=0,unavailableFlows=0,directedStrips=0;
      for(const auto& binding:*bindings)if(binding.sourceGroup && binding.route){
        const auto& route=trade->groups[*binding.sourceGroup].destinations[*binding.route];
        if(!route.selected)continue;
        const auto key=std::pair{*binding.sourceGroup,*binding.route};
        auto result=checked.find(key);
        if(result==checked.end()){
          std::string why;
          auto geometry=v3::BuildNativeTradeGeometry(*trade,binding.sourceCenter,route,&why);
          result=checked.emplace(key,bool(geometry)).first;
          if(geometry)directedStrips+=geometry->strips.size();else ++unavailable[why];
        }
        if(result->second)++drawableFlows;else ++unavailableFlows;
      }
      std::printf("native geometry audit selectedFlows=%zu uniqueRoutes=%zu drawableFlows=%zu unavailableFlows=%zu directedStrips=%zu (not formal rendering acceptance)\n",
          drawableFlows+unavailableFlows,checked.size(),drawableFlows,unavailableFlows,directedStrips);
      for(const auto& [why,count]:unavailable)std::printf("native geometry unavailable uniqueRoutes=%zu reason=%s\n",count,why.c_str());
    }
    size_t shown=0;
    for(const auto& pair:trade->statePairs)for(const auto& amount:pair.goods)if(shown<8){
      std::printf("native_export source=%u destination=%u good=%u quantityFixed=%lld scale=%lld\n",
          pair.source,pair.destination,amount.good,(long long)amount.valueFixed,(long long)v3::kNativeTradeQuantityScale);
      ++shown;
    }
    std::puts("multiple alternatives remain unselected: external probe never invokes the native selector; no route drawn or statistic published");
    const auto& geometry=*trade->geometry;
    auto graph=v3::BuildNativeSplineGraph(geometry,&error);
    if(!graph){std::printf("FAIL native geometry conversion: %s\n",error.c_str());return 7;}
    std::printf("PASS owned current spline geometry map=%ux%u points=%zu native_segments=%zu connections=%zu graph_edges=%zu graph_controls=%zu\n",
        geometry.width,geometry.height,geometry.points.size(),geometry.strips.size(),geometry.connections.size(),
        graph->edges.size(),graph->points.size());
    std::puts("geometry membership does not establish canal access; native selected path/passages still govern drawing");
    if(trade->seaGeography){
      const auto& sea=*trade->seaGeography;size_t mapped=0,keys=0,blocked=0,ports=0;
      for(const auto& group:trade->groups)for(const auto& destination:group.destinations)if(destination.selected){
        auto selected=v3::SelectNativeRouteConnections(*trade,sea,group.anchor,destination,&error);
        if(!selected){std::printf("FAIL native sea connections: %s\n",error.c_str());return 8;}
        ++mapped;keys+=selected->sea.keys.size();blocked+=selected->sea.blockedStraitAdjacencies;
        for(const auto& port:selected->ports)if(port)++ports;
      }
      std::printf("PASS owned sea geography windowThread=%lu context=%u definitions=%zu provinces=%zu states=%zu mappedSelected=%zu keys=%zu ports=%zu blockedStraitAdjacencies=%zu\n",
          window.thread,uint32_t(context.token),sea.definitions.size(),sea.provinces.size(),sea.states.size(),mapped,keys,ports,blocked);
    }
  }
  return 0;
}
