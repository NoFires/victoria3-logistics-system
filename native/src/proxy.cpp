// Ironroad freight bootstrap: version.dll must remain beside victoria3.exe.
// System version exports are forwarded unchanged; the payload lives in freight/.
#include <windows.h>
#include <winver.h>
#include <cstdio>
#include <string>

namespace {
HMODULE g_self = nullptr;
HMODULE RealVersion() {
  static HMODULE mod = []() -> HMODULE {
    wchar_t sys[MAX_PATH]{};
    UINT n = GetSystemDirectoryW(sys, MAX_PATH);
    if (!n || n + 16 >= MAX_PATH) return nullptr;
    std::wstring path(sys, n);
    path += L"\\version.dll";
    return LoadLibraryW(path.c_str());
  }();
  return mod;
}

template <typename T>
T RealProc(const char* name) {
  HMODULE m = RealVersion();
  return m ? (T)GetProcAddress(m, name) : nullptr;
}


DWORD WINAPI LoadFreight(LPVOID) {
  HMODULE pinned = nullptr;
  GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN,
      reinterpret_cast<LPCWSTR>(&LoadFreight), &pinned);
  wchar_t path[32768]{};
  const DWORD n = GetModuleFileNameW(g_self, path, 32768);
  if (!n || n >= 32768) return 0;
  const auto slash = std::wstring(path, n).find_last_of(L"\\/");
  if (slash == std::wstring::npos) return 0;
  const auto root = std::wstring(path, slash) + L"\\freight";
  CreateDirectoryW(root.c_str(), nullptr);
  const auto dll = root + L"\\freight.dll";
  const HMODULE payload = LoadLibraryExW(dll.c_str(), nullptr,
      LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_SYSTEM32);
  const DWORD error = payload ? ERROR_SUCCESS : GetLastError();
  char message[128];
  const int length = std::snprintf(message, sizeof(message),
      "Ironroad freight loader: %s (error=%lu)\r\n",
      payload ? "loaded freight.dll" : "LoadLibraryExW failed", (unsigned long)error);
  OutputDebugStringA(message);
  const auto log = root + L"\\loader.log";
  HANDLE file = CreateFileW(log.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, nullptr,
      OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (file != INVALID_HANDLE_VALUE) {
    DWORD written = 0;
    WriteFile(file, message, length, &written, nullptr);
    CloseHandle(file);
  }
  return 0;
}
}  // namespace

extern "C" {

__declspec(dllexport) BOOL APIENTRY GetFileVersionInfoA(LPCSTR file, DWORD handle, DWORD len,
                                                         LPVOID data) {
  auto fn = RealProc<decltype(&GetFileVersionInfoA)>("GetFileVersionInfoA");
  return fn ? fn(file, handle, len, data) : FALSE;
}
__declspec(dllexport) BOOL APIENTRY GetFileVersionInfoW(LPCWSTR file, DWORD handle, DWORD len,
                                                         LPVOID data) {
  auto fn = RealProc<decltype(&GetFileVersionInfoW)>("GetFileVersionInfoW");
  return fn ? fn(file, handle, len, data) : FALSE;
}
__declspec(dllexport) DWORD APIENTRY GetFileVersionInfoSizeA(LPCSTR file, LPDWORD handle) {
  auto fn = RealProc<decltype(&GetFileVersionInfoSizeA)>("GetFileVersionInfoSizeA");
  return fn ? fn(file, handle) : 0;
}
__declspec(dllexport) DWORD APIENTRY GetFileVersionInfoSizeW(LPCWSTR file, LPDWORD handle) {
  auto fn = RealProc<decltype(&GetFileVersionInfoSizeW)>("GetFileVersionInfoSizeW");
  return fn ? fn(file, handle) : 0;
}
__declspec(dllexport) BOOL APIENTRY VerQueryValueA(LPCVOID block, LPCSTR sub, LPVOID* buf,
                                                   PUINT len) {
  auto fn = RealProc<decltype(&VerQueryValueA)>("VerQueryValueA");
  return fn ? fn(block, sub, buf, len) : FALSE;
}
__declspec(dllexport) BOOL APIENTRY VerQueryValueW(LPCVOID block, LPCWSTR sub, LPVOID* buf,
                                                   PUINT len) {
  auto fn = RealProc<decltype(&VerQueryValueW)>("VerQueryValueW");
  return fn ? fn(block, sub, buf, len) : FALSE;
}
__declspec(dllexport) DWORD APIENTRY VerFindFileA(DWORD flags, LPSTR file, LPSTR winDir,
                                                  LPSTR appDir, LPSTR curDir, PUINT curLen,
                                                  LPSTR destDir, PUINT destLen) {
  auto fn = RealProc<decltype(&VerFindFileA)>("VerFindFileA");
  return fn ? fn(flags, file, winDir, appDir, curDir, curLen, destDir, destLen) : 0;
}
__declspec(dllexport) DWORD APIENTRY VerFindFileW(DWORD flags, LPWSTR file, LPWSTR winDir,
                                                  LPWSTR appDir, LPWSTR curDir, PUINT curLen,
                                                  LPWSTR destDir, PUINT destLen) {
  auto fn = RealProc<decltype(&VerFindFileW)>("VerFindFileW");
  return fn ? fn(flags, file, winDir, appDir, curDir, curLen, destDir, destLen) : 0;
}
__declspec(dllexport) DWORD APIENTRY VerInstallFileA(DWORD flags, LPSTR src, LPSTR dest,
                                                     LPSTR srcDir, LPSTR destDir, LPSTR curDir,
                                                     LPSTR tmp, PUINT tmpLen) {
  auto fn = RealProc<decltype(&VerInstallFileA)>("VerInstallFileA");
  return fn ? fn(flags, src, dest, srcDir, destDir, curDir, tmp, tmpLen) : 0;
}
__declspec(dllexport) DWORD APIENTRY VerInstallFileW(DWORD flags, LPWSTR src, LPWSTR dest,
                                                     LPWSTR srcDir, LPWSTR destDir, LPWSTR curDir,
                                                     LPWSTR tmp, PUINT tmpLen) {
  auto fn = RealProc<decltype(&VerInstallFileW)>("VerInstallFileW");
  return fn ? fn(flags, src, dest, srcDir, destDir, curDir, tmp, tmpLen) : 0;
}
__declspec(dllexport) DWORD APIENTRY VerLanguageNameA(DWORD lang, LPSTR buf, DWORD len) {
  auto fn = RealProc<decltype(&VerLanguageNameA)>("VerLanguageNameA");
  return fn ? fn(lang, buf, len) : 0;
}
__declspec(dllexport) DWORD APIENTRY VerLanguageNameW(DWORD lang, LPWSTR buf, DWORD len) {
  auto fn = RealProc<decltype(&VerLanguageNameW)>("VerLanguageNameW");
  return fn ? fn(lang, buf, len) : 0;
}
__declspec(dllexport) BOOL APIENTRY GetFileVersionInfoExA(DWORD flags, LPCSTR file, DWORD handle,
                                                          DWORD len, LPVOID data) {
  auto fn = RealProc<decltype(&GetFileVersionInfoExA)>("GetFileVersionInfoExA");
  return fn ? fn(flags, file, handle, len, data) : FALSE;
}
__declspec(dllexport) BOOL APIENTRY GetFileVersionInfoExW(DWORD flags, LPCWSTR file, DWORD handle,
                                                          DWORD len, LPVOID data) {
  auto fn = RealProc<decltype(&GetFileVersionInfoExW)>("GetFileVersionInfoExW");
  return fn ? fn(flags, file, handle, len, data) : FALSE;
}
__declspec(dllexport) DWORD APIENTRY GetFileVersionInfoSizeExA(DWORD flags, LPCSTR file,
                                                               LPDWORD handle) {
  auto fn = RealProc<decltype(&GetFileVersionInfoSizeExA)>("GetFileVersionInfoSizeExA");
  return fn ? fn(flags, file, handle) : 0;
}
__declspec(dllexport) DWORD APIENTRY GetFileVersionInfoSizeExW(DWORD flags, LPCWSTR file,
                                                               LPDWORD handle) {
  auto fn = RealProc<decltype(&GetFileVersionInfoSizeExW)>("GetFileVersionInfoSizeExW");
  return fn ? fn(flags, file, handle) : 0;
}
__declspec(dllexport) BOOL APIENTRY GetFileVersionInfoByHandle(HANDLE file, LPCWSTR filename,
                                                               DWORD len, LPVOID data) {
  auto fn = RealProc<BOOL(APIENTRY*)(HANDLE, LPCWSTR, DWORD, LPVOID)>("GetFileVersionInfoByHandle");
  return fn ? fn(file, filename, len, data) : FALSE;
}

}  // extern "C"


BOOL WINAPI DllMain(HINSTANCE self, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH) {
    g_self = self;
    DisableThreadLibraryCalls(self);
    HANDLE thread = CreateThread(nullptr, 0, &LoadFreight, nullptr, 0, nullptr);
    if (thread) CloseHandle(thread);
  }
  return TRUE;
}
