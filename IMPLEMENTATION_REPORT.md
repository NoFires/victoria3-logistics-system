# Ironroad implementation report

## Current status (R61, 2026-10-24)

Current candidate/deployed DLL:
`0cb83a456e3b6237db2b36574848ac0e6f87ef3c96890d7edc5ff11d51672d56`
(29,752,211 B).

The formal market panel's fifth tab now shows **live, market-scoped model
numbers** from the running game: `model allocation (market N[, auto])`,
`Freight (order-cost)`, `Target / allocated / unmet`, Show/Hide, and the vanilla
goods list. Live snapshot (R44/R60): 1117 states, 28,554 orders, 49 goods,
59 markets; default market 118 (77 states), UK market 79; `market_override=<id>`
pins a market. Flow ribbons come from the model allocation only; line width uses
allocated quantity; unmet demand is never drawn as shipped.

Known gaps: player-market auto-detection (default is the largest market),
real built-rail extraction (`Built rail: --`), formal hover/filter integration,
save/menu round-trip acceptance, and full eight-width visual acceptance in the
formal UI. The sections below are historical records of earlier builds.

## Native GUI packaged and mounted — 2026-10-23

Deployed DLL SHA-256:
`a99f8d9ecb58c76245d19c8b72bffe5bb333a7bff79dd390459fed21ec3f9c74`.
The DLL embeds own GUI/localisation/tab fragments, derives market_panel.gui from
the user's exact verified installed copy and mounts a SHA-addressed cache ZIP
during the existing early VFS hook. Distribution and installed hashes match.
PID 50272 and 42316 show native GUI mount before GUI enumeration; inspected
startup logs contain no Ironroad-specific parser errors. Visual tab/list/detail
interaction, effective base-file override priority, native freight data/actions
and state integration remain unverified/incomplete. Summary fields still show
unknown values; Show/Hide only toggles its GUI variable until the bridge exists.

Tests: real C++ patch/archive generation, independent ZIP CRC and exact Python
agreement, bad version/path/duplicate/resource rejection, deterministic reuse,
10 actual PE resources verified, default debug-hidden harness PID 49180 zero
vertices. No standalone log-writing tests were run while the game was live.

Earlier build identities below describe historical evidence only.

## Native UI steering and debug-only window — 2026-10-22

The user explicitly requires the mod to blend into the original game interface.
This overrides the starter's floating ImGui product panel. ImGui is now optional
debug tooling, closed by default, initially on Diagnostics. Insert opens it;
`IRONROAD_DEBUG_UI`/explicit standalone readback can enable it for development.
Existing ImGui logistics widgets are labelled prototypes and are not the main UI.

Deployed debug-role DLL SHA-256:
`2ffa5880c1b35399757a1ba70664b3965cd348250ef39767a8d8ec246ac18d99`.
Distribution/installed hashes match. The standalone explicit-readback test passes
(541934 bright pixels). A separate launch with both debug/readback environment
flags absent initializes the loader/backend and presents with zero UI vertices
(PID 9772), verifying the debug window is closed by default. This is offline
loader/visibility evidence, not an in-game native Logistics acceptance run.

Native market GUI inspection found its unused fifth-tab overrides, existing
search/category controllers, goods datamodel, and original goods-button action
that opens the complete native goods page. An original Logistics-content fragment
and exact-version-gated market GUI patch prototype are implemented. Tests retain
the existing four tabs and native search/filter bindings and reject unknown base
GUI hashes. The generated preview is outside the game installation and is not
yet packaged into the DLL. NATIVE_GUI_PLAN.md tracks VFS/parse verification,
native data/actions, state/goods integration and runtime acceptance still needed.

The copied presentation/selection components also have tests for category
combinations, search by display name, fixed share denominators, market-specific
prices, unknown-value handling, stable goods identity, list/detail navigation,
hover restoration and world replacement. Actual ImGui frames are exercised;
these are development tests rather than proof of native engine UI parsing.

## Background routing, shared native cache and video presentation — 2026-10-22

New deployed DLL SHA-256:
`954e7fd99a9b0b36265cb0d3d74719120fbb69185860b4c6802fc1d09d49cacf`.
Distribution/installed hashes match. This build has **offline** verification;
the earlier user-confirmed in-game arrow-direction check belongs to the previous
DLL recorded below, not this new deployment.

Preview routing now runs on an owned Win32 worker with immutable derived map
data and stable state IDs. The render callback polls completed native controls;
it no longer runs a spline/province search or coordinate conversion. The mailbox
implements 100ms quiet / 250ms maximum coalescing, cancellation during searches,
latest-generation publication and an explicit error result after exceptions.
A separate LRU holds up to 128 preview routes, independent of colour/calibration.

One exact-points/constructor-style RibbonCache controls stable native backend
slots, with one global hard cap of 8192. It pins all current cache hits before
eviction, reuses only compatible constructor types and retains failed geometry
without per-frame retry. Cold work is limited to sixteen native uploads and a
soft 2ms budget per frame, shared across wrapped passes and superseding batches.
Registered path contexts and fixed game-allocated buffers remain stable. No new
native destructor or guessed release operation was added. Hooks/worker/DLL are
explicitly process-lifetime; hot unloading is unsupported. World/device invalidation
and memory plateau still require the original acceptance work.

The user's MP4 was decoded and inspected locally. VIDEO_REFERENCE.md records
observed category switching, Show/Hide controls, freight/laid-track statistics,
goods prices/shares and single-good views. Five traffic palettes now cover
green/lime/yellow/orange/red. The video has no legend; relative-throughput meaning
follows the original specification and is not called capacity utilisation.

Allocated segment quantity determines one of eight native width variants.
Widths use fixed absolute thresholds, independently of goods weights, target
imports, route length or current filter maximum. Quantity changes inside a band
reuse geometry. Preview consumption is a clearly labelled manual test input.
The freight model/view → ribbon conversion is implemented, including colour,
width, authored point order and net movement direction, but remains disconnected
from live economy data and the disabled freight ImGui modes.

Validation: all native geometry/spline/scope/freight/cache/worker tests pass;
actual-map background routing and blocked-computation cancellation are exercised.
The cache tests cover global 8192 cap, pinning/LRU, hash collisions, failed
suppression, reorder/reopen reuse and both frame budgets. All nine shader
palettes compile; WARP GPU readback verifies their actual RGB output and checks
centred forward/reverse arrowheads at all eight widths. The clipped-half-width
negative control still fails as expected. DLL resources match all five maps and
92 native style definitions, with no game textures or playset metadata embedded.
The standalone loader/ImGui readback passes (326767 bright pixels).

No fresh game-visible width/traffic-palette test or real cold-market performance
measurement is claimed. Live snapshot access/identities/market/rail extraction,
prices and track-unit conversion, full freight UI/view cache, competing-source
ownership and lifecycle/performance acceptance remain incomplete.

## Full-width arrow UV correction — 2026-10-22

Runtime PID 20636 proved the embedded archive mounted during native MountData
before `.asset` enumeration. Green, orange, yellow and blue types resolved,
constructed native buffers and submitted continuously with an empty playset
enabled-mod list. The user's screenshot confirmed visible coloured drawing.

The screenshot also exposed clipped triangle heads. The native vertex builder
at `0x37FE4A7` reads float **1.0** from RVA `0x486AD68` for one ribbon side and
writes zero on the other; transverse UV is **0..1**, not 0..2. The shader's old
multiply-by-0.5 sampled half of the building texture, placing the visible triangle
tip at a ribbon edge. Removing that factor restores the complete triangle.

An offscreen WARP D3D11 regression executes the actual pixel-shader Code block
against the installed building arrow texture. With native 0..1 transverse inputs,
forward and reverse arrow tips are centred at y=63.50 of 128 pixels; tip x=123
versus x=76 demonstrates the opposite along-route directions. A negative control
reinstating the old half-width mapping fails the tip-centre check. This is an
actual shader rendering test; engine camera/terrain binding still needs the
fresh game run. The outer shader DSL comment was also corrected from // to #.

Direction-fix DLL SHA-256: `73bd9e47a35adcb0f51d9322a962e17a5e35133acd57c34557350e567c929e4e`.
Fresh game PID 37480 mounted the embedded archive before asset enumeration,
resolved building-flow output, built the 17-point/19-knot UK route and submitted
it continuously. The game's current debug/error logs contain no Ironroad shader
errors or VFS freeze assertion; content_load.json still has an empty enabledMods
array. The user explicitly confirmed: the arrowheads are centred and direction
is correct. This closes the DLL-only style/direction verification. It does not
complete the missing live freight integration or broader lifecycle acceptance.

## Standalone DLL conversion — 2026-10-22

The 22:04 game run still could not resolve the custom types. Its debug log mounted
only base/DLC directories, and the launcher had regenerated content_load.json with
an empty enabledMods array. Thus the earlier file edit did not establish durable
loading. The user explicitly requested a DLL-only implementation instead of a
playset solution. No launcher database edits were made.

The DLL now embeds its original shader/type adapters in a ZIP Windows resource,
plus all five derived map-data resources. A content-addressed ZIP and managed map
cache are prepared under LocalAppData automatically. During the game's native
MountData callback, the DLL mounts this ZIP before VFS configuration freezes.
Definitions sit under gfx/models/military, where the original .asset loader
consumes them. A pre-enumeration hook verifies this timing. Definitions reference the game's own building-flow DDS
textures. No .mod file, metadata, playset entry or manually installed data file is
needed. Deployment copies only version.dll and retires exact legacy Ironroad mod
files. The launcher database is not touched.

Built/deployed SHA-256: `65c84460c5613c702f7ee506a565b96b870c0b64e569f2b87e11b77dcd7c98c9`.
The DLL-resource verifier confirms exact ZIP/data contents and no packaged game
textures/playset metadata. The standalone loader/panel test confirms resources
extract and load (781 states, 4606 spline edges); GPU readback passed with 327971
bright pixels. The first standalone runtime attempt (PID 28760) mounted its ZIP,
but scanning at ThreadedInitTextureLookup proved too late and did not parse
arrow definitions. The VFS freeze assertion also ruled out late mounting. The
revised MountData/pre-enumeration implementation is awaiting the fresh-process
check; mounted-file evidence alone is not registration proof. Historical sections
below describe superseded packaging.

## Building-flow style and direction correction — 2026-10-22

The user confirmed that vanilla tan draws after the pass-selection fix. The next
live log repeatedly reports `ironroadGreen unresolved`: custom assets were not
mounted. This game's content reader (RVA 0x389FF10 -> 0x38A0170 -> 0x38A0340) reads
`enabledMods` entries as objects with a string `path` field. The user's file had
an empty enabledMods array. Packaging now supplies modern `.metadata/metadata.json`;
deployment backs up and updates this configuration without removing other mods.

The new CArrow shader references the game's own `gfx/lines/arrow_diffuse.dds` and
`line_mask.dds`, with the selected-building output green and input orange tints.
It explicitly maps CArrow's distance-in-V to the texture's forward U axis, uses
the thinner building-flow UV scale/animation and samples terrain height. It does
not use the incompatible Jomini line vertex ABI. The wide variant is removed
from the menu and old settings migrate to building-flow green.

The native cubic builder renders knots 1 through count-2. Upload now duplicates
endpoint controls so even a two-point route draws from the requested source to
the destination. Geometry caching includes these controls. Regression tests
check actual cubic endpoints and full reversal, coordinate conversion and bounds.

Offline validation: native build, geometry/spline/pass/freight regressions,
resource/effect/metadata validation, HLSL vertex/pixel compilation for all four
palettes (engine bindings stubbed), and configuration enable/idempotence/remove
tests pass. A fresh game run must still confirm engine resource binding, visual
appearance and animation direction. Freight snapshot integration remains pending.

Deployed DLL SHA-256: `7c75a2eb4fb007e5d4b8bf24b041a5cccc713971a0efdd0f587204eab18ee792`.
Installed and distribution hashes match; modern metadata, effect and seven type
definitions are installed and this folder is enabled in content_load.json.
The standalone D3D11 readback check passed (327,971 bright pixels); it validates
the loader/panel only. Pre-change DLL, live log and loader configuration are
preserved in `E:\Code\v3-analysis\checkpoints\20261022-before-building-flow`.

## Current correction — 2026-10-22, arrow pass selection

The 21:04 live run (PID 44352) resolved both vanilla arrow types and produced
vertex/constant buffers, but its captured map showed no route. The previous
"submitted" counter therefore did **not** prove visible drawing.

Static review of both native arrow submission routines established that their
context contains three index handles at offsets **0, 8, 16**. The old diagnostics
incorrectly printed consecutive 32-bit words at 0, 4, 8. The first render callback
in that run belonged to stage 3, while read-only process inspection of the
handles loaded by native submission established domain 0 / stage 1. The old
frame guard consumed the only submission at the first callback and skipped the
native arrow stage.

The corrected bridge selects the domain/stage using those **live native handles**
(not a fixed pass number), and deduplicates once per frame and logical view.
Wrapped-map views remain eligible. `SUBMIT scope=...` summaries expose the actual
selected scope and counts. A native regression test reproduces the callback
order and checks stage selection, handle layout, duplicate callbacks, changing
live indices, uninitialized handles and separate wrapped views. The missing-pass
message now correctly identifies main-menu/loading as a normal possibility.

The build also decodes the installed version-4 `spline_network.splnet`: 33,780
point records, 4,273 strips, 3,359 endpoint junctions. Authored routing preserves
the strip point order, returns ordered edge IDs and distance, and rejects
disconnected routes instead of silently drawing a chord. `splines.bin` stores
image-space rows; upload flips Z to bottom-origin world space (8192 x 3616).
Native city anchor 100 and anchor 1500 supply independent coordinate checks.

**Current verification boundary:** decoder tests, native spline/pass tests and
standalone rendering checks are available; the user subsequently confirmed vanilla
tan draws correctly. The historical sections below describe the
earlier implementation and must not be read as proof that this correction is
visible in-game. The freight model and views remain work in progress.

Build target: Victoria 3 **1.13.11** (`caligula_rev.txt`), executable SHA-256
`7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f`, 96,872,568 bytes.

This report separates **verified offline**, **verified in a standalone D3D11 process**, and
**not yet verified in a live game**. No claim is made beyond the evidence below.

## 1. Architecture

```
version.dll (proxy, loaded by victoria3.exe at startup)
 鈹溾攢 forwards all 17 version.dll exports to System32\version.dll
 鈹溾攢 bootstrap thread (never under the loader lock)
 鈹?  鈹溾攢 verifies executable SHA-256 + 7 entry-point byte signatures
 鈹?  鈹溾攢 MinHook: CArrowObject::SetPath, arrow render pass
 鈹?  鈹溾攢 probe D3D11 swap chain 鈫?hooks IDXGISwapChain::Present / Present1
 鈹?  鈹斺攢 loads state positions + height field from binaries\ironroad
 鈹斺攢 Present hook
     鈹溾攢 creates ImGui context on the game's device/context
     鈹溾攢 draws the control panel (copies a small request, owns no engine objects)
     鈹斺攢 increments the frame counter

arrow render pass hook
 鈹溾攢 consumes the latest settled request (queued by the panel)
 鈹溾攢 type lookup (by asset name) 鈫?construct supply arrow 鈫?SetPath 鈫?constant buffer
 鈹斺攢 appends ready arrows to the pass bucket using this pass's own context
```

The panel never calls engine rendering functions. The render pass is the only place that
constructs geometry or submits arrows, and it uses the pass context it was called with.

## 2. Native profile (summary)

Full evidence is in `native/PROFILE.md`.

| Capability | RVA | Status |
|---|---|---|
| `CAssetFactory::GetArrowType` | `0x37E7D60` | code path verified: reads only `rdx` as an MSVC `std::string` |
| supply arrow factory (0xC0 object) | `0x26D0A20` | verified: `operator new(0xC0)`, base ctor, supply vtable, `+0xB0 = 0` |
| `CArrowObject::SetPath` | `0x37FD5B0` | verified against both game callers; asserts path layout |
| arrow render pass | `0x37FD860` | verified triple indexing; hooked |
| bucket append | `0x37FE620` | verified: reads `this+0x50` type 鈫?`type+0x30` bucket id |
| constant-buffer creation | `0x26CEF30` | verified from both updaters |
| `CPdxNewDeleteAllocator` | `0x512CDC0` | RTTI singleton, vtable slot 1 = allocate |
| arrow path interface | `0x2674100` / `0x2674120` | installed by the arrow subsystem itself |

Object layout (from the constructor and the supply factory, size `0xC0`):

| Offset | Meaning |
|---|---|
| `+0x00` | vtable (supply vtable `0x46588E8` installed by the factory) |
| `+0x10..0x4F` | 4×4 transform, identity + translation |
| `+0x50` | `CPdxArrowType*` |
| `+0x58` | `-1` (initial value from the constructor) |
| `+0x5C` | flags; bit 0 = visible, set to 7 like the game's updaters |
| `+0x60` | per-object constant buffer (created by `0x26CEF30`) |
| `+0x68` | vertex buffer (created by `SetPath`) |
| `+0x70` | retained path context (read every frame by the render pass) |
| `+0x78 / +0x90` | centerline / ribbon-edge `CPdxVector<SVector3>` |
| `+0xA8` | total spline length (written by the vertex builder) |
| `+0xB0` | supply subclass constants uploaded as shader CB1 (8 bytes) |

Path context retained at `+0x70` (read by the render pass through `0x2674120`):

```c
struct ArrowPath {          // +0x00
  float moveLength;         // +0x00  -> shader MoveLength
  float animationLength;    // +0x04  -> shader AnimationLength
  float clipLength;         // +0x08  -> shader ClipLength
  float reserved;           // +0x0C
  CPdxVector<Vec3> points;  // +0x10  (data, capacity, count, allocator)
};                          // size 0x28
```

## 3. Coordinate model

The arrow vertex shader transforms `vPosition` directly with the game's
`ViewProjectionMatrix`; positions are world space. The engine's map extent is the original
heightmap, 16384 × 7232, and the province texture is 8192 × 3616 (×2). The generator emits
state positions as the `city` province centroid ×2, with `y` sampled from the height map.
The `height_scale`/`coord_scale` values are map/build specific and are adjustable in the
panel; `0.01` is the provisional height scale.

Routing is **stage 2**: a land-following path over the local province adjacency graph
(`data\provinces.bin`, built from `provinces.png` borders and `province_terrains.txt`
movement factors), smoothed with two Chaikin passes, with a straight-chord fallback for
islands or disconnected states. This is an approximation of the authored spline network,
not a replacement: the game's `nodes.dat` spline graph and its coastal/river corridors are
not decoded yet, so a path can differ from the route a unit would actually take. The panel
exposes both modes and the report's acceptance matrix keeps "coastal river" unimplemented.

## 4. Verified evidence

### 4.1 Offline discovery (static)

`tools/discover_local.py` was run against the installed executable; all twelve candidate
groups were found uniquely and both constructor derivations agreed:

```
allocator: CANDIDATE_FOUND
bucket_append: CANDIDATE_FOUND
bucket_list: CANDIDATE_FOUND
constant_buffer_slot: CANDIDATE_FOUND
constructor: CANDIDATE_FOUND
constructor_cross_check: CANDIDATE_FOUND
constructor_fields: CANDIDATE_FOUND
ensure_constant_buffer: CANDIDATE_FOUND
render_device: CANDIDATE_FOUND
render_pass: CANDIDATE_FOUND
set_path: CANDIDATE_FOUND
type_resolver: CANDIDATE_FOUND
```

The private report (with addresses) is kept outside this repository at
`E:\Code\v3-analysis\arrow-candidates.json` for the exact executable hash above.

### 4.2 Loader (standalone D3D11 harness)

`native\build_tests.ps1` + `dist\harness.exe` load `dist\version.dll` into a real D3D11
process:

- proxy export parity: `GetFileVersionInfoSizeA` 鈫?proxy `1764`, system `1764`;
  version parsed as `10.0.22621.5415`;
- 260 frames presented, `Present` returned `S_OK` through the trampoline;
- the native profile is correctly disabled for the harness (hash mismatch), exercising the
  fallback path.

### 4.3 ImGui overlay (GPU readback)

`tools\selftest.ps1` runs the harness with `IRONROAD_READBACK=1`, then reads the backbuffer
back through a staging texture:

```
PASS: loader + ImGui overlay verified (>340,000 bright pixels)
```

The readback image (`%LOCALAPPDATA%\IronroadV3\readback.ppm`) shows the full panel:
status, origin/destination pickers, Draw/Clear, view selector, calibration sliders and
diagnostics. This proves the Present hook, ImGui Win32 input setup, ImGui D3D11 backend,
font atlas and our render-target handling all work on this machine.

Two backend defects were found and fixed while bringing this up:

- `imgui_impl_dx11.cpp` created the vertex constant buffer with an uninitialized
  `D3D11_BUFFER_DESC` (`StructureByteStride` garbage under GCC) and ignored the HRESULT;
  it now zero-initializes the descriptor and checks the result.
- the probe/readback confirmed the panel is rendered correctly, while `CopyFromScreen`
  does not capture this DXGI surface in the agent session (a capture limitation, not a
  rendering failure).

### 4.4 Map data

`tools\verify_data.py`:

```
OK: 781 states, 781 unique keys, 675 with a city colour
OK: x[557..16278] z[92..6900]
OK: height field 1024x452, 925736 bytes
OK: province graph 40875 nodes, 243298 edges, 2281208 bytes
```

The province router was also exercised offline by `dist\route_probe.exe`:

```
STATE_SVEALAND -> STATE_ILE_DE_FRANCE: 87 nodes, 348 points
STATE_BRANDENBURG -> STATE_LOMBARDY: 14 nodes, 56 points
STATE_GOTLAND -> STATE_SCANIA: no province path (straight fallback needed)
```

### 4.5 Live game evidence (Victoria 3 1.13.11, 2560x1440)

The loader was deployed and the game was started by the operator. The log records a
complete, crash-free native pipeline:

```
native profile ready: Victoria 3 1.13.11 sha256=7f53e3...
native hooks installed (render pass 00007ff74076d860, set path 00007ff74076d5b0)
ImGui ready (hwnd ..., 2560x1440)
WARN arrow render pass hook has not fired after 301 frames   <- main menu, before a map
arrow pass #1 entered ctx=000000a9bb91f370 (frame 2160)     <- map loaded
resolved arrow type 'supplyRouteArrow' (width=1.000 subdivisions=5 smoothing=1 bucket=2)
calling native supply-arrow factory (type ..., size 0xC0)...
arrow object created: 00000218f20800c0 type=000002190deea080
calling SetPath (32 points, mode=province) + EnsureConstantBuffer...
route Svealand -> Gotaland built: 32 points (province, 8 nodes),
  vb=00000218f2050f40 cb=00000218f21dd230 len=227.3, 0 ms
```

The panel reported `Arrow pass 41,434 calls / 27,295 submitted`, `Auto-test done`,
`Route active`, and a second user-triggered build (generation 2). No exception was
logged. This verifies the type lookup, per-object construction, `SetPath`, vertex and
constant buffer creation, the pass hook and the bucket append against the running game.
What is not yet confirmed is the on-screen position/colour of the ribbon: the auto-test
route is in Sweden while the observed camera was over the British Isles, so the submit
counts are the evidence that submission happened, not that the ribbon was visible.

## 5. Not verified / disabled

| Item | State | Missing evidence |
|---|---|---|
| Arrow type lookup against the running game | **verified live** | type resolved (width/subdivisions/bucket sane) |
| Object construction / SetPath / constants | **verified live** | object created, `vb` and `cb` non-null |
| Bucket append and render pass submission | **verified live** (no crash, counts increase) | per-frame duplicate behaviour and visual result |
| In-game ribbon alignment and height scale | provisional | one in-game calibration run / visual check |
| Authored spline routes (coastal/river) | **not implemented** | `nodes.dat` graph decoding |
| State imports/exports and market freight views | **disabled** with a reason | economy snapshot adapter |
| Colour bands (green/red/yellow/blue) | arrow types + 4x4 solid-colour DDS supplied in the optional mod | mod not enabled / not observed in-game |

The disabled views are visible but greyed out in the panel and cannot be selected. There is
no silent fallback that draws something that looks like data.

## 6. Acceptance matrix (from `03_TESTING_AND_TROUBLESHOOTING.md`)

| Test | Status |
|---|---|
| Discovery | **pass** (all 12 groups, both ctor derivations agree) |
| Loader | **pass** (export parity offline; hash/profile + hooks live in game) |
| ImGui | **pass** (panel rendered offline by readback; visible in-game at 2560x1440) |
| Single ribbon | **native path pass live**; on-screen placement pending a camera/route in view |
| Native pass | **pass live** (41,434 pass calls / 27,295 submissions, no exception) |
| Clear | **pending** (visibility bit cleared immediately) |
| Coastal river | **not implemented** (province path approximation; authored splines not decoded) |
| Reopen | **pending** (SetPath skips identical point sets; objects are retained) |
| Filter/hover | **not applicable yet** (views disabled) |
| Unchanged view | **pending live run** (same point set is not re-uploaded) |
| Cold market view | **not applicable yet** (bounded 1 route) |
| Memory | **pending live run** (one object per session; no per-frame construction) |
| Lifecycle | **pending live run** (no engine pointers retained across frames) |

## 7. Performance design

- The UI only copies a request (two state indices + a generation number).
- Requests are applied on the render thread in the arrow pass, never in ImGui callbacks.
- A settle gate commits 100 ms after the last selection/slider change (and never waits more
  than 250 ms from the first change), so dragging a slider does not rebuild every frame.
- An unchanged route (same generation and point set) performs no geometry upload;
  `SetPath` itself also compares the new points with the object's stored centerline.
- One long-lived arrow object is reused for route changes; there is no per-frame
  construction and no unbounded pool growth for the preview.
- The province A* runs once per committed request (40,875 nodes / 243,298 edges on this
  map); the resulting polyline is capped at 512 points before the native setter.
- Logging is buffered and flushed at most twice a second (errors flush immediately); no
  per-frame disk flush.
- `auto_test=1` builds one short route after the renderer settles, so the native pipeline is
  exercised on the first launch without operator input. Disable it in the panel or ini.

## 8. Deployment and rollback
`TEST_PROCEDURE.md` is the operator checklist. `tools\analyze_log.py` summarises a game
run (arrow pass entries, type resolution, route builds, captures, errors) and
`tools\calibrate.py` derives `height_scale` from real captured arrow paths and rewrites the
ini.

`deploy.ps1` checks that the game is closed, backs up a foreign `binaries\version.dll`
once, replaces older Ironroad builds without a backup, copies the loader and `ironroad`
data, and prints the log path. `-Uninstall` removes the loader only if it is an Ironroad
build (content marker or hash match) and restores the backup. Saves, the executable and
installed mods are never modified.

## 9. Remaining work

1. Launch the game with the overlay and confirm the panel appears (the offline equivalent
   is already proven).
2. Draw a route and inspect the `CAPTURE setpath` lines: they log real arrow coordinates
   and confirm the path ABI. Compare them with `data\positions.txt` to fix
   `coord_scale` / `height_scale` once.
3. If the ribbon is not visible, check the pass hook statistics (passed/failed) before
   changing addresses; every call is hash-gated and signature-checked.
4. Implement the spline graph and the economy snapshot adapter for the remaining views.


