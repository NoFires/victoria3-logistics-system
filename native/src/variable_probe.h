#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace v3 {
struct GuiVariableEntry {
  uint32_t hash = 0;
  std::string key, value;
};
struct GuiVariableProbe {
  bool ok = false;
  void* object = nullptr;
  int32_t count = 0;
  uint32_t mask = 0, level = 0;
  double loadFactor = 0;
  std::vector<GuiVariableEntry> entries;
  std::string error;
};
// Read-only probe of the engine GUI variable map ("GetVariableSystem").
// Verifies the build-specific getter signature before calling it, validates all
// pointers with VirtualQuery and never writes to the map. Gated behind an
// explicit debug setting; see DECISIONS.md D17.
GuiVariableProbe ProbeGuiVariables(uint8_t* base);
// Formal UI polling: copy only exact requested keys, resolving hash collisions
// by the full key. Same verified getter/map layout and UI-thread ownership.
GuiVariableProbe ProbeGuiVariableKeys(uint8_t* base,const std::vector<std::string>& keys);
// Reads a second instance of the same string-map template at a static address.
// Used only to validate the entry layout/hash on populated maps; read-only.
GuiVariableProbe ProbeGlobalStringMap(uint8_t* base);
// Debug-only write experiment: inserts or updates one GUI variable through the
// engine map (Murmur3 hash + engine std::string assign). Requires the
// build-specific signatures; see DECISIONS D17/D18.
bool WriteGuiVariableProbe(uint8_t* base, const std::string& key, const std::string& value,
                           std::string* error);
// Inserts or updates a GUI variable string. Used by the native panel text bridge
// for labels/placeholders only; never for fabricated live readings (D18).
bool SetGuiVariable(uint8_t* base, const std::string& key, const std::string& value,
                    std::string* error);
}  // namespace v3
