#include "desktop_test_input.h"
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cstdio>

namespace fs=std::filesystem;
using namespace v3::desktop_test;
static void Check(bool value,const char* why){if(!value)throw std::runtime_error(why);}
static BOOL WINAPI UnusedClip(const RECT*){return FALSE;}
static LRESULT CALLBACK Procedure(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp){
  if(Message(hwnd,msg,wp,lp))return 0;
  return DefWindowProcW(hwnd,msg,wp,lp);
}
static std::wstring InputName(){
  HDESK input=OpenInputDesktop(0,FALSE,DESKTOP_READOBJECTS);
  wchar_t name[256]{};DWORD needed=0;
  const bool ok=input && GetUserObjectInformationW(input,UOI_NAME,name,sizeof(name),&needed);
  if(input)CloseDesktop(input);
  Check(ok,"input desktop observation failed");return name;
}
static int Child(const wchar_t* mode,const fs::path& report){
  std::ofstream out(report);
  try{
    Check(MH_Initialize()==MH_OK,"MinHook init failed");
    WNDCLASSW cls{};cls.hInstance=GetModuleHandleW(nullptr);cls.lpfnWndProc=Procedure;
    cls.lpszClassName=L"IronroadDesktopInputTest";Check(RegisterClassW(&cls),"test class failed");
    HWND hwnd=CreateWindowW(cls.lpszClassName,L"test",WS_OVERLAPPEDWINDOW,30,40,200,150,
        nullptr,nullptr,cls.hInstance,nullptr);
    Check(hwnd,"test window failed");
    const bool enabled=!wcscmp(mode,L"enabled");
    if(!wcscmp(mode,L"input") && InputName()==L"Default"){
      wcscpy(desktop,L"Default");
      Check(!Safe(hwnd),"the actual input desktop cannot authorize virtual input");
      desktop[0]=0;
    }
    Check(!Install(nullptr),"missing window must refuse test hooks");
    Check(Install(hwnd)==enabled,"desktop admission differs");
    const UINT request=RegisterWindowMessageW(kCaptureMessage);
    Check(request,"capture message registration failed");
    if(enabled){
      Check(MH_CreateHook((void*)GetProcAddress(GetModuleHandleW(L"user32.dll"),"ClipCursor"),
          (LPVOID)&UnusedClip,nullptr)==MH_ERROR_ALREADY_CREATED,
          "isolated mode must protect the shared clipping API before any cursor test");
      RECT clip{1,2,3,4};
      Check(!RejectClip(&clip) && GetLastError()==ERROR_NOT_SUPPORTED,"virtual clipping must refuse shared bounds");
      Check(!RejectClip(nullptr) && GetLastError()==ERROR_NOT_SUPPORTED,"virtual unclipping must not touch shared bounds");
      POINT p{};Check(!GetCursorPos(&p),"no physical pointer fallback before a virtual event");
      POINT expected{-15,23};Check(ClientToScreen(hwnd,&expected),"client transform failed");
      SendMessageW(hwnd,WM_MOUSEMOVE,0,MAKELPARAM(-15,23));
      Check(GetCursorPos(&p) && p.x==expected.x && p.y==expected.y,"signed virtual client position differs");
      Check(SetCursorPos(123456,-234567),"virtual warp refused");
      Check(GetCursorPos(&p) && p.x==123456 && p.y==-234567,"virtual warp lost full screen precision");
      SendMessageW(hwnd,request,kCaptureCookie^1,0);Check(!TakeCapture(),"wrong capture cookie accepted");
      SendMessageW(hwnd,request,kCaptureCookie,1);Check(!TakeCapture(),"unexpected capture payload accepted");
      SendMessageW(hwnd,request,kCaptureCookie,0);Check(TakeCapture() && !TakeCapture(),"capture must be one shot");
      HWND other=CreateWindowW(cls.lpszClassName,L"other",WS_OVERLAPPEDWINDOW,0,0,100,100,
          nullptr,nullptr,cls.hInstance,nullptr);Check(other,"second window failed");
      SendMessageW(other,WM_MOUSEMOVE,0,MAKELPARAM(1,2));
      SendMessageW(other,request,kCaptureCookie,0);
      Check(!TakeCapture() && GetCursorPos(&p) && p.x==123456,"unbound window changed test state");
      SendMessageW(hwnd,request,kCaptureCookie,0);
      Check(Install(other) && !TakeCapture() && !GetCursorPos(&p),"window rebind retained an old request/point");
      SendMessageW(hwnd,WM_MOUSEMOVE,0,MAKELPARAM(3,4));Check(!GetCursorPos(&p),"retired window resurrected pointer");
      SendMessageW(other,WM_MOUSEMOVE,0,MAKELPARAM(5,6));Check(GetCursorPos(&p),"new window point unavailable");
      DestroyWindow(other);
      Check(!GetCursorPos(&p) && !SetCursorPos(123,456),"destroyed window must never fall back to the real pointer");
      Check(!RejectClip(nullptr) && GetLastError()==ERROR_ACCESS_DENIED,"retired desktop clipping must stay refused");
    }else{
      SendMessageW(hwnd,request,kCaptureCookie,0);Check(!TakeCapture(),"disabled test mode accepted capture");
      Check(!active.load(),"refused desktop activated hooks");
    }
    MH_DisableHook(MH_ALL_HOOKS);Check(MH_Uninitialize()==MH_OK,"hook cleanup failed");
    DestroyWindow(hwnd);out<<"PASS";return 0;
  }catch(const std::exception& e){out<<e.what();return 1;}
}
static fs::path Self(){wchar_t path[32768]{};Check(GetModuleFileNameW(nullptr,path,32768),"self path failed");return path;}
static PROCESS_INFORMATION Spawn(const fs::path& exe,std::wstring args,const std::wstring& desktop){
  std::wstring command=L"\""+exe.wstring()+L"\" "+args;
  STARTUPINFOW startup{};startup.cb=sizeof(startup);startup.lpDesktop=desktop.empty()?nullptr:const_cast<wchar_t*>(desktop.c_str());
  PROCESS_INFORMATION process{};
  Check(CreateProcessW(exe.c_str(),command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,
      nullptr,exe.parent_path().c_str(),&startup,&process),"child launch failed");return process;
}
static DWORD Finish(PROCESS_INFORMATION& process){
  const DWORD wait=WaitForSingleObject(process.hProcess,15000);
  if(wait!=WAIT_OBJECT_0){
    // Only the just-created, source-owned test executable/harness uses this helper.
    TerminateProcess(process.hProcess,99);WaitForSingleObject(process.hProcess,5000);
  }
  DWORD code=99;GetExitCodeProcess(process.hProcess,&code);
  CloseHandle(process.hThread);CloseHandle(process.hProcess);process={};
  Check(wait==WAIT_OBJECT_0,"owned test child timed out");return code;
}
static void RunCase(const wchar_t* mode,bool hidden,const wchar_t* request){
  const auto input=InputName();
  std::wstring label=L"CodexFreightTest_"+std::to_wstring(GetCurrentProcessId())+L"_"+mode;
  HDESK desk=hidden?CreateDesktopW(label.c_str(),nullptr,nullptr,0,
      DESKTOP_CREATEWINDOW|DESKTOP_READOBJECTS|DESKTOP_WRITEOBJECTS|DESKTOP_ENUMERATE,nullptr):nullptr;
  Check(!hidden || desk,"test desktop creation failed");
  const std::wstring requested=request?request:label;
  SetEnvironmentVariableW(kEnvironment,!wcscmp(mode,L"absent")?nullptr:requested.c_str());
  wchar_t reportName[MAX_PATH]{};
  Check(GetTempFileNameW(Self().parent_path().c_str(),L"ird",0,reportName),"report creation failed");
  auto process=Spawn(Self(),L"--child "+std::wstring(mode)+L" \""+reportName+L"\"",
      hidden?L"WinSta0\\"+label:L"");
  const DWORD code=Finish(process);
  std::ifstream file{fs::path(reportName)};std::string report{std::istreambuf_iterator<char>(file),{}};file.close();
  DeleteFileW(reportName);if(desk)Check(CloseDesktop(desk),"test desktop close failed");
  Check(InputName()==input,"test changed input desktop");
  if(code || report!="PASS")throw std::runtime_error("desktop case failed: "+report);
  std::printf("PASS desktop test admission case: %ls\n",mode);
}
static void Runtime(const fs::path& dll){
  const auto input=InputName();const auto label=L"CodexFreightTest_"+std::to_wstring(GetCurrentProcessId())+L"_capture";
  HDESK desk=CreateDesktopW(label.c_str(),nullptr,nullptr,0,
      DESKTOP_CREATEWINDOW|DESKTOP_READOBJECTS|DESKTOP_WRITEOBJECTS|DESKTOP_ENUMERATE,nullptr);
  Check(desk,"runtime desktop failed");
  SetEnvironmentVariableW(kEnvironment,label.c_str());
  SetEnvironmentVariableW(L"IRONROAD_HARNESS_CAPTURE_PHASE",L"1");
  for(const auto key:{L"IRONROAD_DEBUG_UI",L"IRONROAD_READBACK",L"IRONROAD_NO_IMGUI",L"IRONROAD_NO_RENDER",L"IRONROAD_EMPTY_UI",L"IRONROAD_CLEAR_TEST"})
    SetEnvironmentVariableW(key,nullptr);
  const fs::path ppm=dll.parent_path()/L"freight/readback.ppm";
  const auto previous=fs::exists(ppm)?fs::last_write_time(ppm):fs::file_time_type::min();
  auto process=Spawn(Self().parent_path()/L"harness.exe",L"\""+dll.wstring()+L"\"",L"WinSta0\\"+label);
  Sleep(5300);
  struct Search {DWORD pid;HWND window;};Search search{process.dwProcessId,nullptr};
  EnumDesktopWindows(desk,[](HWND hwnd,LPARAM param)->BOOL{
    auto& s=*(Search*)param;DWORD pid=0;GetWindowThreadProcessId(hwnd,&pid);wchar_t kind[64]{};
    if(pid==s.pid && GetClassNameW(hwnd,kind,64) && !wcscmp(kind,L"IronroadHarness"))s.window=hwnd;
    return TRUE;
  },(LPARAM)&search);
  const bool posted=search.window && PostMessageW(search.window,RegisterWindowMessageW(kCaptureMessage),kCaptureCookie,0);
  const DWORD code=Finish(process);Check(CloseDesktop(desk),"runtime desktop close failed");
  Check(InputName()==input && posted && code==0,"isolated runtime did not finish normally");
  Check(fs::exists(ppm) && fs::last_write_time(ppm)>previous,"late GPU capture was not produced");
  std::ifstream image(ppm,std::ios::binary);std::string magic;int width=0,height=0,maximum=0;
  image>>magic>>width>>height>>maximum;image.get();
  Check(magic=="P6" && width==960 && height==960 && maximum==255,"capture header differs");
  image.seekg(std::streamoff((height/2*width+width/2)*3),std::ios::cur);unsigned char pixel[3]{};
  image.read((char*)pixel,3);
  Check(image.good() && pixel[0]>100 && pixel[1]<40 && pixel[2]<40,"capture is stale or includes an unexpected debug panel");
  std::printf("PASS isolated runtime: late GPU frame %dx%d pixel=%u/%u/%u, default ImGui hidden\n",
      width,height,pixel[0],pixel[1],pixel[2]);
}
int wmain(int argc,wchar_t** argv){
  if(argc==4 && !wcscmp(argv[1],L"--child"))return Child(argv[2],argv[3]);
  try{
    if(argc==3 && !wcscmp(argv[1],L"--runtime")){Runtime(fs::absolute(argv[2]));return 0;}
    Check(argc==1,"unknown test arguments");
    RunCase(L"absent",false,nullptr);
    RunCase(L"input",false,L"Default");
    RunCase(L"mismatch",true,L"CodexFreightTest_other");
    RunCase(L"malformed",true,L"unexpected");
    const std::wstring oversized(300,L'x');RunCase(L"oversized",true,oversized.c_str());
    RunCase(L"enabled",true,nullptr);
    return 0;
  }catch(const std::exception& e){std::fprintf(stderr,"FAIL desktop test: %s\n",e.what());return 1;}
}
