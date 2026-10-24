#pragma once
#include <windows.h>
#include <string>

namespace v3 {
// The installed DLL is self-contained. These are automatically managed caches,
// never launcher mods, and contain no copies of the game's textures/shaders.
bool PrepareEmbeddedRuntime(HMODULE self, std::string* error);
bool InstallEmbeddedAssetHook(std::string* error);
const std::wstring& RuntimeDataDirectory();
bool EmbeddedAssetsMounted();
}
