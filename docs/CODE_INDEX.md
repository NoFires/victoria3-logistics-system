# 完整代码与文件索引

整理日期：2026-10-24。目录不迁移，文件按实际路径列出；不列构建产物、日志、缓存。

## 模块导航

| 模块 | 主要文件 |
|---|---|
| 启动/安装/资源 | proxy、dllmain、embedded_assets；build.ps1、deploy.ps1 |
| 游戏适配/渲染 | game、offsets、arrow_*、ribbon_cache、flow_width |
| 地理/路径 | route_data、route_geography、state_region |
| 经济输入/身份 | native_economy、native_goods_filter、goods_catalog/snapshot、state_market、native_rail |
| 快照/线程 | native_access、native_snapshot、latest_worker、freight_worker、preview_worker |
| 货运模型 | freight、freight_engine、freight_ribbons |
| 原版界面/会话 | native_gui、logistics_ui/panel/session、market_panel、state_panel、native_*_owner |
| 调试/基础设施 | overlay、dev_snapshot、variable_probe、log、util、mutex |
| 当前VFS研究 | native_vfs、live_vfs_probe（尚未接完整地图资源解析） |

## 全部文件

### root（17 文件）

- [.gitignore](../.gitignore)
- [00_START_HERE.md](../00_START_HERE.md)
- [01_IMPLEMENTATION_PROMPT.md](../01_IMPLEMENTATION_PROMPT.md)
- [02_NATIVE_DISCOVERY.md](../02_NATIVE_DISCOVERY.md)
- [03_TESTING_AND_TROUBLESHOOTING.md](../03_TESTING_AND_TROUBLESHOOTING.md)
- [AGENTS.md](../AGENTS.md)
- [COMPLETION_AUDIT.md](../COMPLETION_AUDIT.md)
- [FREIGHT_MODEL.md](../FREIGHT_MODEL.md)
- [IMPLEMENTATION_REPORT.md](../IMPLEMENTATION_REPORT.md)
- [MASTER_PLAN.md](../MASTER_PLAN.md)
- [NATIVE_GUI_PLAN.md](../NATIVE_GUI_PLAN.md)
- [PACKAGE_MANIFEST.txt](../PACKAGE_MANIFEST.txt)
- [README.md](../README.md)
- [TEST_PROCEDURE.md](../TEST_PROCEDURE.md)
- [VIDEO_REFERENCE.md](../VIDEO_REFERENCE.md)
- [deploy.ps1](../deploy.ps1)
- [requirements.txt](../requirements.txt)

### native/src（60 文件）

- [native/src/arrow_geometry.h](../native/src/arrow_geometry.h)
- [native/src/arrow_lease.h](../native/src/arrow_lease.h)
- [native/src/arrow_scope.h](../native/src/arrow_scope.h)
- [native/src/dev_snapshot.cpp](../native/src/dev_snapshot.cpp)
- [native/src/dev_snapshot.h](../native/src/dev_snapshot.h)
- [native/src/dllmain.cpp](../native/src/dllmain.cpp)
- [native/src/embedded_assets.cpp](../native/src/embedded_assets.cpp)
- [native/src/embedded_assets.h](../native/src/embedded_assets.h)
- [native/src/flow_width.h](../native/src/flow_width.h)
- [native/src/freight.cpp](../native/src/freight.cpp)
- [native/src/freight.h](../native/src/freight.h)
- [native/src/freight_engine.cpp](../native/src/freight_engine.cpp)
- [native/src/freight_engine.h](../native/src/freight_engine.h)
- [native/src/freight_ribbons.h](../native/src/freight_ribbons.h)
- [native/src/freight_worker.cpp](../native/src/freight_worker.cpp)
- [native/src/freight_worker.h](../native/src/freight_worker.h)
- [native/src/game.cpp](../native/src/game.cpp)
- [native/src/game.h](../native/src/game.h)
- [native/src/goods_catalog.h](../native/src/goods_catalog.h)
- [native/src/goods_snapshot.h](../native/src/goods_snapshot.h)
- [native/src/latest_worker.h](../native/src/latest_worker.h)
- [native/src/log.cpp](../native/src/log.cpp)
- [native/src/log.h](../native/src/log.h)
- [native/src/logistics_panel.cpp](../native/src/logistics_panel.cpp)
- [native/src/logistics_panel.h](../native/src/logistics_panel.h)
- [native/src/logistics_session.cpp](../native/src/logistics_session.cpp)
- [native/src/logistics_session.h](../native/src/logistics_session.h)
- [native/src/logistics_ui.cpp](../native/src/logistics_ui.cpp)
- [native/src/logistics_ui.h](../native/src/logistics_ui.h)
- [native/src/market_panel.h](../native/src/market_panel.h)
- [native/src/mutex.h](../native/src/mutex.h)
- [native/src/native_access.h](../native/src/native_access.h)
- [native/src/native_economy.h](../native/src/native_economy.h)
- [native/src/native_goods_filter.h](../native/src/native_goods_filter.h)
- [native/src/native_gui.cpp](../native/src/native_gui.cpp)
- [native/src/native_gui.h](../native/src/native_gui.h)
- [native/src/native_market_owner.h](../native/src/native_market_owner.h)
- [native/src/native_rail.h](../native/src/native_rail.h)
- [native/src/native_snapshot.h](../native/src/native_snapshot.h)
- [native/src/native_state_owner.h](../native/src/native_state_owner.h)
- [native/src/native_vfs.h](../native/src/native_vfs.h)
- [native/src/offsets.h](../native/src/offsets.h)
- [native/src/overlay.cpp](../native/src/overlay.cpp)
- [native/src/overlay.h](../native/src/overlay.h)
- [native/src/preview_worker.cpp](../native/src/preview_worker.cpp)
- [native/src/preview_worker.h](../native/src/preview_worker.h)
- [native/src/proxy.cpp](../native/src/proxy.cpp)
- [native/src/publish_key.h](../native/src/publish_key.h)
- [native/src/ribbon_cache.h](../native/src/ribbon_cache.h)
- [native/src/route_data.cpp](../native/src/route_data.cpp)
- [native/src/route_data.h](../native/src/route_data.h)
- [native/src/route_geography.h](../native/src/route_geography.h)
- [native/src/state_market.h](../native/src/state_market.h)
- [native/src/state_panel.h](../native/src/state_panel.h)
- [native/src/state_region.h](../native/src/state_region.h)
- [native/src/stored_zip.h](../native/src/stored_zip.h)
- [native/src/util.cpp](../native/src/util.cpp)
- [native/src/util.h](../native/src/util.h)
- [native/src/variable_probe.cpp](../native/src/variable_probe.cpp)
- [native/src/variable_probe.h](../native/src/variable_probe.h)

### native/test（27 文件）

- [native/test/arrow_geometry_tests.cpp](../native/test/arrow_geometry_tests.cpp)
- [native/test/arrow_lease_tests.cpp](../native/test/arrow_lease_tests.cpp)
- [native/test/arrow_scope_tests.cpp](../native/test/arrow_scope_tests.cpp)
- [native/test/dev_snapshot_tests.cpp](../native/test/dev_snapshot_tests.cpp)
- [native/test/flow_shader_tests.cpp](../native/test/flow_shader_tests.cpp)
- [native/test/freight_engine_tests.cpp](../native/test/freight_engine_tests.cpp)
- [native/test/freight_scale_tests.cpp](../native/test/freight_scale_tests.cpp)
- [native/test/freight_tests.cpp](../native/test/freight_tests.cpp)
- [native/test/goods_catalog_tests.cpp](../native/test/goods_catalog_tests.cpp)
- [native/test/harness.cpp](../native/test/harness.cpp)
- [native/test/live_panel_probe.cpp](../native/test/live_panel_probe.cpp)
- [native/test/live_vfs_probe.cpp](../native/test/live_vfs_probe.cpp)
- [native/test/logistics_session_tests.cpp](../native/test/logistics_session_tests.cpp)
- [native/test/logistics_ui_tests.cpp](../native/test/logistics_ui_tests.cpp)
- [native/test/market_panel_tests.cpp](../native/test/market_panel_tests.cpp)
- [native/test/native_access_tests.cpp](../native/test/native_access_tests.cpp)
- [native/test/native_economy_tests.cpp](../native/test/native_economy_tests.cpp)
- [native/test/native_gui_test.cpp](../native/test/native_gui_test.cpp)
- [native/test/native_rail_tests.cpp](../native/test/native_rail_tests.cpp)
- [native/test/native_snapshot_tests.cpp](../native/test/native_snapshot_tests.cpp)
- [native/test/native_vfs_tests.cpp](../native/test/native_vfs_tests.cpp)
- [native/test/preview_worker_tests.cpp](../native/test/preview_worker_tests.cpp)
- [native/test/publish_key_tests.cpp](../native/test/publish_key_tests.cpp)
- [native/test/ribbon_cache_tests.cpp](../native/test/ribbon_cache_tests.cpp)
- [native/test/spline_tests.cpp](../native/test/spline_tests.cpp)
- [native/test/state_market_tests.cpp](../native/test/state_market_tests.cpp)
- [native/test/state_panel_tests.cpp](../native/test/state_panel_tests.cpp)

### native/assets（2 文件）

- [native/assets/building_flow.shader](../native/assets/building_flow.shader)
- [native/assets/ironroad_logistics.gui](../native/assets/ironroad_logistics.gui)

### native/third_party（29 文件）

- [native/third_party/imgui/backends/imgui_impl_dx11.cpp](../native/third_party/imgui/backends/imgui_impl_dx11.cpp)
- [native/third_party/imgui/backends/imgui_impl_dx11.h](../native/third_party/imgui/backends/imgui_impl_dx11.h)
- [native/third_party/imgui/backends/imgui_impl_win32.cpp](../native/third_party/imgui/backends/imgui_impl_win32.cpp)
- [native/third_party/imgui/backends/imgui_impl_win32.h](../native/third_party/imgui/backends/imgui_impl_win32.h)
- [native/third_party/imgui/imconfig.h](../native/third_party/imgui/imconfig.h)
- [native/third_party/imgui/imgui.cpp](../native/third_party/imgui/imgui.cpp)
- [native/third_party/imgui/imgui.h](../native/third_party/imgui/imgui.h)
- [native/third_party/imgui/imgui_demo.cpp](../native/third_party/imgui/imgui_demo.cpp)
- [native/third_party/imgui/imgui_draw.cpp](../native/third_party/imgui/imgui_draw.cpp)
- [native/third_party/imgui/imgui_internal.h](../native/third_party/imgui/imgui_internal.h)
- [native/third_party/imgui/imgui_tables.cpp](../native/third_party/imgui/imgui_tables.cpp)
- [native/third_party/imgui/imgui_widgets.cpp](../native/third_party/imgui/imgui_widgets.cpp)
- [native/third_party/imgui/imstb_rectpack.h](../native/third_party/imgui/imstb_rectpack.h)
- [native/third_party/imgui/imstb_textedit.h](../native/third_party/imgui/imstb_textedit.h)
- [native/third_party/imgui/imstb_truetype.h](../native/third_party/imgui/imstb_truetype.h)
- [native/third_party/minhook/MinHook.h](../native/third_party/minhook/MinHook.h)
- [native/third_party/minhook/include/MinHook.h](../native/third_party/minhook/include/MinHook.h)
- [native/third_party/minhook/src/buffer.c](../native/third_party/minhook/src/buffer.c)
- [native/third_party/minhook/src/buffer.h](../native/third_party/minhook/src/buffer.h)
- [native/third_party/minhook/src/hde/hde32.c](../native/third_party/minhook/src/hde/hde32.c)
- [native/third_party/minhook/src/hde/hde32.h](../native/third_party/minhook/src/hde/hde32.h)
- [native/third_party/minhook/src/hde/hde64.c](../native/third_party/minhook/src/hde/hde64.c)
- [native/third_party/minhook/src/hde/hde64.h](../native/third_party/minhook/src/hde/hde64.h)
- [native/third_party/minhook/src/hde/pstdint.h](../native/third_party/minhook/src/hde/pstdint.h)
- [native/third_party/minhook/src/hde/table32.h](../native/third_party/minhook/src/hde/table32.h)
- [native/third_party/minhook/src/hde/table64.h](../native/third_party/minhook/src/hde/table64.h)
- [native/third_party/minhook/src/hook.c](../native/third_party/minhook/src/hook.c)
- [native/third_party/minhook/src/trampoline.c](../native/third_party/minhook/src/trampoline.c)
- [native/third_party/minhook/src/trampoline.h](../native/third_party/minhook/src/trampoline.h)

### native（3 文件）

- [native/PROFILE.md](../native/PROFILE.md)
- [native/build.ps1](../native/build.ps1)
- [native/build_tests.ps1](../native/build_tests.ps1)

### tools（31 文件）

- [tools/Update-ModLoad.ps1](../tools/Update-ModLoad.ps1)
- [tools/analyze_log.py](../tools/analyze_log.py)
- [tools/audit_bundle.py](../tools/audit_bundle.py)
- [tools/calibrate.py](../tools/calibrate.py)
- [tools/discover_local.py](../tools/discover_local.py)
- [tools/gen_positions.py](../tools/gen_positions.py)
- [tools/gen_provinces.py](../tools/gen_provinces.py)
- [tools/gen_splines.py](../tools/gen_splines.py)
- [tools/ironpe.py](../tools/ironpe.py)
- [tools/make_arrow_asset.py](../tools/make_arrow_asset.py)
- [tools/make_arrow_colors.py](../tools/make_arrow_colors.py)
- [tools/native_gui.py](../tools/native_gui.py)
- [tools/package_runtime.py](../tools/package_runtime.py)
- [tools/recovery_core.py](../tools/recovery_core.py)
- [tools/render_model.py](../tools/render_model.py)
- [tools/selftest.ps1](../tools/selftest.ps1)
- [tools/summarize_draw_log.py](../tools/summarize_draw_log.py)
- [tools/test_debug_default.ps1](../tools/test_debug_default.ps1)
- [tools/test_flow_gpu.py](../tools/test_flow_gpu.py)
- [tools/test_flow_shader.py](../tools/test_flow_shader.py)
- [tools/test_freight_deploy.py](../tools/test_freight_deploy.py)
- [tools/test_freight_loader.py](../tools/test_freight_loader.py)
- [tools/test_helpers.py](../tools/test_helpers.py)
- [tools/test_mod_load.py](../tools/test_mod_load.py)
- [tools/test_native_gui.py](../tools/test_native_gui.py)
- [tools/test_native_gui_archive.py](../tools/test_native_gui_archive.py)
- [tools/test_native_gui_runtime.ps1](../tools/test_native_gui_runtime.ps1)
- [tools/test_route.py](../tools/test_route.py)
- [tools/test_splines.py](../tools/test_splines.py)
- [tools/verify_data.py](../tools/verify_data.py)
- [tools/verify_embedded.py](../tools/verify_embedded.py)

### data（5 文件）

- [data/heights.bin](../data/heights.bin)
- [data/positions.txt](../data/positions.txt)
- [data/provinces.bin](../data/provinces.bin)
- [data/splines.bin](../data/splines.bin)
- [data/terrain_names.txt](../data/terrain_names.txt)

### docs（15 文件）

- [docs/CODE_INDEX.md](../docs/CODE_INDEX.md)
- [docs/GITHUB.md](../docs/GITHUB.md)
- [docs/IDEAS.md](../docs/IDEAS.md)
- [docs/transport/ACCEPTANCE.md](../docs/transport/ACCEPTANCE.md)
- [docs/transport/BLOCKERS.md](../docs/transport/BLOCKERS.md)
- [docs/transport/CAPABILITIES.md](../docs/transport/CAPABILITIES.md)
- [docs/transport/DECISIONS.md](../docs/transport/DECISIONS.md)
- [docs/transport/DOCUMENT_COMPARISON.md](../docs/transport/DOCUMENT_COMPARISON.md)
- [docs/transport/history/AGENTS-through-R82.md](../docs/transport/history/AGENTS-through-R82.md)
- [docs/transport/history/MASTER_PLAN-through-R82.md](../docs/transport/history/MASTER_PLAN-through-R82.md)
- [docs/transport/KNOWLEDGE.md](../docs/transport/KNOWLEDGE.md)
- [docs/transport/PERFORMANCE.md](../docs/transport/PERFORMANCE.md)
- [docs/transport/STATUS.md](../docs/transport/STATUS.md)
- [docs/transport/TASKS.md](../docs/transport/TASKS.md)
- [docs/transport/WORKLOG.md](../docs/transport/WORKLOG.md)

