# Native profile — Victoria 3 1.13.11

## R103 owned VFS configuration and directory policy (not a resource resolver)

Exact EXE7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f/image140000000 unchanged.03B4A8D0 normalizes backslash to slash in replacement root/path then03B4AA53 calls028B2150 on filesystem+48. Native array pointer+0,capacity+8,count+C; replacement entries stride40 hex contain root string0/path20. Exclusion array filesystem+60 has capacity68/count6C,stride20. Both strings have inline16 bytes,length10,capacity18,heap pointer0 when capacity>15. Existing mounts decoder extended to DecodeNativeVfsConfiguration, copying rules with mounts and actual search-path bytes and rereading all observations. It is still diagnostic-only/not included by product runtime; installed payload51ec975f unchanged.

Enumeration03B4AF80 exclusion stage uses length check and041DA8E0 byte-prefix comparison. No component-boundary or case-folding step there. Replacement directory match is exact byte equality:03B4B095 calls03B5BCA0 for NUL strings,03B4B0B9 compares equal-length views.03B5BCA0 SIMD/scalar paths return equality without case conversion;041DA8E0 compares bytes, not locale strings. For matching rules, root lookup uses actual filesystem+28 searchPaths byte equality; minimum index wins, owner included. Missing owner leaves INT_MAX (no cutoff). Child recursion still traverses parent enumeration; directory equality must not be promoted to a whole-path prefix rule. Pure NativeVfsDirectoryRootCount implements this step only; actual disk/ZIP enumeration, relative path/mount translation, map parsing and content graph identity remain unimplemented.

Unit tests cover both arrays/inline and heap strings,UTF-8/order,empty tables and empty exclusion,counts/capacity/pointer and string overflows,bad reads/mismatched terminators,all relevant copy-mutation stages,retirement,owned lifetime and rule priority/case/exact-vs-prefix behavior. Existing native/build_tests.ps1 already runs native_vfs_tests; targeted compilation/test PASS. New probe compiled and read user's PID57776/start23:07:08/windowThread36096/module7ff6b6960000/context0 twice:24 mounts,0 replacements,1 exclusion gui/gui_editor. Map directory limits24,excluded gui directory0. No attach/pause/input/write/getter/Mod changes. Actual modified-map replacement behavior is not live-proved.

Evidence/backup checkpoint2026-10-24-r103-vfs-rules/{replace-registration.c,replace-pair-append.c,enumeration.c,rules-assembly.txt,rules-live.txt,rules-live-repeat.txt}; native/build/vfs-round103.log. Next implement physical source resolution and recursive enumeration matching the engine before deriving a runtime content-identified graph.

## R101 completed-command owned snapshot handoff (production, partial lifecycle evidence)

Same exact EXE SHA2567f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f. Scope destructor033BD750 signature40 53 48 83 EC 20 48 8B, native command-runner call00CD2BBC/return00CD2BC1. Hook forwards the native destructor exactly once; only this return site with phase4/access2/hidden0/owns1/context1 can copy before forwarding, on the thread already holding that native scope. No new engine getter/scope/queue invocation, no simulation-lock acquisition from render/UI. Full catalog/state orders/rail copies use that thread's GS58 TLS and the existing bounded production decoders plus combined read-window checks. UI only drains owned data and validates source/epoch/latest data revision/full-handle anchor; model workers receive no engine pointers.

Resident requests remain after UI Take. Each actual copy has a unique monotonic data revision; newer data/error may replace ready data, stale success/failure cannot overwrite newer state. Paused completed commands always copy (the last command may precede an arbitrarily long idle); running copies remain limited to once per30s. Native BindSource advances the shared epoch before a changed machine/world/stateDB copy. Ensure replaces only older-world requests atomically, preserving a native new-world result when UI notices later. UI adopts the epoch/reset owners without retiring that result; queued GUI writes and native submissions require the current epoch. Numeric identity replacement is not a reliable retirement event or an ABA proof.

Failures retained:519864 initial20,267,078.94 stale after later initialization commands; paused speed change copied the correct20,267,075.76.269206 resident request fixed that, but reload copied the replacement source as old world1 before UI noticed/retires, discarding all ready new-source data and starving while paused. Dedicated resident test failed before fix; resident/source-bind tests and the entire unified entry pass after fix. Final payloadeed941b09eb4e2b6018d524e711d768dae0c584d567d15d7f67c00f51b94f2b2 (35151385B), bootstrap71dae36d…; dist equals installed.

PID52300/start2026-10-24T22:24:53.236934+08:00 native61820/UI35204. Paused initial rev2/3/4 copies125/110/110ms; UI accepted4. Direct same-save reload bound epoch2 at352203796; copies12/13/14 took125/125/110ms, UI adopted2 at352204187 and accepted14 at352204218, BEFORE F4/tab/speed/unpause actions. Market20,267,075.76,target30892.46,allocated14817.35,unmet16075.11;195 ribbons recovered, then state433 local Freight column/115 ribbons visible. Three hover/leave pairs:zero new native copies/zero null publications, nine queued totals identical. Fourth pair refused by foreground guard, excluded. User then requested manual control:all automation ended, no unpause/WM_CLOSE scheduled, PID remains for user. Current build running-update proof, reliable event/ABA, other-save/device/map scenarios and full performance remain pending.

Evidence checkpoint2026-10-24-r101-snapshot-handoff:eed941-handoff.log,eed941-world2-recovered.png,eed941-world2-state-prices.png,eed941-hover-summary.json,eed941-handoff-files.json; failed candidates retained. At handoff autosave hash/size/mtime and ini bytes equal pre-test. Full entry39 PASS lines/zero FAIL, selftest560468 bright pixels/default ImGui vertices0.

## R100 task joins and observed partial tick contract (diagnostic only)

Exact EXE remains7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f, image base140000000. Job wrapper VT0441A6A8:+8=03B292C0 tries queued execution, +10=03B29270 executes if needed then waits for job+60 state2; state0 queued/1 running/2 complete. 007C2540/00E78020 iterate handles first through+8 in reverse order, then all through+10. 03B29440 repeats the pattern before releasing handles. Do not label+8 alone as join.

CTickTask_UpdateStateTrades RTTI string0549FAF0/type descriptor0549FAE0/COL049D6998/VT044E6F38. Factory01343ED0 installs that table; its+38 leaf01339950 takes machine from batch+8, world+120, passes world+1C00 to0133C0C0. Nested013460B0/01346260 schedule through01348C30/01348DE0 then explicitly try/wait all handles before returning. Counter0103F6C0 has38 callers across unrelated container owners: this does not enumerate all state-order writes.

Opt-in probe_access adds exact0139E220 entry signature40 55 53 56 57 41 54 41 (full file hash also checked). Actual secondary Execute accepts the native command this pointer and returns void; caller00CD2810 passes that pointer and does not consume a result. Hook forwards it once and only logs16 pre/post scalar observations; it does not scan databases, copy orders, queue work, alter GUI or retain command/world pointers. native_access.h validates readable fields/type/mode/overflow; it grants no access lease. Default false, original user ini restored after testing.

Payloadd590417c… (34751412B) passed the unified entry and standalone checks. In PID56932/module7ff6b6960000/window thread38656, all16 observations were simulation thread45624, access2/hidden0/owns1/context1 before and after, date advanced6 each time, machine/world/state DB identities equal. This proves that callback's observed thread/contract for this bounded run, not full snapshot atomicity or every writer. Actual collector remains on the R98 path.

Next initial-paused/ordinary-command candidate:00CD2810 stage12 calls007E1B60 at00CD2B3D (return00CD2B42), then restores inner bookkeeping, then destroys its native access scope at00CD2BBC (return00CD2BC1). No new hook enabled there. Establish full input completion/initial load and an owned-copy handoff before switching production collection. See checkpoint2026-10-24-r100-simulation-completion/{join-static.txt,join-assembly.txt,trade-task-static.txt,command-assembly.txt,live-report.json,d590-session-only.log}.

## R99 game access scopes (static + external read-only observations; no new hook)

Same EXE SHA2567f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f, image base0x140000000. GameStateAccess enum at module+0514E580, source+588; HiddenGameStateAccess at0514E570/source+578. Observed scope values0 disabled,1 Interface,2 GameState,3 released/yielded. Interface assertion0443B0C0 allows Interface1 OR HiddenAllowed1. TLS array=GS:58, slot0:+5AE0 thread owns scope,+92C context. Asset filesystem TLS+8648 is unrelated.

Acquire033BD460 takes SRW0513E058 then SRW0513DCA0 then native mutex root+590, sets TLS+5AE0, releases only the outer0513E058. Ctor033BD640(storage*,access,context,source) optionally acquires, swaps global access/source and TLS context/native flag058FD79D, calls root virtual+F0. Dtor033BD750 calls+F8, restores swapped values and, if acquired here, unlocks root+590/SRW0513DCA0 and clears thread flag. Ghidra's typed qword root+0xB2 in the dtor means byte+590; independent assembly checked. Exact root primary VT04430800 has both+F0/+F8=0061C140 no-op in this build. Do not infer a separate world pointer swap from these virtuals.

Yield033BD820/8F0 temporarily releases/restores the scope. Crucially00E765E0 does the same around parallel batch execution;007C2C00 temporarily allows HiddenGameStateAccess for batch commands. Taking one native lock or reading on the window thread is not proof that every input writer has joined. No native scope invocation or new product hook has been enabled on this evidence.

CPartialDailyTickCommand factory007D7940 explicitly installs primary VT044365F0 and secondary04436590 at object+18. Actual execution0139E220 enters007E0D80 then007C8BD0 (command batches); adjacent VT entries/FC3020 belong to other commands and must not be used as its update function. Regular UI logic007D53E0→007D64B0 enqueues this command;007D5740 is startup. Graphics020808C0 is Draw with Interface scope, unsuitable for database copies. Next: prove a non-render completion phase, all relevant writes joined, exact thread/ABI and initial paused/load path before implementation.

External PROCESS_VM_READ-only PID49800/window thread46604/module7ff6b6960000: paused300 samples had access1/own1=22,access3/own0=278. Unpaused2200 samples had access1=199,2=29,3=1972,hidden1=30; context0, machine/world/DB unchanged. Reads are sequential, not atomic; mixed combinations cannot prove mutual exclusion. Bounded test normal WM_CLOSE, save hash/mtime unchanged. Product remains59e6d3d4…; no formal UI scope was open in this test. Evidence STATUS/WORKLOG R99 and private checkpoint2026-10-24-r99-access-window/access-{static,assembly}.txt, access-{paused,unpaused}.txt.

## R98 bounded state economy copy (exact 1.13.11; not an atomic epoch proof)

EXE SHA2567f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f, image base0x140000000. Native lookup007D40C0 resolves module+05964EF0, database+20 table/+2C slot count, stride10 hex and object at slot+8; object+8 is the full32-bit typed handle, low24 bits select the slot. Native state primary vtable044DE308. Production now copies the complete bounded table, validates full handles/types/duplicates, and invokes no native lookup. A test at slot2049 with handleAB000801 fails under the former2048 cap.

State production+C8/consumption+78 use counters with data+8, capacity+10, length+14, masks+20. Getter0103F440 reads only set-mask indices; allocator0103E450 sizes the array to current goods count (catalog+8C). Writer0103F6C0 updates the value and mask separately. This is precisely why rereads alone cannot establish an atomic economy epoch. Numeric values are fixed point1e5; dividing by100 produces milli-orders1e3. Negative/unreadable/out-of-catalog inputs are rejected, never changed to0.

native_economy.h returns owned state identities and orders. EconomyReadWindow rereads the combined metadata/order/rail observation set before acceptance; observed changes reject the candidate without falsely changing world generation. UI lifecycle identity checks resolve the full handle through safe copies rather than calling the native resolver or dereferencing an old pointer. A reliable world event/ABA-safe lifetime and atomic game-access window remain unverified.

Payload59e6d3d4… passed the unified entry and autonomous paused/one bounded unpaused update (1836-01-01→01-07). States1147, orders25396→25391, actual market0 card20,267,075.76→19,815,791.98 and195 visible ribbons. DLL total copy/recheck93–110ms; not P95/simulation performance acceptance. Evidence: STATUS/WORKLOG R98, private checkpoint2026-10-24-r98-economy-copy. No debugger attach, suspension or engine economic writes.

## R95 frozen filesystem capture (diagnostic only)

Exact EXE SHA2567f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f, static image base0x140000000. Getter RVA03B4D230 returns module+05144588+TLSindex*0128; initialized byte+0120. The window thread's TEB+58 points to TLS array, slot0 has initialized bit0 at+8600 and context index+8648. External read-only NtQueryInformationThread/ReadProcessMemory confirmed thread32864/context0 in PID48816, module0x7ff6b6960000. No native getter was called.

Filesystem+B8 points to rawVFS, frozen byte+FA, linked mounts+8. Mount item has data0/rootchar*8/mountPoint10/sourcePrefix18/prefixBytes20/backend28/next30. SearchPaths fs+28 is a null-terminated char** list. Mount03B2F4D0 prepends; RefreshRoots03B46FF0 copies this order. Directory backend enumerate/open/stat RVAs03B33D80/03B33FE0/03B34220; ZIP03B40A50/03B40CC0/03B410E0. ZIP backend table itself was on the heap, so validate function identity rather than table RVA. native_vfs.h copies owned strings, bounds/duplicate/bad-read/change/current-request checks, and reconciles search paths before publication. live_vfs_probe twice passed24 mounts (22directories/2ZIP); new fixture tests integrated in build_tests.ps1, full entry PASS. Payload remains8115373d…, installed=dist.

**Not a resource resolver:** Mod038A0620 passes descriptor+C0/CC replace paths to03B4A8D0, which appends normalized root/path string pairs to fs+48 (stride40/count54). Enumeration03B4AF80 consults that table and fs+60 exclusion paths (stride20/count6C), restricting matching directories to the owning root's search priority or higher. The decoder does not yet copy these tables or resolve current map content; runtime use/mapMod compatibility are not enabled by this unit. Precise nested/boundary semantics still require verification. Private evidence and original decompilations: checkpoints/2026-10-24-r95-active-vfs. Game was paused1836-01-01 and normally closed without saving. See STATUS R95 for the corrected Orleans rail-only connectivity diagnosis.

## R93c checked native-pool recovery

The world gate now compares nativePoolWorld with economyWorld. Once a current real frame is ready, the verified arrow pass checks the current asset style/type binding, constructor callback membership, retained arrow/path/point/VB/CB identity, original graphics/device identity and stable copies. It clears old active requests and marks only that exact world ready. There is no reconstruction/destruction or simulation lock. Device/graphics changes also retire cache hits; a swapchain replacement compares devices before releasing the old reference, so a recycled COM address cannot authorize the old pool.

Prototype39c recovered180 arrows after menu reload(world2) and in-game Load(world3), with uploads180 unchanged. Final8115373d (33,718,124 B, installed=dist) also passed menu reload with180 retained arrows, UI status restoration, minimize/restore and actual2560x1440→1920x1080→2560x1440 client resizing. Error.log GUI hits0; production lease decoding validated92 types/180 objects. Full regression and D3D11/default-hidden checks pass. Evidence: STATUS/WORKLOG R93c and private checkpoint2026-10-24-r93-world-recovery.

This establishes same-device/same-asset recovery for these save tests. A24 remains partial: another save, reliable world events under anchor-address reuse, new-device native-buffer rebuilding, asset replacement and active-map compatibility are still pending. Earlier permanent-restart gates below are historical, not the current implementation.


## R92 lifecycle evidence and R90/R91 UI acceptance

Real State.GetMarket identity (305 markets) replaces the invalid state+48 grouping. Native market/state figures, Chinese search/categories/empty results/hover/show-hide, native details/context menu and state switching have live evidence. R91 localized ledger values are fully readable; 20 verified close/reopen cycles reused native geometry (uploads371 unchanged), without sustained private-byte growth. This is partial acceptance, not the complete performance protocol.

R92 read-only snapshots across save/menu/reload are byte-identical for92 ironroad types/371 registered arrows, including path/VB/CB pointers. The type's stride0x48 vector holds geometry-rebuild callbacks, not proven world ownership. Constructor registration and type/arrow destructors were inspected; direct teardown remains disabled. The permanent world-retirement gate still blocks arrows after reload, while current-world numeric data resumes. R92 adds a separate localized unavailable-map message. No recovery implementation or device/asset replacement acceptance is claimed. Exact evidence and hashes: STATUS/WORKLOG R92 and private checkpoint2026-10-24-r92-native-lifetime. Historical statements below are superseded where they conflict.


## R90 market identity correction

State.GetMarket follows the country/explicit MarketArea branch and resolves the native market object, whose full ID also keys the GUI wrapper. state+0x48 is not that chain. The production read-only decoder matched all 1147 live states (3 area branches): state433 maps to country1/market0. There are305 native markets versus52 old field48 clusters; old market-scope allocation claims are invalidated. state_market.h validates typed handles, RTTI-derived vtables and reread stability. Static assembly/whole-roster evidence: private checkpoints/2026-10-24-r90-native-markets; EXE SHA-2567f53e326… unchanged. No native getter/resolver is invoked by the decoder.


## R82 actual state/region identity (important correction)

Live state433 (user's Midlands screenshot) belongs to market111 and points at
state+0xE78 to a region definition, vtable RVA0x44E7850, engine definition ID17
at+0x10, key STATE_MIDLANDS MSVC string+0x18. Authored map ID is20. Registration
0xE83A0 -> State.GetStateRegion callback0x1A06B80 -> definition0x15715A0;
constructor0x156D1D0 verifies the definition ID/name layout. Critical instructions
independently checked with Capstone. +0xE48/0x7BB480 was an early discarded
candidate, not this region chain. Runtime state IDs must never be used as map IDs.
Actual new key decoder maps1147/1147 runtime states to675 authored geography keys.
Only read-only process access, no attach/breakpoint/UI/engine calls. New adapter
is built but not deployed; prior live geographic allocation conclusions need
revalidation. Full actual-map/Mod resource and geometry revision support remains
pending. Private evidence: state-region-r82-notes.md / assembly.txt / live.bin.


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


Local-only reversing notes. Every address is an **RVA** for the executable identified
below. Do not reuse these numbers for another build; the loader refuses to call anything
when the file hash or an entry-point signature differs.

- Executable: `binaries\victoria3.exe`
- SHA-256: `7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f`
- Size: 96,872,568 bytes
- Branch/revision: `release/1.13.11` (`caligula_rev.txt`)
- Image base: `0x140000000` (all runtime use is `module base + RVA`)

## Derivation

R74 filter/cache research: GetFilters callback0x225B150 wraps CPdxArray of
GoodsCategory bytes at panel+0x110 (ptr+0,capacity+8,count+0xC).
IsSelected0x225A630 returns !contains(category); setter0x225A730 changes
exclusions and sets wrapper+0x320 dirty. Search getter0x225AFB0 returns
panel+0x80 CGuiSearchBar; normalized string+8 is panel+0x88. Callback0x225DB90
writes it to market wrapper+0x328 and marks dirty. Unicode lower loop0x3B1AD60
is part of vanilla normalization; native matching also uses localized keywords.
AccessMarketGoods registration0x1D9A60 ->0x1CDFAC0 ->native0x1CD8650,
cache helper0x1CD8690 and builder0x1CDAAF0/0x1CDA600. Return is cached rows
at wrapper+0x60; cache lock+0x54,dirty byte+0x320. Registry global0x59684A0
holds a sorted pointer vector (ptr+0,capacity+8,count+0xC,lock+0x18), exact
market wrapper vtable0x45850B0/ID+0x10. Cached row vtable0x4552218,
definition ptr+0x10,state+0x18 must FFFFFFFF,market+0x1C; definition uses
the R72 verified goods layout. Candidate observes writers only, never locks.
Live sample: two registry entries IDs0/FFFFFFFF, both dirty1/rowCount0;
actual decoder replay rejects as pending. Positive filtered rows are unobserved;
no new DLL execution or filter interaction is established. Evidence private
native-filter-r74-static.md/live.json/notes.md and project round74 logs.

R73 interface-state research: global RVA0x592D0E0 points to the current
CInGameInterfaceIdler (vtable0x4431500). Native resolver0x2080CD0 requires its
machine at+0x18 to have phase4 at machine+0x588, then RTTI-casts gfx at+0x10
(CIngameInterfaceIdlerGfx, vtable0x45B13D8). The candidate does no cast/call:
it validates the exact primary vtable and copies gfx+0x20 -> handler
(CIngameInterfaceHandler, vtable0x45AE328). Handler constructor0x2045AE0 embeds
InformationPanelBar at+0xD8, with self-pointer, vector+8, count+0x14 (56).
Bar constructor0x20A8900 places CMarketPanel at vector slot9, vtable0x45CC9C0,
name MSVC string+8, selected tab string+0x30, market ID+0x70.
IsPanelOpen registration0x30F500 -> callback0x2129E50 -> native0x21255E0,
tail helper0x2125610: bar history pointer+0x20, count+0x2C; open iff count>0,
bar byte+0x40==0 and last 0x30-byte history entry's kind==9. Selected-tab native
comparison0x214C120 reads panel+0x30 (setter0x214C770).
The complete closed chain was observed live in PID52316 and replayed through
market_panel.h. All original functions remain uncalled by the candidate.
New DLL execution/open transitions, same-frame close and full lifecycle remain
unverified; current candidate polls the existing UI timer at500ms. Raw evidence
is private market-panel-r73-static.md / live.json / notes.md.

R71 static / R72 live goods metadata: name registrations
`0x94460` / `0x94310` / `0x92830` identify IsLocal / IsTradeable / GetKey.
Callbacks `0x1978310` / `0x1978230` dereference wrapper+0x10, then definition+0x40:
local = bit 1, tradeable = bit 2 and not local. Direct key getter `0x1957E50`
copies definition+0x18 MSVC string (length +0x28, capacity +0x30). CGoods primary
vtable `0x4517F80`, definition size 0x70, ID +0x10, category byte +0x41. Database
primary vtable `0x4517DF0`, vector +0x80, count +0x8C (postload `0x174F8E0`,
max 128). Constructor `0x1753C10` assigns ID from database count.
Singleton getter `0x7B7290` uses TLS index and can initialize, lock and register
cleanup: do not call it as a harmless read. Its slot table starts `0x514E0D8`,
stride 16, initialized flag +8; no first-populated-slot selection is authorized.
The pure decoder in goods_catalog.h requires explicit exact-build identities and
a safe reader. R72 observed all53 native definitions and main-thread TLS context,
verified IDs/vtables plus key/category/local/tradeable by installed definition key,
and replayed the actual decoder/context resolver on copied bytes. The candidate
goods_snapshot.h/CollectRealEconomy caller observes GS:0x58 -> EXE TLS storage,
initialized bit+0x8600, token+0x8648 (low32 index<16), then only that initialized
context slot. Safe RPM and context/catalog rechecks run on the existing UI timer.
The new DLL adapter is NOT_RUN; stable rereads do not prove an atomic economy
snapshot or all object lifetimes. Evidence: private goods-catalog-r71-static.md
and goods-context-r72-notes.md.

The standalone DLL mounts its embedded archive immediately after a native
MountData call at RVA `0x3B47C40`, before VFS configuration freezes. The archive
places its type definitions under `gfx/models/military` so the normal `.asset`
parser consumes them. A diagnostic hook at `0x3B42800` verifies mounting happened
before the asynchronous `gfx/models` / `.asset` pre-enumeration cache is populated.
Its full arguments are three pointers to 16-byte string views and an int depth.
`0x3807B00` and `0x3800070` were rejected after runtime evidence showed these are
ThreadedInitTextureLookup, not model-definition parsing. ZIP mounting is performed by
the engine's context-local VFS entry `0x3B2F4D0` (ignored first argument, UTF-8
archive path in RDX, mount root in R8), followed by `0x3B46FF0`, as in the native
MountData caller. All four entry signatures join the executable hash gate.
These are static ABI findings; fresh startup registration logs provide runtime
evidence separately. No asset registration occurs in a render or simulation hook.

`tools/discover_local.py` (adapted from the supplied starter) performed name-anchored
recovery against this exact file. It walks RTTI complete object locators, `.pdata` unwind
extents, RIP-relative cross references and Capstone instruction patterns. It does **not**
execute the game. All twelve recipes returned `CANDIDATE_FOUND`; the private report is
`E:\Code\v3-analysis\arrow-candidates.json`.

The manual work on top of the discovery was reconstructing the path context and the
per-object data that the renderer consumes, from the two shipped arrow updaters and the
render pass itself.

## Entry points

| RVA | Symbol | Evidence |
|---|---|---|
| `0x037FD4D0` | `CArrowObject::CArrowObject(CPdxArrowType const*)` | unique referrer of the `CArrowObject` vtable `0x481D3E0`; the constructor-lambda cross-check agrees |
| `0x026D0A20` | supply arrow factory | `mov ecx, 0xC0; call operator new; call 0x37FD4D0; store supply vtable 0x46588E8; [obj+0xB0]=0` |
| `0x026D7150` | military arrow factory | same shape, installs `0x4658A08` |
| `0x037FD5B0` | `CArrowObject::SetPath(ArrowPath const*)` | unique shared callee of both updaters reaching the `SArrowVertex` buffer allocator |
| `0x037FCCD0` | ribbon edge generation | called by `SetPath` with `2*width*0.5`; consumes the centerline vector |
| `0x037FDC50` | vertex buffer build | called by `SetPath`; creates the GPU buffer and writes `+0xA8` |
| `0x026CEF30` | constant buffer creation | unique shared callee storing the field read by vtable slot 15 |
| `0x037E7D60` | `CAssetFactory::GetArrowType(std::string const*)` | called by the typed helper with the freshly built name in `rdx` |
| `0x037FE620` | bucket append | unique `CArrowObject` vtable slot reading `this+0x50` type and `type+0x30` bucket id |
| `0x037FD860` | arrow render pass | unique reader of the bucket container indexing it with `[rcx]`, `[rcx+8]`, `[rcx+0x10]` |
| `0x0512CDC0` | `CPdxNewDeleteAllocator` singleton | unique `.data` qword carrying vtable `0x4419230` |

Signature bytes checked by `Game::Initialize` (first 8 bytes at each RVA) are in
`native/src/offsets.h`.

## ABI

x64 Microsoft calling convention; `__fastcall` in the source is documentation only.

```c
void* __fastcall GetArrowType(void* unused_rcx, MsvcString* name);
void* __fastcall CreateSupplyArrow(CPdxArrowType* type);      // returns 0xC0 object
void  __fastcall SetPath(CArrowObject* self, ArrowPath* path);
void  __fastcall EnsureConstantBuffer(CArrowObject* self);
void  __fastcall BucketAppend(CArrowObject* self, PassContext* ctx);
void  __fastcall RenderPass(PassContext* ctx);
void* __fastcall Allocator_Allocate(void* alloc, size_t bytes, size_t align);  // vtbl[1]
```

`GetArrowType` ignores `rcx`: its first action is `call 0x38181E0`, a function-local-static
getter that never reads `rcx` and returns the asset factory. It reads `rdx` as an MSVC
`std::string`: data/SSO pointer at `+0x00`, size at `+0x10`, capacity at `+0x18`
(`cmp qword ptr [rdx+0x18], 0xF; mov ebx, [rdx+0x10]`). The loader builds that layout by
hand; names longer than 15 characters use a heap buffer.

`SetPath` first stores `path` at `self+0x70`, then calls the path interface:
`0x2674100(path, &temp)` copies `path->points` (a `CPdxVector<SVector3>`) into a temporary
with the game allocator. `.pdata`/instruction evidence for the layout:

```
02674100  test rcx,rcx ; je ret
02674108  lea rdx,[rcx+0x10]     ; source vector
0267410c  mov rcx,rax            ; destination temp vector
0267410f  jmp 0x25EAED0          ; CPdxVector::operator=
```

`0x25EAED0` uses the destination allocator (`+0x10`, vtable slot 1, align 4) and copies
`count*12` bytes; it never takes the source allocator. Element stride 12 (`SVector3`).

The render pass reads the retained path each frame:

```
02674120  test rcx,rcx ; je ret
02674125  eax=[rcx+4] ; *rdx=eax
0267412a  eax=[rcx]   ; *r8 =eax
0267412f  eax=[rcx+8] ; *r9 =eax
```

The pass builds constant buffer 0 as
`{ path[1], path[0], path[2], arrow+0xA8, type+0x68 }`, which is exactly the shader's
`{ AnimationLength, MoveLength, ClipLength, TotalLength, Width }`. The loader sets
`moveLength = 1e9` (fully drawn), `animationLength = clipLength = 0`.

## Object field map

Observed in the constructor (`0x37FD4D0`), both factories, both path updaters
(`0x26D0140`, `0x26D69B0`) and the render pass:

```
0x00 vtable
0x08 u32 0
0x0C (padding)
0x10 4x4 transform (identity; translation at 0x40/0x44/0x48/0x4C=1)
0x50 type*
0x58 u32 -1
0x5C u8  flags; ctor ORs 7; render pass requires bit 0 set
0x60 constant buffer (shared_ptr-like object created by 0x26CEF30)
0x68 vertex buffer (created by 0x37FDC50)
0x70 path*            <- retained and dereferenced every frame
0x78 CPdxVector<SVector3> centerline
0x90 CPdxVector<SVector3> ribbon edges (offset/subdivided)
0xA8 f32 total length (written by the vertex builder)
0xAC u32 0
0xB0 u64 supply/military constants uploaded as CB1
```

The supply vtable slot 15 (`0x26CF000`) uploads 8 bytes from `this+0xB0`:
`rcx=[rdx]` (device), `rdx=[this+0x60]` (CB), `r8=this+0xB0`, `r9d=8`, `jmp [rax+0xB8]`.
Zeroed `+0xB0` therefore means `IsSelected=0, IsFaded=0`, i.e. normal drawing.

## Pass context

The render pass receives one pointer with three dwords at `+0x00`, `+0x08`, `+0x10`. It
validates them against nested global containers, obtains a bucket list via `0x3828380`,
and iterates bucket entries of `0x18` bytes (`{data, capacity, count, allocator}` at
`+0`/`+8`/`+0xC`/`+0x10`) containing arrow pointers.

`BucketAppend(self, ctx)` reads the **same** triple from `ctx` (`[rdx]`, `[rdx+8]`,
`[rdx+0x10]`) and grows the bucket container to `ctx[2]` via `0x3820A10`, then appends
`self`. Submission therefore happens in the pass hook, using the pass's own `rcx` as the
append context, before the original pass runs. Submitting from `Present` is wrong: the
context is a temporary owned by the caller.

## Why the supply subclass is used

The base `CArrowObject` shares slot 15 with both subclasses but the slot target is a no-op
(`0x61C140`); the real constant-buffer uploader is `0x26CF000`, reached only through the
supply/military vtable. Constructing through `0x26D0A20` gives a correctly sized/aligned
`0xC0` object, installs the supply vtable, and zeroes `+0xB0`. The loader never writes the
game's shared vtable.

## Constants used

- Ribbon width argument: `SetPath` calls `0x37FCCD0(vec, 2*type[0x68]*0.5f, 0)`; `0.5f`
  lives at `0x486ACF0`.
- Type fields: `+0x30` i16 bucket id, `+0x68` f32 width, `+0x6C` i32 subdivisions,
  `+0x70` u8 line smoothing.
- Object size: `0xC0` (`mov ecx, 0xC0` in both factories).

## Verification boundary

The static layout above is derived from the shipped callers, not from symbols. It has not
been observed against a live renderer in this session. The first live run is expected to
confirm:

1. `GetArrowType("supplyRouteArrow")` returns a type with a sane width/subdivision/bucket.
2. `CreateSupplyArrow` returns a `0xC0` object whose vtable is `0x46588E8`.
3. `SetPath` builds a vertex buffer (`+0x68 != 0`) and total length (`+0xA8 > 0`).
4. `EnsureConstantBuffer` creates `+0x60`.
5. The pass hook appends once per frame and the ribbon is visible.

The `CAPTURE setpath obj=... type=... n=... first=(...) last=(...)` log lines from the
SetPath hook provide items 2–4 plus real world coordinates for calibration.
