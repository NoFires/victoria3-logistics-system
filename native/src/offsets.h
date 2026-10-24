// Build-specific profile for Victoria 3 1.13.11 (steam release/1.13.11).
// Every address is an RVA; runtime resolves base + RVA and verifies the file hash
// and the first bytes of each entry point before any call is enabled.
#pragma once
#include <cstdint>

namespace v3 {

constexpr const char* kGameVersion = "1.13.11";
constexpr const char* kExpectedExeSha256 =
    "7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f";

// Discovery source (tools/discover_local.py against the installed executable):
//   RTTI CArrowObject -> constructor, subclass vtables, SetPath, bucket append,
//   render pass, CPdxNewDeleteAllocator singleton.
constexpr uint32_t kRvaArrowCtor          = 0x037FD4D0;  // CArrowObject::CArrowObject(CPdxArrowType const*)
constexpr uint32_t kRvaCreateSupplyArrow  = 0x026D0A20;  // new(0xC0) + base ctor + supply vtable + field 0xB0
constexpr uint32_t kRvaSetPath            = 0x037FD5B0;  // CArrowObject::SetPath(ArrowPath const*)
constexpr uint32_t kRvaOffsetPathPoints   = 0x037FCCD0;  // ribbon edge generation (used by SetPath)
constexpr uint32_t kRvaBuildVertexBuffer  = 0x037FDC50;  // vertex buffer upload (used by SetPath)
constexpr uint32_t kRvaEnsureConstants    = 0x026CEF30;  // per-object constant buffer creation
constexpr uint32_t kRvaGetArrowType       = 0x037E7D60;  // CAssetFactory::GetArrowType(std::string const*)
constexpr uint32_t kRvaBucketAppend       = 0x037FE620;  // append arrow to pass bucket (this, passContext)
constexpr uint32_t kRvaRenderPass         = 0x037FD860;  // arrow render pass (passContext triple)
constexpr uint32_t kRvaAllocator          = 0x0512CDC0;  // CPdxNewDeleteAllocator singleton
// GUI variable system singleton getter ("GetVariableSystem"). Returns a
// non-polymorphic string->string hash map; probe-only for now (see D17).
constexpr uint32_t kRvaVariableSystem     = 0x03A92920;
// Script function registration helper; debug probe only (name, flags, callback).
constexpr uint32_t kRvaScriptRegister     = 0x03AB6D00;
// R99/R100: CPartialDailyTickCommand secondary virtual Execute; observation
// only, opt-in. Completion has not yet authorized the economy collector.
constexpr uint32_t kRvaPartialDailyTick   = 0x0139E220;
constexpr uint32_t kRvaAccessScopeEnd     = 0x033BD750;
constexpr uint32_t kRvaCommandScopeReturn = 0x00CD2BC1;
// R71 static + R72 live catalog/TLS validation; read-only, never call the
// singleton getter (it may initialize TLS and lock). Context-selected slot only.
constexpr uint32_t kRvaGoodsDatabaseSlots = 0x0514E0D8;
constexpr uint32_t kRvaGoodsDatabaseVtable = 0x04517DF0;
constexpr uint32_t kRvaGoodVtable = 0x04517F80;
// R73: original InformationPanelBar.IsPanelOpen and constructor ownership chain.
// Read-only copied state; no RTTI cast/getter or GUI effect is called.
constexpr uint32_t kRvaInterfaceIdlerGlobal = 0x0592D0E0;
constexpr uint32_t kRvaInterfaceIdlerVtable = 0x04431500;
constexpr uint32_t kRvaInterfaceGfxVtable = 0x045B13D8;
constexpr uint32_t kRvaInterfaceHandlerVtable = 0x045AE328;
constexpr uint32_t kRvaMarketPanelVtable = 0x045CC9C0;
constexpr uint32_t kRvaGuiMarketRegistry = 0x059684A0;
constexpr uint32_t kRvaGuiMarketVtable = 0x045850B0;
constexpr uint32_t kRvaGuiGoodsVtable = 0x04552218;
// Same non-polymorphic string-map template, used by another engine subsystem.
// Read-only research probe target; the GUI variable map is the authority.
constexpr uint32_t kRvaGlobalStringMap    = 0x0595BE60;
constexpr uint32_t kRvaStringMapInsert    = 0x0079C0D0;  // insert-or-find(map, out, hash, pair)
constexpr uint32_t kRvaStringAssign        = 0x0061F280;  // std::string::assign(dst, src, size)
// Engine-owned asset loading window and its native VFS. Used to register our
// embedded adapters without launcher/playset entries or modified game files.
constexpr uint32_t kRvaPreEnumerateAssets = 0x03B42800;
constexpr uint32_t kRvaMountData          = 0x03B47C40;
constexpr uint32_t kRvaMountAssetPath     = 0x03B2F4D0;
constexpr uint32_t kRvaRefreshAssetRoots  = 0x03B46FF0;
// These are live index handles, NOT pass numbers. Both vanilla arrow submission
// routines load them into the native context before BucketAppend. See
// 0x26D0596..0x26D05C3 and 0x26D6F6E..0x26D6F9B on this executable.
constexpr uint32_t kRvaArrowSubmitDomain  = 0x0513CB18;
constexpr uint32_t kRvaArrowSubmitStage   = 0x051389D0;

// Object layout observed from the constructor and both constructor wrappers.
constexpr uint32_t kObjectSize            = 0xC0;
constexpr uint32_t kOffType               = 0x50;
constexpr uint32_t kOffPath               = 0x70;
constexpr uint32_t kOffMatrix             = 0x10;
constexpr uint32_t kOffFlags              = 0x5C;
constexpr uint32_t kOffConstantBuffer     = 0x60;
constexpr uint32_t kOffVertexBuffer       = 0x68;
constexpr uint32_t kOffTotalLength        = 0xA8;

// First bytes of each entry point, verified against the running image.
struct Sig { uint32_t rva; uint8_t bytes[8]; const char* name; };
constexpr Sig kSigs[] = {
    {kRvaAccessScopeEnd,    {0x40,0x53,0x48,0x83,0xEC,0x20,0x48,0x8B}, "access-scope-end"},
    {kRvaPartialDailyTick,   {0x40,0x55,0x53,0x56,0x57,0x41,0x54,0x41}, "partial-daily-tick"},
    {kRvaPreEnumerateAssets,{0x44,0x89,0x4C,0x24,0x20,0x4C,0x89,0x44}, "pre-enumerate-assets"},
    {kRvaMountData,         {0x48,0x89,0x5C,0x24,0x08,0x48,0x89,0x74}, "mount-data"},
    {kRvaMountAssetPath,    {0x48,0x89,0x5C,0x24,0x08,0x48,0x89,0x6C}, "mount-asset-path"},
    {kRvaRefreshAssetRoots, {0x40,0x57,0x48,0x83,0xEC,0x20,0x48,0x89}, "refresh-asset-roots"},
    {kRvaCreateSupplyArrow, {0x48,0x89,0x5C,0x24,0x08,0x57,0x48,0x83}, "supply-arrow-create"},
    {kRvaSetPath,           {0x48,0x89,0x5C,0x24,0x08,0x48,0x89,0x6C}, "set-path"},
    {kRvaEnsureConstants,   {0x48,0x89,0x5C,0x24,0x18,0x48,0x89,0x74}, "ensure-constants"},
    {kRvaGetArrowType,      {0x48,0x89,0x5C,0x24,0x08,0x48,0x89,0x6C}, "get-arrow-type"},
    {kRvaBucketAppend,      {0x48,0x89,0x5C,0x24,0x18,0x57,0x48,0x83}, "bucket-append"},
    {kRvaRenderPass,        {0x48,0x81,0xEC,0x78,0x01,0x00,0x00,0x48}, "render-pass"},
    {kRvaBuildVertexBuffer, {0x48,0x89,0x5C,0x24,0x18,0x48,0x89,0x74}, "build-vertex-buffer"},
    {kRvaVariableSystem,    {0x40,0x53,0x48,0x83,0xEC,0x20,0x65,0x48}, "gui-variable-system"},
    {kRvaStringMapInsert,   {0x48,0x89,0x5C,0x24,0x08,0x48,0x89,0x74}, "string-map-insert"},
    {kRvaStringAssign,      {0x48,0x89,0x5C,0x24,0x10,0x48,0x89,0x6C}, "string-assign"},
};

}  // namespace v3
