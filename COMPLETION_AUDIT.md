# Completion audit — R61 (2026-10-24)

The complete goal is **not** achieved. This checklist preserves the supplied
prompt's scope; it does not redefine completion as a successful route-preview
DLL.

**Latest UI override:** the user requires integration into the original game UI.
The standalone ImGui window is debug-only and closed by default. The starter's
ImGui-as-main-interface requirement is superseded.

Latest user requirements override the initial palette/packaging plan: selected
building-flow artwork, correct arrows along source→destination, and DLL-only
deployment without launcher mods/playsets.

| Requirement | Current evidence (R61) | Result / next proof needed |
|---|---|---|
| Exact local build and safe native ABI gates | `game.cpp`/`offsets.h` hash + entry signatures; `PROFILE.md`; startup log | Implemented for installed 1.13.11; lifecycle acceptance partial |
| One-DLL delivery and rollback | `verify_embedded.py` PASS; `deploy.ps1` install/uninstall; R47 rollback drill (game runs without DLL) | Implemented and demonstrated |
| Original building-flow artwork | Embedded shader references installed DDS; GPU palette tests PASS | Implemented and observed in game |
| Correct complete route and endpoint direction | Spline/geometry tests; GPU arrowhead forward/reverse + negative control | Short routes verified; broad camera/terrain acceptance partial |
| Actual authored coastal/river routing | Spline decoder + native graph tests; UK→Sweden coastal path | Implemented; visible coastal acceptance partial |
| Route/clear controls, stable selectors and queued commands | `PreviewWorker` stable IDs, immutable geography; actual-map worker tests | Implemented; fresh-game acceptance partial |
| Native state/market/goods UI with search/category/selection | DLL generates version-gated market patch; C++/Python ZIP/CRC tests PASS; user confirmed the Logistics tab, goods list and original goods details | Confirmed. Hover/filter integration and state-panel integration not done |
| Real model-allocated shipments, target/allocated separation | `freight.cpp` min-cost-flow/conservation/unmet tests; live snapshot (R44): 1117 states / 28,554 orders / 49 goods | Implemented as a **model**; the numbers are model allocation, not engine shipment records |
| Verified game-access snapshot window | UI-timer collection, vtable+container probe, 5 s retry; R44 live data; world invalidation drill (R49) | Reader is passive; real save/menu round-trip still unverified |
| Market scoping | `state+0x48` = market id (59 markets, R53); UK=79 (R57); snapshot groups by market; `market_override` (R54) | Player-market auto-detection incomplete; default = largest market (`..., auto`) |
| Distinct routing/view/native geometry caches | Preview route cache + exact-points/style RibbonCache + freight view/ribbon cache; native tests PASS | Integrated; runtime hit counters observed |
| Global bounded 8192 native-object pool | RibbonCache tests cap/compatible-type LRU/pins/failures/hash collisions; runtime `native pool 989/8192` | Implemented and observed |
| Cold work <=16 uploads/frame and soft 2ms | Native timing/policy tests PASS | Policy implemented; real cold batch 127–155 ms in worker (not frame time) |
| 100ms settle / 250ms max, cancellation and ownership | Win32 worker handoff tests; cancel/exception tests | Implemented; freight/hover ownership still simplified |
| No engine pointers in workers and no scans/Dijkstra in rendering | Worker owns immutable map data; render consumes completed controls | Implemented; real snapshot handoff is the remaining seam |
| More consumption gives larger arrows | Eight width variants; absolute allocated quantity; GPU checks eight widths/nine palettes | Implemented; formal-UI width acceptance partial (real flows are thin) |
| Close/reopen cache reuse and unchanged-view zero uploads | Native tests verify reopen/ordering/identity/failed suppression; R55 10× open/close cycles stable | Partially verified; zero-upload per-frame not instrumented |
| Device/world lifecycle and no stale retained objects | Swap-chain resize handling; world-change invalidation (R49) | Save/menu round-trip and device reset still need proof |
| Memory plateau, performance and diagnostics | R48 3-minute plateau ±24 MB; R43/R44 worker 127–155 ms; R60 tab renders real numbers | Memory/perf evidence exists; simulation/frame comparison not measured |
| Built rail status | `builtRailEdges` empty; tab shows `Built rail: --` | **Incomplete** — no building/rail extraction yet |

Passing a helper test, resource verifier, mount log or submit counter does not
prove complete game-visible behavior. Completion requires the full acceptance
matrix with current executable/DLL identities (see `docs/transport/ACCEPTANCE.md`).
