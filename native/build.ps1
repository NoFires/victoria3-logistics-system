# Build the Ironroad loader DLL with the local MSYS2 UCRT64 toolchain.
# Usage:  powershell -ExecutionPolicy Bypass -File native\build.ps1 [-Clean]
param([switch]$Clean, [switch]$ResourcesOnly)

$ErrorActionPreference = 'Stop'
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Split-Path -Parent $here
$tools = 'E:\Program\C Compiler\MSYS2\ucrt64\bin'
$gpp = Join-Path $tools 'g++.exe'
$gcc = Join-Path $tools 'gcc.exe'
if (-not (Test-Path $gpp)) { throw "Toolchain not found: $gpp" }
$env:Path = "$tools;$env:Path"

$build = Join-Path $here 'build'
$dist = Join-Path $root 'dist'
if ($Clean -and (Test-Path $build)) { Remove-Item -Recurse -Force $build }
New-Item -ItemType Directory -Force $build, $dist | Out-Null

$cxxflags = @(
  '-std=c++20', '-O2', '-g', '-fno-omit-frame-pointer',
  '-DUNICODE', '-D_UNICODE', '-DWIN32_LEAN_AND_MEAN', '-DNOMINMAX',
  '-D_CRT_SECURE_NO_WARNINGS', '-D_WIN32_WINNT=0x0A00',
  "-I$here\src", "-I$here\third_party\imgui", "-I$here\third_party\imgui\backends",
  "-I$here\third_party\minhook"
)

$objects = @()

function Compile-Cxx($src, $obj) {
  if ($ResourcesOnly) {
    $newestHeader = Get-ChildItem (Join-Path $here 'src') -Filter '*.h' | Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1
    if (-not (Test-Path -LiteralPath $obj) -or
        (Get-Item -LiteralPath $obj).LastWriteTimeUtc -lt (Get-Item -LiteralPath $src).LastWriteTimeUtc -or
        (Get-Item -LiteralPath $obj).LastWriteTimeUtc -lt $newestHeader.LastWriteTimeUtc) {
      throw "Stale object; run a full build first: $obj"
    }
    $script:objects += $obj
    return
  }
  Write-Host "CXX $src"
  & $gpp @cxxflags -c $src -o $obj
  if ($LASTEXITCODE -ne 0) { throw "compile failed: $src" }
  $script:objects += $obj
}
function Compile-C($src, $obj) {
  if ($ResourcesOnly) {
    if (-not (Test-Path -LiteralPath $obj) -or (Get-Item -LiteralPath $obj).LastWriteTimeUtc -lt (Get-Item -LiteralPath $src).LastWriteTimeUtc) {
      throw "Stale object; run a full build first: $obj"
    }
    $script:objects += $obj
    return
  }
  Write-Host "CC  $src"
  & $gcc -O2 -D_WIN32_WINNT=0x0A00 "-I$here\third_party\minhook" -c $src -o $obj
  if ($LASTEXITCODE -ne 0) { throw "compile failed: $src" }
  $script:objects += $obj
}

Compile-Cxx "$here\src\dllmain.cpp"      "$build\dllmain.o"
Compile-Cxx "$here\src\log.cpp"          "$build\log.o"
Compile-Cxx "$here\src\util.cpp"         "$build\util.o"
Compile-Cxx "$here\src\game.cpp"         "$build\game.o"
Compile-Cxx "$here\src\embedded_assets.cpp" "$build\embedded_assets.o"
Compile-Cxx "$here\src\native_gui.cpp" "$build\native_gui.o"
Compile-Cxx "$here\src\route_data.cpp"   "$build\route_data.o"
Compile-Cxx "$here\src\dev_snapshot.cpp" "$build\dev_snapshot.o"
Compile-Cxx "$here\src\variable_probe.cpp" "$build\variable_probe.o"
Compile-Cxx "$here\src\freight.cpp"      "$build\freight.o"
Compile-Cxx "$here\src\freight_engine.cpp" "$build\freight_engine.o"
Compile-Cxx "$here\src\freight_worker.cpp" "$build\freight_worker.o"
Compile-Cxx "$here\src\preview_worker.cpp" "$build\preview_worker.o"
Compile-Cxx "$here\src\logistics_ui.cpp" "$build\logistics_ui.o"
Compile-Cxx "$here\src\logistics_session.cpp" "$build\logistics_session.o"
Compile-Cxx "$here\src\logistics_panel.cpp" "$build\logistics_panel.o"
Compile-Cxx "$here\src\overlay.cpp"      "$build\overlay.o"
Compile-Cxx "$here\third_party\imgui\imgui.cpp"         "$build\imgui.o"
Compile-Cxx "$here\third_party\imgui\imgui_draw.cpp"    "$build\imgui_draw.o"
Compile-Cxx "$here\third_party\imgui\imgui_tables.cpp"  "$build\imgui_tables.o"
Compile-Cxx "$here\third_party\imgui\imgui_widgets.cpp" "$build\imgui_widgets.o"
Compile-Cxx "$here\third_party\imgui\backends\imgui_impl_dx11.cpp"  "$build\imgui_impl_dx11.o"
Compile-Cxx "$here\third_party\imgui\backends\imgui_impl_win32.cpp" "$build\imgui_impl_win32.o"
Compile-C "$here\third_party\minhook\src\buffer.c"      "$build\mh_buffer.o"
Compile-C "$here\third_party\minhook\src\hook.c"        "$build\mh_hook.o"
Compile-C "$here\third_party\minhook\src\trampoline.c"  "$build\mh_trampoline.o"
Compile-C "$here\third_party\minhook\src\hde\hde64.c"   "$build\hde64.o"
Compile-C "$here\third_party\minhook\src\hde\hde32.c"   "$build\hde32.o"

$runtime = Join-Path $dist 'freight'
New-Item -ItemType Directory -Force $runtime | Out-Null
$out = Join-Path $runtime 'freight.dll'
& python (Join-Path $root 'tools\package_runtime.py') --output $build
if ($LASTEXITCODE -ne 0) { throw 'embedded runtime packaging failed' }
& (Join-Path $tools 'windres.exe') (Join-Path $build 'embedded.rc') -O coff -o (Join-Path $build 'embedded_resources.o')
if ($LASTEXITCODE -ne 0) { throw 'embedded resource compilation failed' }
$objects += (Join-Path $build 'embedded_resources.o')
Write-Host "LINK $out"
& $gpp -shared -static '-Wl,--exclude-all-symbols' '-Wl,--no-insert-timestamp' -o $out @objects `
  '-ld3d11' '-ldxgi' '-ldxguid' '-ld3dcompiler' '-ldwmapi' '-luser32' '-lgdi32' '-lshell32' '-lole32' '-luuid' '-ladvapi32' '-lversion'
if ($LASTEXITCODE -ne 0) { throw "link failed" }

# Build the independent proxy; the core retains all embedded resources.
$proxy = Join-Path $dist 'version.dll'
& $gpp @cxxflags -shared -static '-Wl,--no-insert-timestamp' (Join-Path $here 'src\proxy.cpp') -o $proxy
if ($LASTEXITCODE -ne 0) { throw 'proxy build failed' }
foreach ($path in @($proxy, $out)) {
  $item = Get-Item -LiteralPath $path
  Write-Host ("Built {0} ({1} bytes)" -f $item.FullName, $item.Length)
}
