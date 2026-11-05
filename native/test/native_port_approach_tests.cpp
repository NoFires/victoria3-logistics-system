#include "native_port_approach.h"
#include <cstdio>
#include <cstdlib>
using namespace v3;
void Check(bool ok,const char* why){if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
int main(){
  constexpr uint64_t module=0x140000000,db=0x10000,entries=0x11000,stateObject=0x12000,otherState=0x14000,
      stateRegion=0x16000,physicalRegion=0x18000,logicalRegion=0x1a000,root=0x20000,machine=0x21000,
      world=0x22000,map=0x30000,definitions=0x31000,seaPhysical=0x32000,seaLogical=0x33000,
      provinces=0x34000,province=0x35000,roster=0x40000,connection=0x41000,definition=0x42000,
      landNode=0x43000,seaNode=0x44000,landRuntime=0x45000,seaRuntime=0x46000,members=0x47000,
      countryDb=0x58000,countryEntries=0x59000,country=0x5a000,otherCountry=0x5b000,
      marketDb=0x60000,marketEntries=0x61000,market=0x62000,otherMarket=0x63000,zeroMarket=0x64000;
  constexpr uint32_t state=0xab000001,owner=0xcd000002;
  std::map<uint64_t,uint8_t> bytes;std::set<uint64_t> dependencies;bool record=false,live=true;
  auto put=[&](uint64_t at,auto value){for(size_t i=0;i<sizeof(value);++i)bytes[at+i]=((uint8_t*)&value)[i];};
  auto fill=[&](uint64_t at,size_t n){for(size_t i=0;i<n;++i)bytes[at+i]=0;};
  GoodsMemoryReader read=[&](uint64_t at,void* p,size_t n){
    for(size_t i=0;i<n;++i){if(record)dependencies.insert(at+i);auto found=bytes.find(at+i);if(found==bytes.end())return false;((uint8_t*)p)[i]=found->second;}return true;
  };
  auto region=[&](uint64_t at,uint32_t id,uint32_t geo){
    fill(at,0x1d0);put(at,module+0x044e7850);put(at+0x10,id);put(at+0x40,geo);put(at+0x44,uint8_t(1));
  };
  put(module+0x05964ef0,db);put(db+0x20,entries);put(db+0x2c,uint32_t(4));fill(entries,72);
  for(auto [object,id]:{std::pair{stateObject,state},std::pair{otherState,owner}}){
    put(entries+8+uint64_t(id&0xffffff)*16,object);put(object,module+0x044de308);put(object+8,id);
    put(object+0x48,id);put(object+0xe78,stateRegion);
    put(object+0x11b8,uint8_t(0));put(object+0xb48,UINT32_MAX);put(object+0xe48,uint32_t(id==state?0:1));
  }
  put(module+0x05964e20,countryDb);put(countryDb+0x20,countryEntries);put(countryDb+0x2c,uint32_t(2));
  put(countryEntries+8,country);put(countryEntries+24,otherCountry);
  for(auto [object,id]:{std::pair{country,0u},std::pair{otherCountry,1u}}){
    put(object,module+0x04494cb8);put(object+0x18,id);put(object+0xa24,uint32_t(7));
  }
  put(module+0x05964ed8,marketDb);put(marketDb+0x20,marketEntries);put(marketDb+0x2c,uint32_t(10));
  for(auto [object,id]:{std::pair{market,7u},std::pair{otherMarket,9u},std::pair{zeroMarket,0u}}){
    put(marketEntries+8+id*16,object);put(object,module+0x0449c648);put(object+8,id);
  }
  region(stateRegion,17,42);region(physicalRegion,900,700);region(logicalRegion,800,600);
  put(physicalRegion+0x60,members);put(physicalRegion+0x68,uint32_t(2));put(physicalRegion+0x6c,uint32_t(2));
  put(members,std::array<uint16_t,2>{10,11});
  put(module+0x0592d128,root);put(root+0x608,machine);put(machine+0x120,world);
  put(machine+0xb0,provinces);put(machine+0xb8,uint32_t(4));put(machine+0xbc,uint32_t(4));
  put(provinces+16,province);put(province,module+0x04437a00);put(province+0x18,state);
  put(module+0x058fccf0,map);put(map,module+0x0441c938);put(map+0x2a0,definitions);
  put(map+0x2a8,uint32_t(32));put(map+0x2ac,uint32_t(32));
  put(definitions+80,seaPhysical);put(definitions+160,seaLogical);
  put(seaPhysical,module+0x044dc488);put(seaPhysical+0xc8,physicalRegion);
  put(seaLogical,module+0x044dc488);put(seaLogical+0xc8,logicalRegion);
  put(world+0x5618,module+0x044bb380);fill(world+0x5680,24);put(world+0x5680,roster);
  put(world+0x5688,uint32_t(1));put(world+0x568c,uint32_t(1));put(roster,connection);
  fill(connection,0x38);put(connection,module+0x044e08d8);put(connection+8,uint32_t(0xf0000013));
  put(connection+0x18,definition);put(connection+0x20,uint32_t(5));put(connection+0x28,landRuntime);put(connection+0x30,seaRuntime);
  fill(definition,0x60);put(definition,module+0x04515e20);put(definition+0x10,uint32_t(31));
  put(definition+0x40,landNode);put(definition+0x48,seaNode);put(definition+0x50,uint64_t(0x123456780003));put(definition+0x58,uint32_t(4));
  for(auto [node,runtime]:{std::pair{landNode,landRuntime},std::pair{seaNode,seaRuntime}}){
    fill(node,0x68);put(node,module+0x044f8a78);fill(runtime,0x80);put(runtime,module+0x044e3d78);put(runtime+0x78,node);
  }
  put(landNode+0x40,uint32_t(0x800108));put(landNode+0x44,uint16_t(2));put(landNode+0x58,stateRegion);put(landNode+0x60,uint64_t(0x400));
  put(seaNode+0x40,uint32_t(0x800200));put(seaNode+0x58,physicalRegion);put(seaNode+0x60,uint64_t(0x100));
  std::string why;auto copy=[&]{return DecodeNativePortApproaches(read,module,state,20,[&]{return live;},&why);};
  const auto stable=bytes;record=true;auto result=copy();record=false;
  if(!result)std::printf("reason=%s\n",why.c_str());
  Check(result && result->state==state && result->market==7 && result->eligibleShores==1 && result->queries.size()==1,
      "selected full state binds actual typed physical port");
  const auto candidate=result->queries.front();
  Check(candidate.ownerState==state && candidate.stateDefinition==17 && candidate.stateGeo==42 &&
      candidate.seaDefinition==900 && candidate.seaGeo==700 && candidate.logicalProvince==20 && candidate.physicalProvince==10 &&
      candidate.spline==0x123456780003 && candidate.stateAnchor==0x800108,
      "runtime state, engine definition, geo, canonical province and key remain distinct");
  for(uint64_t at:dependencies){bytes=stable;bytes.erase(at);Check(!copy(),"every observed unreadable dependency rejected");}
  unsigned badCase=0;
  auto bad=[&](auto change){bytes=stable;change();++badCase;auto value=copy();if(value)std::printf("unexpected valid case=%u\n",badCase);Check(!value,"malformed or stale port input rejected");};
  bad([&]{put(stateObject+8,uint32_t(1));});bad([&]{put(country+0xa24,UINT32_MAX);});
  bad([&]{put(map,module+0x0441c930);});bad([&]{put(map+0x2ac,uint32_t(33));});
  bad([&]{put(world+0x568c,uint32_t(2));});bad([&]{put(world+0x5680,UINT64_MAX-3);});
  bad([&]{put(connection+0x20,uint32_t(8));});bad([&]{put(definition,module+0x04515e28);});
  bad([&]{put(definition+0x40,UINT64_MAX-4);});bad([&]{put(landNode+0x60,uint64_t(2));});
  bad([&]{put(seaRuntime+0x78,landNode);});bad([&]{put(landNode+0x44,uint16_t(4));});
  bad([&]{put(province+0x18,uint32_t(0xcd000001));});
  bad([&]{put(province+0x18,owner);put(otherState+0xe78,logicalRegion);});
  bad([&]{put(physicalRegion+0x68,uint32_t(1));});bad([&]{put(physicalRegion+0x60,UINT64_MAX-1);});
  bad([&]{put(members,uint16_t(11));});bad([&]{put(seaPhysical+0xc8,logicalRegion);});
  bad([&]{put(physicalRegion+0x1c8,uint8_t(1));});bad([&]{put(definition+0x50,uint64_t(0x123456780002));});
  bytes=stable;put(seaLogical+0xc8,physicalRegion);auto connected=copy();
  Check(connected && connected->eligibleShores==1 && connected->queries.empty(),"same current sea region needs no approach query");
  bytes=stable;put(province+0x18,owner);auto split=copy();
  Check(split && split->queries.size()==1 && split->queries[0].ownerState==owner,
      "same-market split owner is preserved rather than remapped to the selected state");
  put(otherCountry+0xa24,uint32_t(9));auto foreign=copy();
  Check(foreign && foreign->eligibleShores==0 && foreign->foreignMarketShores==1 && foreign->queries.empty(),
      "foreign-market port in the same region never becomes an approach request");
  bytes=stable;
  auto clone=[&](uint64_t to,uint64_t from,size_t n){for(size_t i=0;i<n;++i)bytes[to+i]=bytes.at(from+i);};
  constexpr uint64_t connection2=0x50000,definition2=0x51000,land2=0x52000,sea2=0x53000,landRun2=0x54000,seaRun2=0x55000;
  clone(connection2,connection,0x38);clone(definition2,definition,0x60);clone(land2,landNode,0x68);clone(sea2,seaNode,0x68);
  clone(landRun2,landRuntime,0x80);clone(seaRun2,seaRuntime,0x80);
  put(connection2+8,uint32_t(0xf0000014));put(connection2+0x18,definition2);put(connection2+0x28,landRun2);put(connection2+0x30,seaRun2);
  put(definition2+0x10,uint32_t(32));put(definition2+0x40,land2);put(definition2+0x48,sea2);put(definition2+0x50,uint64_t(0x123456780043));
  put(landRun2+0x78,land2);put(seaRun2+0x78,sea2);put(land2+0x40,uint32_t(0x800109));put(sea2+0x40,uint32_t(0x800201));
  put(world+0x5688,uint32_t(2));put(world+0x568c,uint32_t(2));put(roster+8,connection2);
  auto multiple=copy();Check(multiple && multiple->eligibleShores==2 && multiple->queries.size()==2 &&
      multiple->queries[0].spline!=multiple->queries[1].spline,"multiple physical ports remain distinct; no first-candidate selection");
  put(connection2+8,uint32_t(0xf0000013));Check(!copy(),"duplicate current connection identities rejected");
  bytes=stable;region(0x48000,17,42);put(landNode+0x58,uint64_t(0x48000));Check(!copy(),"duplicate region identities do not alias by ID/geo");
  bytes=stable;put(entries+8,stateObject);put(stateObject+8,uint32_t(0));put(country+0xa24,uint32_t(0));put(province+0x18,uint32_t(0));
  auto zero=DecodeNativePortApproaches(read,module,0,20,[&]{return live;});Check(zero && zero->market==0 && zero->queries[0].ownerState==0,"legal zero state and market preserved");
  bytes=stable;put(stateObject+0x48,UINT32_MAX);put(otherState+0x48,uint32_t(123));put(province+0x18,owner);
  auto grouping=copy();Check(grouping && grouping->market==7 && grouping->queries.size()==1,
      "unrelated state grouping cannot select or split a native market");
  bytes=stable;unsigned seen=0;GoodsMemoryReader changing=[&](uint64_t at,void* p,size_t n){
    if(at==landNode && ++seen==2)put(landNode+0x48,int64_t(1000));return read(at,p,n);
  };
  Check(!DecodeNativePortApproaches(changing,module,state,20,[&]{return live;}),"same-key metadata changes during copy rejected");
  bytes=stable;unsigned observations=0;
  Check(!DecodeNativePortApproaches(read,module,state,20,[&]{return ++observations==1;}),"retirement before publication rejected");
  live=false;Check(!copy(),"retired caller rejected");live=true;
  Check(!DecodeNativePortApproaches(read,UINT64_MAX,state,20,[&]{return true;}) &&
      !DecodeNativePortApproaches(read,module,UINT32_MAX,20,[&]{return true;}) &&
      !DecodeNativePortApproaches(read,module,state,0,[&]{return true;}),"invalid or overflowing request rejected");
  bytes.clear();Check(result->queries[0]==candidate,"port metadata owns values after source destruction");
  std::puts("PASS: current typed port approaches, split-state owner/market isolation, canonical sea, mutation, retirement and owned values");
}
