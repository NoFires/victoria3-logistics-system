# Acceptance tests and diagnosis

## Test in layers

| Test | Pass condition |
|---|---|
| Discovery | All required candidates found uniquely; constructor derivations agree; private report matches the local executable |
| Loader | Original exports/behavior preserved; startup evidence visible; missing capability leaves the game usable |
| ImGui | Panel opens/closes; camera and input work; no per-frame database work; resize/reset handled |
| Single ribbon | One object follows a short known route at the correct map location |
| Native pass | Ribbon keeps drawing after the calibration army/tooltip is gone; one submission per relevant frame |
| Clear | No stale ribbon and no unfinished request resurfacing after close |
| Coastal river | Route selection, distance and painted polyline all use the authored coastal corridor |
| Reopen | Closing/reopening the same view reuses geometry |
| Filter/hover | Only the settled filter is committed; leaving a good restores the correct view |
| Unchanged view | Zero geometry uploads after settling; no repeated rebuild batches |
| Cold market view | Construction progresses within the configured per-frame count/time budget |
| Memory | Repeating the same view/filter cycle reaches a plateau; no continuing growth or new D3D allocation errors |
| Lifecycle | Save reload, main-menu return and device lifecycle do not use stale engine pointers or native objects |

Use a small route to establish ABI correctness before enabling hundreds of native calls. The
finished feature must support large markets; validation should isolate failures before scaling up.

## Offline helper tests

```powershell
py -3.12 -B tools/test_helpers.py
```

These tests use only the Python standard library. They verify the reference policy: 395 paths,
bounded cold batches, 1,000 unchanged requests, cached filters/reopen, capacity limits, cancellation,
debounce behavior and asset-name validation. They do not render anything in Victoria 3.

Write equivalent tests at the real native cache/scheduler seam when implementing it. Include
failed native uploads, exact geometry changes with unchanged endpoints, style changes, duplicate
requests, currently used cache-entry pinning, incompatible-type eviction refusal and partial-batch cancellation.

## Performance measurement

Use the same save, camera, game speed and overlay settings. Compare panel closed versus open after
the first load settles. Measure render-frame time and simulation progress separately. A steady FPS
counter alone does not establish that simulation ticks are healthy.

Record per completed batch:

- requested and visible ribbons;
- geometry uploads and cache hits;
- failed uploads and bounded native pool size;
- CPU update time excluding time between budgeted frames;
- skipped duplicate requests;
- optional process/GPU memory at a low sampling frequency.

The source project uses `DRAW PERF` summary lines. Analyze those locally with:

```powershell
py -3.12 -B tools/summarize_draw_log.py --log "D:/YourLogs/ironroad-proxy.log"
```

The helper prints only numeric summaries. It never echoes raw log lines, pointers or addresses.
Its tail limit is configurable. Keep raw game logs out of the shareable starter bundle.

Expected outcomes: stable view → no uploads; cached hover → no uploads; cold view → bounded batches;
new geometry/style → only required uploads. A changed goods sort order must not recreate every arrow.

## Known symptoms

| Symptom | Investigate |
|---|---|
| Nothing draws until an army moves | Submission is piggybacking on an incidental native append rather than the verified render pass |
| Crash while appending | Missing native render-context argument, wrong ABI, context retained beyond its lifetime, wrong hook entry |
| Geometry exists but looks wide/bright/wrong | Per-object constants missing/stale, wrong virtual uploader, duplicate submission, texture/overlay mismatch |
| Changing selected flag changes color unexpectedly | Native selection/highlight shader behavior is being confused with an arbitrary tint |
| Custom type lookup fails | Mod not mounted at startup, wrong asset name/path, asset parse errors, effect/texture lookup failure |
| Ribbons follow roads below rivers | Routing graph excludes authored coastal river links, or painting uses a different graph/edge sequence |
| Every mouse movement stalls | Immediate heavy callbacks, repeated roster scans, rebuilding all geometry, blocking logs |
| Repeated buffer creation errors | Excessive allocations, invalid/degenerate context or geometry, ownership bug, device problems; inspect before assuming a leak |
| Memory grows with repeated identical views | No native reuse, unbounded per-color pools, failed releases, queued stale batches |
| All freight changes when filtering | View-level filtering is feeding back into allocations or mutating the underlying economy snapshot |
| Native UI panel stuck open or log floods | Broken lifecycle visibility override or unsupported GUI callbacks; inspect gui.log as well as error.log |

For the last case, if integrating into native game panels later, preserve engine-managed window
visibility and verify every scripted callback against this build. Avoid attaching a root visibility
expression that overrides the native window lifecycle. ImGui itself does not require these GUI scripts.

## Reporting completion

Report exactly which layers were verified. A successful discovery report, a passing cache model
and a successful native-library load still leave live rendering, path fidelity and memory stability
unproven. Keep a reversible backup of the recipient's last known working integration.
