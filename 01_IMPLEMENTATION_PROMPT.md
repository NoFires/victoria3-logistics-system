# Implementation prompt: Victoria 3 CArrow + Dear ImGui transport overlays

You are implementing a native route-preview feature for my local Victoria 3 installation.
Read this entire prompt and the other documents in the supplied starter folder first.
Inspect my repository and current executable before choosing integration points.
Complete the native implementation locally, with tests and a reversible deployment.

## Intended result

I want a Dear ImGui control panel that can select two locations, show their routed path as
native world-space CArrow geometry, clear it, and display many freight paths with filters.
The ribbons must follow the actual route polylines, including authored coastal splines used
as rivers. Moving the camera must preserve correct alignment with the game map.

ImGui is the control and diagnostics surface. The game's CArrow renderer draws the world-space
ribbons. Commands from the panel must be queued; arbitrary UI callbacks must not call engine
rendering functions directly. Avoid calling Python from a frame hook or simulation tick.

The provided package contains no native implementation, binary locations, or precompiled bridge.
Write the required native code for this installation. The Python files supply discovery recipes
and an executable reference for rendering policy. Do not treat them as an injector or a ready-made mod.

## Inspect and record before implementation

1. Identify the executable, build identity, graphics backend, active playset, and mod folder.
2. Determine whether this project already has a proxy/plugin loader, hook manager, ImGui backend,
   Present callback, window-message handling, and a renderer-reset lifecycle. Reuse working infrastructure.
3. Inventory the local map data, spline graph and pathfinder. Determine how coastal and river
   corridors are represented, and whether routing already returns edge IDs and polylines.
4. Record a checkpoint and deployment backup. Keep game assets and the original executable intact.
5. Run the Python discovery script. Review the resulting candidates locally and verify ABI/layout
   assumptions against this exact executable. Produce an explicit readiness report for each capability.

A Windows proxy library beside the game executable is one possible loader. If used, it must
forward the real system library's exports correctly. Keep expensive initialization outside the
loader-lock entry point. Loader deployment and export parity need their own tests. An existing
working loader is preferable to adding a second one.

## Architecture and ownership

Use separate components with narrow interfaces:

| Component | Owns | May access |
|---|---|---|
| ImGui controls | Selection, filters, status and user commands | Copied UI snapshots and command queue |
| Game-data adapter | Current state identities and economy snapshots | Engine objects only inside verified game-access windows |
| Geography/router | Graph, route costs, edge IDs, spline polylines, revisions | Owned data; guarded updates from the adapter |
| View builder | Filtered edge freight totals and ribbon requests | Immutable economy/graph snapshots |
| Arrow cache | Persistent native arrow slots and owned point/context storage | Native graphics functions only at verified render-side entry points |
| Submission hook | Submits ready native arrows once per relevant frame/pass | The current native render context, valid only during that call |
| Compatibility layer | Locally derived locations and verification evidence | This executable's verified image and live invariants |

An engine object pointer from a state roster must not survive its guarded access window.
Keep stable handles and numeric copies; resolve handles afresh before using engine objects.
The render loop must not scan state databases, read building variables, solve trade allocations,
run Dijkstra, or execute scripted GUI effects. Avoid waiting on a simulation lock from rendering.

## Native discovery and verification

Required capabilities are the arrow constructor, path setter, arrow-type lookup, bucket append,
the appropriate arrow render pass, and the allocator expected by the path context. Also verify
the per-object constant-buffer creation/upload path and the vtable slot that invokes it.
The supplied `02_NATIVE_DISCOVERY.md` maps these capabilities to name-anchored recipes.

Independently establish all of the following locally:

- Object allocation size, alignment and lifetime rules.
- Constructor calling convention, argument roles and native type registration behavior.
- Path-context field order, scalar meaning, point layout, count/capacity types, allocator and ownership.
- Exact function entry points and enough instruction semantics to validate the ABI.
- Which fields establish drawable geometry and per-object shader constants.
- How the native renderer replaces/releases geometry buffers, and how device/world reset affects them.
- The full argument list of the bucket-append and render-pass functions.

Do not import another build's addresses or assume that a matching function prologue proves all of
these contracts. Discovery failures must disable the affected feature with a useful reason.
Never resolve multiple candidates by choosing the first. Never guess a missing context layout.

## Bring up ImGui

Integrate Dear ImGui with the verified local graphics backend. Establish a minimal panel with:

- Capability status, including why native drawing is disabled.
- Origin and destination selectors using stable IDs and names.
- Draw route and Clear controls.
- View selector: route preview, state imports/exports, market freight.
- Goods search/category filter and optional single-good hover selection.
- Diagnostics: pending requests, visible ribbons, new uploads, cache hits, pool size, last update time.

Initialize the backend only after the device and swap chain exist. Handle resize, minimized
windows, device reset, input capture and shutdown through the actual backend lifecycle.
Preserve the original window-message and rendering behavior. Restore graphics state after drawing ImGui.
Do not use screen-space ImGui polylines as the final transport overlay.

The ImGui callback copies a small request and returns. The native renderer consumes it later.
Python scripts run offline during development; they are not involved in this queue.

## Bring up one native CArrow

First resolve a known arrow type by its name on this build. For a custom type, generate a separate
mod asset using `tools/make_arrow_asset.py`, enable the mod, and restart before testing resolution.
Keep the vanilla troop arrow definitions intact.

Construct one native object in stable, correctly sized/aligned storage. Keep the object's backing
storage and any retained point/context arrays valid for the native lifetime. The constructor may
register the object in its type's container. Reconstructing it in place or freeing it without
unregistering it can leave dangling native references.

Provide a valid nondegenerate polyline and the context layout verified from native callers.
Use the game's expected allocator. Reject invalid counts, nonfinite coordinates and zero-length
geometry before invoking the engine. Do not pass a temporary host container as a native container
unless layout, allocation and ownership are all independently verified.

Call construction/path update only from the established render-side path with a live device.
Confirm that native geometry and per-object shader constants are actually ready.

### Constant-buffer detail that matters

The base arrow constructor can leave the per-object constant-buffer path inactive. The military
and supply subclasses can supply the real uploader through a different virtual implementation.
If that path is absent, the renderer can inherit stale shader constants from another draw, giving
wrong width, opacity or appearance even though a vertex buffer exists.

Compare the base, military and supply vtables. Derive the relevant differing index from the local
build and verify its target. A no-op function can occupy several slots: finding the first matching
stub address does not identify the correct slot. If adapting an object's vtable is necessary,
use a verified per-object copy; do not globally overwrite the game's shared vtable.

### Submission detail that matters

The native bucket append needs the render pass's own context. That context can be a temporary
created by the caller. Capture/use it only during the verified callback; never retain it for a
later Present call. Calling append without its second argument was a real crash cause in the
source project.

Hook or observe the game's actual append to identify the relevant pass, and submit your arrows
from the verified render-pass hook before the pass consumes that bucket. Continue calling the
original functions with their full verified arguments. Calibrate from native behavior rather
than importing a hardcoded pass selector. After calibration, custom arrows must draw without
requiring a moving army or a supply tooltip to remain visible.

Submit each ready object once per selected frame/pass. Add a recursion guard. Do not submit from
both Present and the arrow pass. Duplicate submission causes excessive brightness and unnecessary work.

## Geography and route fidelity

Use the same graph and cost policy for choosing routes, computing freight distances, and painting
them. Authored river transport may use coastal spline types. Admit the authored links themselves;
replacing them with dirt-road geometry underneath the river changes transport geography.

Trace a route to its ordered edge IDs. For each edge, orient its stored spline points consistently
with travel direction; concatenate while removing duplicate join points. Preserve turns and endpoints.
Verify map-to-world conversion against the recipient's map dimensions and native movement paths.
Handle seam wrapping, height/depth policy and coordinates explicitly; do not carry over dimensions
from another map. A straight chord between states is only an initial geometry check.

Version the graph whenever membership, connectivity, river availability or route costs change.
Invalidate shortest-path caches using the relevant revision. Separate physical geometry revisions
from freight quantities: changing how much moves over a link does not change its spline.

## Freight overlay behavior

The data adapter should provide actual allocated shipments, with source, destination, good,
quantity and route reference. Keep target imports distinct from shipments actually allocated.
Draw actual flows. This visualization does not require a new trade solver or changes to MAPI.

- State view: green incoming routes and deep red outgoing routes, with freight-weighted opacity.
- Market view: aggregate freight onto each physical segment once, then use green/yellow/red bands
  for relative traffic intensity. These colors do not imply a capacity bottleneck unless the model
  actually has segment capacities.
- Goods hover: show only that good's segment contributions. Leaving returns to the current filter.
- Market total: sum shipment freight once. Summing segment totals would count a shipment again
  for every segment it crosses.
- Goods share: good freight divided by all market freight; define the denominator explicitly.
- Laid track: sum unique built rail segments by physical length; it is separate from freight.

If the existing freight model defines each shipment's total charge from its entire route,
distinguish that charge from physical flow throughput when coloring intermediate segments.
Document whether edge intensity sums quantities or attributed freight charges, and use one
definition consistently.

## Performance requirements

Keep three different caches: routing results, view-level segment aggregates, and native render
geometry. A cached Dijkstra result does not prevent a renderer from rebuilding GPU buffers.

Identify cached ribbon geometry by graph revision/edge identity or exact point sequence, together
with the native constructor-time style type. The Python model uses exact points and style.
Deduplicate identical ribbons before submission. Sorting or a different goods order must not
invalidate native geometry. Returning to a cached view should reactivate existing objects.

Discrete color/opacity bands are acceptable. A quantity change within a band should require no
native geometry upload. If native type registration is fixed at construction, retain separate
cached variants for each used band. Do not re-run the constructor to change color.

Use a bounded pool. The source project caps it at 8,192 objects total, rather than that many per
each color bank. Evict only unpinned entries, and reuse a slot only with a compatible native type.
Verify buffer replacement semantics locally before implementing eviction. Do not add an unverified
destructor or raw resource-release call. Failed geometry must not cause an every-frame retry loop.

Coalesce UI changes with approximately 100 ms settling and a maximum wait of 250 ms. Hovering
across several rows should commit the final selection. Closing the panel must immediately cancel
pending construction and remove its submitted arrows. Give competing preview sources explicit
ownership/priority so one tooltip cannot leave another panel's view permanently cleared.

Limit cold geometry construction per frame. Start with at most 16 uploads and a soft 2 ms budget;
one indivisible native call may overrun that time. Make progress visible, cancel superseded work,
and reuse completed geometry. No thread should block until the whole market is constructed.

Never force a disk flush for every path, frame or diagnostic line. Batch routine logging, retain
prompt startup/error evidence, and emit aggregate timing counters. The source project measured
1,185 diagnostic lines at about 3.57 seconds with per-line flushing and about 4.9 milliseconds
after batching. That is a logging benchmark, not a promised game-speed multiplier.

## Language-neutral control flow

```text
WHEN UI selection changes:
    copy stable selection identifiers
    replace the pending request; record its generation

IN a valid game-data access window:
    obtain the necessary numeric snapshot
    resolve engine handles afresh and discard engine pointers before leaving

WHEN the selection settles or the data revision changes:
    reuse cached routes
    aggregate selected shipments onto their physical spline segments
    create a list of unique geometry-and-style requests
    replace the renderer's pending batch

ON the verified render-side update:
    ignore an identical request
    activate cached native objects
    build a bounded amount of missing geometry
    keep unfinished work for the next frame
    cancel work whose generation has been superseded

IN the verified native arrow pass:
    use this call's native context
    submit each ready ribbon once
    invoke the original pass correctly

ON close:
    cancel this view's work
    stop submitting its objects
    retain safely owned cache entries for reuse
```

## Required deliverables and acceptance

Produce the local native implementation, build/deploy instructions, capability report, original
custom mod definitions, meaningful regression tests, performance measurements and a rollback path.
Run the acceptance matrix in `03_TESTING_AND_TROUBLESHOOTING.md`.

Explicitly distinguish offline discovery, unit-test success, native loading success and observed
in-game drawing. If the current binary's ABI or lifecycle cannot be verified, leave that capability
disabled and identify the exact missing evidence. Do not claim a working game integration from
the Python model alone.
