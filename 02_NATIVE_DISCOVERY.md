# Local discovery and integration boundaries

The Python code is adapted from the source project's name-anchored recovery tools. Historical
address tables, old byte-signature constants, generated profiles and native source were removed.
The wrapper emits a research report, not a profile compatible with an existing native loader.

## Discovery recipes

| Capability | Local evidence chain |
|---|---|
| Arrow constructor | `CArrowObject` RTTI → complete object locator → vtable → unique constructor referrer |
| Constructor cross-check | RTTI for a lambda wrapper inside that constructor → wrapper constructor → enclosing constructor |
| Path setter | Military and supply subclass updaters → common callees → arrow-vertex-buffer allocator relationship |
| Constant-buffer virtual | Compare base, military and supply vtables; find the unique index where both subclasses agree and differ from the base |
| Constant-buffer creation | Find the shared updater callee writing the field consumed by the uploader |
| Type resolver | Military typed helper → name builder → string-keyed lookup receiving the built name |
| Bucket append | Constructor-derived type field → unique vtable method loading a signed bucket identifier |
| Render bucket list | Append's native context reader → bucket accessor → referenced data container |
| Render pass | Reader of that container with the matching index/context access pattern |
| Allocator | `CPdxNewDeleteAllocator` RTTI → vtable → unique static instance |
| Device global | Data global loaded by the constant-buffer creation path |

The wrapper additionally derives the bucket identifier's field displacement from candidate
instructions instead of shipping a game-specific value for it.

`ironpe.py` parses the PE image and its unwind metadata. It traces unwind chains when identifying
function extents, constructs cross-reference indexes, finds RTTI/string anchors, and uses Capstone
for instruction inspection. The source recipes contain compiler-pattern assumptions. A new patch,
compiler or changed allocator type name can invalidate them even if the gameplay looks unchanged.

## What success means

`CANDIDATE_FOUND` means a recipe's assertions passed on the supplied file. It is evidence for manual
inspection. It does not validate the running image, native argument layouts, thread safety, object
lifetime or GPU ownership. A code signature generated from a candidate verifies repeat identity;
it does not independently prove the candidate is the right function.

Keep the private report associated with the exact executable hash. Resolve runtime module location
locally, account for relocation, and verify the running image before enabling any calls/hooks.
Do not copy results from the source project's version or from another person's report.

If a recipe reports `NEEDS_REVIEW`, inspect its private failure record and the native callers.
Do not loosen uniqueness checks until something happens to match. Dependencies can also fail because
an earlier capability was unresolved. Review the first missing link before its dependents.

## Work that is deliberately not automated here

- The native path-context structure and its ownership semantics must be reconstructed from the
  current callers and checked against live values. There is no supplied structure layout.
- The object size/alignment and safe allocation/destruction strategy need current-build evidence.
- The complete ABI of each hook/call must be verified. A linear register-use scan is not a full ABI proof.
- The live render context and the correct pass must be observed; no preselected pass index is supplied.
- The base arrow's constant-buffer adaptation must be validated on the current vtables.
- Swap-chain/device reset, world reload, renderer shutdown and native registration lifetimes require live tests.
- Extracting your map's authored splines and building the shared transport graph is a separate adapter task.

These are the main places where blindly copying a working integration between versions can crash.

## Creating a local arrow type

Run from the extracted starter folder, substituting your own installation and mod paths:

```powershell
.\.venv\Scripts\python.exe -B tools/make_arrow_asset.py --game-root "D:/YourSteamLibrary/steamapps/common/Victoria 3" --out "D:/YourMods/MyTransport/gfx/models/military/local_freight_preview.asset"
```

The helper checks for the referenced local shader and texture, refuses output inside the game
installation, and refuses to overwrite an existing file. It creates only an arrowType text definition.
The default name is `localFreightPreview`. The implementing agent must create/register the rest of
the mod using the recipient's normal launcher setup, enable it, restart, and resolve that exact name.

The initial type uses the locally installed vanilla arrow shader. For custom import/export/heat
colors, make recipient-local shader variants or use a verified native per-object color facility.
Keep texture/overlay bindings, material effect names and constructor-time type registration consistent.
The source project used distinct type names for discrete color/opacity bands. Full shader or texture
files are not supplied in this ZIP.
