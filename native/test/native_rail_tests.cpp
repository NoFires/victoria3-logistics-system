#include "native_rail.h"
#include <cstdio>
#include <cstdlib>
#include <map>

void require(bool value,const char* what){if(!value){std::printf("FAIL: %s\n",what);std::exit(1);}}
struct Memory {
  std::map<uint64_t,uint8_t> bytes;bool current=true,mutate=false;int reads=0;
  template<class T>void Put(uint64_t address,T value){auto* p=(uint8_t*)&value;for(size_t i=0;i<sizeof(value);++i)bytes[address+i]=p[i];}
  void Zero(uint64_t address,size_t count){for(size_t i=0;i<count;++i)bytes[address+i]=0;}
  bool Read(uint64_t address,void* out,size_t size){
    for(size_t i=0;i<size;++i){auto it=bytes.find(address+i);if(it==bytes.end())return false;((uint8_t*)out)[i]=it->second;}
    if(address==0x1592d128 && ++reads==1 && mutate)Put<uint64_t>(address,0);
    return true;
  }
};
int main(){
  constexpr uint64_t base=0x10000000,keyA=(uint64_t(200)<<35)|(uint64_t(100)<<6)|1,
    keyB=(uint64_t(300)<<35)|(uint64_t(200)<<6)|1;
  v3::SplineGraph graph;graph.width=100;graph.height=100;
  graph.nodes={{100,0,0},{0x10000001,5,1},{200,10,0},{300,20,0}};
  graph.points={{0,0},{5,1},{5,1},{10,0},{10,0},{20,0}};
  graph.edges={{11,0,1,1,0,2,5.1f},{12,1,2,1,2,2,5.1f},{13,2,3,1,4,2,10}};
  graph.nativeConnections={{keyA,{11,12}},{keyB,{13}}};
  Memory good;good.Put<uint64_t>(base+0x0592d128,0x10000);good.Put<uint64_t>(0x10608,0x11000);
  good.Put<uint64_t>(0x11120,0x20000);good.Put<uint64_t>(0x25618,base+0x044bb380);
  good.Zero(0x25680,0x18);good.Put<uint64_t>(0x25680,0x30000);good.Put<uint32_t>(0x25688,2);good.Put<uint32_t>(0x2568c,2);
  for(unsigned i=0;i<3;++i){
    const uint64_t node=0x60000+i*0x100,def=0x70000+i*0x100;
    good.Zero(node,0x80);good.Put<uint64_t>(node,base+0x044e3d78);good.Put<uint64_t>(node+0x78,def);
    good.Put<int64_t>(def+0x48,i*1000000);good.Put<int64_t>(def+0x50,10000000);
  }
  for(unsigned i=0;i<2;++i){
    const uint64_t object=0x40000+i*0x100,def=0x50000+i*0x100;
    good.Put<uint64_t>(0x30000+i*8,object);good.Zero(object,0x38);good.Put<uint64_t>(object,base+0x044e08d8);
    good.Put<uint32_t>(object+8,i);good.Put<uint64_t>(object+0x18,def);good.Put<uint32_t>(object+0x20,i==0?4:2);
    good.Put<uint64_t>(object+0x28,0x60000+i*0x100);good.Put<uint64_t>(object+0x30,0x60100+i*0x100);
    good.Zero(def,0x80);good.Put<uint64_t>(def,base+0x04515e20);good.Put<uint32_t>(def+0x10,i);
    good.Put<uint64_t>(def+0x40,0x70000+i*0x100);good.Put<uint64_t>(def+0x48,0x70100+i*0x100);
    good.Put<uint64_t>(def+0x50,i==0?keyA:keyB);good.Put<uint32_t>(def+0x58,1);
    good.Put<uint64_t>(def+0x68,0x80000+i*0x100);good.Put<uint32_t>(def+0x70,i==0?2:1);good.Put<uint32_t>(def+0x74,i==0?2:1);
  }
  good.Put<std::array<int64_t,3>>(0x80000,{500000,500000,9900000});
  good.Put<std::array<int64_t,3>>(0x80018,{500000,1000000,10000000});
  good.Put<std::array<int64_t,3>>(0x80100,{1000000,2000000,10000000});
  auto check=[&](Memory& memory,const v3::SplineGraph& map){
    return v3::DecodeNativeRails([&](auto a,auto b,auto c){return memory.Read(a,b,c);},base,map,[&]{return memory.current;});
  };
  auto result=check(good,graph);
  require(result && result->potentialConnections==2 && result->builtConnections==1 &&
    result->builtEdges==std::unordered_set<uint32_t>{11,12},"one built native connection enables both physical strips only");
  auto test=[&](auto change,const char* what){auto m=good;auto g=graph;change(m,g);require(!check(m,g),what);};
  test([&](auto& m,auto&){m.template Put<uint64_t>(0x25618,base);},"unknown manager type rejected");
  test([&](auto& m,auto&){m.template Put<uint32_t>(0x25688,131073);},"roster capacity bound enforced");
  test([&](auto& m,auto&){m.template Put<uint32_t>(0x2568c,3);},"roster count exceeds capacity rejected");
  test([&](auto& m,auto&){m.template Put<uint64_t>(0x30008,0x40000);},"duplicate runtime object rejected");
  test([&](auto& m,auto&){m.template Put<uint32_t>(0x40108,0);},"duplicate runtime ID rejected");
  test([&](auto& m,auto&){m.template Put<uint64_t>(0x50000,base);},"wrong definition type rejected");
  test([&](auto& m,auto&){m.template Put<uint64_t>(0x50150,keyA);},"duplicate native key rejected");
  test([&](auto&,auto& g){g.nativeConnections.erase(keyB);},"unknown current map key rejected");
  test([&](auto&,auto& g){g.nativeConnections.emplace(keyB+64,std::vector<uint32_t>{13});},"map connection missing from world rejected");
  test([&](auto&,auto& g){g.nativeConnections[keyA]={11};},"incomplete split link rejected");
  test([&](auto&,auto& g){g.points[1].x=8;},"same-key changed geometry rejected");
  test([&](auto& m,auto&){m.template Put<uint64_t>(0x60078,0x70100);},"endpoint definition mismatch rejected");
  test([&](auto& m,auto&){m.template Put<uint32_t>(0x50074,3);},"path count mismatch rejected");
  test([&](auto& m,auto&){m.template Put<uint64_t>(0x50068,UINT64_MAX-8);},"path address overflow rejected");
  test([&](auto& m,auto&){m.bytes.erase(0x80000);},"unreadable path rejected");
  test([&](auto& m,auto&){m.template Put<uint32_t>(0x40020,8);},"unknown current transport type rejected");
  test([&](auto& m,auto&){m.template Put<uint32_t>(0x50058,0);},"railway without physical rail definition rejected");
  test([&](auto& m,auto&){m.mutate=true;m.reads=0;},"world chain mutation during copy rejected");
  test([&](auto& m,auto&){m.current=false;},"retired snapshot rejected");
  auto zero=good;zero.Put<uint32_t>(0x40020,2);require(check(zero,graph)->builtEdges.empty(),"known no-built-rail snapshot remains valid");
  auto rebuilt=good;rebuilt.Put<uint32_t>(0x40120,4);require(check(rebuilt,graph)->builtEdges.size()==3,"new native built state changes copied rail membership");
  auto stale=good;int checks=0;
  require(!v3::DecodeNativeRails([&](auto a,auto b,auto c){return stale.Read(a,b,c);},base,graph,[&]{return ++checks==1;}),"retirement before publication rejected");
  std::puts("PASS: native built railway identity, split physical strips, geometry, removal/change, bounds and stale-copy rejection");
}
