#include "log.h"
#include "util.h"

#include <windows.h>
#include <cstdarg>
#include <cstdio>
#include "mutex.h"
#include <string>

namespace v3 {
namespace {
v3::Mutex g_mutex;
std::string g_buffer;
std::wstring g_path;
ULONGLONG g_lastFlush = 0;
bool g_ready = false;

const char* LevelTag(LogLevel l) {
  switch (l) {
    case LogLevel::Warn: return "WARN";
    case LogLevel::Error: return "ERR ";
    default: return "INFO";
  }
}

void Append(const std::string& line, bool force) {
  if (!g_ready) return;
  v3::LockGuard lock(g_mutex);
  g_buffer += line;
  ULONGLONG now = GetTickCount64();
  if (!force && g_buffer.size() < 32768 && now - g_lastFlush < 500) return;
  HANDLE f = CreateFileW(g_path.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, nullptr,
                         OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (f != INVALID_HANDLE_VALUE) {
    DWORD wrote = 0;
    WriteFile(f, g_buffer.data(), (DWORD)g_buffer.size(), &wrote, nullptr);
    CloseHandle(f);
  }
  g_buffer.clear();
  g_lastFlush = now;
}
}  // namespace

void LogInit(const std::wstring& directory) {
  CreateDirectoryW(directory.c_str(), nullptr);
  g_path = directory + L"\\freight.log";
  // Rotate once the log grows past 8 MiB; keeps routine runs from filling the disk.
  WIN32_FILE_ATTRIBUTE_DATA attr{};
  if (GetFileAttributesExW(g_path.c_str(), GetFileExInfoStandard, &attr) &&
      ((uint64_t)attr.nFileSizeHigh << 32 | attr.nFileSizeLow) > (8ull << 20)) {
    std::wstring old = directory + L"\\freight.old.log";
    DeleteFileW(old.c_str());
    MoveFileW(g_path.c_str(), old.c_str());
  }
  g_ready = true;
  SYSTEMTIME st;
  GetLocalTime(&st);
  char header[160];
  std::snprintf(header, sizeof(header),
                "=== Ironroad Victoria 3 overlay %04d-%02d-%02d %02d:%02d:%02d pid=%lu ===\n",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
                (unsigned long)GetCurrentProcessId());
  Append(header, true);
}

void LogLine(LogLevel level, const char* fmt, ...) {
  char msg[2048];
  va_list ap;
  va_start(ap, fmt);
  std::vsnprintf(msg, sizeof(msg), fmt, ap);
  va_end(ap);
  char line[2200];
  std::snprintf(line, sizeof(line), "[%8llu] %s %s\n",
                (unsigned long long)GetTickCount64(), LevelTag(level), msg);
  Append(line, level == LogLevel::Error);
}

void LogFlush(bool force) {
  v3::LockGuard lock(g_mutex);
  if (g_buffer.empty()) return;
  if (!force && GetTickCount64() - g_lastFlush < 500) return;
  // Unlock before Append takes the mutex again.
  std::string pending;
  pending.swap(g_buffer);
  HANDLE f = CreateFileW(g_path.c_str(), FILE_APPEND_DATA, FILE_SHARE_READ, nullptr,
                         OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (f != INVALID_HANDLE_VALUE) {
    DWORD wrote = 0;
    WriteFile(f, pending.data(), (DWORD)pending.size(), &wrote, nullptr);
    CloseHandle(f);
  }
  g_lastFlush = GetTickCount64();
}

const std::wstring& LogPath() { return g_path; }

}  // namespace v3

