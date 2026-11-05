#include "native_access.h"
#include "native_strait_probe.h"
#include "native_path_probe.h"
#include "native_sea_cache.h"
#include <cstdio>
#include <cstdlib>
#include <map>
using namespace v3;
void Check(bool ok,const char* why){if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
int main(){
  constexpr uint64_t module=0x140000000,tlsArray=0x10000,tls=0x20000,
      root=0x30000,machine=0x40000,world=0x50000,db=0x60000,idler=0x70000;
  std::map<uint64_t,uint8_t> bytes;
  auto put=[&](uint64_t at,auto value){for(size_t i=0;i<sizeof(value);++i)bytes[at+i]=((uint8_t*)&value)[i];};
  GoodsMemoryReader read=[&](uint64_t at,void* out,size_t n){
    for(size_t i=0;i<n;++i){auto f=bytes.find(at+i);if(f==bytes.end())return false;((uint8_t*)out)[i]=f->second;}return true;
  };
  put(tlsArray,tls);put(tls+0x5b60,uint8_t(1));put(tls+0x34c,uint32_t(0));
  put(module+0x05404748,uint32_t(2));put(module+0x05404738,uint32_t(0));
  put(module+0x05c07c18,idler);put(idler,module+0x04678bc0);put(idler+0x20,uint8_t(1));
  put(module+0x05c07c68,root);put(root,module+0x04660618);put(root+0x588,uint32_t(4));put(root+0x608,machine);
  put(machine+8,uint32_t(59883360));put(machine+0x120,world);put(module+0x05bd69c8,db);
  auto decode=[&]{return ObserveNativeAccess(read,module,tlsArray);};
  auto original=bytes;auto s=decode();
  Check(s && s->access==2 && s->owns==1 && s->context==0 && s->date==59883360 &&
        s->machine==machine && s->world==world && s->stateDb==db,"copies native access and world identities");
  for(uint32_t access=0;access<=3;++access){put(module+0x05404748,access);Check(bool(decode()),"all native modes observed without granting permission");}
  put(module+0x05404748,uint32_t(4));Check(!decode(),"unknown access rejected");bytes=original;
  put(module+0x05404738,uint32_t(2));Check(!decode(),"unknown hidden mode rejected");bytes=original;
  put(tls+0x5b60,uint8_t(2));Check(!decode(),"invalid thread ownership rejected");bytes=original;
  put(idler+0x20,uint8_t(2));Check(!decode(),"invalid paused state rejected");bytes=original;
  put(root,module+0x04430808);Check(!decode(),"wrong application type rejected");bytes=original;
  put(machine+0x120,uint64_t(0));Check(!decode(),"missing world rejected");bytes=original;
  put(module+0x05bd69c8,uint64_t(0));Check(!decode(),"missing state database rejected");bytes=original;
  for(const auto& [address,value]:original){bytes=original;bytes.erase(address);Check(!decode(),"every unreadable field rejected");}
  bytes=original;put(tlsArray,UINT64_MAX);Check(!decode(),"TLS address overflow rejected");
  Check(!ObserveNativeAccess(read,UINT64_MAX,tlsArray),"module overflow rejected");
  Check(!ObserveNativeAccess({},module,tlsArray) && !ObserveNativeAccess(read,module,0),"missing reader or TLS rejected");
  // The following opt-in diagnostic fixtures retain their explicit 1.13.11 ABI.
  // They do not validate 1.15 runtime support; those probes remain disabled.
  put(module+0x0592d128,root);put(module+0x05964ef0,db);original=bytes;
  {
    bytes=original;constexpr uint64_t heapTls=0x81000,heap=0x82000,descriptor=0x83000,data=0x84000;
    put(module+0x058f8c90,uint32_t(3));put(tlsArray+24,heapTls);put(heapTls+0x38,heap);put(heapTls+0x44,uint8_t(1));
    put(module+0x0512cdc0,module+0x04419230);put(module+0x04419238,module+0x0061be50);
    put(module+0x04419240,module+0x0061be90);
    put(descriptor,NativeTradeBufferObservation{{data,7,3},module+0x0512cdc0});
    auto allocator=[&]{return ObserveNativeThreadAllocator(read,module,tlsArray);};
    auto buffer=[&]{return ObserveNativeTradeBuffer(read,descriptor);};
    const auto stable=bytes;auto observed=allocator();auto owned=buffer();
    Check(observed && observed->index==3 && observed->tls==heapTls && observed->heap==heap,
        "native heap observation follows actual TLS index and exact allocator methods");
    Check(owned && owned->array.data==data && owned->array.capacity==7 && owned->array.count==3 &&
        owned->allocator==module+0x0512cdc0,"trade buffer descriptor preserves pointer, capacity, count and allocator");
    for(const auto& [at,value]:stable)if(!original.count(at)){
      bytes.erase(at);
      if(at>=descriptor && at<descriptor+24)Check(!buffer(),"every unreadable buffer descriptor byte rejected");
      else Check(!allocator(),"every unreadable thread allocator field rejected");
      bytes=stable;
    }
    auto badAllocator=[&](auto change){bytes=stable;change();Check(!allocator(),"invalid current native allocator rejected");};
    badAllocator([&]{put(module+0x058f8c90,uint32_t(4096));});
    badAllocator([&]{put(tlsArray+24,uint64_t(0));});
    badAllocator([&]{put(tlsArray+24,UINT64_MAX-0x20);});
    badAllocator([&]{put(heapTls+0x38,uint64_t(0));});
    badAllocator([&]{put(heapTls+0x44,uint8_t(0));});
    badAllocator([&]{put(heapTls+0x44,uint8_t(2));});
    badAllocator([&]{put(module+0x0512cdc0,module+0x04419238);});
    badAllocator([&]{put(module+0x04419238,module+0x0061be58);});
    badAllocator([&]{put(module+0x04419240,module+0x0061be98);});
    auto badBuffer=[&](NativeTradeBufferObservation value){bytes=stable;put(descriptor,value);Check(!buffer(),"malformed trade buffer descriptor rejected");};
    badBuffer({{data,2,3},module+0x0512cdc0});badBuffer({{data,65537,3},module+0x0512cdc0});
    badBuffer({{data,5000,4097},module+0x0512cdc0});badBuffer({{UINT64_MAX-3,2,1},module+0x0512cdc0});
    badBuffer({{0,2,0},module+0x0512cdc0});badBuffer({{data,0,0},module+0x0512cdc0});badBuffer({{data,2,1},0});
    bytes=stable;put(descriptor,NativeTradeBufferObservation{{data,7,0},module+0x0512cdc0});
    Check(bool(buffer()),"empty count may retain an allocated native buffer");
    put(descriptor,NativeTradeBufferObservation{{0,0,0},module+0x0512cdc0});Check(bool(buffer()),"moved-from native buffer may be empty");
    bytes=stable;unsigned seen=0;
    GoodsMemoryReader changing=[&](uint64_t at,void* p,size_t n){
      if(at==heapTls+0x38 && ++seen==2)put(heapTls+0x38,heap+8);return read(at,p,n);
    };
    Check(!ObserveNativeThreadAllocator(changing,module,tlsArray),"native thread heap changes during observation rejected");bytes=stable;seen=0;
    changing=[&](uint64_t at,void* p,size_t n){
      if(at==descriptor && ++seen==2)put(descriptor+16,module+0x0512cdc8);return read(at,p,n);
    };
    Check(!ObserveNativeTradeBuffer(changing,descriptor),"buffer allocator changes during observation rejected");
    Check(!ObserveNativeThreadAllocator({},module,tlsArray) && !ObserveNativeThreadAllocator(read,UINT64_MAX,tlsArray) &&
        !ObserveNativeThreadAllocator(read,module,UINT64_MAX) && !ObserveNativeTradeBuffer({},descriptor) &&
        !ObserveNativeTradeBuffer(read,UINT64_MAX),"missing reader and overflowing allocator/buffer addresses rejected");
    bytes.clear();Check(owned->array.count==3 && observed->heap==heap,"diagnostic observations own their scalar values after source destruction");bytes=original;
  }
  {
    bytes=original;
    constexpr uint64_t cache=0xa0000,index=0xa1000,matrix=0xa2000,entries=0xa3000,ranges=0xa4000;
    put(world+0x4500,cache);put(cache,module+0x044eb4d8);
    put(cache+8,std::array<uint8_t,0x28>{});put(cache+0x10,index);put(cache+0x18,uint32_t(3));
    put(cache+0x1c,uint32_t(3));put(cache+0x20,uint8_t(2));
    put(index,std::array<uint8_t,72>{});
    for(auto item:{std::array<uint32_t,4>{2,1,10,0},{0,1,20,1},{3,2,30,2}}){
      put(index+item[0]*12,uint8_t(item[1]));put(index+item[0]*12+4,uint16_t(item[2]));put(index+item[0]*12+8,item[3]);
    }
    put(cache+0x30,std::array<uint8_t,24>{});put(cache+0x30,matrix);put(cache+0x38,uint32_t(3));put(cache+0x3c,uint32_t(3));
    put(matrix,std::array<uint8_t,72>{});put(matrix,entries);put(matrix+8,uint32_t(2));put(matrix+12,uint32_t(2));
    put(entries,std::array<uint8_t,64>{});put(entries+32,ranges);put(entries+40,uint32_t(3));put(entries+44,uint32_t(3));
    put(entries+56,int64_t(0x123456789ab));put(ranges,std::array<uint16_t,6>{30,0,15,1,10,0});
    bool live=true;std::string why;
    auto copy=[&]{return DecodeNativeSeaCacheRoute(read,module,10,30,[&]{return live;},&why);};
    const auto stable=bytes;auto output=copy();
    if(!output)std::printf("sea cache reason: %s\n",why.c_str());
    Check(output && output->source==10 && output->destination==30 && output->distanceFixed==0x123456789ab &&
        output->provinces==std::vector<NativeProvinceRange>({{30,30},{15,16},{10,10}}),
        "current typed sea cache uses native collision lookup and triangular column, preserving distance64 and membership");
    auto reverse=DecodeNativeSeaCacheRoute(read,module,30,10,[&]{return live;});
    Check(reverse && reverse->source==30 && reverse->destination==10 && reverse->provinces==output->provinces,
        "unordered native cache pair preserves the requested orientation without reversing membership");
    auto empty=DecodeNativeSeaCacheRoute(read,module,10,20,[&]{return live;});
    Check(empty && empty->provinces.empty() && empty->distanceFixed==0,"existing empty cache entry is observable, not an unreachable-search claim");
    for(const auto& [at,value]:stable)if(!original.count(at)){
      // Only row0/column1 and its ranges are part of this lookup's read set.
      if((at>=matrix+24 && at<matrix+72) || (at>=entries && at<entries+32))continue;
      bytes.erase(at);Check(!copy(),"every unreadable cache dependency rejected");bytes=stable;
    }
    auto bad=[&](auto change){bytes=stable;change();Check(!copy(),"malformed current cached sea route rejected");};
    bad([&]{put(cache,module+0x044eb4d0);});
    bad([&]{put(cache+0x1c,uint32_t(5));});
    bad([&]{put(cache+0x18,uint32_t(2));});
    bad([&]{put(cache+0x20,uint8_t(255));});
    bad([&]{put(index+36,uint8_t(255));});
    bad([&]{put(index+40,uint16_t(10));});
    bad([&]{put(index+44,uint32_t(0));});
    bad([&]{put(cache+0x10,UINT64_MAX-5);});
    bad([&]{put(cache+0x30,UINT64_MAX-5);});
    bad([&]{put(matrix,UINT64_MAX-5);});
    bad([&]{put(matrix+12,uint32_t(1));});
    bad([&]{put(entries+32,UINT64_MAX-5);});
    bad([&]{put(entries+44,uint32_t(4));});
    bad([&]{put(entries+56,int64_t(-1));});
    bad([&]{put(ranges+4,std::array<uint16_t,2>{30,0});});
    bad([&]{put(ranges,uint16_t(31));});
    bad([&]{
      put(index+36,uint8_t(0));put(index+60,uint8_t(2));put(index+64,uint16_t(30));put(index+68,uint32_t(2));
    }); // the key exists elsewhere, but the original probe sequence cannot find it
    bytes=stable;put(entries+56,int64_t(0));Check(copy() && copy()->distanceFixed==0,"nonempty original cached distance may be zero");bytes=stable;
    unsigned seen=0;GoodsMemoryReader changing=[&](uint64_t at,void* p,size_t n){
      if(at==entries+32 && ++seen==2)put(entries+56,int64_t(7));return read(at,p,n);
    };
    Check(!DecodeNativeSeaCacheRoute(changing,module,10,30,[&]{return live;}),"cache entry mutation during copy rejected");bytes=stable;seen=0;
    changing=[&](uint64_t at,void* p,size_t n){if(at==index && ++seen==2)put(index+8,uint32_t(0));return read(at,p,n);};
    Check(!DecodeNativeSeaCacheRoute(changing,module,10,30,[&]{return live;}),"cache index mutation during copy rejected");bytes=stable;
    unsigned observations=0;Check(!DecodeNativeSeaCacheRoute(read,module,10,30,[&]{return ++observations==1;}),"retirement before cache publication rejected");
    live=false;Check(!copy(),"retired sea cache scope rejected");live=true;
    Check(!DecodeNativeSeaCacheRoute(read,module,0,30,[&]{return live;}) &&
        !DecodeNativeSeaCacheRoute(read,module,10,10,[&]{return live;}) &&
        !DecodeNativeSeaCacheRoute(read,UINT64_MAX,10,30,[&]{return live;}),"unsupported or overflowing cache request rejected");
    bytes.clear();Check(output->provinces[1].last==16 && output->distanceFixed==0x123456789ab,"cached path owns all values after native source destruction");bytes=original;
  }
  {
    constexpr uint64_t source=0x90000,target=0x90100,data=0x91000;
    for(uint64_t at:{source,target})put(at,std::array<uint8_t,24>{});
    put(source,NativeStraitOutputHeader{data,4,4});
    put(data,std::array<uint32_t,4>{0xab000001,0,0xab000001,0xcd000002});
    put(target,NativeStraitOutputHeader{data+0x20,1,1});put(data+0x20,uint32_t(0xef000003));
    auto copy=[&]{return CopyNativePortSearchInputs(read,source,target);};
    const auto stable=bytes;auto input=copy();
    Check(input && input->sources==std::vector<uint32_t>({0xab000001,0,0xab000001,0xcd000002}) &&
        input->targets==std::vector<uint32_t>({0xef000003}),"port request preserves full state handles, zero, order and duplicates");
    for(const auto& [at,value]:stable)if(!original.count(at)){
      bytes.erase(at);Check(!copy(),"every unreadable port request byte rejected");bytes=stable;
    }
    for(uint64_t header:{source,target}){
      for(auto bounds:{std::pair{0u,1u},std::pair{65537u,1u},std::pair{4097u,4097u},std::pair{1u,0u}}){
        put(header+8,bounds.first);put(header+12,bounds.second);Check(!copy(),"port request bounds and empty invocation rejected");bytes=stable;
      }
      put(header,UINT64_MAX-1);Check(!copy(),"port request pointer overflow rejected");bytes=stable;
    }
    put(data,UINT32_MAX);Check(!copy(),"unknown full state cannot enter port candidates");bytes=stable;
    unsigned seen=0;GoodsMemoryReader changing=[&](uint64_t at,void* p,size_t n){
      if(at==data && ++seen==2)put(data,uint32_t(7));return read(at,p,n);
    };
    Check(!CopyNativePortSearchInputs(changing,source,target),"port candidate change during copy rejected");bytes=stable;
    Check(!CopyNativePortSearchInputs({},source,target) && !CopyNativePortSearchInputs(read,UINT64_MAX,target),
        "missing or overflowing port request rejected");
    bytes=original;Check(input->sources.back()==0xcd000002,"port request owns states after source destruction");
    constexpr uint64_t result=0x92000,from=0x92100,to=0x92102,fromState=0x92104,toState=0x92108,distance=0x92110,ranges=0x93000;
    put(result,std::array<uint8_t,24>{});put(result,NativeStraitOutputHeader{ranges,3,3});
    put(ranges,std::array<uint16_t,6>{20,0,15,1,10,0});put(from,uint16_t(20));put(to,uint16_t(10));
    put(fromState,uint32_t(0xcd000002));put(toState,uint32_t(0xef000003));put(distance,int64_t(0x123456789ab));
    NativeTradeSearchResult nested{20,10,0x123456789ab,{{20,20},{15,16},{10,10}}};
    auto select=[&]{return CopyNativePortSearchSelection(read,result,from,to,fromState,toState,distance,*input,nested);};
    const auto selectedStable=bytes;auto output=select();
    Check(output && output->fromState==0xcd000002 && output->toState==0xef000003 && output->path==nested,
        "port selection preserves original winning full states rather than first equal-province candidates");
    for(const auto& [at,value]:selectedStable)if(!original.count(at)){
      bytes.erase(at);Check(!select(),"every unreadable final port output byte rejected");bytes=selectedStable;
    }
    for(auto address:{fromState,toState}){
      put(address,UINT32_MAX);Check(!select(),"reachable path cannot have unknown selected state");bytes=selectedStable;
      put(address,uint32_t(1));Check(!select(),"selected state must belong to its exact full candidate group");bytes=selectedStable;
    }
    put(from,uint16_t(10));Check(!select(),"port result must match its nested original province direction");bytes=selectedStable;
    put(distance,int64_t(1));Check(!select(),"port result cannot borrow equal-geometry query with different distance");bytes=selectedStable;
    put(ranges+4,std::array<uint16_t,2>{20,0});Check(!select(),"port output overlapping membership rejected");bytes=selectedStable;
    put(result+12,uint32_t(4));Check(!select(),"port output count cannot exceed capacity");bytes=selectedStable;
    put(result,UINT64_MAX-1);Check(!select(),"port output array address overflow rejected");bytes=selectedStable;
    seen=0;changing=[&](uint64_t at,void* p,size_t n){if(at==fromState && ++seen==2)put(at,uint32_t(0));return read(at,p,n);};
    Check(!CopyNativePortSearchSelection(changing,result,from,to,fromState,toState,distance,*input,nested),
        "selected full state change during output copy rejected");bytes=selectedStable;seen=0;
    changing=[&](uint64_t at,void* p,size_t n){if(at==ranges && ++seen==2)put(at,uint16_t(21));return read(at,p,n);};
    Check(!CopyNativePortSearchSelection(changing,result,from,to,fromState,toState,distance,*input,nested),
        "selected path change during output copy rejected");bytes=selectedStable;
    put(fromState,uint32_t(0));put(distance,int64_t(0));nested.distance=0;
    Check(select()->fromState==0,"full state zero and native nonempty zero distance remain known");
    put(result,NativeStraitOutputHeader{});put(from,UINT16_MAX);put(to,UINT16_MAX);
    put(fromState,UINT32_MAX);put(toState,UINT32_MAX);nested={};
    Check(select() && select()->path.provinces.empty(),"witnessed failed port search preserves unknown endpoints and known empty path");
    put(fromState,uint32_t(0));Check(!select(),"failed query cannot retain a selected full state");
    bytes.clear();Check(output->fromState==0xcd000002 && output->path.provinces[1].last==16,
        "final port selection owns all values after original output lifetime ends");bytes=original;
  }
  {
    constexpr uint64_t source=0x90000,target=0x90100,options=0x90200,data=0x91000;
    for(uint64_t at:{source,target,options,options+0x18}){std::array<uint8_t,24> header{};put(at,header);}
    put(source,NativeStraitOutputHeader{data,2,2});put(data,std::array<uint16_t,2>{20,10});
    put(target,NativeStraitOutputHeader{data+0x20,1,1});put(data+0x20,uint16_t(30));
    put(options,NativeStraitOutputHeader{data+0x40,3,3});put(data+0x40,std::array<uint32_t,3>{0xf0000003,0xf0000003,UINT32_MAX});
    put(options+0x18,NativeStraitOutputHeader{data+0x60,2,2});put(data+0x60,std::array<uint16_t,4>{1,2,0,UINT16_MAX});put(options+0x30,uint16_t(77));
    const auto stable=bytes;auto copy=[&]{return CopyNativeTradeSearchInputs(read,source,target,options);};
    auto input=copy();Check(input && input->sources==std::vector<uint16_t>({20,10}) && input->targets==std::vector<uint16_t>({30}) &&
        input->forcedStraits==std::vector<uint32_t>({0xf0000003,0xf0000003,UINT32_MAX}) &&
        input->blockedPairs==std::vector<std::array<uint16_t,2>>({{1,2},{0,UINT16_MAX}}) && input->selector==77,
        "trade request preserves candidate order and all uninterpreted option restrictions");
    bytes.clear();Check(input->sources[0]==20 && input->blockedPairs[1][1]==UINT16_MAX,"trade request owns inputs after source destruction");bytes=stable;
    for(const auto& [at,value]:stable)if(!original.count(at)){
      bytes.erase(at);Check(!copy(),"every unreadable trade request byte rejected");bytes=stable;
    }
    for(uint64_t header:{source,target,options,options+0x18})for(auto bounds:{std::pair{0u,1u},std::pair{65537u,1u},std::pair{4097u,4097u}}){
      put(header+8,bounds.first);put(header+12,bounds.second);Check(!copy(),"each trade request array bounds checked");bytes=stable;
    }
    for(uint64_t header:{source,target,options,options+0x18}){
      put(header,UINT64_MAX-1);Check(!copy(),"each trade request array pointer overflow rejected");bytes=stable;
    }
    for(uint64_t header:{source,target}){
      put(header+12,uint32_t(0));Check(!copy(),"unknown empty endpoint request rejected");bytes=stable;
    }
    put(data,uint16_t(0));Check(!copy(),"unknown source province rejected");bytes=stable;
    put(data+0x20,uint16_t(UINT16_MAX));Check(!copy(),"unknown destination province rejected");bytes=stable;
    unsigned seen=0;GoodsMemoryReader changing=[&](uint64_t at,void* p,size_t n){
      if(at==options+0x30 && ++seen==2)put(options+0x30,uint16_t(88));return read(at,p,n);
    };
    Check(!CopyNativeTradeSearchInputs(changing,source,target,options),"trade option selector mutation during copy rejected");bytes=stable;seen=0;
    changing=[&](uint64_t at,void* p,size_t n){if(at==data+0x40 && ++seen==2)put(at,uint32_t(9));return read(at,p,n);};
    Check(!CopyNativeTradeSearchInputs(changing,source,target,options),"trade option handles mutation during copy rejected");bytes=stable;
    Check(!CopyNativeTradeSearchInputs({},source,target,options) && !CopyNativeTradeSearchInputs(read,UINT64_MAX,target,options) &&
          !CopyNativeTradeSearchInputs(read,source,target,UINT64_MAX),"missing or overflowing trade request rejected");
    bytes=original;
  }
  {
    constexpr uint64_t result=0x90000,data=0x91000;
    std::array<uint8_t,0x28> row{};put(result,row);put(result,NativeStraitOutputHeader{data,4,3});
    put(result+0x18,uint16_t(20));put(result+0x1a,uint16_t(10));put(result+0x20,int64_t(0x123456789ab));
    put(data,std::array<uint16_t,2>{20,0});put(data+4,std::array<uint16_t,2>{15,1});put(data+8,std::array<uint16_t,2>{10,0});
    auto copy=[&]{return CopyNativeTradeSearchResult(read,result,{20},{10});};
    const auto stable=bytes;auto output=copy();
    Check(output && output->fromProvince==20 && output->toProvince==10 && output->distance==0x123456789ab &&
          output->provinces==std::vector<NativeProvinceRange>({{20,20},{15,16},{10,10}}),
          "trade search preserves selected direction, native range order and distance64");
    bytes.clear();Check(output->provinces[1].last==16,"trade search owns ranges after native source destruction");bytes=stable;
    Check(!CopyNativeTradeSearchResult(read,result,{10},{20}),"trade search cannot consume another endpoint request");
    for(uint64_t at:{result,result+0x18,result+0x1a,result+0x20,data,data+8}){
      bytes.erase(at);Check(!copy(),"every unreadable trade search field rejected");bytes=stable;
    }
    for(auto h:{NativeStraitOutputHeader{data,2,3},NativeStraitOutputHeader{data,65537,3},
                NativeStraitOutputHeader{data,4097,4097},NativeStraitOutputHeader{0,4,3},NativeStraitOutputHeader{UINT64_MAX-4,4,3}}){
      put(result,h);Check(!copy(),"trade search bounds, missing buffer and overflow rejected");bytes=stable;
    }
    for(auto r:{std::array<uint16_t,2>{0,0},std::array<uint16_t,2>{65534,1},std::array<uint16_t,2>{10,0}}){
      put(data,r);Check(!copy(),"trade search illegal, overflowing or duplicate province ranges rejected");bytes=stable;
    }
    put(result+0x18,uint16_t(99));Check(!copy(),"selected trade endpoint must be in returned membership");bytes=stable;
    put(result+0x20,int64_t(-1));Check(!copy(),"negative trade search distance rejected");bytes=stable;
    unsigned seen=0;GoodsMemoryReader changing=[&](uint64_t at,void* p,size_t n){
      if(at==result && ++seen==2)put(result+0x18,uint16_t(10));return read(at,p,n);
    };
    Check(!CopyNativeTradeSearchResult(changing,result,{20},{10}),"trade search metadata changes during copy rejected");bytes=stable;seen=0;
    changing=[&](uint64_t at,void* p,size_t n){
      if(at==data && ++seen==2)put(data,uint16_t(21));return read(at,p,n);
    };
    Check(!CopyNativeTradeSearchResult(changing,result,{20},{10}),"trade search membership changes during copy rejected");bytes=stable;
    put(result,NativeStraitOutputHeader{});put(result+0x18,uint16_t(UINT16_MAX));put(result+0x1a,uint16_t(UINT16_MAX));put(result+0x20,int64_t(0));
    auto unavailable=copy();Check(unavailable && unavailable->provinces.empty(),"original unreachable trade result is known empty, distinct from copy failure");
    put(result+0x18,uint16_t(20));Check(!copy(),"partly selected empty trade result rejected");
    bytes=stable;put(result+0x20,int64_t(0));Check(copy().has_value(),"native trade zero distance is not confused with unreachable");
    put(result,NativeStraitOutputHeader{data,1,1});put(data,std::array<uint16_t,2>{20,0});put(result+0x1a,uint16_t(20));
    auto same=CopyNativeTradeSearchResult(read,result,{20},{20});Check(same && same->provinces.size()==1 && same->distance==0,"same-province native search keeps known zero-distance path");
    bytes=stable;
    Check(!CopyNativeTradeSearchResult({},result,{20},{10}) && !CopyNativeTradeSearchResult(read,UINT64_MAX,{20},{10}) &&
          !CopyNativeTradeSearchResult(read,result,{}, {10}) && !CopyNativeTradeSearchResult(read,result,{0},{10}) &&
          !CopyNativeTradeSearchResult(read,result,{20},{UINT16_MAX}),"trade search missing reader, overflowing result or unknown request rejected");
    bytes=original;
  }
  {
    constexpr uint64_t finder=0xa0000,request=0x90000,db=0xb0000,country=0xc0000,entries=0xd0000;
    put(finder,module+0x044a0438);put(finder+0x168,uint32_t(1));put(finder+0x180,country);put(finder+0x190,uint32_t(1));
    put(country,module+0x04494cb8);put(country+0x18,uint32_t(0xf0000012));
    put(module+0x059667f0,db);put(db,module+0x04439800);put(db+0x20,entries);put(db+0x2c,uint32_t(2));
    const std::array<NativePathEndpoint,2> endpoints{{{0,{10,20}},{0xe0000001,{30,40}}}};
    for(size_t i=0;i<2;++i){
      const uint64_t node=0xe0000+i*0x100,definition=0xf0000+i*0x100;
      put(request+0x18+i*0x20,endpoints[i].node);put(entries+8+i*16,node);
      put(node,module+0x044e3d78);put(node+8,endpoints[i].node);put(node+0x78,definition);
      put(definition,module+0x044f8a78);put(definition+0x48,endpoints[i].position);
    }
    const auto stable=bytes;
    auto observe=[&]{return ObserveNativeFleetPathRequest(read,module,finder,request);};
    auto copied=observe();
    Check(copied && copied->country==0xf0000012 && copied->traversal==1 && copied->from==endpoints[0] &&
          copied->to==endpoints[1],"fleet request copies original complete country/node IDs and exact definition positions");
    bytes.clear();Check(copied->to==endpoints[1],"fleet request owns values after native source destruction");bytes=stable;
    for(uint64_t at:{finder,finder+0x168,country,db,0xe0000ull,0xf0000ull}){
      put(at,uint32_t(0));Check(!observe(),"wrong finder, country, database, node or definition rejected");bytes=stable;
    }
    put(request+0x38,uint32_t(0xf0000001));Check(!observe(),"recycled node slot cannot replace complete requested handle");bytes=stable;
    put(request+0x38,UINT32_MAX);Check(!observe(),"coordinate fallback is not guessed as a native node");bytes=stable;
    put(db+0x2c,uint32_t(1));Check(!observe(),"node outside current roster rejected");bytes=stable;
    put(db+0x2c,uint32_t(131073));Check(!observe(),"excessive node roster rejected");bytes=stable;
    put(db+0x20,UINT64_MAX-8);Check(!observe(),"node roster address overflow rejected");bytes=stable;
    for(const auto& [at,value]:stable)if(!original.count(at)){
      bytes.erase(at);Check(!observe(),"every unreadable fleet request field rejected");bytes=stable;
    }
    unsigned seen=0;
    GoodsMemoryReader changing=[&](uint64_t at,void* p,size_t n){
      if(at==request+0x38 && ++seen==2)put(request+0x38,uint32_t(0xf0000001));return read(at,p,n);
    };
    Check(!ObserveNativeFleetPathRequest(changing,module,finder,request),"fleet request mutation during copy rejected");bytes=stable;
    Check(!ObserveNativeFleetPathRequest({},module,finder,request) &&
          !ObserveNativeFleetPathRequest(read,UINT64_MAX,finder,request) &&
          !ObserveNativeFleetPathRequest(read,module,UINT64_MAX,request) &&
          !ObserveNativeFleetPathRequest(read,module,finder,UINT64_MAX),"missing or overflowing fleet request input rejected");
    bytes=original;
  }
  {
    constexpr uint64_t result=0x90000,data=0x91000;
    const NativePathEndpoint first{0,{10,20}},middle{0xe0000001,{30,40}},last{0xf0000002,{50,60}};
    std::array<uint8_t,0x68> header{};put(result,header);
    put(result+0x40,data);put(result+0x48,uint32_t(2));put(result+0x4c,uint32_t(2));
    auto row=[&](uint64_t at,uint32_t connection,const auto& from,const auto& to,int64_t distance,int64_t cost){
      std::array<uint8_t,64> bytes{};put(at,bytes);put(at,connection);put(at+4,from.node);put(at+8,from.position);
      put(at+0x18,to.node);put(at+0x20,to.position);put(at+0x30,distance);put(at+0x38,cost);
    };
    const int64_t distance=0x123456789ab,cost=1ll<<59;
    row(data,0xf1000000,first,middle,0,17);row(data+64,0xf1000001,middle,last,distance,cost);
    put(result+0x58,distance);put(result+0x60,cost+17);
    const auto stable=bytes;std::string why;
    auto copy=[&]{return CopyNativePathResult(read,result,first,last,&why);};
    auto output=copy();
    Check(output && why.empty() && output->distance==distance && output->cost==cost+17 &&
          output->steps.size()==2 && output->steps[0].connection==0xf1000000 && output->steps[0].from==first &&
          output->steps[1].to==last,"original path records retain order, complete IDs, zero distance and independent cost64");
    bytes.clear();Check(output->steps[1].distance==distance && output->steps[1].from==middle,"path copy owns numeric values after source destruction");bytes=stable;
    Check(!CopyNativePathResult(read,result,last,first),"reverse request cannot consume forward original path");
    for(auto at:{result+0x40,result+0x48,result+0x4c,result+0x58,result+0x60,data,data+64}){
      bytes.erase(at);Check(!copy(),"unreadable native path fields rejected");bytes=stable;
    }
    for(auto [capacity,count]:{std::pair{1u,2u},std::pair{65537u,2u},std::pair{4097u,4097u}}){
      put(result+0x48,capacity);put(result+0x4c,count);Check(!copy(),"native path array overflow or excessive count rejected");bytes=stable;
    }
    for(uint64_t pointer:{uint64_t(0),UINT64_MAX-63}){
      put(result+0x40,pointer);Check(!copy(),"missing or overflowing native path storage rejected");bytes=stable;
    }
    put(data,UINT32_MAX);Check(!copy() && why=="unsupported native custom transition","custom transition never masquerades as physical connection");bytes=stable;
    for(auto at:{data+4,data+0x18,data+64+4,data+64+0x18}){
      put(at,uint32_t(77));Check(!copy(),"full native path endpoints and adjacency checked");bytes=stable;
    }
    put(data+64+8,int64_t(31));Check(!copy(),"same node with different coordinate cannot bridge a gap");bytes=stable;
    for(auto at:{result+0x58,result+0x60,data+0x30,data+0x38}){
      put(at,int64_t(-1));Check(!copy(),"negative native distance or cost rejected");bytes=stable;
    }
    put(result+0x60,cost+18);Check(!copy(),"native path totals must equal copied steps");bytes=stable;
    for(auto at:{data+0x30,data+0x38}){
      put(at,INT64_MAX);Check(!copy(),"native path sum overflow rejected");bytes=stable;
    }
    unsigned seen=0;
    GoodsMemoryReader changing=[&](uint64_t at,void* p,size_t n){
      if(at==result+0x40 && ++seen==2)put(result+0x4c,uint32_t(1));return read(at,p,n);
    };
    Check(!CopyNativePathResult(changing,result,first,last),"native result descriptor mutation during copy rejected");bytes=stable;seen=0;
    changing=[&](uint64_t at,void* p,size_t n){
      if(at==data && ++seen==2)put(data,uint32_t(8));return read(at,p,n);
    };
    Check(!CopyNativePathResult(changing,result,first,last),"native result content mutation during copy rejected");bytes=stable;
    put(result+0x4c,uint32_t(0));put(result+0x58,int64_t(0));put(result+0x60,int64_t(0));
    auto empty=CopyNativePathResult(read,result,first,first);
    Check(empty && empty->steps.empty() && empty->distance==0 && !copy(),"empty native route requires same exact physical endpoint");bytes=stable;
    Check(!CopyNativePathResult({},result,first,last) && !CopyNativePathResult(read,UINT64_MAX,first,last) &&
          !CopyNativePathResult(read,0,first,last) && !CopyNativePathResult(read,result,{},last),"missing native path reader, result or full endpoint rejected");
    bytes=original;
  }
  {
    constexpr uint64_t allowed=0x80000,denied=0x80100,aData=0x81000,dData=0x82000;
    put(allowed,NativeStraitOutputHeader{aData,8,3});put(denied,NativeStraitOutputHeader{dData,8,2});
    put(aData,UINT32_MAX);put(aData+4,uint32_t(0));put(aData+8,uint32_t(0xf0000012));
    put(dData,UINT32_MAX);put(dData+4,uint32_t(0x80000007));
    const auto stable=bytes;
    auto copy=[&]{return CopyNativeStraitClassification(read,allowed,denied,1,1);};
    auto result=copy();
    Check(result && result->allowed==std::vector<uint32_t>({0,0xf0000012}) &&
          result->denied==std::vector<uint32_t>({0x80000007}),
          "native classification copies full handles in order and excludes existing prefixes");
    Check(bytes==stable,"borrowed classification inputs remain unchanged");
    bytes.clear();Check(result->allowed[1]==0xf0000012,"owned classification survives source destruction");bytes=stable;
    for(auto header:{allowed,denied}){
      put(header,NativeStraitOutputHeader{0,8,2});Check(!copy(),"missing populated classification buffer rejected");bytes=stable;
      put(header+8,uint32_t(0));Check(!copy(),"count above capacity rejected");bytes=stable;
      put(header+8,uint32_t(65537));Check(!copy(),"excessive classification capacity rejected");bytes=stable;
      put(header+12,uint32_t(4097));Check(!copy(),"excessive classification count rejected");bytes=stable;
      put(header,NativeStraitOutputHeader{UINT64_MAX-4,8,2});Check(!copy(),"classification pointer overflow rejected");bytes=stable;
      put(header+12,uint32_t(0));Check(!copy(),"shrinking below pre-call prefix rejected");bytes=stable;
    }
    put(aData+8,uint32_t(0));Check(!copy(),"duplicate full handle within allowed output rejected");bytes=stable;
    put(dData+4,uint32_t(0xf0000012));Check(!copy(),"handle present in both allowed and denied rejected");bytes=stable;
    put(dData+4,UINT32_MAX);Check(!copy(),"invalid native strait handle rejected");bytes=stable;
    unsigned seen=0;
    GoodsMemoryReader changed=[&](uint64_t at,void* out,size_t n){
      if(at==allowed && ++seen==2)put(allowed+12,uint32_t(2));return read(at,out,n);
    };
    Check(!CopyNativeStraitClassification(changed,allowed,denied,1,1),"descriptor mutation during output copy rejected");bytes=stable;seen=0;
    changed=[&](uint64_t at,void* out,size_t n){
      if(at==aData+4 && ++seen==2)put(aData+8,uint32_t(99));return read(at,out,n);
    };
    Check(!CopyNativeStraitClassification(changed,allowed,denied,1,1),"full handle mutation during output copy rejected");bytes=stable;
    bytes.erase(dData+4);Check(!copy(),"unreadable classification entry rejected");bytes=stable;
    put(allowed,NativeStraitOutputHeader{});put(denied,NativeStraitOutputHeader{dData,8,1});
    put(dData,uint32_t(0x80000007));
    result=CopyNativeStraitClassification(read,allowed,denied,0,0);
    Check(result && result->allowed.empty() && result->denied==std::vector<uint32_t>({0x80000007}),
          "known denied result remains distinct from unavailable classification");
    put(denied,NativeStraitOutputHeader{});
    result=CopyNativeStraitClassification(read,allowed,denied,0,0);
    Check(result && result->allowed.empty() && result->denied.empty(),"valid empty classifier outputs copied");
    Check(!ReadNativeStraitOutputHeader(read,UINT64_MAX) && !ReadNativeStraitOutputHeader({},allowed),
          "missing reader and descriptor overflow rejected");
  }
  {
    constexpr uint64_t c=0x90000,f=0x90100,t=0x90200,p=0x90300,d=0x90400,ranges=0x91000;
    put(c,uint32_t(0xe0000000));put(f,uint32_t(0));put(t,uint32_t(0xa0000077));
    put(d,int64_t(0x123456789ab));put(p,NativeStraitOutputHeader{ranges,4,2});
    put(ranges,std::array<uint16_t,2>{3000,3});put(ranges+4,std::array<uint16_t,2>{4000,0});
    const auto stable=bytes;
    auto copy=[&]{return CopyNativeLaneSelection(read,c,f,t,p,d);};
    auto selected=copy();
    Check(selected && selected->country==0xe0000000 && selected->fromState==0 && selected->toState==0xa0000077 &&
          selected->distance==0x123456789ab && selected->provinces==std::vector<NativeProvinceRange>({{3000,3003},{4000,4000}}),
          "selected native lane preserves complete state handles, endpoint order, compressed membership and 64-bit distance");
    Check(bytes==stable,"selected lane input is not changed");
    bytes.clear();Check(selected->provinces[1].first==4000,"selected lane survives source destruction");bytes=stable;
    for(auto address:{c,f,t}){put(address,UINT32_MAX);Check(!copy(),"invalid selected identity rejected");bytes=stable;}
    put(t,uint32_t(0));Check(!copy(),"identical selected endpoints rejected");bytes=stable;
    put(d,int64_t(-1));Check(!copy(),"negative selected distance rejected");bytes=stable;
    put(d,int64_t(0));Check(bool(copy()),"known zero distance is preserved");bytes=stable;
    for(auto h:{NativeStraitOutputHeader{ranges,1,2},NativeStraitOutputHeader{ranges,65535,2},
                NativeStraitOutputHeader{ranges,4,0},NativeStraitOutputHeader{UINT64_MAX-4,4,2}}){
      put(p,h);Check(!copy(),"invalid selected province set bounds rejected");bytes=stable;
    }
    for(auto value:{std::array<uint16_t,2>{0,0},std::array<uint16_t,2>{3002,0},std::array<uint16_t,2>{65534,1}}){
      put(ranges+4,value);Check(!copy(),"unordered, overlapping or overflowing selected province range rejected");bytes=stable;
    }
    unsigned seen=0;
    GoodsMemoryReader changing=[&](uint64_t at,void* out,size_t n){
      if(at==d && ++seen==2)put(d,int64_t(1));return read(at,out,n);
    };
    Check(!CopyNativeLaneSelection(changing,c,f,t,p,d),"selected distance mutation rejected");bytes=stable;seen=0;
    changing=[&](uint64_t at,void* out,size_t n){
      if(at==ranges && ++seen==2)put(ranges+4,std::array<uint16_t,2>{4001,0});return read(at,out,n);
    };
    Check(!CopyNativeLaneSelection(changing,c,f,t,p,d),"selected membership mutation rejected");bytes=stable;
    bytes.erase(t);Check(!copy(),"unreadable selected endpoint rejected");bytes=stable;
    Check(!CopyNativeLaneSelection({},c,f,t,p,d) && !CopyNativeLaneSelection(read,c,f,t,UINT64_MAX,d),
          "missing reader and selected descriptor address overflow rejected");
  }
  {
    const NativeLaneAccessScope scope{2,31,7};
    NativeShippingLane first;first.id=0xf0000001;first.country=0xa0000005;
    first.fromState=0;first.toState=0xb0000008;first.travelDistanceFixed=0x123456789ab;
    first.provinces={{10,12},{20,20}};
    auto second=first;second.id=0xe0000002;second.toState=0xc0000009;
    NativeShippingSnapshot shipping{{first,second}};
    const std::vector<uint32_t> straits{0xf0000003,0};
    auto record=[](const NativeShippingLane& lane,NativeStraitClassification access){
      return NativeLaneAccessRecord{lane.id,{lane.country,lane.fromState,lane.toState,
          lane.travelDistanceFixed,lane.provinces},std::move(access)};
    };
    NativeLaneAccessBatch batch{scope,{record(first,{{0xf0000003,0},{}}),record(second,{{},{0,0xf0000003}})}};
    std::string why;
    auto bind=[&]{return BindNativeLaneAccess(batch,scope,shipping,straits,&why);};
    const auto stableBatch=batch;const auto stableShipping=shipping;
    auto bound=bind();
    Check(bound && bound->at(first.id)==std::vector<uint32_t>({0,0xf0000003}) && bound->at(second.id).empty() && why.empty(),
          "same-country lanes retain separate original classifications and sorted complete handles");
    batch.records.clear();shipping.lanes.clear();
    Check(bound->at(first.id).size()==2 && bound->at(second.id).empty(),"bound access owns values after source destruction");
    batch=stableBatch;shipping=stableShipping;
    for(const auto stale:{NativeLaneAccessScope{1,31,7},NativeLaneAccessScope{2,30,7},NativeLaneAccessScope{2,31,6},
                         NativeLaneAccessScope{0,31,7},NativeLaneAccessScope{2,0,7},NativeLaneAccessScope{2,31,0}}){
      Check(!BindNativeLaneAccess(batch,stale,shipping,straits),"world, revision and collection generation must match independently");
    }
    for(unsigned field=0;field<6;++field){
      auto& lane=shipping.lanes[0];
      if(field==0)lane.id=0xd0000001;
      if(field==1)++lane.country;
      if(field==2)std::swap(lane.fromState,lane.toState);
      if(field==3)++lane.toState;
      if(field==4)++lane.travelDistanceFixed;
      if(field==5)++lane.provinces[1].last;
      Check(!bind() && !why.empty(),"changed full lane handle, country, endpoints, distance or membership rejects old record");
      shipping=stableShipping;
    }
    batch.records.pop_back();Check(!bind(),"missing lane record cannot borrow same-country access");batch=stableBatch;
    batch.records[1]=batch.records[0];Check(!bind(),"duplicate lane record rejected");batch=stableBatch;
    batch.records[0].lane=123;Check(!bind(),"unknown lane record rejected");batch=stableBatch;
    shipping.lanes[1].id=first.id;Check(!bind(),"duplicate copied lane handle rejected");shipping=stableShipping;
    for(const auto invalid:{NativeStraitClassification{{0},{}},NativeStraitClassification{{0},{0}},
                            NativeStraitClassification{{0,0},{}},NativeStraitClassification{{0,0xe0000003},{}},
                            NativeStraitClassification{{UINT32_MAX,0},{}}}){
      batch.records[0].access=invalid;Check(!bind(),"partial, overlapping, duplicate or stale strait classification rejected");
    }
    batch=stableBatch;
    Check(!BindNativeLaneAccess(batch,scope,shipping,{0,0}) &&
          !BindNativeLaneAccess(batch,scope,shipping,{0,UINT32_MAX}),"invalid current strait roster rejected");
    std::reverse(batch.records.begin(),batch.records.end());std::reverse(shipping.lanes.begin(),shipping.lanes.end());
    Check(bind()==bound,"enumeration order does not change bound access");batch=stableBatch;shipping=stableShipping;
    batch.records[0].selected.distance=0;shipping.lanes[0].travelDistanceFixed=0;
    Check(bool(bind()),"known zero native distance accepted");batch=stableBatch;shipping=stableShipping;
    batch.records[0].selected.provinces={{0,12}};shipping.lanes[0].provinces={{0,12}};
    Check(!bind(),"matching malformed province sets still rejected");batch=stableBatch;shipping=stableShipping;
    shipping.lanes[0].provinces.clear();batch.records.erase(batch.records.begin());
    Check(bool(bind()),"known empty original path needs no passage record");
    auto international=first;international.kind=1;international.id=77;shipping.lanes.push_back(international);
    Check(bool(bind()),"international treaty lane does not consume domestic classification");
    NativeLaneAccessBatch empty{scope,{}};
    Check(BindNativeLaneAccess(empty,scope,{},{}).has_value(),"known empty current roster remains distinct from missing batch");
  }
  std::puts("PASS: native access observations, owned copies and lane access binding (not an access lease)");
}
