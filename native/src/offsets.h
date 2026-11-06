// Victoria 3 1.15.0 profile. All entry RVAs and their first bytes are verified
// against SHA256 473186e8d8deba99...; every address is resolved as base + RVA
// and re-verified at load. The capture hook, renderer, GUI and VFS entries are
// the 1.15 build; the original trade-path selector keeps its old adapter
// disabled because 1.15 changed it to (candidate descriptor, hash).
#pragma once
#include <cstdint>

namespace v3 {

constexpr bool kNativeProfileComplete = true; // 1.15 entry signatures verified
constexpr bool kNativeDiagnosticsVerified = false; // legacy opt-in probes stay off

constexpr const char* kGameVersion = "1.15.0";
constexpr const char* kExpectedExeSha256 =
    "473186e8d8deba993651e0ae4cf02a003b02f5accb99b096a6a0a8bd9506cbaa";

// Discovery source (tools/discover_local.py against the installed executable):
//   RTTI CArrowObject -> constructor, subclass vtables, SetPath, bucket append,
//   render pass, CPdxNewDeleteAllocator singleton.
constexpr uint32_t kRvaArrowCtor          = 0x03AA27A0;  // CArrowObject::CArrowObject(CPdxArrowType const*)
constexpr uint32_t kRvaCreateSupplyArrow  = 0x02949AF0;  // new(0xE0) + base ctor + supply vtable + constants 0xB0..0xDA
constexpr uint32_t kRvaSetPath            = 0x03AA2880;  // CArrowObject::SetPath(ArrowPath const*)
constexpr uint32_t kRvaOffsetPathPoints   = 0x03AA1FA0;  // ribbon edge generation (used by SetPath)
constexpr uint32_t kRvaBuildVertexBuffer  = 0x03AA2F20;  // vertex buffer upload (used by SetPath)
constexpr uint32_t kRvaEnsureConstants    = 0x02943310;  // per-object constant buffer creation
constexpr uint32_t kRvaGetArrowType       = 0x03A8CEC0;  // CAssetFactory::GetArrowType(std::string const*)
constexpr uint32_t kRvaBucketAppend       = 0x03AA38F0;  // append arrow to pass bucket (this, passContext)
constexpr uint32_t kRvaRenderPass         = 0x03AA2B30;  // arrow render pass (passContext triple)
constexpr uint32_t kRvaAllocator          = 0x053D8DC0;  // CPdxNewDeleteAllocator singleton
// GUI variable system singleton getter ("GetVariableSystem"). Returns a
// non-polymorphic string->string hash map; probe-only for now (see D17).
constexpr uint32_t kRvaVariableSystem     = 0x03D3B4F0;
// Script function registration helper; debug probe only (name, flags, callback).
constexpr uint32_t kRvaScriptRegister     = 0x03D5F8E0;
// R99/R100: CPartialDailyTickCommand secondary virtual Execute; observation
// only, opt-in. Completion has not yet authorized the economy collector.
constexpr uint32_t kRvaPartialDailyTick   = 0x00C9E210;
constexpr uint32_t kRvaAccessScopeEnd     = 0x036627C0;
constexpr uint32_t kRvaCommandScopeReturn = 0x01078EA6;
// Native world aggregate destructor; machine+120 owns this exact object.
// Called before any contained manager (including state DB at world+1C10).
constexpr uint32_t kRvaDestroyNativeWorld = 0x014A3360;
// Original full world-market path getter: (current world object, source state,
// destination state). It resolves the current trade group, ranks every retained
// candidate with the original cost/strait rules and returns the chosen path.
// Call only inside CopyNativeSnapshot's completed-command access scope.
constexpr uint32_t kRvaSelectTradePath     = 0x00D039C0;
// Original per-state current world-market hub, including its selected/nearest
// cache checks and native fallback. Read-only; no engine lock is taken.
constexpr uint32_t kRvaWorldMarketHub      = 0x00DA2500;
constexpr uint32_t kRvaTradePathCost       = 0x00CFB440;
constexpr uint32_t kRvaTradeStraitCost     = 0x00CFB310;
constexpr uint32_t kRvaTradeStraitLookup   = 0x00B04970;
// R71 static + R72 live catalog/TLS validation; read-only, never call the
// singleton getter (it may initialize TLS and lock). Context-selected slot only.
constexpr uint32_t kRvaGoodsDatabaseSlots = 0x05400BB8;
constexpr uint32_t kRvaGoodsDatabaseVtable = 0x0475F0B8;
constexpr uint32_t kRvaGoodVtable = 0x0475DB30;
// R73: original InformationPanelBar.IsPanelOpen and constructor ownership chain.
// Read-only copied state; no RTTI cast/getter or GUI effect is called.
constexpr uint32_t kRvaInterfaceIdlerGlobal = 0x05C07C18;
constexpr uint32_t kRvaInterfaceIdlerVtable = 0x04678BC0;
constexpr uint32_t kRvaInterfaceGfxVtable = 0x04833458;
constexpr uint32_t kRvaInterfaceHandlerVtable = 0x0482CA08;
constexpr uint32_t kRvaMarketPanelVtable = 0x04849150;
constexpr uint32_t kRvaGuiMarketRegistry = 0x05C43760;
constexpr uint32_t kRvaGuiMarketVtable = 0x0480C528;
constexpr uint32_t kRvaGuiGoodsVtable = 0x0480D108;
// Same non-polymorphic string-map template, used by another engine subsystem.
// Read-only research probe target; the GUI variable map is the authority.
constexpr uint32_t kRvaGlobalStringMap    = 0x0595BE60;
constexpr uint32_t kRvaStringMapInsert    = 0x0083D070;  // insert-or-find(map, out, hash, pair)
constexpr uint32_t kRvaStringAssign        = 0x00677A30;  // std::string::assign(dst, src, size)
// Engine-owned asset loading window and its native VFS. Used to register our
// embedded adapters without launcher/playset entries or modified game files.
constexpr uint32_t kRvaPreEnumerateAssets = 0x03DEB040;
constexpr uint32_t kRvaMountData          = 0x03DF0480;
constexpr uint32_t kRvaMountAssetPath     = 0x03DD7D10;
constexpr uint32_t kRvaRefreshAssetRoots  = 0x03DEF830;
// These are live index handles, NOT pass numbers. Both vanilla arrow submission
// routines load them into the native context before BucketAppend. See
// 0x2945335..0x2945360 and 0x29453AE..0x2945410 on this executable.
constexpr uint32_t kRvaArrowSubmitDomain  = 0x053EAE70;
constexpr uint32_t kRvaArrowSubmitStage   = 0x053E6A70;

// Object layout observed from the constructor and both constructor wrappers.
constexpr uint32_t kObjectSize            = 0x000000E0;
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
    {kRvaWorldMarketHub,    {0x48,0x89,0x5C,0x24,0x10,0x48,0x89,0x6C}, "world-market-hub"},
    {kRvaSelectTradePath,   {0x48,0x89,0x5C,0x24,0x08,0x44,0x89,0x44}, "select-trade-path"},
    {kRvaTradePathCost,     {0x48,0x89,0x5C,0x24,0x10,0x48,0x89,0x6C}, "trade-path-cost"},
    {kRvaTradeStraitCost,   {0x40,0x53,0x55,0x57,0x48,0x83,0xEC,0x20}, "trade-strait-cost"},
    {kRvaTradeStraitLookup, {0x48,0x83,0xEC,0x58,0x48,0x8B,0x05,0xBD}, "trade-strait-lookup"},
    {kRvaAccessScopeEnd,    {0x40,0x53,0x48,0x83,0xEC,0x20,0x48,0x8B}, "access-scope-end"},
    {kRvaDestroyNativeWorld,{0x48,0x89,0x5C,0x24,0x08,0x48,0x89,0x74}, "native-world-destructor"},
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
