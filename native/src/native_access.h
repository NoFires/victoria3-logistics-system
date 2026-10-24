#pragma once
#include "goods_catalog.h"
#include <optional>

namespace v3 {
// Diagnostic observation only. Sequential reads do not grant a game-access
// lease, prove that tasks have joined, or authorize copying on another thread.
struct NativeAccessObservation {
  uint32_t access=0,hidden=0,context=0,date=0,phase=0;
  uint8_t owns=0,paused=0;
  uint64_t machine=0,world=0,stateDb=0;
};
inline std::optional<NativeAccessObservation> ObserveNativeAccess(
    const GoodsMemoryReader& read,uint64_t module,uint64_t tlsArray) {
  if(!read || !module || module>UINT64_MAX-0x05964ef8 || !tlsArray)return {};
  auto field=[&](uint64_t p,uint64_t off,auto& value){
    return p && off<=UINT64_MAX-p && sizeof(value)<=UINT64_MAX-(p+off) &&
        read(p+off,&value,sizeof(value));
  };
  NativeAccessObservation result;uint64_t tls=0,root=0,type=0,idler=0,idlerType=0;
  if(!field(tlsArray,0,tls) || !field(tls,0x5ae0,result.owns) || result.owns>1 ||
     !field(tls,0x92c,result.context) ||
     !field(module,0x0514e580,result.access) || result.access>3 ||
     !field(module,0x0514e570,result.hidden) || result.hidden>1 ||
     !field(module,0x0592d0e0,idler) || !field(idler,0,idlerType) || idlerType!=module+0x04431500 ||
     !field(idler,0x20,result.paused) || result.paused>1 ||
     !field(module,0x0592d128,root) || !field(root,0,type) || type!=module+0x04430800 ||
     !field(root,0x588,result.phase) ||
     !field(root,0x608,result.machine) || !field(result.machine,8,result.date) ||
     !field(result.machine,0x120,result.world) || !result.world ||
     !field(module,0x05964ef0,result.stateDb) || !result.stateDb)return {};
  return result;
}
} // namespace v3
