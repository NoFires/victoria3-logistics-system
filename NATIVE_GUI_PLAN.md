# Native game UI integration — current user requirement

### 2026-10-28 — R134完成：正式颜色／八档线宽图例、双语居中及编辑GUI校验

当前9afd正式市场／地区提供Business、单商品稳定色及Native建筑绿；单商品未选时地图为空且给出原版控件操作提示，统计保持。市场color与地区color键值带当前scope，owner／视图cache包含颜色，统计发布忽略纯地图颜色；hover继承当前模式，重开重置Business。

业务色保持地区输入绿／输出橙，市场热度使用完整市场固定区段商品数量尺度；单商品模式按原生商品ID取既有稳定五色，未选或移出为空地图，原生外观统一已验证建筑绿。颜色只改视图，不改国际原版记录、国内最终模型分配、需求账、完整行／卡片／市场分母、路径或宽度。八档图例显示实际宽度比例2:3:4:6:8:12:16:24及数量阈值10／50／200／500／2000／5000／20000，提示比例示意、非像素／容量／距离工作量；0不画、未知--。

四个必需GUI类型按真实type声明计数，跳过注释／字符串；缺失或重名拒绝整个扩展挂载、记录明确原因并保留玩家修改，提示对照freight/native-gui/.defaults。当前9afd实际坏GUI67464中，注释伪声明不能冒充类型，原版四页签／frame0、修改字节保持，正常退出后严格恢复；后续63460重新加载完整GUI通过。16个按钮文字框改为100%边界／居中／鼠标透传，中英截图及实际点击验证通过。

当前中英21项及居中截图／实际点击、缺类型拒绝与恢复通过；首长文案和无界锚点失败保留。原玩家GUI缺少新必需类型时明确拒绝扩展并提示.defaults，不能静默覆盖编辑或挂载半套类型。完整最终交付矩阵仍待。证据及R133回滚：E:/Code/v3-analysis/checkpoints/2026-10-28-r134-native-legend/{legend-audit.json,live,logs,before,NEXT.md}。

### 2026-10-28 — R133完成：地区商品／方向／运输方式地图筛选

State Local Prices now includes two native control rows for map direction and transport mode; clicking the new Trade cell selects that good, while the original goods icon still opens native details. All goods resets the good selection. Complete per-good state quantities remain visible. Sea includes original port approaches; a mixed route keeps its original segments and only the chosen mode is painted. Tooltip explains green imports/orange exports and the map-only semantics.

Three fixed variable keys carry a world/state/reopen context in their values. Ready identity is published last; native polling reads exact keys in the existing sample, owns copied selections only, and old contexts cannot authorize the new panel. Native cache keys include direction/mode; statistics checks deliberately ignore map-only fields. Closing/switching resets the controls. Current Chinese live19case evidence and full50checks are in E:/Code/v3-analysis/checkpoints/2026-10-28-r133-state-filters/state-filter-audit.json. New controls English/layout polish, formal color modes/width legend and older edited-resource upgrade contract remain incomplete. Older sections below describe historical layers, not the sole current status.

## R93c checked native-pool recovery

The world gate now compares nativePoolWorld with economyWorld. Once a current real frame is ready, the verified arrow pass checks the current asset style/type binding, constructor callback membership, retained arrow/path/point/VB/CB identity, original graphics/device identity and stable copies. It clears old active requests and marks only that exact world ready. There is no reconstruction/destruction or simulation lock. Device/graphics changes also retire cache hits; a swapchain replacement compares devices before releasing the old reference, so a recycled COM address cannot authorize the old pool.

Prototype39c recovered180 arrows after menu reload(world2) and in-game Load(world3), with uploads180 unchanged. Final8115373d (33,718,124 B, installed=dist) also passed menu reload with180 retained arrows, UI status restoration, minimize/restore and actual2560x1440→1920x1080→2560x1440 client resizing. Error.log GUI hits0; production lease decoding validated92 types/180 objects. Full regression and D3D11/default-hidden checks pass. Evidence: STATUS/WORKLOG R93c and private checkpoint2026-10-24-r93-world-recovery.

This establishes same-device/same-asset recovery for these save tests. A24 remains partial: another save, reliable world events under anchor-address reuse, new-device native-buffer rebuilding, asset replacement and active-map compatibility are still pending. Earlier permanent-restart gates below are historical, not the current implementation.


## R92 lifecycle evidence and R90/R91 UI acceptance

Real State.GetMarket identity (305 markets) replaces the invalid state+48 grouping. Native market/state figures, Chinese search/categories/empty results/hover/show-hide, native details/context menu and state switching have live evidence. R91 localized ledger values are fully readable; 20 verified close/reopen cycles reused native geometry (uploads371 unchanged), without sustained private-byte growth. This is partial acceptance, not the complete performance protocol.

R92 read-only snapshots across save/menu/reload are byte-identical for92 ironroad types/371 registered arrows, including path/VB/CB pointers. The type's stride0x48 vector holds geometry-rebuild callbacks, not proven world ownership. Constructor registration and type/arrow destructors were inspected; direct teardown remains disabled. The permanent world-retirement gate still blocks arrows after reload, while current-world numeric data resumes. R92 adds a separate localized unavailable-map message. No recovery implementation or device/asset replacement acceptance is claimed. Exact evidence and hashes: STATUS/WORKLOG R92 and private checkpoint2026-10-24-r92-native-lifetime. Historical statements below are superseded where they conflict.


### 2026-10-24 — R86：地区货运后端首次接入运行时（实机待验）

Runtime now copies the accepted state roster, polls the verified state panel decoder on the UI timer, arbitrates NativeStateOwner, formats state goods work and publishes through ironroad_state_rows_id_v1. The UI writer hides rows before writes and reveals only after owner/world/revision/source/frame/state checks; closed/switched/stale scopes withdraw. Rendering uses numeric session copies only.

State selection uses its snapshot market and therefore does not require the unresolved GUI market ID bridge. Extended session tests, GUI/archive/resources, D3D11 and default-hidden checks pass; deployed core d78dda6e… still requires manual in-game state values/switch/close acceptance. Earlier statements that the state bridge has no runtime caller are superseded. Detailed evidence is in STATUS/WORKLOG R86.

## R81 numeric state owner and publication gate

NativeStateOwner now consumes only copied NativeStateRoster data. It chooses
the displayed state's actual market and owns priority15, preserving other owners
on close. SnapshotIsCurrent checks source-copy identity even when world/revision
are equal. CanPublish also checks owner, document/frame scope and FrameIsCurrent;
stale source/owner, closed state, reset and synthetic data cannot publish. Market
summary publication separately requires a Market document. All14 native suites
and build/embedded checks pass. These candidate interfaces still have no native
state-decoder/row-writer caller. In-game state/layout/lifecycle remain unverified.
The user explicitly reauthorized Computer Use this turn, superseding the earlier
Escape stop; live verification may resume with normal target/ownership checks.


## R80 state Local Prices column candidate

The locally hash-verified states_panel.gui now gains a Freight header/cell in
its original Local Prices table. Native sort/price/balance/navigation/menu and
window bindings remain; only local table column widths are compacted. Other
tabs and the non-local-goods icon section are unchanged. C++ and Python patch
outputs match byte-for-byte; no original state GUI is embedded in the DLL.

State rows use ironroad_state_work_<signed state ID>_<goods key>, revealed only
when ironroad_state_rows_id_v1 equals the current State.GetIDString. Static
registration/getter plus Capstone confirms signed int32 decimal formatting,
including high-bit IDs. FormatStateFreightGoods requires a matching state-view
document and unique state/market identity; synthetic/unknown input is rejected.
No runtime state owner/publisher exists yet: the cell remains --. In-game parser,
layout/widths and positive state context remain NOT_RUN. Candidate is built and
not deployed; installed DLL remains R78. Evidence: round80 logs and private
state-column-r80-static.md / assembly.txt.


## R79 state context candidate (offline only)

The exact EXE constructor0x20A8900 places states_panel at bar slot14;
name-to-kind0x20ABFC0 independently returns14. Constructor0x22539D0 sets
vtable0x45CC848 and ID+0x70. The State typed-ID setter0x22547B0 writes that
field, and method0x2254770 resolves it. Ghidra plus Capstone checked the
critical instructions; installed GUI uses local_goods for Local Prices.
state_panel.h safely copies the candidate context; state_panel_tests is wired
into build_tests.ps1 and all14 suites pass. No runtime caller or native getter
was added. Live state/snapshot ID correspondence, access window and open/close
transitions remain unverified; the Freight column is still pending. DLL remains
R78 unchanged. Private evidence: state-panel-r79-static.md / assembly.txt.


The user explicitly supersedes the starter's ImGui product interface: freight
controls must blend into the original market, state and goods panels. ImGui is
an optional debug panel, closed by default. Delivery stays a pure DLL without a
launcher mod/playset. The native UI work is not yet complete.

## R78 market reference layout (in-game acceptance pending)

The market Logistics fragment now has Freight/Laid track cards, Show/Hide,
market-price/Freight/share headers and per-good work/share cells. It keeps the
original goods row's button, tooltip, right-click and native order. The vanilla
price/graph block is extracted locally from the version-verified installed
market_panel.gui and substituted into a marker in our own fragment before ZIP
creation; the DLL contains no copied price block or base textures. Python and
the actual C++ generator agree byte-for-byte, with absent/duplicate markers and
unknown GUI versions rejected. Base-game crate and railway icons are referenced.

Native variable rows are canonical by goods key, reject synthetic/invalid input,
and keep work/shares fixed during hover. Removed rows clear to --. Publication
checks the owned frame again on the UI thread and gates cells on the current
panel context; a full successful write is required to reveal them. Known zero
work remains 0.00, while a zero-denominator percentage is --. Existing GUI keys
now update the validated value directly instead of allocating another pair on
each update; new-key ownership behavior is unchanged and still needs runtime QA.

R78 is built and deployed, but no native parser/layout screenshot or new row
publication has been observed in-game. Hover timing, same-frame close and UI
cost remain unverified. The state Local Prices column is still pending and
requires the current-state context/data bridge; R77 already supplies its work
aggregation. Computer Use remains stopped following the user's Escape signal.

## R76 candidate and new reference layout

The user explicitly chose to restore the original search/category controls.
Both native and Python patch generators now retain their exact original blocks,
including expand/collapse, while extending the parent visibility to Logistics.
A new control-preservation regression failed before removal of the old hide
patch and passes afterward. This is generated-GUI evidence, not a visual pass.

The Logistics goods row adds hierarchy enter/leave effects to the inherited
container, retaining its original inner button, prices, tooltips, click and
right-click. Installed map_markers.gui uses the same Set/Clear/Concatenate
event pattern; Goods.GetKey's callback/direct getter were rechecked locally.
Each good has its own variable key, carrying a panel/world context token. Native
hover accepts only copied metadata and currently visible IDs; leave restores
the base owner. Close/tab/market/world/metadata changes invalidate hover; a
retained entry cannot reactivate after reopening. Multiple rows/read failure
withdraw hover. Render/publication accepts the formal hover owner with ImGui
hidden. Numeric polling is now a 25ms UI timer; economy collection stays on its
separate 500ms timer. Actual callback timing, same-frame close and overhead in
the new DLL remain NOT_RUN; a timer interval is not a measured latency guarantee.

The user subsequently provided state Local Prices and market Logistics images,
and chose **allocated orders × goods weight × physical distance** for Freight.
Required next: append Freight to the original state table; market cards and rows
must follow the images, with selected-filter work on the card and whole-market
share denominator fixed. Hover affects only the map. These state columns/cards
are not implemented by the R76 event bridge. See MASTER_PLAN §3.2.1; original
reference PNGs are preserved privately under v3-analysis/references.

## Earlier locally verified script extension points

R74 candidate extends the R73 state bridge with native matched-goods IDs.
`MarketPanel.GetFilters` is a CPdxArray of excluded category bytes at+0x110,
not selected bits. SearchBar string is panel+0x88; its original callback writes
the normalized localized query into the market wrapper+0x328. The original
`Market.AccessMarketGoods` builds cache+0x60 after vanilla category/search rules.
The candidate reads that cache without invoking getters or locks, requiring
unique market identity, matching query, clean dirty/writer flags and valid row
ownership/definition/category. It forwards only sorted numeric IDs to the view.
Unavailable cache withdraws the prior formal view; clean empty matches remain
distinct from missing data. The live registry sample contained only dirty,
zero-row entries, so positive row layout/UI correspondence is NOT_RUN.
Fixture tests exercise localized queries, stale/busy/ambiguous inputs, known
empty matches and filter invariance; they do not prove game interaction.

R73 candidate now observes the original panel state through safe numeric copies,
without invoking GUI effects/getters. The exact ownership chain is version/type
checked: interface idler -> gfx -> interface handler -> information-panel bar ->
market panel slot9. Native IsPanelOpen uses nonempty history, !closed and last
history kind9; Logistics additionally requires the copied selected-tab string.
The closed chain was observed in PID52316 and replayed through the actual decoder.
Open/tab/market transitions in the new DLL are NOT_RUN. The candidate activates
only the native Logistics owner, selects the panel's market (explicit override
remains), and withdraws it on close/tab change/unavailable data. Repeated timer
samples do not restart worker work; other owners are preserved. Formal variables
require the current native-owner frame and matching world/revision/market.
Synchronization currently uses the existing 500ms UI timer, so this does not
establish same-frame close handling. Hover/filter/state panels and full lifecycle
acceptance remain incomplete. No candidate deployment occurred while R69 ran.

Installed `game/gui/market_panel.gui` SHA-256:
`1f682222b689666731e755bc52b412b3126f4afff40cde05b13a0757b027ec3e`.

- Market panel already binds `MarketPanel.GetMarket` and its original header,
  flag, back button, pin, close controls, sounds and animations.
- Its `fixed_top` uses native `tab_buttons`. The shared template in
  `gui/shared/tab_bars.gui` has an unused fifth tab with normal/selected/click
  overrides. The prototype adds a Logistics tab using those existing blocks.
- Search uses `MarketPanel.GetSearchBar`; categories use the original
  `goods_filters` and `MarketPanel.GetFilters`. The prototype extends their
  visibility to Logistics, retaining the native controllers.
- `Market.AccessMarketGoods(MarketPanel.GetFilters)` provides native goods rows.
  Reusing `goods_entry_button` retains game icons, prices/deviation graphs,
  tooltips, right-click menu and `InformationPanelBar.OpenGoodsPanel`.
- The existing goods panel already supplies price maps, buy/sell order totals,
  building-type breakdowns, taxes and tariff controls. Reuse it instead of
  recreating its functionality in a floating ImGui page.

`tools/native_gui.py` derives a patch from the locally installed GUI, rejects
unknown hashes and writes previews only outside the game installation. Own
widgets live in `native/assets/ironroad_logistics.gui`. English and Chinese
localisations and the tab fragment are embedded as resources 301–304. Runtime
`native_gui.cpp` derives the base override locally, generates a deterministic
stored ZIP and caches it by SHA-256. No copied base-game GUI is embedded in the
DLL, and installed GUI files are not changed.

Deployed DLL SHA-256 (2026-10-23):
`a99f8d9ecb58c76245d19c8b72bffe5bb333a7bff79dd390459fed21ec3f9c74`.
PID 50272 and subsequent PID 42316 both logged GUI mount during MountData,
followed by `native GUI enumeration; prepared=1 mounted=1`. No Ironroad-specific
GUI parser errors were present in the inspected startup logs. PID 50272 later
exited without a recorded exception; this is not a visual acceptance result.
The user subsequently confirmed the native Logistics tab, goods list and original
goods details all work. This establishes effective GUI override precedence and
the market entry/list/detail path; it does not validate freight data/actions,
hover, state panels or lifecycle acceptance.

Tests validate unique anchors, balanced quoted/commented blocks, preservation
of original four tabs/search/filter bindings and unknown-version rejection.
`tools/test_native_gui_runtime.ps1` compiles the actual C++ generator and checks
SHA-256 vectors, unknown-version rejection, missing resources, duplicate/unsafe
ZIP paths and deterministic cache reuse. An independent Python ZIP reader checks
CRCs and byte-for-byte agreement with the Python patch. Default-hidden test PID
49180 produced zero ImGui vertices. These do not prove visible UI interactions.

## DLL / VFS integration path

The existing early MountData hook already mounts the DLL-owned archive before
the VFS freezes and before model enumeration. Static instructions at
`victoria3.exe+0x3B2F5FE..0x3B2F605` prepend a new mount to the context's linked
source list (`new+0x30 = oldHead; head = new`). Both own archives are now mounted
in this window, with native GUI enumeration observed afterward. Effective base
override precedence is now supported by the user's visible-tab confirmation.
Do not late-mount after `_FrozenConfig`.

Keep the DLL free of a copied base-game GUI: derive the changed market file from
the user's own version-checked original into the automatic cache, combine it with
our original GUI/localisation fragments and mount that managed archive. Do not
edit installed GUI files or a launcher playset.

## Required remaining integration

1. Retain regression coverage for the user-confirmed market Logistics tab,
   goods list and original goods details; expand acceptance to native search,
   categories, back/close and other panels as data/actions are connected.
2. Add a verified native data/action bridge for model freight, shares, unique
   built track, current market/state/good selection and Show/Hide/hover commands.
   Use owned numeric copies and stable IDs; no economy scans in rendering.
3. Keep native market filters and goods-page selection as authoritative UI
   state. Close/back/world changes cancel pending work and remove owned arrows.
4. Bind model values to native summary cards and goods columns, and retain
   existing native prices/buy/sell/building details. Unknown values stay unknown.
5. Accept the original video's layout and interactions in game, including
   category/search, per-good selection, width/colour and return/close behaviour.

## Additional static bridge evidence (not active native hooks)

R68 rechecked `MarketPanel.GetMarket` against the installed EXE hash with
Ghidra and independent Capstone decoding. The getter at RVA `0x225AC90` takes
`(panel, callerOut)`, passes the ID at `panel+0x70` to `0x10571F0`, stores the
returned GUI wrapper in callerOut, increments wrapper+8 and returns callerOut.
Constructor `0x1CD6DC0` sets wrapper vtable `0x45850B0` and copies the ID to
wrapper+0x10. Callback `0x225AED0` releases the reference. The resolver takes
locks and can allocate a wrapper; it is not a harmless polling API.

An observation-only hook now exists behind default-off `probe_panel_market=0`.
It requires the supported executable hash and getter prefix, preserves the
original call/output/return, copies at most 16 numeric samples with bounded
ReadProcessMemory calls, checks wrapper vtable and ID equality, and retains no
pointer. It neither selects a market nor scans economy data. Runtime panel
switching, thread/lifetime and ID correspondence remain NOT_RUN; formal market
selection continues to use the documented heuristic/override until that evidence
exists. Private evidence: `E:\Code\v3-analysis\panel-market-r68-static.md`.

The notes below are historical static findings; R68 adds the disabled getter
probe described above, not a general-purpose script callback interface.

For the exact executable profile recorded in PROFILE.md, native GUI registration
wrappers resolve `InformationPanel.IsTabSelected` via RVA 0x316AF0 and SelectTab
via 0x316C40. Their callbacks are 0x214CD10 / 0x214CD30. Native SelectTab reaches
0x214C770, which copies the arbitrary string into panel+0x30 and invalidates the
panel; there is no tab enum restriction in that setter. Visual acceptance is
still necessary. MarketPanel.GetMarket registration 0x354250 uses callback
0x225AED0 and getter 0x225AC90 (panel+0x70). GetFilters registration 0x3547B0 uses
0x225B150 (filter storage panel+0x110). GetVariableSystem registration 0x5AE6C0
uses 0x3A98400 and singleton getter 0x3A92920. None of these addresses has been
added as a new runtime call or hook. Private decompilation is in
`E:\Code\v3-analysis\native-gui-bindings.txt`; assembly was independently checked.
In particular, helper 0x73A1D0 constructs an argument-value vector, **not** a safe
economy access guard. It must not be used as evidence for the snapshot window.

The earlier ImGui logistics component and its tests remain an offline presentation
prototype/debug surface. It is not the player-facing implementation or evidence
that native UI integration is finished.
