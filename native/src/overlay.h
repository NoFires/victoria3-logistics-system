#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <atomic>
#include <mutex>
#include <cstdint>
#include "mutex.h"
#include <string>
#include <vector>
#include <memory>
#include <set>
#include <array>

#include "game.h"
#include "arrow_scope.h"
#include "publish_key.h"
#include "route_data.h"
#include "preview_worker.h"
#include "ribbon_cache.h"
#include "logistics_panel.h"
#include "logistics_session.h"
#include "native_market_owner.h"
#include "native_state_owner.h"
#include "state_region.h"
#include "native_snapshot.h"

namespace v3 {

struct Settings {
  std::string arrowType = "ironroadBuildingOutput";
  float heightScale = 0.0f;
  float heightOffset = 0.0f;
  float coordScale = 1.0f;
  bool showWorldCheck = false;
  bool nativeEnabled = true;
  int routeMode = 2;  // 0 = chord, 1 = province approximation, 2 = authored splines
  bool autoTest = true;
  float previewConsumption = 100;
  // Debug-only synthetic freight snapshot; never presented as live data.
  bool testSnapshot = false;
  // Debug-only read-only probe of the engine GUI variable map (D17).
  bool probeVariables = false;
  // Debug-only write experiment (single SSO key/value); never for live data.
  bool probeWrite = false;
  // Publish native-panel text labels into GUI variables for the formal tab.
  // Verified in R18: long keys + values >=16 chars use the heap path and are
  // written from the UI thread via WndProcThunk. Values remain labels/-- until a
  // real (non-synthetic) document exists.
  bool nativeVariables = true;
  // Debug-only: write ironroad_hide_flows once from the UI thread so the native
  // tab's Show/Hide enabled states can be verified without a mouse click.
  bool testHideVariable = false;
  // Debug-only: observe the GetStateProduction script callback (no value use).
  bool probeEconomy = false;
  // Debug-only bounded observations before/after the native partial tick.
  // Does not enable a new economy snapshot access window.
  bool probeAccess = false;
  // Debug-only: force the world-change invalidation once (safe lifecycle drill
  // without driving the pause/load menu).
  bool testWorldChange = false;
  // Debug-only: log the production/consumption container headers of a few known
  // states so a shared market pointer (if any) can be identified.
  bool probeMarket = false;
  // Optional market id override (ini only). 0 = pick the market with the most
  // states (heuristic); set to the player's market id to pin it.
  int marketOverride = 0;
  // Debug-only: log script function registrations whose names mention market.
  bool probeRegister = false;
  // Debug-only: enumerate the script function registry (0x1459FD648) once and
  // log entries whose names mention market.
  bool probeRegistry = false;
  // Debug-only observation of the verified MarketPanel::GetMarket getter.
  // Default off until panel identity/thread/lifetime are verified in game.
  bool probePanelMarket = false;
};

class Overlay {
 public:
  static Overlay& Instance();

  bool Install(std::string* error);          // installs native entry-point hooks
  void RequestRoute(int from, int to);        // small copied request -> worker
  void ClearRoute();
  void SetArrowType(const char* name);
  void PublishLogisticsDocument(std::shared_ptr<const LogisticsDocument> document);
  // Debug/economy probe: feed a real engine-read snapshot into the logistics
  // session (synthetic=false). The formal UI labels it "model allocation".
  void PublishRealEconomy(std::shared_ptr<const EconomySnapshot> snapshot);
  std::optional<LogisticsSelection> TakeLogisticsRequest();

 private:
  Overlay() = default;

  void OnPresent(IDXGISwapChain* swap);
  bool InitImGui(IDXGISwapChain* swap);
  void DrawUi();
  void DrawPreviewControls();
  void DrawDiagnostics();
  void QueueLogisticsSelection(bool active);
  void OnRenderPass(void* passContext);
  void ApplyPendingRoute();
  void ApplyPendingFreight();
  void ProbeVariablesIfEnabled(bool force = false);
  void PublishLogisticsVariables(const std::shared_ptr<const LogisticsDocument>& document);
  void QueueLogisticsVariables(const std::shared_ptr<const LogisticsDocument>& document);
  void OnUiThreadPublish();
  bool UploadRibbon(size_t slot, const RibbonGeometry& geometry);
  void DeactivateRibbons();
  void ReportRibbonBatch(const char* state);
  void LoadSettings();
  void SaveSettings();
  void DrawStatus();
  bool PickState(const char* label, int* value, const char* filterId);
  void EnsureBackbufferRtv(IDXGISwapChain* swap);
  void DebugReadbackOnce();
  bool IsMainSwapchain(IDXGISwapChain* swap);
  void ReinitForSwapchain(IDXGISwapChain* swap);
  void RecordFrameTiming(IDXGISwapChain* swap,UINT flags);

  static HRESULT WINAPI PresentThunk(IDXGISwapChain* self, UINT sync, UINT flags);
  static HRESULT WINAPI Present1Thunk(IDXGISwapChain1* self, UINT sync, UINT flags,
                                      const DXGI_PRESENT_PARAMETERS* params);
  static void __fastcall PassThunk(void* ctx);
  static void __fastcall SetPathThunk(void* arrow, void* path);
  static void __fastcall AppendThunk(void* arrow, void* ctx);
  static void __fastcall AccessScopeEndThunk(void* scope);
  static void __fastcall DestroyNativeWorldThunk(void* world);
  static LRESULT CALLBACK WndProcThunk(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

  // D3D
  ID3D11Device* m_device = nullptr;
  ID3D11DeviceContext* m_context = nullptr;
  IDXGISwapChain* m_swap = nullptr;
  ID3D11RenderTargetView* m_rtv = nullptr;
  ID3D11Texture2D* m_backbuffer = nullptr;
  UINT m_rtvWidth = 0, m_rtvHeight = 0;
  HWND m_hwnd = nullptr;
  WNDPROC m_origWndProc = nullptr;
  bool m_imguiReady = false;
  bool m_visible = false;
  bool m_presentHooked = false;
  bool m_emptyUi = false;
  bool m_noRender = false;
  bool m_frameTiming = false; // diagnostic IRONROAD_FRAME_TIMING=1, default off
  uint64_t m_frameTimingFrequency = 0;
  uint32_t m_debugFrames = 0;
  D3D11_VIEWPORT m_viewport{0.f, 0.f, 0.f, 0.f, 0.f, 1.f};
  bool m_initializing = false;
  bool m_imguiDisabled = false;
  bool m_passWarned = false;
  bool m_captureFrameRequested = false;
  uint64_t m_autoCaptureAt = 0;
  bool m_readbackWanted = false;
  bool m_readbackDone = false;

  // Hooks
  void* m_origPresent = nullptr;
  void* m_origPresent1 = nullptr;
  void* m_origPass = nullptr;
  void* m_origSetPath = nullptr;
  void* m_origAppend = nullptr;
  void* m_origAccessScopeEnd = nullptr;
  void* m_origDestroyNativeWorld = nullptr;
  std::atomic<bool> m_nativeCaptureHooked{false};
  bool m_inAppend = false;
  bool m_piggyback = false;
  uint32_t m_appendLog = 0;
  uint64_t m_piggyFrame = ~0ull;
  bool m_inPass = false;
  bool m_inSetPath = false;
  bool m_submittingOwn = false;

  // Game/native state
  bool m_nativeHooks = false;
  void* m_type = nullptr;
  void* m_arrow = nullptr;
  struct NativeSlot {
    void* arrow = nullptr;
    void* type = nullptr;
    void* vertex = nullptr;
    void* constant = nullptr;
    std::string style;
    ArrowPath path;
    void* pointBuffer = nullptr;
  };
  // Pool slots stay at stable addresses. The scheduler reuses a slot only for
  // its constructor-time style and never calls an unverified destructor.
  std::vector<std::unique_ptr<NativeSlot>> m_slots;
  ID3D11Device* m_nativePoolDevice = nullptr; // identity only; m_device owns its reference
  bool m_nativePoolDeviceChanged = false; // replacement requires verified native buffer rebuilding
  uint64_t m_nativePoolGraphics = 0;
  uint64_t m_nativeRecoveryAttempt = 0;
  std::string m_nativeRecoveryError;
  bool RecoverNativeWorld();
  RibbonCache m_ribbons;
  RibbonCache::Statistics m_ribbonBatchStart;
  bool m_ribbonBatchOpen = false;
  uint64_t m_ribbonBatchKey = 0, m_ribbonBatchWorld = 0, m_ribbonBatchRevision = 0;
  NativeSlot* m_slot = nullptr;
  std::string m_renderStyle;
  uint64_t m_cacheHits = 0;
  bool m_routeActive = false;
  // Identity of the freight frame currently requested into m_ribbons. Zero when
  // the freight view owns no native ribbons.
  uint64_t m_freightKey = 0;
  std::atomic<bool> m_wantsRoute{false};
  uint64_t m_builtGeneration = 0;
  std::atomic<uint64_t> m_generation{0};
  uint64_t m_lastTypeAttempt = 0;
  std::string m_typeFailedFor;
  int m_from = -1, m_to = -1;

  uint64_t m_seenGeneration = 0;
  std::unique_ptr<PreviewWorker> m_preview;

  // Submission bookkeeping
  uint64_t m_lastSubmitFrame = 0;
  ArrowSubmissionGuard m_submitGuard;
  void* m_lastSubmitCtx = nullptr;
  uint64_t m_passCalls = 0;
  uint64_t m_matchingPassCalls = 0;
  uint64_t m_lastPassSummary = 0;
  uint64_t m_submitted = 0;
  uint64_t m_skippedDuplicate = 0;
  uint64_t m_uploadedRoutes = 0;
  double m_lastBuildMs = 0.0;
  // GUI variable probe state (read-only, debug only).
  uint64_t m_lastVarProbe = 0;
  size_t m_varProbeCount = size_t(-1);
  std::set<uint32_t> m_varProbeSeen;
  std::string m_varProbeSummary = "not probed";
  size_t m_globalProbeCount = size_t(-1);
  std::set<uint32_t> m_globalProbeSeen;
  std::string m_globalProbeSummary = "not probed";
  size_t m_varProbeLogged = 0, m_globalProbeLogged = 0;
  bool m_writeTestDone = false;
  bool m_variablesPrimed = false;
  uint64_t m_passStableSince = 0;
  bool m_postLoadWriteDone = false;
  Mutex m_publishMutex;
  std::array<std::string, 9> m_pendingPublish;
  bool m_publishPending = false;
  std::array<std::string, 9> m_lastPublished;
  std::vector<std::pair<std::string,std::string>> m_pendingGoods,m_lastGoods;
  std::shared_ptr<const FreightFrame> m_pendingPublishFrame;
  std::shared_ptr<const LogisticsDocument> m_pendingStateDocument;
  std::string m_pendingStateId,m_lastStateId;
  uint64_t m_pendingPublishApplied=0;
  std::vector<std::string> m_writtenGoodsKeys; // UI thread only; clear removed rows

  // Capture for calibration
  bool m_autoTestDone = false;
  int FindState(const char* key) const;
  uint32_t m_captureRemaining = 40;
  uint32_t m_presentLog = 0;
  double m_lastPresentMs = 0.0;

  Settings m_settings;
  std::shared_ptr<const RouteGeography> m_geography = std::make_shared<RouteGeography>();
  int m_originState = 0;
  int m_destState = 1;
  int m_viewMode = 3;
  LogisticsPanel m_logisticsPanel;
  std::unique_ptr<LogisticsSession> m_logisticsSession;
  Mutex m_logisticsMutex;
  bool m_hideTestWritten = false;
  // True while the native panel's ironroad_hide_flows variable exists; the
  // render thread hides the debug freight ribbons without touching data.
  std::atomic<bool> m_hideFlows{false};
  bool m_economyCollected = false;
  std::atomic<bool> m_goodsMetadataUnavailable{true};
  NativeMarketOwner m_nativeMarketOwner;
  NativeStateOwner m_nativeStateOwner;
  NativeStateRoster m_nativeStateRoster; // numeric snapshot only; UI thread
  std::atomic<bool> m_nativePanelUnavailable{true};
  std::atomic<bool> m_nativeFiltersPending{false};
  std::string m_nativePanelError; // UI thread only; errors logged on change
  std::unordered_set<uint32_t> m_realMarketIds; // UI thread only, latest accepted collection
  std::unordered_map<std::string,uint32_t> m_realGoodsByKey; // copied metadata, UI thread only
  std::vector<std::string> m_hoverVariableKeys; // exact keys from that same current catalog
  std::string m_hoverContext; // last attempted native hover context, UI thread only
  bool m_hoverContextReady=false;
  void PollNativeMarketPanel();
  int m_economyAttempts = 0;
  void* m_economyAnchor = nullptr;
  uint32_t m_economyAnchorId = 0;
  int m_lifecycleTicks = 0;
  // Invalidates the copied snapshot when the map/world is replaced (save reload,
  // new game, main menu). Engine pointers are never dereferenced after this
  // check other than through the registry lookup itself.
  void CheckWorldLifecycle(bool force = false);
  // Shared invalidation used by the real anchor check and the debug drill.
  void InvalidateEconomy(const char* why);
  // Top markets summary (id:stateCount) published to the formal tab.
  std::mutex m_marketSummaryMutex;
  std::string m_marketSummary;
  std::atomic<uint64_t> m_economyWorld{1};
  uint64_t m_uiEconomyWorld=1; // UI thread only, last applied native epoch
  void ApplyEconomyWorld(uint64_t world,const char* why);
  std::atomic<uint64_t> m_economyRevision{0};
  // An old validation cannot clear a newer invalidation: readiness is tagged
  // with the exact world, not a boolean reset by an asynchronous completion.
  std::atomic<uint64_t> m_nativePoolWorld{1};
  bool NativeWorldRetired() const { return m_nativePoolWorld.load()!=m_economyWorld.load(); }
  std::atomic<bool> m_nativeRecoveryStatusDirty{false};
  void PollHideFlows();
  NativeSnapshotMailbox m_snapshotMailbox;
  std::atomic<uint64_t> m_lastNativeSnapshotAt{0},m_lastNativeRequestRevision{0};
  void RequestNativeSnapshot();
  void CopyAtNativeScopeEnd(uint64_t caller);
  // UI drains owned copies only. Engine database copies run in the completed
  // native command scope, independently of optional debug probes.
  void CollectRealEconomy();
  std::shared_ptr<const LogisticsDocument> m_logisticsDocument;
  // Republishing an identical document rewrites the same GUI variables. The
  // hidden-debug path used to publish a null document every frame (~65/s), so a
  // publication only happens when the document or one of the scope-bearing
  // status fields actually changes (see publish_key.h).
  PublishedKey m_publishedKey;
  std::optional<LogisticsSelection> m_logisticsRequest;
  bool m_logisticsWasActive=false;
  char m_filter[64] = {0};
  char m_filterDest[64] = {0};
  bool m_stateDataLoaded = false;
  std::string m_stateError;
  char m_lastResult[160] = "idle";
  std::wstring m_dataDir;
  // Canonical preferences file (runtime data root). Legacy copies under the map
  // data directory are still read for migration.
  std::wstring m_settingsPath;
};

}  // namespace v3













