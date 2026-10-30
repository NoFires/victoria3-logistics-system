#include "embedded_assets.h"
#include "game.h"
#include "native_gui.h"
#include "editable_resources.h"
#include "log.h"
#include "offsets.h"
#include "util.h"
#include <MinHook.h>
#include <atomic>
#include <cstring>
#include <filesystem>
#include <stdexcept>

namespace v3 {
namespace {
std::wstring dataDirectory, assetArchive, guiArchive;
std::atomic<bool> mountedArchive{false};
std::atomic<bool> mountedGui{false};
using MountFn = int(__fastcall*)(void*, const char*, const char*);
using ResetRootsFn = void(__fastcall*)();
struct NativeStringView { const char* data; uint32_t length; uint32_t flags = 0; };
static_assert(sizeof(NativeStringView) == 16);
using PreEnumerateFn = void(__fastcall*)(const NativeStringView*, const NativeStringView*, const NativeStringView*, int32_t);
PreEnumerateFn originalEnumerate = nullptr;
using MountDataFn = void(__fastcall*)(void*, void*, void*);
MountDataFn originalMountData = nullptr;

void __fastcall MountDataThunk(void* roots, void* prefix, void* paths) {
  originalMountData(roots, prefix, paths);
  auto* base = Game::Instance().Base();
  auto mount = [&](const std::wstring& path, std::atomic<bool>& mounted, const char* label) {
    if (path.empty() || mounted.load()) return;
    const auto archive = Narrow(path);
    if (reinterpret_cast<MountFn>(base + kRvaMountAssetPath)(nullptr, archive.c_str(), nullptr)) {
      reinterpret_cast<ResetRootsFn>(base + kRvaRefreshAssetRoots)();
      mounted.store(true);
      V3_INFO("%s archive mounted during native MountData BEFORE VFS configuration freezes (no playset)", label);
    } else V3_ERR("%s archive mount failed during native MountData", label);
  };
  mount(assetArchive, mountedArchive, "embedded");
  mount(guiArchive, mountedGui, "native GUI");
}

bool WriteResource(HMODULE self, int id, const std::wstring& path, std::string* error) {
  HRSRC resource = FindResourceW(self, MAKEINTRESOURCEW(id), RT_RCDATA);
  HGLOBAL loaded = resource ? LoadResource(self, resource) : nullptr;
  const void* bytes = loaded ? LockResource(loaded) : nullptr;
  DWORD size = resource ? SizeofResource(self, resource) : 0;
  if (!bytes || !size) { if (error) *error = "embedded resource missing: " + std::to_string(id); return false; }
  std::vector<uint8_t> previous;
  if (ReadFileBytes(path, &previous) && previous.size() == size &&
      std::memcmp(previous.data(), bytes, size) == 0) return true;
  std::wstring temporary = path + L".tmp-" + std::to_wstring(GetCurrentProcessId());
  HANDLE file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
  DWORD written = 0;
  bool ok = file != INVALID_HANDLE_VALUE && WriteFile(file, bytes, size, &written, nullptr) && written == size;
  if (file != INVALID_HANDLE_VALUE) CloseHandle(file);
  if (ok) ok = MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
  if (!ok) {
    if (error) *error = "cannot cache embedded resource " + std::to_string(id) + " (Win32 " + std::to_string(GetLastError()) + ")";
    DeleteFileW(temporary.c_str());
  }
  return ok;
}

void __fastcall PreEnumerateThunk(const NativeStringView* root, const NativeStringView* extension,
    const NativeStringView* excluded, int32_t depth) {
  const bool models = root && root->length == 10 && std::memcmp(root->data, "gfx/models", 10) == 0;
  const bool assets = extension && extension->length == 6 && std::memcmp(extension->data, ".asset", 6) == 0;
  // The archive must be mounted before the game's asynchronous file-list cache
  // is populated, not in the later ThreadedInitTextureLookup callback.
  if (models && assets) {
    if (mountedArchive.load()) {
      V3_INFO("embedded archive ready BEFORE gfx/models .asset enumeration");
    } else {
      V3_ERR("embedded startup mount missed; refusing to mutate frozen VFS configuration");
    }
  }
  if (root && root->length == 3 && std::memcmp(root->data, "gui", 3) == 0)
    V3_INFO("native GUI enumeration; prepared=%d mounted=%d", !guiArchive.empty(), mountedGui.load());
  originalEnumerate(root, extension, excluded, depth);
}
}

bool PrepareEmbeddedRuntime(HMODULE self, std::string* error) {
  try {
    const auto root = DirectoryOf(ModulePath(self));
    if (root.empty()) { if (error) *error = "freight DLL directory unavailable"; return false; }
    dataDirectory = root + L"\\data";
    std::filesystem::create_directories(dataDirectory);
    const std::string editing=
        "Freight editable resources / 可编辑资源\r\n\r\n"
        "native-gui/gui/: GUI files generated from your installed game and our fragment.\r\n"
        "native-gui/localization/: English and Simplified Chinese labels/tooltips.\r\n"
        "assets/gfx/models/military/ironroad_arrows.asset: our arrow definitions.\r\n"
        "assets/gfx/FX/ironroad_building_flow.shader: our arrow shader.\r\n\r\n"
        "关闭游戏后编辑上述文件，重启后生效。无需启用外部 Mod。\r\n"
        "Edit while the game is closed; restart to apply. No external Mod/playset is needed.\r\n"
        "玩家修改会保留。升级仅替换仍与上次默认值相同的文件。\r\n"
        "Your edits are preserved. Upgrades replace only untouched previous defaults.\r\n"
        "每个目录中的 .defaults 保存当前发行默认值，供比较／复制恢复。\r\n"
        "Each .defaults folder holds current shipped defaults for comparison/reset.\r\n"
        "删除一个可编辑文件后，下次启动会重新生成；删除前可自行备份。\r\n"
        "Deleting an editable file regenerates its current default at next startup.\r\n"
        "不要编辑内容命名的 ZIP 缓存；它们由上述明文文件自动生成。\r\n"
        "Do not edit content-addressed ZIP caches; they are generated from the plain files.\r\n"
        "原版完整 GUI 只从本机安装派生，换游戏版本时请对照新 .defaults 合并自己的修改。\r\n"
        "Full base GUI is derived locally. When upgrading game versions, merge edits with new .defaults.\r\n"
        "GUI 格式错误会记录到 freight.log / 游戏 error.log，不覆盖你的文件。\r\n"
        "Invalid GUI is logged; your source file is retained. Keep internal variable/style names when changing appearance.\r\n\r\n"
        "国内运量为观察性物流模型分配；国际部分保留原版贸易数量。\r\n"
        "Domestic quantities are modeled logistics allocations; international quantities retain original trade data.\r\n"
        "地图与当前可用连接从游戏对象读取；data 中的旧派生图仅供历史调试，不作为正式地图绑定表。\r\n";
    EditableResources(root,{{"EDITING_RESOURCES.txt",{editing.begin(),editing.end()}}});
    // Content-address the archive so a running game never has its mounted file
    // replaced by a different build's standalone test process.
    HRSRC zip = FindResourceW(self, MAKEINTRESOURCEW(151), RT_RCDATA);
    auto block = zip ? LoadResource(self, zip) : nullptr;
    auto bytes = block ? static_cast<const uint8_t*>(LockResource(block)) : nullptr;
    if (!bytes) { if (error) *error = "embedded assets ZIP missing"; return false; }
    const auto assetFiles=EditableResources(root+L"\\assets",ReadStoredZip({bytes,bytes+SizeofResource(self,zip)}));
    const auto archive=BuildStoredZip(assetFiles);
    assetArchive=root+L"\\assets-"+Widen(Sha256Bytes(archive.data(),archive.size()))+L".zip";
    std::vector<uint8_t> previous;
    if(!ReadFileBytes(assetArchive,&previous) || previous!=archive)WriteResourceFile(assetArchive,archive);
    const wchar_t* files[] = {L"positions.txt", L"heights.bin", L"provinces.bin", L"terrain_names.txt", L"splines.bin"};
    for (int i = 0; i < 5; ++i)
      if (!WriteResource(self, 201+i, dataDirectory + L"\\" + files[i], error)) return false;
    if (Game::Instance().Ready()) {
      std::string why;
      const auto gameRoot = DirectoryOf(DirectoryOf(ModulePath(nullptr)));
      if (PrepareNativeGui(self, gameRoot, root, &guiArchive, &why))
        V3_INFO("native GUI archive prepared from verified installed market panel: %ls", guiArchive.c_str());
      else V3_ERR("native GUI disabled: %s", why.c_str());
    }
    // Copy old preferences only when the new canonical file is absent.
    const auto rootIni = root + L"\\freight.ini";
    if (!std::filesystem::exists(rootIni)) {
      const auto oldRoot = LocalAppDataDir() + L"\\IronroadV3";
      const std::wstring candidates[] = {
          root + L"\\ironroad.ini", oldRoot + L"\\ironroad.ini",
          oldRoot + L"\\data\\ironroad.ini",
          DirectoryOf(root) + L"\\ironroad\\ironroad.ini"};
      for (const auto& source : candidates) {
        if (!std::filesystem::exists(source)) continue;
        const auto temporary = rootIni + L".migration.tmp";
        if (!CopyFileW(source.c_str(), temporary.c_str(), FALSE) ||
            !MoveFileExW(temporary.c_str(), rootIni.c_str(), MOVEFILE_WRITE_THROUGH)) {
          DeleteFileW(temporary.c_str());
          if (error) *error = "freight preferences migration failed";
          return false;
        }
        break;
      }
    }
    V3_INFO("self-contained DLL resources prepared; runtime data=%ls", dataDirectory.c_str());
    return true;
  } catch (const std::exception& ex) {
    if (error) *error = std::string("embedded runtime preparation: ") + ex.what();
    return false;
  }
}

bool InstallEmbeddedAssetHook(std::string* error) {
  if (!Game::Instance().Ready() || assetArchive.empty()) return false;
  auto result = MH_Initialize();
  if (result != MH_OK && result != MH_ERROR_ALREADY_INITIALIZED) {
    if (error) *error = "MinHook init for embedded assets failed";
    return false;
  }
  auto target = Game::Instance().Base() + kRvaPreEnumerateAssets;
  auto mountTarget = Game::Instance().Base() + kRvaMountData;
  if (MH_CreateHook(mountTarget, reinterpret_cast<LPVOID>(&MountDataThunk), reinterpret_cast<LPVOID*>(&originalMountData)) != MH_OK ||
      MH_EnableHook(mountTarget) != MH_OK) {
    if (error) *error = "embedded startup MountData hook failed";
    return false;
  }
  if (MH_CreateHook(target, reinterpret_cast<LPVOID>(&PreEnumerateThunk), reinterpret_cast<LPVOID*>(&originalEnumerate)) != MH_OK ||
      MH_EnableHook(target) != MH_OK) {
    if (error) *error = "embedded model asset hook failed";
    return false;
  }
  V3_INFO("embedded asset pre-enumeration hook installed");
  return true;
}
const std::wstring& RuntimeDataDirectory() { return dataDirectory; }
bool EmbeddedAssetsMounted() { return mountedArchive.load(); }
}
