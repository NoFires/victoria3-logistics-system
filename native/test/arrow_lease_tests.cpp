#include "arrow_lease.h"
#include <cstdio>
#include <cstdlib>

void require(bool value,const char* message){if(!value){std::printf("FAIL: %s\n",message);std::exit(1);}}
struct Memory {
  std::map<uint64_t,uint8_t> bytes;
  bool current=true,mutate=false;int graphicsReads=0;
  template<class T>void Put(uint64_t address,T value){
    auto* p=(uint8_t*)&value;for(size_t i=0;i<sizeof(value);++i)bytes[address+i]=p[i];
  }
  void Zero(uint64_t address,size_t size){for(size_t i=0;i<size;++i)bytes[address+i]=0;}
  bool Read(uint64_t address,void* out,size_t size){
    for(size_t i=0;i<size;++i){auto it=bytes.find(address+i);if(it==bytes.end())return false;((uint8_t*)out)[i]=it->second;}
    if(address==0x10000000+0x0592d5c8 && ++graphicsReads==1 && mutate)Put<uint64_t>(address,0x9999);
    return true;
  }
};
int main(){
  using namespace v3;
  const uint64_t base=0x10000000,type=0x10000,arrow=0x20000,path=0x30000;
  NativeArrowLease lease{"ironroadBuildingInputW0",arrow,type,path,0x40000,0x50000,0x60000};
  Memory good;
  good.Put<uint64_t>(base+0x0592d5c8,0x70000);
  good.Put<uint8_t>(base+0x0513f2b8,1);
  good.Put<uint64_t>(base+0x0513f268+0x30,0x80000);
  good.Put<uint32_t>(base+0x0513f268+0x3c,1);
  good.Put<uint8_t>(base+0x0513f268+0x40,0);
  good.Zero(0x80000,3*0x30);good.Put<uint8_t>(0x80004,1);
  good.Put<uint64_t>(0x80008,0x90000);good.Put<uint64_t>(0x80018,lease.style.size());
  good.Put<uint64_t>(0x80020,31);good.Put<uint64_t>(0x80028,type);
  for(size_t i=0;i<=lease.style.size();++i)good.Put<uint8_t>(0x90000+i,i==lease.style.size()?0:lease.style[i]);
  good.Put<uint64_t>(type,base+0x0481daa8);good.Put<uint64_t>(type+0x38,0xa0000);
  good.Put<uint32_t>(type+0x40,1);good.Put<uint32_t>(type+0x44,1);
  good.Put<uint64_t>(type+0x48,base+0x0512cdc0);good.Zero(0xa0000,0x48);good.Put<uint64_t>(0xa0000,arrow);
  good.Put<uint64_t>(arrow,base+0x046588e8);good.Put<uint64_t>(arrow+0x50,type);
  good.Put<uint64_t>(arrow+0x60,lease.constant);good.Put<uint64_t>(arrow+0x68,lease.vertex);
  good.Put<uint64_t>(arrow+0x70,path);good.Put<uint64_t>(path+0x10,lease.points);
  good.Put<uint32_t>(path+0x18,4096);good.Put<uint32_t>(path+0x1c,4);
  good.Put<uint64_t>(path+0x20,base+0x0512cdc0);
  auto check=[&](Memory& memory,const std::vector<NativeArrowLease>& leases){
    std::string error;
    return ValidateNativeArrowLeases([&](auto a,auto b,auto c){return memory.Read(a,b,c);},
      base,0x70000,leases,[&]{return memory.current;},&error);
  };
  require(check(good,{lease}),"retained pool with current asset registration recovers");
  auto test=[&](auto change,const char* message){auto memory=good;change(memory);require(!check(memory,{lease}),message);};
  test([&](auto& m){m.template Put<uint64_t>(0x80028,type+0x100);},"replaced asset type rejected");
  test([&](auto& m){m.template Put<uint8_t>(0x80004,0);},"removed asset rejected");
  test([&](auto& m){m.template Put<uint64_t>(0xa0000,arrow+8);},"unregistered object rejected");
  test([&](auto& m){m.template Put<uint64_t>(arrow,base+0x046588f0);},"wrong arrow type rejected");
  test([&](auto& m){m.template Put<uint64_t>(arrow+0x70,path+8);},"retained path replaced rejected");
  test([&](auto& m){m.template Put<uint64_t>(arrow+0x68,lease.vertex+8);},"replaced VB rejected");
  test([&](auto& m){m.template Put<uint64_t>(arrow+0x60,lease.constant+8);},"replaced CB rejected");
  test([&](auto& m){m.template Put<uint64_t>(path+0x10,lease.points+8);},"retained points replaced rejected");
  test([&](auto& m){m.template Put<uint32_t>(path+0x1c,4097);},"invalid path count rejected");
  test([&](auto& m){m.template Put<uint64_t>(type+0x48,base);},"unknown registry allocator rejected");
  test([&](auto& m){m.template Put<uint32_t>(type+0x44,8193);},"registration count bound enforced");
  test([&](auto& m){m.template Put<uint32_t>(base+0x0513f268+0x3c,65536);},"asset map bound enforced");
  test([&](auto& m){m.template Put<uint64_t>(base+0x0513f268+0x30,UINT64_MAX-16);},"asset entry pointer overflow rejected");
  test([&](auto& m){m.template Put<uint64_t>(base+0x0592d5c8,0x70001);},"changed graphics device rejected");
  test([&](auto& m){m.bytes.erase(path+0x10);},"unreadable retained state rejected");
  test([&](auto& m){m.mutate=true;m.graphicsReads=0;},"changes during recovery copy rejected");
  test([&](auto& m){m.current=false;},"retired request rejected");
  auto duplicate=good;require(!check(duplicate,{lease,lease}),"duplicate native slot rejected");
  auto stale=good;int checks=0;std::string error;
  require(!ValidateNativeArrowLeases([&](auto a,auto b,auto c){return stale.Read(a,b,c);},
    base,0x70000,{lease},[&]{return ++checks==1;},&error),"world replaced before commit cannot recover");
  auto empty=good;require(check(empty,{}),"unused pool can enter a fresh world with unchanged graphics");
  std::puts("PASS: current native asset membership, retained pool resources, bounds, mutation and stale-world recovery rejection");
}
