#pragma once
#include <windows.h>
#include <cstdint>
#include <string>
#include <vector>
namespace v3 {
inline constexpr char kMarketGuiHash[]="1f682222b689666731e755bc52b412b3126f4afff40cde05b13a0757b027ec3e";
inline constexpr char kStateGuiHash[]="ee259534f21785d962672ba25ebb66e6f3c881ffc2f74fdc44caaf989ab76be6";
bool PatchNativeStateGui(const std::vector<uint8_t>& original,std::string* patched,std::string* error);
bool PatchNativeMarketGui(const std::vector<uint8_t>& original,const std::string& tabOverride,
                         std::string* patched,std::string* error);
bool PrepareNativeGui(HMODULE self,const std::wstring& gameRoot,const std::wstring& cacheRoot,
                      std::wstring* archivePath,std::string* error);
} // namespace v3
