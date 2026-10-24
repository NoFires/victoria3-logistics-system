// freight.dll core, loaded by the version.dll bootstrap from freight/.
#include <windows.h>

#include <string>

#include "game.h"
#include "embedded_assets.h"
#include "log.h"
#include "overlay.h"
#include "util.h"

namespace {
HMODULE g_self = nullptr;

// Diagnostic only: log the first access violation, then let Windows handle it as
// usual. Used to tell a bad native call apart from a rendering problem.
LONG CALLBACK AccessViolationLogger(EXCEPTION_POINTERS* info) {
  static LONG once = 0;
  if (!info || !info->ExceptionRecord) return EXCEPTION_CONTINUE_SEARCH;
  DWORD code = info->ExceptionRecord->ExceptionCode;
  if (code != 0xC0000005 && code != 0xC0000409) return EXCEPTION_CONTINUE_SEARCH;
  if (InterlockedExchange(&once, 1) != 0) return EXCEPTION_CONTINUE_SEARCH;
  void* address = info->ExceptionRecord->ExceptionAddress;
  HMODULE module = nullptr;
  wchar_t name[MAX_PATH] = L"?";
  if (GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                             GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                         (LPCWSTR)address, &module) &&
      module) {
    GetModuleFileNameW(module, name, MAX_PATH);
  }
  V3_ERR("VEH code=0x%08lx address=%p module=%ls", (unsigned long)code, address,
         name);
  V3_INFO("VEH access=%p read=%d write=%d",
          (void*)info->ExceptionRecord->ExceptionInformation[1],
          (int)info->ExceptionRecord->ExceptionInformation[0] == 0,
          (int)info->ExceptionRecord->ExceptionInformation[0] == 1);
  v3::LogFlush(true);
  return EXCEPTION_CONTINUE_SEARCH;
}

DWORD WINAPI BootstrapThread(LPVOID) {
  // Windows starts this thread after loader notifications finish. Install the
  // asset hook promptly, before model loading; no work occurs under loader lock.
  // Active native hooks and workers retain DLL code for the process lifetime.
  // Rollback is performed with the game closed, never by hot-unloading hooks.
  HMODULE pinned = nullptr;
  GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
      reinterpret_cast<LPCWSTR>(&BootstrapThread), &pinned);
  std::wstring logDir = v3::DirectoryOf(v3::ModulePath(g_self));
  v3::LogInit(logDir);
  V3_INFO("loader attached: pid=%lu base=%p", (unsigned long)GetCurrentProcessId(),
          GetModuleHandleW(nullptr));
  AddVectoredExceptionHandler(1, &AccessViolationLogger);

  std::wstring exePath = v3::ModulePath(nullptr);
  std::string err;
  bool ok = v3::Game::Instance().Initialize(exePath, &err);
  if (!ok) V3_WARN("native profile disabled: %s", err.c_str());
  if (!v3::PrepareEmbeddedRuntime(g_self, &err)) {
    V3_ERR("embedded runtime unavailable: %s", err.c_str());
    v3::LogFlush(true);
    return 0;
  } else if (ok && !v3::InstallEmbeddedAssetHook(&err)) {
    V3_ERR("embedded asset hook unavailable: %s", err.c_str());
  }

  if (!v3::Overlay::Instance().Install(&err)) {
    V3_ERR("overlay install failed: %s", err.c_str());
  } else {
    V3_INFO("overlay installed");
  }
  v3::LogFlush(true);
  return 0;
}
}  // namespace

BOOL WINAPI DllMain(HINSTANCE self, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH) {
    g_self = self;
    DisableThreadLibraryCalls(self);
    HANDLE t = CreateThread(nullptr, 0, &BootstrapThread, nullptr, 0, nullptr);
    if (t) CloseHandle(t);
  }
  return TRUE;
}
