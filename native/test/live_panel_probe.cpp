// Read-only live decoder for the native market/state panel chain (diagnostic).
//
// Why: `market_panel.h` / `state_panel.h` gate every decode on
// `machine+0x588 == 4` ("in game"). R73 only ever sampled that chain with the
// panel CLOSED. R84 captured the first open-panel session and the DLL logged
// `panel=4294967295 tab=default`, i.e. available=true but phase!=4. This tool
// replays the exact production decode against the live process and prints the
// raw field values so the gate can be corrected against evidence.
//
// It opens the process with PROCESS_VM_READ only: no writes, no getters, no
// debugger attach, no pause. Nothing here ships in the DLL.

#include <windows.h>
#include <tlhelp32.h>

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "market_panel.h"
#include "state_panel.h"

namespace {

HANDLE g_process = nullptr;

bool ReadMemory(uint64_t address, void* out, size_t size) {
  if (!g_process || !address || !size) return false;
  SIZE_T got = 0;
  if (!ReadProcessMemory(g_process, reinterpret_cast<LPCVOID>(address), out, size, &got)) return false;
  return got == size;
}

uint64_t FindModuleBase(DWORD pid, const wchar_t* name) {
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
  if (snap == INVALID_HANDLE_VALUE) return 0;
  MODULEENTRY32W entry{};
  entry.dwSize = sizeof(entry);
  uint64_t base = 0;
  if (Module32FirstW(snap, &entry)) {
    do {
      if (_wcsicmp(entry.szModule, name) == 0) {
        base = reinterpret_cast<uint64_t>(entry.modBaseAddr);
        break;
      }
    } while (Module32NextW(snap, &entry));
  }
  CloseHandle(snap);
  return base;
}

// Same MSVC small-string layout the production headers accept.
bool ReadMsvcString(uint64_t object, uint64_t offset, std::string& out) {
  unsigned char bytes[32] = {};
  if (!ReadMemory(object + offset, bytes, sizeof(bytes))) return false;
  uint64_t size = 0, capacity = 0;
  std::memcpy(&size, bytes + 16, 8);
  std::memcpy(&capacity, bytes + 24, 8);
  std::printf("      raw[%3llu] size=%llu cap=%llu", static_cast<unsigned long long>(offset),
              static_cast<unsigned long long>(size), static_cast<unsigned long long>(capacity));
  if (!size || size > 128 || size > capacity || (capacity < 16 && capacity != 15)) {
    std::printf("  (not a valid string)\n");
    return false;
  }
  std::string value(size + 1, '\0');
  if (capacity < 16) {
    std::memcpy(value.data(), bytes, value.size());
  } else {
    uint64_t pointer = 0;
    std::memcpy(&pointer, bytes, 8);
    if (!ReadMemory(pointer, value.data(), value.size())) {
      std::printf("  (unreadable heap string)\n");
      return false;
    }
  }
  if (value.back() != 0) {
    std::printf("  (unterminated)\n");
    return false;
  }
  value.pop_back();
  std::printf("  -> \"%s\"\n", value.c_str());
  out = value;
  return true;
}

template <typename T>
void ShowField(uint64_t object, uint64_t offset, const char* label) {
  T value{};
  if (!object || !ReadMemory(object + offset, &value, sizeof(value))) {
    std::printf("  %-22s @0x%llx unreadable\n", label, static_cast<unsigned long long>(offset));
    return;
  }
  if constexpr (sizeof(T) == 8) {
    std::printf("  %-22s @0x%llx = 0x%016llx\n", label, static_cast<unsigned long long>(offset),
                static_cast<unsigned long long>(value));
  } else {
    std::printf("  %-22s @0x%llx = 0x%x (%d)\n", label, static_cast<unsigned long long>(offset),
                static_cast<unsigned>(value), static_cast<int>(value));
  }
}

bool VtableMatches(uint64_t object, uint64_t module, uint32_t rva, const char* label) {
  uint64_t vtable = 0;
  if (!object || !ReadMemory(object, &vtable, sizeof(vtable))) {
    std::printf("  %-22s unreadable\n", label);
    return false;
  }
  const uint64_t expected = module + rva;
  std::printf("  %-22s = 0x%016llx  expected 0x%016llx  %s\n", label,
              static_cast<unsigned long long>(vtable), static_cast<unsigned long long>(expected),
              vtable == expected ? "MATCH" : "MISMATCH");
  return vtable == expected;
}

}  // namespace

int wmain(int argc, wchar_t** argv) {
  // Optional argv[2]: comma/space separated simulation market ids from the DLL log
  // ("real economy market ids:"). Used to search the GUI market object for the link.
  std::vector<uint32_t> snapshotMarkets;
  if (argc > 2) {
    const std::wstring raw(argv[2]);
    std::wstring current;
    for (wchar_t c : raw) {
      if (c == L',' || c == L' ' || c == L'\t') {
        if (!current.empty()) {
          snapshotMarkets.push_back(static_cast<uint32_t>(_wtoi(current.c_str())));
          current.clear();
        }
      } else {
        current.push_back(c);
      }
    }
    if (!current.empty()) snapshotMarkets.push_back(static_cast<uint32_t>(_wtoi(current.c_str())));
    std::sort(snapshotMarkets.begin(), snapshotMarkets.end());
    std::printf("snapshot market ids supplied: %zu\n", snapshotMarkets.size());
  }
  DWORD pid = 0;
  if (argc > 1) pid = static_cast<DWORD>(_wtoi(argv[1]));
  if (!pid) {
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
      std::printf("FAIL: cannot enumerate processes\n");
      return 1;
    }
    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(entry);
    if (Process32FirstW(snap, &entry)) {
      do {
        if (_wcsicmp(entry.szExeFile, L"victoria3.exe") == 0) {
          pid = entry.th32ProcessID;
          break;
        }
      } while (Process32NextW(snap, &entry));
    }
    CloseHandle(snap);
  }
  if (!pid) {
    std::printf("FAIL: victoria3.exe is not running\n");
    return 1;
  }

  g_process = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);
  if (!g_process) {
    std::printf("FAIL: OpenProcess(%lu) error %lu\n", pid, GetLastError());
    return 1;
  }
  const uint64_t module = FindModuleBase(pid, L"victoria3.exe");
  std::printf("pid=%lu module_base=0x%016llx\n", pid, static_cast<unsigned long long>(module));
  if (!module) {
    std::printf("FAIL: victoria3.exe module base not found\n");
    return 1;
  }

  // --- raw chain, values printed before any judgement -----------------------
  std::printf("\n[1] interface idler\n");
  uint64_t idler = 0;
  ReadMemory(module + v3::kRvaInterfaceIdlerGlobal, &idler, sizeof(idler));
  std::printf("  idler@module+0x%X = 0x%016llx\n", v3::kRvaInterfaceIdlerGlobal,
              static_cast<unsigned long long>(idler));
  VtableMatches(idler, module, v3::kRvaInterfaceIdlerVtable, "idler vtable");

  uint64_t machine = 0;
  ReadMemory(idler + 0x18, &machine, sizeof(machine));
  std::printf("\n[2] machine (idler+0x18) = 0x%016llx\n", static_cast<unsigned long long>(machine));
  VtableMatches(machine, module, 0, "machine vtable(raw)");

  std::printf("\n[3] phase candidate around machine+0x588\n");
  for (uint64_t offset = 0x560; offset <= 0x5B0; offset += 4) {
    int32_t value = 0;
    if (!ReadMemory(machine + offset, &value, sizeof(value))) continue;
    if (offset == 0x588) {
      std::printf("  machine+0x%llx = %d   <== production gate (phase==4)\n",
                  static_cast<unsigned long long>(offset), value);
    } else if (value >= 0 && value <= 16) {
      std::printf("  machine+0x%llx = %d\n", static_cast<unsigned long long>(offset), value);
    }
  }

  int32_t phase = 0;
  ReadMemory(machine + 0x588, &phase, sizeof(phase));
  std::printf("\n[4] gfx/handler chain (walked regardless of the phase gate)\n");
  uint64_t gfx = 0, handler = 0, bar = 0, panels = 0;
  int32_t panelCount = 0;
  ReadMemory(idler + 0x10, &gfx, sizeof(gfx));
  std::printf("  gfx (idler+0x10)       = 0x%016llx\n", static_cast<unsigned long long>(gfx));
  VtableMatches(gfx, module, v3::kRvaInterfaceGfxVtable, "gfx vtable");
  ReadMemory(gfx + 0x20, &handler, sizeof(handler));
  std::printf("  handler (gfx+0x20)     = 0x%016llx\n", static_cast<unsigned long long>(handler));
  VtableMatches(handler, module, v3::kRvaInterfaceHandlerVtable, "handler vtable");
  uint64_t barSelf = 0;
  ReadMemory(handler + 0xd8, &barSelf, sizeof(barSelf));
  std::printf("  bar self (handler+d8)  = 0x%016llx  expected 0x%016llx  %s\n",
              static_cast<unsigned long long>(barSelf),
              static_cast<unsigned long long>(handler + 0xd8),
              barSelf == handler + 0xd8 ? "MATCH" : "MISMATCH");
  ReadMemory(handler + 0xe0, &panels, sizeof(panels));
  ReadMemory(handler + 0xec, &panelCount, sizeof(panelCount));
  std::printf("  panels (handler+e0)    = 0x%016llx\n", static_cast<unsigned long long>(panels));
  std::printf("  panel count (ec)       = %d   (production expects 56)\n", panelCount);

  std::printf("\n[5] panel table: vtable identity + panel name per slot\n");
  for (int index = 0; index < panelCount && index < 64; ++index) {
    uint64_t panel = 0;
    if (!ReadMemory(panels + static_cast<uint64_t>(index) * 8, &panel, sizeof(panel))) continue;
    uint64_t vtable = 0;
    if (!panel || !ReadMemory(panel, &vtable, sizeof(vtable))) continue;
    uint32_t rva = 0;
    bool known = false;
    if (vtable >= module && vtable - module <= 0x800000) {
      rva = static_cast<uint32_t>(vtable - module);
      known = rva == v3::kRvaMarketPanelVtable || rva == 0x045CC848;
    }
    if (!known && index != 9 && index != 14) continue;
    std::printf("  slot %2d panel=0x%016llx vtable_rva=0x%08X%s%s\n", index,
                static_cast<unsigned long long>(panel), rva,
                rva == v3::kRvaMarketPanelVtable ? "  [market_panel]" : "",
                rva == 0x045CC848 ? "  [states_panel]" : "");
    std::string name;
    if (ReadMsvcString(panel, 8, name)) std::printf("        name=\"%s\"\n", name.c_str());
    std::string tab;
    if (ReadMsvcString(panel, 0x30, tab)) std::printf("        tab=\"%s\"\n", tab.c_str());
    ShowField<uint32_t>(panel, 0x70, "panel id (0x70)");
    if (barSelf) {
      ShowField<int32_t>(barSelf, 0x2c, "history count");
      ShowField<uint8_t>(barSelf, 0x40, "closed");
      ShowField<uint64_t>(barSelf, 0x20, "history data");
    }
  }

  // --- market identity cross-check -----------------------------------------
  // The owner refuses when panel.market is absent from the snapshot's market
  // set (state+0x48 ids). Dump plausible-id fields so an offset error can be
  // told apart from a genuinely distinct market id.
  std::printf("\n[5b] market_panel dword scan (plausible ids 1..4095, markers 33/35/112)\n");
  uint64_t marketPanel = 0;
  if (panels) ReadMemory(panels + 9 * 8, &marketPanel, sizeof(marketPanel));
  if (marketPanel) {
    std::printf("  market_panel=0x%016llx\n", static_cast<unsigned long long>(marketPanel));
    for (uint64_t offset = 0; offset < 0x200; offset += 4) {
      uint32_t value = 0;
      if (!ReadMemory(marketPanel + offset, &value, sizeof(value))) break;
      if (value == 0 || value > 4095) continue;
      const char* note = "";
      if (value == 33) note = "  <== top market 33 (primary)";
      else if (value == 35) note = "  <== top market 35";
      else if (value == 112) note = "  <== top market 112";
      std::printf("  +0x%03llx = %u%s\n", static_cast<unsigned long long>(offset), value, note);
    }
    int32_t typedId = 0;
    ReadMemory(marketPanel + 0x70, &typedId, sizeof(typedId));
    std::printf("  panel+0x70 as int32 = %d\n", typedId);
  } else {
    std::printf("  market_panel slot unreadable\n");
  }

  // --- GUI market registry: enumerate the engine-side market ids -------------
  // Layout taken from native_goods_filter.h (registry -> entries, object+0x10=id).
  std::printf("\n[5c] GUI market registry enumeration\n");
  uint64_t registry = 0;
  if (!ReadMemory(module + v3::kRvaGuiMarketRegistry, &registry, sizeof(registry)) || !registry) {
    std::printf("  registry global unreadable\n");
  } else {
    uint64_t entries = 0;
    int32_t capacity = 0, count = 0;
    uint32_t lock = 0;
    ReadMemory(registry, &entries, sizeof(entries));
    ReadMemory(registry + 8, &capacity, sizeof(capacity));
    ReadMemory(registry + 12, &count, sizeof(count));
    ReadMemory(registry + 0x18, &lock, sizeof(lock));
    std::printf("  registry=0x%016llx entries=0x%016llx count=%d capacity=%d lock=0x%x\n",
                static_cast<unsigned long long>(registry), static_cast<unsigned long long>(entries),
                count, capacity, lock);
    if (count >= 0 && count <= 2048 && entries) {
      std::vector<uint32_t> ids;
      ids.reserve(static_cast<size_t>(count));
      for (int32_t i = 0; i < count; ++i) {
        uint64_t object = 0, vtable = 0;
        uint32_t id = 0;
        if (!ReadMemory(entries + static_cast<uint64_t>(i) * 8, &object, sizeof(object)) || !object) continue;
        if (!ReadMemory(object, &vtable, sizeof(vtable))) continue;
        ReadMemory(object + 0x10, &id, sizeof(id));
        const bool vtOk = vtable == module + v3::kRvaGuiMarketVtable;
        ids.push_back(id);
        if (!vtOk) std::printf("  entry %2d id=%u VTABLE_MISMATCH\n", i, id);
      }
      std::sort(ids.begin(), ids.end());
      std::printf("  ids(count=%zu):", ids.size());
      for (size_t i = 0; i < ids.size(); ++i) std::printf(" %u", ids[i]);
      std::printf("\n");
      const uint32_t snapshotMarkers[] = {33u, 35u, 112u};
      for (uint32_t marker : snapshotMarkers)
        std::printf("  snapshot market %u present in GUI registry: %s\n", marker,
                    std::binary_search(ids.begin(), ids.end(), marker) ? "YES" : "NO");
      uint32_t panelMarket = 0;
      if (marketPanel) ReadMemory(marketPanel + 0x70, &panelMarket, sizeof(panelMarket));
      std::printf("  panel market %u present in GUI registry: %s\n", panelMarket,
                  std::binary_search(ids.begin(), ids.end(), panelMarket) ? "YES" : "NO");
      // The entry the panel resolves: look for links into the simulation side.
      for (int32_t i = 0; i < count; ++i) {
        uint64_t object = 0;
        uint32_t id = 0;
        if (!ReadMemory(entries + static_cast<uint64_t>(i) * 8, &object, sizeof(object)) || !object) continue;
        ReadMemory(object + 0x10, &id, sizeof(id));
        if (id != panelMarket) continue;
        std::printf("  -> panel market object=0x%016llx\n", static_cast<unsigned long long>(object));
        for (uint64_t offset = 0; offset < 0x400; offset += 4) {
          uint32_t value = 0;
          if (!ReadMemory(object + offset, &value, sizeof(value))) break;
          if (value == 0 || value > 4095 || offset == 0x10) continue;
          const char* note = "";
          if (value == 33) note = "  <== snapshot market 33";
          else if (value == 35) note = "  <== snapshot market 35";
          else if (value == 112) note = "  <== snapshot market 112";
          std::printf("     +0x%03llx = %u%s\n", static_cast<unsigned long long>(offset), value, note);
        }
        break;
      }
      // Cross-check: is object+0x374 a simulation market id for every entry?      std::printf("  --- gui_id -> object+0x374 (candidate simulation market id) ---\n");
      for (int32_t i = 0; i < count; ++i) {
        uint64_t object = 0, vtable = 0;
        uint32_t id = 0, candidate = 0;
        if (!ReadMemory(entries + static_cast<uint64_t>(i) * 8, &object, sizeof(object)) || !object) continue;
        if (!ReadMemory(object, &vtable, sizeof(vtable))) continue;
        if (vtable != module + v3::kRvaGuiMarketVtable) continue;
        ReadMemory(object + 0x10, &id, sizeof(id));
        if (!ReadMemory(object + 0x374, &candidate, sizeof(candidate))) continue;
        const char* note = "";
        if (candidate == 33) note = "  <== snapshot market 33 (primary)";
        else if (candidate == 35) note = "  <== snapshot market 35";
        else if (candidate == 112) note = "  <== snapshot market 112";
        std::printf("   gui_id=%-4u sim_id_field=%-6u%s\n", id, candidate, note);
      }
      // Structure around the panel's market object: which fields are pointers,
      // and does any of them lead to an object carrying the simulation id?
      if (marketPanel) {
        uint64_t panelMarketId = 0;
        ReadMemory(marketPanel + 0x70, &panelMarketId, sizeof(uint32_t));
        for (int32_t i = 0; i < count; ++i) {
          uint64_t object = 0;
          uint32_t id = 0;
          if (!ReadMemory(entries + static_cast<uint64_t>(i) * 8, &object, sizeof(object)) || !object) continue;
          ReadMemory(object + 0x10, &id, sizeof(id));
          if (id != static_cast<uint32_t>(panelMarketId)) continue;
          std::printf("  --- pointer fields of panel market object 0x%016llx ---\n",
                      static_cast<unsigned long long>(object));
          for (uint64_t offset = 0; offset < 0x400; offset += 8) {
            uint64_t value = 0;
            if (!ReadMemory(object + offset, &value, sizeof(value))) break;
            if (value < 0x10000 || value > 0x00007FFFFFFFFFFFULL || (value & 7)) continue;
            uint64_t targetVtable = 0;
            uint32_t targetId8 = 0;
            const bool readable = ReadMemory(value, &targetVtable, sizeof(targetVtable));
            ReadMemory(value + 8, &targetId8, sizeof(targetId8));
            char vtableNote[64] = "";
            if (readable && targetVtable >= module && targetVtable - module < 0x800000)
              std::snprintf(vtableNote, sizeof(vtableNote), " vt_rva=0x%08X",
                            static_cast<uint32_t>(targetVtable - module));
            else if (readable)
              std::snprintf(vtableNote, sizeof(vtableNote), " vt=0x%llx (not module)",
                            static_cast<unsigned long long>(targetVtable));
            std::printf("   +0x%03llx -> 0x%016llx  dword@+8=%u%s\n",
                        static_cast<unsigned long long>(offset),
                        static_cast<unsigned long long>(value), targetId8, vtableNote);
          }
          break;
        }
      }
    }
  }

  // --- state panel -> state object (to cross-check the market id) -----------
  std::printf("\n[8] states_panel pointer scan for a state object (int@+8 in 1..3000)\n");
  uint64_t statesPanel = 0;
  if (panels) ReadMemory(panels + 14 * 8, &statesPanel, sizeof(statesPanel));
  if (statesPanel) {
    uint32_t panelStateId = 0;
    ReadMemory(statesPanel + 0x70, &panelStateId, sizeof(panelStateId));
    std::printf("  states_panel=0x%016llx panel state id=0x%x (%u)\n",
                static_cast<unsigned long long>(statesPanel), panelStateId, panelStateId);
    for (uint64_t offset = 0; offset < 0x300; offset += 8) {
      uint64_t value = 0;
      if (!ReadMemory(statesPanel + offset, &value, sizeof(value))) break;
      if (value < 0x10000 || value > 0x00007FFFFFFFFFFFULL || (value & 7)) continue;
      int32_t candidateId = 0;
      uint32_t marketField = 0;
      if (!ReadMemory(value + 8, &candidateId, sizeof(candidateId))) continue;
      if (candidateId < 1 || candidateId > 3000) continue;
      ReadMemory(value + 0x48, &marketField, sizeof(marketField));
      const char* note = panelStateId != UINT32_MAX && static_cast<uint32_t>(candidateId) == panelStateId
                             ? "   <== matches panel state id" : "";
      std::printf("   states_panel+0x%03llx -> 0x%016llx id@+8=%-5d market@+0x48=%-6u%s\n",
                  static_cast<unsigned long long>(offset),
                  static_cast<unsigned long long>(value), candidateId, marketField, note);
    }
  } else {
    std::printf("  states_panel slot unreadable\n");
  }

  // --- [9] search the GUI market object for the simulation market link ------
  if (!snapshotMarkets.empty() && marketPanel) {
    std::printf("\n[9] market object search for simulation market id membership\n");
    const auto member=[&](uint32_t value){
      return std::binary_search(snapshotMarkets.begin(),snapshotMarkets.end(),value);
    };
    uint64_t object=0;
    uint64_t reg=0,entries2=0;int32_t count2=0;
    if (ReadMemory(module+v3::kRvaGuiMarketRegistry,&reg,sizeof(reg)) && reg &&
        ReadMemory(reg,&entries2,sizeof(entries2)) && ReadMemory(reg+12,&count2,sizeof(count2))) {
      uint32_t panelMarket=0;
      ReadMemory(marketPanel+0x70,&panelMarket,sizeof(panelMarket));
      for (int32_t i=0;i<count2 && i<2048;++i) {
        uint64_t candidate=0;uint32_t id=0;
        if (!ReadMemory(entries2+uint64_t(i)*8,&candidate,sizeof(candidate)) || !candidate) continue;
        ReadMemory(candidate+0x10,&id,sizeof(id));
        if (id==panelMarket) { object=candidate;break; }
      }
    }
    if (!object) {
      std::printf("  panel market object not found\n");
    } else {
      std::printf("  object=0x%016llx\n",static_cast<unsigned long long>(object));
      for (uint64_t offset=0;offset<0x600;offset+=4) {
        uint32_t value=0;
        if (!ReadMemory(object+offset,&value,sizeof(value))) break;
        if (member(value))
          std::printf("   self +0x%03llx = %u  <== snapshot market\n",
                      static_cast<unsigned long long>(offset),value);
      }
      for (uint64_t offset=0;offset<0x600;offset+=8) {
        uint64_t target=0;
        if (!ReadMemory(object+offset,&target,sizeof(target))) break;
        if (target<0x10000 || target>0x00007FFFFFFFFFFFULL || (target&7)) continue;
        for (uint64_t inner=0;inner<0x80;inner+=4) {
          uint32_t value=0;
          if (!ReadMemory(target+inner,&value,sizeof(value))) break;
          if (member(value))
            std::printf("   +0x%03llx -> 0x%016llx dword@+0x%02llx = %u  <== snapshot market\n",
                        static_cast<unsigned long long>(offset),
                        static_cast<unsigned long long>(target),
                        static_cast<unsigned long long>(inner),value);
        }
      }
    }
  }

  // --- [10] vector-like members of the market object (state roster?) ---------
  if (marketPanel) {
    std::printf("\n[10] market object vector-like members (ptr@o, capacity@o+8, count@o+12)\n");
    uint64_t object = 0;
    {
      uint64_t reg = 0, entries2 = 0;
      int32_t count2 = 0;
      if (ReadMemory(module + v3::kRvaGuiMarketRegistry, &reg, sizeof(reg)) && reg &&
          ReadMemory(reg, &entries2, sizeof(entries2)) && ReadMemory(reg + 12, &count2, sizeof(count2))) {
        uint32_t panelMarket = 0;
        ReadMemory(marketPanel + 0x70, &panelMarket, sizeof(panelMarket));
        for (int32_t i = 0; i < count2 && i < 2048; ++i) {
          uint64_t candidate = 0;
          uint32_t id = 0;
          if (!ReadMemory(entries2 + uint64_t(i) * 8, &candidate, sizeof(candidate)) || !candidate) continue;
          ReadMemory(candidate + 0x10, &id, sizeof(id));
          if (id == panelMarket) { object = candidate; break; }
        }
      }
    }
    if (object) {
      for (uint64_t offset = 0; offset < 0x600; offset += 8) {
        uint64_t data = 0;
        int32_t capacity = 0, count = 0;
        if (!ReadMemory(object + offset, &data, sizeof(data))) break;
        if (!ReadMemory(object + offset + 8, &capacity, sizeof(capacity))) break;
        if (!ReadMemory(object + offset + 12, &count, sizeof(count))) break;
        if (!data || count <= 0 || count > 400 || capacity < count || capacity > 4000) continue;
        if (data < 0x10000 || data > 0x00007FFFFFFFFFFFULL || (data & 3)) continue;
        std::vector<uint32_t> values(static_cast<size_t>(count));
        if (!ReadMemory(data, values.data(), count * 4)) continue;
        int32_t smallest = INT32_MAX, largest = 0;
        bool plausible = true;
        for (uint32_t v : values) {
          if (v == 0 || v > 3000) { plausible = false; break; }
          smallest = std::min(smallest, static_cast<int32_t>(v));
          largest = std::max(largest, static_cast<int32_t>(v));
        }
        if (!plausible) continue;
        std::printf("   +0x%03llx vector count=%d cap=%d range=%d..%d  first:",
                    static_cast<unsigned long long>(offset), count, capacity, smallest, largest);
        for (size_t i = 0; i < values.size() && i < 12; ++i) std::printf(" %u", values[i]);
        std::printf("\n");
      }
    } else {
      std::printf("  market object not found\n");
    }
  }

  // --- [11] which GUI market field carries the simulation market id? --------
  // Structural test, mirroring the state-side probe: enumerate every GUI market
  // object, build slots from its own dwords plus the first dwords of each of its
  // pointer targets, and keep the slots that (a) are distinct across the GUI ids
  // and (b) land inside the snapshot market id set supplied on the command line.
  std::printf("\n[11] GUI market objects: slot -> simulation market id?\n");
  {
    constexpr int kOwnDwords = 0x400 / 4;      // 256
    constexpr int kPointerSlots = 0x400 / 8;   // 128
    constexpr int kTargetDwords = 16;
    constexpr int kSlotCount = kOwnDwords + kPointerSlots * kTargetDwords;
    uint32_t panelId = 0;
    if (marketPanel) ReadMemory(marketPanel + 0x70, &panelId, sizeof(panelId));
    struct Entry { uint32_t id; uint64_t object; };
    std::vector<Entry> objects;
    {
      uint64_t reg = 0, entries2 = 0;
      int32_t count2 = 0;
      if (ReadMemory(module + v3::kRvaGuiMarketRegistry, &reg, sizeof(reg)) && reg &&
          ReadMemory(reg, &entries2, sizeof(entries2)) && ReadMemory(reg + 12, &count2, sizeof(count2)) &&
          count2 > 0 && count2 <= 2048) {
        for (int32_t i = 0; i < count2; ++i) {
          uint64_t candidate = 0;
          uint32_t id = 0;
          if (!ReadMemory(entries2 + uint64_t(i) * 8, &candidate, sizeof(candidate)) || !candidate) continue;
          if (!ReadMemory(candidate + 0x10, &id, sizeof(id))) continue;
          objects.push_back({id, candidate});
        }
      }
    }
    std::printf("  objects=%zu snapshot ids=%zu panel=%u\n", objects.size(), snapshotMarkets.size(), panelId);
    if (objects.size() >= 3) {
      std::vector<std::vector<uint32_t>> rows(objects.size(), std::vector<uint32_t>(kSlotCount, 0));
      for (size_t i = 0; i < objects.size(); ++i) {
        auto& row = rows[i];
        std::vector<uint32_t> own(kOwnDwords, 0);
        if (!ReadMemory(objects[i].object, own.data(), own.size() * 4)) continue;
        for (int d = 0; d < kOwnDwords; ++d) row[d] = own[d];
        for (int p = 0; p < kPointerSlots; ++p) {
          uint64_t target = 0;
          if (!ReadMemory(objects[i].object + uint64_t(p) * 8, &target, sizeof(target))) break;
          if (!target || target < 0x10000 || target > 0x00007FFFFFFFFFFFULL || (target & 7)) continue;
          uint32_t values[kTargetDwords] = {0};
          if (!ReadMemory(target, values, sizeof(values))) continue;
          for (int d = 0; d < kTargetDwords; ++d)
            row[kOwnDwords + p * kTargetDwords + d] = values[d];
        }
      }
      struct Candidate { int slot; size_t count; size_t inSet; uint32_t panelValue; };
      std::vector<Candidate> candidates;
      for (int slot = 0; slot < kSlotCount; ++slot) {
        std::vector<uint32_t> seen;
        seen.reserve(objects.size());
        size_t present = 0, inSet = 0;
        bool distinct = true;
        uint32_t panelValue = 0;
        for (size_t i = 0; i < objects.size(); ++i) {
          const uint32_t value = rows[i][slot];
          if (!value) continue;
          ++present;
          if (std::find(seen.begin(), seen.end(), value) != seen.end()) { distinct = false; break; }
          seen.push_back(value);
          if (!snapshotMarkets.empty() &&
              std::binary_search(snapshotMarkets.begin(), snapshotMarkets.end(), value)) ++inSet;
          if (marketPanel && objects[i].id == panelId) panelValue = value;
        }
        if (!distinct || present < 3) continue;
        // Report anything that separates the GUI ids; rank by snapshot membership.
        if (!snapshotMarkets.empty() && inSet == 0) continue;
        candidates.push_back({slot, present, inSet, panelValue});
      }
      std::sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
        if (a.inSet != b.inSet) return a.inSet > b.inSet;
        if (a.count != b.count) return a.count > b.count;
        return a.slot < b.slot;
      });
      std::printf("  candidates=%zu (slots distinct across GUI ids and matching snapshot ids)\n",
                  candidates.size());
      for (size_t k = 0; k < candidates.size() && k < 12; ++k) {
        const Candidate& c = candidates[k];
        if (c.slot < kOwnDwords)
          std::printf("    own +0x%03x              objects=%zu in-snapshot=%zu panel=%u\n",
                      c.slot * 4, c.count, c.inSet, c.panelValue);
        else {
          const int rel = c.slot - kOwnDwords;
          std::printf("    ptr +0x%03x -> +0x%02x      objects=%zu in-snapshot=%zu panel=%u\n",
                      (rel / kTargetDwords) * 8, (rel % kTargetDwords) * 4, c.count, c.inSet, c.panelValue);
        }
      }
      // vtable RVAs of the panel object's pointer targets: identifies the target type.
      for (size_t i = 0; i < objects.size(); ++i) {
        if (!marketPanel || objects[i].id != panelId) continue;
        std::printf("  panel object=0x%016llx pointer targets:\n",
                    static_cast<unsigned long long>(objects[i].object));
        for (int p = 0; p < 0x40; ++p) {
          uint64_t target = 0;
          if (!ReadMemory(objects[i].object + uint64_t(p) * 8, &target, sizeof(target))) break;
          if (!target || target < 0x10000 || target > 0x00007FFFFFFFFFFFULL || (target & 7)) continue;
          uint64_t vt = 0;
          if (!ReadMemory(target, &vt, sizeof(vt))) continue;
          std::printf("    +0x%03x -> 0x%016llx vtable=module+0x%llX\n", p * 8,
                      static_cast<unsigned long long>(target),
                      static_cast<unsigned long long>(vt > module ? vt - module : vt));
        }
      }
    }
  }

  // --- [12] candidate simulation market object behind the GUI market ---------
  // The GUI market object points at module objects whose vtables sit next to the
  // documented GUI market vtable (0x45850B0); dump those targets fully and look
  // for a state roster (vector of ids in 1..2048) or a small market index.
  std::printf("\n[12] candidate simulation market object behind the panel market\n");
  if (marketPanel) {
    uint32_t panelId12 = 0;
    ReadMemory(marketPanel + 0x70, &panelId12, sizeof(panelId12));
    uint64_t object = 0;
    {
      uint64_t reg = 0, entries3 = 0;
      int32_t count3 = 0;
      if (ReadMemory(module + v3::kRvaGuiMarketRegistry, &reg, sizeof(reg)) && reg &&
          ReadMemory(reg, &entries3, sizeof(entries3)) && ReadMemory(reg + 12, &count3, sizeof(count3))) {
        for (int32_t i = 0; i < count3 && i < 2048; ++i) {
          uint64_t candidate = 0;
          uint32_t id = 0;
          if (!ReadMemory(entries3 + uint64_t(i) * 8, &candidate, sizeof(candidate)) || !candidate) continue;
          ReadMemory(candidate + 0x10, &id, sizeof(id));
          if (id == panelId12) { object = candidate; break; }
        }
      }
    }
    if (!object) {
      std::printf("  panel market object not found\n");
    } else {
      for (int p = 0; p < 0x40; ++p) {
        uint64_t target = 0;
        if (!ReadMemory(object + uint64_t(p) * 8, &target, sizeof(target))) break;
        if (!target || target < 0x10000 || target > 0x00007FFFFFFFFFFFULL || (target & 7)) continue;
        uint64_t vt = 0;
        if (!ReadMemory(target, &vt, sizeof(vt))) continue;
        // Only module-owned objects carry a real vtable.
        if (vt < module || vt > module + 0x08000000ULL) continue;
        const uint64_t rva = vt - module;
        std::printf("  object +0x%03x -> 0x%016llx vtable=module+0x%llX\n", p * 8,
                    static_cast<unsigned long long>(target), static_cast<unsigned long long>(rva));
        uint32_t dwords[0x100] = {0};
        if (!ReadMemory(target, dwords, sizeof(dwords))) continue;
        std::printf("    small dwords:");
        for (int d = 2; d < 0x100; ++d) {  // skip the vtable itself (qword at +0..+7)
          const uint32_t v = dwords[d];
          if (v == 0 || v > 4096) continue;
          std::printf(" +0x%03x=%u", d * 4, v);
        }
        std::printf("\n");
        for (int f = 0; f < 0x40; ++f) {
          uint64_t data = 0;
          int32_t capacity = 0, count = 0;
          if (!ReadMemory(target + uint64_t(f) * 8, &data, sizeof(data))) break;
          if (!data || data < 0x10000 || data > 0x00007FFFFFFFFFFFULL || (data & 3)) continue;
          if (!ReadMemory(target + uint64_t(f) * 8 + 8, &capacity, sizeof(capacity))) continue;
          if (!ReadMemory(target + uint64_t(f) * 8 + 12, &count, sizeof(count))) continue;
          if (count <= 0 || count > 2048 || capacity < count || capacity > 8192) continue;
          std::vector<uint32_t> values(static_cast<size_t>(count));
          if (!ReadMemory(data, values.data(), count * 4)) continue;
          int32_t smallest = INT32_MAX, largest = 0;
          bool plausible = true;
          for (uint32_t v : values) {
            if (v > 4096) { plausible = false; break; }
            smallest = std::min(smallest, static_cast<int32_t>(v));
            largest = std::max(largest, static_cast<int32_t>(v));
          }
          if (!plausible) continue;
          std::printf("    vector +0x%03x count=%d cap=%d range=%d..%d first:",
                      f * 8, count, capacity, smallest, largest);
          for (size_t i = 0; i < values.size() && i < 10; ++i) std::printf(" %u", values[i]);
          std::printf("\n");
        }
      }
    }
  }

  // --- production decoders, unchanged --------------------------------------
  const v3::GoodsMemoryReader reader = [](uint64_t address, void* out, size_t size) {
    return ReadMemory(address, out, size);
  };  const v3::NativeMarketPanelState market = v3::DecodeNativeMarketPanel(reader, module);
  std::printf("\n[6] DecodeNativeMarketPanel\n");
  std::printf("  available=%d ingame=%d marketOpen=%d market=%u tab=\"%s\" error=\"%s\" visible=%s\n",
              static_cast<int>(market.available), static_cast<int>(market.ingame),
              static_cast<int>(market.marketOpen), market.market, market.tab.c_str(),
              market.error.c_str(),
              market.visibleGoods ? std::to_string(market.visibleGoods->size()).c_str() : "none");

  const v3::NativeStatePanelState state = v3::DecodeNativeStatePanel(reader, module);
  std::printf("\n[7] DecodeNativeStatePanel\n");
  std::printf("  available=%d ingame=%d stateOpen=%d state=%u tab=\"%s\" error=\"%s\"\n",
              static_cast<int>(state.available), static_cast<int>(state.ingame),
              static_cast<int>(state.stateOpen), state.state, state.tab.c_str(), state.error.c_str());

  std::printf("\nphase(machine+0x588)=%d\n", phase);
  CloseHandle(g_process);
  return 0;
}
