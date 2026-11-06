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
      marketDb=0x60000,marketEntries=0x61000,market=0x62000,otherMarket=0x63000,zeroMarket=0x64000,physicalNode=0x48100,landProvinceDefinition=0x35100;
  constexpr uint64_t shoreKey=(uint64_t(0x800200)<<35)|(uint64_t(0x800108)<<6)|3;
  constexpr uint32_t state=0xab000001,owner=0xcd000002;
  std::map<uint64_t,uint8_t> bytes;std::set<uint64_t> dependencies;bool record=false,live=true;
  auto put=[&](uint64_t at,auto value){for(size_t i=0;i<sizeof(value);++i)bytes[at+i]=((uint8_t*)&value)[i];};
  auto fill=[&](uint64_t at,size_t n){for(size_t i=0;i<n;++i)bytes[at+i]=0;};
  GoodsMemoryReader read=[&](uint64_t at,void* p,size_t n){
    for(size_t i=0;i<n;++i){if(record)dependencies.insert(at+i);auto found=bytes.find(at+i);if(found==bytes.end())return false;((uint8_t*)p)[i]=found->second;}return true;
  };
  auto region=[&](uint64_t at,uint32_t id,uint32_t geo){
    fill(at,0x1d0);put(at,module+0x046ba948);put(at+0x10,id);put(at+0x40,geo);put(at+0x44,uint8_t(1));
  };
  put(module+0x05bd69c8,db);put(db+0x20,entries);put(db+0x2c,uint32_t(4));fill(entries,72);
  for(auto [object,id]:{std::pair{stateObject,state},std::pair{otherState,owner}}){
    put(entries+8+uint64_t(id&0xffffff)*16,object);put(object,module+0x046bad90);put(object+8,id);
    put(object+0x48,id);put(object+0xf10,stateRegion);
    put(object+0x1258,uint8_t(0));put(object+0xb78,UINT32_MAX);put(object+0xed8,uint32_t(id==state?0:1));
  }
  put(module+0x05c42c20,countryDb);put(countryDb+0x20,countryEntries);put(countryDb+0x2c,uint32_t(2));
  put(countryEntries+8,country);put(countryEntries+24,otherCountry);
  for(auto [object,id]:{std::pair{country,0u},std::pair{otherCountry,1u}}){
    put(object,module+0x0468fce0);put(object+0x18,id);put(object+0xaf4,uint32_t(7));
  }
  put(module+0x05c43518,marketDb);put(marketDb+0x20,marketEntries);put(marketDb+0x2c,uint32_t(10));
  for(auto [object,id]:{std::pair{market,7u},std::pair{otherMarket,9u},std::pair{zeroMarket,0u}}){
    put(marketEntries+8+id*16,object);put(object,module+0x046d4670);put(object+8,id);
  }
  region(stateRegion,17,42);region(physicalRegion,900,700);region(logicalRegion,800,600);
  put(physicalRegion+0x60,members);put(physicalRegion+0x68,uint32_t(2));put(physicalRegion+0x6c,uint32_t(2));
  put(members,std::array<uint16_t,2>{10,11});
  put(module+0x05c07c68,root);put(root,module+0x04660618);put(root+0x608,machine);put(machine+0x120,world);
  put(machine+0xb0,provinces);put(machine+0xb8,uint32_t(4));put(machine+0xbc,uint32_t(4));
  put(provinces+16,province);put(province,module+0x046ac1c0);put(province+0x18,state);put(province+8,landProvinceDefinition);
  put(landProvinceDefinition,module+0x046ab568);put(landProvinceDefinition+0xc8,stateRegion);
  put(module+0x05bd7308,map);put(map,module+0x046d6850);put(map+0x2a0,definitions);
  put(map+0x2a8,uint32_t(32));put(map+0x2ac,uint32_t(32));
  put(definitions+80,seaPhysical);put(definitions+160,seaLogical);
  put(seaPhysical,module+0x046ab568);put(seaPhysical+0xc8,physicalRegion);
  put(seaLogical,module+0x046ab568);put(seaLogical+0xc8,logicalRegion);
  auto array=[&](uint64_t at,uint64_t data,uint32_t count){fill(at,24);put(at,data);put(at+8,count);put(at+12,count);};
  auto node=[&](uint64_t at,uint64_t def,uint32_t id,uint16_t roles,uint16_t province,uint32_t control){
    fill(at,0x30);fill(def,0x20);put(at,id);put(at+6,province);put(at+8,def);
    put(def,id);put(def+4,roles);put(def+8,control);put(def+12,province);
  };
  auto link=[&](uint64_t at,uint64_t def,uint32_t id,uint8_t kind,uint64_t first,uint64_t second,uint64_t firstDef,uint64_t secondDef,uint64_t key){
    fill(at,0x40);fill(def,0x28);put(at,id);put(at+4,uint8_t(5));put(at+8,def);put(at+0x10,first);put(at+0x18,second);
    put(def,id);put(def+4,kind);put(def+8,firstDef);put(def+0x10,secondDef);put(def+0x20,key);
  };
  put(world+0x5028,module+0x046b1d48);array(world+0x5038,seaRuntime,1);array(world+0x5050,0,0);
  array(world+0x5068,landRuntime,2);array(world+0x5080,connection,2);
  node(landRuntime,landNode,0,0x200,10,0x800108);node(landRuntime+0x30,seaNode,1,0x80,10,0x800200);
  node(seaRuntime,physicalNode,0,0xc02,2,0x108);
  link(connection,definition,0,5,landRuntime,landRuntime+0x30,landNode,seaNode,shoreKey);
  link(connection+0x40,definition+0x40,1,6,landRuntime,seaRuntime,landNode,physicalNode,UINT64_MAX);
  std::string why;auto copy=[&]{return DecodeNativePortApproaches(read,module,state,20,[&]{return live;},&why);};
  const auto stable=bytes;record=true;auto result=copy();record=false;
  if(!result)std::printf("reason=%s\n",why.c_str());
  Check(result && result->state==state && result->market==7 && result->eligibleShores==1 && result->queries.size()==1,
      "selected full state binds actual typed physical port");
  const auto candidate=result->queries.front();
  Check(candidate.ownerState==state && candidate.stateDefinition==17 && candidate.stateGeo==42 &&
      candidate.seaDefinition==900 && candidate.seaGeo==700 && candidate.logicalProvince==20 && candidate.physicalProvince==10 &&
      candidate.spline==shoreKey && candidate.stateAnchor==0x800108,
      "runtime state, engine definition, geo, canonical province and key remain distinct");
  for(uint64_t at:dependencies){bytes=stable;bytes.erase(at);Check(!copy(),"every observed unreadable dependency rejected");}
  unsigned badCase=0;
  auto bad=[&](auto change){bytes=stable;change();++badCase;auto value=copy();if(value)std::printf("unexpected valid case=%u\n",badCase);Check(!value,"malformed or stale port input rejected");};
  bad([&]{put(stateObject+8,uint32_t(1));});bad([&]{put(country+0xaf4,UINT32_MAX);});
  bad([&]{put(map,module+0x046d6858);});bad([&]{put(map+0x2ac,uint32_t(33));});
  bad([&]{put(world+0x508c,uint32_t(3));});bad([&]{put(world+0x5080,UINT64_MAX-3);});
  bad([&]{put(connection+4,uint8_t(7));});bad([&]{put(definition,uint32_t(1));});
  bad([&]{put(definition+8,UINT64_MAX-4);});bad([&]{put(landNode+4,uint16_t(2));});
  bad([&]{put(landRuntime+0x38,landNode);});bad([&]{put(physicalNode+12,uint16_t(4));});
  bad([&]{put(province+0x18,uint32_t(0xcd000001));});
  bad([&]{put(province+0x18,owner);put(otherState+0xf10,logicalRegion);});
  bad([&]{put(physicalRegion+0x68,uint32_t(1));});bad([&]{put(physicalRegion+0x60,UINT64_MAX-1);});
  bad([&]{put(members,uint16_t(11));});bad([&]{put(seaPhysical+0xc8,logicalRegion);});
  bad([&]{put(physicalRegion+0x1c8,uint8_t(1));});bad([&]{put(definition+0x20,shoreKey^uint64_t(1));});
  bad([&]{put(physicalNode+4,uint16_t(1));});bad([&]{put(landNode+8,uint32_t(0x800109));});
  bytes=stable;put(seaLogical+0xc8,physicalRegion);auto connected=copy();
  Check(connected && connected->eligibleShores==1 && connected->queries.empty(),"same current sea region needs no approach query");
  bytes=stable;put(province+0x18,owner);auto split=copy();
  Check(split && split->queries.size()==1 && split->queries[0].ownerState==owner,
      "same-market split owner is preserved rather than remapped to the selected state");
  put(otherCountry+0xaf4,uint32_t(9));auto foreign=copy();
  Check(foreign && foreign->eligibleShores==0 && foreign->foreignMarketShores==1 && foreign->queries.empty(),
      "foreign-market port in the same region never becomes an approach request");
  bytes=stable;
  array(world+0x5038,seaRuntime,2);array(world+0x5068,landRuntime,4);array(world+0x5080,connection,4);
  constexpr uint64_t virtual2=0x52000,water2=0x53000,physical2=0x54000,shore2=0x55000,access2=0x56000;
  node(landRuntime+0x60,virtual2,2,0x200,10,0x800109);node(landRuntime+0x90,water2,3,0x80,10,0x800201);
  node(seaRuntime+0x30,physical2,1,0x402,2,0x109);
  const uint64_t key2=(uint64_t(0x800201)<<35)|(uint64_t(0x800109)<<6)|3;
  link(connection+0x80,shore2,2,5,landRuntime+0x60,landRuntime+0x90,virtual2,water2,key2);
  link(connection+0xc0,access2,3,6,landRuntime+0x60,seaRuntime+0x30,virtual2,physical2,UINT64_MAX);
  auto multiple=copy();Check(multiple && multiple->eligibleShores==2 && multiple->queries.size()==2 &&
      multiple->queries[0].spline!=multiple->queries[1].spline,"multiple physical ports remain distinct; no first-candidate selection");
  put(connection+0x80,uint32_t(0));Check(!copy(),"duplicate array-local connection identities rejected");
  bytes=stable;region(0x48000,17,42);put(landProvinceDefinition+0xc8,uint64_t(0x48000));Check(!copy(),"duplicate region identities do not alias by ID/geo");
  bytes=stable;put(entries+8,stateObject);put(stateObject+8,uint32_t(0));put(country+0xaf4,uint32_t(0));put(province+0x18,uint32_t(0));
  auto zero=DecodeNativePortApproaches(read,module,0,20,[&]{return live;});Check(zero && zero->market==0 && zero->queries[0].ownerState==0,"legal zero state and market preserved");
  bytes=stable;put(stateObject+0x48,UINT32_MAX);put(otherState+0x48,uint32_t(123));put(province+0x18,owner);
  auto grouping=copy();Check(grouping && grouping->market==7 && grouping->queries.size()==1,
      "unrelated state grouping cannot select or split a native market");
  bytes=stable;unsigned seen=0;GoodsMemoryReader changing=[&](uint64_t at,void* p,size_t n){
    if(at==landNode && ++seen==2)put(landNode+0x10,int64_t(1000));return read(at,p,n);
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
