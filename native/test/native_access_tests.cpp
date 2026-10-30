#include "native_access.h"
#include "native_strait_probe.h"
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
  put(tlsArray,tls);put(tls+0x5ae0,uint8_t(1));put(tls+0x92c,uint32_t(0));
  put(module+0x0514e580,uint32_t(2));put(module+0x0514e570,uint32_t(0));
  put(module+0x0592d0e0,idler);put(idler,module+0x04431500);put(idler+0x20,uint8_t(1));
  put(module+0x0592d128,root);put(root,module+0x04430800);put(root+0x588,uint32_t(4));put(root+0x608,machine);
  put(machine+8,uint32_t(59883360));put(machine+0x120,world);put(module+0x05964ef0,db);
  auto decode=[&]{return ObserveNativeAccess(read,module,tlsArray);};
  auto original=bytes;auto s=decode();
  Check(s && s->access==2 && s->owns==1 && s->context==0 && s->date==59883360 &&
        s->machine==machine && s->world==world && s->stateDb==db,"copies native access and world identities");
  for(uint32_t access=0;access<=3;++access){put(module+0x0514e580,access);Check(bool(decode()),"all native modes observed without granting permission");}
  put(module+0x0514e580,uint32_t(4));Check(!decode(),"unknown access rejected");bytes=original;
  put(module+0x0514e570,uint32_t(2));Check(!decode(),"unknown hidden mode rejected");bytes=original;
  put(tls+0x5ae0,uint8_t(2));Check(!decode(),"invalid thread ownership rejected");bytes=original;
  put(idler+0x20,uint8_t(2));Check(!decode(),"invalid paused state rejected");bytes=original;
  put(root,module+0x04430808);Check(!decode(),"wrong application type rejected");bytes=original;
  put(machine+0x120,uint64_t(0));Check(!decode(),"missing world rejected");bytes=original;
  put(module+0x05964ef0,uint64_t(0));Check(!decode(),"missing state database rejected");bytes=original;
  for(const auto& [address,value]:original){bytes=original;bytes.erase(address);Check(!decode(),"every unreadable field rejected");}
  bytes=original;put(tlsArray,UINT64_MAX);Check(!decode(),"TLS address overflow rejected");
  Check(!ObserveNativeAccess(read,UINT64_MAX,tlsArray),"module overflow rejected");
  Check(!ObserveNativeAccess({},module,tlsArray) && !ObserveNativeAccess(read,module,0),"missing reader or TLS rejected");
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
  std::puts("PASS: native access observations and owned classifier/selected-lane copies (not an access lease)");
}
