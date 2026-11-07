#pragma once
#include <windows.h>
#include <MinHook.h>
#include <atomic>
#include <cstdint>
#include <cwchar>

namespace v3::desktop_test {
inline constexpr wchar_t kEnvironment[]=L"IRONROAD_TEST_DESKTOP";
inline constexpr wchar_t kCaptureMessage[]=L"Ironroad.IsolatedDesktopTest.Capture.v1";
inline constexpr WPARAM kCaptureCookie=0x49525431;
inline std::atomic<bool> active{false},pointReady{false},capture{false};
inline std::atomic<uint64_t> point{0};
inline std::atomic<HWND> window{nullptr};
inline wchar_t desktop[256]{};
inline UINT captureMessage=0;
inline void* getTarget=nullptr;
inline void* setTarget=nullptr;
inline void* clipTarget=nullptr;

inline bool NameIs(HDESK handle,const wchar_t* expected) {
  wchar_t name[256]{};DWORD needed=0;
  return handle && GetUserObjectInformationW(handle,UOI_NAME,name,sizeof(name),&needed) &&
      !wcscmp(name,expected);
}
inline bool Safe(HWND hwnd) {
  DWORD pid=0;const DWORD thread=GetWindowThreadProcessId(hwnd,&pid);
  if(!thread || pid!=GetCurrentProcessId() ||
     !NameIs(GetThreadDesktop(thread),desktop) ||
     !NameIs(GetThreadDesktop(GetCurrentThreadId()),desktop))return false;
  HDESK input=OpenInputDesktop(0,FALSE,DESKTOP_READOBJECTS);
  if(!input)return false;
  wchar_t name[256]{};DWORD needed=0;
  const bool separate=GetUserObjectInformationW(input,UOI_NAME,name,sizeof(name),&needed) &&
      wcscmp(name,desktop)!=0;
  CloseDesktop(input);
  return separate;
}
inline void Store(LONG x,LONG y) {
  point.store(uint64_t(uint32_t(x))|(uint64_t(uint32_t(y))<<32));
  pointReady.store(true);
}
inline BOOL WINAPI Get(POINT* out) {
  if(!out || !active.load() || !pointReady.load() || !Safe(window.load())){
    SetLastError(ERROR_ACCESS_DENIED);return FALSE;
  }
  const uint64_t p=point.load();out->x=LONG(uint32_t(p));out->y=LONG(uint32_t(p>>32));return TRUE;
}
inline BOOL WINAPI Set(int x,int y) {
  if(!active.load() || !Safe(window.load())){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
  Store(x,y);return TRUE;
}
inline BOOL WINAPI RejectClip(const RECT*) {
  // Virtual events do not need system-wide confinement or unclipping.
  SetLastError(active.load() && Safe(window.load())?ERROR_NOT_SUPPORTED:ERROR_ACCESS_DENIED);
  return FALSE;
}
// Process-local hooks live as long as the existing renderer hooks. Never call
// the physical cursor functions from test mode, including a retired desktop.
inline bool Install(HWND hwnd) {
  if(active.load()){
    if(!Safe(hwnd))return false;
    window.store(hwnd);pointReady.store(false);capture.store(false);return true;
  }
  const DWORD count=GetEnvironmentVariableW(kEnvironment,desktop,256);
  constexpr wchar_t prefix[]=L"CodexFreightTest_";
  if(!count || count>=256 || wcsncmp(desktop,prefix,wcslen(prefix)) || !Safe(hwnd))return false;
  HMODULE user=GetModuleHandleW(L"user32.dll");
  getTarget=user?(void*)GetProcAddress(user,"GetCursorPos"):nullptr;
  setTarget=user?(void*)GetProcAddress(user,"SetCursorPos"):nullptr;
  clipTarget=user?(void*)GetProcAddress(user,"ClipCursor"):nullptr;
  if(!getTarget || !setTarget || !clipTarget)return false;
  if(MH_CreateHook(getTarget,(LPVOID)&Get,nullptr)!=MH_OK)return false;
  if(MH_CreateHook(setTarget,(LPVOID)&Set,nullptr)!=MH_OK){MH_RemoveHook(getTarget);return false;}
  if(MH_CreateHook(clipTarget,(LPVOID)&RejectClip,nullptr)!=MH_OK){
    MH_RemoveHook(getTarget);MH_RemoveHook(setTarget);return false;
  }
  captureMessage=RegisterWindowMessageW(kCaptureMessage);
  window.store(hwnd);
  if(!captureMessage || MH_EnableHook(getTarget)!=MH_OK || MH_EnableHook(setTarget)!=MH_OK ||
     MH_EnableHook(clipTarget)!=MH_OK){
    MH_DisableHook(getTarget);MH_DisableHook(setTarget);MH_DisableHook(clipTarget);
    MH_RemoveHook(getTarget);MH_RemoveHook(setTarget);MH_RemoveHook(clipTarget);
    window.store(nullptr);return false;
  }
  active.store(true);return true;
}
inline bool Message(HWND hwnd,UINT msg,WPARAM wp,LPARAM lp) {
  if(!active.load() || hwnd!=window.load())return false;
  if(msg==captureMessage){
    if(wp==kCaptureCookie && !lp && Safe(hwnd))capture.store(true);
    return true;
  }
  if(msg==WM_MOUSEMOVE && Safe(hwnd)){
    POINT p{LONG(int16_t(LOWORD(lp))),LONG(int16_t(HIWORD(lp)))};
    if(ClientToScreen(hwnd,&p))Store(p.x,p.y);
  }
  return false;
}
inline bool TakeCapture() {
  return capture.exchange(false) && active.load() && Safe(window.load());
}
} // namespace v3::desktop_test
