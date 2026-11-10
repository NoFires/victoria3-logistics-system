# AGENTS.md — 本仓库的稳定约束、构建入口与工作日志

> 用途：给下一个接手的编码 AI 的最短入口。**不替代** `MASTER_PLAN.md` 与四份原始消息文档。
> 每次完成一个有意义的工作单元，必须更新本文件第 6 节「进度日志」以及 `docs/transport/STATUS.md`。
> 仓库实际文件为 `AGENTS.md`；用户所称 agent.md 的工作记录统一维护于本文件，避免另建重复日志。

## 1. 阅读顺序与裁决

1. `MASTER_PLAN.md`（总任务书，含第 2.4 节按主题重读要求）
2. `00_START_HERE.md`、`01_IMPLEMENTATION_PROMPT.md`、`02_NATIVE_DISCOVERY.md`、`03_TESTING_AND_TROUBLESHOOTING.md`（**完整读**）
3. 本文件
4. `docs/transport/COORDINATION.md` → `STATUS.md` → `TASKS.md` → `BLOCKERS.md` → `DECISIONS.md` → `WORKLOG.md`
5. 按当前主题读 `FREIGHT_MODEL.md`、`NATIVE_GUI_PLAN.md`、`TEST_PROCEDURE.md`、`IMPLEMENTATION_REPORT.md`、`native/PROFILE.md`

裁决：用户最新要求 > MASTER_PLAN > 原始四文档；当前代码/二进制/实测 > 旧报告。事实、推测、决策分开写。

### 1.1 编码前思考

- 明确假设，不确定时询问，不靠猜测推进。
- 存在歧义时列出多种解释，不默默选定单一方案。
- 有明显更简单的做法时，直接指出优化思路。
- 发现代码矛盾或逻辑不一致时，暂停受影响的工作，说明矛盾并请求澄清。

### 1.2 简洁优先

- 用最少的代码解决问题，避免冗余实现。
- 不为一次性需求创建抽象层或复杂架构。
- 不为“未来可能用到”的场景增加扩展性或配置项。
- 当前任务涉及的代码可大幅精简时，主动简化；不借此重构无关模块。
- 以资深工程师视角检查复杂度，过于复杂的实现应先简化再交付。

### 1.3 精准修改

- 仅修改与当前任务直接相关的内容。
- 不顺手优化相邻代码、注释或排版。
- 不重构原本正常且与当前任务无关的模块。
- 匹配项目现有代码风格，保留原有编码习惯。
- 删除因本次修改产生的无效导入和废弃变量。
- 原有死代码或冗余内容仅作文字提醒，不擅自删除。

### 1.4 目标驱动执行

- 执行前定义清晰、可验证的成功标准。
- 修复 Bug：先用用例复现，再修复至用例通过。
- 新增校验：覆盖异常输入，确保相关测试通过。
- 代码重构：完成后运行原有相关测试，确认行为保持正确。
- 多步骤复杂任务：先给出简短计划，为每一步标明验证方式。

## 2. 不可违反的稳定约束

- **DLL 自包含交付**：玩家产物为 `dist/version.dll` 引导＋`dist/freight/freight.dll` 主体；用户已明确允许双DLL，替代旧单DLL限制。引导留EXE同目录，主体及 `freight.ini`／`freight.log`／缓存集中到 `binaries\freight\`，不依赖playset／外部Mod／Python／手工资源；配置与缓存原子写入。
- **原版 UI 是正式界面**：市场/地区/商品面板承担正式交互；ImGui 仅调试、默认关闭，正式箭头不得依赖其可见状态。
- **优先复用原版函数**：搜索、分类、排序及其他原版已有能力尽量复用当前版本已验证入口；只有原版确实缺少所需能力时才补最小实现，记录差异与验证，不并行重写原版流程。
- **数据语义**：`targetImport`、`allocatedImport`、`unmetDemand`、区段吞吐、工作量与费用分开。按MASTER §2.3.1–2.3.2，国际原版贸易保留1e5整数，国内物流用最终模型分配并明确标注；两项独立保存，展示可相加，不能冒充吨数／运输船容量。国内目标／满足／未满足分开，线宽不使用未满足需求。缺几何与缺数量分开；未知不伪装成零或测试值。
- **Freight 中转商品**：必须以 Freight 作为中转商品计算物流需求，遵循 `MASTER_PLAN.md` §4.5；目标／已满足／未满足分开，保留来源商品贡献。R67 已实现需求核算（默认系数 1 是模型假设）；一期不另建供给池、不注册游戏商品；满足量来自最终货流，尚需端到端实机验收。
- **地图身份**：运行州ID、引擎州定义ID与派生地图ID分开；正式关系读取当前运行对象、定义、省份及样条，禁止写死ID→key地图表。改图须绑定当前实际数据版本，不能将原版图冒充兼容。
- **禁止反馈**：过滤、悬停、换色、显示/隐藏只改视图，不得修改价格、MAPI、订单、建筑、人口需求、分配结果或市场分母。
- **线程与所有权**：渲染回调不扫描州/建筑数据库、不跑分配/Dijkstra、不等模拟锁、不执行 GUI 脚本效果。worker 不持 engine 指针。异步结果必须带 `world`/`revision`/generation，过期丢弃。
- **原生安全**：未验证 ABI/生命周期保持 disabled；不跨构建复用地址、不按“第一个候选”取值、不全局改共享 vtable、不重构造已注册对象。
- **单写者与分支协调**：同一迁移任务同一时间只允许一个活动工作区／分支。开工前运行 `python tools\agent_preflight.py`，用 `--claim "任务" --owner <会话ID> --round R2xx` 登记共享 claim；收工 `--release`。发现第二个工作区已有未提交迁移改动时停止，按 [docs/transport/COORDINATION.md](docs/transport/COORDINATION.md) 快照、三路比对、按功能取优；禁止两套实现并行推进到交付，禁止直接合并两套 R 号或候选哈希。
- **不破坏用户工作**：仓库已使用 Git，用户要求完成更新后提交并推送现有远程；核对测试与范围，不改写远程历史。改源码前备份到 `E:\Code\v3-analysis\checkpoints\<日期>-<主题>`，不盲目恢复旧 `dist`。
- **证据层级**：静态候选 → helper/模型测试 → native harness → DLL 加载 → 原生提交 → 实机可见 → 全交互/生命周期/性能。前一层不证明后一层。
- **不越权操作**：游戏运行且文件被锁时不强替 DLL，不自动结束可能含未保存进度的游戏；部署/卸载只按 `deploy.ps1`。
- **文件编码**：Markdown 必须 UTF-8（无 BOM）。不要用 PowerShell `Get-Content | Set-Content` 批量改写中文文档（R6 曾因此损坏编码）；用编辑工具或显式 UTF-8 写入。

## 3. 构建、测试、部署

```powershell
python tools\agent_preflight.py                                    # 开工前预检；收工 --release
powershell -ExecutionPolicy Bypass -File native\build.ps1          # dist\version.dll + dist\freight\freight.dll
powershell -ExecutionPolicy Bypass -File native\build_tests.ps1    # native\build\tests
python -B tools\verify_embedded.py
python -B tools\test_native_gui.py
powershell -ExecutionPolicy Bypass -File tools\test_native_gui_runtime.ps1
python -B tools\test_flow_gpu.py
powershell -ExecutionPolicy Bypass -File tools\selftest.ps1        # game closed
powershell -ExecutionPolicy Bypass -File tools\test_debug_default.ps1
powershell -ExecutionPolicy Bypass -File deploy.ps1                # game closed
powershell -ExecutionPolicy Bypass -File deploy.ps1 -Uninstall
```

工具链固定 `E:\Program\C Compiler\MSYS2\ucrt64\bin`。删除 `native\build` 后需完整构建；`-ResourcesOnly` 不重建缺失对象。

**环境依赖**：完整测试入口要求**没有** `victoria3` 进程——`build_tests.ps1` 末尾的 `test_freight_deploy.py` 会走 `deploy.ps1`，而 `deploy.ps1` 的守卫在有游戏进程时以 `Close Victoria 3 before deploying.` 拒绝（R84 实测）。游戏运行时该步失败属预期拒绝，不是回归；此时应记录为环境阻塞，等游戏关闭后重跑。

运行时只读诊断（需游戏正在运行，不属统一测试入口、因此不接入 `build_tests.ps1`）：

```powershell
# 复用 market_panel.h / state_panel.h 的生产解码逻辑，PROCESS_VM_READ 只读；不 attach、不暂停、不写
$env:Path = "E:\Program\C Compiler\MSYS2\ucrt64\bin;$env:Path"
g++ -std=c++20 -O2 -static -municode -DNOMINMAX -DUNICODE -D_UNICODE -Inative\src -o native\build\tests\live_panel_probe.exe native\test\live_panel_probe.cpp
native\build\tests\live_panel_probe.exe
```

R95 VFS只读探针（先核对PROFILE中的精确EXE hash；同样不接无游戏测试入口）：

```powershell
g++ -std=c++20 -O2 -static -DNOMINMAX -DUNICODE -D_UNICODE -Inative\src -o native\build\tests\live_vfs_probe.exe native\test\live_vfs_probe.cpp
native\build\tests\live_vfs_probe.exe <Victoria3进程ID>
```

该探针通过游戏窗口线程TLS读取当前冻结挂载链，只复制目录/ZIP与顺序；不调用引擎getter，不代表已解析replace_path或兼容改图。

注意：`g++` 在工具链不在 `PATH` 时会以退出码 1 且**无任何诊断输出**的方式失败，先设置 `PATH`。

R228 只读港内核对（游戏由用户手动进入新档并暂停；实际商品数须与当前目录一致）：

```powershell
# 与普通 live_shipping_probe 相同编译依赖；程序不 attach、不写、不调用原版 getter。
native\build\tests\live_shipping_probe.exe <Victoria3进程ID> --world-trade <当前商品数> --port-access
```

完整编译命令和证据见本轮检查点；此探针不是完整快照原子性或正式绘制验收。

## 4. 目录边界

| 路径 | 说明 |
|---|---|
| `native/src` | 原生实现；`game.cpp`/`offsets.h` 使用精确版本门控；当前交付绑定 1.15.0（R200–R219） |
| `native/test` | 回归测试；新增测试必须接入 `native/build_tests.ps1` |
| `data/` | 五份派生地图数据，参与 DLL 资源打包，**不可删** |
| `native/assets`、`native/third_party` | GUI/shader 片段与 ImGui/MinHook，**不可删** |
| `tools/` | 离线分析/生成/验证脚本，不是运行时依赖 |
| `dist/version.dll`、`dist/freight/freight.dll` | 引导与主体，唯一两份玩家DLL |
| `native/build/` | 可再生中间产物（已忽略） |
| `docs/transport/` | 状态/任务/决策/阻塞/能力/验收/性能/知识记录 |

## 5. 当前状态与恢复入口

当前产品主体`27cefa0a…`／52,197,771B（R262 构建，安装＝dist）＋引导`71dae36d…`／779,879B；R263–R269 只加日志行与测试，R271–R273 仅 Ghidra 研究、未改源码／未重建。G1／G4／G5／G8 部署矩阵与 G7 前置项已在当前构建关闭，G2 仍 open。R276 冻结核验：`dist`＝安装（两份 DLL 哈希与长度逐项相同），且 `d0f1b6f..HEAD` 对 `native/src`／`native/test`／`native/assets`／`data`／`tools` 无差异——代码树就是产出该主体的树。海端：新档日更新 38 部分、周更新 39 部分（R258）；38 条 partial 的判据是端落到共享地区海军节点（`regional_trade_geometry.h` 的 `regionalPort`），非"无见证"；R275 实测 38/38 至少一端本州无 `portTransfers` 记录。**R273 已撤回 R272 全部静态锚点**（五字节表实为格式字符串、`FUN_1428fbc00` 为 libc++ string 元素循环的尾部、两个 map/getter 地址无函数边界），原版岸线选择仍未识别，剩余可行路线是实机只读探针。无游戏、goal ACTIVE，优先读 [STATUS.md](docs/transport/STATUS.md) R273。


交付说明（文件哈希、构建/部署、界面操作、模型口径、限制、回滚）见 [docs/DELIVERY.md](docs/DELIVERY.md)；回滚副本与最终日志在 `E:\Code\v3-analysis\checkpoints\2026-11-06-final-delivery`。


更早的 §5 状态快照与 §6 轮次记录（R83–R261）已归档到 [AGENTS-history-through-R261.md](docs/transport/history/AGENTS-history-through-R261.md)；本文件只保留当前状态与最近数轮，逐轮细节仍以 `docs/transport/STATUS.md` 为准。

## 6. 进度日志（简要记录，最新在上）

### 2026-11-10 — R274：AGENTS.md 精简 326 KB → 21 KB，R83–R261 原样归档

本文件 1739 行/326,647 B 每会话占约 97k tokens。§5 中截至 R224 的过期快照与 §6 中 R260 及更早逐轮记录整体搬到 `docs/transport/history/AGENTS-history-through-R261.md`（1558 行/306,676 B），按行数守恒校验（134+56+82+1467=1739）、UTF-8 无 BOM、CRLF 保持，未改写任何结论。本文件只留 §1–§4＋当前状态＋R263–R273。仅文档改动，未改源码／未重建／未部署，产品仍是 `27cefa0a…`。

### 2026-11-10 — R277：另一存档载入自动化尝试（9 场实机，负结果，未改源码）

冻结构建 `27cefa0a…` 上 9 场短会话全部 exit 0、五输入逐字节不变。结论：PostMessage 的 ESC 与真实 ESC 都无效（键盘通道不通）；地图拾取吃 PostMessage（州面板被点开）；前端**按钮**吃真实光标（载入游戏可点开对话框，坐标 (660,811)）；但**存档列表行**对 PostMessage 与真实光标单击/双击都零反应（截图逐像素相同、`phase` 恒 0、无 retirement、`continue_game.json` 仍 `autosave_exit`）。故 G6 的"另一存档载入／返回菜单"在本环境不可自动化，需人工或未验证 ABI 的控件级路径。证据 checkpoint2026-11-10-r277-g6-saves。

### 2026-11-10 — R276：G7/G8 冻结事实核实（未改源码／未重建／未部署）

实测 `dist` 与游戏安装目录两份 DLL 哈希＋长度逐项相同（引导 `71dae36d…`／779,879 B、主体 `27cefa0a…`／52,197,771 B，安装＝dist）；`git diff d0f1b6f..HEAD -- native/src native/test native/assets data tools` 为空 → 当前树就是产出该主体的树，冻结可由 HEAD 复现。已具备证据：72 PASS（R262）、A25 三对＋20 轮（R264）、部署矩阵（R266）、改图五夹具（R267）、生命周期（R268）、G5（R269）、G1（R262/R263）。**不能并入"全绿"**：G2 38 partial（定性见 R275）、G6 另一存档（需人工）、A01–A27 同构建全矩阵（需实机）、A24 真设备、Gora UI + 兼容。故冻结候选确定，但 G7/G8 统一验收不宣称通过。

### 2026-11-10 — R275：38 条 partial 全部归入文档化 peer 回退类（日志复核，未改源码）

用既有 `freight.log` 末条分配行复核，无需运行游戏：38 条 lane id 全唯一，**两端都有 `portTransfers` 记录的 0 条**（起点有/终点无 35、起点无/终点有 1、两端都无 2），即 38/38 至少一端本州无港口接入，全部落在 `native_trade_geometry.h:61` 的 `!found` 回退分支（取同 region+geo 的 peer 海锚→`regionalPort`→`complete=false`）。这只证明我们的 partial 判据与该分支一致，**不提供原版实际选中的岸线**；`seaAnchor` 命中/ambiguous 未进日志。G2 保持 open。

### 2026-11-10 — R273：R272 岸线选择算法全部撤回，G2 仍 open（仅研究，未改源码）

逐项复核 R272 的五个静态锚点，全部不成立并撤回：①`UNK_14464f590` 32 字节实为 ASCII 格式串`" '{}' is already used. Consider "`，不是五字节表；②`FUN_1428fbc00` 只是 `FUN_1428fba80` 的尾部（`1428fbc31 ADD RSP,0xa8/RET`），后者反编译为 libc++ `std::string` 元素区间赋值/析构循环（`*puVar15=0xf` 为 SSO 容量、`FUN_14417d850` 为 operator delete、`FUN_1441a267c` 为 noreturn throw），所谓"0x38 步长见证 map"即 STL 元素尺寸＝2×0x18 string+8B；③`FUN_140df87b0` 无函数边界；④`FUN_142883cf0` 无函数边界（"状态记录 getter"不成立）；⑤我自己对日志 `P`/`-` 的解读也错——它是"该州有 `portTransfers` 记录"（`freight_engine.cpp:338-341`），不是"有无见证"。另更正 R258 地址表：`148bdcaf0`/`149eb5400` 超出镜像最大地址 `145dc57ff`，`140dcb6b0` 为中函数内地址，故"3 失败"实为 1 中函数+2 越界。

仍成立的事实：`FUN_142883e70` 是 4 叉堆 sift-up（节点 0x20B、子 `i*4+1..4`、父 `(i+(sign&3))>>2`）＋`vector<uint32_t>` 约1.5×扩容，属开放表/寻路优先队列，**不是岸线选择**；岸锚派生 `((geo*100)&0x1fffffff)|0x800000` 与 `regionalPort`/`unavailable` 语义由本方源码确认（`native_trade_geometry.h:68,103-104,95,181,178`），不依赖 Ghidra。结论：原版岸线选择代码仍未识别，38 条 partial 仍只是"端落到共享地区海军节点"的模型判据。剩余可行路线是实机只读探针（38 条 lane 逐端对 `portTransfers`/地区配置），需游戏运行；当晚已有三次未定因自退，未在无人值守下启动。G2 保持 open，不报通过。Ghidra 库仅删除了 R271 自建的伪片段，可用 `create_function` 复现。证据 checkpoint2026-11-10-r270-g2-ghidra（r273b.txt/r273c.py/r273c.txt）。

### 2026-11-10 — R271：G2 链边界重建 9/12，反编译存档

R258 已知 12 地址在当前库批量 create_function：9 成功（含 FUN_142883e70 429B 最大逻辑块），3 失败（中函数/跳板）。Ghidra 自动分析未达贸易子系统（12 地址全无函数无 xref），边界重建为溯源前置已完成；反编译全文存 chain-decomp.c。下一步追岸线选择逻辑＋解析 3 失败地址。证据 checkpoint2026-11-10-r270-g2-ghidra。

### 2026-11-10 — R272：G2 岸线选择算法识别——锚点按州×100 算法派生＋见证 map 查找【R273 已全部撤回】

结合并行 R258 优质反编译与我重建的 FUN_142883e70 验证：状态记录经 machine+0x120+0x2db8 容器按 id 提取；遍历 UNK_14464f590 五字节表谓词；**岸锚=state×100&掩码|0x800000 算法派生**；双端排序打包 64 位 key 查 0x38 步长见证 map 取最优。partial=map 无对应 (州×100) 对见证——38 条逐端判定路径明确（无硬编码 ID 表，符合约束）。下一步：重建 FUN_1428fbc00 边界、dump 五字节表与 map 内容对照 partial。证据 checkpoint2026-11-10-r270-g2-ghidra。

### 2026-11-10 — R269：G5 完整交互矩阵通过并关闭（27cefa0a…）

桥/后端/握手全链诊断健康（8089 监听、ensure_backend OK、101 tools 注册），失败为本会话启动时的一次性 MCP 连接且客户端不可重连——新建 `ghidra_call.py` 直连助手（子进程 stdio JSON-RPC 驱动 launcher_dsh.py），实测冻结库 program_info 正常。G2 溯源解锁；注意 R258 地址是 Capstone 回查地址非当前库函数起始。证据 checkpoint2026-11-10-r270-g2-ghidra。

### 2026-11-10 — R269：G5 完整交互矩阵通过并关闭（27cefa0a…）

R256 坐标序列＋ordered_ui 逐步断言：分类 41→9→41、"谷物"搜索 1 行、空结果 0 行、清空恢复 41、hover 进出、面板 X 关闭→F4 重开 41 行——全过；截图底部账本 30021.15/10520.41/19500.74 与 R262 日志行逐位互证。会话退出 0、五输入一致、content 恢复用户状态。G5 关闭；G2 待桥、G6 另一存档待用户、G7/G8 冻结终验收尾。证据 checkpoint2026-11-10-r269-g5-ui。

### 2026-11-10 — R268：生命周期同档重绑通过（27cefa0a…）；另一存档自动化受阻

Session A：同档重载池恢复 241 槽、缩放往返/最小化零额外退休、退出 0、五输入一致——同档/缩放/最小化在 27cefa0a 关闭（R259 绑 0ea4caf3，其后仅加日志行，本轮补严格同构建）。Session B 另一存档载入对话框盲点击三轮不可靠（误触返回游戏两次），止损关闭（退出 0、五输入一致）——另一存档/返回菜单保留 R247 旧构建证据，需控件名解析级点击或人工协助。遗留测试夹具两枚已守卫清除。下一步 G2（待桥）/G5/G7-G8。证据 checkpoint2026-11-09-r254-lifecycle。

### 2026-11-10 — R267：G4 完整关闭——province/spline 单会话补齐（27cefa0a…）

≤5 分钟单会话规则分两场：province（省交换 id=140 geo=155 valid=1 绑定＋41 行＋完整批次）、spline（X+10 delta=0 精确绑定＋41 行＋完整批次）；两场退出 0、五输入一致、零自退——短会话策略零中断。**G4 五夹具在 27cefa0a 全部通过关闭**；Gora UI + 兼容问题仍为独立待决项（BLOCKERS R262）。下一步 G2（待 Ghidra 桥）或 G5/G6。证据 checkpoint2026-11-10-r265-map-fixtures。

### 2026-11-10 — R266：部署矩阵重绑通过（27cefa0a…，G8 预备项关闭）

四用例各 ~70 秒短会话：缺失→126/损坏→193/卸载→无条目/重装→0，安装=dist、五输入一致、无残留进程（loader 行为不受用户 UI Mod 影响，已注记）。G8 部署矩阵预备项在当前构建关闭；G7/G8 其余项待冻结终验。证据 checkpoint2026-11-10-r266-deploy-matrix。

### 2026-11-10 — R265 attempt 2：改图 3/5 通过后第三次自退——无人长会话停止

replace/directory/zip 三夹具完整通过（geo=901 绑定、41 行物流、206 批次、退出 0、输入一致）；province 探针期间第三次游戏自退 0（matches=2 为退出期读数不作结论）、spline 未轮到。按规则停止长时无人会话，待用户确认机器占用与 Ghidra 桥重启。mod 目录遗留夹具两枚已守卫清除（本轮+并行 R254 遗留），ironroad_arrow 目录未动。证据 checkpoint2026-11-10-r265-map-fixtures。

### 2026-11-10 — R265 中断：改图首轮探针通过后遭游戏自退（待用户确认机器占用）

R254 管线改靶 27cefa0a 后首轮 replace：载入/探针（geo=901 绑定正确）/快照交接见证全过，随后游戏自退 0（今晚第二次，无崩溃事件记录）——preserve-live 守卫中止并留证；残留夹具已清、content 恢复用户状态。两次自退原因待用户确认（机器占用或计划任务）；G2 另需重启 Ghidra MCP 桥。期间不再盲目起长会话。

### 2026-11-10 — R264：当前构建 A25 三对与 20 轮重绑通过（27cefa0a…）

六臂 P95 +0.379/+0.430/+0.394 ms（≤2）、模拟零降幅、20 轮 u0/r206/p206、内存平台、峰 529µs、暂停期重开零捕获。过程修正：8 MB 日志截断丢失六臂数据（分析改会话后即时执行＋日志副本）、漏暂存基线 content 一轮（无效已作废）、一次游戏自退（四臂留档）。会话退出 0、五输入一致、content 恢复用户状态。当前构建证据链＝官方入口＋A25＋20 轮＋G1 矩阵；改图矩阵/另一存档/部署矩阵/G2/A01–A27 仍待。证据 checkpoint2026-11-09-r261-perf/PERF-ARMS.md。

### 2026-11-09 — R263：G1 关闭——州作用域与第二市场实机对账通过

R262 日志行机制补两例：州 5（伦敦周边）作用域 29 商品 total=2833.96/1751.38/1082.58 maxdiff 0.02；market 2（经巴黎州面板市场链接进入）26 商品 total=2120.46/0.00/2120.46 maxdiff 0.03（满足=0 为模型结果，未满足全保留）。逐行守恒全成立，会话退出 0、五输入一致、content 恢复用户状态。**G1 三作用域实机矩阵完成并关闭**；下一步 G2 海端 partial（Ghidra 桥需重连）。证据 checkpoint2026-11-09-r262-g1-matrix。

### 2026-11-09 — R262：G1 真档逐商品需求账矩阵通过（27cefa0a…），发现 Gora UI + 兼容问题

截图通道在真档 41 行不可靠后改为日志级对账：发布路径新增基础作用域需求账日志行（FormatFreightDemandLedger 纯函数＋发射＋载荷断言先 PASS）。官方 72 PASS，构建 27cefa0a…（52,197,771 B）部署安装=dist。实机会话：market 0 需求账 41/41 商品、逐行求和与总账差 ≤0.03、逐行守恒全成立、grain 与截图 tooltip 互证；会话退出 0、五输入一致、content 恢复用户 Gora UI + 状态。**新发现**：Gora UI + UI Mod 使铁道路物流页签消失（owner 不激活），已记 BLOCKERS（R262）。G1 剩余：州作用域与第二市场实机例。证据 checkpoint2026-11-09-r262-g1-matrix。


R261 及更早的逐轮记录已归档（原样搬运、未改写）：[AGENTS-history-through-R261.md](docs/transport/history/AGENTS-history-through-R261.md)。
