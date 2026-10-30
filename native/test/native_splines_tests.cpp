#include "native_splines.h"
#include <cstdio>
#include <cstdlib>
#include <limits>

void Check(bool ok,const char* why){if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
  static constexpr uint64_t module=0x140000000,graphics=0x10000,database=0x20000,
      templates=0x30000,pixels=0x40000,points=0x50000,strips=0x60000,connections=0x70000,
      controls=0x80000,segments=0x90000,idA=0x100000002,idB=0x200000002,key=(uint64_t(30)<<35)|2;
  std::map<uint64_t,uint8_t> bytes;uint64_t mutate=0;int reads=0;bool current=true;
  std::string error;
  template<class T>void Put(uint64_t at,T value){for(size_t i=0;i<sizeof(value);++i)bytes[at+i]=((uint8_t*)&value)[i];}
  void Zero(uint64_t at,size_t size){for(size_t i=0;i<size;++i)bytes[at+i]=0;}
  void Map(uint64_t at,uint64_t data,uint32_t count,uint32_t mask){
    Zero(at,0x28);Put(at+8,data);Put(at+0x10,count);Put(at+0x14,mask);
  }
  Fixture(){
    Put(module+0x05901fa0,graphics);Put(graphics,module+0x0464e4e8);Put(graphics+0x2c0,database);
    Put(module+0x058fccf0,templates);Put(templates,module+0x0441c938);Put(templates+0x240,pixels);
    Put(pixels+0x50,uint32_t(300));Put(pixels+0x54,uint32_t(200));
    Map(database,points,4,7);Map(database+0x28,strips,2,1);Map(database+0x50,connections,1,1);
    Zero(points,8*0x1c);Zero(strips,2*0x30);Zero(connections,2*0x38);
    for(unsigned i=0;i<4;++i){
      const auto at=points+i*0x1c;Put(at,uint8_t(1));Put(at+4,uint32_t(i*10)|0x40000000);
      Put(at+8,v3::NativeSplinePoint{float(100+i*10+(i==3?10:0)),float(i==1?30:20)});
    }
    for(unsigned i=0;i<2;++i){
      const auto at=strips+i*0x30;Put(at+4,uint8_t(1));const auto id=i?idB:idA;
      Put(at+8,id|(uint64_t(0xab)<<56));Put(at+0x10,controls+i*0x100);
      Put(at+0x18,uint32_t(i?2:3));Put(at+0x1c,uint32_t(i?2:3));Put(at+0x28,id);
    }
    Put(controls,std::array<uint32_t,3>{20,10,0});Put(controls+0x100,std::array<uint32_t,2>{20,30});
    Put(connections+4,uint8_t(1));Put(connections+8,key);Put(connections+0x10,segments);
    Put(connections+0x18,uint32_t(2));Put(connections+0x1c,uint32_t(2));Put(connections+0x28,key);
    Put(segments,std::array<uint64_t,2>{idA|(uint64_t(0xcd)<<56),idB});
  }
  bool Read(uint64_t at,void* out,size_t size){
    for(size_t i=0;i<size;++i){auto f=bytes.find(at+i);if(f==bytes.end())return false;((uint8_t*)out)[i]=f->second;}
    if(at==mutate && ++reads==1)bytes[at]^=1;return true;
  }
  auto Decode(){return v3::DecodeNativeSplineNetwork([&](auto a,auto p,auto n){return Read(a,p,n);},module,[&]{return current;},&error);}
};
int main(){
  Fixture f;const auto original=f.bytes;auto copy=f.Decode();if(!copy)std::printf("error=%s\n",f.error.c_str());
  Check(copy && copy->width==300 && copy->height==200 && copy->points.size()==4 && copy->strips.size()==2 &&
      copy->connections.size()==1,"current native dimensions, controls, segments and connection copied");
  Check(copy->points.at(0).x==100 && copy->strips.at(Fixture::idA)==std::vector<uint32_t>{20,10,0} &&
      copy->connections.at(Fixture::key)==std::vector<uint64_t>{Fixture::idA,Fixture::idB},
      "preserve zero anchor and 56-bit segment identities while removing native style flags");
  auto graph=v3::BuildNativeSplineGraph(*copy,&f.error);
  Check(graph && graph->Valid() && graph->edges.size()==2 && graph->nativeConnections.at(Fixture::key)==std::vector<uint32_t>{0,1},
      "split native connection binds both dense edges without truncating colliding native segment IDs");
  Check(graph->nodes.at(graph->byId.at(0)).z==180 && graph->nodes.at(graph->byId.at(30)).x==140 &&
      graph->points[0].x==120 && graph->points[1].x==110 && graph->points[1].z==170 && graph->points[2].x==100,
      "original turns and authored orientation preserved; Z converted once using current height");
  auto changed=*copy;changed.points.at(10).x=105;
  auto moved=v3::BuildNativeSplineGraph(changed);
  Check(moved && moved->points[1].x==105 && moved->edges[0].length!=graph->edges[0].length,
      "same connection and endpoints with changed control geometry produce the new shape and length");
  for(uint32_t kind=0;kind<4;++kind){
    auto typed=*copy;typed.strips.clear();typed.connections.clear();
    const uint64_t a=(Fixture::idA&~uint64_t(63))|kind,b=(Fixture::idB&~uint64_t(63))|kind;
    typed.strips[a]=copy->strips.at(Fixture::idA);typed.strips[b]=copy->strips.at(Fixture::idB);
    typed.connections[(Fixture::key&~uint64_t(63))|kind]={a,b};
    auto transport=v3::BuildNativeSplineGraph(typed);
    Check(transport && transport->edges.size()==2 && transport->edges[0].type==kind,
        "four original transport types retain their exact kind in the current geometry graph");
  }
  auto test=[&](auto change,const char* why){f.bytes=original;f.mutate=0;f.reads=0;f.current=true;change(f);Check(!f.Decode(),why);};
  test([](auto& m){m.current=false;},"retired source rejected before copying");
  test([](auto& m){m.template Put<uint64_t>(Fixture::graphics,1);},"wrong graphics type rejected");
  test([](auto& m){m.template Put<uint64_t>(Fixture::graphics+0x2c0,UINT64_MAX-8);},"database pointer overflow rejected");
  test([](auto& m){m.template Put<uint64_t>(Fixture::templates,1);},"wrong current map type rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::pixels+0x54,0);},"missing current map dimensions rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::database+0x14,6);},"non power-of-two point buckets rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::database+0x10,5);},"point active count mismatch rejected");
  test([](auto& m){m.template Put<uint8_t>(Fixture::points,255);},"in-band point sentinel rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::points+4,0xffffffff);},"invalid point identity rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::points+0x1c+4,0);},"duplicate masked native point rejected");
  test([](auto& m){m.template Put<float>(Fixture::points+8,std::numeric_limits<float>::infinity());},"nonfinite native point rejected");
  test([](auto& m){m.template Put<uint64_t>(Fixture::strips+0x30+8,Fixture::idA);
    m.template Put<uint64_t>(Fixture::strips+0x30+0x28,Fixture::idA);},"duplicate masked segment rejected");
  test([](auto& m){m.template Put<uint64_t>(Fixture::strips+0x28,0);},"segment key/value identity mismatch rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::strips+0x1c,4);},"control count above capacity rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::controls,999);},"unknown native control rejected");
  test([](auto& m){m.template Put<uint64_t>(Fixture::connections+0x28,0);},"connection key/value identity mismatch rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::connections+0x1c,3);},"segment count above capacity rejected");
  test([](auto& m){m.template Put<uint64_t>(Fixture::segments,0);},"unknown native segment rejected");
  test([](auto& m){for(unsigned i=0;i<0x38;++i)m.bytes[Fixture::connections+0x38+i]=m.bytes.at(Fixture::connections+i);
    m.template Put<uint32_t>(Fixture::database+0x60,2);},"duplicate full native connection rejected");
  test([](auto& m){m.bytes.erase(Fixture::controls);},"unreadable native controls rejected");
  test([](auto& m){m.template Put<uint32_t>(Fixture::database+0x60,0);},"unloaded native connection geometry unavailable");
  for(auto address:{Fixture::module+0x05901fa0,Fixture::graphics+0x2c0,Fixture::templates+0x240,Fixture::pixels+0x54,
      Fixture::database,Fixture::points,Fixture::strips,Fixture::connections,Fixture::controls,Fixture::segments})
    test([&](auto& m){m.mutate=address;},"mutation in any input rejects the combined native geometry copy");
  f.bytes=original;f.mutate=0;f.reads=0;int checks=0;
  Check(!v3::DecodeNativeSplineNetwork([&](auto a,auto p,auto n){return f.Read(a,p,n);},Fixture::module,[&]{return ++checks==1;}),
      "retirement before publication rejected");
  auto invalid=*copy;invalid.connections.begin()->second={Fixture::idB,Fixture::idA};
  Check(!v3::BuildNativeSplineGraph(invalid),"misordered segments cannot manufacture a connection");
  invalid=*copy;invalid.connections.emplace((uint64_t(40)<<35)|2,invalid.connections.begin()->second);
  Check(!v3::BuildNativeSplineGraph(invalid),"wrong final native anchor rejected");
  invalid=*copy;invalid.points.at(10).x=std::numeric_limits<float>::quiet_NaN();
  Check(!v3::BuildNativeSplineGraph(invalid),"nonfinite worker geometry rejected");
  f.bytes=original;f.current=true;f.mutate=0;f.reads=0;
  f.Put(Fixture::connections+0x1c,uint32_t(0));f.Put(Fixture::connections+0x10,uint64_t(0));
  auto empty=f.Decode();auto emptyGraph=empty?v3::BuildNativeSplineGraph(*empty):std::nullopt;
  Check(emptyGraph && emptyGraph->edges.empty() && emptyGraph->nativeConnections.count(Fixture::key) &&
      emptyGraph->nativeConnections.at(Fixture::key).empty(),"known empty native connection remains empty; no invented line or pointer read");
  f.bytes.clear();Check(copy->strips.at(Fixture::idB)[1]==30 && graph->points[1].x==110,
      "native source destruction cannot change owned network or converted geometry");
  std::puts("PASS: current native spline copies, exact full segment identities, split/oriented controls, current map dimensions, same-key geometry changes, bounds and retirement");
}
