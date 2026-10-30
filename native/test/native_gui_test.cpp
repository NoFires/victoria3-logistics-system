#include "native_gui.h"
#include "stored_zip.h"
#include "editable_resources.h"
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
  const auto sample=v3::BuildStoredZip({{"a/b.txt",{1,2,3}}});
  auto unpacked=v3::ReadStoredZip(sample);
  assert(unpacked.size()==1 && unpacked[0].name=="a/b.txt" && unpacked[0].bytes==std::vector<uint8_t>({1,2,3}));
  auto corrupt=sample;corrupt[30+7]^=1;Reject([&]{v3::ReadStoredZip(corrupt);});
  Reject([]{v3::ReadStoredZip({0,1,2});});
  const auto editable=std::wstring(argv[3])+L"\\editable-"+std::to_wstring(GetCurrentProcessId());
  const std::vector<v3::ArchiveFile> first{{"gui/example.gui",{'a'}}},updated{{"gui/example.gui",{'b'}}},latest{{"gui/example.gui",{'c'}}};
  assert(v3::EditableResources(editable,first)[0].bytes==first[0].bytes);
  assert(v3::EditableResources(editable,updated)[0].bytes==updated[0].bytes);
  v3::WriteResourceFile(editable+L"\\gui\\example.gui",{'p'});
  assert(v3::EditableResources(editable,latest)[0].bytes==std::vector<uint8_t>({'p'}));
  std::vector<uint8_t> baseline;
  assert(v3::ReadFileBytes(editable+L"\\.defaults\\gui\\example.gui",&baseline) && baseline==latest[0].bytes);
  DeleteFileW((editable+L"\\gui\\example.gui").c_str());
  assert(v3::EditableResources(editable,latest)[0].bytes==latest[0].bytes);
  Reject([&]{v3::EditableResources(editable,{{"../outside",{1}}});});
  const auto module=LoadLibraryExW(argv[1],nullptr,LOAD_LIBRARY_AS_DATAFILE);
  assert(module);
  const auto assetResource=FindResourceW(module,MAKEINTRESOURCEW(151),RT_RCDATA);
  const auto assetHandle=LoadResource(module,assetResource);
  const auto assetBytes=static_cast<const uint8_t*>(LockResource(assetHandle));
  const auto assets=v3::ReadStoredZip({assetBytes,assetBytes+SizeofResource(module,assetResource)});
  assert(assets.size()==2 && assets[0].name=="gfx/models/military/ironroad_arrows.asset" && assets[1].name=="gfx/FX/ironroad_building_flow.shader");
  const auto editableAssets=v3::EditableResources(editable+L"\\assets",assets);
  assert(editableAssets[1].bytes==assets[1].bytes);
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
  const auto ownPath=std::wstring(argv[3])+L"\\native-gui\\gui\\ironroad_logistics.gui";
  std::vector<uint8_t> own;
  assert(v3::ReadFileBytes(ownPath,&own));
  auto edited=own;const std::string comment="\n# persistent player edit\n";
  edited.insert(edited.end(),comment.begin(),comment.end());v3::WriteResourceFile(ownPath,edited);
  assert(v3::PrepareNativeGui(module,argv[2],argv[3],&again,&error) && again!=archive);
  std::wstring customAgain;
  assert(v3::PrepareNativeGui(module,argv[2],argv[3],&customAgain,&error) && customAgain==again);
  std::vector<uint8_t> retained;assert(v3::ReadFileBytes(ownPath,&retained) && retained==edited);
  auto broken=edited;broken.push_back('{');v3::WriteResourceFile(ownPath,broken);
  assert(!v3::PrepareNativeGui(module,argv[2],argv[3],&again,&error) && again.empty());
  assert(v3::ReadFileBytes(ownPath,&retained) && retained==broken);
  v3::WriteResourceFile(ownPath,own);
  assert(v3::PrepareNativeGui(module,argv[2],argv[3],&again,&error) && again==archive);
  std::string missingType(own.begin(),own.end());
  const std::string required="type ironroad_state_filters =";
  const auto typeAt=missingType.find(required);assert(typeAt!=std::string::npos);
  missingType.replace(typeAt,required.size(),"type player_state_filters =");
  missingType+="\n# type ironroad_state_filters = widget {}\n";
  missingType+="types player_text { type player_note = textbox { raw_text = \"type ironroad_state_filters = widget {}\" } }\n";
  const std::vector<uint8_t> missing(missingType.begin(),missingType.end());
  v3::WriteResourceFile(ownPath,missing);
  assert(!v3::PrepareNativeGui(module,argv[2],argv[3],&again,&error) && again.empty());
  assert(error.find("ironroad_state_filters")!=std::string::npos);
  assert(v3::ReadFileBytes(ownPath,&retained) && retained==missing);
  v3::WriteResourceFile(ownPath,own);
  auto duplicate=own;const std::string duplicateType="\ntypes duplicate_player { type ironroad_state_filters = widget {} }\n";
  duplicate.insert(duplicate.end(),duplicateType.begin(),duplicateType.end());v3::WriteResourceFile(ownPath,duplicate);
  assert(!v3::PrepareNativeGui(module,argv[2],argv[3],&again,&error) && again.empty());
  assert(error.find("ironroad_state_filters")!=std::string::npos);
  assert(v3::ReadFileBytes(ownPath,&retained) && retained==duplicate);
  v3::WriteResourceFile(ownPath,own);
  assert(v3::PrepareNativeGui(module,argv[2],argv[3],&again,&error) && again==archive);
  assert(!v3::PrepareNativeGui(GetModuleHandleW(nullptr),argv[2],argv[3],&again,&error));
  assert(again.empty() && error.find("resource missing")!=std::string::npos);
  FreeLibrary(module);
  std::cout<<v3::Narrow(archive)<<'\n';
  return 0;
}
