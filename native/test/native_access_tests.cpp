#include "native_access.h"
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
  std::puts("PASS: native access observations (not an access lease)");
}
