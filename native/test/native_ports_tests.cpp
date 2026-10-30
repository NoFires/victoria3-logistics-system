#include "native_ports.h"
#include <cstdio>
#include <cstdlib>
#include <limits>

void Check(bool ok,const char* why){if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
  static constexpr uint64_t module=0x140000000,root=0x10000,machine=0x11000,world=0x12000,
      graphics=0x20000,manager=0x30000,types=0x40000,type=0x50000,regions=0x60000,
      region=0x70000,definition=0x80000,instances=0x90000,headers=0xa0000,positions=0xb0000;
  static constexpr uint32_t second=0xab000801;
  std::map<uint64_t,uint8_t> bytes;uint64_t mutate=0;unsigned reads=0;bool current=true;std::string error;
  std::map<uint32_t,v3::NativeSeaRegion> states{{0,{501,1001}},{second,{502,1002}}};
  template<class T>void Put(uint64_t a,T value){for(size_t i=0;i<sizeof(value);++i)bytes[a+i]=((uint8_t*)&value)[i];}
  void Zero(uint64_t a,size_t n){for(size_t i=0;i<n;++i)bytes[a+i]=0;}
  void Array(uint64_t a,uint64_t data,uint32_t count){Zero(a,0x18);Put(a,data);Put(a+8,count);Put(a+12,count);}
  void Position(unsigned category,unsigned slot,float x,float z){
    std::array<float,10> transform{};transform[0]=x;transform[1]=500;transform[2]=z;
    Put(positions+category*0x100+slot*0x28,transform);
  }
  Fixture(){
    Put(module+0x0592d128,root);Put(root+0x608,machine);Put(machine+0x120,world);
    Put(module+0x05901fa0,graphics);Put(graphics,module+0x0464e4e8);Put(graphics+0x290,manager);
    Array(world+0x3c70,regions,503);Array(module+0x051385c0,types,5);Zero(types,5*16);
    Array(manager+0x1f8,headers,5);
    for(unsigned i=0;i<2;++i){
      Put(regions+(501+i)*8,region+i*0x100);Put(region+i*0x100,module+0x044df758);
      Put(region+i*0x100+0x90,definition+i*0x100);Zero(definition+i*0x100,0x48);
      Put(definition+i*0x100,module+0x044e7850);Put(definition+i*0x100+0x10,uint32_t(501+i));
      Put(definition+i*0x100+0x40,uint32_t(1001+i));Put(definition+i*0x100+0x44,uint8_t(1));
    }
    for(unsigned category=0;category<5;++category){
      const uint64_t object=type+category*0x200;
      Put(types+category*16,object);Zero(object,0xa0);Put(object,module+0x047295f0);
      Put(object+0x90,category);Put(object+0x98,category);
      Array(object+0x50,instances+category*0x100,2);Zero(instances+category*0x100,2*0x30);
      for(unsigned slot=0;slot<2;++slot){
        Put(instances+category*0x100+slot*0x30+4,uint32_t(1001+slot));
        // Authored positions deliberately disagree with the current manager.
        Put(instances+category*0x100+slot*0x30+8,std::array<float,3>{88,99,77});
        Position(category,slot,-1,-1);
      }
      Array(headers+category*0x18,positions+category*0x100,2);
    }
    Position(4,0,10,20);Position(4,1,0,0);
  }
  bool Read(uint64_t a,void* out,size_t n){
    for(size_t i=0;i<n;++i){auto f=bytes.find(a+i);if(f==bytes.end())return false;((uint8_t*)out)[i]=f->second;}
    if(a==mutate && ++reads==1)bytes[a]^=1;return true;
  }
  auto Decode(){return v3::DecodeNativePortHubs([&](auto a,auto p,auto n){return Read(a,p,n);},module,states,
      [&]{return current;},&error);}
};
int main(){
  Check(v3::NativePortSplineKeys(3000,1)==std::array<uint64_t,2>{0x4249f0020001903,0x4249f0020001903} &&
      v3::NativePortSplineKeys(1,3000)[0]==0x4249f0020001903 &&
      v3::NativePortSplineKeys(3000,4)[0]==0x4249f0020006403 &&
      v3::NativePortSplineKeys(3000,152)[0]==0x4249f00200ed803,
      "type3 packing matches independent actual installed spline connection records");
  Fixture f;const auto original=f.bytes;auto result=f.Decode();
  if(!result)std::printf("error: %s\n",f.error.c_str());
  Check(result && result->size()==2 && result->at(0) && !result->at(Fixture::second),
      "full state IDs/definition IDs/geo IDs stay distinct; all five current hub types considered");
  f.Position(4,0,-1,0);auto literal=f.Decode();
  Check(literal && literal->at(0),"literal native eligibility accepts one zero and one negative coordinate");
  f.Position(4,0,0,0);literal=f.Decode();
  Check(literal && !literal->at(0),"current origin is ineligible even when authored position exists");
  f.bytes=original;f.Put(Fixture::type+4*0x200,Fixture::module+0x047435c8);literal=f.Decode();
  Check(literal && !literal->at(0),"native null hub type is known ineligible, not a guessed nearest port");
  f.bytes=original;f.Put(Fixture::type+4*0x200+0x90,UINT32_MAX);literal=f.Decode();
  Check(literal && literal->at(0),"locator IsValid is constant true; category ID is not a parsed-type validity flag");
  auto bad=[&](auto change,const char* why){f.bytes=original;f.states={{0,{501,1001}},{Fixture::second,{502,1002}}};
    f.mutate=0;f.reads=0;f.current=true;change(f);Check(!f.Decode(),why);};
  bad([](auto& m){m.current=false;},"retired source cannot read hubs");
  bad([](auto& m){m.template Put<uint64_t>(Fixture::graphics,1);},"unknown graphics object rejected");
  bad([](auto& m){m.template Put<uint64_t>(Fixture::graphics+0x290,0);},"missing current manager rejected");
  bad([](auto& m){m.template Put<uint32_t>(Fixture::world+0x3c78,502);},"current region count above capacity rejected");
  bad([](auto& m){m.states[0].definition=503;},"state template outside runtime region array rejected");
  bad([](auto& m){m.template Put<uint64_t>(Fixture::region,1);},"unknown current region type rejected");
  bad([](auto& m){m.template Put<uint32_t>(Fixture::definition+0x40,1003);},"state template and actual runtime region geo mismatch rejected");
  bad([](auto& m){m.states[Fixture::second].geo=1001;},"two different definitions cannot impersonate one geo ID");
  bad([](auto& m){m.template Put<uint32_t>(Fixture::module+0x051385cc,4);},"incomplete current hub registration is unavailable");
  bad([](auto& m){m.template Put<uint64_t>(Fixture::type,1);},"unknown hub type rejected");
  bad([](auto& m){m.template Put<uint64_t>(Fixture::type,Fixture::module+0x04725b98);},"parsed map object type cannot impersonate the current registered locator");
  bad([](auto& m){m.template Put<uint32_t>(Fixture::type+0x98,5);},"native finalized type index outside transforms rejected");
  bad([](auto& m){m.template Put<uint32_t>(Fixture::type+0x5c,3);},"instance count above capacity rejected");
  bad([](auto& m){m.template Put<uint32_t>(Fixture::headers+12,1);},"current transform count mismatch rejected");
  bad([](auto& m){m.template Put<uint32_t>(Fixture::instances+4,1003);},"unsorted original instance IDs rejected");
  bad([](auto& m){m.template Put<uint64_t>(Fixture::manager+0x1f8,UINT64_MAX-8);},"transform header pointer overflow rejected");
  bad([](auto& m){m.template Put<uint64_t>(Fixture::headers,UINT64_MAX-8);},"current position pointer overflow rejected");
  bad([](auto& m){m.Position(4,0,std::numeric_limits<float>::quiet_NaN(),20);},"nonfinite current hub position rejected");
  bad([](auto& m){m.bytes.erase(Fixture::positions);},"unreadable current position rejects whole copy");
  for(auto a:{Fixture::module+0x0592d128,Fixture::graphics+0x290,Fixture::world+0x3c70,
      Fixture::regions+501*8,Fixture::region+0x90,Fixture::definition,Fixture::types,Fixture::type,
      Fixture::manager+0x1f8,Fixture::headers,Fixture::instances,Fixture::positions})
    bad([&](auto& m){m.mutate=a;},"mutation in any observed port component rejects the entire copy");
  f.bytes=original;f.current=true;f.mutate=0;unsigned calls=0;
  Check(!v3::DecodeNativePortHubs([&](auto a,auto p,auto n){return f.Read(a,p,n);},Fixture::module,f.states,
      [&]{return ++calls==1;}),"source retirement at publication rejects copied hubs");
  f.bytes.clear();Check(result->at(0) && !result->at(Fixture::second),"owned eligibility survives source destruction");

  v3::NativeWorldTradeSnapshot trade;auto geometry=std::make_shared<v3::NativeSplineNetwork>();
  v3::NativeSeaGeography geography;
  geography.states={{0,{501,1001}},{Fixture::second,{502,1002}}};geography.stateHubs={{0,true},{Fixture::second,true}};
  geography.provinces={{1,{{601,11},{2,9}}},{2,{{602,12},{1,9}}}};
  v3::NativeTradeDestination destination;destination.state=Fixture::second;destination.selected=0;
  destination.candidates={{{{1,2}},{},100000}};
  const auto first=v3::NativePortSplineKeys(11,1001)[0],later=v3::NativePortSplineKeys(12,1001)[0],
      target=v3::NativePortSplineKeys(12,1002)[0];
  geometry->connections[v3::NativeSeaSplineKeys(11,12)[0]]={1};
  geometry->connections[first]={}; // original first known link is empty; do not substitute later
  geometry->connections[later]={2};geometry->connections[target]={3};trade.geometry=geometry;
  auto selected=v3::SelectNativeRouteConnections(trade,geography,0,destination);
  Check(selected && selected->sea.keys.size()==1 && selected->ports[0]==first && selected->ports[0]!=later &&
      selected->ports[1]==target,"original first boundary candidate wins per endpoint; known empty is not replaced by a later line");
  geography.stateHubs[0]=false;selected=v3::SelectNativeRouteConnections(trade,geography,0,destination);
  Check(selected && !selected->ports[0] && selected->ports[1]==target,"known ineligible endpoint does not fabricate a shore connector");
  geography.stateHubs[0]=true;geography.provinces[1].adjacent={2};geography.provinces[2].adjacent={1};
  selected=v3::SelectNativeRouteConnections(trade,geography,0,destination);
  Check(selected && !selected->ports[0] && !selected->ports[1],"original considers port candidates only at ProvinceSet boundaries");
  geography.provinces[1].adjacent.push_back(9);geography.provinces[2].adjacent.push_back(9);
  Check(!v3::SelectNativeRouteConnections(trade,geography,0x22000000,destination),"recycled low state slot cannot impersonate endpoint");
  geography.stateHubs.erase(0);Check(!v3::SelectNativeRouteConnections(trade,geography,0,destination),"missing hub data stays unavailable");
  geography.stateHubs[0]=true;destination.selected.reset();
  Check(!v3::SelectNativeRouteConnections(trade,geography,0,destination),"unselected native path cannot choose ports");
  destination.candidates.clear();selected=v3::SelectNativeRouteConnections(trade,geography,0,destination);
  Check(selected && selected->sea.keys.empty() && !selected->ports[0] && !selected->ports[1],"known empty native path remains empty");
  std::puts("PASS: current native hub copy, exact endpoint key/boundary/tie semantics, bounds, mutation, retirement and owned lifetime");
}
