$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$toolchain = 'E:\Program\C Compiler\MSYS2\ucrt64\bin'
$env:Path = "$toolchain;$env:Path"
$build = Join-Path $root 'native\build\tests'
New-Item -ItemType Directory -Force -Path $build | Out-Null
$exe = Join-Path $build 'native_gui_test.exe'
& (Join-Path $toolchain 'g++.exe') -std=c++20 -O2 -static -municode -DNOMINMAX -DUNICODE -D_UNICODE "-I$root\native\src" "$root\native\test\native_gui_test.cpp" "$root\native\src\native_gui.cpp" "$root\native\src\util.cpp" -lshell32 -lole32 -luuid -o $exe
if ($LASTEXITCODE -ne 0) { throw 'native GUI test compilation failed' }
$game = 'C:\Program Files (x86)\Steam\steamapps\common\Victoria 3'
$cache = Join-Path $build 'native-gui-test-cache'
$archive = & $exe "$root\dist\freight\freight.dll" $game $cache
if ($LASTEXITCODE -ne 0) { throw 'native GUI runtime tests failed' }
& python "$root\tools\test_native_gui_archive.py" $archive
if ($LASTEXITCODE -ne 0) { throw 'native GUI archive validation failed' }
