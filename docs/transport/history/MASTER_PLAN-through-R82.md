# MASTER_PLAN 历史记录（截至 R82）

仅为历史证据；第19节为早期基线，第20节为历轮记录，均不代表当前状态。原章节及正文保留。

## 19. 当前代码库、整理记录与恢复入口（2026-10-23）

本节是本轮代码库整理后的事实记录；第 1–18 节仍是完整产品目标。整理没有缩减功能范围，也没有将项目标记为完成。用户已确认的原生 UI 入口必须保留。

### 19.1 目录与产物边界

```text
v3-carrow-imgui-starter/
  MASTER_PLAN.md                 总目标、当前代码地图和下一步
  00_*.md … 03_*.md              保留原始交接材料
  README.md                     构建、安装、调试入口
  COMPLETION_AUDIT.md            需求逐项完成审计
  NATIVE_GUI_PLAN.md             原版 GUI 实现与待接数据桥
  FREIGHT_MODEL.md               模型来源、单位和统计口径
  TEST_PROCEDURE.md              实机验证步骤
  IMPLEMENTATION_REPORT.md       分版本历史证据；旧结论不当作当前状态
  VIDEO_REFERENCE.md             视频中的交互与功能参考
  deploy.ps1                    单 DLL 部署及卸载
  data/                         构建必需的五份派生地图数据
  native/assets/                自有 shader、原生 GUI 片段
  native/src/                   当前原生实现（32 个文件）
  native/test/                  当前回归测试与 harness（10 个文件）
  native/third_party/            保留现有 ImGui、MinHook 依赖
  native/build/                 可再生对象、资源、测试产物；已加入忽略规则
  tools/                        分析、生成、验证及迁移工具（29 个文件）
  dist/version.dll              当前唯一玩家交付产物
```

`data/positions.txt`、`heights.bin`、`provinces.bin`、`terrain_names.txt`、`splines.bin` 参与 DLL 资源打包，不能按“二进制／生成文件”一概删除。玩家不需要手工复制这些数据。`native/assets` 与第三方依赖也不是临时文件。

原始 `PACKAGE_MANIFEST.txt` 仅保留 starter 来源校验记录，不代表当前源码或 DLL 的完整校验清单。`tools/audit_bundle.py` 用于原始“仅文本分享包”的限制检查，不能对当前含原生源码／资源的开发仓库直接套用并据此删除必要文件。

### 19.2 当前模块职责与接线状态

| 模块／文件（相对项目根目录） | 当前职责与边界 |
|---|---|
| `native/src/dllmain.cpp` | version.dll 系统导出转发、loader-lock 外启动、日志与能力初始化；活动 DLL 固定到进程寿命 |
| `game.cpp/.h`、`offsets.h` | 当前本机 EXE 身份／签名门控、CArrow 原生 ABI 与渲染调用；只针对已验证版本 |
| `embedded_assets.cpp/.h` | 从 DLL 准备地图／样式缓存，在引擎早期 MountData 挂载自有样式与 GUI 归档 |
| `native_gui.cpp/.h`、`stored_zip.h` | 校验本机原版市场 GUI，派生补丁，生成确定性 ZIP 与原子缓存；不修改安装目录原文件 |
| `native/assets/ironroad_logistics.gui`、`tools/native_gui.py` | 原版市场第五页签、原版商品行／详情入口、中文／英文自有文本；货运摘要和显示动作的数据桥尚未完成 |
| `route_data.cpp/.h` | 地图数据、原省份路由、真实样条图与有序路径；直线只保留为显式调试模式 |
| `preview_worker.cpp/.h`、`latest_worker.h`、`mutex.h` | 预览路由后台工作、不可变结果交接、稳定 ID、路由缓存、输入合并与代数取消 |
| `freight.cpp/.h` | 观察性货运分配模型、守恒／缺货／市场隔离、商品与区段汇总；尚未接实时游戏输入 |
| `flow_width.h`、`freight_ribbons.h` | 分配数量到固定宽度档、区段到样式／方向请求；不把目标需求当已运数量 |
| `ribbon_cache.h`、`arrow_geometry.h`、`arrow_scope.h` | 精确几何身份、去重、全局原生池、类型兼容重用、上传预算、原生 pass 与端点规则 |
| `overlay.cpp/.h` | D3D11／ImGui 调试、渲染更新与原生提交；当前正式货运 owner 与完整生命周期仍待接线 |
| `logistics_ui.cpp/.h`、`logistics_panel.cpp/.h` | 复制后的展示文档、筛选／悬停／详情状态和 ImGui 调试原型；不是正式原版 UI 的替代品 |
| `log.cpp/.h`、`util.cpp/.h` | 批量日志、文件／路径、编码转换、SHA-256 等公共支持 |
| `tools/package_runtime.py`、`verify_embedded.py` | 构建 10 项 DLL RCDATA 资源并独立检查真实 DLL 内容 |
| `tools/Update-ModLoad.ps1`、`test_mod_load.py` | 旧安装的精确迁移及配置保留测试；部署脚本仍引用，因此保留；不作为新播放集安装方案 |

**实际尚未接通的关键链路**：安全经济读取窗口 → 数值快照 → 模型分配 → 完整货运视图缓存 → 原版 UI 数据／动作 → 正式 owner 的原生提交。目前不能从“已有模型、控件和预览工作线程”推断这条链路已经运行。

### 19.3 本轮删除与保留决策

删除了以下已被替代或可再生内容：

- 根目录一次性地图诊断：`citydbg2.py`、`dbg_city.py`。
- 已被正式回归与 harness 覆盖的旧探针：`native/test/atlas_probe.cpp`、`imgui_probe.cpp`、`route_probe.cpp`。
- 依赖旧 `dist/mod` 播放集包的 `tools/verify_mod.py`；当前资源验证入口为 `tools/verify_embedded.py`。
- 旧分发目录 `dist/mod/`、`dist/ironroad/`、`dist/ironroad_arrow.mod`；其中旧配置已备份，不覆盖当前 LocalAppData 偏好。
- `dist` 中全部旧测试／探针 EXE；当前 DLL 保留。
- 原有 `native/build` 中间文件、旧测试缓存、Python `__pycache__`、`data/route_test.png`。

本轮累计移除 71 个旧文件，约 52.4 MiB。源码／配置删除前均有备份；生成文件记录了路径、大小与 SHA-256。随后验证生成的新测试产物统一放到忽略的 `native/build/tests`，不重新混入 `dist`。

调整了输出入口：`native/build_tests.ps1`、`tools/test_flow_gpu.py`、`tools/test_native_gui_runtime.ps1` 输出到 `native/build/tests`；`selftest.ps1` 和 `test_debug_default.ps1` 使用该处 harness。`tools/test_route.py` 新增 `--out`，默认图片位于 `native/build/diagnostics/route_test.png`，不再污染必需数据目录。新增 `.gitignore`，没有初始化或重置 Git 仓库。

保留原始四文档、完整当前核心源码、真实回归测试、五份地图数据、自有资源、第三方依赖与历史证据文档。没有因“未完成”而删除正式货运模型或未来原版 UI 所需接口。

自动审批拒绝了批量递归删除。最终使用逐文件校验路径与 hash 后删除，再逐个删除已确认为空的目录；没有跨 shell 删除或清理项目外的目录。

### 19.4 当前版本与恢复点

- 游戏 profile：Victoria 3 1.13.11，EXE SHA-256 `7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f`。
- 候选产物：`E:\Code\v3-carrow-imgui-starter\dist\version.dll`。
- 安装位置：`C:\Program Files (x86)\Steam\steamapps\common\Victoria 3\binaries\version.dll`。
- 两者本轮实查 SHA-256 一致：`a99f8d9ecb58c76245d19c8b72bffe5bb333a7bff79dd390459fed21ec3f9c74`；DLL 大小 22,563,873 字节。
- 本轮整理没有改核心 C++ 行为、重新构建 DLL、重新部署或启动游戏；只重新编译／运行相关开发测试。
- 整理前有限备份：`E:\Code\v3-analysis\checkpoints\20261023-repository-cleanup-before`，包含被清理的自有源码、旧分发配置／包、当前 DLL、待修改脚本／文档及清单；第三方依赖和五份主数据保留在原位。
- `files-before.json` 为整理前非中间产物校验清单；`generated-and-legacy-deletions.json` 记录生成／旧包删除项；原启动日志另存 `ironroad-before-tests.log`。不要盲目恢复整个旧 `dist`，只在需要时按清单取回具体文件。
- 原先已确认正确箭头方向的历史回滚点仍在 `E:\Code\v3-analysis\checkpoints\20261022-pure-dll-direction-verified`，不得用新候选覆盖。

### 19.5 整理后的验证与开发命令

本轮已通过：原生几何、样条路由、pass scope、货运分配、RibbonCache、后台预览、复制展示／ImGui 原型测试；真实 C++ GUI 归档生成与独立 ZIP／CRC 校验；DLL 内嵌资源；GUI 补丁；Python helper、数据、样条解码及旧配置迁移测试；八档宽度、正反方向和九种颜色的真实离屏 GPU 回归（含旧半宽采样反例）。这些是对应边界的验证，不是全产品实机验收。

新路径下的独立 harness 也通过：显式打开调试面板的离屏读回为 541,927 个亮像素；默认关闭测试 PID 53620 为零 ImGui 顶点。测试前保存了原日志。最终文件校验确认核心源码／自有资源／第三方／必需数据、原始四文档和 DLL 均未变化，所有保留 Python 工具语法有效。校验摘要及整理后文件清单保存在本节所列外部备份目录。

```powershell
# 构建唯一玩家交付 DLL
powershell -ExecutionPolicy Bypass -File native\build.ps1

# 构建并运行原生回归；产物在 native/build/tests
powershell -ExecutionPolicy Bypass -File native\build_tests.ps1
powershell -ExecutionPolicy Bypass -File tools\test_native_gui_runtime.ps1
python -B tools\verify_embedded.py
python -B tools\test_native_gui.py
python -B tools\test_flow_gpu.py

# 仅游戏关闭时运行：它们使用独立 harness 并写入诊断日志
powershell -ExecutionPolicy Bypass -File tools\selftest.ps1
powershell -ExecutionPolicy Bypass -File tools\test_debug_default.ps1

# 游戏关闭后部署；本轮整理未执行部署
powershell -ExecutionPolicy Bypass -File deploy.ps1
```

工具链继续使用构建脚本中已配置的 `E:\Program\C Compiler\MSYS2\ucrt64\bin`。删除 `native/build` 后需要完整构建；`-ResourcesOnly` 不会凭空重建缺失对象。离线脚本不等于玩家运行时依赖。

### 19.6 恢复开发时的优先工作

开始下列任一项前，先按第 2.4 节及对应 P 阶段重新完整阅读原始文件并记录；本优先清单和既有代码地图不替代原文。

1. **P4 / A13–A15**：证明安全的实时经济读取窗口；核实稳定身份、市场归属、供需单位及已建铁路；将复制快照接入现有分配器。未证实前保留未知值。
2. **P5 / A05–A07、A18–A19**：市场基础页签／商品列表／原版详情已确认正常；下一项是验证原生数据和动作绑定，接入显示／隐藏、筛选／悬停、返回／关闭及地区视图。不能让正式箭头依赖 ImGui 的可见状态。
3. **P3、P6 / A16–A22**：完成货运视图缓存与 owner／world 代数协议，接入既有线宽／颜色请求，核对固定分母、铁路去重和筛选尺度。
4. **P7 / A23–A26**：同存档实测大量线路、内存平台、模拟开销、读档／主菜单／设备生命周期与回滚。A05–A07 仅部分子路径已有证据，不能直接把整项记为 PASS。

后续原生绑定研究证据位于项目外的 `E:\Code\v3-analysis\native-gui-bindings.txt` 和 `native-gui-register-string.txt`。这些是静态研究，尚未作为新的运行时调用接入；不能仅根据反编译候选安装新 hook。尤其不能把 GUI 参数向量构造函数误认成经济读取 guard。

## 20. 实施进度更新（2026-10-23，R6）

> 本节由编码 AI 在每个可验收工作单元后追加，记录与原计划的偏差、当前产物和下一步；第 1–18 节的产品目标不变。详细状态、验收和证据以 `AGENTS.md` 与 `docs/transport/` 为准。

### 20.1 本轮已完成（离线证据，未实机）

- 货运链已连通到调试面板：分配模型 → `FreightEngine` 分配/视图双层缓存 → `FreightWorker` 后台装配 → `LogisticsSession` 多 owner 仲裁 → 调试面板文档/热度图例 → 共享 `RibbonCache` 分帧上传与原生提交（仅调试态）。
- 热度改为按完整市场快照的固定尺度；`targetImport`/`allocatedImport`/`unmetDemand` 分离，不随过滤变化；未满足不画成已运输。
- R6 修正区域 hub 接入：在 `region*100+0..4` 中选 policy 允许的最高活跃度 hub，避免“城市 hub 只有未建铁路”导致州被误判不可达；48-region 合成分配 8/96 → 32/96，全铁路 649-region 1280/1296。
- 合成测试快照（显式 TEST INPUT）可在真实地图产生 39–1472 条 ribbon；649-region 冷构建 17.6 ms（无铁路）/105 ms（全铁路），热视图 0 ms。
- 回归：`native/build_tests.ps1` 11 项 PASS；生产 DLL 候选 SHA-256 `5bca95e09e2526676dffbff74e528e2e7b4ac1f92e6bd6777fbdbd4643aa5ddc`（27,935,041 B），`tools/verify_embedded.py` PASS。

### 20.2 仍未完成 / 阻塞

- 实时经济读取窗口与适配器（P4 / A13–A15）未建立；`EconomySnapshot` 仍只来自测试/合成输入。
- 原版 UI 数据/动作桥（P5）未接；货运摘要、显示/隐藏、hover、地区视图、返回/关闭仍缺。
- 正式货运 owner 未接原版面板；货运箭头与多 owner 切换未实机验证。
- 已部署 DLL 仍是旧版 `a99f8d9e…c3e74`，当前候选未部署。
- A05–A10、A16–A26 的实机项多数 NOT_RUN；回滚演练未按当前候选执行。
- hub 回退是显式模型规则，需真实已建铁路快照复核；合成未满足比例不代表真实市场。

### 20.3 下一步

1. 实机自检：游戏关闭时部署候选，设置 `test_snapshot=1`，在调试面板 UI prototype 观察货运箭头/宽度/颜色/关闭残留（A16–A18、A21）。
2. 原版 UI 最小变量/动作 ABI 验证，再逐步接需求统计、热度说明与显示/隐藏。
3. 经济读取窗口：从 state 容器读取函数的引擎内部调用者反推布局/线程/寿命；未证实不得 hook。
4. 记录维护：每个工作单元更新 `AGENTS.md` 与 `docs/transport/STATUS.md`；每个可验收单元完成后追加本节。

### 20.4 R7 更新：合成输入覆盖宽度档

- 合成测试快照改为“每个商品成对生产/消费 + 八档数量”，使供给/需求平衡并让调试视图实际出现多个原生宽度档：48-region 实测宽度档 0/2/3/4/5/6/7，58 条 ribbon。
- 配对后全铁路可分配比例恢复合理：48-region 99.5%，649-region 96.5%（此前单侧数量不等导致的假失败已修正）。
- 规模更新：649-region 无铁路 25.8 ms/655 ribbons；全铁路 271.8 ms/1801 ribbons；热视图 0 ms、hover ≤0.7 ms。README/性能表已同步。
- 新候选 DLL：`c6f7799570a279110b08d54d62b5ce3bbea845b32e05c5737e29f8f6b19d7b70`（27,962,872 B）；仍未部署、未实机。
- 下一步不变：实机自检（`test_snapshot=1` 观察宽度/颜色/关闭）、真实已建铁路数据复核、原版 UI 数据桥、经济读取窗口研究。

### 20.5 R8 更新：首次实机验证（调试货运视图）

- 部署候选 `c6f7799570a279110b08d54d62b5ce3bbea845b32e05c5737e29f8f6b19d7b70` 并启动游戏；启动日志确认 loader、profile hash、GUI 归档挂载、嵌入资源、native hooks、ImGui ready，且 `synthetic freight test snapshot restored`。
- 在调试面板 UI prototype 页，实机看到：`freight view gen=2 … ribbons=58`、多种 `ironroadTraffic*W*` 宽度类型解析、持续 `SUBMIT scope=0/1/0`（结束前 153k 次提交、71 次上传、605 cache hits），地图上出现多宽度彩色 freight ribbon；文档显示 TEST INPUT、Freight 16.15M、Target/Allocated/Unmet、热度参考。
- 原版市场“物流”页签仍显示实时数据未接通与 `--`，符合“调试输入不进正式 UI”的约束。
- 未完成：正式原版 UI 数据/动作桥、实时经济快照、真实已建铁路复核、完整 A05–A26 实机矩阵；本次是调试合成输入的可见性验证，不能替代 A13–A15/A18–A19/A25。

### 20.6 R9 更新：设置路径修复与迁移

- R8 实机发现设置文件被忽略：`RuntimeDataDirectory()` 返回 `%LOCALAPPDATA%\IronroadV3\data`，而文档说设置在根目录。
- R9 修正：规范设置文件改为 `%LOCALAPPDATA%\IronroadV3\ironroad.ini`；读取先根后 data 回退；写入只写根并建目录；首次运行从 `data\ironroad.ini` 或旧 `binaries\ironroad\ironroad.ini` 迁移且不覆盖已有根文件。
- 实机验证：部署候选 `bef613d4907bddc570517a18219327a7302560c2c4772978d3ffddf3fe6b34bf`（28,123,676 B），删除根 ini 后启动；根 ini 自动生成、设置正确读取、合成快照恢复、GUI/ImGui 正常、无崩溃；测试后关闭并恢复 `test_snapshot=0`。
- 仍缺：原生 UI 数据/动作桥、实时经济快照与完整验收；变量系统只完成静态线索（`GetVariableSystem` 单例/注册包装），尚未调用。

### 20.7 R10 更新：GUI 变量系统静态解剖

- 只读分析确认 `GetVariableSystem` 返回一个非多态的 **string→string 哈希表**单例（0x28 字节头部、0x48 字节条目：hash/占用/MsvcString 键/MsvcString 值），并定位构造、重哈希与 insert-or-find（已存在时不更新值）。
- 尚未定位：条目 hash 算法、更新已有值的 setter、GUI `.Exists/.Toggle/.Get/.Set` 与该对象的映射、线程亲和性。**未在 DLL 调用任何变量系统函数。**
- 下一安全步骤：门控的 getter-only 运行时探测，读取现有 `ironroad_hide_flows` 条目验证布局并反推 hash；只有证明后才考虑短 SSO 值的写入实验。
- 证据：`E:\Code\v3-analysis\var-system-r10-notes.txt`；决策见 `docs/transport/DECISIONS.md` D17。

### 20.8 R11 更新：GUI 变量系统 getter-only 探针

- 新增 `native/src/variable_probe.{h,cpp}` 与 `probe_variables` 调试设置：签名门控、VirtualQuery 校验、只读遍历。getter RVA 与 8 字节签名并入 profile gate。
- 实机验证：对象地址 = base+0x59FD800；头部 `buckets=1 count=0 level=0 load=0.90 entries=0`，与静态解剖一致；无崩溃；测试后关闭游戏、探针恢复 0。
- 未取得条目：需要先由原生 GUI 创建 `ironroad_hide_flows`（打开市场“物流”页签并点击显示/隐藏）。本轮自动化未能可靠打开该页签，因此 0x48 条目布局/hash/setter/线程仍未验证；**未写入任何变量**。
- 候选/已部署 DLL：`b171d15d9cabb8a532e71f471b069bf060b642792cb679aebc6e1304346120a7`（28,382,723 B）。

### 20.9 R12 更新：条目布局验证、Murmur3 与 GUI 变量写入打通

- 只读探针读取同模板全局 map（5639 条），验证 0x48 条目布局；用 40 条真实 key/hash 确认哈希为 **MurmurHash3 x86_32 seed=0**（40/40）。
- 引擎 `std::string::assign`（0x61F280）+ insert-or-find（0x79C0D0）已接入，任意长度键/值可插入/更新；实机 `ir_probe`、`ironroad_probe_long_key` 均成功，读回 `long-value-2`，无崩溃。
- 限制：原版 GUI 变量 API 只有 `Exists/Toggle/Set`，**没有 `Get` 文本读取**；变量只能作标志。货运数值/文本仍需注册 GUI 数据函数（返回类型 ABI 未验证）。
- 写入仅经 `probe_write` 调试开关；正式 UI 使用前需验证线程亲和性、生命周期与回滚（D18）。
- 候选/已部署：`05f54995d632fa1b458acf3b4f6a05c1bfbfe6c806b13954e784f234f3e9c43e`（28,448,007 B）；`build_tests` 11 项 PASS；证据 `native/build/r12-*.log`。

### 20.10 R13 更新：原版市场面板自动化与“物流”页签确认

- 找到 `open_market` = F4（`game/input_profile/default.profile`）和左侧 sidebar 贸易按钮；自动化打开市场面板成功。
- 实机截图确认第五页签 **“物流”** 与 `商品/贸易/粮食安全/成员` 并列存在，页签内容含“显示/隐藏”按钮（约 physical 590,230）。
- 曾加临时 GUI debug 文本尝试验证 `GetVariableSystem.Get`，因面板关闭/点击坐标时序未切到页签内容，未得结论；最终构建前已移除，正式 GUI 保持干净。
- 记录：`probe_write=1` 需同时 `probe_variables=1` 才执行；F4 的 PostMessage 未触发，下轮用 SendInput。
- 候选/已部署：`a59fdbfc26e23837b5db8acd5537fa41a4b5c216ea0e5aebbf64a15cee035977`（28,448,007 B）；`build_tests` 11 项 PASS；证据 `native/build/r13-*`。

### 20.11 R14 更新：自动进图与 F4 打开市场；页签点击待校准

- 验证启动参数 **`-continuelastsave`**：直接加载最近存档进入地图，绕过“载入游戏”对话框。
- 验证 **F4**：`keybd_event(VK_F4=0x73, scancode 0x3E)` 可打开市场面板；`PostMessage` 无效。`pdx_settings.json` 记录 `GUI.scale=1.1`。
- 第五页签“物流”点击仍未命中（(585,232) 落到地图、(532,211) 展开过滤器）；临时 debug 文本已移除，正式 GUI 干净；未做 `.Get` 结论。
- 候选/已部署：`e728cd5e6d13e5acf8d3c460e986df3bf79d120a5450efd283b7947ebbfd0e6b`（28,448,007 B）；`build_tests` 11 项 PASS；证据 `native/build/r14-*`。

### 20.12 R15 更新：`.Get` 验证成功；地图期渲染线程写入不安全

- 临时标题调试文本实机读回 `VAR_EXISTS=1 VAR_GET=long-value…`：**`GetVariableSystem.Get('key')` 存在并返回写入的字符串**。正式文本可用“C++ 写变量 + GUI `.Get`”实现，无需注册 GUI 数据函数。
- 尝试 `ir_status/ir_freight/...` 文本桥：在地图加载期由 Present 线程写入失败（`entry not found after insert`）并导致进程退出；判定为与 UI/GUI 初始化的竞争。
- 回退：`native_variables` 默认 0、GUI 恢复 pending、移除标题调试；文本桥代码保留但禁用，等待 UI 线程安全窗口。
- 候选/已部署：`3ffd86e7b10a51a292972660f9a69f689dc164aa28f4eefd38788d2475eb244a`（28,457,628 B）；`build_tests` 11 项 PASS；证据 `native/build/r15-*`。

### 20.13 R16 更新：写入线程边界隔离与回退

- 隔离：`.Get` 读缺失 key 不崩溃；`native_variables=0` 时地图正常加载；单个后载写入不崩溃但失败（`entry not found after insert`）；批量加载期写入会崩溃。
- 结论：Present 线程不是 GUI 变量表的合法写者；变量表在加载/首次 UI 使用时由 UI 线程占用。需要 hook UI 线程窗口（市场面板打开/更新）再写。
- 回退：移除发布全部 key 的调用；单 key 后载探针仅 `native_variables=1` 时一次性尝试；标题调试移除；默认全部关闭。
- 候选/已部署：`c87c5ae6119bb7b5497dd12582cfe2e646c50d7dfe1f40b843f549dcd948086e`（28,460,700 B）；`build_tests` 11 项 PASS；证据 `native/build/r16-*`。

### 20.14 R17 更新：UI 线程写窗口与后载 pair 布局问题

- 复用窗口子类：`WM_APP+0x51` → `OnUiThreadPublish()`，UI 线程执行变量写入；渲染线程只入队并 PostMessage。
- 后载写入不再崩溃，但 entry 只有 hash 匹配，key 字节为垃圾、value 为空，`.Get` 返回空。问题从线程转为**后载变量表 pair/所有权布局不匹配**。
- 已回退默认关闭（`native_variables=0`），GUI pending；需 dump 主菜单 vs 后载 entry 的 0x48 字节或改用引擎自己的 Set。
- 候选/已部署：`2bccef97b95aafaaa8d9cc37690e015da4545c601fab1c6ec99dd30939928d96`（28,466,083 B）；`build_tests` 11 项 PASS；证据 `native/build/r17-*`。

### 20.15 R18 更新：后载变量文本桥打通

- 根因是 SSO 短字符串在后载 insert/move 路径损坏；改用 **长 key（>=16）+ 值补齐到 >=16**（堆字符串）后，UI 线程写入的 7 个 `ironroad_*_v1` 变量全部正确，探针读回 status/freight 等。
- 标题 `.Get` 实机显示 `STATUS=waiting for economy data FREIGHT=--`，端到端确认；正式页签已改用 `.Get` 绑定，`native_variables` 默认开启；合成输入显示 waiting/`--`。
- 未完成：页签内容的实机可视化（点击/布局）；真实（非合成）货运文档接入。
- 候选/已部署：`387c7924df718ad042a77eaeffa4fb1571ca95e112ef78e9feb85931bd7eaa71`（28,471,070 B）；`build_tests` 11 项 PASS；证据 `native/build/r18-*`。

### 20.16 R19 更新：页签内容容器修复与 `.Get` 文本可见

- `ironroad_market_logistics_content` 必须插入 `market_panel_details_content` **块内**；块外同级元素不会显示。
- 临时默认页签可见性下实机显示“物流”、`waiting for economy data`、`Freight: --`、`Track: --` 与按钮（`r19-inside.png`）。生产可见性恢复为仅第五页签；点击自动化仍未命中。
- 候选/已部署：`a8a83cc94fdb48a22a5f68659194d44e59bfb16c3c1ef0ca079b8dfabaa2015e`（28,472,606 B）；`build_tests` 11 项 PASS；证据 `native/build/r19-*`。

### 20.17 R20 更新：标题点击切页实验失败

- 临时在标题 `raw_text` 后注入 `onclick=SelectTab('ironroad_logistics')`；实机点击标题（~420,165）后实际选中“成员”页签，不是物流。
- 实验已回退；正式构建保持 R19 的块内内容 + 仅第五页签可见。
- 候选/已部署：`9d5b1dee32e10a820bd5d3c38000c1bbfd53eecae0312a52f3588734fd54fdbe`（28,472,606 B）；`build_tests` 11 项 PASS；证据 `native/build/r20-*`。

### 20.18 R21 更新：物流页签内容与 `.Get` 最终实机确认

- 类型实例级 `visible` 不生效；改为 **container 包裹** 并设 `visible=IsTabSelected('ironroad_logistics')`，默认页签隐藏、物流页签显示。内容必须与 `market_panel_details_content` 同级。
- 页签提示 **Tab 5 / SHIFT+5**；`Shift+5` 后物流页签高亮，内容显示 `waiting for economy data`、`-- / -- / --`、`Freight: --`、`Track: --`、显示/隐藏按钮与货物列表。
- 候选/已部署：`336208e29500851c3eb845ab7ddaabe81b90bd3e6e3c71c473160032dea22b80`（28,472,094 B）；`build_tests` 11 项 PASS；证据 `native/build/r21-*`。

### 20.19 R22 更新：物流页签隐藏商品过滤器/搜索栏

- 为 FILTERS 按钮、搜索栏与过滤器容器增加 `Not(IsTabSelected('ironroad_logistics'))`；仅在物流页签隐藏。
- 实机 F4+Shift+5：物流页签显示干净内容（waiting/`--`/Freight/Track/按钮/货物列表），FILTERS 与搜索栏消失（`r22-clean.png`）。
- 余留：Target/按钮行轻微重叠；真实数据仍缺。
- 候选/已部署：`5a0ed5faffd7a09bb76f8d1424b41c5a10ccfe8a0db042af898fa836cf213252`（28,498,718 B）；`build_tests` 11 项 PASS；证据 `native/build/r22-*`。

### 20.20 R23 更新：物流页签布局微调

- 重写页签顶部布局：状态/Freight/Target 标签/值行/按钮/Built rail 各自独立行，显式 `size` 与 `multiline`；显示/隐藏按钮单独 flowcontainer。
- 实机 F4+Shift+5 无重叠：waiting/`--`、按钮、`Built rail: --`、货物列表（`r23-layout.png`）。
- 候选/已部署：`8f1a686cea6889eb9f84ffc03c5a3fbebe0935f4c3f3cc28e3910b9e070e780f`（28,498,718 B）；`build_tests` 11 项 PASS；证据 `native/build/r23-*`。

### 20.22 R25 更新：程序化验证显示/隐藏按钮状态

- 新增调试设置 `test_hide_variable`（默认 0，ini-only）：UI 线程写入一次 `ironroad_hide_flows`（堆字符串）。
- 实机对比：无变量时 `显示` disabled、`隐藏` enabled；写入后 `显示` enabled、`隐藏` disabled（`r25-baseline-row.png` / `r25-hideflows-row.png`），确认 `Exists/Toggle` 绑定正确。
- 候选/已部署：`43db0537e1df90a4b9ecadb7debde8fdd5e2477f6962f07636adf9f3f9d2b855`（28,505,886 B）；`build_tests` 11 项 PASS；证据 `native/build/r25-*`。

### 20.23 R26 更新：`ironroad_hide_flows` 驱动调试货运视图

- UI 线程每 500ms 只读轮询 `ironroad_hide_flows`，更新 `m_hideFlows` 并记录状态变化；`ApplyPendingFreight` 在隐藏时停用/隐藏调试货运 ribbon（纯视图）。
- 实机 `test_hide_variable=1`：`TEST ironroad_hide_flows written` → `native panel hide flows=1`，确认轮询/标志生效；未做 prototype 视图肉眼消失验证。
- 候选/已部署：`c544f1e910fbc94117764c18fb5511682f9329a8e9cff134b98dae5cc9a38831`（28,513,281 B）；`build_tests` 11 项 PASS；证据 `native/build/r26-*`。

### 20.24 R27 更新：GetStateProduction 脚本回调观测

- 新增 `probe_economy`（默认 0）：MinHook `0x019791F0`，仅记录 `ctx/result/args` 与 ctx 字段，随后调用原函数；不取值、不构造上下文。
- 实机：州面板“本地价格”页签逐商品触发 `GetStateProduction`，捕获多组 `ctx/result/args`；经济桥获得可观测入口。
- 下一步：dump ctx/args，定位商品 id 与数值，设计只读快照采集。
- 候选/已部署：`9b8a8b31ff14c339141c53c68d667dab7332ba5ae6f448ce7ff49c3ab0ea6726`（28,518,598 B）；`build_tests` 11 项 PASS；证据 `native/build/r27-*`。

### 20.25 R28 更新：经济回调 ctx/args/result 结构

- 通过（商品面板→本地价格）稳定触发 GetStateProduction；`ctx+0x18` 低 32 位为州 id，args count=7/元素 0x20，result 为包装值容器。
- 商品 id 与定点数值尚未解码；下一步 hook reader `0x0195BA00` 或检查其 out 容器。
- 候选/已部署：`098293db77f3e9cca9ad18e2bfde54f23ac111396ef1876e6ef007224f6d99b1`（28,521,158 B）；`build_tests` 11 项 PASS；证据 `native/build/r28-*`。

### 20.26 R29 更新：生产读取器/容器读取器钩子

- 新增生产读取器 `0x0195BA00`、keyed `0x0103F3C0`、indexed `0x0103F440` 观测钩子；读取器 detour 原样返回原 rax。
- 实机商品“本地价格”走 indexed 路径（同一 out、多个 arg、步长 0x70）；out 为条目向量头部，数值/商品 id 待解析；keyed 未触发。
- 候选/已部署：`450abd873cbdca0716a574ff5010c5b8e0f210161f1113faa0b690e9651b5b2f`（28,534,558 B）；`build_tests` 11 项 PASS；证据 `native/build/r29-*`。

### 20.27 R30 更新：州经济容器布局静态解码

- Ghidra headless 反编译确认：indexed 值=`array[goodId]`（array@container+8，位图@+0x20，goodId=arg+0x10）；keyed 用 arg+0xC0 键与 +0x30/步长 0x20 表；读取器容器=state+0xC8（生产）/+0x78（消费），state=registry(ctx+0x18)。
- 读取器钩子加入直接读取实验；实机样本被 tooltip 遮挡未取得 `eco direct`。
- 候选/已部署：`7e32789a92c75c0bcdf4885cb26f93f71d78ca722fa892b6c75f27124e3b6281`（28,540,190 B）；`build_tests` 11 项 PASS；证据 `native/build/r30-*`、`E:\Code\v3-analysis\container-r30.txt`。

### 20.28 R31 更新：实机生产/消费数值与 1e5 单位

- 读取器钩子内按州去重直接读取 `state+0xC8/0x78`；实机 state 432 prod=1.2M、state 436 prod=0.4M/cons=20.85M。
- 100 州钢累计 prod=3,424,477,556、cons=5,942,248,942；对 UI 市场 36.6K/61.2K 比例 ≈9.4e4/9.7e4 → **固定点 1e5**。
- 候选/已部署：`eded5217d23162222fce2a2e53a2cbb2bc2eddde78904e0f0580eaad7bcdcb30`（28,643,288 B）；`build_tests` 11 项 PASS；证据 `native/build/r31-*`。

### 20.29 R32 更新：州枚举验证（reader 路径有效）

- 州对象 id 位于 `state+0x8`；registry 盲枚举（id+vtable 过滤）得到 1117 个“州”但总量 4.5e18，无效。
- reader 路径（商品“本地价格”逐州）给出的州有效：state 205 prod 8 项/296,415、cons 17/240,237；state 201 prod 11 项/8,264,509、cons 38/11,239,733。
- 结论：真实快照应从 reader 观察到的市场州集合构建；下一步缓存该集合并生成快照。
- 候选/已部署：`d90dc2e590d856a7afc09b042d2922344bcc5fde3e0ff58e41ebf99022cacd7f`（28,654,634 B）；`build_tests` 11 项 PASS；证据 `native/build/r32-*`。

### 20.30 R33 更新：真实市场快照采集落盘

- 读取器钩子内采集当前市场州（reader 路径）全部商品生产/消费，去重缓存；每 25 州写 `%LOCALAPPDATA%\IronroadV3\economy_dump.txt`。
- 过滤 `good<128 && value<1e12`（高位索引读到指针垃圾）；100 州总量 prod=24,007,540,245、cons=36,843,839,954（≈240,075/368,438 单位，1e5）。
- 候选/已部署：`474e638c41f971e3d5ab46f68320e77056c5c6e53c31c611cc1d388306d1f6c4`（28,703,482 B）；`build_tests` 11 项 PASS；证据 `native/build/r33-*`。

### 20.31 R34 更新：真实快照接入会话

- 采集数据映射为 `EconomySnapshot` 并以 `synthetic=false` 注入会话；`PublishRealEconomy` 激活原生 owner。
- 实机：`orders=3082 goods=49` 接受；会话 `owner=10 gen=1 frame=1 ribbons=116`，真实数据生成 116 条模型分配 ribbon。
- 正式页签文本仍 waiting（发布/时序待修）。
- 候选/已部署：`bd63bf36916931a098f9d6b2d3d1f590b22562272e5deb83e868b1b3a6d6a733`（28,892,668 B）；`build_tests` 11 项 PASS；证据 `native/build/r34-*`。

### 20.32 R35 更新：正式 UI 显示真实模型分配

- 根因：insert-or-find 不更新已存在 key 的 value；修复为 insert 后对 `entry+0x28` 调用引擎 `assign` 更新值。
- 实机物流页签显示真实数据：`model allocation`、`Freight 23108927.60`、`Target/allocated/unmet 259398.15 / 14746.49 / 244651.66`，无崩溃。
- 真实数据链路（引擎→快照→模型→原版面板）首次完整可见。
- 候选/已部署：`77a3efd064b28d5b2cb27dd23f168a2d53a45f2674644b3811711906f3939b0c`（28,895,740 B）；`build_tests` 11 项 PASS；证据 `native/build/r35-*`。

### 20.33 R36 更新：Freight 口径/标签修正

- `allFreight` 是 Σ shipment.freight（milli-orders×cost），不是订单数；显示改为 `/kQuantityScale`，标签改为 `Freight (order-cost)`。
- 实机页签显示 `model allocation`、`Freight (order-cost): 23108.93`、`Target/allocated/unmet 259398.15 / 14746.49 / 244651.66`。
- 分配率 ≈5.7%，指向 state→region/铁路映射问题。
- 候选/已部署：`75cc6de5ef452761f4897cf223cad79ec5ff11d6554821d3c301aa93f5a50f74`（28,895,740 B）；`build_tests` 11 项 PASS；证据 `native/build/r36-*`。

### 20.34 R37 更新：region 映射确认与跨市场混样

- 运行时州 id 与 `data/positions.txt` 的 region id 一致（201/205/432/436/132/350）；790/940 缺失 → 无 hub。
- 州对象未直接含 `STATE_` key（不需要）；商品“本地价格”列的是全球州，快照全部标 market=1 → 跨市场混样，是低分配率主要嫌疑。
- 候选/已部署：`8feb3b81c2e8f625aac88302cd5259c082d70ccd1ee60cfe21cd8d5d16c56ea1`（28,904,528 B）；`build_tests` 11 项 PASS；证据 `native/build/r37-*`。

### 20.35 R38 更新：州字段扩展转储；页签非市场作用域

- 对 12 个州转储 state 前 0x200 字节；实机仅 6 个全球州被访问（205/201/350/132/432/436），英国本土 16–23 缺失 → 本地价格页签是全球排序子集，非当前市场。
- 州对象 0x00–0x100 未见 market 字段（0x80/0xa8/0xd0 每州数据指针，0xb0 类别计数）；需同市场样本或静态分析。
- 候选/已部署：`ad0a4dc14f10a34ae70e966522ff89fcbf48c69148a546f769accdda48b14fce`（28,920,642 B）；`build_tests` 11 项 PASS；证据 `native/build/r38-*`。

### 20.36 R39 更新：全州枚举接入正式 UI

- reader 钩子内新增一次性全州枚举（lookup + vtable 相等 + `state+8==id` 三重校验、容器读取），绕过“本地价格”页签首屏限制。
- 实机 1117 州：`totalProd=88,936,587,381`、`totalCons=94,231,898,026`（≈889,366/942,319 单位）；正式页签 `Freight 126318.85`、`Target 519674.48 / Allocated 93784.10 / Unmet 425890.38`，分配率 18.0%。
- 候选/已部署：`d98d815b8f46520b056168489d2d2b1725d3f47099c1957880f2b52448e1566a`（28,922,465 B）；`build_tests` 11 项 PASS；证据 `native/build/r39-*`。

### 20.37 R40 更新：market 字段追踪（负结果）

- 对比英国本土 16/17/18 与外国 132/432 的 state 0x200 字节、0x80/0xa8/0xd0 目标对象 0x80 字节：无“英国相同、外国不同”的 qword/dword。
- market 非 state 直接字段，可能经所有者国家/省份间接获得；需先验证州身份或 hook 市场面板 market 对象。
- 候选/已部署：`a6fe068d0d76205cf2d701066ce7e38f37a78959add8b4bfc272f02659df6a2d`（28,928,697 B）；`build_tests` 11 项 PASS；证据 `native/build/r40-*`。

### 20.38 R41 更新：真实数据 ribbon 上屏

- `ApplyPendingFreight` 不再要求 ImGui 可见；原生 owner 活跃即提交 ribbon。
- 实机：真实帧 989 ribbons 上屏，`compute=768.119 ms`（后台 worker），`native pool 989/8192`、`visible yes 999`；地图可见细线。
- 候选/已部署：`b6582a238252bb0abedb69c19f05a21008adb31552d0e04a053cb0cc423630b4`（28,929,209 B）；`build_tests` 11 项 PASS；证据 `native/build/r41-*`。

### 20.39 R42 更新：分配性能剖析与首个优化

- 分段计时：`allocation=741.30 ms / view=10.73 ms`；分配内 `flow=729.03 / decomp=11.16 ms`（7016 shipments）。
- 分解阶段 importer O(1) 查表，分解降至 11 ms；瓶颈为 min-cost flow 增广。
- 候选/已部署：`f85067d9fb6df0f33ad8f09eb8c588337c8c151aec6c281b59f19c33f16bed87`（28,966,428 B）；`build_tests` 11 项 PASS；证据 `tests-round42.log`。

### 20.40 R43 更新：分配并行化

- `AllocateFreight` 按 (market, good) 组并行（最多 8 线程）；balances 每组独立、shipments 合并；异常经 exception_ptr 回传。
- 实机：`allocation=126.94 ms / total=133.38 ms`（8 线程），`flow=893.20 decomp=12.62 shipments=7016`，ribbons=989；约 5.8×。
- 候选/已部署：`df10b6c29abb77cfe587c8b65d44a65f1800583ebb84056d93404f0f156b0f06`（29,182,830 B）；`build_tests` 11 项 PASS；证据 `tests-round43.log`。

### 20.41 R44 更新：默认真实数据（无 probe）

- 新增 `CollectRealEconomy`（UI 定时器、vtable+容器数据探测、5 秒重试），移除真实数据对 `probe_economy` 的依赖。
- 实机（probe_economy=0）：`collected states=1117 orders=28554 goods=49`，页签真实数值 `126318.85 / 519674.48 / 93784.10 / 425890.38`，989 ribbons，`compute=140.695 ms`。
- 候选/已部署：`0a21fdb1890e53cec79b8a81512a76745c1c57e0d93304848bcf086e2242dad0`（29,300,253 B）；`build_tests` 11 项 PASS；证据 `native/build/r44-final.png`。

### 20.42 R45 更新：世界变化的生命周期失效

- 采集记录 anchor（首个州指针+vtable+id），`CheckWorldLifecycle` 每 ~2 s 复核；不匹配则 ResetWorld、清空文档并重新采集。
- 实机 20 s 无 `world changed` 误报；数据保持（1117 州/28554 订单/989 ribbons，137 ms）。
- 候选/已部署：`f447b9b754b9dceab3b93a027be0c413b155152d5053e2442f2458dfabd9349c`（29,302,357 B）；`build_tests` 11 项 PASS；证据 `tests-round45.log`。

### 20.43 R46 更新：周期刷新与重载演练尝试

- 每 30 秒重新采集（revision+1），worker 后台重算；实机 rev=1→2、gen=1→2，989 ribbons，compute 141.7/144.9 ms。
- 重载演练：暂停菜单自动化点击命中“切换国家”界面（未选国家/未保存/未改档），端到端演练待补。
- 候选/已部署：`b19e8308fb1ac83cddda35a4c2b8ff73812e7039f4d5a7017d988798268eee97`（29,303,893 B）；`build_tests` 11 项 PASS；证据 `tests-round46.log`。

### 20.44 R47 更新：回滚部署演练通过

- 卸载 DLL → 游戏正常进入地图（无物流页签/箭头）；重新部署 → 采集 1117/28554/49、989 ribbons、144.2 ms，功能恢复。
- 无源码改动；候选/已部署 `b19e8308fb1ac83cddda35a4c2b8ff73812e7039f4d5a7017d988798268eee97`；`build_tests` 11 项 PASS；证据 `native/build/r47-*`。

### 20.45 R48 更新：内存平台与刷新稳定性

- 3 分钟 18 次采样：warm 7278–7302 MB（±24 MB，0.3%），后段下降、无泄漏趋势；周期刷新 rev 5→6、989 ribbons、~139 ms。
- WorkingSet 含游戏基线（~7.3 GB），mod 增量未单独隔离。
- 无源码改动；候选/已部署 `b19e8308fb1ac83cddda35a4c2b8ff73812e7039f4d5a7017d988798268eee97`；`build_tests` 11 项 PASS；证据 `tests-round48.log`。

### 20.46 R49 更新：世界失效安全演练

- 抽出 `InvalidateEconomy`；新增 `test_world_change`（默认 0）调试注入。
- 实机：采集 → `TEST world-change: invalidating` → 重新采集 → `gen=3 rev=2`、989 ribbons、139 ms。
- 真实重载自动化打开国家预览界面；未继续（未改档）。
- 候选/已部署 `5e74477a73f50e0924f72b449ab2986f8b619d698d623a91e23544e9984691fc`（29,309,069 B）；`build_tests` 11 项 PASS；证据 `tests-round49.log`。

### 20.47 R50 更新：GUI 补丁镜像修复与全量回归

- Python `patch_market` 与 C++ 逐字对齐（container/内容块/FILTERS/搜索栏/过滤器容器）；`test_native_gui_runtime.ps1` 恢复 PASS（exact agreement）。
- 全量回归：native_gui / native_gui_runtime / flow_gpu / selftest / debug_default / verify_embedded / build_tests 全部 PASS。
- DLL 未改动；候选/已部署 `5e74477a73f50e0924f72b449ab2986f8b619d698d623a91e23544e9984691fc`；证据 `tests-round50.log`。

### 20.48 R51 更新：容器头探针与验收矩阵

- `probe_market`：8 个州的 `state+0xC8/+0x78` 容器头转储；16/17/18 与 132/432 无共享 market 指针/ID（负结果）。
- `ACCEPTANCE.md` 更新为 R51 状态（PASS/PARTIAL 明确列出，缺口标注）。
- 候选/已部署 `a25f78ae7006562a826fa3992339d64743d3877e4173656dd7cd91aa6fd7ea45`（29,311,629 B）；`build_tests` 11 项 PASS；证据 `tests-round51.log`。

### 20.49 R52 更新：全局作用域标注

- 正式页签状态改为 `model allocation (global)`，如实标注全局作用域；实机显示真实数值。
- 州面板点击威尔士/苏格兰高地名称与 positions 一致；runtime id 未由 reader 证实。
- 候选/已部署 `5db7d577ce9375a2104fd705e1867f2ed1029aa493aeea89fa9c38d0346559a4`（29,312,141 B）；`build_tests` 11 项 PASS；证据 `native/build/r52-*`。

### 20.50 R53 更新：market 作用域突破

- `state+0x48` = market id（1117 州仅 59 个值；威尔士/高地=79、莫斯科=71）。
- 快照按市场分组；`primaryMarket`=州数最多市场；session/filter/selection 使用；页签 `model allocation (market N)`。
- 实机：`markets=59 primary=118 states=77`、37 ribbons、136 ms；页签 `16440.38 / 1862.16 / 14578.22`。
- primary 为启发式（州数最多），玩家市场精确选择待做。
- 候选/已部署 `371d2db7ed57dcd20fc11ced2d76ba901a3881629d1ada0a45ad4e249dabc42f`（29,618,209 B）；`build_tests` 11 项 PASS；证据 `native/build/r53-*`。

### 20.51 R54 更新：market_override 可配置市场

- 新增 ini `market_override`（默认 0=最大市场；>0 锁定市场 id）。
- 实机（79）：`primary=79 states=30`、47 ribbons；页签 `model allocation (market 79)`、`7465.20 / 451.99 / 7013.21`。
- 玩家市场自动识别仍未完成；override 提供可控修正。
- 候选/已部署 `f02169a82b3153c262577ba85a6505cf40a9e968d64c537956cc33e2308a66ea`（29,625,098 B）；`build_tests` 11 项 PASS；证据 `native/build/r54-override79.png`。

### 20.52 R55 更新：面板开关循环稳定性

- 10 次 F4/ESC 开关（含 Shift+5）存活、无错误/无 `world changed`；周期刷新 rev 1→2、37 ribbons（135–137 ms）。
- 重开后页签正常：`model allocation (market 118)`、`16440.38 / 1862.16 / 14578.22`。
- 无源码改动；候选/已部署 `f02169a82b3153c262577ba85a6505cf40a9e968d64c537956cc33e2308a66ea`；证据 `native/build/r55-cycles.png`。

### 20.53 R56 更新：脚本注册探针（负结果）

- 新增 `probe_register` + `kRvaScriptRegister=0x3AB6D00` 探针；实机安装成功，但未观察到 market 注册。
- 结论：该路径不能取得市场面板 market；玩家市场识别仍用“最大市场 + `market_override`”。
- 候选/已部署 `227793796c7413948ca97af290a258f69b25a6a1b078983eae2f428756046e1c`（29,630,860 B）；`build_tests` 11 项 PASS；证据 `tests-round56.log`。

### 20.54 R57 更新：市场归属核对

- `probe_market` 新增 `eco market id=`：17/19=79（UK）、132=71、350=83、201=64、205=33、432/436=164；默认 primary=118 不是 UK。
- 游戏变量表仅含我们的 7 个键，无市场上下文。
- `market_override=79` 可得到 UK 市场；玩家市场自动识别仍未完成。
- 候选/已部署 `a98e45b56e08d7f8960e308797d2aaf351fb3e63925e8d2491909f5ab163a394`（29,632,396 B）；`build_tests` 11 项 PASS；证据 `tests-round57.log`。

### 20.55 R58 更新：README 用户文档刷新

- 修正标题乱码；过时描述改为正式第五页签 + 实时市场作用域数据。
- 新增 Logistics tab semantics：order-cost 口径、target/allocated/unmet、`model allocation (market N)`、`market_override` 用法、非游戏订单/MAPI 声明。
- 无源码改动；DLL 仍为 `a98e45b56e08d7f8960e308797d2aaf351fb3e63925e8d2491909f5ab163a394`；README UTF-8 校验通过。

### 20.56 R59 更新：市场提示日志与设置持久化

- 采集日志新增 `top=`（前三市场州数）：`top=118:77 164:75 35:74`，便于选择 `market_override`。
- `SaveSettings` 补写 `probe_market`/`market_override`（保存不再丢失）。
- 候选/已部署 `065bbf34db83f4843aafc29095e7f4692d5a6b1629866ebcc45a4efa0f66aa85`（29,750,126 B）；`build_tests` 11 项 PASS；证据 `tests-round59.log`。

### 20.57 R60 更新：自动市场标注

- 页签自动选择时显示 `model allocation (market N, auto)`；`market_override>0` 时显示 `(market N)`。
- 实机自动模式：`model allocation (market 118, auto)`、`11728.46`、`16440.38 / 1862.16 / 14578.22`；37 ribbons、138.8 ms。
- 候选/已部署 `0cb83a456e3b6237db2b36574848ac0e6f87ef3c96890d7edc5ff11d51672d56`（29,752,211 B）；`build_tests` 11 项 PASS；证据 `native/build/r60-auto.png`。

### 20.58 R61 更新：完成审计与实现报告刷新

- `COMPLETION_AUDIT.md` 重写为 R61 状态（逐项证据 + 缺口：玩家市场自动识别、铁路、hover/filter、真实读档）。
- `IMPLEMENTATION_REPORT.md` 顶部新增 Current status（当前哈希、实时市场作用域数据、已知缺口）。
- 无源码改动；DLL 仍为 `0cb83a456e3b6237db2b36574848ac0e6f87ef3c96890d7edc5ff11d51672d56`；文档 UTF-8 有效。

### 20.59 R62 更新：页签市场概览行

- 新增第 8 个 GUI 变量 `ironroad_markets_v1`（前五市场 `id:states`，互斥保护）与页签 `Markets (id:states): ...` 行，便于选择 `market_override`。
- 实机：`118:77 164:75 35:74 33:68 34:58`；变量发布无失败；37 ribbons、136.2 ms。
- 候选/已部署 `c69340f169f680346a1643409995bfc602f38d6ff57b3d643ab2324d11b3a16a`（29,771,621 B）；`build_tests` 11 项 PASS；证据 `native/build/r62-markets.png`。

### 20.60 R63 更新：全新缓存重装演练

- 删除 `%LOCALAPPDATA%\IronroadV3` → 卸载/重装 → 启动自动重建缓存（data/assets/native-gui/ini/log）。
- 页签正常：`model allocation (market 118, auto)`、`11728.46`、`16440.38 / 1862.16 / 14578.22`、`Markets (id:states): ...`；37 ribbons、141.4 ms；无错误。
- 无源码改动；候选/已部署 `c69340f169f680346a1643409995bfc602f38d6ff57b3d643ab2324d11b3a16a`；`build_tests` 11 项 PASS；证据 `native/build/r63-fresh.png`。

### 20.61 R64 更新：脚本函数注册表探针（负结果）

- Ghidra：注册表固定全局 `0x1459FD648`，条目 0x78，size 在 +0x08（实测 141）。
- 新增 `probe_registry`：实机读取表头/条目，未提取出条目名（偏移不符），未找到 market 函数；探针默认关闭。
- 候选/已部署 `200f08d2d738d829afa47097758f14d6250cfcd20c9945cda324d714bd4de134`（29,781,483 B）；`build_tests` 11 项 PASS；证据 `tests-round64.log`。

### 20.80 R82 更新：地区列可见与真实地图身份修复

- 用户手动截图确认中英格兰本地价格Freight列可见且--；表头省略号待调整，数值／全交互未通过。只读采样PID35764，实际state_panel解码与字节回放得到州433、market111、local_goods，无界面输入／进程写入／调试器附加。
- State.GetStateRegion与定义构造经Ghidra＋Capstone核对：州433→STATE_MIDLANDS（引擎定义ID17）→派生地图ID20，三类ID不同。旧order.region=id是错误绑定，历史实机货流地理／分配结论需重新验收；地区发布草稿已暂停并私存。
- 新state_region.h复制稳定key与身份／字符串校验，adapter按key绑定派生图，未知／歧义／变化拒绝。1147/1147实机州映射成功，675唯一地区key；新回归与全14项、完整build、嵌入校验PASS。候选290ee571a172264695f9889430932ca48d131e623364f55f6f660267798a8ebd（32,063,114B），未部署，游戏保持R81。
- 用户新增兼容改图Mod要求见§7.1。当前仅身份映射必要步骤，读取Mod实际资源／几何版本尚未完成；不将原版key匹配当完整兼容。地区owner／数值行、表头／日志／icon问题与全部原目标继续。
- 备份2026-10-23-r82-state-live-bridge；证据native/build/*round82.log，私有state-region-r82-notes.md／assembly.txt／live helper与字节；AGENTS／STATUS／WORKLOG／TASKS／PROFILE／模型同步。

### 20.79 R81 更新：地区owner与当前快照发布边界

- 部署／启动后补充：安装hash与候选一致；PID35764真实GUI挂载／枚举和1147州快照有日志证据，未通过可见布局。用户再次Escape停止后要求手动操作，停止自动输入，转手动地区本地价格验收。重复null publish和市场卡片icon margin_right属性错误待下一候选修复，原始日志私存state-ui-r81-runtime.log。
- 新NativeStateRoster在接受快照时复制州→市场，拒绝冲突／无效ID；NativeStateOwner仅当前原版地区local_goods上下文持有独立owner15，使用该州实际市场。重复采样不重排计算、切州复用分配、关闭／切页不清其他owner，明确保留ID0。
- Session新增SnapshotIsCurrent验证自有快照指针，补齐同world／revision替换边界。CanPublish核对当前owner／文档与帧的州／市场／版本、源指针和FrameIsCurrent；迟到结果、新帧配旧owner、合成输入／reset均拒绝。市场变量限定Market文档，防止地区文档进入市场摘要。
- 新会话用例及全14项回归／完整build／资源验证PASS；候选234635d6c92231a7102f31044e0dafcabfac45cc776c85276e0a13279f10c1a0（31,984,223B）。此项完成数字owner／发布门控接口，原生state decoder／owner／行写入仍未接运行时，地区列仍--；实机上下文／布局／交互未验，完整目标继续。
- 用户本轮明确允许恢复游戏界面自动操作，解除Escape停止；使用computer-use技能核验唯一目标后继续安全部署／实机验证，不强制结束或替换运行DLL。备份2026-10-23-r81-state-owner，证据native/build/*round81.log，AGENTS／STATUS／WORKLOG／TASKS／UI同步。

### 20.78 R80 更新：地区本地价格列与州作用域数据格式

- 按§3.2.1派生本机states_panel.gui，仅本地价格表追加Freight列并调整表内列宽，保留价格／差额／原版排序与行内交互；其他页签和非本地商品区块保留。原始GUI需精确hash，DLL只嵌入自有单元格／本地化，缓存才含本机派生文件。
- 文档明确kind／state，新逐行格式拒绝市场或其他州数据、未知／重复州、错误市场、无版本及合成输入；州＋商品键独立，ready ID匹配State.GetIDString才可显示，否则--。R77的当前州输入／输出工作量每笔一次保持，hover不改变行事实。
- 静态恢复State.GetIDString注册／类型／getter，Ghidra与Capstone确认有符号int32十进制（包括高位ID），对应新回归通过。14项全回归＋最终相关UI回归、完整build、GUI／实际C++归档逐字镜像、嵌入资源、selftest／默认隐藏PASS。
- 候选b257418f7861d39cc1dd6f26287576ec46d2451cea61dfb6390aad7f32a3233a（31,985,143B），未部署，安装仍R78。此单元仅完成地区列布局与数据格式，owner／运行时行发布仍缺，正向当前州／GUI解析与可见布局NOT_RUN；未启动或操控游戏，Computer Use保持停止。
- 备份2026-10-23-r80-state-freight-column，证据native/build/*round80*.log／r80-gui-preview、私有state-column-r80-static.md／assembly.txt。AGENTS／STATUS／WORKLOG／TASKS／UI／模型同步；继续owner／发布门控和所有未验收目标。

### 20.77 R79 更新：地区当前州上下文离线解码

- 原版 states_panel 的构造／名称映射／State类型setter及独立汇编核对：panel bar slot14、vtable RVA0x45CC848、ID+0x70，Local Prices 使用 local_goods。EXE hash与既有profile一致；静态候选不能代替运行时当前州／快照对应验证。
- 新增 state_panel.h，复制自有状态、核验所有权／打开谓词／tab及ID、拒绝坏输入和复制期间变化；无getter调用、worker引擎指针或runtime caller。未验证访问窗口／生命周期，保持不接正式界面。
- 新增原生校验用例并接构建入口，共14项回归PASS，verify_embedded PASS；本轮未重建／部署DLL，R78候选及安装hash不变。未启动／附加／操控游戏，Computer Use保持停止。
- 本子单元完成，地区Freight列／owner与行发布仍待当前州实测接线；真实铁路、Freight独立供给、原子快照及全部实机／生命周期验收继续。备份2026-10-23-r79-state-context；证据native/build/tests-round79.log、state-context-round79-final.log及私有state-panel-r79-static.md／assembly.txt。AGENTS／STATUS／WORKLOG／TASKS／UI／PROFILE同步，总目标active。

### 20.76 R78 更新：参考图市场卡片、商品列与逐行数据桥

- 按 §3.2.1 接市场 Freight／已铺铁路卡片、商品工作量／占比列、显示／隐藏；原版价格图表、tooltip、右键和商品行顺序保留。价格块由本机已校验 GUI 派生，DLL 仅含自有布局与占位符，仍纯DLL交付。
- 逐行数据由当前自有文档生成，按商品键排序去重／校验；份额分母固定，hover不改行数值；未知／合成／无效值为--，合法零工作量与无定义百分比分开。UI线程发布前后复核当前帧，旧／部分写入由上下文门控隐藏，移除行清空；已有键直接更新值以免重复分配临时pair。
- 13项回归、完整build、GUI／C++ ZIP逐字一致、verify_embedded PASS；候选／已部署 `48f0e495698bcb754bdd400a77cdc2f05c68543bf1c2d880d6f05ff5cb69cf29`（31,753,550B），无游戏进程时deploy.ps1安装、hash一致，旧R76留备份。
- 原生 GUI 解析／新布局截图／逐行发布／性能尚未实机验证；不据生成器通过宣布“像参考图”已验收。地区列和当前州桥仍继续，Computer Use保持停止。备份2026-10-23-r78-market-reference-ui，AGENTS／STATUS／WORKLOG／模型／UI文档同步。

### 20.75 R77 更新：参考图的筛选卡片与地区工作量作用域

- 新增 selectionFreight，将全市场分母、当前筛选工作量、临时 hover 地图贡献分开；正式卡片使用当前筛选，hover不改卡片或份额。地区每商品工作量仅汇总该州最终输入／输出，每笔一次，未更改 Freight 中转需求公式或分配。
- 地区反例先失败；恢复原构建／测试句柄确认退出0，13项回归、完整build、verify_embedded PASS。候选 `3695238f83be193aa596e101360f53ec6b70c01a27ba5182b0f216aaf9d3787f`（31,174,023B），未部署／未实机，安装仍R76。
- 本部分仅完成统计作用域；§3.2.1 的地区列、市场卡片／商品行布局与数据桥继续实现。界面自动操作保持停止，备份2026-10-23-r77-reference-freight-scopes，AGENTS／STATUS／WORKLOG／模型文档同步。

### 20.74 R76 更新：原版筛选恢复、商品悬停与参考图需求

- 用户已裁决恢复原版搜索／分类；C++／Python 删除旧子控件隐藏改写，原版控件／折叠保留。控件保留测试先失败后通过，版本拒绝与归档镜像继续通过。
- 候选商品行 enter／leave 使用原版变量效果及逐商品键／面板world上下文，临时 hover owner 只接受当前可见商品；移出恢复筛选，关闭／切页／身份失效撤两个正式owner，保留调试 owner。未知／歧义／旧上下文／失败读取拒绝，正式hover不依赖 ImGui。UI定时25ms、经济周期500ms分开，同帧关闭与新轮询开销待实测。
- 13项回归、完整 build、GUI／归档／资源、selftest（560231 bright pixels）及默认隐藏 PASS。候选／已部署 `15fee7d63d34db078fafa1128de1afff1244c2a06b5e68896c15f044c16a5329`（31,174,023B）；确认无游戏进程后通过 deploy.ps1 安装，hash一致。旧PID52316及后续8032均已退出，未强制结束；sky启动handler错误，新候选实机仍 NOT_RUN。
- 用户新增参考图及工作量口径已写 §3.2.1：原版地区列与市场卡片／商品列为下一优先实现，不能以悬停事件桥替代其交付；截图数字不用作实时数据。备份2026-10-23-r76-native-hover，AGENTS／STATUS／WORKLOG／TASKS／UI文档同步，总目标继续。

### 20.73 R75 更新：原版摘要的货运工作量单位修正

- 核对当前分配器／视图／文档，`shipment.freight=(quantity/kQuantityScale)×weight×distance` 已是 weighted order-distance；原版显示再除以 1000 并误标 order-cost。以 7 orders×10 map units=70 的显示反例先失败后修复，直接格式化工作量，更新正式标签／README／FREIGHT_MODEL。
- 13 项回归通过：费率只影响费用；权重影响工作量；筛选保持全市场工作量；独立 Freight 需求不受这些显示修正影响。无数据／合成／无效值仍 --，有效零保持可见。完整 build、原版 GUI／C++ 归档镜像、verify_embedded PASS；候选 `64485541f14b7252297a0fa96a85bee3c0114e5fbd328c6d0af76e8001d020f8`（30,921,468 B），未部署／未实机验证新单位。
- 控件入口调查确认旧 R22 仍刻意隐藏物流搜索／分类，而总任务书要求复用；依据用户新加 AGENTS §1.1 已请求裁决，此处保持暂停。既有 GUI 测试 PASS 仅证明绑定／镜像，不证明可见交互。游戏仍 R69，未强制结束或替换运行 DLL。
- 备份 2026-10-23-r75-freight-work-units；反例及最终构建测试 native/build/*round75.log，AGENTS／STATUS／WORKLOG 同步。正式 hover／逐帧关闭、Freight 独立供给／铁路／原子快照和全部实机验收仍继续，原目标未缩减。

### 20.72 文档维护：编码工作规范

- 按用户要求优化 AGENTS.md，新增 §1.1–1.4：明确假设与歧义、优先简单实现、限制修改范围、以可验证目标推进；修正“代码重构svg”笔误。
- 保留既有项目约束与历史记录，补充 AGENTS 进度及 STATUS。仅文档变更，检查备份差异与 UTF-8 无 BOM；不增加代码或实机验收结论，R74 候选及剩余任务保持原状态。

### 20.71 R74 更新：复用原版搜索／分类结果的视图筛选

- 补齐明确市场 ID 0 与旧版“未指定市场”的区别：原版 owner／SelectMarket 显式保留 0，只有未指定选择才自动回退；会话反例覆盖 market 0／primary 7，不能据此认定当前存档市场 0 有效。最终 tests-round74-final.log／build-round74-final.log 均退出 0，13 项回归通过。
- 静态核对原版分类是字节型排除数组（panel+0x110），IsSelected为“不在排除表”；搜索经Unicode转小写并包含原版本地化／关键词匹配，不能按英文good key重做。定位Market.AccessMarketGoods的缓存列表（wrapper+0x60）、dirty／writer标志与商品身份。
- 新增native_goods_filter.h，在现有UI线程安全复制该原版缓存的商品ID：唯一market匹配、query一致、dirty／writer清空、类型／商品／market／分类一致和重读验证；不调用getter、不持引擎锁。原版搜索／分类结果直接限定地图贡献，worker只接自有ID；待刷新与已知无匹配分开，前者撤视图并显示waiting，后者保留市场统计而无线路。
- 新增allowedGoods视图筛选及规范化缓存身份；重复排序不重建视图，搜索／分类不重跑分配、不改全市场分母／热度／Freight目标与满足量。13项原生回归、完整build、资源校验PASS；候选 `1a97a6823e2fc72fa7f1f2178807803a4001fc89faee3ccd8df3fdc8b0223cfd`（30,916,271B），未部署。
- 实机PID52316仅观察到registry两项（market0／FFFFFFFF），均dirty1／rowCount0；实际解码器回放拒绝为“待刷新”，**没有正向筛选列表或新DLL搜索／分类交互证据**。原版缓存就绪后的布局／UI对应、500ms响应延迟、hover及完整生命周期仍待验，不把fixture正例当实机通过。
- 保持无人值守，游戏仍R69、调试器已分离，未强制结束／替换DLL。重读01／03全文，复核原版GUI／profile与当前仲裁；备份2026-10-23-r74-native-filter-view，证据私有native-filter-r74-*及native/build/*round74*.log。Freight独立供给／铁路／全部正式验收继续保留。

### 20.70 R73 更新：原版市场状态与正式 owner 接线

- Ghidra＋Capstone确认信息面板栏持有市场面板（slot9）、原版IsPanelOpen判断（history非空、closed=false、最后类型9）、页签字符串与market字段；x64dbg在PID52316验证整条所有权链，当前为关闭态／默认页／无market。实际C++解析器对实机字节回放PASS；未执行原生getter、RTTI cast或GUI效果。
- 新增market_panel.h安全复制／类型／所有权／字符串／重读校验；native_market_owner.h接现有仲裁，仅已打开的物流页签拥有正式视图，按面板market选择，不再由快照就绪激活最大市场。关闭／切页／未知市场／元数据缺失撤正式owner；重复采样不重启worker，保留其他owner。原版摘要仅接受当前正式owner帧和相同world／revision／market。
- 当前同步入口仍是500ms UI定时器，**不视为逐帧即时关闭验收**。候选完整构建、13项原生回归、关闭态实机字节回放、资源校验PASS；最终DLL `e0a2e7863db4f016091888b4e6d885678fae3149db8a12e6e4f22b58c7a3d1a6`（30,376,351B），未部署／未跑新DLL。打开／市场切换／关闭的实机交互、hover／筛选、完整生命周期仍未通过。
- 仍运行R69，窗口未被控制工具枚举；调试器只附加本次测试进程，已分离确认stopped、游戏Responding／日志继续，未强制结束／替换DLL。重读01／02／03全文；备份E:\Code\v3-analysis\checkpoints\2026-10-23-r73-native-panel-owner；私有market-panel-r73-*与native/build/*round73*.log。用户要求无人值守继续，Freight独立供给、铁路及全部正式验收仍未完成。

### 20.69 R72 更新：实机商品目录／TLS 验证与真实快照接线

- 本机 x64dbg2.3.0 连接当前测试 PID52316：53个商品的原生 ID／key／category／local／tradeable 按key对原版定义53/53一致；53定义和目录向量重复字节一致。主线程51844的TLS初始化／token／index0／slot0与静态getter一致；两个实际C++解析器用实机字节回放均PASS，不按文件顺序猜ID、不选首个非空slot、不调用会初始化／持锁的getter。
- 新增 goods_snapshot.h；正式采集器在既有UI定时器线程、精确版本门槛下安全RPM复制当前已初始化context，采集前后比较目录；按原生ID填真实名称键／分类／local／tradeable，删除goodN默认值。未知／缺失／变更元数据时撤正式owner和文档，显示waiting／`--`，不把失败变成零需求；worker只持自有值。默认weight／Freight系数1仍是模型假设。
- 完整构建、12项原生回归、实机字节／TLS回放、资源校验PASS；最终候选 `05b58a2d1b85c61ac952b4c38b9744be16c915f4b84c6b6092dd827a2156293b`（30,159,766B），未部署。当前仍R69 `17268f5e…`；**新DLL adapter／修正后的正式总量尚未实机验证**，稳定重读不证明原子经济快照或完整生命周期。
- 附加期间自动TLS断点／Nakama首次C++异常曾暂停，已清除本次新会话自动断点、正常恢复并分离，确认无活动调试目标、游戏Responding／日志继续。未显式写经济数据、未强制结束／替换DLL。重读01／02／03，备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-r72-live-goods`；证据私有goods-context-r72-notes.md和native/build/*round72*.log，原始字节不进入分发。继续无人值守推进，Freight独立供给／正式交互／铁路／生命周期仍未完成。

### 20.68 R71 更新：商品元数据解码与不可交易商品语义

- 用户要求暂时无人值守，不再等待手动显示隐藏游戏窗口；当前 PID 52316 继续运行 R69，不强制结束／暂停／替换 DLL。
- 同哈希 EXE 静态确认 Goods.GetKey／IsLocal／IsTradeable 及数据库构造／postload；商品 ID 由数据库分配，不能按原版文件排列猜测。新增独立 copied-memory 解码器，严格拒绝类型／ID／字符串／重复 key／观察到的复制期间变化；结果不持 engine 指针。未接正式读取，生命周期／线程窗口及运行时 catalog 身份尚未验证。
- 模型增加独立 tradeable 标志，非本地不可交易商品也排除跨州分配与 Freight 中转需求；缓存随分类变化失效，原始消费副本不变。**当前真实 adapter 仍 goodN／默认 localOnly=false、tradeable=true；本轮尚未修复正式输入的这项缺口，现有实机总量不能据此认定分类正确。**
- 完整构建、12 项原生回归、资源验证 PASS；候选 `698c8b8dc5486c4fc3f3e96086da208deeaf115b1f2d4b352c22ab0d21b08d56`，29,840,576 B；未部署，运行中仍 R69。x64dbg 插件未上线，未附加／中断游戏。
- 重读 01／02／03 全文及模型／profile；备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-r71-goods-metadata`，静态证据 `E:\Code\v3-analysis\goods-catalog-r71-static.md`，构建／测试 `native/build/*round71.log`。下一步验证安全元数据访问并接真实快照；Freight 独立供给机制／正式 UI 动作／完整生命周期继续保持未完成。

### 20.67 R70 更新：默认正式 owner 运行证据与重复文档发布修复

- 通过正常进程启动接口（非窗口工具启动器）成功启动已部署 R69，PID 52316，`-continuelastsave`；启动器错误不代表游戏本体无法启动。启动采用 Hidden，游戏未暴露可操作窗口；已向用户请求显示窗口，未发送 UI 输入或强制结束进程。
- 当前进程日志：默认 ImGui 启动 vtx=0，真实采集 1117 州／28554 订单／49 商品、59 市场，primary=118／77 州；正式 owner=10 生成 37 ribbons，world=1、revision 1→6，后台计算约 133 ms。持续 `SUBMIT scope=0/1/0` 已证实正式 owner 在调试窗口关闭时可提交，但尚非可见图像证明。
- 热态连续 170.453 秒、35 个采样点 uploads 总数恒定 37，submission count 7966→303596；冷阶段另有 uploads=16，已保留而非过滤伪装热态。证据 `runtime-round70.log`／`runtime-summary-round70.json`。仅通过“该市场该场景热态零新增上传”的日志门槛，未完成 UI 筛选／重开、相机、模拟 tick 或完整性能验收。
- 实机日志还发现 6 次快照采集对应 349 次实时文档发布。原因：LatestWorker::Poll 保留同一 reply，Session 未使用 m_lastApplied 去重，每 500ms 重建／发布同一文档。新增回归先 FAIL，修复为接收／最终发布都拒绝已应用 generation，保留当前渲染 frame，不影响新 revision／owner／reopen 发布。
- R70 候选 `f3a9a4befc8db63e6f2562ba4a71be6e38b447055aecc38d98756ef440c5e7e3`（29,840,576 B），完整 build／11 项原生回归／verify_embedded PASS；未部署、未跑会占用游戏日志的 standalone selftest。运行中仍为 R69 `17268f5e…`，不强替 DLL。R70 代码尚无实机运行证据。
- 备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-r70-runtime-publication`；证据 `native/build/*round70*`。窗口可见性、R67 Freight 新行、市场 getter 实机样本及 R69 完整读档仍未验收，Freight 供给范围问题仍待用户答复，总目标进行中。

### 20.66 R69 更新：世界重置／迟到快照与发布门控修复

- 新增反例先复现：Engine／Session 的过期验证依赖 snapshot 非空，ResetWorld 清空指针后可接受旧 world。修复为独立保留已接受身份和退休门槛；reset 后必须严格更新 world，高 revision 不能复活旧世界。
- worker 重置改为独立原子失效代数，Cancel／debounce 合并不会吞掉 reset。Session 在自有锁内串行入队与 reset，接受快照立即撤下旧帧；最终发布重新核对 snapshot 指针、owner、filter 和 generation，render 上传／提交使用 FrameIsCurrent 再验证。
- 实时源 world 不再固定 1，失效时递增；周期刷新先检查旧 anchor，不因 collected 标志为 false 而跳过。旧 reader 的累计观测样本无有效 world／market，改为仅诊断落盘，不进入正式会话。
- 修复 OnRenderPass 中残留的 ImGui 可见性门槛，正式 native owner 可在调试界面关闭时走提交路径；这是源码修复，未取得本版本实机可见证据，不能将旧提交日志或离线测试当作最终通过。
- **临时安全边界**：检测到真实 world 替换后，原生对象注册／销毁寿命仍未验证，停止后续旧 native 槽访问／提交并在 UI 提示重启恢复地图。数字层可接受新 world；未猜测析构／释放，完整跨读档支持仍未完成，A24 不通过整体门槛。anchor 完全复用的识别仍须真实生命周期信号与实机验证。
- 验证：`repro-round69.log` 修复前 FAIL；`tests-round69-final.log` 11 项 PASS，覆盖退休 world／高 revision 反例、合并取消保留 reset、旧帧立即失效与发布门控。最终 build／GUI runtime／verify_embedded／selftest／debug_default PASS。候选／已部署 `17268f5ea3074e6072b3b68d7b64c29b837987cf0ebd1ff36b59fdb0e3bb3bba`（29,839,552 B）。
- 仅经 deploy.ps1 部署。备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-r69-world-reset`（含旧安装 R67 installed-version.dll）；证据 `native/build/*round69*.log`／`changes-round69.patch`。本轮启动器仍报 window-opened handler 未就绪，无游戏窗口／进程，实机 NOT_RUN；未终止游戏或改存档。

### 20.65 R68 更新：市场面板 getter 的静态契约与关闭态探针

- 同哈希 EXE 的 Ghidra／独立 Capstone 复核：`MarketPanel.GetMarket` getter RVA `0x225AC90` 为 `(panel, callerOut) -> callerOut`；从 `panel+0x70` 的 32 位 ID 经 `0x10571F0` 得到 GUI wrapper，并递增其引用计数。wrapper 构造 `0x1CD6DC0` 将 ID 写入 `+0x10`，vtable `0x45850B0`。
- 发现重要边界：该 resolver 会持锁、未命中时创建 0x3B0 wrapper，不可当作无副作用 getter 从定时器／Present 主动轮询。脚本 callback `0x225AED0` 会释放 getter 返回的引用，不能额外持有或释放。
- 新增 ini `probe_panel_market`（默认 0）。仅在 EXE hash 与 getter 指令前缀均匹配时安装观测 hook；原函数恰好调用一次、原样返回 out／RAX；复制并核对 panelId／wrapperId／vtable，最多日志 16 次，不改变选择／分配、不保留指针。
- 本轮证据层级：静态契约＋候选构建＋关闭态 loader 验证。面板切换、线程／生命周期及 market id 动态匹配 NOT_RUN；正式自动识别仍未完成，禁止据静态候选替换最大市场／override。
- 候选 `09b219a9e596f999bd3e2182672b2e551b2fa85e450f266b2297bdf64af27fd0`（29,877,463 B）；安装目录保持 R67 `52009510d0ff6c64af6e03b77ab109c7d9c6d5bafd56ce101116a2b6fb1c7bd9`。build、verify_embedded、debug_default PASS；R67 模型回归沿用，未机械重跑无关测试。
- 私有证据 `E:\Code\v3-analysis\panel-market-r68-static.md`；`native/build/build-round68.log`、`debug-default-round68.log`。回滚点 `E:\Code\v3-analysis\checkpoints\2026-10-23-r68-market-getter`。下一步在可操作游戏中启用探针，核对两种市场切换后，再接 world／owner 门控与正式选择。

### 20.64 R67 更新：Freight 中转需求核算与原版文本桥

- 分配前按原商品净输入需求 × 独立 `freightPerOrder` 系数生成 Freight 目标；分解最终货流后填 fulfilled／unmet，来源保留在州／市场／商品 balance。固定版本 1，默认系数 1 为模型假设，不含距离／运价，不冒充原版公式。
- 不可达／缺货不丢目标；本地商品与 Freight 自身不递归；市场／州作用域汇总不受筛选和 hover 影响。系数／中转分类进入缓存指纹，变更不改变原商品线宽／几何。
- 原版新增 `ironroad_demand_v1` 文本行，复制文档保留总量和商品贡献；无数据／合成输入显示 `--`。仅需求核算已完成，独立 Freight 供给与游戏内商品注册仍待设计，A27 未整体验收。
- 完整 DLL 构建、11 项原生回归（含新增需求反例）、GUI／原生归档一致性、嵌入资源、加载器 selftest、默认隐藏调试 UI 均 PASS。归档校验首次在新 DLL 构建完成前读到旧资源而失败，构建后按正确依赖顺序重跑 PASS；未放宽测试。
- 候选／已部署 DLL `52009510d0ff6c64af6e03b77ab109c7d9c6d5bafd56ce101116a2b6fb1c7bd9`，29,863,251 B。Windows 控制工具两次启动失败（`accessibility window-opened handler did not become ready`），未发现游戏进程，新增行实机可见性 NOT_RUN。
- 证据 `native/build/*round67.log`；源码／旧 DLL 回滚点 `E:\Code\v3-analysis\checkpoints\2026-10-23-r67-freight-demand`。已同步 AGENTS、STATUS、模型／任务／验收／决策／工作日志。

### 20.63 R66 更新：Freight 中转商品计算物流需求

- 用户新增要求已纳入 §4.5、P4 与 A27：以 Freight 为中转商品计算物流需求，保留各原商品贡献以及 Freight 目标／已满足／未满足。
- 实现差异：当前 `Shipment::freight` 仅统计已分配货流工作量，独立中转需求链尚未实现；换算公式、单位和供给机制待设计。
- 已同步 `AGENTS.md`、`docs/transport/STATUS.md`、`TASKS.md`、`WORKLOG.md` 与 `FREIGHT_MODEL.md`。本单元仅文档更新，UTF-8 无 BOM 校验通过，未构建或部署；DLL 仍为 `200f08d2d738d829afa47097758f14d6250cfcd20c9945cda324d714bd4de134`。

### 20.62 R65 更新：正式 UI 市场对比截图

- 默认：`model allocation (market 118, auto)`、`11728.46`、`16440.38 / 1862.16 / 14578.22`、37 ribbons。
- `market_override=79`：`model allocation (market 79)`、`601.13`、`7465.20 / 451.99 / 7013.21`、47 ribbons。
- 当前缩放下地图 ribbon 未清晰可见（market 79 含海外州）；页签数值确认作用域切换。
- 无源码改动；候选/已部署 `200f08d2d738d829afa47097758f14d6250cfcd20c9945cda324d714bd4de134`；`build_tests` 11 项 PASS；证据 `native/build/r65-*`。






















### 20.21 R24 更新：重开生命周期与商品面板联动

- Esc→F4→Shift+5 重开物流页签，内容/变量/按钮/货物列表正常，无崩溃（`r24-reopen.png`）。
- 点击商品行打开原版商品面板（钢，国际/本地/用途、消费税/关税、销售/购买订单，`r24-show.png`）。
- 显示/隐藏按钮的自动化点击仍受 GUI 缩放影响；功能绑定未变。本轮无源码改动，候选保持 `8f1a…780f`。



















