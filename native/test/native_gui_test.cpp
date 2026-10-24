#include "native_gui.h"
#include "stored_zip.h"
#include "util.h"
#include <cassert>
#include <iostream>
#include <stdexcept>

template<class F> void Reject(F f) {
  bool rejected=false;
  try { f(); } catch(const std::runtime_error&) { rejected=true; }
  assert(rejected);
}
int wmain(int argc,wchar_t** argv) {
  assert(argc==4);
  assert(v3::Sha256Bytes(nullptr,0)=="e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
  assert(v3::Sha256Bytes("abc",3)=="ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
  Reject([]{v3::BuildStoredZip({});});
  for(const std::string& name:std::vector<std::string>{"/root","../root","x\\y","a:b",std::string("x\0y",3)})
    Reject([&]{v3::BuildStoredZip({{name,{1}}});});
  Reject([]{v3::BuildStoredZip({{"same",{1}},{"same",{2}}});});
  const auto module=LoadLibraryExW(argv[1],nullptr,LOAD_LIBRARY_AS_DATAFILE);
  assert(module);
  std::vector<uint8_t> source;
  assert(v3::ReadFileBytes(std::wstring(argv[2])+L"\\game\\gui\\market_panel.gui",&source));
  auto bad=source;bad.push_back('\n');
  std::string patched,error;
  assert(!v3::PatchNativeMarketGui(bad,"{}",&patched,&error));
  assert(patched.empty() && error.find("differs from verified")!=std::string::npos);
  assert(!v3::PatchNativeMarketGui(source,"",&patched,&error));
  std::vector<uint8_t> stateSource;
  assert(v3::ReadFileBytes(std::wstring(argv[2])+L"\\game\\gui\\states_panel.gui",&stateSource));
  bad=stateSource;bad.push_back('\n');patched="old state GUI";
  assert(!v3::PatchNativeStateGui(bad,&patched,&error));
  assert(patched.empty() && error.find("differs from verified")!=std::string::npos);
  assert(!v3::PatchNativeStateGui(stateSource,nullptr,&error));
  assert(v3::PatchNativeStateGui(stateSource,&patched,&error));
  assert(patched.find("ironroad_state_freight_cell = {}")!=std::string::npos);
  std::wstring archive,again;
  if(!v3::PrepareNativeGui(module,argv[2],argv[3],&archive,&error)) {
    std::cerr<<error<<'\n';return 1;
  }
  assert(v3::PrepareNativeGui(module,argv[2],argv[3],&again,&error) && archive==again);
  assert(!v3::PrepareNativeGui(GetModuleHandleW(nullptr),argv[2],argv[3],&again,&error));
  assert(again.empty() && error.find("resource missing")!=std::string::npos);
  FreeLibrary(module);
  std::cout<<v3::Narrow(archive)<<'\n';
  return 0;
}
