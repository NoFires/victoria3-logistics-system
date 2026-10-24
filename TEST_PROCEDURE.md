# In-game test procedure

## R93 current world recovery protocol

Read current hashes/status before testing. R93 prototype39c recovered native arrows after both main-menu reload (world2) and direct in-game loading (world3), with180 retained slots and zero additional uploads. The final8115373d candidate also checks graphics/device identity on cache hits; a new device still requires a separately verified rebuilding path. Older permanent-restart instructions below are historical.

Keep an autonomous test session paused, preserve the existing save, and leave exit autosave unchecked. Record initial market/state figures, slot count and uploads. Return to menu/reload, then open the native panel: require a strictly newer world, a current real frame, successful native lease verification and newly visible arrows. The unavailable-map message must clear after UI publication. Check no native submission between retirement and recovery; record old/new generation events and copied market scope. Repeat using in-game Load without menu. Unknown/replaced type, registration, path, buffer or graphics identity must remain disabled. Test different-save/new-game data separately; same-save recovery is not that acceptance.

After recovery test minimize/restore and actual client resize, restore original window bounds, and check map alignment/figures and error.log. These window operations are not a GPU device-removal test. Final performance/complete A24 must still cover their explicit remaining scopes in MASTER_PLAN.


## R69 lifecycle boundary and current expectations

R69 fixes numeric snapshot retirement and cancellation. A world reset now
requires a strictly newer world generation; a higher revision of the retired
world must be rejected. New snapshots immediately remove the old published
frame. Worker resets survive request coalescing and Cancel. Native panel
submission is allowed with ImGui hidden; this code change still needs live
verification against the current DLL.

Native arrow registration/destruction across actual world teardown remains
unverified. When the adapter detects a world replacement, R69 stops subsequent
access/submission of old native slots and labels the map unavailable until a
game restart. Numeric snapshots can refresh for the new world. This is a
temporary capability gate, **not** a pass for reload/main-menu lifecycle. The
old R49 invalidation drill no longer proves that arrows can resume after reset.
Anchor reuse at the same address/id/vtable may also evade the current detector;
a verified world lifecycle signal is still required for full acceptance.

Current native summaries include model allocation orders and a separate Freight
intermediary target/fulfilled/unmet row (R67). Missing values remain `--`.
The older prototype instructions below are historical where they conflict with
this current behavior. Never infer game visibility from offline tests alone.

Do this once with the current build. The loader is installed in
`binaries\version.dll`; the game must not be running while you copy a new build.
This DLL embeds its styles and map data. Do not enable a launcher mod/playset for it.
The log must show `embedded archive ready BEFORE gfx/models .asset enumeration`
during startup, then `resolved arrow type 'ironroadBuildingOutput'` on the map.

Native GUI acceptance for the current integration build:

1. Startup must log `native GUI archive mounted during native MountData` before
   `native GUI enumeration; prepared=1 mounted=1`. Check error.log for GUI errors.
2. Open a market: the fifth **Logistics / 物流** tab must appear beside the four
   original tabs. Switching/back/close must retain normal original panel behavior.
3. On Logistics, original search/category filters must change the original goods
   rows. A goods row must open the original goods panel with native prices,
   buy/sell orders and building details.
4. Freight/track summaries remain `--` until the snapshot bridge exists. Show/Hide
   currently changes its native GUI variable only; it is not yet a working native
   freight-render command. Do not treat the visible shell as completed logistics.

Offline generator checks: `tools/test_native_gui.py`,
`tools/test_native_gui_runtime.ps1` and `tools/verify_embedded.py`.

Current worker/width checks (not yet established by the earlier direction-only run):

1. Draw a route, then set **Preview consumption** to 5, 100, 500 and 5000.
   Width should grow while centreline, source/destination and arrowhead alignment
   stay fixed. This slider is a manual preview, not a live economic reading.
2. Switch the five Traffic colours in the colour menu. Check green/lime/yellow/
   orange/red variants load in game. The market view will choose these bands
   automatically from relative throughput once the live snapshot is connected.
3. Drag the amount/calibration slider quickly; only the latest settled result
   should activate. Press Clear or hide/close the panel during computation:
   an old completed route must not reappear.
4. Reopen, draw the same endpoints/style/width band, and check uploads remain
   unchanged while cache hits increase. Changing amount within a band requires
   no native upload. Reversing endpoints changes direction and geometry identity.
5. A disconnected authored or province route must report no route rather than
   silently drawing a chord. Chord remains an explicit preview mode.

`native/build_tests.ps1` exercises the actual worker with the installed derived
map and the native scheduling policy with deterministic mock native calls. GPU
tests execute the real shader at eight widths and verify all nine palettes.
These checks do not establish live many-ribbon render time, world reset safety,
or complete freight/economy integration.

Direction regression (offscreen, no game interaction):

```powershell
python tools\verify_embedded.py
python tools\test_flow_shader.py
python tools\test_flow_gpu.py
```

The GPU check uses the actual pixel shader and installed building-flow texture.
It checks forward/reverse arrow tips stay on the ribbon centre with the native
transverse UV range 0..1, and rejects the former half-width sampling as a negative
control. It does not replace the in-game camera/terrain/animation checks below.

## 1. Launch and check the panel

1. Start Victoria 3 normally.
2. No separate panel should appear on startup. Press **Insert** to open the
   optional **Ironroad debug** panel, initially on Diagnostics.
3. Its UI/State prototype pages are development tools. The player-facing native
   Logistics tab is still in integration; do not treat prototype widgets as
   proof of native GUI or live freight completion. See NATIVE_GUI_PLAN.md.
4. In **Diagnostics → Status**, expected:

   | Row | Expected |
   |---|---|
   | Loader | version.dll proxy |
   | Build | 1.13.11 |
   | Native hooks | installed |
   | Arrow type | `not resolved` → `ironroadBuildingOutput` after the first map frame |
   | ImGui | ready |

   If `Build` says *unsupported*, the loader saw a different executable hash and
   disabled the arrow calls on purpose; send the log line.

## 2. Draw the first ribbon

The pass-selection correction reads the same live domain/stage handles used by
the game's own arrow submission routines. The first 12 pass callbacks log
`scope=domain/stage/view native-scope=domain/stage eligible=...`. Once a route is
active, `SUBMIT scope=...` appears at most every five seconds. Do not infer a
visible ribbon from non-null buffers or counters alone: check the captured map.
`python tools\analyze_log.py` now reports only the latest process by default,
so a prior harness run cannot make a current game run look unsupported.

On the **Route preview** page the build runs an automatic short test route (Home Counties -> Lancashire, falling back
to Svealand -> Götaland when those states are unavailable) after the first few arrow
passes once the arrow type resolves. This exercises the native pipeline even
without a click; disable it with `auto_test=0` in
`%LOCALAPPDATA%\IronroadV3\ironroad.ini` or the panel checkbox.

1. Load a save (or start a new game) and wait until the map is interactive.
2. Open **Route preview**, choose **Origin** and **Destination**, and pick **Geometry** (`Province land path`
   follows land over the province graph; `Straight chord` is the fallback/comparison).
3. Click **Draw route**.
4. Choose **Colour**: Building flow green/orange/yellow/blue or Vanilla tan.
   `deploy.ps1` installs the standalone DLL; restart after deployment.
   The default is building-flow green. The launcher enabled-mod list may be empty.
5. Swap origin/destination and redraw: both glyphs and animation should reverse,
   with the same complete route endpoints. Repeat for green and orange; both use
   the building selection artwork. A nonzero submit counter alone does not prove this.
5. Expected in the panel:
   - `Route: active`
   - `submitted` keeps increasing once per frame
   - `last: ok: 48 pts, vb=yes cb=yes len=...`
5. The ribbon should be visible above the map between the two state centres.

If nothing is visible, first make sure the camera is over the chosen states. Then use the
**Calibration** sliders for a decisive visibility test:

1. Set `Height offset` to **+500** (leave scale at 0.01). A working ribbon must now float
   clearly above the terrain.
2. If it appears, lower the offset until it sits on the ground, then run
   `tools\calibrate.py` from a real movement arrow to get `height_scale`.
3. If it does not appear even at +500, the problem is not height; check `last:` in
   Diagnostics (`vb=` must be `yes`) and the pass/submission counters.

The log records `vb`, `cb` and `len` for every build, so a missing buffer is
distinguishable from wrong coordinates.

## 3. Capture exactly what the panel sees

When the auto-test route is built, the overlay also auto-captures one frame three seconds
later (`readback.ppm`), so the first launch is verifiable without any click. You can also
click **Capture frame (readback.ppm)** in Diagnostics at any time. The overlay copies the game backbuffer (including the panel) to
`%LOCALAPPDATA%\IronroadV3\readback.ppm`; convert it to PNG or look at it directly to see
whether the ribbon is present and where.

## 4. Calibrate from a real arrow (important)

1. Move an army or a fleet over a few provinces (any native movement arrow).
2. The log records up to 40 real `CAPTURE setpath ...` lines.
3. Close the game, then run:

   ```powershell
   python tools\analyze_log.py
   python tools\calibrate.py --ini "%LOCALAPPDATA%\IronroadV3\ironroad.ini"
   ```

   `calibrate.py` computes `height_scale = median(native y / heightmap sample)` and writes
   it to the ini. Restart the game and draw the route again.

## 5. What to report back

- Whether the panel appeared.
- The Status rows (especially `Arrow type`).
- Whether the ribbon drew, and if not, the `last:` diagnostics line.
- The full log at `%LOCALAPPDATA%\IronroadV3\ironroad.log`.

## 6. Rollback

```powershell
powershell -ExecutionPolicy Bypass -File E:\Code\v3-carrow-imgui-starter\deploy.ps1 -Uninstall
```

This removes the loader (only if it is the Ironroad build) and restores an original
`version.dll` if one was ever backed up. Saves and installed mods are untouched.
