// Exact-build read-only diagnostic. Verify the EXE hash in PROFILE.md first.
// No attach, pause, writes, native getters or filesystem initialization.
#include "native_vfs.h"
#include <windows.h>
#include <tlhelp32.h>
#include <cstdio>

namespace {
HANDLE process=nullptr;
bool Read(uint64_t address,void* out,size_t size){
  SIZE_T got=0;
  return address && size<=UINT64_MAX-address &&
    ReadProcessMemory(process,(LPCVOID)address,out,size,&got) && got==size;
}
struct Window {DWORD pid=0,thread=0;unsigned count=0;};
BOOL CALLBACK FindWindow(HWND window,LPARAM parameter){
  auto& result=*(Window*)parameter;DWORD pid=0;
  DWORD thread=GetWindowThreadProcessId(window,&pid);wchar_t title[256]{};
  if(pid==result.pid && IsWindowVisible(window) &&
     GetWindowTextW(window,title,256) && !wcscmp(title,L"Victoria 3")){
    result.thread=thread;++result.count;
  }
  return TRUE;
}
struct ThreadBasicInfo {LONG status;void* teb;ULONG_PTR processId,threadId,affinity;LONG priority,basePriority;};
using QueryThread=LONG(NTAPI*)(HANDLE,ULONG,void*,ULONG,ULONG*);
bool Context(HANDLE thread,QueryThread query,uint32_t& context){
  ThreadBasicInfo info{};DWORD exit=0;uint64_t tlsArray=0,tls=0;uint32_t initialized=0;
  return GetExitCodeThread(thread,&exit) && exit==STILL_ACTIVE &&
    query(thread,0,&info,sizeof(info),nullptr)>=0 && info.teb &&
    Read((uint64_t)info.teb+0x58,&tlsArray,8) && Read(tlsArray,&tls,8) &&
    Read(tls+0x8600,&initialized,4) && (initialized&1) &&
    Read(tls+0x8648,&context,4) && context<16;
}
}
int main(int argc,char** argv){
  if(argc!=2){std::puts("usage: live_vfs_probe <Victoria3 PID>; verify exact EXE hash first");return 2;}
  DWORD pid=0;try{pid=std::stoul(argv[1]);}catch(...){return 2;}
  process=OpenProcess(PROCESS_VM_READ|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
  if(!process)return 3;
  HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);
  MODULEENTRY32W entry{};entry.dwSize=sizeof(entry);uint64_t module=0;
  if(Module32FirstW(snapshot,&entry))do{
    if(!_wcsicmp(entry.szModule,L"victoria3.exe")){module=(uint64_t)entry.modBaseAddr;break;}
  }while(Module32NextW(snapshot,&entry));
  CloseHandle(snapshot);if(!module){CloseHandle(process);return 4;}
  Window window{pid};EnumWindows(FindWindow,(LPARAM)&window);
  auto query=(QueryThread)GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"NtQueryInformationThread");
  HANDLE thread=window.count==1?OpenThread(THREAD_QUERY_INFORMATION,FALSE,window.thread):nullptr;
  uint32_t context=UINT32_MAX;
  if(!query || !thread || !Context(thread,query,context)){
    std::puts("current game window filesystem context unavailable");
    if(thread)CloseHandle(thread);CloseHandle(process);return 5;
  }
  std::string error;
  auto configuration=v3::DecodeNativeVfsConfiguration(Read,module,context,[&]{
    uint32_t live=UINT32_MAX;return Context(thread,query,live) && live==context;
  },&error);
  std::printf("pid=%lu windowThread=%lu module=%llx context=%u\n",pid,window.thread,(unsigned long long)module,context);
  if(configuration)for(size_t i=0;i<configuration->mounts.size();++i){
    const auto& mount=configuration->mounts[i];
    std::printf("[%zu] %s root=%s mount=%s source=%s\n",i,
      mount.kind==v3::NativeVfsKind::Directory?"directory":"zip",
      mount.root.c_str(),mount.mountPoint.c_str(),mount.sourcePrefix.c_str());
  }
  if(configuration){
    for(const auto& rule:configuration->replacements)
      std::printf("replace root=%s path=%s\n",rule.root.c_str(),rule.path.c_str());
    for(const auto& path:configuration->exclusions)std::printf("exclude path=%s\n",path.c_str());
    for(const char* directory:{"map_data","map_data/state_regions","gfx/map/spline_network","gui/gui_editor"})
      std::printf("directory=%s allowedRoots=%zu\n",directory,v3::NativeVfsDirectoryRootCount(*configuration,directory));
  }
  std::printf("%s mounts=%zu replacements=%zu exclusions=%zu error=%s\n",configuration?"PASS":"FAIL",
      configuration?configuration->mounts.size():0,configuration?configuration->replacements.size():0,
      configuration?configuration->exclusions.size():0,error.c_str());
  CloseHandle(thread);CloseHandle(process);return configuration?0:6;
}
