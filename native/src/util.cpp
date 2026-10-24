#include "util.h"

#include <windows.h>
#include <shlobj.h>
#include <algorithm>
#include <cstring>

namespace v3 {
namespace {

// Minimal SHA-256 (FIPS 180-4). Kept local so the loader has no crypto dependency.
struct Sha256 {
  uint32_t h[8] = {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                   0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
  uint64_t bits = 0;
  uint8_t buf[64];
  size_t used = 0;

  static uint32_t rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }

  void Block(const uint8_t* p) {
    static const uint32_t k[64] = {
        0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
        0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
        0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
        0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
        0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
        0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
        0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
        0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
    uint32_t w[64];
    for (int i = 0; i < 16; ++i)
      w[i] = (uint32_t)p[i * 4] << 24 | (uint32_t)p[i * 4 + 1] << 16 |
             (uint32_t)p[i * 4 + 2] << 8 | (uint32_t)p[i * 4 + 3];
    for (int i = 16; i < 64; ++i) {
      uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
      uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
      w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
    uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4], f = h[5], g = h[6], hh = h[7];
    for (int i = 0; i < 64; ++i) {
      uint32_t s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
      uint32_t ch = (e & f) ^ (~e & g);
      uint32_t t1 = hh + s1 + ch + k[i] + w[i];
      uint32_t s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
      uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
      uint32_t t2 = s0 + maj;
      hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d;
    h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
  }

  void Update(const uint8_t* p, size_t n) {
    bits += (uint64_t)n * 8;
    while (n) {
      size_t take = std::min(n, sizeof(buf) - used);
      std::memcpy(buf + used, p, take);
      used += take; p += take; n -= take;
      if (used == 64) { Block(buf); used = 0; }
    }
  }

  std::string Finish() {
    uint64_t total = bits;
    uint8_t pad = 0x80;
    Update(&pad, 1);
    uint8_t zero = 0;
    while (used != 56) Update(&zero, 1);
    uint8_t len[8];
    for (int i = 0; i < 8; ++i) len[i] = (uint8_t)(total >> (56 - i * 8));
    // Update() adds to bits; length bytes are appended manually.
    std::memcpy(buf + used, len, 8);
    Block(buf);
    used = 0;
    uint8_t out[32];
    for (int i = 0; i < 8; ++i) {
      out[i * 4] = (uint8_t)(h[i] >> 24);
      out[i * 4 + 1] = (uint8_t)(h[i] >> 16);
      out[i * 4 + 2] = (uint8_t)(h[i] >> 8);
      out[i * 4 + 3] = (uint8_t)h[i];
    }
    return ToHex(out, 32);
  }
};

}  // namespace

std::string ToHex(const uint8_t* data, size_t size) {
  static const char* digits = "0123456789abcdef";
  std::string out;
  out.resize(size * 2);
  for (size_t i = 0; i < size; ++i) {
    out[i * 2] = digits[data[i] >> 4];
    out[i * 2 + 1] = digits[data[i] & 15];
  }
  return out;
}

std::string Sha256Bytes(const void* data, size_t size) {
  Sha256 hash;
  if (size) hash.Update(static_cast<const uint8_t*>(data),size);
  return hash.Finish();
}

bool ReadFileBytes(const std::wstring& path, std::vector<uint8_t>* out) {
  HANDLE f = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                         nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  if (f == INVALID_HANDLE_VALUE) return false;
  LARGE_INTEGER size{};
  if (!GetFileSizeEx(f, &size) || size.QuadPart > (LONGLONG)1 << 34) {
    CloseHandle(f);
    return false;
  }
  out->resize((size_t)size.QuadPart);
  size_t done = 0;
  while (done < out->size()) {
    DWORD got = 0;
    if (!ReadFile(f, out->data() + done, (DWORD)std::min<size_t>(out->size() - done, 1 << 24), &got, nullptr) ||
        got == 0) {
      CloseHandle(f);
      return false;
    }
    done += got;
  }
  CloseHandle(f);
  return true;
}

std::string Sha256File(const std::wstring& path, std::string* error) {
  std::vector<uint8_t> data;
  if (!ReadFileBytes(path, &data)) {
    if (error) *error = "cannot read executable";
    return {};
  }
  Sha256 s;
  s.Update(data.data(), data.size());
  return s.Finish();
}

std::wstring ModulePath(void* module) {
  wchar_t buf[2048];
  DWORD n = GetModuleFileNameW((HMODULE)module, buf, 2048);
  if (!n) return {};
  return std::wstring(buf, n);
}

std::wstring DirectoryOf(const std::wstring& path) {
  size_t p = path.find_last_of(L"\\/");
  return p == std::wstring::npos ? std::wstring() : path.substr(0, p);
}

std::wstring LocalAppDataDir() {
  PWSTR p = nullptr;
  std::wstring out;
  if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, nullptr, &p)) && p) {
    out = p;
    CoTaskMemFree(p);
  }
  if (out.empty()) {
    wchar_t buf[MAX_PATH];
    DWORD n = GetEnvironmentVariableW(L"LOCALAPPDATA", buf, MAX_PATH);
    if (n && n < MAX_PATH) out.assign(buf, n);
  }
  return out;
}

std::string Narrow(const std::wstring& s) {
  if (s.empty()) return {};
  int n = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0, nullptr, nullptr);
  std::string out(n, '\0');
  WideCharToMultiByte(CP_UTF8, 0, s.c_str(), (int)s.size(), out.data(), n, nullptr, nullptr);
  return out;
}

std::wstring Widen(const std::string& s) {
  if (s.empty()) return {};
  int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
  std::wstring out(n, L'\0');
  MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), out.data(), n);
  return out;
}

std::string LowerAscii(std::string s) {
  for (char& c : s)
    if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
  return s;
}

}  // namespace v3
