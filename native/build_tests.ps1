# Build the local test harness (no game required).
$ErrorActionPreference = 'Stop'
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$root = Split-Path -Parent $here
$tools = 'E:\Program\C Compiler\MSYS2\ucrt64\bin'
$gpuPython = (Get-Command python -ErrorAction Stop).Source
$env:Path = "$tools;$env:Path"
$testBuild = Join-Path $here 'build\tests'
New-Item -ItemType Directory -Force $testBuild | Out-Null
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -municode -static `
  -o (Join-Path $testBuild 'harness.exe') (Join-Path $here 'test\harness.cpp') `
  '-ld3d11' '-ldxgi' '-luser32' '-lgdi32'
if ($LASTEXITCODE -ne 0) { throw "harness build failed" }
Write-Host "Built $testBuild\harness.exe"
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'frame_timing_tests.exe') (Join-Path $here 'test\frame_timing_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'frame timing tests build failed' }
& (Join-Path $testBuild 'frame_timing_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'frame timing tests failed' }
& $gpuPython (Join-Path $root 'tools/test_frame_timing.py')
if ($LASTEXITCODE -ne 0) { throw 'frame timing parser tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static -DNOMINMAX "-I$here\src" `
  -o (Join-Path $testBuild 'gui_variable_keys_tests.exe') (Join-Path $here 'test\gui_variable_keys_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'GUI variable key tests build failed' }
& (Join-Path $testBuild 'gui_variable_keys_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'GUI variable key tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'native_access_tests.exe') (Join-Path $here 'test\native_access_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'native access tests build failed' }
& (Join-Path $testBuild 'native_access_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'native access tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'native_snapshot_tests.exe') (Join-Path $here 'test\native_snapshot_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'native snapshot tests build failed' }
& (Join-Path $testBuild 'native_snapshot_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'native snapshot tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'native_vfs_tests.exe') (Join-Path $here 'test\native_vfs_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'native VFS tests build failed' }
& (Join-Path $testBuild 'native_vfs_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'native VFS tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'native_rail_tests.exe') (Join-Path $here 'test\native_rail_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'native rail tests build failed' }
& (Join-Path $testBuild 'native_rail_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'native rail tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'native_shipping_tests.exe') (Join-Path $here 'test\native_shipping_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'native shipping tests build failed' }
& (Join-Path $testBuild 'native_shipping_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'native shipping tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'native_domestic_tests.exe') (Join-Path $here 'test\native_domestic_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'native domestic source tests build failed' }
& (Join-Path $testBuild 'native_domestic_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'native domestic source tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'native_world_trade_tests.exe') (Join-Path $here 'test\native_world_trade_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'native world trade tests build failed' }
& (Join-Path $testBuild 'native_world_trade_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'native world trade tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'native_trade_routes_tests.exe') (Join-Path $here 'test\native_trade_routes_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'native trade route tests build failed' }
& (Join-Path $testBuild 'native_trade_routes_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'native trade route tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'native_trade_geometry_tests.exe') (Join-Path $here 'test\native_trade_geometry_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'native trade geometry tests build failed' }
& (Join-Path $testBuild 'native_trade_geometry_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'native trade geometry tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'native_splines_tests.exe') (Join-Path $here 'test\native_splines_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'native spline tests build failed' }
& (Join-Path $testBuild 'native_splines_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'native spline tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'native_sea_geography_tests.exe') (Join-Path $here 'test\native_sea_geography_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'native sea geography tests build failed' }
& (Join-Path $testBuild 'native_sea_geography_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'native sea geography tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'native_ports_tests.exe') (Join-Path $here 'test\native_ports_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'native port tests build failed' }
& (Join-Path $testBuild 'native_ports_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'native port tests failed' }
& $gpuPython (Join-Path $root 'tools\test_native_trade_selector.py')
if ($LASTEXITCODE -ne 0) { throw 'exact-build native trade selector harness failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'native_economy_tests.exe') (Join-Path $here 'test\native_economy_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'native economy tests build failed' }
& (Join-Path $testBuild 'native_economy_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'native economy tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'publish_key_tests.exe') (Join-Path $here 'test\publish_key_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'publish key tests build failed' }
& (Join-Path $testBuild 'publish_key_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'publish key tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'state_panel_tests.exe') (Join-Path $here 'test\state_panel_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'state panel tests build failed' }
& (Join-Path $testBuild 'state_panel_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'state panel tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'state_market_tests.exe') (Join-Path $here 'test\state_market_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'state market tests build failed' }
& (Join-Path $testBuild 'state_market_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'state market tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'goods_catalog_tests.exe') (Join-Path $here 'test\goods_catalog_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'goods catalog tests build failed' }
& (Join-Path $testBuild 'goods_catalog_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'goods catalog tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'market_panel_tests.exe') (Join-Path $here 'test\market_panel_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'market panel tests build failed' }
& (Join-Path $testBuild 'market_panel_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'market panel tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'arrow_geometry_tests.exe') (Join-Path $here 'test\arrow_geometry_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw "arrow geometry tests build failed" }
& (Join-Path $testBuild 'arrow_geometry_tests.exe')
if ($LASTEXITCODE -ne 0) { throw "arrow geometry tests failed" }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static -DUNICODE -D_UNICODE -DNOMINMAX `
  "-I$here\src" -o (Join-Path $testBuild 'spline_tests.exe') `
  (Join-Path $here 'test\spline_tests.cpp') (Join-Path $here 'src\route_data.cpp') `
  (Join-Path $here 'src\util.cpp') (Join-Path $here 'src\log.cpp') '-ladvapi32' '-lshell32' '-lole32' '-luuid'
if ($LASTEXITCODE -ne 0) { throw "native spline tests build failed" }
& (Join-Path $testBuild 'spline_tests.exe')
if ($LASTEXITCODE -ne 0) { throw "native spline tests failed" }
& python (Join-Path $root 'tools\test_splines.py')
if ($LASTEXITCODE -ne 0) { throw 'spline source/link decoder tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'arrow_scope_tests.exe') (Join-Path $here 'test\arrow_scope_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw "arrow scope tests build failed" }
& (Join-Path $testBuild 'arrow_scope_tests.exe')
if ($LASTEXITCODE -ne 0) { throw "arrow scope tests failed" }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static "-I$here\src" `
  -o (Join-Path $testBuild 'arrow_lease_tests.exe') (Join-Path $here 'test\arrow_lease_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'arrow lease tests build failed' }
& (Join-Path $testBuild 'arrow_lease_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'arrow lease tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static -DUNICODE -D_UNICODE -DNOMINMAX "-I$here\src" `
  -o (Join-Path $testBuild 'freight_tests.exe') (Join-Path $here 'test\freight_tests.cpp') `
  (Join-Path $here 'src\freight.cpp') (Join-Path $here 'src\route_data.cpp') `
  (Join-Path $here 'src\util.cpp') (Join-Path $here 'src\log.cpp') '-ladvapi32' '-lshell32' '-lole32' '-luuid'
if ($LASTEXITCODE -ne 0) { throw "freight tests build failed" }
& (Join-Path $testBuild 'freight_tests.exe')
if ($LASTEXITCODE -ne 0) { throw "freight tests failed" }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static -DUNICODE -D_UNICODE -DNOMINMAX "-I$here\src" `
  -o (Join-Path $testBuild 'freight_engine_tests.exe') (Join-Path $here 'test\freight_engine_tests.cpp') `
  (Join-Path $here 'src\freight_engine.cpp') (Join-Path $here 'src\native_trade_view.cpp') (Join-Path $here 'src\freight_worker.cpp') `
  (Join-Path $here 'src\freight.cpp') (Join-Path $here 'src\route_data.cpp') `
  (Join-Path $here 'src\util.cpp') (Join-Path $here 'src\log.cpp') '-ladvapi32' '-lshell32' '-lole32' '-luuid'
if ($LASTEXITCODE -ne 0) { throw "freight engine tests build failed" }
& (Join-Path $testBuild 'freight_engine_tests.exe')
if ($LASTEXITCODE -ne 0) { throw "freight engine tests failed" }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static -DUNICODE -D_UNICODE -DNOMINMAX "-I$here\src" `
  -o (Join-Path $testBuild 'logistics_session_tests.exe') (Join-Path $here 'test\logistics_session_tests.cpp') `
  (Join-Path $here 'src\logistics_session.cpp') (Join-Path $here 'src\freight_worker.cpp') `
  (Join-Path $here 'src\freight_engine.cpp') (Join-Path $here 'src\native_trade_view.cpp') (Join-Path $here 'src\freight.cpp') `
  (Join-Path $here 'src\logistics_ui.cpp') (Join-Path $here 'src\route_data.cpp') `
  (Join-Path $here 'src\util.cpp') (Join-Path $here 'src\log.cpp') '-ladvapi32' '-lshell32' '-lole32' '-luuid'
if ($LASTEXITCODE -ne 0) { throw "logistics session tests build failed" }
& (Join-Path $testBuild 'logistics_session_tests.exe')
if ($LASTEXITCODE -ne 0) { throw "logistics session tests failed" }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static -DUNICODE -D_UNICODE -DNOMINMAX "-I$here\src" `
  -o (Join-Path $testBuild 'native_trade_view_tests.exe') (Join-Path $here 'test\native_trade_view_tests.cpp') `
  (Join-Path $here 'src\native_trade_view.cpp') (Join-Path $here 'src\logistics_session.cpp') `
  (Join-Path $here 'src\freight_worker.cpp') (Join-Path $here 'src\freight_engine.cpp') `
  (Join-Path $here 'src\freight.cpp') (Join-Path $here 'src\logistics_ui.cpp') `
  (Join-Path $here 'src\route_data.cpp') (Join-Path $here 'src\util.cpp') (Join-Path $here 'src\log.cpp') `
  '-ladvapi32' '-lshell32' '-lole32' '-luuid'
if ($LASTEXITCODE -ne 0) { throw 'native trade view tests build failed' }
& (Join-Path $testBuild 'native_trade_view_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'native trade view tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static -DUNICODE -D_UNICODE -DNOMINMAX "-I$here\src" `
  -o (Join-Path $testBuild 'dev_snapshot_tests.exe') (Join-Path $here 'test\dev_snapshot_tests.cpp') `
  (Join-Path $here 'src\dev_snapshot.cpp') (Join-Path $here 'src\logistics_session.cpp') `
  (Join-Path $here 'src\freight_worker.cpp') (Join-Path $here 'src\freight_engine.cpp') (Join-Path $here 'src\native_trade_view.cpp') `
  (Join-Path $here 'src\freight.cpp') (Join-Path $here 'src\logistics_ui.cpp') `
  (Join-Path $here 'src\route_data.cpp') (Join-Path $here 'src\util.cpp') (Join-Path $here 'src\log.cpp') `
  '-ladvapi32' '-lshell32' '-lole32' '-luuid'
if ($LASTEXITCODE -ne 0) { throw "dev snapshot tests build failed" }
& (Join-Path $testBuild 'dev_snapshot_tests.exe')
if ($LASTEXITCODE -ne 0) { throw "dev snapshot tests failed" }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static -DUNICODE -D_UNICODE -DNOMINMAX "-I$here\src" `
  -o (Join-Path $testBuild 'freight_scale_tests.exe') (Join-Path $here 'test\freight_scale_tests.cpp') `
  (Join-Path $here 'src\dev_snapshot.cpp') (Join-Path $here 'src\freight_engine.cpp') (Join-Path $here 'src\native_trade_view.cpp') `
  (Join-Path $here 'src\freight.cpp') (Join-Path $here 'src\route_data.cpp') `
  (Join-Path $here 'src\util.cpp') (Join-Path $here 'src\log.cpp') `
  '-ladvapi32' '-lshell32' '-lole32' '-luuid'
if ($LASTEXITCODE -ne 0) { throw "freight scale tests build failed" }
& (Join-Path $testBuild 'freight_scale_tests.exe')
if ($LASTEXITCODE -ne 0) { throw "freight scale tests failed" }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static -DNOMINMAX "-I$here\src" `
  -o (Join-Path $testBuild 'ribbon_cache_tests.exe') (Join-Path $here 'test\ribbon_cache_tests.cpp')
if ($LASTEXITCODE -ne 0) { throw 'ribbon cache tests build failed' }
& (Join-Path $testBuild 'ribbon_cache_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'ribbon cache tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static -DUNICODE -D_UNICODE -DNOMINMAX `
  "-I$here\src" -o (Join-Path $testBuild 'preview_worker_tests.exe') `
  (Join-Path $here 'test\preview_worker_tests.cpp') (Join-Path $here 'src\preview_worker.cpp') `
  (Join-Path $here 'src\route_data.cpp') (Join-Path $here 'src\util.cpp') (Join-Path $here 'src\log.cpp') `
  '-ladvapi32' '-lshell32' '-lole32' '-luuid'
if ($LASTEXITCODE -ne 0) { throw 'preview worker tests build failed' }
& (Join-Path $testBuild 'preview_worker_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'preview worker tests failed' }
& (Join-Path $tools 'g++.exe') -std=c++20 -O2 -static -DUNICODE -D_UNICODE -DNOMINMAX `
  "-I$here\src" "-I$here\third_party\imgui" -o (Join-Path $testBuild 'logistics_ui_tests.exe') `
  (Join-Path $here 'test\logistics_ui_tests.cpp') (Join-Path $here 'src\logistics_ui.cpp') `
  (Join-Path $here 'src\logistics_panel.cpp') (Join-Path $here 'src\freight.cpp') `
  (Join-Path $here 'src\route_data.cpp') (Join-Path $here 'src\util.cpp') (Join-Path $here 'src\log.cpp') `
  (Join-Path $here 'third_party\imgui\imgui.cpp') (Join-Path $here 'third_party\imgui\imgui_draw.cpp') `
  (Join-Path $here 'third_party\imgui\imgui_tables.cpp') (Join-Path $here 'third_party\imgui\imgui_widgets.cpp') `
  '-ladvapi32' '-lshell32' '-lole32' '-luuid' '-ldwmapi'
if ($LASTEXITCODE -ne 0) { throw 'logistics UI tests build failed' }
& (Join-Path $testBuild 'logistics_ui_tests.exe')
if ($LASTEXITCODE -ne 0) { throw 'logistics UI tests failed' }

& $gpuPython (Join-Path $root "tools\test_flow_gpu.py")
if ($LASTEXITCODE -ne 0) { throw 'flow shader GPU tests failed' }

& python (Join-Path $root "tools\test_freight_loader.py")
if ($LASTEXITCODE -ne 0) { throw "freight loader tests failed" }

& python (Join-Path $root "tools\test_freight_deploy.py")
if ($LASTEXITCODE -ne 0) { throw "freight deployment tests failed" }

if (Test-Path -LiteralPath (Join-Path $root "dist\freight\freight.dll")) {
  & python (Join-Path $root "tools\test_freight_loader.py") --runtime
  if ($LASTEXITCODE -ne 0) { throw "freight runtime directory tests failed" }
  & powershell -ExecutionPolicy Bypass -File (Join-Path $root 'tools/test_native_gui_runtime.ps1')
  if ($LASTEXITCODE -ne 0) { throw 'native GUI runtime/archive tests failed' }
  & python (Join-Path $root 'tools/test_native_gui.py')
  if ($LASTEXITCODE -ne 0) { throw 'native GUI bindings/layout/legend tests failed' }
  & powershell -ExecutionPolicy Bypass -File (Join-Path $root 'tools/test_frame_timing_runtime.ps1')
  if ($LASTEXITCODE -ne 0) { throw 'frame timing runtime tests failed' }
}
