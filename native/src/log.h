#pragma once
#include <string>

namespace v3 {

enum class LogLevel { Info, Warn, Error };

// Cheap, buffered logger. Routine lines are appended to an in-memory buffer and
// flushed at most twice a second by Flush() from the render thread. Errors flush
// immediately. Never call from DllMain.
void LogInit(const std::wstring& directory);
void LogLine(LogLevel level, const char* fmt, ...);
void LogFlush(bool force);
const std::wstring& LogPath();

#define V3_INFO(...) ::v3::LogLine(::v3::LogLevel::Info, __VA_ARGS__)
#define V3_WARN(...) ::v3::LogLine(::v3::LogLevel::Warn, __VA_ARGS__)
#define V3_ERR(...) ::v3::LogLine(::v3::LogLevel::Error, __VA_ARGS__)

}  // namespace v3
