#include "state_panel.h"
#include "state_region.h"
#include <cstdio>
#include <cstdlib>
using namespace v3;
void Check(bool ok,const char* why){if(!ok){std::printf("FAIL: %s\n",why);std::exit(1);}}
struct Fixture {
  static constexpr uint64_t module=0x140000000,root=0x10000,gfx=0x11000,machine=0x12000,
      handler=0x13000,panels=0x14000,panel=0x15000,history=0x16000,key=0x17000;
  std::array<uint8_t,0x8000> bytes{};
  uint64_t global=root;
  template<class T>void Put(uint64_t a,T value){std::memcpy(bytes.data()+a-root,&value,sizeof(value));}
  void Text(uint64_t object,uint64_t offset,const char* value){
    uint64_t n=std::strlen(value);Put(object+offset+16,n);Put(object+offset+24,uint64_t(n<16?15:63));
    if(n<16)std::memcpy(bytes.data()+object+offset-root,value,n+1);
    else{Put(object+offset,key);std::memcpy(bytes.data()+key-root,value,n+1);}
  }
  Fixture(){
    Put(root,module+kRvaInterfaceIdlerVtable);Put(root+0x18,machine);Put(machine+0x588,int32_t(4));
    Put(root+0x10,gfx);Put(gfx,module+kRvaInterfaceGfxVtable);Put(gfx+0x20,handler);
    Put(handler,module+kRvaInterfaceHandlerVtable);Put(handler+0xd8,handler+0xd8);
    Put(handler+0xe0,panels);Put(handler+0xec,int32_t(56));Put(panels+14*8,panel);
    Put(panel,module+0x045CC848);Text(panel,8,"states_panel");Text(panel,0x30,"default");
    Put(panel+0x70,uint32_t(UINT32_MAX));Put(handler+0xd8+0x40,uint8_t(1));
    Put(handler+0xd8+0x20,history);
  }
  void Open(uint32_t state=17){
    Put(panel+0x70,state);Text(panel,0x30,"local_goods");
    Put(handler+0xd8+0x40,uint8_t(0));Put(handler+0xd8+0x2c,int32_t(2));
    Put(history,int32_t(9));Put(history+0x30,int32_t(14));
  }
  bool Read(uint64_t a,void* out,size_t n)const{
    if(a==module+kRvaInterfaceIdlerGlobal && n==8){std::memcpy(out,&global,8);return true;}
    if(a<root || a-root>bytes.size() || n>bytes.size()-(a-root))return false;
    std::memcpy(out,bytes.data()+a-root,n);return true;
  }
  auto Decode()const{return DecodeNativeStatePanel([&](auto a,auto p,auto n){return Read(a,p,n);},module);}
};
int main(){
  Fixture f;auto s=f.Decode();Check(s.available && s.ingame && !s.stateOpen && s.state==UINT32_MAX,"never-opened state is closed, not state zero");
  f.Open();s=f.Decode();Check(s.available && s.stateOpen && s.state==17 && s.tab=="local_goods","native history kind14 and current local-goods tab");
  f.Open(0);Check(f.Decode().stateOpen && f.Decode().state==0,"explicit state zero remains a valid copied ID");
  f.Open(19);Check(f.Decode().state==19,"state selection replaces retained identity");
  f.Put(Fixture::history+0x30,int32_t(9));s=f.Decode();Check(s.available && !s.stateOpen,"earlier history or retained tab cannot override market panel");
  f.Open();f.Put(Fixture::handler+0xd8+0x40,uint8_t(1));Check(!f.Decode().stateOpen,"closed flag wins over history");
  f.Open();f.Text(Fixture::panel,0x30,"buildings");s=f.Decode();Check(s.stateOpen && s.tab=="buildings","open state and selected tab stay distinct");
  f=Fixture();f.Put(Fixture::machine+0x588,int32_t(1));f.Put(Fixture::root+0x10,uint64_t(0));
  s=f.Decode();Check(s.available && !s.ingame && !s.stateOpen,"menu phase never follows stale gfx");
  f=Fixture();f.Put(Fixture::handler,uint64_t(123));Check(!f.Decode().available,"wrong handler type rejected");
  f=Fixture();f.Put(Fixture::handler+0xd8,uint64_t(0));Check(!f.Decode().available,"wrong bar ownership rejected");
  f=Fixture();f.Put(Fixture::panels+14*8,uint64_t(UINT64_MAX-3));Check(!f.Decode().available,"overflowing panel pointer rejected");
  f=Fixture();f.Put(Fixture::panel,Fixture::module+kRvaMarketPanelVtable);Check(!f.Decode().available,"market object cannot masquerade as state panel");
  f=Fixture();f.Text(Fixture::panel,8,"market_panel");Check(!f.Decode().available,"panel name checked independently of vtable");
  f=Fixture();f.Open(UINT32_MAX);s=f.Decode();Check(!s.available && s.state==UINT32_MAX && s.tab.empty(),"invalid open ID clears partial output");
  for(int32_t count:{-1,1000001}){f=Fixture();f.Put(Fixture::handler+0xd8+0x2c,count);Check(!f.Decode().available,"unbounded history rejected");}
  f=Fixture();f.Open();f.Put(Fixture::history+0x30,int32_t(56));Check(!f.Decode().available,"invalid history kind rejected");
  f=Fixture();f.Open();f.Put(Fixture::panel+0x30+24,uint64_t(2));Check(!f.Decode().available,"invalid string capacity rejected");
  f=Fixture();f.Open();f.Text(Fixture::panel,0x30,"long_local_goods_tab");s=f.Decode();Check(s.available && s.tab=="long_local_goods_tab","heap tab copied into owned value");
  int reads=0;
  s=DecodeNativeStatePanel([&](auto a,auto p,auto n){
    if(a==Fixture::key && ++reads==2)f.bytes[Fixture::key-Fixture::root]='x';
    return f.Read(a,p,n);
  },Fixture::module);Check(!s.available && !s.stateOpen,"heap mutation during copy rejected");
  for(uint64_t address:{Fixture::module+kRvaInterfaceIdlerGlobal,Fixture::panel+0x70,Fixture::history+0x30}){
    f=Fixture();f.Open();reads=0;
    s=DecodeNativeStatePanel([&](auto a,auto p,auto n){
      if(a==address && ++reads==2)return false;
      return f.Read(a,p,n);
    },Fixture::module);Check(!s.available && s.state==UINT32_MAX,"root/selection/history reread failure rejects whole result");
  }
  f=Fixture();f.Open();reads=0;
  s=DecodeNativeStatePanel([&](auto a,auto p,auto n){
    if(a==Fixture::panel+0x70 && ++reads==2)f.Put(Fixture::panel+0x70,uint32_t(19));
    return f.Read(a,p,n);
  },Fixture::module);Check(!s.available && s.tab.empty(),"state switch during copy cannot publish old tab with new ID");
  f=Fixture();f.Open();reads=0;
  s=DecodeNativeStatePanel([&](auto a,auto p,auto n){
    if(a==Fixture::module+kRvaInterfaceIdlerGlobal && ++reads==2)f.global=0;
    return f.Read(a,p,n);
  },Fixture::module);Check(!s.available && !s.ingame,"world interface replacement rejects copied state");
  Check(!DecodeNativeStatePanel({},Fixture::module).available,"missing reader rejected");
  Check(!DecodeNativeStatePanel([](auto,auto,auto){return true;},UINT64_MAX).available,"invalid module rejected before reads");
  f=Fixture();
  constexpr auto nativeState=Fixture::machine,definition=Fixture::panel;
  f.Put(nativeState+8,uint32_t(433));f.Put(nativeState+0xe78,definition);
  f.Put(definition,Fixture::module+0x044E7850);f.Put(definition+0x10,uint32_t(17));
  f.Text(definition,0x18,"STATE_MIDLANDS");
  auto regionKey=[&](){return DecodeNativeStateRegionKey([&](auto a,auto p,auto n){return f.Read(a,p,n);},Fixture::module,nativeState,433);};
  Check(regionKey()=="STATE_MIDLANDS","runtime state433 and definition17 resolve a key, not guessed map ID433 or17");
  f.Put(nativeState+8,uint32_t(434));Check(!regionKey(),"wrong native state identity rejects region key");
  f.Put(nativeState+8,uint32_t(433));f.Put(definition,Fixture::module+kRvaMarketPanelVtable);
  Check(!regionKey(),"wrong region definition type rejected");
  f.Put(definition,Fixture::module+0x044E7850);f.Text(definition,0x18,"STATE_CUSTOM_MOD_LONG_REGION");
  Check(regionKey()=="STATE_CUSTOM_MOD_LONG_REGION","custom map keys and heap strings do not rely on vanilla numeric IDs");
  reads=0;
  auto changedKey=DecodeNativeStateRegionKey([&](auto a,auto p,auto n){
    if(a==Fixture::key && ++reads==2)f.bytes[Fixture::key-Fixture::root]='X';
    return f.Read(a,p,n);
  },Fixture::module,nativeState,433);
  Check(!changedKey,"heap region key changes during copy reject the mapping");
  f.Text(definition,0x18,"STATE_BAD-KEY");Check(!regionKey(),"invalid state region key rejected");
  std::puts("PASS native state panel copied identity, lifecycle and native-state/region-key separation with mutation rejection (offline)");
}
