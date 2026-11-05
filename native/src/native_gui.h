#pragma once
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
namespace v3 {
inline constexpr char kMarketGuiHash[]="831ec4036314d4d27313030e824b1db32f3d639407bf26e2fc26bb4b83fd4ed0";
inline constexpr char kStateGuiHash[]="fa5dfdb31d17f9c00e60b64067b851e1dc2105c846f3dc3e13ce933829c90e2e";
bool PatchNativeStateGui(const std::vector<uint8_t>& original,std::string* patched,std::string* error);
bool PatchNativeMarketGui(const std::vector<uint8_t>& original,const std::string& tabOverride,
                         std::string* patched,std::string* error);
bool PrepareNativeGui(HMODULE self,const std::wstring& gameRoot,const std::wstring& cacheRoot,
                      std::wstring* archivePath,std::string* error);
} // namespace v3
