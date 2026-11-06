#include "native_travel.h"
#include <cstdio>
#include <cstdlib>
#include <map>
using namespace v3;
void Check(bool value,const char* why){if(!value){std::printf("FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
  static constexpr uint64_t module=0x140000000,root=0x10000,machine=0x20000,world=0x30000,
      land=0x40000,naval=0x50000,roads=0x60000,ports=0x70000,landDef=0x80000,navalDef=0x90000,
      roadDef=0xa0000,portDef=0xb0000;
  std::map<uint64_t,uint8_t> bytes;
  template<class T>void Put(uint64_t at,T v){for(size_t i=0;i<sizeof(v);++i)bytes[at+i]=((uint8_t*)&v)[i];}
  void Zero(uint64_t at,size_t n){for(size_t i=0;i<n;++i)bytes[at+i]=0;}
  void Array(uint64_t offset,uint64_t data,uint32_t count){
    const auto at=world+0x5028+offset;Zero(at,24);Put(at,data);Put(at+8,count);Put(at+12,count);
  }
  void Node(uint64_t at,uint64_t definition,uint32_t id,uint16_t roles,uint16_t province,uint32_t control){
    Zero(at,0x30);Zero(definition,0x20);Put(at,id);Put(at+6,province);Put(at+8,definition);
    Put(at+0x10,uint32_t(0xab000010));Put(definition,id);Put(definition+4,roles);
    Put(definition+8,control);Put(definition+0xc,province);
    Put(definition+0x10,int64_t(1200000+id*100000));Put(definition+0x18,int64_t(2400000));
  }
  void Connection(uint64_t at,uint64_t definition,uint8_t kind,uint8_t development,
      uint64_t from,uint64_t to,uint64_t fromDef,uint64_t toDef,uint64_t key){
    Zero(at,0x40);Zero(definition,0x28);Put(at+4,development);Put(at+8,definition);
    Put(at+0x10,from);Put(at+0x18,to);Put(at+0x20,int64_t(345678));
    Put(definition+4,kind);Put(definition+8,fromDef);Put(definition+0x10,toDef);
    Put(definition+0x18,int64_t(123456));Put(definition+0x20,key);
  }
  Fixture(){
    Put(module+0x05c07c68,root);Put(root,module+0x04660618);Put(root+0x608,machine);
    Put(machine+0x120,world);Put(world+0x5028,module+0x046b1d48);
    Array(0x10,land,2);Array(0x40,naval,1);Array(0x28,roads,1);Array(0x58,ports,1);
    Node(land,landDef,0,1,10,321);Node(land+0x30,landDef+0x20,1,0x402,11,654);
    Node(naval,navalDef,0,0x200,100,8388808);
    Connection(roads,roadDef,2,4,land,land+0x30,landDef,landDef+0x20,0x1234567891);
    Connection(ports,portDef,6,5,naval,land+0x30,navalDef,landDef+0x20,UINT64_MAX);
  }
  bool Read(uint64_t at,void* p,size_t n)const{
    for(size_t i=0;i<n;++i){auto it=bytes.find(at+i);if(it==bytes.end())return false;((uint8_t*)p)[i]=it->second;}return true;
  }
  auto Decode(){return DecodeNativeTravelNetwork([&](auto at,auto p,auto n){return Read(at,p,n);},module,[]{return true;});}
};
int main(){
  Fixture f;const auto original=f.bytes;auto copy=f.Decode();
  Check(copy && copy->nodes[0].size()==2 && copy->nodes[1].size()==1,"copy both current node arrays");
  Check(copy->nodes[0][0].id==copy->nodes[1][0].id && copy->nodes[1][0].control==8388808,
      "array-local IDs may coincide; no runtime ID to map key table");
  const auto& road=copy->connections[0][0];const auto& port=copy->connections[1][0];
  Check(road.development==4 && road.kind==2 && road.spline==0x1234567891 && road.distanceFixed==123456 && road.costFixed==345678,
      "built state, definition kind, physical key, native distance and cost stay separate");
  Check(port.spline==UINT64_MAX && port.endpoints[0].naval && !port.endpoints[1].naval && port.endpoints[1].index==1,
      "native keyless port access retains real mixed-array endpoints without invented geometry");
  auto reject=[&](auto mutate,const char* why){f.bytes=original;mutate();Check(!f.Decode(),why);};
  reject([&]{f.Put(Fixture::world+0x5028,uint64_t(1));},"wrong manager type rejected");
  reject([&]{f.Put(Fixture::world+0x5038+8,uint32_t(1));},"count exceeds capacity rejected");
  reject([&]{f.Put(Fixture::world+0x5038+8,uint32_t(131073));},"unbounded node capacity rejected");
  reject([&]{f.Put(Fixture::world+0x5038,UINT64_MAX-1);},"node byte range overflow rejected");
  reject([&]{f.Put(Fixture::landDef,uint32_t(1));},"definition ID differs from current node rejected");
  reject([&]{f.Put(Fixture::navalDef+4,uint16_t(2));},"wrong node array roles rejected");
  reject([&]{f.Put(Fixture::land+6,uint16_t(999));},"runtime and definition province mismatch rejected");
  reject([&]{f.Put(Fixture::roads+4,uint8_t(7));},"unknown development rejected");
  reject([&]{f.Put(Fixture::roadDef+4,uint8_t(9));},"unknown definition connection kind rejected");
  reject([&]{f.Put(Fixture::roadDef+0x18,int64_t(-1));},"negative original distance rejected");
  reject([&]{f.Put(Fixture::roads+0x20,int64_t(-1));},"negative current cost rejected");
  reject([&]{f.Put(Fixture::ports+0x10,Fixture::land);},"same numeric node ID from wrong array rejected");
  reject([&]{f.Put(Fixture::roads+0x18,Fixture::land+0x31);},"interior endpoint pointer rejected");
  reject([&]{f.Put(Fixture::roadDef+0x10,Fixture::navalDef);},"changed endpoint definition cannot borrow another runtime node");
  f.bytes=original;
  for(const auto& [at,value]:original){
    GoodsMemoryReader denied=[&](uint64_t address,void* out,size_t n){return !(address<=at && at-address<n) && f.Read(address,out,n);};
    Check(!DecodeNativeTravelNetwork(denied,Fixture::module,[]{return true;}),"every unreadable dependency rejected");
  }
  unsigned calls=0;
  Check(!DecodeNativeTravelNetwork([&](auto at,auto p,auto n){return f.Read(at,p,n);},Fixture::module,[&]{return ++calls<4;}),
      "source retirement during copy rejected");
  unsigned observations=0;
  auto changing=[&](uint64_t at,void* out,size_t n){
    if(at==Fixture::landDef && ++observations==2)f.Put(Fixture::landDef+8,uint32_t(999));return f.Read(at,out,n);
  };
  Check(!DecodeNativeTravelNetwork(changing,Fixture::module,[]{return true;}),"definition mutation during combined copy rejected");
  f.bytes.clear();
  Check(copy->nodes[0][1].control==654 && copy->connections[1][0].distanceFixed==123456,
      "numeric result survives complete source destruction");
  std::puts("PASS: 1.15 travel arrays, native endpoint identities, keyless access, bounds, read failures, mutation and retirement");
}
