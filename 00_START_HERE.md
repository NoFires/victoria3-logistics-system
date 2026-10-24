# Victoria 3: native CArrow routes controlled by Dear ImGui

This is a source-free native-integration handoff: a complete implementation prompt,
Python discovery helpers, a Python rendering-policy model, and local testing instructions.
It is intended for a developer or coding agent with their own Victoria 3 installation.

## What is included

- `01_IMPLEMENTATION_PROMPT.md`: give this entire document to the implementing agent.
- `02_NATIVE_DISCOVERY.md`: how the Python recovery tools work and what still needs live verification.
- `03_TESTING_AND_TROUBLESHOOTING.md`: acceptance tests, known failure modes, performance checks.
- `tools/discover_local.py`, `recovery_core.py`, `ironpe.py`: read-only executable analysis.
- `tools/make_arrow_asset.py`: creates an original custom arrow definition referencing local game assets.
- `tools/render_model.py`, `test_helpers.py`: executable examples of caching, cancellation and update budgets.
- `tools/summarize_draw_log.py`: reports renderer timings without copying raw log lines.
- `tools/audit_bundle.py`: checks the shareable directory for prohibited file types and obvious address/code leaks.
- `requirements.txt`: Python analysis dependencies.

The ZIP contains **no binary addresses, recovered address profiles, machine-code signatures,
DLLs, executables, C++ source, shaders, textures, or copied game assets**. All locations
used during analysis are derived from the recipient's executable. Ordinary PE-format
constants and instruction-matching expressions in the Python parser are not a table of game addresses.

## Quick start on Windows

Use Python 3.12 for the pinned analysis dependencies. Extract this folder outside the game installation.
Run these commands from the extracted folder in PowerShell:

```powershell
py -3.12 -m venv .venv
.\.venv\Scripts\python.exe -m pip install -r requirements.txt
.\.venv\Scripts\python.exe -B tools/test_helpers.py
.\.venv\Scripts\python.exe -B tools/discover_local.py --exe "D:/YourSteamLibrary/steamapps/common/Victoria 3/binaries/victoria3.exe"
```

Replace the example installation path with your own. The last command prints only symbolic
success/failure statuses. It does not modify the executable, attach to the game, or install hooks.
Exit status zero means the discovery recipes found candidates; it does **not** mean native calls are safe.

For private review of the locally derived results:

```powershell
.\.venv\Scripts\python.exe -B tools/discover_local.py --exe "D:/YourSteamLibrary/steamapps/common/Victoria 3/binaries/victoria3.exe" --private-out "../private-v3-analysis/arrow-candidates.json"
```

The optional report contains locations recovered on that machine. Keep it outside this bundle;
the script refuses to write it inside the shareable folder. No such report is included here.

Then give `01_IMPLEMENTATION_PROMPT.md` to the person or agent implementing the native bridge.
They must write and compile their own native integration locally. Python handles offline analysis
and examples; it cannot replace the in-process native bridge that calls the game's renderer.

## Validation of this handoff

The extracted discovery helpers found all twelve reported candidate groups against the source
project's installed executable on September 4, 2026, including both constructor derivations.
The five Python helper tests passed. These results validate the tools on that executable,
not every Victoria 3 release or an independently written native implementation.

The source project's native renderer and ImGui integration have worked in-game. Its latest
performance changes passed offline scheduling and loading tests; the performance result in a new
recipient installation must be measured locally. Details are in the testing document.

No changes to market prices, MAPI, buildings, population needs, or trade allocations are required
to establish the CArrow/ImGui integration. An existing freight model can provide the route data later.
