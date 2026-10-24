#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace v3 {

std::string Sha256File(const std::wstring& path, std::string* error);
std::string Sha256Bytes(const void* data, size_t size);
std::string ToHex(const uint8_t* data, size_t size);
std::wstring ModulePath(void* module);
std::wstring DirectoryOf(const std::wstring& path);
std::wstring LocalAppDataDir();
bool ReadFileBytes(const std::wstring& path, std::vector<uint8_t>* out);
std::string Narrow(const std::wstring& s);
std::wstring Widen(const std::string& s);
std::string LowerAscii(std::string s);

}  // namespace v3
