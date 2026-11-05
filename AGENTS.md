# AGENTS.md — 本仓库的稳定约束、构建入口与工作日志

> 用途：给下一个接手的编码 AI 的最短入口。**不替代** `MASTER_PLAN.md` 与四份原始消息文档。
> 每次完成一个有意义的工作单元，必须更新本文件第 6 节「进度日志」以及 `docs/transport/STATUS.md`。
> 仓库实际文件为 `AGENTS.md`；用户所称 agent.md 的工作记录统一维护于本文件，避免另建重复日志。

## 1. 阅读顺序与裁决

1. `MASTER_PLAN.md`（总任务书，含第 2.4 节按主题重读要求）
2. `00_START_HERE.md`、`01_IMPLEMENTATION_PROMPT.md`、`02_NATIVE_DISCOVERY.md`、`03_TESTING_AND_TROUBLESHOOTING.md`（**完整读**）
3. 本文件
4. `docs/transport/STATUS.md` → `TASKS.md` → `BLOCKERS.md` → `DECISIONS.md` → `WORKLOG.md`
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
- **数据语义**：`targetImport`、`allocatedImport`、`unmetDemand`、区段吞吐、工作量与费用分开。按MASTER §2.3.1–2.3.2，国际原版贸易保留1e5整数，国内物流用最终模型分配并明确标注；两项独立保存，展示可相加，不能冒充吨数／运输船容量。国内目标／满足／未满足分开，线宽不使用未满足需求。缺几何与缺数量分开；未知不伪装成零或测试值。
- **Freight 中转商品**：必须以 Freight 作为中转商品计算物流需求，遵循 `MASTER_PLAN.md` §4.5；目标／已满足／未满足分开，保留来源商品贡献。R67 已实现需求核算（默认系数 1 是模型假设）；一期不另建供给池、不注册游戏商品；满足量来自最终货流，尚需端到端实机验收。
- **地图身份**：运行州ID、引擎州定义ID与派生地图ID分开；正式关系读取当前运行对象、定义、省份及样条，禁止写死ID→key地图表。改图须绑定当前实际数据版本，不能将原版图冒充兼容。
- **禁止反馈**：过滤、悬停、换色、显示/隐藏只改视图，不得修改价格、MAPI、订单、建筑、人口需求、分配结果或市场分母。
- **线程与所有权**：渲染回调不扫描州/建筑数据库、不跑分配/Dijkstra、不等模拟锁、不执行 GUI 脚本效果。worker 不持 engine 指针。异步结果必须带 `world`/`revision`/generation，过期丢弃。
- **原生安全**：未验证 ABI/生命周期保持 disabled；不跨构建复用地址、不按“第一个候选”取值、不全局改共享 vtable、不重构造已注册对象。
- **不破坏用户工作**：仓库已使用 Git，用户要求完成更新后提交并推送现有远程；核对测试与范围，不改写远程历史。改源码前备份到 `E:\Code\v3-analysis\checkpoints\<日期>-<主题>`，不盲目恢复旧 `dist`。
- **证据层级**：静态候选 → helper/模型测试 → native harness → DLL 加载 → 原生提交 → 实机可见 → 全交互/生命周期/性能。前一层不证明后一层。
- **不越权操作**：游戏运行且文件被锁时不强替 DLL，不自动结束可能含未保存进度的游戏；部署/卸载只按 `deploy.ps1`。
- **文件编码**：Markdown 必须 UTF-8（无 BOM）。不要用 PowerShell `Get-Content | Set-Content` 批量改写中文文档（R6 曾因此损坏编码）；用编辑工具或显式 UTF-8 写入。

## 3. 构建、测试、部署

```powershell
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

## 4. 目录边界

| 路径 | 说明 |
|---|---|
| `native/src` | 原生实现；`game.cpp`/`offsets.h` 绑定已验证的 1.13.11 ABI |
| `native/test` | 回归测试；新增测试必须接入 `native/build_tests.ps1` |
| `data/` | 五份派生地图数据，参与 DLL 资源打包，**不可删** |
| `native/assets`、`native/third_party` | GUI/shader 片段与 ImGui/MinHook，**不可删** |
| `tools/` | 离线分析/生成/验证脚本，不是运行时依赖 |
| `dist/version.dll`、`dist/freight/freight.dll` | 引导与主体，唯一两份玩家DLL |
| `native/build/` | 可再生中间产物（已忽略） |
| `docs/transport/` | 状态/任务/决策/阻塞/能力/验收/性能/知识记录 |

## 5. 当前状态与恢复入口

截至R207：1.15.0适配继续。官方等价原生入口35/35 PASS；稳定视图65秒零新增上传、20轮重开全部`uploads=0 reused=185`、池恒346、`peak_frame_us=685`、`overrun_frames=0`；生命周期修复1.15图形身份偏移（`0x0592d5c8`→`0x05c080b8`），重载world2池恢复、缩放往返无旧帧。部署矩阵：主体缺失126／损坏193／整套卸载无loader／重装0，游戏均存活且安装＝dist、存档与content_load字节保持。改图身份：州geo 164→901与同key样条点X+10均被运行时绑定，正式物流页重建。海路端点：seaOnly缺港口端回退到原版海段链末端，两端均缺港口时按海段key作者方向判定源／终点，`unavailableSea 35→1`、`partialSea 4→38`、paths 185→187，页面数值不变；剩余1条unavailable（611→608 MINDANAO→NORTH_BORNEO）与38条partial的未知端口端保留。当前构建同构建三对性能：P95增加0.593／0.518／0.671 ms（≤2），模拟速度最大降幅0.46%（≤5%），窗口61.5–62.6秒。当前主体`2028e03b…`／50,025,206B、引导`c8020412…`／779,879B，安装＝dist（沙箱禁PowerShell，未走deploy.ps1，需在可用环境补跑）。改图矩阵：州geo→901、同key样条点+10、ZIP、省份归属（模板+0xC8 id/geo 139/154→140/155）、replace_path整目录替换（省略02差分）均通过；设备重建、三处海路末端像素复看与A01–A27未完成，详见STATUS R207–R211。

截至2026-11-05：按用户要求在保存退出后经deploy.ps1卸载引导和主体；配置、日志、缓存及可编辑资源逐项哈希保持，卸载备份已留。游戏更新后的ABI未验证，目标保持暂停，R194未完成源码保留且未部署。当前实际安装已卸载，dist仍保留，详见STATUS。

截至R193：动态typed物理港口／真实市场输入与1份新本国局部原版查询实机核对，独立结果释放及原输入／选择保持；首版state+48市场误读已纠正，最终完整59 PASS／资源／默认debug通过。f328f684…／71dae36d…部署实载，自有会话正常退出0／五输入保持／probe_access恢复0／无游戏、不push／GUI手动。未接正式几何／权限，三末端未修，下一步真实lane／几何／版本交接，完整goal ACTIVE，见STATUS。

截至R192：06e7272e…／71dae36d…完整58 PASS／资源／默认debug通过并部署；103次同参原版额外查询／独立buffer释放实机成功，自然4010／103port／17栈保持。自有32164正常退出0／五输入保持／probe_access恢复0／无游戏、不push／GUI手动。三新接入请求与正式权限／完整目标仍待，见STATUS。

截至R191：f42f9e6b…／71dae36d…完整58 PASS／资源／默认debug通过并部署实载；17次重建实机栈绑定已有CD2BC1完成作用域，7BEB90为yield而非join。自有41000正常退出0／五输入保持／probe_access恢复0／无游戏、不push／GUI手动。三末端与真实任务join／权限生产方继续，见STATUS。

截至R190：30596be5…／71dae36d…完整58 PASS／资源／默认debug通过，部署实载；103次原版结果buffer转移及TLS heap实机一致，4010查询全覆盖。自有28600正常退出0／五输入保持／probe_access恢复0／无游戏、不push／GUI手动。三末端与真实scope／任务join仍未修，详见STATUS。

截至R189：原版结果转移及TLS分配器静态核对，21项隔离机器码用例PASS；产品未改，仍950d3f32…／71dae36d…，R188同产品58 PASS。未启动游戏／五输入保持／probe_access0／不push。三末端和真实调用窗口未修；详见STATUS。

截至R188：950d3f32…／71dae36d…完整58 PASS／资源／默认debug通过并部署；4010自然调用全部复制，三局部对均无同country请求，末端未修。自有95000因用户确认停电非正常关机中断，退出验收未完成；五输入保持／probe_access恢复0／无游戏，不push／GUI手动。详见STATUS。

截至R187：新增仅测试使用的原生海区基础缓存复制器，统一58 PASS／资源检查通过；产品仍719a2d83…／71dae36d…，未重建部署。自有3236同世界只读对照：128份非同省最终查询中83同缓存、45不同（30缓存空／15更长），不能将基础缓存作最终路线。三处缺口有局部缓存条目但未完成商业选择／scope见证，仍未修。正常退出／五输入保持／probe_access恢复0，不push／GUI手动，详见STATUS。

截至R186：完整州选择与同次贸易返回自有复制已实机核对，103国内查询／17次非空重建全覆盖，93当前lane各有唯一完整州值匹配且原版构造反转两端；不是正式身份cookie／epoch。719a2d83…／71dae36d…完整58 PASS、部署实载；自有93548正常退出／五输入保持／probe_access恢复0。三海路末端、真实通行生产方及全目标仍待，不push／GUI手动，见STATUS。

截至R185：同市场分属州按当前原生主陆地枢纽聚合装卸，真实州内道路进入货流。完整58 PASS，08e1e433…／71dae36d…已部署实载；库尔努尔缺段0→21笔与生产view／ribbon回放通过，用户已实机确认本次缺段修复。自有会话均正常退出／五输入保持／probe_access0；随后新85984保留，不操作或替换。不push及其他完整任务继续，见STATUS。

截至R184：新自然贸易请求／同次mode1分类已实机复制，当前b0a1c9c3…／50679724B＋71dae36d…已构建、统一58 PASS／零FAIL、embedded／默认debug零顶点并部署实载。最终93928实录32普通＋96国内查询，93当前lane与构造记录一致；33份同值歧义仍需原版完整州选择输出，103次国内查询仅记录96。两自有暂停会话正常退出0／五输入保持／probe_access恢复0，无游戏、不push。无正式新路线／权限消费，三末端未修，详见STATUS。

截至R183：原版贸易搜索实际使用第7槽选项，旧五参反编译不足以调用。11组原版包装／候选／限制对机器码及11份输出回放通过，新增仅测试使用的结果复制器；统一58 PASS／零FAIL、embedded PASS。未重建／部署／启动游戏／push，产品仍R182 b85b773f…／71dae36d…、dist＝安装、五输入保持／probe_access0。主动商业查询及三处末端仍未接，详见STATUS。

截至R182：自然舰队搜索诊断已接默认关闭probe_access，完整构建／统一58 PASS／embedded／默认debug零顶点通过。当前b85b773f…／50409230B＋71dae36d…已部署并实载；自有32488暂停只读核对两份原版成功结果及当前连接后正常退出0／五输入保持，原probe_access0已恢复，无游戏。无新正式路径或权限消费，三处末端未修，仍不push。详见STATUS。

截至R181：新路径结果复制器仅测试使用；原版重建7组及其原始输出回放通过，完整统一入口58 PASS／零FAIL、embedded PASS。产品仍R179 f53f81d6…／71dae36d…、dist＝安装，本轮未启动游戏／未构建部署／不push。原版搜索成功值／scope／权限／所有权仍待，三海路末端未修。详见STATUS。

截至R180：产品仍R179 f53f81d6…／71dae36d…，同产品历史统一58 PASS；港口出口动态普查完成，624份绑定中13份涉及6地区错配，当前三条部分海路均在其中。原生舰队寻路仅静态候选、未调用／未启用，末端未修复。自有7468只读暂停核对后正常退出0／五输入保持，无游戏。仍不push，详见STATUS。

截至R179：f53f81d6…／71dae36d…完整构建及统一58 PASS，已部署实载并正常退出、五输入保持。国内先按真实物理节点分配，断路转未满足；38838已知完整接缝broken0，余40次为3条部分海路，非海路不完整0。当前无游戏、不push，通行正式收集及最终矩阵继续。详见STATUS。


截至R178：当前fc674422…／71dae36d…完整构建及统一58 PASS，已部署实载并正常退出，五输入保持。同市场跨地区真实道路补转接，69组／839次缺段修复，总1395→556；已知完整58810接缝broken0，剩余转接和物理分配闭环继续。当前无游戏、不push。详见STATUS。


截至R177：当前732a7e5e…／71dae36d…完整构建／统一58 PASS／已部署实载并正常退出，五输入保持。原版372条无key港内路径进入模型／ribbon，伦敦使用564次控制点连续(broken0)，缺转接2642→1395次；其他缺段和最终矩阵继续。当前无游戏、不push。详见STATUS。

截至R176：已在当前原版运行对象找到372条港内连接(kind5／keyMAX／自带path)，伦敦8390108→1503真实关系和坐标已证；尚未接生产快照／绘图，断点未修复。产品仍R175 e20043d3…／71dae36d…，自有只读会话正常退出／五输入保持，无游戏、不push。详见STATUS。

截至R175：当前e20043d3…／71dae36d…完整构建及统一58 PASS，已部署实载并正常退出，五输入保持。同地区同市场分属州转接范围已修正，实际重放缺转接2936→2642次；仍有剩余断点及伦敦海陆锚点未解，不称全修复。当前无游戏、不push。详见STATUS。

截至R174：当前9a795e01…／71dae36d…完整构建及统一58 PASS、已安装实载，测试会话正常退出且五输入保持。州内道路已加入转接；伦敦海军与陆路港口锚点存在实际位置差，仍未连接，用户报告尚未全修复。当前无游戏、不push。详见STATUS。

截至R173：用户已恢复正常启动与测试授权，旧离线／不测试限制已覆盖；不push继续。R169–R172源码完整构建及统一58 PASS，主体c82435da…／引导71dae36d…已部署并实载。39984会话仍运行，出现来源未明日期／输入变化后已停止自动操作，勿强关／替换。正式完整通行采集仍未接线，详见STATUS。

截至R172：逐航线批次代数已接入会话快照／地图帧／统计检查及worker回复发布；正式原生收集器与完整GUI／原生提交时序仍待。R169–R172全部仅本地离线修改，按用户要求未构建／测试／部署／push，无新增PASS。详见STATUS。

截至R171：批次见证改为共享自有atomic代数，已增加邮箱发布／取出及UI交接前复核；原生生产方和已显示帧撤回仍未接通。R169–R171均按用户要求只本地离线修改，未构建／测试／部署／push，不能继承旧产物PASS。详见STATUS。

截至R170：自有逐航线批次已接入可选快照采集入口并增加返回前代数复核，原生生产方与发布期失效未闭环。按用户要求仅本地离线修改，未构建／测试／部署／push；源码新增内容没有测试结论。详见STATUS。

截至R169：本地源码新增逐航线权限交接／消费，按用户要求未构建、未测试、未部署、不push；现有DLL仍为R167历史产物。当前仅允许离线代码／文档工作，暂不恢复测试或游戏操作，详见STATUS。

截至R150：候选dist 23914d65…离线58 PASS、未部署；安装仍178e039d…；权限回调未绑定，原版真实权限与港口末端待修复。

截至R149：候选dist 00e333a6…离线58 PASS、未部署；安装仍178e039d…；快照权限回调默认关闭，原版真实权限与港口末端待修复。

截至R148：候选dist 0a104ab4…离线58 PASS、未部署；安装仍178e039d…；采集桥未绑定真实函数，权限与港口末端仍待修复。

截至R147：主体178e039d…已构建、安装及实载；国家通行结果的模型消费／缓存回归完成，原版采集尚未启用、实际权限与港口末端仍待修复，详见STATUS。

截至R146：主体7ba956dd…已构建、安装及实际加载；控制位快照与缓存回归完成，原版国家权限／三处港口末端仍待修复，详见STATUS。

截至R139：当前实际载入9afd主体，国内模型量重放及三处港口外侧候选核验完成；原版选路未确认，末端仍待修复。完整状态见STATUS。

唯一当前状态为 [STATUS.md](docs/transport/STATUS.md)，详细任务、阻塞和决策分别见同目录 `TASKS.md`、`BLOCKERS.md`、`DECISIONS.md`；旧轮次中的部署／进程描述不当作当前指令。

截至R138：当前主体9afd299a…／47581219B、引导71dae36d…／779879B，源码＝dist＝安装＝R137实际加载；产品源码和两份DLL未变、本轮未部署或启动游戏，完整统一入口58 PASS／零FAIL／退出0。 直接复制实际交付version.dll到含中文／空格的独立路径，工作目录放另一目录及同名无效诱饵；缺少主体和整个freight目录时引导自动建立目录，加载失败码126；损坏PE时193；实际健康主体成功加载且码0。三者均在独立进程中保持系统版本信息查询可用，不是游戏进程内缺失／损坏实机通过。 扩展已有tools/test_freight_loader.py：统一入口同时对重新编译引导和dist实际引导分别验证有效stub、缺失、损坏及错误码0／126／193；17个导出存在、系统版本数据逐字节一致；另外原有--runtime使用真实双DLL验证新配置优先和迁移。无玩家运行时新依赖。 隔离测试及统一入口后两份实际安装与dist哈希相同，原save／settings／ini／content／continue五输入及存档目录保持、无游戏。首私有审计误以为R134清单包含该测试文件而被断言拒绝，核对清单后改为对比本轮before；失败脚本保留。一次仅用于打印的私有脚本遇GBK输出错误，完整测试已结束且结果不受影响。文档归档首版误将保持原CRLF的Python源按Markdown检查而被拒绝，修正后仅重跑归档尾段。 A26新增当前交付引导隔离子项PASS；真实游戏进程中缺失／损坏、目录不可写、完整路径／升级等矩阵仍PARTIAL。三港口末端／运河权限、改图新字段、写任务／提交原子性、真实ABA／新设备与最终同构建A01–A27继续；整体goal ACTIVE。 恢复见STATUS与checkpoint2026-10-29-r138-delivered-loader/NEXT.md。

R119同一构建同进程UK重载world2→菜单／法国新游戏world3→UK world4、最小化／真实客户区resize通过，当前已正常退出；具体场景证据不替代可靠退休事件／ABA／新设备及完整A24。恢复入口checkpoint2026-10-26-r119-world-lifecycle/NEXT.md。

截至R112：同R111已构建/41 PASS候选6a83d933…已临时实装并自主验证原生完成点/372多候选selector/owned交接，3192目的地全部原版已选；一个原生步后新25395订单正确采用。正式UI仍按预期--/frame0拒绝模型回退，海峡未可视修复。正常退出测试会话后原存档/设置/ini逐字节不变，经checkpoint rollback/deploy.ps1恢复实际R102双DLL，当前安装51ec975f…，仓库dist保留6a83d933…候选；无游戏。下一步native同源worker/统计/绘制/UI与端点方向，完整改图/寿命/性能仍待。以STATUS R112及checkpoint2026-10-25-r112-native-capture-live恢复。

**最新操作授权（2026-10-25）**：用户明确“我现在不在电脑旁边，请你自主验证”，允许自主启动及操作测试，覆盖此前手动操控限制；保留不强制结束可能有未保存进度会话的部署约束。

## 6. 进度日志（简要记录，最新在上）

### 2026-11-06 — R216：freight目录不可写降级

icacls拒绝Everyone对freight目录新建写、并对freight.log/loader.log/freight.ini拒绝写；正常入口启动后pid6452存活55秒（世界加载），15秒内三文件大小/mtime不变（写入静默丢弃、无崩溃）；关闭后/reset恢复可写，安装＝dist不变。未做经济交互，只证明只读目录不阻止加载。证据checkpoint2026-11-06-r207-partial-sea-ids（r216-read-only-dir.txt）。

### 2026-11-06 — R215：原版右键菜单与热度颜色模式

市场物流页右键谷物行→原版上下文菜单（信息/钉选/大不列颠的谷物/添加消费税/关税/生产者扩建/消费者）实见，卡片13982.47687不变；右键悬停gen3 ribbons34→移出gen4 187。点击热度→绿→浅绿→黄→橙→红提示、按钮选中、数量不变；Heat/Single/Native三模式均已在1.15操作。右键菜单未逐项点击、逐色像素矩阵未做。证据checkpoint2026-11-06-r207-partial-sea-ids（r215_*）。

### 2026-11-06 — R214：1.15英文locale与双语搜索

`pdx_settings.json`仅改System.language=l_english并在测试后按原字节恢复（bc249507…）。英文页/账本实见；`grain`→3行、卡片1376.37385，`谷物`→空0.00；与中文locale的`谷物`→886.935、`grain`→空对照。两种语言账本13982.47687/3290.11887/10692.358与29725.74/10692.36/19033.39不变。切市场/连续hover/右键未重做。证据checkpoint2026-11-06-r207-partial-sea-ids（r214_*）。

### 2026-11-06 — R213：WM_CHAR搜索与Freight账本实机

游戏文本框可由`WM_CHAR`驱动（SendInput快捷键无效）。中文locale：`grain`无匹配→0.00空列表；`谷物`→仅谷物行、卡片886.935、占比全市场6.3%、账本不变。页面底部账本实读总量/原版贸易/国内模型=13982.47687/3290.11887/10692.358，目标/满足/未满足=29725.74/10692.36/19033.39。英文locale整体切换、切州/切市场未重做。证据checkpoint2026-11-06-r207-partial-sea-ids（search-*.png）。

### 2026-11-06 — R212：两端缺港口seaOnly回退，unavailableSea 2→1

两端均无港口接续的seaOnly航线按海段key作者方向（只作起点／只作终点的唯一锚点）判定源／终点并标记partial；实机`unavailableSea 2→1`、`partialSea 37→38`，仅剩611→608（MINDANAO→NORTH_BORNEO）保持未知；paths=187、页面数值不变。完整入口35/35、geometry/view套件PASS。构建`2028e03b…`／50,025,206B部署安装＝dist。同构建三对：P95 +0.593／+0.518／+0.671 ms（≤2）、模拟最大降0.46%（≤5%）、窗口61.5–62.6秒。证据checkpoint2026-11-06-r207-partial-sea-ids。

### 2026-11-06 — R211：replace_path整目录替换通过（省略差分）

对照ZIP的`map_data/state_regions`故意省略`02_east_europe.txt`并声明replace_paths；实机世界无法建立（`world_ready=false`、`state db unavailable`、error.log成片travel connection缺失，游戏退出），证明1.15执行整目录替换而非合并。含全部17文件的同结构ZIP正常加载且geo=901。content_load恢复、原存档未写。证据checkpoint2026-11-06-r207-partial-sea-ids（r211_omit_test.json、ironroad_omit.zip）。

### 2026-11-06 — R210：省份归属绑定通过

R209把模板索引误当运行省份表索引；修正为读取省份模板+0xC8区域指针（颜色在+0xC4）。基线`x0087C9`→region id=139/geo=154（MAZOVIA），改图后→id=140/geo=155（LOWER_SILESIA）；模板索引23162→19995。改图会话快照1157州、物流页187条、content_load恢复。R209的UPPER_SILESIA结论撤回；replace_path仍未独立证明。证据checkpoint2026-11-06-r207-partial-sea-ids。

### 2026-11-06 — R209：省份归属与replace_path测试（replace_path未证明；省份结论已被R210更正）

省份颜色探针基线`xE165F5`/`x0087C9`→STATE_MAZOVIA；把`x0087C9`移入LOWER_SILESIA的ZIP加载后探针却读到STATE_UPPER_SILESIA（state 1051），模板索引→运行省份表映射或引擎归属规则需专门核对，不作通过。含全部state文件与replace_paths的ZIP加载后州id生效，但1.15 VFS探针报窗口TLS context不可用，替换规则未独立证明。content_load均恢复。证据checkpoint2026-11-06-r207-partial-sea-ids。

### 2026-11-06 — R208：1.15 ZIP Mod身份实机

州id夹具打成ZIP（descriptor＋.metadata＋state_regions文件，3,712B），content_load绝对路径启用；实机`state=1025 key=STATE_BEKES definition=150 geo=901 valid=1`，快照states=1157 goods=54，市场物流页`freight view gen=2 ribbons=187`、`DRAW PERF uploads=187 visible=187 dropped=0`；content_load恢复原字节、ZIP删除。replace_path／省份交换未做。证据checkpoint2026-11-06-r207-partial-sea-ids（zip-test.log、ironroad_map_test.zip）。

### 2026-11-06 — R207：1.15海路缺港口端点回退

诊断日志判明35条unavailableSea全为`native route endpoints are not distinct current port anchors`（州5 HOME_COUNTIES→无港口接续的35目的州）；partial为单侧缺港口的岛屿航线。修复：seaOnly航线一端锚点不在所选海段链上时，用该链自身原版末端控制点替代并标记partial；不造岸点、图端点同源。实机`unavailableSea 35→2`、`partialSea 4→37`、`DRAW PERF paths 185→187`，页面数值不变；完整入口35/35及四个相关套件PASS。剩余2条unavailable（925→590、611→608）与37条partial的未知港口端保留；三末端像素复看待做。同构建`4e0ed054…`三对性能复跑：P95 +0.570／+0.446／+0.556 ms（≤2）、模拟最大降0.21%（≤5%）、窗口61.7–62.6秒，A25同构建立项通过。构建`4e0ed054…`／50,015,990B已部署安装＝dist。证据checkpoint2026-11-06-r207-partial-sea-ids。

### 2026-11-06 — R206：1.15单商品／原生绿模式与原版商品详情实机

单商品模式未选为空图（ribbons=0、提示可见）、卡片量不变；点击谷物行打开原版商品详情（价格/订单/生产消费拆分）且`用途`页可用；关闭→`native market owner closed`。原生绿模式提示颜色不表流向/热度、数量不变。热度未点选、右键菜单未取得；搜索/切州/双语未测，A06/A17/A18仍PARTIAL。证据checkpoint2026-11-06-r201-lifecycle-graphics（r206_*.png）。

### 2026-11-06 — R205：1.15市场页分类／显示隐藏／原版排序实机

日用品分类→卡片贸易量13982.47687→5191.02128、列表切换，取消恢复；隐藏按钮进入选中态（箭头隐藏，数量不变），显示恢复；原版结余排序改变行序并显示原版提示，总量与全市场占比分母不变；关闭面板`native market owner closed`。搜索（键盘注入不可用）／切州／双语未在本轮重做，A18仍PARTIAL。证据checkpoint2026-11-06-r201-lifecycle-graphics（r205_*.png）。

### 2026-11-06 — R204：默认关闭ImGui官方等价复验

无游戏进程且清除调试/计时环境变量后运行`harness.exe dist/version.dll`（等价`test_debug_default.ps1`）：exit0、导出对等、`ImGui ready`、`present #1 hr=0`、后续`imgui=1 vtx=0`、`lists=0 vtx=0`、无`FRAME PERF`，loader `error=0`。PowerShell脚本本身仍受沙箱限制未执行。证据checkpoint2026-11-06-r201-lifecycle-graphics。

### 2026-11-06 — R203：1.15市场物流页hover作用域

市场物流页gen2 owner10 ribbons185；悬停首个商品行→gen3 owner20 ribbons34（uploads33/reused1/visible34），卡片总量13982.47687／510.99MU不变；移出→gen4 owner10 ribbons185（compute0.008ms缓存）。证明1.15下hover只改地图贡献并恢复。分类／显示隐藏／搜索（键盘注入不可用）／切州／双语未在1.15重做，A18–A19仍PARTIAL。证据checkpoint2026-11-06-r201-lifecycle-graphics（hover-*.png、hover-log.txt）。

### 2026-11-06 — R202：1.15部署矩阵与改图身份

- 部署矩阵（同构建、游戏关闭逐例准备）：主体缺失→loader.log `error=126`、主体损坏→`error=193`、整套卸载→无新loader条目、重装→`loaded freight.dll (error=0)`；四例游戏进程均存活≥50秒。事后安装＝dist、EXE／存档／content_load哈希保持。
- 改图身份：1.15目录Mod仅改`STATE_BEKES`的`id` 164→901（其余字节相同），content_load绝对路径启用并在测试后恢复原字节、删除Mod。实机`state=1025 key=STATE_BEKES definition=150 geo=901 valid=1`（无Mod为geo=164）；快照states=1157 goods=54，市场物流页`owner selected`、`freight view gen=2 world=1 rev=4 ribbons=185`、`DRAW PERF uploads=185 visible=185 dropped=0`，显示13982.47687／510.99MU。证明正式模型／视图跟随改图geo身份且不复用旧缓存。
- 同key样条几何：覆盖`spline_network.splnet`仅改点`268437460`的X +10；实机运行时`point=268437460 x=4082.966309`、`expected 4082.966309 delta=0.000000`，网络计数33765／4281／4279不变，快照与物流页（185 ribbons、uploads=185、dropped=0）正常。省份交换／ZIP／设备重建与A01–A27未做。证据checkpoint2026-11-06-r201-lifecycle-graphics。

### 2026-11-06 — R201：稳定视图缓存门槛与1.15重载池恢复修复

- 稳定视图65秒`uploads=346`不变、无新DRAW PERF批次（零新增上传）。20轮关/开市场物流页逐轮`paths=185 uploads=0 reused=185 pool=346 repeats_skipped=1…20`，池≤8192、每帧上传0、`peak_attempts=16`、`peak_frame_us=685`、`overrun_frames=0`；RSSΔ1.9MB、private末6轮波动无单调增长，末4轮相同（平台）。
- 发现并修复1.15生命周期缺陷：`overlay.cpp`三处图形身份读取仍是1.13`0x0592d5c8`（实读无效值`0x404048404c302440`），与校验器1.15`0x05c080b8`（`0x57a50a50000`）不一致，导致菜单→载入world2后误报`native graphics identity changed`、物流页显示未就绪且无箭头。改为`0x05c080b8`后重建`64913eea…`／49,737,579B（引导`c8020412…`），统一35/35，原子部署安装＝dist。
- 修复后重载world2：`native pool recovered world=2 slots=185; old requests cleared`、`freight view gen=7 world=2 ribbons=185 compute=91.031 ms`、`DRAW PERF uploads=0 reused=185 pool=185 visible=185 dropped=0`，页面13982.47687／510.99MU、箭头可见；缩放2560×1440→1920×1080→2560×1440无池退休、无旧帧。
- 三对性能协议（`IRONROAD_FRAME_TIMING=1`、同存档／相机／速度III、每臂热身30秒＋运行65秒）：P95增加0.281／0.407／0.405 ms均≤2；模拟速度11.803→11.731、11.779→11.797、11.821→11.767 date单位/秒，最大降幅0.61%≤5%；窗口61.5–62.6秒、帧流连续且身份一致。未做设备重建、三末端/权限/改图及A01–A27。证据checkpoint2026-11-06-r201-lifecycle-graphics。

### 2026-11-06 — R200：1.15港口接入解码器迁移，官方等价入口35/35

`native_port_approach.h`由1.13迁移到1.15：州地区`state+0xF10`（vtable`0x046ba948`）、市场经`DecodeNativeStateMarket`、world链到manager=`world+0x5028`（vtable`0x046b1d48`）、省份运行表`machine+0xB0`、模板注册表`0x5bd7308`（vtable`0x046ab568`，地区+0xC8）、海军连接表`manager+0x58`（行0x40，kind5、样条key类型3）。陆侧按linked省份判定、海侧物理省份取节点province；仅海区与州地区不同才产生查询；重复id／歧义／错误类型／不可读／变化／退休拒绝。旧1.13夹具替换为1.15夹具，逐项断言并覆盖负例。官方等价原生入口从仓库根运行35/35 PASS、零FAIL（上轮34/35）；`verify_embedded`／GUI 4项／flow GPU含负对照／引导隔离全PASS。PowerShell脚本入口受沙箱限制未跑。新主体`86cdaada…`／49,737,579B＋引导`c8020412…`／779,879B，游戏关闭时原子部署（安装＝dist；未走deploy.ps1）；实机pid39776识别1.15.0、快照states=1157 goods=54，州本地价格页owner15/gen2/ribbons161、市场物流页owner10/gen4/ribbons185、贸易量13982.47687／铁路510.99MU，均真实值且日期保持暂停未存档。详见STATUS R200与checkpoint2026-11-06-r200-port-approach-1.15。

### 2026-11-06 — R199：1.15港口—州接缝修复

用户报本地价格路线图在“海外→伦敦周边→中英格兰”处断开。只读复现：海段lane几何终点为港口陆侧点`0x8005dc`，图端点被掩码归一化到州枢纽`0x5dc`，且lane几何重复包含港口样条→回折/断口。修复：lane改纯海段几何并以实际链端点作图端点（`startAnchor/endAnchor`）；港口边改为合成bit63路径＝原版港口样条＋到主枢纽的见证点连接；无port transfer数据回退原选择器；同海区零距离lane用两条港口样条成链。实数据复核水果两条货流全部相邻边坐标连续（breaks=0）。本地16套测试全PASS；官方等价入口发现旧trade_view夹具缺口，已修复其编译并加入港口连续性回归，其余迁移期夹具失败继续。主体`7d31584a…`／49,735,019B＋引导`fa9ebd38…`已部署（安装＝dist），游戏已重开待开页复看。详见STATUS R199。

### 2026-11-06 — R198：州面板1.15迁移，双页面实机验收

用户报本地价格页物流列`--`：`state_panel.h`仍是1.13偏移与vtable。1.15实读states_panel仍在slot14、vtable`0x0485DB38`、handler+0xE8/+0xF0/+0xFC、history kind14；迁移解码器与market/state面板测试夹具，16套测试全PASS。主体`d36205ed…`／49,618,850B部署（安装＝dist），用户授权自主操作后自动开页验收：州页owner15 frame1、物流列出数值；市场物流页owner10 frame1 ribbons195、贸易量14662.19344／铁路510.99MU、箭头可见。两处`--`消除。州与市场面板同开时州owner会覆盖市场视图（既有优先级，非回归）。副作用：聚焦导致自动解除暂停，速度6推进约3个月（1836-01-13→04-08）后才暂停，未主动存档，需用户确认是否读档。截图与日志见checkpoint r198/，详见STATUS R198。

### 2026-11-06 — R197：1.15贸易表语义修正与无主州处理

用户报物流显示`--`：worker报`native detailed and grouped exports differ`，随后快照报`native state market unavailable`。实机证据：1.15两张贸易表键集合相同但数值逐项不同，grouped按目的组近似常量，不再是detailed的等值聚合；`BindNativeTradeRoutes`改为结构校验＋逐州detailed绑定，不再要求两表等值。无主州`STATE_TEXAS`（owner=`UINT32_MAX`、无特殊市场区）现从贸易名册跳过而非整份拒绝；其他市场失败仍拒绝。离线实数据：4350绑定（4348有路由）、facts 1550 shapes、model 3759 shipments、builtRail 719.251863 MU。构建exit0、14套原生测试全PASS；主体`3ddf32cd…`／49,618,850B已部署（安装＝dist）。用户开页实测：owner selected→`publish document=1`→`freight view gen=2 world=1 rev=4 ribbons=196 compute=98.813ms`→`DRAW PERF visible=196 dropped=0`，`--`修复实机通过；该批次peak_frame_us=2255超2ms软预算1帧，不作性能验收。详见STATUS R197。

### 2026-11-05 — R196：1.15数据管线全链实机通过

修正`market_panel.h`的1.15 handler偏移（barSelf+0xE8／panels+0xF0／count+0xFC），构建exit0、14组原生测试套件全PASS。主体e612657d…／49,713,595B＋引导c8020412…／779,879B在游戏关闭时原子替换安装（沙箱禁止PowerShell，未走deploy.ps1；安装＝dist）。最小化实机pid46532：loader识别1.15.0、三hook安装、3次完整快照复制（states=1157 goods=54）、UI handoff、snapshot accepted，会话段WARN／ERROR=0。游戏自启动即最小化时Present不发生，ImGui定时器需一次离屏首帧才初始化；现保持最小化在屏幕外。正式worker仅由面板持有视图时构建，故本会话无frame；开页→worker→箭头仍需一次开页验收。同会话只读生产代码重放模型通过（shipments=3533、builtRail=719.251863 MU、target／allocated／unmet=136989.210／30890.397／106098.813）；面板链CDataMarket／CDataGoods／CMarketPanel vtable与构造已静态复核。详见STATUS R196与checkpoint2026-11-05-r196-v115-adaptation。

### 2026-11-05 — 逆向工具链接入 Claude Code MCP

按要求把 `E:\Program\ReverseEngineering` 与 `E:\Program\MCP` 既有桥接注册为项目级 `.mcp.json`：ghidra（launcher_dsh.py 懒加载，109 工具）、x64dbg（23 工具）、wrath-analysis（5 工具），命令／端口／环境沿用 Codex 与 DSH 已有配置，无新增下载、未改全局 PATH、未改那两份配置。逐个 stdio 握手成功，并对 ghidra 实调 `list_open_programs`，后端当前为冻结的 1.15.0 victoria3.exe／image140000000／198937 函数。该文件含本机绝对路径，已入 `.git/info/exclude`，不随公开仓库推送；运行中的旧会话需重启并批准项目 MCP 服务器才会出现这些工具。x64dbg 需先开 `Start-x64dbg.cmd`，Ghidra 后端由启动器自动确保、不自动附加游戏。属工具环境变更，不改产品源码／dist／ABI 结论。

### 2026-11-05 — R195进行中：1.15渲染／GUI静态移植及定向回归

12组箭头接口在旧、新精确EXE均重新定位；新增7个发现器反例／正例测试，涵盖不同子类上传实现、缓冲字段不一致／歧义、this别名、调用破坏与新分配对象误判，定向通过并接统一入口。原始指令确认新版供给工厂分配E0、基类字段和float3／20B顶点ABI保持、常量buffer2C、this／path及this／passContext两参保持；原版2945240的domain／stage句柄来源独立确认。已移植这些renderer RVA／签名及资源租约身份，shader常量1与当前原版字段对齐。尚未改全局EXE准入hash，混合迁移阶段不能进入原生调用；未重建或安装中间DLL。

当前两份原版GUI哈希／原生回调和插入锚点已核对，更新C++与Python准入。首轮GUI布局回归发现新版市场价格控件215px使旧540px布局溢出5px；仅在自有物流页派生控件收窄至210，保留原版价格／趋势／导航表达式。修正后4组GUI结构／回调／布局／图例检查PASS；实际flow shader GPU定向检查PASS，包括近景海路高度、正反方向／八档宽度与反例。不是新版进程实际加载或全GUI手动验收。

新旅行网络定位world+5028的原生manager：陆地与海军节点／连接已拆为两对独立容器，旧RTTI读取不可复用。继续核对新版州市场、经济／贸易、访问完成点和native VFS及新网络结构，尚无1.15完整构建／统一回归／部署／实载结论；安装仍卸载，不push。证据checkpoint2026-11-05-r195-v115-compat，暂停R194完整保留。


### 2026-11-05 — R195进行中：Victoria3 1.15.0版本适配

用户在临时卸载后明确要求适配新版本。当前本机EXE为1.15.0／99988600B／SHA256473186e8d8deba993651e0ae4cf02a003b02f5accb99b096a6a0a8bd9506cbaa，当前无游戏、两份安装DLL仍卸载。按新任务继续兼容迁移；不将旧1.13.11验收扩成新版本结论。原始00–03按主题重读，继续local-reverse-engineering：先确认目标／hash，独立恢复、检查ABI／字段／寿命，再分层构建／测试／部署。

源码／测试／文档184份、旧交付双DLL和新EXE均已冻结于checkpoint2026-11-05-r195-v115-compat。暂停前R194的7份源码修改及新helper完整保留为R194-pending.patch和逐文件副本，当前迁移回到R193已验收功能基线，未盲目恢复旧dist。首次留档脚本遇路径键正反斜杠不一致而在改动前拒绝；归一化键后成功。

Ghidra当前新导入本次冻结EXE，x86:LE:64／image140000000；代码候选均绑定本次hash。通用箭头发现已独立找到构造／交叉构造、SetPath、类型解析与分配器；旧“两个子类上传函数地址相同”规则失败，当前两子类不同上传实现都读取对象+60，供给箭头分配尺寸C0→E0／常量2C，需按原始机器码验证而不放松唯一性。Append反编译遗漏参数，原始指令表明传入RDX仍由context函数消费，不能据伪代码改成单参。

旧CTravelConnection／CTravelNode／定义的RTTI在新版不再对应，出现NTravelNetwork::CNode／CConnection与STravelNodeDefinition／STravelConnectionDefinition，陆路采集需要按新结构实现。州、国家市场、TLS所有权等已有多处字段变化候选，未启用未经验证读取或调用。私有RTTI／函数指令对齐取得81份唯一候选，仍仅定位线索；全量临时指令缓存首实现占用过高已停止自身分析进程并改为逐函数释放，重跑完成，未动游戏或用户会话。

当前没有1.15构建／回归／部署／实机通过结论。下一步逐项验证正式渲染／UI／VFS／快照、州市场商品与新旅行网络，再移植并执行新版本测试。全部原始证据留本机checkpoint，不进入玩家资源或硬编码地图表。


### 2026-11-05 — 游戏更新后按用户要求临时卸载 DLL Mod

按用户要求在保存退出后经deploy.ps1卸载引导和主体；配置、日志、缓存及可编辑资源逐项哈希保持，卸载备份已留。游戏更新后的ABI未验证，目标保持暂停，R194未完成源码保留且未部署。当前实际安装已卸载，dist仍保留，详见STATUS。

### 2026-11-03 — R193：动态物理港口接入原版查询实机核对

动态typed物理港口／真实市场输入与1份新本国局部原版查询实机核对，独立结果释放及原输入／选择保持；首版state+48市场误读已纠正，最终完整59 PASS／资源／默认debug通过。f328f684…／71dae36d…部署实载，自有会话正常退出0／五输入保持／probe_access恢复0／无游戏、不push／GUI手动。未接正式几何／权限，三末端未修，下一步真实lane／几何／版本交接，完整goal ACTIVE，见STATUS。

### 2026-11-03 — R193进行中：动态物理港口与本国接入查询

R192同参独立查询／释放是实际进展。本轮新增仅测试／默认关闭诊断使用的typed当前州→物理港口海区输入复制器，保持运行州／定义／geo／实际spline身份分离，不创建ID→key表。异常／读失败／变更／退休／源销毁与分属州市场隔离用例已接统一入口，定向PASS；尚无本轮完整构建回归结论。原版14144FEF0确实取有效非排除海区的首个省份作为规范表示，不把它解释为首个港口候选。物理港口候选全部保留。

原产品06e7272e…的自有9928、23872两次暂停VM_READ核对均正常退出0、五输入保持，probe_access始终0；用户说明的停电属于R188外部中断，不能充当DLL崩溃或正常退出证据。第一次私有probe将13份新decoder复用到已有总32MiB观察窗口，后续读取被预算拒绝；改为每份独立观察窗口后13份全部有效，并逐项与真实原版keyless港内连接的实际owner相同。首版误把state+48分组当市场，导致“6份同市场／7份排除”的结论已撤回；207→206、608→607与936→936的实际owner对照仍成立，市场必须以已有DecodeNativeStateMarket的typed国家／特殊市场区分支重新核对。已修正源码并增加不同分组但同真实市场仍可接入的反例，定向重新PASS；首版构建／回归日志保留，不能作修正后候选验收。该事实不自动证明原版航线本身无路，也不允许借其他市场港口补画。

正在同原版wrapper持有的已验证scope／heap窗口内，使用动态候选、原country、自有CPdxArray24与默认options56执行有界新请求；原生基础缓存仅核对索引登记、不作结果／权限回退。原输出／输入／scope／heap／当前港口重核，独立buffer经原Free释放；异常停后续请求。未接正式worker／权限或绘制，三处末端仍未修，不push／GUI手动。源码前备份及只读证据checkpoint2026-11-03-r193-dynamic-port-queries；产品安装仍R192，完整目标ACTIVE。


### 2026-11-03 — R192：103次原版同参独立重查及原生释放实机通过

已在原版wrapper仍持有相同参数的窗口执行103次独立原生重查，全部结果／输入值相同且临时buffer经原版Free返回，17国／7同省零距离；自然4010查询及103port／17调用栈未被污染。06e7272e…／71dae36d…完整58 PASS／资源／默认debug通过，部署实载；自有32164正常退出0／五输入保持／probe_access恢复0／无游戏、不push。此子项只证明同参窗口，三新港口请求仍待；下一步动态typed州／物理shore参数＋自有CPdxArray24／options56查询，不再重复等价重查。不把它扩为完整原子快照／权限批次或最终矩阵，见STATUS R192。

### 2026-11-03 — R192进行中：原版相同参数重查及独立输出释放

R191实际17条调用栈是进展。本轮复核已有R100／R101／R106完成点与任务证据：CD2BC1已用于正式采集，不新增锁钩子；完整writer完成证明仍待。先只在自然1414500F0→14141F1D0返回窗口、完整原请求仍由原caller持有且scope2／owns1／context1／hidden0及同次heap已验证时，默认关闭probe_access诊断最多128次相同参数独立重查。原借用参数／原输出不变，临时屏蔽自有父trace归类，重查不冒充自然调用；独立40B输出经原版分配器释放，异常即停后续重查。不接不同港口请求或正式路径。源码前备份checkpoint2026-11-03-r192-native-query-replay/before；11项原始机器码重复调用／独立所有权／真实wrapper释放用例PASS（底层heap等仍替代），完整构建中。安装仍f42f9e6b…／71dae36d…，候选无统一回归／部署／实机结论。不push／GUI手动、三末端仍待。


### 2026-11-03 — R191：实机确定贸易重建由ExecuteGameCommand作用域包围，间隔调用并非任务join

17次真实非空重建均经CD2810 ExecuteGameCommand→FC1FE0→14140D670→14140D830，明确在2／1 scope构造与析构之间；现有CD2BC1完成钩子应复用。间隔7BEB90实际是条件yield／reacquire，不是任务join，不能据此开放原子快照／主动查询。f42f9e6b…／71dae36d…完整58 PASS／资源／默认debug通过并部署实载，103次buffer交接／4010查询继续成立。自有41000正常退出0／五输入保持／probe_access恢复0／无游戏、不push／GUI手动；三末端、真实任务完成及权限生产方、最终矩阵继续，见STATUS R191。

### 2026-11-03 — R191进行中：原版贸易重建调用栈与访问窗口

R190的103次真实buffer转移／heap观察是实际进展。本轮按访问窗口主题完整重读01／02／03，沿同hash EXE完整汇编确认国内重建有3条直接caller，一条经callback注册，不能按首个caller推断本次启动路径。源码前备份checkpoint2026-11-03-r191-trade-access-window/before/overlay.cpp；只在原默认关闭probe_access观察器增加24帧有界调用栈及明确return PC，原函数仍只转发一次、无新搜索或正式发布。完整构建进行中，候选尚无统一回归／部署／实机结论；安装仍R19030596be5…／71dae36d…。继续核对acquire／yield／任务join；三末端未修，不push／GUI手动。


### 2026-11-03 — R190：103次自然结果buffer转移与原版线程heap实机核对

默认关闭诊断实机核对103次国内query与wrapper，同次data／capacity／count和allocator全部转移，原版TLS heap保持；完整4010自然查询覆盖。30596be5…／71dae36d…完整58 PASS／资源／默认debug通过，部署实载、自有28600正常退出0／五输入保持／probe_access恢复0／无游戏，不push、GUI手动。结果转移及heap观察子项完成，不证明原子租约／写任务join／world-map-request epoch。下一步核对包围原版贸易重建的实际scope及完成窗口；三末端、正式权限生产方与完整矩阵继续，见STATUS R190。

### 2026-11-03 — R190进行中：自然查询buffer转移与线程heap观察

R189的21项原始机器码用例是实际进展，本轮继续同一调用所有权主题。源码前备份checkpoint2026-11-03-r190-natural-buffer-transfer/before。新增仅观察的当前TLS索引／heap就绪与精确allocator方法核对、24B buffer描述符双读；边界／读失败／变化／源销毁用例进入既有native_access_tests并定向PASS。默认关闭probe_access的自然端口wrapper及其嵌套query记录同次descriptor转移，原函数仍各转发一次，无额外搜索／分配／释放、无正式权限或路径发布。完整构建进行中，尚无本候选统一回归／部署／实机结论；安装仍950d3f32…／71dae36d…。不push、GUI手动，三海路末端及完整目标继续。


### 2026-11-03 — R189：原版结果转移与分配器契约，21项隔离机器码用例通过

- 接续R188完整4010查询是实际进展，本轮离线核对主动查询前的结果所有权，不启动游戏、不操作GUI、不push。精确EXE SHA7f53e326…／image140000000再次校验。停电后的Ghidra当前无打开程序；状态查询明确返回Program not found／空列表，改以同hash本机PE的完整原始汇编及RTTI核对，不将历史连接当当前可用。
- 原版14512CDC0对象RTTI为CPdxNewDeleteAllocator，VT144419230；slot+8的14061BE50接受this／u64字节数／第三槽对齐，实际只转发字节数至14417D960，未消费第三槽。slot+10的14061BE90接受this／data，空指针不调用后端，非空尾调14417D850。不得跨构建复用此地址。14417D960依赖GS:58当前TLS索引1458F8C90及线程heap字段38／初始化44；14417D850从指针所属区域元数据与GS:30核对归属并分支回收。不能用DLL malloc/free或“任意线程调用operator new”代替原版分配器。
- 关键纠正：R183／R186称copy的140D97AC0实际有消费源对象的移动语义。同allocator时先释放旧destination，再转移data／capacity／count并清空source；不同allocator时按目标容量复制或经140C6C8C0／140639FF0增长，source count清零但仍持原buffer供其自身析构。1414502E3调用后，141450306–319只在临时source仍非空时释放；同allocator移动分支不会释放已交给外层的buffer。旧文档“复制后释放内部结果”须按本条解释，不得据此监听一次预期不存在的临时free或重复释放外层内存。
- 新私有隔离harness执行完整原始move／reserve／rebind／allocator wrapper机器码，仅替代底层heap与memcpy：同allocator空／非空／替换／零count有capacity、不同allocator复用／扩容及null-free共10例PASS，逐指针检查最终存活与回收，无重复释放／保留分配。再将原11项完整贸易wrapper／selector／filter harness中的假copy及allocator wrapper替换为这些真实方法，11例PASS，全部结果与R183逐项一致、临时分配全部释放、输入字节保持。合计21项隔离用例，不是实际游戏heap／TLS／主动查询安全证明。
- 原始机器码／工具与fixture全部留外部checkpoint2026-11-01-r189-native-result-allocator，不加入玩家DLL或仓库测试；没有修改产品源码、构建、部署或运行游戏，未重跑无变化的产品统一入口。当前dist＝安装仍950d3f32…／71dae36d…，58 PASS／资源／默认debug为R188同产品历史证据；五保护输入仍一致、probe_access0、无游戏。最初私有脚本名inspect.py遮蔽stdlib导致导入失败，改inspect_allocator.py；无pdata叶函数最初无范围被assert拒绝，按完整14B／精确反汇编范围处理，未放松哈希／ABI边界。
- 结果所有权的静态／隔离子项完成；下一步在已验证的自然外层调用栈内核对同次结果buffer转移及实际allocator／TLS上下文，不从render／worker另行调用。正式查询仍需真实访问窗口、完整world／map／request身份、原生完成／退休及输出寿命证据；不将上述fixture或指针相等冒充租约。三海路末端仍未修，逐lane权限生产方、改图、GUI／生命周期／性能／最终A01–A27继续，goal ACTIVE。

### 2026-11-01 — R188：4,010次自然贸易查询完整覆盖，局部港口查询并未自然产生

- 仅扩大默认关闭probe_access诊断额度：普通检查128→8192／复制32→4096，国内仍512／256；原函数仍只转发一次，country／options／同次分类／scope复核不变。没有主动搜索、正式权限发布、路线补画或经济写入。源码前备份checkpoint2026-11-01-r188-complete-trade-observation/before；不push／GUI手动。
- 完整native/build.ps1退出0、统一build_tests.ps1退出0／58 PASS／零FAIL、embedded PASS、默认debug独立backend94292零UI顶点PASS。主体950d3f324047d94ef2b0686e047b37eeb512790d902a3584fa650de2752419c6／50968135B，引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B；deploy.ps1部署，dist与安装一致。只改变诊断额度，R185业务逻辑不变；旧用户缺段确认不扩为本构建完整GUI验收。
- 自有95000普通续玩，phase4／暂停／date59883804，临时probe_access1。当前DLL符号表解析7个自有atomic地址，绑定进程创建时间／实际模块路径与hash，PROCESS_VM_READ两次计数相同：总4010＝普通3907＋国内103，分别与全部日志copy一一相等；port选择103／103。样本0–4009连续、283个country、已知无路0，无本轮额度截断或复制丢失。模块基址7ffdc2c80000由当时枚举／计数读取得到；事后完整modules.py因进程已不存在拒绝，未伪造模块清单。
- 当前动态PORT_MISMATCH／MISSING_PORT／OWNED_LANE关联三处缺口，按同country／海省对／完整options核对：前两对完全没有自然请求，第三对也无对应country，仅另一country有1份。故本次启动中扩大被动观察已足够，不能借其他国家结果／基础矩阵补画，也不能把缺查询当原版无路。三海路末端仍未修，下一步核对完整原生查询的真实分配释放及调用访问窗口，或找到原版对应端点路径来源；不继续只加配额。
- 恢复工作时95000已不存在，未发送WM_CLOSE、未kill、无close.json或退出码。用户明确说明停电造成电脑非正常关机，记为外部中断；本次正常退出验收未完成。Application事件查询未发现本次Victoria3／freight崩溃条目、最新游戏crash目录仍10月27日，这不能替代退出码。已归档现存日志并复核原始五输入逐字节保持，临时INI原子恢复原probe_access0；相关资源错误0，六条Unexpected token同基线。当前无游戏，没有保存／提交／push。
- 自然调用覆盖核对子项完成，不代表三个局部查询已有原版结果或安全主动调用已证。正式world／map／request身份、访问租约／退休、逐lane权限生产方、改图、完整GUI／生命周期／性能及最终同构建A01–A27继续，整体goal ACTIVE。证据checkpoint2026-11-01-r188-complete-trade-observation/{build.log,tests.log,embedded.log,debug-default.log,deploy.log,live/observation-counters.json,live/all-natural-queries.json,live/approach-query-audit.json,live/port-census.log,live/owned-lanes.log,live/session-freight.log,live/after.json,config-restored.json,audit.json,NEXT.md}。

### 2026-11-01 — R188进行中：完整贸易自然调用覆盖与港口接入查询

- 上轮R187新增自有海区缓存复制并以真实差异否定基础矩阵等于最终查询，是实际进展。本轮继续三处末端：只将现有probe_access诊断的普通查询copy上限32→4096、检查上限128→8192，国内仍256／512，保留全部country／options／同次分类和scope校验。所有限制仍有界，不追加原生寻路，不修改正式路径或经济；不push／GUI手动。
- 源码前备份checkpoint2026-11-01-r188-complete-trade-observation/before；完整native/build.ps1已退出0，当前候选两DLL待完整统一回归，尚无该候选部署／实机结论。安装仍R186719a2d83…，R187也未改过产品。
- 私有只读coverage reader从本次交付DLL的COFF符号表导出自有atomic计数器地址，绑定实际模块路径／hash／进程创建时间后双读；不跨构建复用RVA，不读写引擎任务状态。后续以总调用／copy计数与解析日志核对覆盖，再由当前动态港口错配记录查同country／同海省对的完整自然查询，不借其他国家权限或基础缓存补画。

### 2026-11-01 — R187：原版海区基础缓存自有复制与最终查询差异实机核对

- 上轮R186的完整州选择为实际进展，本轮继续三处海路端点。新增native_sea_cache.h仅由既有native_access_tests使用，未接overlay／正式采集／worker。当前root→world+4500的typed CSeaProvincePathfinder、原版u16索引Robin探测、24B三角行、32B条目和i64距离／原始省份集合复制为自有值；不调用搜索／getter／引擎分配器。空缓存只是观察到空条目，不证明失败搜索，源＝目的属于原版另一个分支，显式不由此helper伪造；非空零距离保留。
- 精确EXE SHA7f53e326…／image140000000；原版14141FA80缓存分支与14141EA10填充、RTTI VT044EB4D8和1413BAAF0构造核对。构造先发布owner+100指针，再调用填充，不能将指针存在或矩阵尺寸正确当完成见证。PE原始xref扫描的1412C2224候选是将call操作码误当disp的假引用，汇编排除；真实构造调用为1413BABAF。后续必须绑定构建完成／退休／world-map-request及原子访问窗口，当前纯复制器不授予这些能力。
- 新测试进入原统一入口：方向与原版三角列、Robin碰撞／存在于错误位置不可被原版探测、类型／重复索引／key／数量／地址／范围／距离／读失败、复制中变化、scope退休与源销毁；定向PASS。完整build_tests.ps1退出0／58 PASS／零FAIL、embedded PASS。未改产品调用点、未重建或部署两份DLL；主体仍719a2d8313a57f86316449bbbb34b023a839d598e6a308d8dc61fa960bcebab4／50968135B，引导71dae36d…／779879B，dist＝安装＝before冻结，R186默认debug是同产品历史证据。
- 自有3236普通续玩，复用R186默认关闭的自然观察临时probe_access1；实际模块为上述同产品、EXE base7ff63dec0000。当前自然135份最终查询（32普通＋103国内）与两个只读probe的cache scope同world4575261c000，phase4／暂停／date59883804，无GUI／相机／推进输入。以84个不同有向海省对复制128份非同省查询对应基础缓存，重复复制逐字节值一致：83份与最终查询的距离及全部范围顺序相同，45份不同＝30份缓存空但最终有路＋15份非空缓存距离更长；国内相同67／不同29，另7同省由原版本身已知零分支处理，未伪装为此缓存读取。
- 此差异否定“原生基础缓存就是最终可通行贸易结果”的假设，不是将空条目当无路的理由。完整14141F1D0尚有候选／限制及141420800结果比较等逻辑；不能据上述值推断具体45份都走哪一分支，也不能绕过country／options／通行取得全路线。初比较脚本曾按全128必相同断言，sample0缓存空与实际正距离导致拒绝；纠正为逐项记录差异，保留失败说明，不把错误假设改成源数据。
- 当前624份州港绑定动态错配13行／6地区，对应5个不同配置出口↔物理港口海区对，全部读得原生缓存。三处缺口对应的3对均两海省／一条当前海样条，空权限几何检查无governed adjacency；另一个错配对35987↔36029有直连几何key，但原版缓存为长路径／448997480、九条几何key且空权限检查挡6个有向governed邻接。这是不能按现有direct key补路的具体反例。三处局部缓存条目不等于完整商业选择见证，尚未据此补画／改变模型或放行海峡。
- 自有3236只读后正常WM_CLOSE退出0，五原始保护输入逐字节保持，临时probe_access1原子恢复原0；扩展相关资源错误0、六条Unexpected token同基线。当前审计无游戏，未保存、提交或push。私有query probe首生成误套模板tail而被assert拒绝，后续编译找不到未生成文件；按实际owned-lane尾部修正并加生成失败守卫后编译通过。Ghidra未定义两个小函数时改读完整汇编，未创建假函数或放松结果。
- 海区基础缓存的自有复制／实机条目核对子项完成，三海路末端与正式权限仍未修。下一步查完整贸易函数对当前这3对局部接入的真实选择／country／options，及原生构建完成、退休与身份交接；不把基础矩阵作最终路线回退。地图兼容、生命周期／GUI原生提交／性能及最终同构建A01–A27继续，整体goal ACTIVE。源码前备份与全部证据checkpoint2026-11-01-r187-native-sea-cache/{before,delivery-before,native-access.log,tests.log,embedded.log,cache-type.json,cache-getter-assembly.txt,cache-constructor-assembly.txt,cache-builder-callers.json,source-change.diff,live/natural-queries.json,live/query-cache-scope.log,live/cache-ports-scope.log,live/cache-query-comparison.json,live/loaded-modules.json,live/close.json,live/after.json,config-restored.json,audit.json,NEXT.md}。

### 2026-11-01 — R187进行中：原生海区缓存复制与港口缺口只读核对

- 上轮R186完成完整州／同次查询实机绑定，是实际进展。本轮继续三处海路末端，按现有主题与local-reverse-engineering精确EXE边界研究，不操作GUI、不push。源码前备份checkpoint2026-11-01-r187-native-sea-cache/before；产品源码／两份DLL未改，安装仍719a2d83…／71dae36d…。
- 精确EXE及14141FA80原始缓存分支核对后，新增只在native_access_tests使用的native_sea_cache.h：当前root／world+4500、typed CSeaProvincePathfinder、原版u16→index Robin碰撞查找、24B三角行、32B条目与i64距离／省份集合复制。数据自有，未调用引擎／分配／搜索；空条目只表示观察到空缓存，不证明失败搜索或通行。异常／越界／错误类型／身份重复／原版探测不可达／复制中变化／退休／源销毁用例定向PASS，已在原统一入口目标内。
- RTTI与构造调用确认VT044EB4D8；1413BAAF0先写owner+100缓存指针，再执行14141EA10填充，因此不能把非空指针当完成见证。PE初扫描的另一候选1412C2224经汇编判为把call操作码当disp产生的假xref，未当真实writer。后续正式消费仍需原生构建完成／寿命窗口与失效绑定，当前helper未接runtime或worker。
- 完整统一回归仍在运行，无本轮最终PASS／实机结论。正在准备只读比较当前自然贸易返回与原生缓存，以及配置出口到物理港口的动态错配对；私有probe模板首生成错误因实际tail不同被assert拒绝，已按实际owned-lane模板修正并编译，不涉及产品代码或游戏。不能据已有direct key／无governed pair就补画；完整目标ACTIVE。

### 2026-11-01 — R186：完整州选择与嵌套贸易返回实机交接，103次国内查询覆盖完整

- 上轮R185是实际源码／部署及用户缺段确认进展；本轮继续原版海路端点绑定。R185的85984在新轮库存中已不存在，未由我方关闭。源码前备份checkpoint2026-11-01-r186-port-selection/before，沿既有local-reverse-engineering精确EXE／ABI／所有权门槛；手动GUI与不push继续。
- 再次核对1414500F0及141419FEF原始caller：完整九槽void ABI，输出ProvinceSet、两个u16海省、两个完整u32州、距离i64、两份u32候选州组与country32；caller返回后才释放候选。原版将同次选中海省映射为完整州。新增CopyNativePortSearchInputs／Selection，保留完整句柄、零、重复和顺序；最终路径／海省／距离必须逐项等于同次嵌套贸易结果，完整州必须属于对应组。已知无路、未知读取失败及合法非空零距离分开；新边界／读失败／变更／方向／源销毁回归进入原native_access_tests。
- 默认关闭probe_access下新增自然wrapper观察，只转发九槽原函数一次、不额外查询／分配／释放。TLS要求同完整country、同次恰好一次合法嵌套贸易调用，与外层前后world／machine／stateDB／date／phase／access／hidden／owns／context／paused及候选匹配，再复制最终州输出并查typed当前完整州；借用指针不出调用栈，日志只含自有数值。国内查询记录额度96→256、检查上限仍512；普通仍32／检查128。未向正式worker／权限批次／几何发布，顺序观察不授予访问租约／ABA安全。
- 完整native/build.ps1退出0、统一build_tests.ps1退出0／58 PASS／零FAIL，embedded PASS、默认debug独立backend95408零UI顶点PASS。当前主体719a2d8313a57f86316449bbbb34b023a839d598e6a308d8dc61fa960bcebab4／50968135B，引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B；经deploy.ps1安装，源码／dist／安装／自有93548实际模块对应。EXE SHA7f53e326…／image140000000／加载base7ff63dec0000。只改变默认关闭的诊断与自有复制器，R185业务逻辑未改；R185用户GUI确认不冒充本轮完整GUI验收。
- 普通EXE续玩、临时probe_access1，phase4／暂停／date59883804。完整取得135份查询＝32普通＋103国内，103份外层完整州选择与各自内层query编号／country／world／线程／构建诊断代数／日期／访问／海省／距离／全部范围逐项相同；17次非空原版重建的逐次classification数与query／port记录数完全相等，补齐R184漏7次。没有跳过，7份非空零距离合法，实际已知无路0，额外限制／受限变化仍未覆盖。
- 同进程PROCESS_VM_READ生产shipping解码复制并释放读取句柄后比较：93条当前lane完整句柄／country／两端州／距离64／全部范围与最终构造记录一致；每条按同world／同batch／完整州对／距离／范围都只有1份外层选择结果相符，多义0。93条全部与原始查询方向相反，明确原版最终构造的方向转换，不能沿用查询两端顺序。此为同批值与方向的强对照，不是跨调用身份cookie／原生集合epoch／原子租约；三条历史部分海路均取得完整州／海省记录，尚未据此补画或改变权限。
- 自有93548没有市场／相机／推进输入，正常WM_CLOSE退出0。五原始保护输入保持，临时probe_access1退出后原子恢复原0；额外校验当前临时INI仍为我方预期后才恢复，避免覆盖外部编辑。扩展相关资源错误0、六条Unexpected token同基线，当前审计无游戏。未保存、提交／push；完整构建／回归／本轮实机没有未解释失败。
- 完整州选择的同次自有复制及实机对照子项完成。下一步追踪这三个实际端点的原版图形来源，并完成查询／最终lane的正式身份及world／map／request版本、访问窗口／退休更新与集合寿命；不得将采样代数充当正式generation。真实逐lane权限生产方仍未绑定，三海路末端未修；受限重算／已有lane更新、改图、GUI／原生提交与生命周期／性能／最终A01–A27继续，整体goal ACTIVE。
- 证据E:/Code/v3-analysis/checkpoints/2026-11-01-r186-port-selection/{before,delivery-before,delivery,port-wrapper-assembly.txt,port-caller-assembly.txt,native-access.log,source-change.diff,build.log,tests.log,embedded.log,debug-default.log,deploy.log,live/session-freight.log,live/owned-lanes.log,live/port-selection-audit.json,live/loaded-modules.json,live/close.json,live/after.json,three-port-records.json,config-restored.json,audit.json,NEXT.md}。原始数据与诊断ID不进入玩家地图表／运行依赖。

### 2026-11-01 — R186进行中：原版完整州选择与同次贸易返回交接

- 上轮R185是实际源码进展并已取得用户缺段修复确认。本轮继续完整目标中的海路端点绑定；新轮开始未检测到游戏进程，未关闭任何用户会话。不push、手动GUI继续。源码前备份checkpoint2026-11-01-r186-port-selection/before；当前安装仍R18508e1e433…，原版正式通行生产方与三处末端仍待。
- 汇编／原始caller再次核对1414500F0九槽void ABI：输出ProvinceSet、两个u16海省、两个完整u32运行州、距离i64、两份u32州组、country32；原版按选中海省从同次映射取完整州，caller在返回后才释放州组。新增两份自有复制器，保留完整句柄／0／重复／顺序，最终输出必须逐项等于同次嵌套贸易结果并属于对应候选组；已知无路与未知／读失败分开，非空零距离合法。原native_access_tests的复制／边界／变更／寿命／方向用例定向PASS。
- 现有默认关闭probe_access下新增自然wrapper观察代码，只转发原函数一次并保留全部九槽；TLS同次恰好一个合法贸易查询、完整country、同世界／DB／日期／访问／线程上下文和前后候选匹配后才记录完整州输出，结果仅自有诊断。国内查询记录额度96→256补足R184103次中漏7次；检查额度仍512。未接正式权限／worker／几何，不把前后观察当租约或ABA证明。
- 完整构建正在进行，新增observer尚无完整统一回归／部署或实机证据；不能继承R18558 PASS到本轮修改。后续先构建／统一检查，再在可用自有暂停会话只读核对完整州及原版最终构造方向；三海路末端和全目标继续ACTIVE。

### 2026-11-01 — R185：同市场分属州装卸修复已部署，当前新会话保留

- 用户新报分属地区道路缺段，本轮优先陆路。新反例在修复前FAIL：任意州接入点可独立装卸，货流绕过实际州内道路。新增自有landHubs，从当前typed主陆地节点role mask1复制完整控制点／省份完整运行州／region，核对原版方向、当前样条及联合复读；按同market／definition／geo共用有见证的主装卸点，进出必须走实体道路。不借州门中转、不按ID→key或最近点选枢纽；新字段进入缓存身份。这个位置是国内观察性模型假设，无主点见证区域仍保留旧接入，不称全图完成。
- 01／02／03按数据／分配／地图主题重读，沿local-reverse-engineering精确版本边界。没有增加新hook／主动查询／经济写入。输入／输出反向、缺真实道路、外市场枢纽、坏／冲突主点及同revision缓存回归进入已有view测试；typed复制／反向原生定义／锚点及region异常／源销毁进入已有domestic测试。完整build.ps1退出0、统一build_tests.ps1退出0／58 PASS／零FAIL，embedded与默认debug独立backend93064零UI顶点通过。
- 当前主体08e1e43388ece6d7e75d327a18e32dc44c46e454f725ff381991cc47af086932／50815294B，引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B；经deploy.ps1部署，dist＝安装＝自有81652实际加载，EXE SHA7f53e326…／image140000000／加载base7ff63dec0000。普通续玩、不加debug_mode、probe_access始终0。
- 同一实际输入释放读取句柄后做自有回放：531主枢纽、25420条余额的target／available逐项保持，target140740221 milli；分配84807060→84922349、未满足55933161→55817872，9424笔按州／商品独立守恒。库尔努尔91501→91504道路（仅本存档诊断标签）从0变21笔；普通完整样条48197接缝broken0。生产facts／view／ribbon函数回放中该段quantity13041700（1e5展示单位），市场0共271段／271条、马德拉斯527共44段／44条，都包含该段。不是当前GUI点击／GPU像素验收。
- 自有95032／81652全程观察phase4／暂停／date59883804，无市场／相机／推进输入，正常WM_CLOSE退出0；各自退出后五保护输入保持，扩展资源错误0、六条Unexpected token同原基线。随后检测到新进程85984（18:18:04），不视为我方测试会话，不操作／关闭／替换DLL，保留运行。未保存、提交或push。文档首归档脚本因断言“没有游戏”遇新进程而拒绝，尚未写最终记录；本次按保留新会话纠正，不把该拒绝当产品回归。
- 2331同市场／2701含跨市场诊断道路没有端点落在别的连接内部，排除首路口假设，未添加无关拆边代码。首探针误传历史goods49被拒绝，按当前catalog53重跑通过。私有PASS打印遇GBK错误不影响完整测试，最终ASCII核对58条（44冒号前缀＋14其他）。下方“R185进行中”是先前记录；失败原始文件均保留。
- 同市场分属州主枢纽装卸漏洞子项完成；用户在本次部署后明确反馈“现在没有发现这些问题了”“问题已修复”，本次州内缺段取得用户实机确认。该确认不外推所有地图／不同市场过境或最终矩阵；无真实货流不为视觉连续填假量。缺主节点见证／不同市场过境、更广场景继续。三海路末端、正式完整通行生产方、改图新字段、scope／写任务join／生命周期／性能及同构建A01–A27未完成，整体goal ACTIVE。备份／证据checkpoint2026-11-01-r185-split-state-land/{before,delivery-before,delivery,source-change.diff,before-test.log,after-test.log,build.log,tests.log,embedded.log,debug-default.log,deploy.log,live,live-final/formal-hub.log,live-final/loaded-modules.json,live-final/close.json,live-final/after.json,audit.json,NEXT.md}。

### 2026-11-01 — R185进行中：分属州装卸入口导致州内道路漏流，修复已回放

- 按用户新报的分属地区缺段转到陆路；未继续增加海路选择hook，不push。源码前备份在checkpoint2026-11-01-r185-split-state-land/before。自有95032普通续玩／probe_access0，暂停date59883804，只读复制后已正常WM_CLOSE退出0；五保护输入保持，无游戏。原版完整通行生产方、三海路末端和最终矩阵继续。
- 2331条当前同市场陆路及2701条含跨市场诊断道路均无“整条连接内部遇到另一连接端点”，排除最初路口猜测。实际模型允许净供需在任一州接入点独立装卸；库尔努尔同市场分属州内91501→91504实体道路因此货流0，并不是样条缺失。马德拉斯还存在不同市场飞地，不能据截图统一放行。
- 新增自有landHubs，从原版当前typed节点role mask1读取主陆地控制点及省份完整运行州、region关系；方向／锚点／原版几何、复读和世界检查保留。按同market／definition／geo将有见证的主枢纽作为州级净订单装卸位置，实际进出必须走实体道路；不以州门作中转。无已验证主枢纽的区域保留既有接入，仍是模型假设，不冒充原版逐笔运输；外市场枢纽不借用。身份比较纳入新字段。
- 新反例在修复前FAIL、修复后原native_trade_view_tests通过；原native_domestic_tests新增typed主枢纽复制／异常与源销毁用例通过，随后增加反向定义用例待统一入口。相同真实自有输入复制531个主枢纽，25420条供需余额与target140740221 milli保持；分配84807060→84922349，9424笔逐州／商品独立守恒。库尔努尔该段从0变为21笔经过，普通完整样条48197接缝broken0；不等于截图像素／所有分属地区验收。
- 完整native/build.ps1进行中；该修订尚无完整统一回归、部署或新DLL实载结论。旧b0a1c9c3…仍为当前安装，58 PASS仅属R184历史产品。首只读探针误传goods49被明确拒绝，按当前原生catalog53重跑通过；未把拒绝当模型失败。

### 2026-11-01 — R184：自然贸易请求／同次分类实机复制，最终州选择仍待绑定

- 新增自有候选／两份选项复制及默认关闭14141F1D0七槽自然观察，TLS内同次mode1分类恰一次且完整country匹配，前后manager／请求／scope观察一致再复制；原函数只转发一次，无主动查询、正式路线／权限／统计发布。新异常用例接原native_access_tests。
- 首648候选只捕获96普通查询，修正诊断额度后最终b0a1c9c3384373b6cfa7f28b8ed494c4bde744e3b3ead351c81fbe03b1d6531c／50679724B＋71dae36d…完整构建／统一58 PASS／零FAIL／embedded／默认debug零顶点，通过deploy.ps1部署并在93928实际加载。
- 最终32普通＋96国内（13国家／重建）自然结果／同次分类通过；独立VM_READ的93当前lane与构造逐项相同。国内55份单条同值、33份多条同值、8份无最终同值，不把相等当获胜身份或选首个港口；103次国内分类仅记录96，4份合法零距离。原版完整州选择、map／request／scope和主动调用寿命仍待，三末端未修。
- 自有33448／93928均暂停同date59883804、无GUI／推进输入，正常WM_CLOSE退出0，原ini／probe_access0和五保护输入保持、资源错误0、无游戏、不push。自然贸易复制子项完成，正式生产方与完整目标继续。证据及前后交付冻结checkpoint2026-11-01-r184-natural-trade-search，详见STATUS／NEXT.md。

### 2026-11-01 — R184进行中：自然贸易请求已捕获，补国内诊断额度

- 首候选648e9669…／50677026B完整构建及统一58 PASS、embedded／默认debug零顶点通过，经deploy.ps1部署并在自有33448实载。暂停date59883804中记录96份七槽自然贸易搜索及同次mode1分类，全部默认选项；首次额度均由caller RVA0122E6F8／build0消耗，未覆盖国内港口重建，不能当国内端点绑定完成。
- 自有33448无GUI／推进输入，正常WM_CLOSE退出0，五保护输入保持、原probe_access0已恢复，当前无游戏。证据checkpoint2026-11-01-r184-natural-trade-search/live及delivery-first；旧产品／原输入备份在before／delivery-before／protected。
- 正在将诊断额度分为普通查询32份与国内重建96份、检查次数分别128／512，源码已改；最终构建正在执行，尚无该修订的新增测试／部署结论。无主动搜索、正式数据／几何消费或经济修改，不push；整体goal ACTIVE。

### 2026-11-01 — R183：贸易搜索选项／返回契约隔离核验（未启用查询）

- 14141F1D0汇编纠正旧wrapper反编译省略：country在第5槽，选项指针在第7槽；选项两份数组＋u16选择值可影响最终限制，不能丢掉。原版包装／候选选择／限制对筛选机器码11例通过；分类器／搜索和内存服务为替身，不是实际商业权限／TLS／主动调用寿命证明。
- CopyNativeTradeSearchResult仅测试调用，保留原范围顺序，核对请求候选、端点／范围／距离及复读变化，区分已知无路与读失败／合法零距离。11份实际机器码原始输出由同一复制器回放一致；新增异常接原native_access_tests，统一58 PASS／零FAIL／退出0、embedded PASS。私有TLS getter长度和pdata首段截断两处错误已修正，失败记录保留；后者复用现有ironpe展开链解析，不改正常工具。
- 产品保持R182 b85b773f…／71dae36d…且dist＝安装；本轮未重建／部署／启动游戏、新hook或游戏进程内查询，五保护输入保持／probe_access0，不提交／push。隔离契约子项完成，正式原版请求／选项／所选端点／版本与寿命还待，三末端未修。证据checkpoint2026-11-01-r183-trade-path-contract，详见STATUS／NEXT.md。

### 2026-11-01 — R182：自然舰队搜索结果观察已实机核对（正式路径仍未接入）

- 新增typed fleet请求自有复制，按原版140C59AF0及当前TPdxRefDatabase<CTravelNode,7>完整ID查节点，RTTI／ABI汇编独立核对；异常与变化测试接原native_access_tests。默认关闭probe_access下观察自然1414043D0调用，只转发一次、核验实际返回1与请求／观察前后匹配后复制，不主动搜索、不发布正式路径或权限。
- 完整构建／统一58 PASS／零FAIL／退出0、embedded与默认debug独立backend零顶点通过。主体b85b773fe2b5f220822e37d593966b90b8998ecb2f97090da0117dcc942cbfc9／50409230B，引导71dae36d…／779879B，dist＝安装＝实际32488双模块。
- 普通暂停续玩取得2份原版fleet成功结果；独立VM_READ将两份结果的完整连接句柄、当前国家／节点、方向、固定坐标及定义距离逐项对上，仍是军事结果及顺序读取，不等于商业权限／原子scope／三条shipping lane绑定。观察date59883804保持，无GUI／推进输入，正常WM_CLOSE退出0／五输入保持；临时probe_access1已原子恢复原0，当前无游戏、不push。
- 自然结果观察子项完成，三处海路末端未修，正式通行生产方仍未绑定。需继续实际端点／地图版本／商业语义／访问窗口／寿命及当前GUI、改图、生命周期、性能与最终矩阵。证据及前后交付冻结checkpoint2026-11-01-r182-native-search-observer，详见STATUS／NEXT.md。

### 2026-11-01 — R181：原版路径结果机器码核验与自有复制器（未接正式调用）

- 精确EXE的141405EF0重建／141407FB0追加机器码7组在独立进程通过，7份原始结果由新CopyNativePathResult回放一致；合成前驱图／预分配输出，不是原版搜索、权限、分配器或寿命证明。普通64B记录保留完整句柄／方向／距离／费用；自定义段显式拒绝、不画直线。
- 新native_path_probe.h仅由已有native_access_tests使用，异常／变化／溢出／同点空路径用例接原入口；完整58 PASS／零FAIL／退出0、embedded PASS。原版141404330无论搜索成功与否都返回结果地址，后续必须核对1414043D0实际成功返回值，不能凭非空结果指针发布。
- 产品未变，仍f53f81d6…／71dae36d…且dist＝安装；无新native调用／hook，无游戏启动／GUI操作，无构建部署／提交／push。五保护输入与R180退出基线保持。当前结果复制子项完成，三海路末端／通行正式生产方及完整目标仍待。备份与证据checkpoint2026-11-01-r181-native-path-result，详见STATUS及NEXT.md。

### 2026-11-01 — R180：港口出口错配普查与原生舰队寻路候选（未启用）

- 本轮只做当前数据只读核对及精确EXE静态分析，未修改产品源码／DLL、未构建／部署、不提交／push。当前主体仍R179 f53f81d65f1058b9a066ba2b48e4ce8159fa16765f36fc5d89bce4728db588e0／50222428B，引导71dae36d…／779879B；R179统一58 PASS为同产品历史证据，本轮没有新增整套PASS。R177港内路径与R179物理可达性已在生产使用，本轮发现的寻路候选仍未启用。
- 独立PROCESS_VM_READ探针复用生产解码器，在同一EconomyReadWindow内复制当前州、港口／海路／样条／naval_exit定义并复核窗口；释放读取句柄后分析自有数据。624份运行州港口绑定中13份、共6个物理地区的配置出口海区与实际港口所在海区不同。当前三条不完整lane218103887／218103899／218103903均配置出口属于成员、实际港口海区不属于成员。动态普查不使用ID→key地图表；上述句柄只是本次证据标签。外部顺序读取仍不构成原子原生访问租约。
- naval_exit模板索引来自当前typed CStateRegionTemplateDatabase；141572DE0汇编以JBE接受index==bound，故+F60是包含端点的最大索引。私有初版误用排他上限，漏四份州，保留port-census.log；修正后的port-census-final.log为最终624份记录，未将此新字段接入产品。1425E8450→1425E8690→1425E8070原版成员边界调用传入成员地区，非外侧邻居；没有可直接改成peer的静态证据。三个当前外侧直连key存在且第一省份对未命中海峡定义，不等价原版选择／通行许可，未补画外侧海段。
- 精确EXE 7f53e326…／image140000000确认NTravelNetwork::CPathFinder构造141403EF0、搜索1414043D0和结果重建141405EF0；基础vtable权限回调为常量true，不能用于绕过原版权限。CFormationPathfinder国家构造140D6CD80／vtable1444A0438的type+168=1由FLEET_CONNECTION_CHARTED_COST_MULT原版常量引用独立确认；通行回调140D6CE50同时涉及海峡谓词140BE7C10及军事地区访问14140B890，不能把军事语义自动当贸易许可，+190遍历选择也不能与type混用。
- 原版搜索参数+40为结果数组，+58原版距离／+60费用；重建候选每条64B：connection+0、fromNode+4、fromXY+8/+10、toNode+18、toXY+20/+28、distance+30、cost+38（偏移均十六进制）。这些是静态候选，尚无真实调用／结果生命周期验收；构造器涉及引擎／TLS分配，不主动调用、不挂新钩子、不向worker交指针。下一步须验证原版country／mode／访问窗口、结果方向、分配释放及寿命后，才讨论末端几何接入。
- 自有正常续玩PID7468实际加载对应双DLL，观察phase4／暂停／date59883804；无市场／相机／推进输入，正常WM_CLOSE退出0，五保护输入逐字节保持，扩展资源错误0、六条Unexpected token同基线，当前无游戏。证据及文档前备份E:/Code/v3-analysis/checkpoints/2026-11-01-r180-native-port-access/{before,audit.json,port_census.cpp,pathfinder-rtti.json,formation-pathfinder-rtti.json,pathfinder-callers.json,naval-exit-assembly.txt,trade-render-boundary-assembly.txt,fleet-cost-define-assembly.txt,live/port-census-final.log,live/loaded-modules.json,live/close.json,live/after.json}。
- 港口出口错配调查子项完成，三处末端仍未修复。真实通行批次生产方仍未绑定／probe_access0；当前UI、原生scope、改图、生命周期、性能及最终同构建A01–A27继续，整体goal ACTIVE。

### 2026-11-01 — R179：物理可达性前置分配，断路不再冒充已满足

- 国内分配从州级中转改为当前物理节点图前置，复用既有最小费用最大流；州级净供需以独立有向收／发门共享上限，中转不能跳过不连通节点。新增断路、替代供应、真实接驳、上限／守恒／零距离／缺几何用例；保留原版已知海运数量，不把绘图缺失当断路。
- 完整构建／统一58 PASS／embedded／默认debug零顶点通过，f53f81d6…／50222428B主体与71dae36d…引导已部署实载。相同只读输入上目标及供给不变，满足94128826→84807060 milli、差额入未满足；25420余额逐项守恒，8842笔最终货流、38838已知完整接缝broken0。余40次为3条原版海路末端几何，非海路不完整0；不是当前GUI／原子scope／性能矩阵通过。
- 自有79444正常退出、五输入保持，无游戏、不push；原生通行正式收集／3处末端／改图／生命周期／性能／最终A01–A27继续。详见STATUS R179及checkpoint2026-10-31-r179-physical-domestic-allocation，before内有R178两DLL与部署入口可回滚。


### 2026-10-31 — R178：跨地区真实道路转接，缺段1395→556次

- 当前同市场跨地区实体道路进入转接搜索，节点保留地区身份，仅由真实连接跨区，反向定义正确绑定；不合并其他市场／无连接重号点，不造线或调用新ABI。绕行用例先失败后通过；修正旧夹具rail key／strip类型位不一致，未弱化生产校验。
- 完整构建／统一58 PASS／embedded／默认debug零顶点通过，主体fc674422…／50721811B、引导71dae36d…／779879B已部署实载。原暂停存档69组／839次缺转接消失，总1395→556；58810个已知完整接缝broken0（排除不完整腿），伦敦564次仍连续，数量保持。外部重放不替代GUI／原子scope／性能证明。
- 两自有会话正常退出、五输入保持、无游戏、不push。余14组／521次转接及35次部分海路继续；州级先分配后补路仍需与物理可达性闭环，原生海峡正式收集及改图／寿命／最终矩阵未完成。详见STATUS R178及checkpoint2026-10-31-r178-transfer-gap-audit。


### 2026-10-31 — R177：原版无key港内路径已接入生产／绘制，伦敦564次控制点连续

- 按R176证据实现NativePortTransfer：只接受当前kind5／definition type5／keyMAX且typed节点角色为陆港1与海军8／9／10、两端同当前region、陆港省份完整运行州与州E78一致的原版连接。复制完整连接ID、原版方向anchor、固定起点及原版[累计距离,X,Z]路径；数组／类型／角色／地区／省份／路径端点与距离校验和outer reread均保留。其他kind5不作为陆路放开，无新引擎调用、无硬编码地图表。
- 模型按同地区同市场转接图接入港内路径，端点通过与原版坐标同一floor量化校验后对齐当前精确样条控制，内部点保留原版路径。复制几何使用自有bit63命名空间并保存在NativeDomesticModel，正式ribbon读取自有路径，不修改原生SplineDB。sea筛选包含港内接入段；距离来自原版末记录1e5单位，费用系数1是模型假设，数量仍按最终货流一次，Freight需求账不递归／不重复。
- 初始港内接入用例先失败后通过；新增角色／省份／地区／数组／负距离／端点错误及源销毁用例、正反向路径／真实距离／正式ribbon用例通过。补货运引擎指纹中的localLand和portTransfers；同revision路径变化用例先失败(cache-before.log)，修复后进入统一套件通过。完整构建、统一58 PASS／零FAIL／退出0、embedded PASS和默认debug零顶点PASS。
- 主体732a7e5e3bbf254830f8bb64374d73f08de97310e174c226614beec78f0baa7e／50666726B，引导71dae36d…／779879B，经deploy.ps1安装，dist＝安装＝实际73120模块。普通续玩／暂停／date59883804中，外部只读生产解码及复制后重放取得372条港内路径、全部有绘图几何、最终使用1282次，伦敦使用564次。逐有向控制点核对伦敦564次，broken0；缺转接由R175的2642次降到1395次。顺序外部观察不是原子原生快照，数量改善不等于其他1395次全部已修复。
- 正式市场页及伦敦海陆流线实见(london-flows／london-port-close.png)，卡片26483.90517／铁路510.99 MU保持；港口附近原版旗标遮挡部分局部画面，564次端点连续的精确结论来自只读重放，不把截图当逐像素接缝证明。第一批UI脚本首次前台guard拒绝点击，PowerShell仍继续后续步骤；截图核对后改为每步退出码守卫重执行，未将首次步骤当成功验收。
- 自有73120全程观察暂停／同日期，正常WM_CLOSE退出0；五受保护输入字节保持，扩展相关错误0、六条Unexpected token与既有基线一致，无游戏。未保存进度、未提交／push。原版港内连接接入子项完成；其他缺转接、源／终端州内路径、海峡正式权限、新地图／寿命／性能及最终A01–A27仍继续，不宣称全部道路断点消失。
- 证据E:/Code/v3-analysis/checkpoints/2026-10-31-r177-native-port-transfer/{before-test.log,cache-before.log,after-test.log,build.log,tests.log,embedded.log,debug-default.log,deploy.log,live/continuity.log,live/loaded-modules.json,live/london-port-close.png,live/close.json,live/after.json}；源码前备份before。

### 2026-10-31 — R176：找到原版无样条key的港内连接及实际路径

- 延续港口断点调查，当前源码／两DLL仍R175 e20043d3…／71dae36d…，本轮无产品代码／构建／部署变化、不push。Ghidra确认当前victoria3.exe／image140000000，独立PE精确SHA7f53e326…核验；R175完整58 PASS仅作为同产品历史证据，未重复整套测试。
- 当前原版splnet没有8390108→1503连接；旧gen_splines.py按geo后缀＋locator／gap阈值生成短桥，不能直接用于当前运行地图。进一步独立PE RIP引用扫描及.pdata边界找到141580FF0：陆路后缀3映射角色1，海军同region陆地区域映射角色10，分别写CTravelNodeDefinition+40完整anchor、+58当前region指针、+60角色位集；保留当前+44省份和+48／50原版固定坐标，不建立硬编码ID地图表。该静态语义由汇编归档交叉核对，未主动调用引擎。
- 只读探针在正常暂停会话54216枚举6112个当前CTravelConnection，复读字段并检查当前世界后释放进程句柄。实际372条港内连接均kind5、definition key UINT64_MAX、path_count1；陆路角色bit1与海军bit8／9／10共享当前region。伦敦原版连接为8390108(role400)→1503(role2)，definition type5，原版起点固定坐标407200000／277900000、终点407200000／278100000，路径第一／末记录均[200000,407200000,278100000]。这是引擎当前保存的实际连接／路径，不是最近点或自造直线；外部顺序观察仍不是原子原生scope证明。
- 修正调查中的窄角色掩码：初版仅含bit8／9，漏掉此处沿海角色bit10，报告bridge0；按141580FF0核对后加入bit10，372条全部匹配，初版与最终port-scan日志均保留。1412A0910及141729250跳表证明definition type2–6走统一运行kind5分支，因此kind5不能统一解读成不可用；也不能因本病例而把全部kind5作为陆路。生产必须单独识别同region的真实港内连接，验证两个typed端点、当前省份／州／市场、路径与角色，再复制自有几何。
- 自有54216仅正常续玩、全程观察暂停／date59883804，没有界面输入／推进／保存；正常WM_CLOSE退出0，五输入字节保持，扩展相关错误0、六条Unexpected token与基线一致。实际模块与安装／dist匹配。归档反编译首尝试超过Windows命令长度206而未执行，随后用编辑工具写入外部静态证据成功；Ghidra xrefs／字符串空结果不作为无调用／无字符串证明。
- 本单元完成港内连接来源定位，正式断点仍未修复。下一步将该原版路径单独接入原子快照／自有几何／转接及距离统计，补异常、跨市场／分属、路径变化与回归，再实机验收伦敦海路→中英格兰连续。禁止仅按keyMAX、kind5或固定角色后缀造线。证据E:/Code/v3-analysis/checkpoints/2026-10-31-r176-port-node-relation/{static-decompilation.json,node-builder-assembly.txt,node-vtable-refs.json,connection-predicate-table.json,port_probe.cpp,port-audit.json,live/port-scan-final.log,live/loaded-modules.json,live/close.json,live/after.json}。

### 2026-10-31 — R175：同地区同市场分属州的物理转接，剩余断点继续

- 用户截图补充分属地区仍割裂。R174按完整运行州隔离localLand，导致相同物理地区内另一个分属州的可用道路不能进入转接。新增同地区同市场分属州用例先失败（before-test.log），再修复。按当前market＋region definition＋geo三元身份组织转接，沿用完整运行州作为订单身份；无地区见证退回精确本州，未知不授权共享。没有写死州ID→地图表。
- 转接同时纳入当前同地区同市场的跨运行州land连接，可以实际跨分属边界再返回，而非把两个逻辑州强制合并。不同市场／不同地区／未知定义用例拒绝补接；保持原数量账，路径仍为现有可用样条，缺线继续不完整。新增两段跨属地实体路段用例验证最终四腿路径。经济分配仍为原有州级观察模型，不把该模型转接宣称原版逐笔贸易路线。
- 完整构建及统一58 PASS／零FAIL／退出0，embedded PASS。当前主体e20043d34ec5c1ef0d051fcec89ffcfa16a50ba1751a949c9852561b074d6b5d／49780394B，引导71dae36d…／779879B，dist＝deploy.ps1安装＝实际26228模块。原有未提交源码保持本地，不push；没有本轮重新执行独立默认debug测试。
- 同一原存档／date59883804／暂停，独立只读复制后模型重放识别60组同市场分属地区；转接腿使用16373次、缺转接2642次，R174同存档观察为15690／2936。外部顺序复制不是原子原生快照，计数改善不等同所有路径修复。经原版地点查找器定位海得拉巴，再打开英国市场物流页，印度道路流线及26483.90517／510.99 MU卡片实见（india-flows.png）；镜头范围和色彩不同于用户截图，未逐断点同镜头对照，不将其当全部可视验收。
- 自有26228会话观察始终暂停／同日期，正常WM_CLOSE退出0，原save／settings／ini／content／continue五输入字节保持；扩展相关资源错误0，六条Unexpected token与基线一致。当前无游戏。测试未保存推进、未修改游戏经济。初始Ghidra复核025E8070只说明原海路的海军锚点规则，字符串搜索没有返回，不据此建立海陆锚点关系或启用新ABI。
- 本单元修正转接范围过窄，但仍有2642次缺转接（非唯一断点数），伦敦海军／陆路港口位置差异尚未解决；其他不连续场景、真实港口关系、海峡权限和最终矩阵继续。证据E:/Code/v3-analysis/checkpoints/2026-10-31-r175-split-state-transfer/{before-test.log,after-test.log,build.log,tests.log,embedded.log,live/split.log,live/india-flows.png,live/loaded-modules.json,live/close.json,live/after.json}；源码前备份before。

### 2026-10-31 — R174：州内陆路转接补入最终路径；伦敦海陆锚点仍待连接

- 用户报告海路到伦敦周边再到中英格兰时箭头断开。静态定位DecodeNativeDomesticSources丢弃同州道路，BuildNativeDomesticModel以整州节点分配，已选各段的物理端点不一定相接。新增海路→州内道路→跨州陆路用例先失败（before-test.log），不是仅按截图猜测。
- 采集localLand保留当前已验证可用的同州type2／3／4连接，单独纳入内容身份，不制造自发自收货流；模型按连续两段的真实进出控制点，在该州当前连接上用同费率选择州内转接段并缓存。最终shipment插入有向自有样条，数量及目标／满足账保持，新增距离、费用、工作量计入shipment。无当前道路时插入不完整标记、保留已有片段及数量，不造直线。该转接是国内观察模型，不称原版逐笔已选路径。
- 新端点逐段连续及缺道路用例、原版逐行解码器的同州道路／铁路保留用例通过；原有混合方式用例自身无州内连接，调整为明确断点的三段路径断言，原有数量／筛选仍通过。完整构建和统一58 PASS／退出0、embedded PASS；主体9a795e01572823c0995709d7210d50aba4d37b1002a7e69c209d5f6a0794775e／49592982B，引导71dae36d…／779879B。经deploy.ps1安装，dist＝安装＝实际71336模块。本轮没有重新执行独立默认debug测试，不沿用旧构建结论。
- 当前存档独立只读复制后关闭句柄的重放取得1394条州内连接，模型最终货流使用15690次转接腿，另2936次缺合法转接；这是顺序外部观察，不是原子原生快照或完整可视验收。伦敦运行州5的四条localLand对应控制点1500–1504，实际使用0次。海军港口锚点8390108坐标4072.53589／2779.91675，陆路端点1503为4072.21387／2781.49634，既非同一控制ID也非同坐标；因此新增州内路径仍不能单独接通用户的伦敦海陆例子。禁止将最近点／硬编码ID绑定或随意直线升级为正式连接。
- 实机仅核对正常加载与生产采集，没有完成伦敦连接可视验收。自有71336始终所观察日期59883804／暂停，正常WM_CLOSE退出0；save／settings／ini／content／continue五输入字节保持，相关扩展资源错误0、六条Unexpected token与既有基线一致。当前无游戏；用户不push要求继续，未提交／推送。
- 下一步查原版当前港口的海军锚点与陆路港口节点之间的实际关系／几何，完成该端点后再对伦敦→中英格兰实际连续路径验收。海峡权限等原有未完成项继续，不能宣称本Bug全修复。证据E:/Code/v3-analysis/checkpoints/2026-10-31-r174-land-continuity/{before-test.log,after-test.log,build.log,tests.log,embedded.log,live/continuity.log,live/anchors.log,live/loaded-modules.json,live/close.json,live/after.json}；源码前备份before。

### 2026-10-30 — R173：恢复构建／统一回归及正常存档加载，当前会话保留

- 用户明确恢复正常启动游戏测试，覆盖此前资源占用期的离线／不测试限制；不push要求继续有效。本轮未继续GUI队列改码，先验证R169–R172累计改动。完整native/build.ps1退出0、统一build_tests.ps1为58 PASS／零FAIL／退出0，新增完整航线绑定、快照邮箱及会话退休用例均进入原有套件并通过；verify_embedded和test_debug_default也退出0。默认关闭ImGui的独立实际backend记录零顶点。未发生需修复的编译或测试失败。
- 经deploy.ps1安装，当前主体c82435daf55b0a00f7fbcb1a374a6e5c0d0492f4d44c05f842f022c9f9a8ec99／48822550B，引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B，dist＝安装。正常EXE续玩入口、不加debug_mode，实际PID39984加载两份对应DLL；初始phase4／暂停／date59883804。原ini保持probe_access0，未强启未验证的生产方。
- 初始市场0物流页截图实见原版贸易＋国内模型量26483.90517、已铺铁路510.99 MU、逐商品数量／占比及地图线，生产日志显示owner10／frame1／220条ribbon。此为当前构建加载及既有正式路径回归，不是原生完整通行批次已接入：正式overlay仍未提供新批次，海峡故障、受限重建、已有航线更新及全生命周期矩阵继续。
- 随后的悬停／关闭步骤期间，实际日期推进至59885904且暂停状态在0／1间变化，截图相机、面板及日志owner也变化，来源未确定；本轮脚本没有主动解暂停或推进命令。停止进一步自动输入／退出，保留仍在运行的39984，不把这段混合操作视为受控关闭重开／悬停不改统计／性能验收，也不声称正常退出。关闭截图实际为海区面板，不能按文件名当作成功关闭复现。
- 截至只读audit，原存档／settings／ini／content／continue五输入哈希均与启动前一致，两份dist／安装／实际模块一致，采样error.log中扩展相关错误0；会话未结束，这不替代最终退出后检查或所有游戏错误对照。原存档保护基线仍8008fc75…。未修改经济、未保存进度、未提交或推送。证据E:/Code/v3-analysis/checkpoints/2026-10-30-r173-offline-changes-validation/{build.log,tests.log,embedded.log,debug-default.log,deploy.log,audit.json,live/loaded-modules.json,live/logistics.png,live/session-observations.jsonl}。
- 当前目标继续：真实原生收集／退休事件驱动、GUI排队写入和原生提交边界仍待，三处港口末端／改图／性能／最终同构建A01–A27未完成。后续可继续只读核对和源码研究；对当前疑似同时操作的会话不争夺输入、不强制关闭或替换DLL。

### 2026-10-30 — R172：批次退休接入会话／帧／统计校验（未构建／未测试）

- 按用户最新要求继续仅本地离线修改，不构建、不运行测试、不操作游戏、不部署、不push。上一轮邮箱检查是源码进展，本轮延续到已交worker及地图的会话结果。所有R169–R172改动仍未经编译／执行，不继承R167历史PASS。
- LogisticsSession在适配层保留自有原子代数及期望值，worker仍只收到原有不可变数值快照。SetSnapshot拒绝过期或不完整见证；HasEquivalentSnapshot要求同token／同代数，避免相同内容跨集合寿命跳过重新绑定。Overlay将采集结果的见证沿PublishRealEconomy传给会话；旧无见证路径保留。
- SnapshotIsCurrent、FrameIsCurrent、StatisticsAreCurrent及Frame／Status增加代数复核，退休后可在下一次相应检查时拒绝已有结果，不等待UI合并。Poll在提交工作前撤下退休快照／帧并取消worker；消费回复及发布文档前再次复核，原有owner选择保留，当前替代快照可恢复相同面板。未将批次退休冒充世界变化，未改国内分配、国际数量或游戏经济。
- 在已接统一入口的logistics_session_tests写入待执行用例：真实worker产出后的帧／统计退休、相同数字不同token、同内容定时采集、退休后替换恢复、未完成回复拒绝、不完整见证和世界退休不可绕过。用例尚未运行，静态阅读与diff空白检查不是并发／渲染／UI验收。
- 原生收集器仍未填充批次及驱动失效token；已有航线更新、受限重建、首次绑定前退休／ABA、GUI已排队写入及原生提交边界的完整失效时序仍待核对。原子检查与后续使用之间不是全链事务，不宣称所有竞态消失或海峡已修复。备份E:/Code/v3-analysis/checkpoints/2026-10-30-r172-session-lane-retirement/before；资源可用后先构建并执行相关统一回归。

### 2026-10-30 — R171：逐航线批次的邮箱发布／取出失效检查（未构建／未测试）

- 本轮延续只离线改代码、不测试、不push要求。前一轮完成可选采集入口，属于源码进展；本轮补其返回后到邮箱和UI交接间的失效检查。未编译、未运行测试、未操作游戏、未部署／推送；当前修改仍全部保留本地，没有新增PASS。
- 将R170任意current回调简化为共享自有atomic代数见证；输入同时匹配world／revision／generation，生产方必须单调推进代数、冻结批次，不得恢复旧值或跨集合寿命复用token。NativeCaptureResult仅在适配层保留token及期望代数，worker的EconomySnapshot继续只含数值副本。
- Publish先保留既有请求身份和版本顺序检查，再拒绝已退休批次并通过既有错误通道报告；TakeError和Take复核未取副本，NativeCaptureMatches复核已取的保留副本。迟到旧版本在触及当前结果前拒绝，避免旧失败清除新结果；无批次见证的历史调用仍按原路径处理，缺一半见证则拒绝。
- 已在统一入口现有native_snapshot_tests写入待执行用例：发布后未取即退休、取出后退休、返回到发布间退休、错误检查到取出间退休、新版本重试、旧结果不清除新结果、缺token／缺代数及源释放后的自有存储。只做文本审阅，尚未执行这些用例，不能宣称并发或生命周期验收。
- 正式原生收集器尚未绑定新输入，也尚未驱动这些token；首次绑定前退休、已有航线更新、受限重建、已发布worker／原生帧即时撤回及全部竞态仍需完成。顺序原子读取不是全链事务，不以本轮邮箱校验替代这些要求；现行海峡显示故障未验收。备份E:/Code/v3-analysis/checkpoints/2026-10-30-r171-lane-epoch-publication/before；资源可用后先构建并执行R169–R171相关回归。

### 2026-10-30 — R170：采集入口接入自有逐航线批次（未构建／未测试）

- 延续用户仅离线改代码、暂不测试及不push的最新要求。本轮只改源码与记录，未编译、未运行测试、未启动／操作游戏、未部署／推送；上一轮是源码进展，本轮继续推进可选交接入口。两轮源码均未取得新增PASS，现有DLL保持历史产物。
- NativeLaneAccessInput仅在适配层持有不可变自有批次、独立采集代数及只读自有epoch的current回调；世界／版本／代数不匹配或缺少有效性见证即拒绝。CopyNativeSnapshot新增默认空的可选输入，采集前、绑定时和全部复制完成返回前核对有效性；按本次复制的航线及海峡注册表执行R169完整绑定，worker只获得复制后的逐航线允许列表，不获得回调或引擎指针。
- 明确拒绝同时传入逐航线批次与旧国家权限回调，避免混用两个时点的语义。旧调用点参数不变，正式overlay仍未提供新输入；未启用任何未经验证的原生函数，现行注册回退仍未修复。持有shared_ptr不延长批次的有效代数，生产方必须冻结批次并独立维护失效事件。
- 在既有native_snapshot_tests增加待执行用例，覆盖世界／版本／代数不匹配、见证缺失／退休、源释放后自有所有权、零身份、冲突来源，以及无效批次在读取引擎前拒绝。仅文本审阅；整份成功采集及复制中失效路径尚缺执行证据，不把入口拒绝用例等同完整快照验收。
- 下一步仍需原版创建和已有航线更新的正式收集、首次绑定前退休／ABA、受限重建与当前集合代数；返回后到邮箱发布的批次退休、当前帧撤回也未闭环，不能只凭本次返回前复核启用。资源可用后先构建并执行R169／R170相关回归，再补真实生产方接线及最终验收。源码前备份E:/Code/v3-analysis/checkpoints/2026-10-30-r170-snapshot-lane-access/before。

### 2026-10-30 — R169：逐航线通行结果交接与消费代码（未构建／未测试）

- 用户本轮要求仅离线改代码，暂不测试以保留资源；随后明确不需要push。本轮未编译、未运行任何测试、未启动或操作游戏、未部署、未推送，改动保留本地。现有dist／安装未由本轮更新，R167的58 PASS不适用于新增源码。
- 新增native_lane_access.h，将既有自有分类／已选航线值类型从诊断头移入共享头；BindNativeLaneAccess按独立传入的world／revision／collection generation、完整lane句柄、国家、两端州、64位距离及全部省份范围匹配当前副本。完整允许／拒绝集合必须无重复且恰好覆盖当前海峡注册表；缺失／重复／过期或同句柄内容变化整批拒绝。输出为按完整航线句柄索引的自有允许列表，不写死地图身份、不按国家借用其他航线的结果。
- NativeDomesticSources新增可选laneStraitAccess，国内几何消费优先使用逐航线结果；字段存在但缺某条非空航线时拒绝，不回退国家表／注册表。该字段加入NativeTradeEquivalent，权限变化可使缓存不等价；国内模型数量口径不变。原生采集器尚未填充新字段，现行注册回退仍存在，本轮不宣称海峡已修复。
- 在已接统一入口的native_access_tests与native_trade_view_tests写入待执行用例：世界／版本／代数不匹配、句柄与港口／距离／成员变化、源销毁、完整分类、同国两航线不同结果、缓存失效与缺记录拒绝。只做文本审阅，未执行复现或回归，未取得任何新增PASS。
- 交接函数不证明引擎访问租约或原子性；未来生产方必须从独立当前采集状态取得expected，并在发布前再次确认。原版创建／已有航线更新的完整收集、首次绑定前世界退休、受限重建、批次寿命与正式采集接线仍待；不得把诊断采样代数直接当正式集合代数。备份E:/Code/v3-analysis/checkpoints/2026-10-30-r169-lane-access-handoff/before；下一步待资源可用后先构建并运行相关回归，再继续生产方接线。

### 2026-10-30 — R168：按用户要求同步当前开发源码到GitHub

- 用户明确要求更新完成后push。核对现有main／origin及docs/GITHUB.md的既有公开上传范围，使用已有NoFires SSH远程，不新建仓库、不改写历史。fetch确认基线与origin/main一致后，将自前次发布以来的物流源码、测试、GUI／shader与记录整理为源码提交a688e34c709423a7c56ffece7ab309f1edd744dc：90文件、10466增加／199删除；其中包括当前R167默认关闭的分类／最终航线创建观察与回归。
- 提交前核对范围、文本文件与高置信凭据模式、索引内容及git diff --cached --check；dist、native/build、日志、存档、外部v3-analysis与本地凭据不进入提交。Git原有autocrlf给出LF→CRLF提示，未执行checkout或批量改写源文件。编译产物仍按既有规则独立交付，不作为Git源码上传。
- git push origin main成功，随后git ls-remote返回a688e34c709423a7c56ffece7ab309f1edd744dc，与本地HEAD完全一致，工作区干净；该成功发生在本记录之前。本记录及GitHub说明作为后续文档提交同步，最终远端核对留在外部checkpoint。当前产品仍R167的a400dd88…／71dae36d…，统一58 PASS为同产品证据；本单元只改记录，不重跑无变化的产品测试，不宣称正式海峡接入或完整目标完成。
- 现行约束已改为每次完成更新后核对测试和提交范围并推送已有远程，同时保留源码修改前checkpoint备份。后续继续原版结果的世界／请求交接、失效与正式几何消费。证据E:/Code/v3-analysis/checkpoints/2026-10-30-r168-git-sync/以及R167的git-candidate-audit.json／git-staged-check.txt。

### 2026-10-30 — R167：完整通行分类绑定原版最终创建航线的实机观察

- 核对原版141419DA0→14140DF60→14140FAA0：反编译器漏列创建包装函数第九参数，汇编确认其为原版距离值，再按引用交给原始构造器；最终州方向按构造器的实际from／to参数保存，不按候选遍历方向猜测。独立进程执行精确EXE的14140DF60原机器码，10例覆盖完整州ID、反向／零州、64位大距离、类型与无效返回；构造器、有效性和引擎服务为替身，不是实际分配器／数据库／寿命证明。
- 默认关闭的probe_access新增14140D830国内重建调用范围和14140FAA0九指针构造器观察。只转发自然原函数一次；同一嵌套线程范围内收集完整分类，国家／模式不符、同次重建分类变化、复制失败或采样超限即拒绝。构造前最终输入、返回的完整句柄对象和重建结束时当前类型化DB对象三次核对国家、完整州、原版距离和全部压缩省份范围；不选候选、不按country跨调用复用权限、不保留指针到worker、不发布正式结果。
- CopyNativeLaneSelection新增到既有probe头，源销毁、完整句柄／端点顺序、距离64位／零／负、非法与变化范围、复制中变化等用例接入已有native_access_tests。完整构建后复核增加异常／超限时整次失效及当前对象类型检查，按相同参数重编overlay并经ResourcesOnly守卫重链；最终统一58 PASS／零FAIL／退出0及embedded PASS。当前主体a400dd882243ccc0c8e439ba4324c74b9bb4bedbe2aa37049cfbc633624db3bb／48590180B，引导71dae36d…／779879B；源码、dist、安装及实际49244模块对应。
- 同一原存档会话49244记录185条不同完整lane句柄、17国家、33次重建代数，全部matched=1且绑定拒绝0：date59883804有93条，后续date59883912有92条。原版英国lane218103823/from5/to547/distance447262046在generation1创建；后续generation286为lane234881040/from5/to551/distance469386716，两者省份集合也不同。这将孟买→西孟加拉的变化绑定到同进程实际原版创建结果，未强制选港。当前记录均为mode1的九条允许／零拒绝，尚未验证受限状态下的最终创建绑定。
- 外部独立只读lane探针构建成功，但执行返回4，随后确认49244已不存在；日志有世界退休、未见新crash目录，退出原因及退出码未知，不能称本轮由我方正常WM_CLOSE退出，也不能称完成外部逐航线复核。会话后续存在UI发布与日期推进，仅记录原生日志事实，不把未知操作者或这些操作外推为交互／性能验收。finish核对五输入保持、扩展相关资源错误0、六条Unexpected token与基线一致；最终恢复原ini SHA256bbc678fb…／probe_access0，当前无游戏。
- 本单元完成原版最终航线生成边界的观察核对，仍未将诊断数据交给正式几何。世界／请求原子交接、旧代数拒绝、已有航线更新与受限重算、三处港口末端及最终矩阵继续；不宣称海峡故障已修复。证据E:/Code/v3-analysis/checkpoints/2026-10-30-r167-native-lane-binding/{before,caller-assembly.txt,create-assembly.txt,constructor-assembly.txt,create_harness.py,creation-harness-result.txt,build.log,incremental-build.log,tests.log,embedded.log,after.json,live/binding-records.json,live/session-freight.log}。
- 用户最新要求完成更新后push到Git。现有仓库main跟踪origin/main，远程为NoFires/victoria3-logistics-system，fetch后基线0 ahead／0 behind；修正本文件旧“无Git”约束，仍保留源码前checkpoint备份。按既有公开范围整理此前未提交源码、测试和记录，排除dist／native/build／日志／外部私有证据；实际提交与远端验证单独记录。

### 2026-10-30 — R166：原版完整海峡分类输出的自然调用复制与实机开关核对

- 在既有默认关闭的probe_access下增加原版14141FDA0观察钩子，按精确EXE及13字节入口签名核验后转发原始四参调用一次；借用数组仅在返回后、调用方尚持有时复制。新native_strait_probe.h保留完整句柄及顺序、排除调用前已有前缀，限制长度并复读描述符／内容；重复、交叉允许拒绝、非法句柄、不可读和观察到的变化均拒绝。线程局部逐调用trace与同次140BE7C10自然返回逐项比较，异常离开通过RAII恢复前级trace。无主动权限调用、无指针交worker、无正式权限发布。
- 生产复制器的异常输入与源销毁测试加入已有native_access_tests统一入口。完整构建后首轮58 PASS；复核补上异常trace恢复／地址边界与hidden复核，按相同编译参数重编overlay并经ResourcesOnly守卫重链，最终统一58 PASS／零FAIL／退出0、embedded PASS、默认debug零顶点通过。当前主体23b812d88ea7d92a6374cf91f9b7422e3cbad8cb7ae9e90a57fb878a368faf51／48381144B，引导71dae36d…／779879B；源码、dist、安装及实际23560模块对应。
- 第二独立暂停会话23560中，通过原版tag/date与博斯普鲁斯面板构造未保存的限制和恢复，接受377份完整分类、283国家、14线程；全部caller RVA0141F236／mode1，数组与同次自然谓词返回精确一致，复制拒绝0。47国家均有同world／同完整海峡1的允许→拒绝→允许；英国country1在classifier sample5／6862／9709、日期59883804／59888160／59892864分别为[9允许,0拒绝]→[8允许,1拒绝]→[9允许,0拒绝]。不是仅看UI控制位。该人为日期测试不代表正常模拟速度或所有运河建筑／外交分支。
- 288份观测为access2／owns1／context1，另89份为access2／owns0／context0；原版分类的自然调用具有不同线程／上下文，不能据此授权快照线程主动调用。完整分类复制仍不等于世界／请求原子交接或航线绑定；诊断map只作有界变化日志，不能直接充当正式权限缓存。国内注册回退及三处港口末端未修复，下一步在原版选路／航线生成边界绑定完整结果与world／revision／generation并验证过期拒绝。
- 首局67280的快捷脚本未改变日期，截图未建立预期海峡状态，三段audit实际FAIL；该局只接受283份初始分类，不能当开关成功。稍后进程在明确close前已消失，日志有世界退休、未见新crash目录、Application/WER查询无返回条目，但退出原因／退出码未知。第二局改逐步确认控制台、tag和日期，完成上述三段，并暂停WM_CLOSE正常退出0。两局finish均核对五输入保持，最终恢复原ini SHA256bbc678fb…／probe_access0；扩展相关游戏资源错误0，六条Unexpected token与既有基线一致，不是全部debug错误完全一致。当前无游戏。
- 证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r166-native-classification-live/{before,build.log,incremental-build.log,final-tests.log,embedded.log,debug-default.log,source.diff,after.json,live/second/classification-audit.json,live/second/session-freight.log,live/second/restricted.png,live/second/reopened.png,live/second/close.json}。原版完整分类复制子项完成，正式海峡修复及最终交付仍继续。

### 2026-10-30 — R165：原版海峡分类输出与追加语义的隔离验证

- 在独立进程核验精确EXE SHA2567f53e326…，复制原版14141FDA0分类机器码及140BE7C10／140BE7E10权限机器码，保留相对布局；合成世界、海峡、国家、关系／有效性替身和预分配输出缓冲区。原版分类输出R8为允许、R9为拒绝，保留完整32位句柄及原版遍历顺序；36例覆盖三海峡六种排列、两种模式与三个国家（含带代数句柄），10例底层权限对照也通过。输入逐字节不变，输出计数与哨兵边界符合预期。
- 额外验证原版分类函数会保留已有输出前缀并追加，每次调用不清空：连续两次调用正确累加；空注册表不触碰原有输出。正式采集若假设其输出天然替换或复用旧数组，将混入旧结果；实际调用方14141F1D0先构造空数组的现有静态证据与此一致。该fixture容量足够，未走原版分配器增长；外交／条约和建筑依赖仍为替身，不是完整原生ABI／实际游戏寿命证明。
- 进一步静态核对14140D830调用141419DA0的stack参数与胜出结果到14140DF60构造链，指出绑定当前航线须同时携完整州、路径与距离，不能只按country缓存零散谓词；尚未安装这些新钩子或接正式权限结果。现有正式国内注册回退问题继续。
- 首次独立夹具将浮点常量地址误写为过大RVA，VirtualAlloc断言在执行原版代码前拒绝；修正到0486ADB0及对应页后全部通过。无游戏操作、产品源码／DLL／构建／部署变化，当前仍8d2f0d92…／71dae36d…；R164的58 PASS仅历史同产品证据。下一步在原版合法调用边界复制完整分类并绑定航线，验证世界／请求代数与一致性，再进入正式几何消费。
- 证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r165-native-classification-output/{classification_harness.py,result.txt}。本单元是隔离输出语义验证，不是实机海峡修复或全部目标完成。


### 2026-10-30 — R164：原版海峡函数完整允许→拒绝→允许实机时序

- 修正默认关闭的probe_access观察器：记录某国家／海峡／模式的最新返回变化，移除相反旧返回的去重项，并将事件总数限制8192（仍限150000次自然调用／4096组合、忙锁丢样）。不主动调用原版、不保存引擎指针、不把诊断结果发布给正式物流。完整构建、统一入口58 PASS（含R161换港回归）、embedded PASS；当前主体8d2f0d9291765da8091f688ee84620027b02226eb0f3e8a947a604400ad2a6ac、引导71dae36d…，dist＝安装＝实际48556加载。
- 两个独立暂停测试会话均使用游戏自带tag/date命令和原版博斯普鲁斯面板：切TUR，日期1836-07-20后设置封锁／仅控制国，日期1837-02-01后重新开放；所有改变仅在未保存的诊断会话。最终48556中，同英国country1、海峡完整句柄1、mode1，自然原版返回在sample46为1、92204为0、126825为1，脚本断言三段顺序通过。这是原版实际调用证据，不是合成权限回调或仅看UI字段；不推广到运河建筑、所有外交分支或快照线程主动调用安全。
- 第一会话54560首个前台guard拒绝按键，而旧PowerShell批次仍继续后续动作，未构造出预期状态；检查截图后使用逐步check=True脚本恢复。该局只取得1→0，第三段未记录，不作成功证据。随后正常关闭并复用同构建新会话48556，以受控脚本完成全部三段；driver.log与两次trace保留。未经验证的首次操作不当作模型／权限变更验收。
- 两个自有会话均暂停WM_CLOSE退出0；finish核对原存档／设置／临时ini／content／continue字节保持，结束后恢复原ini SHA256bbc678fb…／probe_access0。扩展相关错误0、六条Unexpected token与基线一致；不宣称全部debug日志相同。本轮未验证该新构建正式地图／性能、原子结果收集或寿命，完整交付继续。
- 原版函数的真实权限翻转子项完成，正式物流仍用旧注册回退、未接入诊断数据，海峡显示故障尚未修复。下一步验证自有结果收集与世界／请求代数、同原生完成点交接、过期拒绝和性能，不能将诊断集合直接升级为正式权限表。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r164-strait-return-transition/{build.log,tests.log,live/second/return-trace.json,live/second/session-freight.log,live/second/after.json,live/second/driver.log}。


### 2026-10-30 — R163：受限海峡原版自然调用的真实允许→拒绝

- 沿用当前e65d7a42…主体／71dae36d…引导，临时probe_access1及-debug_mode，在自有暂停会话通过原版console切TUR，通过原版博斯普鲁斯海峡面板操作。原存档海峡已有非军事状态及冷却至1836-07-11，初次封锁点击被拒；经help date核对游戏自带日期命令后，仅在未保存会话将日期设1836-07-20，再通过面板设置封锁／仅实控国。此为人为构造的诊断状态，不是正常模拟推进或生产经济改动。
- 第一会话18820取得面板状态但没有拒绝诊断记录，正常退出后以同构建新会话71424重新执行；第二局自然原版mode1结果中，当前海峡完整句柄1／定义4先有284国允许，后有47国拒绝，47国均观察到允许→拒绝，包括英国country1；控制国country95有允许、未见拒绝。首次拒绝sample87002，未外部主动调用引擎；原生调用本身有真实受限分支证据，但不代表异线程主动调用安全或原子采集。
- 外部顺序只读字段在受限态为[2,3,0,0]，marker0；随后日期命令改至1837-02-01，面板重新开放，字段恢复[0,0,0,0]。重复允许结果被现有诊断去重，未取得重新开放后的第三次函数返回trace；不得宣称完整允许→拒绝→允许函数时序验收。只读脚本首次误将read返回的单值当四元组，TypeError后改逐字节读取成功，未写进程。
- 两个自有暂停会话均WM_CLOSE正常退出0；finish核对原save／settings／临时ini／content／continue字节保持，最后恢复原始ini（probe_access0、bbc678fb…）。扩展相关错误0，六条Unexpected token与基线一致；不是全部debug日志无变化的结论。未保存日期、国家或海峡修改，无产品源码／构建／部署变动，未重跑统一入口。
- 当前自然结果尚未接正式物流，现有注册即允许缺口仍未修复；下一步基于真实拒绝病例验证结果收集、世界／权限代数及提交一致性，保留ABI与调用窗口保护，继续运河建筑分支、末端几何及最终验收。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r163-strait-transition/{transition-audit.json,live/second/session-freight.log,live/second/restricted-controls.json,live/second/reopened-controls.json,live/second/restricted.png,live/second/reopened.png}。


### 2026-10-30 — R162：无扩展原版复现印度初始港口重算

- 在确认用户游戏已退出后，通过deploy.ps1 -Uninstall卸载当前双DLL，正常启动当前续玩存档。实际进程71508仅加载C:/Windows/System32/VERSION.dll，无freight.dll，证明此次对照未加载扩展。原版暂停初始date59883804，lane218103823/country1/from5/to547（孟买）；此存档已由用户推进并保存，SHA2568008fc75…，不同于R159初始fecc16f…，不混作同文件。
- 先推进到59883834仍为孟买，后推进至59883930观察到原版lane234881040/from5/to551（西孟加拉）；这复现了无DLL条件下的初始孟买→后续西孟加拉重算。端点改变发生在两次观察之间，不能精确断言具体重算tick，也不能据此证明DLL采集毫无额外延迟；但该现象不依赖扩展，不能将初始孟买本身归因DLL写入。外部探针只读身份枚举，内部路径selector／权限为测试回调，不作为真实选路或原子快照证明。
- 私有会话助手首次SetForegroundWindow失败，后AppActivate与AttachThreadInput也未获得前台，守卫拒绝发键；改为Alt激活前台后操作成功。失败的首次advanced-audit仍处原日期，不当作已推进。全程仅操作自己会话，暂停后WM_CLOSE正常退出0，未保存推进。
- 经deploy.ps1恢复主体e65d7a4296e90c1e17f4edeaa7709d36bb9f32e36f7650b3c961f7c67e628437和引导71dae36d…，安装＝dist。五输入（save/settings/ini/content/continue）字节均保持，本轮无产品源码或构建，未测试恢复后再次实载；当前无游戏。R161换港发布回归已通过，当前原版端点选取不作强制干预；下一步关注真实额外延迟、海峡权限和末端缺几何。调查子项完成不等于完整交付或初始化所有问题关闭。
- 证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r162-stock-port-refresh/{live/stock-modules.json,live/initial-audit.txt,live/advanced2-audit.txt,live/week-audit.txt,live/close.json,after.json}。


### 2026-10-30 — R161：原版港口换点的生产缓存与发布回归

- 在现有 native_trade_view_tests.cpp 增加端到端换港用例：保持同world／同revision／同商品订单与市场，旧国内lane直接接旧物理港口，新lane更换完整句柄与目的港，通过当前陆路继续到相同需求州；新旧港口几何同时保留在输入，避免通过删资源伪造旧线消失。
- 验证初始帧确含旧港口；仅lane变化即可使NativeTradeEquivalent不等价、生产FreightEngine重建facts；同最终数量的新帧只采用新港口并消除旧港口贡献。正式LogisticsSession拒绝等价快照判断，SetSnapshot后旧FrameIsCurrent立即false，worker完成后的新帧落在新港口。最终专项测试编译与整份native贸易视图／会话套件退出0、PASS；test-result.txt留存。首次通过后补强初始旧线存在断言并重跑通过。
- 本轮只改测试，无产品源码／DLL／部署变化，仍为e65d7a42…主体及71dae36d…引导。用例位于已接统一入口的既有测试中；用户16276游戏仍在运行，未执行会部署的完整build_tests.ps1。该隔离回归不证明原版为何初始孟买、不证明采集30秒节流的实际延迟，也不证明无DLL原版行为；只完成自有快照进入生产缓存／worker／发布后的换港子项。后续仍需原版重算时机与采集／发布时间线、海峡权限、三处港口末端及最终验收。
- 未操作用户游戏。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r161-port-retarget-regression/{before,native_trade_view_tests.exe,test-result.txt}。


### 2026-10-30 — R160：用户推进后印度原版航线改为西孟加拉

- 用户补充初始显示孟买、等待游戏时间推进后变为西孟加拉，并确认当前显示正常。对用户正在运行的16276仅执行外部PROCESS_VM_READ，不操作、暂停或关闭游戏；当前原版国内lane201326608/country1/from5/to551，当前州551为STATE_WEST_BENGAL。R159另一个初始续玩会话原版lane50331663为from5/to547（STATE_BOMBAY）；两次不同进程、不同模拟阶段的观察不能证明同一原子变化或DLL因果，但证明变化不仅是显示标签，原版航线身份与端点本身不同。
- 当前生产日志已记录revision6／9053模型货流和235可见路线，用户确认从西孟加拉出发；只读模型重放同为9053笔。外部比较器的selector／权限是测试回调，不代表原版选择调用或正式绘制逐边trace。国内lane完整内容已参与缓存身份比较，现有运行采集有30秒节流；是否造成该场景额外滞后、初始为何保存孟买、无扩展原版是否同样延迟，仍需同存档对照，不宣称初始化Bug已修复或DLL无因果。
- 不将孟买长期端点判为预期最终行为，不硬编码切到孟加拉。本轮无产品源码修改、构建或部署；当前用户会话保持控制权。后续复现先保存原版重算时间／lane代数、DLL采集与正式发布的时间线，确认哪一层滞后再修改。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r160-port-refresh/audit.txt，以及R159初始audit.txt。整体goal继续。


### 2026-10-30 — R159：英国市场印度港口端点报告的原版对照

- 用户指出英国市场印度海运画到葡萄牙孟买，预期孟加拉；随后确认原版航运线路面板也列孟买，并询问是否理论上应为孟加拉。当前原版 GUI sea_region_panel.gui 使用 ShippingLane.GetEndState；静态原版渲染 1425E90F0→1425E8450 取 CShippingLane+48／+4C 两州，现有解码读取同字段，不宜硬改到特定地区。
- 在自行启动的普通续玩进程 176 中，独立 PROCESS_VM_READ 比较器读取当前州定义 key、owner、market、center 及航线：STATE_BOMBAY 546 的 owner30/market11，547 的 owner218/market0；两者同地区不同运行身份。原版国内 lane50331663/country1/from5/to547，未把546当原生端点。本机孟加拉州16／551也在market0，但当前航线未以它们为此连接端点。比较器的路径selector和权限是测试回调，身份枚举不据此证明实际选路／原子性；日志中9038模型货流是隔离重放。
- 该证据排除了直接用“孟买名称／地图地区”认定端点属于葡萄牙，也说明不能按首都直觉改为孟加拉。原版141419DA0静态链比较候选州对成本，具体本存档为何胜出仍未运行追踪。共享地区的实际地图港口落点是否跨入葡萄牙控制省份仍待独立核对；残缺端点的旧逻辑仍是另一待处理问题。本轮未改产品源码、DLL或部署。
- 先前R158中断会话未完成海峡开关验收，未将其称成功；继续前确认无游戏并将临时freight.ini恢复原始bbc678fb…／probe_access0。R159取证后进程176已在收尾前结束，退出原因／退出码未取得；随后发现新进程16276/start11:45:52，非本单元启动，不操作或关闭。finish检查因存在新游戏而拒绝，不能声称本单元正常WM_CLOSE或完成五输入退出审计。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r159-india-ports/live/{audit.txt,coverage-probe.cpp,launch.json}。


### 2026-10-30 — R157：原版海峡结果对当前国内国家的覆盖核对

- 将默认关闭的 probe_access 海峡诊断扩展为最多前 150000 次自然调用、最多 4096 个国家／海峡／模式／结果组合的去重记录；观察锁忙时丢样，不阻塞等待，不保存指针或向正式快照发布。统一入口 58 PASS、verify_embedded PASS；主体 e65d7a4296e90c1e17f4edeaa7709d36bb9f32e36f7650b3c961f7c67e628437，引导 71dae36d…，源码／dist／安装／本次实际模块一致。
- 当前暂停存档、同一进程 69436：诊断日志记录 284 个国家与 9 条海峡共 2556 对，均为 mode1／允许，最后首次记录样本序号 35558。独立外部只读比较器复用当前解码器，读取 17 个国内航线国家及 9 条当前海峡，153 对全部在诊断记录中出现。比较器的选路和通行回调均是测试值，只用于枚举身份，不是原版权限结果或真实选路；日志与外部读取为顺序观察、并非原子快照。不能外推其他存档／国家、世界重载、权限改变或拒绝结果。
- 独立比较器首次链接缺 freight.cpp，第二次缺 log.cpp，补齐后编译通过；记录原始失败过程。本轮只观察暂停初始存档，未实测当前构建正式物流页面或性能。测试会话正常 WM_CLOSE 退出 0，存档／设置／内容／continue SHA256 不变；临时诊断设置已恢复原始 freight.ini SHA256 bbc678fb…／probe_access=0。扩展相关错误 0，六条 Unexpected token 与基线一致，未证明全部日志完全相同。
- 本结果支持继续验证自然结果收集，但不是证明可安全缓存：仍需原版国家／海峡完整句柄验证、同步边界、权限变化、旧世界失效和性能。正式权限回调未启用，受限海峡显示及三处港口末端尚未修复。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r157-native-coverage/{tests.log,live/coverage-probe.txt,live/coverage-audit.json,live/session-freight.log,live/after.json}。


### 2026-10-30 — R156：原版海峡判断自然调用频率实机观察

- 沿用已安装 R155 主体 a341e425…；按该 DLL 的 nm 符号和 PE ImageBase 得到诊断调用计数器 RVA 0x2bc350，先核验完整 DLL SHA256 与实际模块，再以外部 PROCESS_VM_READ 读取，无附加、暂停线程或写进程。该数字仅用于此精确构建的诊断，不写入产品映射。
- 同一自启存档进程 76216 在 phase4/暂停/date59883360：02:55:35.918 为 42333，02:55:52.357 为 51216，02:56:11.436 为 61512，02:56:17.025 为 64518。各次读数是顺序观察；暂停下持续增长表明前 32 条日志远非全部自然调用，但不证明每个国家／海峡已覆盖，也不证明哪一条结果与快照原子一致。
- 之后以速度 1 短暂推进并恢复暂停：date59883360→59883384，02:56:27.387 计数 86496；此区间包括界面操作和暂停，不能当稳定帧率或单一模拟速度基准。大量调用意味着正式被动收集必须限制额外开销，并验证国家覆盖、权限变化与世界退役；不能直接复用诊断日志作为权限缓存。
- 自有进程正常 WM_CLOSE 退出 0，未保存推进；原存档、设置、content_load、continue 的 SHA256 不变，临时 probe_access=1 已恢复原始配置 SHA256 bbc678fb…／0。扩展相关错误 0，六条 Unexpected token 与基线一致；未证明全部日志一致。本轮无源码／DLL构建或部署，沿用 R155 58 PASS；海峡问题、港口末端和最终交付仍未完成。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r156-permission-cadence/live/{count.py,counts.jsonl,session-observations.jsonl,session-freight.log,after.json}。


### 2026-10-30 — R155：原版海峡判断自然调用的实机诊断

- 在 overlay.cpp 的既有 probe_access 调试开关下增加默认关闭的观察钩子：仅转发游戏已发起的 140BE7C10 调用一次，限前 32 条采样；核对精确版本及函数入口字节，不主动调用、保存引擎指针或把结果用于物流。构建后统一入口 58 PASS、verify_embedded PASS；主体 a341e42542373b64bd2511d1bdcc438e4da7036ec86574eac0e269a1383e5d50，引导 71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e，dist＝安装＝实际加载。
- 暂停原存档的自启会话 71040 中，原版线程 9848 自然调用 32 次，caller RVA 141FE52、mode 1，涉及两个国家和九条注册海峡；样本均 marker=0、控制字节全零且返回允许。前后顺序观察均 phase4/access2/owns1/context1，同世界同日期。此会话快照复制线程为 68512，与样本线程不同；同一标志不证明完整调用期、同步、对象寿命或复杂分支可在快照线程安全运行。
- 诊断启用期间市场物流页和商品值可见，暂停会话 WM_CLOSE 正常退出 0，原存档、设置、内容与 continue 输入字节不变；临时 probe_access=1 的配置在退出后恢复原始 SHA256 bbc678fb…／probe_access=0。扩展相关错误 0，六条 Unexpected token 与既有基线一致；不是完整错误日志零差异证明。
- 本次真实调用仅为直接放行样本，不覆盖已建运河、拒绝、外交或条约，正式权限回调仍未绑定，海峡显示问题与三处港口末端仍未修复。下一步采集受限及状态变化时原版判断和其寿命，并验证无额外调用的完整结果收集／失效方案。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r155-natural-permission-probe/{tests.log,build.log,live/probe-audit.json,live/session-freight.log,live/market.png}。


### 2026-10-30 — R154：当前候选双 DLL 实装及原版物流页实机核对

- 经 deploy.ps1 部署现有已通过 R150 离线 58 PASS 的候选，游戏进程 58312 实际加载主体 23914d6503a31fee5ca4521436ae019b46a0ff6ae83028228a8a263b1cc95d66 和引导 71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e；源码未改，本轮未重跑完整测试。之前安装主体 178e039d… 已备份于 before/，当前源码对应的 dist 与安装两份 DLL 哈希一致。
- 自启原存档保持暂停、日期 59883360；实际原版市场物流页显示贸易量 25963.19216、铁路 510.99 MU 与逐商品数据。日志记录 1147 州、53 商品、3192 原版选择、918 陆路／93 国内海路、9038 模型货流、3 条部分海路和 218 可见箭头；这不证明每条几何完整或海峡权限正确。截图及该会话日志留存 live/。
- 自己的暂停会话经 WM_CLOSE 正常退出 0；原存档、设置、freight.ini、content_load.json、continue_game.json 的 SHA256 均未变。游戏日志中未检出 ironroad／freight／native-gui 相关错误，六条 Unexpected token 与既有基线一致；未逐项证明其他日志行完全一致。
- 该测试验证本候选一次启动与当前市场页，不代表海峡权限、港口端点、完整生命周期或 A01–A27 通过；国内权限回调仍未绑定，不能宣称海峡修复。当前无游戏。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r154-candidate-live/。


### 2026-10-30 — R153：国内原版搜索的附加限制参数核对

- 复核本机静态 EXE SHA256 7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f。国内调用链 1414500F0 在常规直线路径将两个临时数组清空，将其组成的结构指针作为第七参数传给 14141F1D0；后者先以 mode 1 调 14141FDA0，按 140BE7C10 的返回值分类，再将第七参数第一数组中的海峡从允许组移入拒绝组。汇编证据分别为 domestic-call-assembly.txt 和 search-classification-assembly.txt。由此可知该国内调用的空附加数组不会额外移除已允许海峡；不外推其他调用者或全部初始化分支。
- 这是静态参数核对，不代表最终实际选中哪条路径。原版判断仍依赖运河建筑、国家关系和调用时上下文；140742920 可初始化 TLS 并获取锁，相关真实生命周期与访问范围仍未证明，正式权限回调仍禁用。国内现有注册回退仍可能错误显示受限海峡，三处港口末端仍缺，不能称修复完成。
- 本轮未改产品代码、构建、部署或操作游戏。下一步围绕原版在真实航线生成期间的自然调用时机与权限数据生命周期取证，不能拿空附加数组或模拟测试替代完整权限和路径验证。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r153-domestic-route-arguments/。


### 2026-10-30 — R152：原版权限两层调用的隔离组合验证

- 在独立进程校验精确EXE 7f53e326…后同时执行原版140BE7C10与140BE7E10原始机器码，以合成对象和替身提供两岸国家、类型有效性与从属关系，未接触游戏进程。10例覆盖同一限制下本国／外国、完整国家ID、两种模式、模式相关限制，以及分别只在一侧施加限制；输入逐字节未变。替身不执行真实外交、条约或建筑查询。
- 汇编复核14141F1D0在调用14141FDA0前设置DL=1，后者向140BE7C10传原版海峡、国家和该模式；这明确国内路径生成链中该调用的参数模式。静态分析与合成对象验证不证明实际调用期、原生对象寿命、TLS／锁、外交条约分支或线程安全；真实回调仍未绑定，海峡权限问题仍未修复。x64dbg桥状态查询超时并终止等待，没有附加或暂停游戏。
- 本单元未改产品源码、构建、部署或启动游戏；候选dist仍R150的23914d65…，安装仍R147的178e039d…，R150同候选58 PASS为历史测试。三处港口末端及最终验收继续。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r152-native-permission-chain/{chain_harness.py,result.txt,mode-assembly.txt}。


### 2026-10-30 — R151：只读存档数据上的完整快照交接模拟

- 自主暂停会话74536使用外部PROCESS_VM_READ探针复用当前CopyNativeSnapshot生产源码：仅在探针内模拟访问门标志、路线selector及全允许／全拒绝权限回调，复制当前存档数据形成world1／revision1和2的两份自有结果；17个国家、模拟权限612次，两结果内容身份不同。第三次在回调内模拟请求退役，整份复制被拒绝。
- 两份结果均在关闭进程读取句柄后计算国内物流，9038笔货流逐项比较来源、目的地、市场、商品、数量及每条模型边方向完全相同，总量91297724 milli-orders。原版selector并未调用，测试对多候选故意返回首项，仅用于交接，不是合法选路证明；权限也为测试值。外部顺序观察和模拟访问标志不构成引擎访问租约或原子快照，不扩展为真实函数、实际权限／海峡修复或港口验收。
- 实机实际模块仍已安装R147主体178e039d…；候选dist仍R150主体23914d65…，未部署。会话WM_CLOSE正常退出0，五输入字节未变、扩展相关错误0、Unexpected token基线一致。本单元未改产品源码／DLL，沿用R150同候选58 PASS历史。原版140BE7C10真实调用依赖、权限变化／旧世界并发、三处港口末端及最终验收继续。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r151-owned-snapshot/{live_snapshot_probe.cpp,live/snapshot-probe.txt,live/close.json,live/after.json}。


### 2026-10-30 — R150：权限结果变化的复制期复核

- R148权限采集桥此前只判断一次；合成回调在海峡控制位不变时由允许变拒绝，旧代码实际FAIL。现在在同一调用方提供的访问范围内复核所有国家／海峡结果，允许→拒绝、拒绝→允许、第二次不可用均拒绝整个结果；保留已知拒绝和未知的区别。所读字段复读与世界退休检查仍保留。
- 两次观察不能证明未读取的外交／条约对象未改变或ABA安全，也不证明原版函数无副作用、锁等待或完整ABI；回调仍未绑定真实游戏，额外调用的实机性能未测。完整合成世界的CopyNativeSnapshot成功链、真实权限变化与三处港口末端仍待完成。
- 完整构建、统一入口58 PASS／零FAIL及embedded通过。候选dist主体23914d6503a31fee5ca4521436ae019b46a0ff6ae83028228a8a263b1cc95d66未部署／实机；安装仍R147的178e039d…，源码／dist／安装不一致。未操作游戏，现有生产海峡行为未改变。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r150-permission-consistency/{before,build.log,full-tests.log,record.py}。


### 2026-10-30 — R149：权限采集桥接入快照交接，生产回调仍关闭

- CopyNativeSnapshot增加可选权限回调；仅在现有完成命令访问门内、国内及原版贸易输入复制之后调用R148采集桥，失败则拒绝整个快照，成功将自有国家权限表随同世界／revision交给R147模型。默认不传回调，现有游戏入口未绑定原版函数，实机行为未切换。
- 原有native_snapshot_tests加入无效访问范围和退役时权限回调不得运行的断言；R148隔离测试覆盖合成允许／拒绝／变化，R147模型测试覆盖国家结果与缓存变化。尚未用完整合成世界对CopyNativeSnapshot整条成功链做端到端测试，也未验证真实140BE7C10的外交、条约、建筑／锁及对象生命周期；不能声称原版通行修复完成。
- 完整构建、统一入口58 PASS／零FAIL与embedded通过。候选dist主体00e333a6f0646586c69c35e33e44aeb6bc6f953830d9a22ec7eb043575c974cf未部署／实机，安装仍R147主体178e039d…；源码／dist／安装当前不一致。未操作游戏；国家权限和三处港口末端仍待，整体目标继续。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r149-permission-handoff/{before,build.log,full-tests.log,record.py}。


### 2026-10-30 — R148：原版权限采集桥隔离实现，未绑定游戏调用

- 新增DecodeDomesticStraitAccess供已验证访问范围内的权限回调使用：按国内lane国家去重，核验国家／海峡完整句柄、类型、当前定义、省份及已复制控制字段，生成自有国家→可通行完整句柄表；明确拒绝无法取得的结果，false作为已知拒绝，复制期间已读取字段变化或范围退役时拒绝结果。回调仅在对象校验后调用，不把指针交给worker。
- 已接现有native_domestic_tests统一入口：合成回调覆盖允许／拒绝／不可用、国家去重、旧海峡与旧国家代数、控制位变化及回调前退役。R147模型消费测试负责国家差异与缓存失效；这些都是离线测试，不等于原版权限函数ABI／游戏生命周期已验证。原版外交、条约、建筑及其可能变化尚未完整纳入一致性窗口，真实引擎回调未绑定，不能以此宣称海峡修复。
- 完整构建、统一入口58 PASS／零FAIL、embedded通过；候选dist主体0a104ab476a39d5dc3f808c5fcd9a8d93f1f0c24b7bf6f74d522e99aa6dc0a97未部署／实机，实际安装仍R147的178e039d…，当前源码／dist／安装不一致。未操作游戏，现有生产行为仍走旧路径。
- 下一步核对140BE7C10及其外交／条约／TLS与锁路径，保证当前访问范围和相关依赖一致后接入快照；再实际验证不同国家、权限变化、世界退役及原版路线。三处港口末端与最终同构建验收继续。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r148-permission-capture/{before,build.log,full-tests.log,record.py}。


### 2026-10-30 — R147：按国家接收原版通行结果的数据通路

- 国内自有快照新增可选的国家→可通行海峡完整句柄结果；存在结果时按lane.country选择，拒绝缺国家、无序／重复或过期海峡句柄，并将结果纳入缓存内容身份。worker只接自有数据；原版采集尚不提供该结果，当前实机仍走原有注册海峡路径，不能声称国家权限或运河修复已完成。
- 以R142已复现的注册即放行缺口为背景，新集成回归为两国家提供相反合成结果，确认几何分别更新、交换权限使缓存身份变化、最终模型量不变；缺国家和旧代数海峡拒绝。此为模型测试，不是引擎调用或实机受限国家验收。
- 完整构建及统一入口58 PASS／零FAIL，embedded、selftest560452、默认隐藏通过；deploy.ps1安装主体178e039d3ebae6aaa9039e9ba03c6fe14960416bd07da48a0d7ee8a883353c12、引导71dae36d…；自主会话3148实际加载同哈希，市场量25963.19216／510.99 MU、218提交，918陆／93海／9038笔／3部分几何。采集尚未启用新权限结果，此实机只验证旧路径未回归。
- 本局一次DRAW批次记录peak_frame_us=3006、overrun_frames=1，超过软2ms预算；不是最终三轮性能验收，保留该证据。会话WM_CLOSE退出0，五输入字节不变、扩展相关错误0、Unexpected token基线一致。下一步验证原版通行函数完整调用及安全窗口并实现采集、权限变化与旧世界拒绝；三港口末端与最终验收继续。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r147-permission-owned/{before,build.log,full-tests.log,live,record.py}。


### 2026-10-30 — R146：海峡原始控制字段进入快照与缓存身份

- 修复同一海峡句柄下控制字段变化仍被判为等价：原样复制运行对象+A8..AB与定义+4B到自有快照，并纳入完整内容比较；不把这些字节解释为国家权限。生产头与既有世界贸易测试修改前已备份。先新增用例，旧源码实际FAIL；修复后四字段逐项变化、定义标记变化及复制中控制位变化拒绝均通过，原有测试保留。
- 完整构建成功，统一入口58 PASS／零FAIL；embedded、selftest560446亮像素、默认关闭调试界面通过。通过deploy.ps1安装主体7ba956dd5b591e5d2e1e4b0d4846e18d66812a6756b594cf422b364bed6a5541，引导71dae36d…；会话62296实际模块哈希吻合，市场页25963.19216／510.99 MU，218条线段提交，国内918陆／93海／9038笔／3部分几何。截图和日志属于当前暂停存档，不扩展为受限海峡实机验收。
- 自启会话正常WM_CLOSE退出0，五项受保护输入字节不变，扩展相关错误0、六条Unexpected token与旧基线一致。国际继续用原版已选贸易路线，国内继续用原版港口lane与模型运量；海峡国家权限尚未接入，现有国内把全部注册海峡加入候选的问题和三处港口末端仍未修复。原版通行函数可能触发TLS初始化／锁及外交条约分支，须完成ABI、访问期和生命周期验证后才能调用。
- 证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r146-strait-snapshot/{before,build.log,full-tests.log,live,record.py}。完整目标继续，最终同构建权限／改图／生命周期／性能验收未通过。


### 2026-10-30 — R145：当前存档海峡权限快速分支只读核验

- 以当前EXE／暂停原存档自主会话73064，只读复用完整海峡句柄、类型及定义校验，按两次顺序观察复读9条注册海峡；其定义+4B均0、海峡+A8至AB均0。按R142原版汇编和R143隔离验证，这些字段落在原版140BE7C10直接放行分支。此推断不等于实际调用该函数，顺序复读不等于原子快照。
- 当前存档没有受限字段或需要建筑的注册海峡，不能验证受限国家／开放运河／权限变化。原版建筑查找路径中的140742920可能初始化TLS、取得独占锁，不能未经生命周期和访问条件验证而从现有回调直接调用；本轮未启用调用或修改产品代码。注册存在冒充权限的问题仍未修复。
- 会话正常WM_CLOSE退出0，五项受保护文件字节不变，扩展相关错误0、Unexpected token与基线一致；主体仍9afd，无游戏。国内物流量仍按模型计算。三处港口末端、权限变化及最终验收继续。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r145-current-strait-flags/live/{strait_flags.py,strait_flags-first.json,strait_flags.json,close.json,after.json}。


### 2026-10-30 — R144：原版国家通行分支隔离核验

- 隔离进程校验精确EXE 7f53e326…，执行原版140BE7E10机器码；国家从属及第二关系检查以受控stub代替、条约为空。11例覆盖同一限制条件下本国／外国完整ID、两种调用模式、可控从属关系变化和开放规则；结果符合原版指令且输入逐字节未变。一个限制条件下本国通过、不同国家不通过；模拟从属关系可改变结果。
- 此结果仅证明合成对象和替身函数下该函数的分支行为，未验证真实从属、条约／外交子调用、游戏访问期或对象生命周期，不能启用原版调用，也不是实际游戏可通行的判定。R142中当前国内几何将注册海峡视为可用的缺口仍在，三处末端未修复。
- 原拟临时隐藏全部权限未确认的海峡会同时丢失合法路线，无法完成用户要求；未进行该修改，继续追踪真实原版权限来源。产品源码、DLL、部署和游戏状态未变，沿用R138同核58 PASS作为历史证据。完整目标保持进行中。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r144-country-permission/{owned_country_harness.py,result.txt}。


### 2026-10-29 — R143：原版海峡检查的隔离快速分支验证

- 独立进程以精确EXE SHA256 7f53e326…校验后复制140BE7C10原始机器码，保留相对地址布局；仅用合成stub替代建筑查找与虚函数，其他引擎页面不提交。23例覆盖定义是否要求运河建筑、建筑有效性、建筑+D0低两位的四种组合，以及两侧字段的7种直接放行组合；结果符合指令，输入逐字节未变。无游戏加载、附加或调用。
- 在此隔离fixture中，已注册的运河对象仍可能因建筑对象无效或状态位返回0；这佐证R142不能把注册本身当通行。字段的游戏语义、当前实机状态、外交／条约等深分支、完整ABI和生命周期均未验证；测试依赖合成stub，不能据此启用游戏内函数调用或宣称海峡权限已修复。
- 无产品源码、DLL、部署或实机改动；国内量仍为模型值。三处港口末端、真实国家通行权限及全部最终验收继续。证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r143-native-permission-harness/{fast_path.py,fast-path-result.txt}，调用链与汇编见R142 checkpoint。


### 2026-10-29 — R142：国内海峡通行条件缺口的负对照

- 当前国内模型在构建几何时将全部当前注册海峡加入path.straits；NativeTradeStrait只保存句柄、省份对、费率和定义，没有国家通行结果。独立夹具复用生产SelectNativeSeaConnections，构造已注册海峡和相应省份／样条，复现不带国家权限输入仍选出该边。此为合成负对照，不证明当前存档存在实际被拒绝路线，也不证明原版选路必经该边。
- 精确EXE SHA256 7f53e326…静态调用链14141FDA0遍历当前海峡，并以海峡对象、国家对象及模式参数调用140BE7C10分类；后者检查定义／运行状态、两侧字段并向140BE7E10传国家对象。Ghidra省略部分参数，已用permission-assembly.txt和classification-assembly.txt核对x64寄存器。不能将费率或注册存在冒充国家可通行；具体状态语义、可安全读取或调用的访问窗口仍待验证。
- 独立测试首次运行缺工具链PATH退出1且无诊断；补PATH后退出0并保留复现脚本，编译的fixture重命名main产生无返回警告；未改生产源码／DLL、未操作游戏、未重新执行统一测试。R138同核58 PASS仍为历史证据。模型量不变；港口末端与真实权限仍未修复，整体目标继续。
- 下一步在原版安全采集窗口确定国家／海峡权限的只读或已验证原生调用与寿命，设计禁用／开放／权限改变和旧世界失效回归，再接入当前航线几何；不得按ID黑名单或仅凭注册状态补线。证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r142-strait-permission/{permission_negative.cpp,140be7c10.c,140be7e10.c,14141fda0.c,permission-assembly.txt,classification-assembly.txt}。


### 2026-10-29 — R141完成：原版寻路端点来源与当前定义只读核对

- 静态反编译14144FEF0→141572DE0：候选运行州经定义的当前naval出口索引取得海区定义，取其省份数组首个16位值建立搜索候选；14141F910在两个候选数组间寻找合法成本最小路径并写出选中的两值。14141F1D0也包含海峡处理及另一搜索分支，不能将本分析简化为所有路径都只走同一分支。关键字段经14144FEF0汇编交叉核查；未调用引擎函数、未启用hook。
- 自启暂停会话41088只读当前6州两次，三目的州207／936／608的出口海区3157／3056／3072，首海省分别36064／35988／36029，数组均一项。各值与R139另一次同存档会话中各航线成员边界吻合；此为跨会话顺序比较，非同一原子快照，也不是对搜索实际返回值的运行时追踪。外侧港口样条不因此获得补画授权，路线末端仍待解决。
- 会话正常WM_CLOSE退出0，五输入字节不变、未见扩展资源相关错误，六条Unexpected token与基线一致；未修改产品源码或部署，主体仍9afd。沿用R138完整58 PASS作为历史同构建证据；国内模型运量和国际原版贸易口径不变。完整几何、通行权限、改图及生命周期最终验收仍未完成。证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r141-native-endpoints/{endpoint-audit.json,endpoint-assembly.txt,14144fef0.c,14141f910.c,live}。


### 2026-10-29 — R140完成：国内航线生成端点的静态调用链

- 对精确EXE SHA256 7f53e326…、静态基址140000000，原版国内kind0创建链14140D830→141419DA0→1414500F0，前者将生成结果与端点传给14140DF60→14140FAA0创建航线。141419DA0遍历候选州对、比较路径成本，并复制获胜的省份集合与州ID；当前证据是静态反编译，关键调用及寄存器由objdump交叉核对，不证明运行时原子性或调用安全。
- 1414500F0将路径搜索结果中的两个16位值和其他结果写给其调用者，141419DA0将两个16位输出放在局部变量；本调用链中尚未看到将它们随胜出集合一起保存。它们与港口缺段的对应关系和权限语义仍需独立确认，不能直接认定为所需海区、擅自重跑引擎寻路或增加hook。此发现为下一步追踪实际端点来源提供具体切入点，并非港口修复。
- 无产品源码、构建、部署或游戏操作；沿用已安装9afd及R138同版本58 PASS作为历史证据。国内最终运量继续来自模型；三处港口完整几何、运河权限、地图及生命周期最终验收仍未完成。证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r140-lane-writer/{14140d830.c,141419da0.c,1414500f0.c,selection-disassembly.txt,find_calls.py,parents.py}。


### 2026-10-29 — R139完成：当前港口外侧候选边界核验，末端仍待修复

- 当前游戏只读复制原版航线、实际海区邻接、样条及海峡定义，关闭进程句柄后用当前源码头独立重放：918陆路、93国内海路、9038笔、3条部分几何；对应边模型分配量0.586／418.679／0，零是模型结果而不是未知。顺序外部观察并非原子游戏快照。
- 三处缺失港口各有可连接lane成员的外侧样条，分别有2／1／1条候选；这些边不属于原版lane省份集合，各直接边均未找到匹配的当前海峡定义，但这既不证明原版选路，也不证明所有通行权限。台湾原版lane距离为0，直接补画外侧边无法证明忠实于该lane；不能硬编码地图身份或自行选其中一条。另有两条候选位于lane内，首版独立探针重复列出同key，修正去重后保存两版证据，未发现生产重复缺陷。
- 我方暂停会话34156正常WM_CLOSE退出0；五项受保护输入字节未变，安装与dist仍为主体9afd299a…及引导71dae36d…，无游戏。未修改产品源码、重建或部署；沿用R138同版本完整58 PASS，并非本轮重新执行。未见扩展相关错误，六条原有Unexpected token与基线一致；不据此声称所有游戏日志完全相同。
- 三处港口完整末端与原版合法路线来源仍待解决；国内运量继续标注模型值，国际贸易保持原版数据。下一步核对原版是否存在可验证的端点或另一合法路径来源，保留改图、运河权限、生命周期及最终验收未完成状态。证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r139-port-boundary/{boundary-audit.json,boundary_probe.cpp,live,NEXT.md}。


### 2026-10-29 — R139进行中：港口外侧海区与原版航线边界核验

- R138完成交付引导异常隔离回归、统一58 PASS，当前9afd无游戏。重用R136已确认的三处运行州／原版航线与现有源码，先只读本机splnet：三处港口的其他海区都存在通向lane成员的type2候选，西密克罗尼西亚存在两个候选；这不是原版已选路径／通行证明。
- 正在编译独立只读探针，从当前游戏实际复制航线、相邻省份、海峡定义及样条后关闭句柄，再枚举候选及所属边界；验证结果须区分原lane成员、运行海峡和原版选择，不按本机静态ID写产品或补线。源码／双DLL／五输入先备份。



### 2026-10-29 — R138完成：实际交付引导的缺失／损坏主体隔离回归

- 当前主体9afd299a…／47581219B、引导71dae36d…／779879B，源码＝dist＝安装＝R137实际加载；产品源码和两份DLL未变、本轮未部署或启动游戏，完整统一入口58 PASS／零FAIL／退出0。

- 直接复制实际交付version.dll到含中文／空格的独立路径，工作目录放另一目录及同名无效诱饵；缺少主体和整个freight目录时引导自动建立目录，加载失败码126；损坏PE时193；实际健康主体成功加载且码0。三者均在独立进程中保持系统版本信息查询可用，不是游戏进程内缺失／损坏实机通过。

- 扩展已有tools/test_freight_loader.py：统一入口同时对重新编译引导和dist实际引导分别验证有效stub、缺失、损坏及错误码0／126／193；17个导出存在、系统版本数据逐字节一致；另外原有--runtime使用真实双DLL验证新配置优先和迁移。无玩家运行时新依赖。

- 隔离测试及统一入口后两份实际安装与dist哈希相同，原save／settings／ini／content／continue五输入及存档目录保持、无游戏。首私有审计误以为R134清单包含该测试文件而被断言拒绝，核对清单后改为对比本轮before；失败脚本保留。一次仅用于打印的私有脚本遇GBK输出错误，完整测试已结束且结果不受影响。

- A26新增当前交付引导隔离子项PASS；真实游戏进程中缺失／损坏、目录不可写、完整路径／升级等矩阵仍PARTIAL。三港口末端／运河权限、改图新字段、写任务／提交原子性、真实ABA／新设备与最终同构建A01–A27继续；整体goal ACTIVE。

- 证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r138-delivered-loader/{loader-audit.json,isolated-results.json,isolated 中文 空间,full-tests.log,before,NEXT.md}。



### 2026-10-29 — R138进行中：实际交付引导缺失／损坏主体验收

- R137当前9afd／71da源码＝dist＝安装，实际卸载恢复及原版日志对照通过。P7已按MASTER§2.4完整重读00／01／02／03；备份本单元将涉及的测试、源码、文档及双DLL。现有缺失／损坏用例每次独立编译引导，尚需直接针对当前交付version.dll验证。
- 在独立临时目录使用实际双DLL引导，覆盖缺失／损坏主体、中文空格路径／不同工作目录、系统版本转发和具体加载错误；需要时只改测试并接统一入口。隔离结果不冒充游戏实机、目录不可写或全部A26；原安装与玩家输入不动。



### 2026-10-29 — R137完成：原版地点查找器对照与当前版本卸载恢复

- 主体9afd299a…／47581219B、引导71dae36d…／779879B；源码／资源／dist未变，复用R134完整55 PASS与R135同核性能。本轮通过deploy.ps1实际卸载后恢复，最终源码＝dist＝安装＝实际24108加载。

- 同原存档／中文／暂停59883360，地点查找器输入台湾并点击第一个地区结果，分别保存点击前后error.log。仅系统VERSION.dll加载的原版27220与当前扩展24108均为点击前0条、点击后新增1条PdxDebugConstants命名索引错误；完整规范化错误Counter相同、点击差分只有该行。R136的新增日志在本机无Freight时可复现，不能直接归因扩展；此结论只限本机EXE／存档／语言／操作，不推断所有游戏版本或其他命名索引错误。

- 卸载前核对无deploy.ps1会清理的旧文件／playset条目并备份42个非主体文件；卸载与重装之间全部42个配置／缓存／可编辑资源逐字节相同，再次实机启动后仅freight.log／loader.log自然追加。实际模块分别为stock系统VERSION.dll、当前version.dll＋freight.dll；恢复后的市场0物流页40商品／218段／25963.19216／510.99MU实见。

- 两次我方会话27220／24108均全程暂停、正常WM_CLOSE退出0；五项受保护输入及存档目录字节保持、无新存档和游戏进程。当前retirement2实见，无扩展资源相关报错；不能将正常退出扩为ABA／新设备通过。

- A26当前卸载／资源保留／重装实载子项PASS，完整缺失／损坏／不可写／路径／升级等矩阵仍PARTIAL。三处港口末端、运河权限、改图新字段、写任务／提交原子性与真实ABA／新设备及最终同构建A01–A27继续；整体goal ACTIVE。本轮消除了R136这条日志的扩展独有疑点，未修复三处港口缺段。

- 证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r137-stock-location-control/{stock-control-audit.json,live,runtime-before.json,before,NEXT.md}。



### 2026-10-29 — R137进行中：原版地点查找器日志与卸载恢复对照

- R136当前9afd、三个真实运行州及原生航线核对后正常退出；台湾地点搜索结果点击同秒出现一条PdxDebugConstants命名索引错误，原因未确定。P7完整重读00／01／02／03，备份双DLL、五输入、全部freight非主体文件及文档；确认deploy.ps1无遗留待清理文件，未改变用户playset。
- 同原存档／中文语言／暂停状态分别运行原版卸载与当前安装版：地点查找器输入台湾，分别采集点击前、后及正常退出错误日志，核对实际加载模块、设置／存档／资源；最后经deploy.ps1恢复双DLL并验证原版物流入口。错误复现不自动归因，结果只限本机与该操作。



### 2026-10-29 — R136完成：三处当前港口身份与界面核对，缺段仍待修复

- 主体9afd299a…／47581219B、引导71dae36d…／779879B，源码＝dist＝安装＝实际46268加载；本轮未改源码／ABI／资源、未构建或部署，复用R134的55 PASS及R135性能证据。

- 当前暂停原存档在精确EXE7f53e326…／module7ff63d2e0000下，线程TLS对应当前上下文只读两次相同：三个真实运行州207／936／608的州定义及naval_exit分别对应3157／3056／3072，均属于各自原版lane成员；可用类型3港口样条却分别连接3002／3157／3056，均不属于各自lane成员。R121曾因会话结束未完成运行时字段读取，本轮补到当前运行对象证据；两次顺序观察不是原子快照，也不证明所有Mod。

- R121历史编译的独立探针在关闭PROCESS_VM_READ句柄后重放得到918陆／93海／9038笔、partialSea3，三条对应边的模型量0.586／418.679／0；当前生产日志独立确认相同全局数。端口和几何两个生产头及其回归与R121逐字节一致，但不能把旧探针冒充本轮生产逐边trace。零是合法模型分配，缺几何不改成未知运量。

- 实际原版南海航线面板展开大清港口连接，台湾行及原版商船需求1.40实见；该字段不是货物运量。独立面板探针确认台湾936、苏禄北婆罗洲608、菲律宾西密克罗尼西亚207均打开local_goods，三处截图保留；台湾与207物流列有非零数，608当前为零。按地区名称搜索后最初点击的是同名另一运行州206，探针识别后排除并改选207；截图显示列表／现有视图，不证明原版或扩展的完整末端几何。

- 我方46268/start2026-10-29T00:41:42.2087748+08:00一直暂停／日期59883360，正常WM_CLOSE退出0；原save／settings／ini／content／continue五输入字节和存档目录保持，当前无游戏。退休2实见。日志对R134同语言多出一条“PdxDebugConstants”命名索引错误，原因未确定；严格Counter相等首审计被拒绝并保留，没有ironroad／freight／native-gui相关错误，不能称完全无新增游戏错误。

- 三处完整末端几何仍未修复；后续优先追查其他当前原生几何来源与权限语义，不猜路线。运河实际开放关闭／权限、更多改图、并发／ABA／新设备及最终同构建A01–A27继续，整体goal ACTIVE。本轮原生日志新增项需在后续可比场景定位，不能将它归因扩展或直接排除。

- 证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r136-port-runtime/{port-runtime-audit.json,live,ghidra-1425e8070.txt,audit-first-rejected.py,NEXT.md}。



### 2026-10-29 — R136进行中：国内港口原版连接及当前定义核验

- R135当前三对性能及20轮缓存验证已完成，实际安装9afd；重读01／02／03与MASTER§2.4和原版几何约束，备份当前文档／源码／双DLL与五输入。上一轮R121只核对本机定义，未成功读取运行时naval_exit字段，也没有三地实际屏幕验收。
- 原版025E8450／8690／8070反编译再次核对，仅按当前lane成员、相邻省份和端点定义查询现有类型3样条，不能据此用其他海区或最近海岸补图。下一步自主启动同一暂停原存档，只读复制运行时定义／海区、现有lane与样条，核对原生显示；不把本机旧表或测试ID固化为产品映射。复现和原生语义明确后，才考虑可验证的最小修复及相应回归。



### 2026-10-29 — R135完成：当前同核三对性能与20轮颜色组合缓存验收

- 主体仍9afd299ab0f0aba088e9f6db68147319fbdeabd4fae74fdfaed453e31f451a69／47581219B，引导71dae36d…／779879B；源码／资源／dist／安装未变，复用R134完整55 PASS。无新增构建／部署／原生ABI／hook；当前实际54780双模块核对，timing-audit.json证据及指定性能门槛PASS。

- 沿R129／R130同原存档59883360、英国地标／2560×1440／原图形设置／原生速度3，每臂正常重载同存档，暂停预热30秒后运行65秒；六窗完整连续64.83–64.85秒，窗内无截图／输入、所有尖峰保留。三对关闭→打开P95为17.9185→19.2502、17.8774→19.1865、17.9017→19.0847 ms，增加1.3317／1.3091／1.1830 ms均≤2；模拟最大降幅0.774997%≤5。Present入口间隔不是GPU／显示延迟，根市场关闭还改变原版地图模式，只证明本存档／速度协议。

- 随后world6／rev52／date59884134暂停，20轮四分类轮换＋三颜色／单商品hover进入离开／恢复全部／Escape关闭→F4原版市场→点物流，逐轮VM_READ确认closed／market0／tab，176份观察通过。完整市场量26371.75358／510.99MU不因视图改变；与原日期的25963.19216差异来自本轮正常模拟推进，不将跨日期数量当UI不变对照。前四轮建立变体后，后16轮uploads和实际pool均1721不变；SUBMIT的cache是累计命中数，允许增长，不能当对象池。稳定66.313秒uploads1721不变，全进程private固定15880417280B；后16轮private范围15879147520–15892967424B、净减1712128B，属于本窗口平台证据，不证明所有分配器／8192实际满池。

- 172个实际完成DRAW批次账逐项相等，失败／丢弃0，池与native槽峰1721，每帧尝试峰16，Pump帧／单步峰1161／708µs、超2ms帧0。QPC含调度／driver等待，软预算不作硬实时保证；R134真实2056／2392µs超时保留为各自历史局，不删除或归到本局。

- 我方54780/start2026-10-28T23:09:37.6659011+08:00正常WM_CLOSE退出0；原存档／settings／ini／content_load／continue五输入字节及存档目录保持，无游戏。重载退休2…6及退出退休7实见，不扩为ABA／新设备通过；完整错误对R134中文无新唯一行，六条Unexpected身份保持、扩展资源错误0，重复读档的次数差异单独保留。

- 整体goal ACTIVE；当前9afd指定三对、20轮组合平台与稳定零上传子项完成，不再列为当前缺证。A25整体仍PARTIAL：更广工作负载／最终全矩阵不外推。继续三港口末端确证、运河开放关闭／权限、更多地图与铁路新字段、写任务／提交原子性与真实ABA／新设备，以及完整A01–A27和异常安装升级卸载恢复。若后续代码改变，应在最终构建重跑必要门槛。

- 证据／当前对备份：E:/Code/v3-analysis/checkpoints/2026-10-28-r135-final-performance/{timing-audit.json,budget-report.json,error-comparison.json,live/benchmark,before,NEXT.md}。



### 2026-10-28 — R135进行中：当前9afd三对帧／模拟与组合缓存复验

- R134已完成正式三配色／八档图例及双语居中、坏GUI拒绝与恢复，55 PASS；源码＝dist＝安装9afd299a…、无游戏。P7按MASTER§2.4完整重读00／01／02／03及§10，备份当前源／文档／双DLL和五输入至checkpoint2026-10-28-r135-final-performance。
- 不改产品；按同原存档／相机／速度3／设置，三对关闭与物流各暂停预热30秒＋运行65秒，连续原始帧保留尖峰，P95≤+2ms／模拟≤-5%；再至少20次组合循环和≥65秒零上传／平台。旧R130通过不替代当前9afd，预算实际软超时保留。当前仅准备，未宣称通过。


### 2026-10-28 — R134完成：正式颜色／八档线宽图例、双语居中及编辑GUI校验

- 主体9afd299ab0f0aba088e9f6db68147319fbdeabd4fae74fdfaed453e31f451a69／47581219B，引导71dae36d…／779879B；源码＝dist＝安装＝最后实际63460加载。完整55 PASS／零FAIL，embedded／GUI精确归档／selftest560464／默认隐藏vtx0及默认无FRAME PERF通过；只经deploy.ps1部署。

- 业务色保持地区输入绿／输出橙，市场热度使用完整市场固定区段商品数量尺度；单商品模式按原生商品ID取既有稳定五色，未选或移出为空地图，原生外观统一已验证建筑绿。颜色只改视图，不改国际原版记录、国内最终模型分配、需求账、完整行／卡片／市场分母、路径或宽度。八档图例显示实际宽度比例2:3:4:6:8:12:16:24及数量阈值10／50／200／500／2000／5000／20000，提示比例示意、非像素／容量／距离工作量；0不画、未知--。

- 当前9afd英文67696与中文63460各21项颜色／商品／方向／关页重开检查通过；市场218、单商品等待0／谷物hover123／移出0，州433全部248／输入107／输出141，原生绿与业务色恢复原段数。市场量25963.19216／510.99MU、州433量3548.63121全程保持。英文原版首行Artillery为66段，中文所选第二行加工食品为22段；这是原版本地化排序，不把两次选择当同商品对照。中文额外显示／隐藏实际截图与日志通过，数量不变。每个正常会话仅一次国内分配918陆／93海／9038笔，三处partialSea仍保留已知数量及可确认片段。

- 四个必需GUI类型按真实type声明计数，跳过注释／字符串；缺失或重名拒绝整个扩展挂载、记录明确原因并保留玩家修改，提示对照freight/native-gui/.defaults。当前9afd实际坏GUI67464中，注释伪声明不能冒充类型，原版四页签／frame0、修改字节保持，正常退出后严格恢复；后续63460重新加载完整GUI通过。16个按钮文字框改为100%边界／居中／鼠标透传，中英截图及实际点击验证通过。

- 七个我方会话全部正常退出0；原存档／settings／ini／content_load／continue五输入最终字节相同，无新存档、当前无游戏。英文退出仅删除受控language字段，逐字段证明后恢复原中文字节。完整规范化error Counter逐局等于R132同语言基线，未新增扩展错误；每局retirement2实见，不扩为ABA／新设备通过。

- 整体目标active；三港口末端、运河开放／关闭及权限变化、更多地图／铁路新字段、写任务与提交并发／ABA／新设备、最终同构建帧／模拟三对、压力平台及完整A01–A27继续。颜色／图例／当前双语布局及坏GUI子项已完成，P5／A18–A19和A25整体仍DOING／PARTIAL。R130旧核性能不能替代当前9afd。

- 证据及R133回滚：E:/Code/v3-analysis/checkpoints/2026-10-28-r134-native-legend/{legend-audit.json,live,logs,before,NEXT.md}。



### 2026-10-28 — R134进行中：正式颜色／八档线宽图例及编辑资源契约

- 居中复验前发现仅缩短文案仍沿用无明确文字框的旧锚点。新增GUI静态用例先FAIL后PASS，16按钮标签统一100%文字框／align=center／鼠标透传；新完整入口包含GUI静态回归，55 PASS／零FAIL。资源重包后主体9afd299a…／47581219B，全部非资源.o与先前逐字节相同，assets862ecc6e…未变；已deploy并实际启动英文67696，当前在做居中版中英交互复验，尚未将该阶段判为完成。


- 初英文24612已实际加载bd5c449a…／47579683B，市场单商品等待0／首两行123与132／移出0，Native与Heat218；州433量3548.63121不变，首行原版tooltip确认为Artillery、66段，三配色保持；完整248／图例八档及首末阈值、关页／重开通过。正常退出0，英文只删除受控language字段，原中文字节与五输入已恢复。
- 旧英文地区All modes等标签偏长已缩为In／Out／All与Click Trade，中文提示同步缩短；只重包本地化，C++／GUI布局／shader保持。当前dist最终候选b671255dfadea26ffc52d02832ba0dd022ab23faa3933f28c9aaf5502c7a886f，安装仍首英文bd5c449a…；最终51入口与附加检查正在串行运行，尚未部署／实机最终候选。


- 上轮R133为progress：地区地图筛选及当前中文19场景通过，当前309a安装＝dist、无游戏。按P5／颜色主题完整重读01／02／03、MASTER3.2／6与实际FlowWidth阈值及当前样式；备份源码／测试／GUI／文档及双DLL至checkpoint2026-10-28-r134-native-legend。
- 采用业务色（地区流向、市场固定全市场量尺度热度）、单商品稳定ID色高亮、统一原生建筑绿三选项，不新增原生调用／任意tint。未选商品时高亮模式为空图并提示；颜色不改数量、几何、需求账、分配或分母。图例展开显示八档实际宽度比例及数量阈值，默认折叠；较长中文按钮缩短，选中商品单元表示选择状态。
- 通过标准：先覆盖颜色／缓存／统计不变、hover进出及scope拒绝；必需自定义GUI类型缺失／重名（排除注释字符串）保留编辑并拒绝挂载；完整构建／统一入口／归档及harness串行通过后deploy，当前双语实际控件／图例、切换／关闭、原输入保护。候选PASS不冒充实机或全目标通过。

### 2026-10-28 — R133完成：地区商品／方向／运输方式地图筛选

- 主体309aed48c5880adf61d83b6883200715dd86291277c72a6c0da7ceaa2cbcb9ba／47369073B，引导71dae36d…／779879B，源码＝dist＝安装＝实际18592加载；完整50 PASS、资源／GUI精确归档／selftest560382／默认隐藏vtx0通过。地区新增商品、输入／输出和陆路／海路地图筛选，州各商品完整数量、国内分配、国际原版记录、需求账及全市场分母保持；切州／关页／商品详情撤帧并重置。
- 当前原存档暂停59883360／world1／rev4，州433量3548.63121保持；全部248段，输入107／输出141，陆路65／海路183；加工食品22段，输入陆路1／输入海路21。州5恢复完整282段／5506.40802，返回州433及关闭重开248段。丝绸运量0时合法空地图，恢复全部248；原版加工食品详情无自定义线，原版Back恢复完整州视图，市场0恢复40商品／218段／25963.19216／510.99MU。
- 仅一次9038笔国内分配；23完成批次／池635／16每帧／702µs峰／0超时。正常退出0／五输入及存档目录保持／错误Counter相同／无游戏；资源CRLF归档及并行共享日志首FAIL保留，最终串行通过。私有grain*实际加工食品，原版详情已核实。
- 整体目标active，P5／A18–A19及A25整体仍DOING／PARTIAL。新增控件英文布局、较长中文“全部…”标签省略及商品选择指示待完善；颜色模式／线宽图例、旧玩家编辑GUI升级契约仍待。三港口末端／运河权限、地图新字段／并发／ABA／新设备、最终同构建三对及完整A01–A27继续。 证据checkpoint2026-10-28-r133-state-filters/state-filter-audit.json；下方进行中为历史。

### 2026-10-28 — R133进行中：地区地图筛选

- R132为progress（同核双语及实际卸载恢复PASS，ui-audit.json）；当前6094安装及dist不变、无游戏。按P5重读01／03与MASTER3.2／6，备份源码／测试／GUI／工具／文档及双DLL至checkpoint2026-10-28-r133-state-filters。
- 语义：商品／方向限制地图货流，运输方式只显示其实际区段；州各商品完整数量、国内分配、国际记录、全市场分母、铁路及需求账保持。海运含原版港口连接；混合路线不改路、不重新分配。控件使用既有GUI变量Set／Clear，带当前world／state／重开代数隔离，不能复活旧作用域。
- 最小通过标准：方向／混合方式／商品组合及缓存身份、未知值与旧上下文、关闭和换州回归；GUI原版绑定与宽度保持；完整构建／统一入口／归档验证后经deploy部署并实际切换，候选测试不冒充实机。当前仅计划与备份完成，无新ABI。

### 2026-10-28 — R132完成：同构建中英UI及实际卸载／恢复验收

- 当前主体6094ea6e…／47173344B、引导71dae36d…保持，74份native源未变，复用R131完整50 PASS，不重建。实际英文／中文搜索、分类、连续hover、商品右键／详情／原价与提示、州433→5→433及返回市场通过；对应商品ID和数量逐项相同。实际卸载后仅系统VERSION.dll加载、原版四页签；通过deploy.ps1恢复同对DLL并再启动，中文物流页40商品／218段／25963.19216／510.99MU实见。四局正常退出0、五输入字节恢复、无新存档／无游戏。
- 实际英文stock复现条约缺键，本机原文件未改；英文退出仅规范化受控language字段，首严格FAIL保留后按精确语义守卫恢复原中文字节。冷批次真实2030／2039／2163µs各一次软超时，峰池1201，最多16／帧，未填8192。
- 这只完成当前中英交互及卸载恢复子项，整体目标active。源码确认地区商品／方向／运输方式筛选控件尚未实现，颜色模式／线宽图例正式UI未齐，P5及A18–A19保持DOING／PARTIAL；最终同构建性能三对、三港口末端／运河权限、地图新字段／并发／ABA／新设备和完整A01–A27仍待。 证据checkpoint2026-10-28-r132-english-native-ui/ui-audit.json；下方进行中为历史。

### 2026-10-28 — R132进行中：当前构建英文原版UI验收

- R132英文59640已实际加载6094并正常退出0：clothes/CLOTHES两项1565.19777，luxury clothes一项500.81954、空结果0，4分类数量加和等于25963.19216，hover与排序不改分母；Paper右键／详情／本地价格／用途及逐历史返回、英文口径／MU提示实见。当前州433→5→433分别3548.63121／5506.40802／原值恢复，248／282／248段，Overview撤帧与市场218段恢复通过。
- 英文退出时Game只删除显式System.language字段，首strict finish的settings hash断言FAIL保留；精确JSON比较确认仅这个受控字段，其他字段不变，守卫仅接受精确English或精确删除该字段形状，原中文字节已恢复，五输入hash全相同。原版grain搜索也返回Paper/Fine Art/Grain三项，不擅自做字面再筛选。首Back采样实际仍商品LocalPrices，继续两步才回物流；首track提示被Trade提示覆盖，移出后重采验证，不计错误截图为通过。即将用恢复后的中文设置做同核对照，未改产品源码或DLL。


- 上一轮为progress：6094预算记录／20组合轮实机完成。本轮完整重读00／01／02／03，保持同6094源码／dist／安装，不重建、不新增ABI。成功标准为真实English locale下搜索分类／空结果／连续hover、商品详情／右键／原价提示、地区列及切换无旧值，而非中文界面输入英文词。
- 事前备份9文档、双DLL和原五输入至checkpoint2026-10-28-r132-english-native-ui；核对全部native源hash与R131一致。仅把实际settings的System.language从l_simp_chinese临时改l_english，其他JSON字段及四输入保持；测试正常退出后校验仅语言变化、按原字节恢复并核对五hash。恢复必须经语义／hash守卫，不能覆盖外部变化。
- 当前尚未启动／实机，原首候选R131失败仍保留；R130三对不当新6094证据，完整目标active，A25／设备／权限／地图等剩余门槛继续。


### 2026-10-28 — R131完成：原生构建预算记录与20轮组合缓存验收

- Pump既有QPC记录构建用时／逻辑帧峰值／超时，批次汇总world/rev/gen；保留16／2ms／8192，无新ABI／GUI／地图／模型变化。首候选实机重开218却复用0，严格对账／新用例FAIL后修复仅统计已完成项，完整重跑50 PASS。
- 当前6094ea6e…47173344B已部署实载，selftest560410／默认隐藏通过。20轮分类hover恢复关闭重开／101批次，实际726次上传／0失败丢弃、峰值16／帧和550µs，池后16轮平台；稳定66.172秒零新上传。8192为合成边界，实际未填满。
- 正常退出0／五输入及存档目录不变／无新唯一错误／无游戏；本局退出无原生retirement行，不能加寿命结论。R130帧三对不当本版证据，目标active，最终三对及剩余矩阵继续；备份／失败历史／审计checkpoint2026-10-28-r131-ribbon-budget。


### 2026-10-28 — R131进行中：原生构建预算实测计数

- R131首候选18be148d…／47173344B已50 PASS并实载37192，冷218条／7.030ms累计Pump CPU、单逻辑帧1.274ms／最多16、超时0。实机重开218条却reused0，严格批次对账FAIL；新增用例精确复现后，完全相同请求只把已完成项计为复用，未完成项不计，目标PASS。原测试会话正常退出0／五输入不变，first-candidate及initial-candidate保留源码／DLL／失败日志。当前正在重新完整构建／回归，旧首候选不可作最终计数通过。


- 以R130实载e4e0583…为基线，无游戏；源码／双DLL／9文档事前备份至checkpoint2026-10-28-r131-ribbon-budget。按性能主题完整重读01／02／03，验证标准是实际计数与2ms软超时可追踪，而非预先承诺单次原生调用绝不超时。
- 复用Pump已有两次clock，不新增原生ABI／钩子／数据库访问；记录累计实际构建CPU（不含帧间等待）、单步／逻辑帧峰值、同帧总尝试峰值及超2ms帧数。保持16／2ms／8192和原分配／路径行为。完成／取消／替换时输出一次DRAW PERF，批次delta及world/rev/gen、已知池上界可回查，峰值是缓存生命期累计而非该批独占。
- 目标cache回归覆盖同帧多次、同帧换请求不重置预算、单步3ms超时只计一次、跨帧10秒空隙不计CPU、失败抑制及8192／8200压力计数，PASS。当前仅目标测试；完整构建／统一入口／部署／实机尚待，安装保持R130，整体A25仍PARTIAL。


### 2026-10-28 — R130完成：精确GUI键轮询与三轮性能复验通过

- 25ms悬停／500ms开关及身份诊断改精确当前键＋stored hash预筛、完整键消碰撞；无新ABI／hook／GUI／模型改变，旧全表夹具FAIL、新键2项PASS。完整50 PASS／selftest560432／默认隐藏通过，主体e4e0583…47160425B已部署实载。
- 同R129协议三对65秒P95增1.0024／0.9268／0.9308ms／模拟均<5%，本场景通过，旧失败保留。连续hover／中文搜索／空结果／显示隐藏与原版Goods英文词零结果对照通过；国内已知模型量保持。
- 20次实际Escape关闭／F4重开点物流且逐次只读确认，随后稳定66.156秒uploads=538不变。初版F4误作关闭被拒绝，未计成功。正常退出0／五输入与存档目录不变／无新唯一错误／无游戏；目标active、剩余矩阵见STATUS。备份与审计checkpoint2026-10-27-r130-filtered-gui-probe。


### 2026-10-28 — R130进行中：正式GUI轮询不再复制整个变量表

- R129实际P95失败保留；定位25ms悬停timer完整ProbeGuiVariables、500ms显示开关及低频身份诊断均全复制全局字符串。当前单元只在既有验证getter／UI线程窗口中按当前商品目录的精确键及已存hash预筛，不改变逐行事件／owner／过滤／发布／模型／ABI。
- 事前备份源码／R129双DLL／五输入至checkpoint2026-10-27-r130-filtered-gui-probe。原01／02／03按UI／性能主题完整重读；保持25ms响应和旧行leave不能清新行hover的逐商品键协议，不靠降采样掩盖成本。
- 原4096项夹具实际旧路径复制4096→断言FAIL；新路径2项、同分／碰撞／顺序／缺失／SSO／截断键冒充／越界及只读不变PASS。目标离线全复制7.4738ms、精确筛选0.0131ms，不冒充游戏帧时。新测试接统一入口；初次测试auto多变量类型不一致编译错误已修正。完整构建／测试／部署／实机正在继续，安装仍R129b13289d…，A25保持FAIL。


### 2026-10-27 — R129：默认关闭的原始计时链完成，三轮性能基线发现超标

- 当前主体b13289d1228bceb3c295fb37fc311069e25329013d8f249f636e9a6e1579da55／47017006B，引导71dae36d…／779879B，源码＝dist＝安装＝实际加载。完整49 PASS，embedded／GUI结构与实际归档、selftest560447、默认隐藏vtx0及默认无计时日志通过。原生ABI／资源／模型与R128不变。
- 既有Present／Present1增加IRONROAD_FRAME_TIMING=1才启用的诊断：线程自有64个原始QPC间隔批量日志，嵌套转发只计外层，TEST／非当前swap排除；world／swap／时钟变化撤旧部分批次。无逐帧日志／数据库扫描／配置修改。native helper、解析器与实际D3D入口回归接统一入口；非法开关2保持关闭。指标为Present入口间隔，包含调度／呈现节奏，不冒充GPU执行或显示延迟。
- 自主52156/start18:15:32.8660935+08/module7ff63d2e0000，实际双DLL核对。每臂重新载入同一原存档59883360、英国相同地标视角、2560×1440、原设置／速度3，暂停预热30秒后运行65秒；各有64.82–64.85秒完整连续帧，窗内无截图／输入，原始尖峰不剔除。关闭／开启三轮P95分别18.1201→27.2114、18.8955→25.5687、18.4805→27.7414 ms，增加9.0913／6.6732／9.2609 ms，**全部超过+2 ms，A25整体FAIL**。本协议日期推进降幅约0.775%／0.001%／0.001%，未超过5%，含一个原生tick量化误差，不扩为所有速度／存档通过。
- 单轮诊断控制：保留物流页、只隐藏箭头仍28.0334 ms；原版市场商品页20.6771 ms。不能将超标直接归因箭头；优先检查新增物流页绑定／刷新。根市场关闭还会改变原版市场地图呈现，报告同时保留这一作用域差异；控制组不替换三轮失败基线，不改验收门槛。
- 首次30天保护上限提前停止且鼠标边缘导致相机漂移，短样本拒绝保留；测试保护改60天（原半年自动存档未改），有效窗约32天。快速菜单批次曾未进入读档而超时；复核同一活进程并观察菜单后继续，没有重启游戏或把超时当退出。首版audit预期退出world10日志而失败；实见只到重载退休2…9，正常WM_CLOSE退出0由close.json独立证明，不虚称退出析构／flush已观测。
- 已正常退出0、当前无游戏，原save／settings／ini／content／continue五输入字节及存档目录保持；完整error新唯一行0、六条Unexpected token身份保持、扩展资源相关错误0。多次读档后错误次数不能当同协议旧核Counter相等。timing-audit.json核对证据完整性PASS，**不等于性能PASS**；私有证据／回滚R128位于checkpoint2026-10-27-r129-frame-timing。目标active，GUI性能优化、三港口末端／权限／改图新字段／并发设备／最终矩阵继续；下方进行中记录为历史。

### 2026-10-27 — R129进行中：受控帧间隔诊断与性能验收

- 以R128主体87c2c194…为基线，源码／DLL及五输入事前备份至checkpoint2026-10-27-r129-frame-timing，无游戏。按地图／性能主题重读01／02／03，核对PERFORMANCE及既有Present钩子；三港口末端仍缺明确原生关系，不补造。
- 本机当前token非管理员且无Performance Log Users／Performance Monitor Users角色；外部ETW不作为已可用证据。现有Present／Present1入口增加仅IRONROAD_FRAME_TIMING=1启用的原始QPC诊断，线程自有64间隔批次，嵌套转发只计外层、TEST与非当前swap排除，world／swap／时钟变化撤旧部分批次。默认关闭，不扫描原生数据库／不逐帧日志／不改配置。
- 原始Present入口间隔不是GPU执行或显示延迟；离线分析拒绝截断、计数／时间不一致、混身份、断档和不足要求时长，最近秩P95。目标回归正在接统一入口；完整构建、默认关闭与启用的实际harness及3×60秒受控开关／模拟速度仍待，不能提前判A25通过。


### 2026-10-27 — R128完成：当前已铺铁路统计及实际拆除／重载验证

- 主体87c2c1947d88a48bc003062a6e7efb46e52cd99593674e33b3105646622b89cb／46997736B，引导71dae36d…／779879B，源码＝dist＝安装＝最后实际加载R128。完整46 PASS、embedded／GUI结构及实际归档、selftest560391、默认隐藏vtx0通过。当前无游戏，原存档／设置／ini／content_load／continue五输入与存档目录保持。
- 当前type4已建铁路独立复制为owned builtRails，包含州内及跨市场记录；模型land连接筛选不变。worker按同一当前市场两端、完整56位物理strip去重，累计当前样条二维折线地图单位MU；正式卡片及中英提示已接通。商品／搜索／hover／货量不决定基础设施长度；已知空为0.00 MU，缺来源为--，未校准公里保持未知。无新ABI／地址／hook／引擎调用。
- 原存档实读34条已建连接（24州内），市场0为22条唯一strip；独立Python由原坐标求510.9929662664673 MU，正式510.99 MU。原版UI铁路2→1不变，1→0删除5连接（含1州内），29条／市场17strip、387.7629690309868 MU，正式387.76 MU。普通游戏变更后的市场／国内模型量与各自owned精确一致；没有用未知量替代国内模型物流。
- 同进程正常重载原存档，world1退休／world2恢复34连接、22strip及510.99 MU和原数量；我方正常关闭exit0／退休3。最终重新安装R128并启动PID60588核对实际双DLL、510.99 MU和完整中文提示后，正常退出0。拆除仅在我方未保存世界进行，没有改写原存档。
- 初次严格audit误用R127新游戏流程作同存档错误基线，FAIL及原脚本保留。临时经deploy.ps1部署R123，同存档／相同两次拆除及重载后，完整normalized error Counter与R128逐项／次数相同；最终R128启动Counter也等于旧版同存档启动。没有把已有空key报文称零错误。初局55440下次观察时已不在，退出原因／操作者／退出码未捕获，晚间另有一组已有报文，保留限制，不称整局零新增或我方正常退出。
- rail-audit.json PASS；证据与恢复入口：E:/Code/v3-analysis/checkpoints/2026-10-27-r128-built-track/{NEXT.md,rail-audit.json,live,logs}。先前R127改图场景属先前构建证据，新铁路字段未重跑全部改图矩阵。港口三末端、运河开放权限、更多省份／bitmap改动、渲染并发／新设备／完整生命周期、公里标定、3×60秒性能及最终同构建验收继续；历史R1200.12未解释，目标active。下方R128进行中记录为历史过程，以本完成记录为准。

### 2026-10-27 — R128进行中：当前已建铁路唯一地图长度

- 继续P3／P5，沿用已完整重读原01／02／03，备份26文件／五输入至checkpoint2026-10-27-r128-built-track；无游戏，安装仍R123。按MASTER允许的明确地图单位接线，不猜公里换算。
- 先复现7单位已有铁路／无货流仍被正式文档漏计，before-rail-tests.log实际FAIL。当前decoder补保留所有type4（含州内及跨市场）独立自有副本，不改变国内分配边；worker按当前同市场两端／唯一56位物理strip计二维折线MU，已知零／未知分开，身份／缓存及正式卡片／双语提示接线。解码目标回归PASS，完整视图／构建及实机尚待，不能以源码替代安装。

### 2026-10-27 — R127：真实目录／ZIP与replace_path覆盖验收通过

- 无源码／ABI／DLL变化，当前R1233be8cbcc…／46613962B、73源hash一致、46 PASS沿用。同60文件目录与deflate ZIP，17州定义全部改文件名，原生VFS实际25挂载／1replacement，州目录allowedRoots=1；新key州0／定义0／geo1500、两州11／8省及全部918陆路逐条相同，93国内海运模型连接保持。
- 目录53280市场2590031259fixed、国内2311645000、州0 359098837／州7 299463152，9030笔，正式219／231／207段；ZIP25876市场2596938903、国内2309998100、州0 364445907／州7 301342341，9062笔，正式222／243／188段。各自市场／两州与稳定owned全部精确相同，UI前后探针逐字不变；独立新局未固定seed，不做跨局数量因果推断。
- 分析目录绝对ZIP初试58968启动早退，exception为Abnormal Program Termination，末尾Invalid path passed in Directories；无退出码／模块枚举，不称正常退出。静态当前EXE归档分支使用用户mod目录+basename；相同ZIP放用户mod后成功，失败及原finish严格基线拒绝保留，不改产品／放宽检查。
- 两有效局我方正常退出0／退休2，原五输入／原资产保持、列表恢复，按hash仅移除本次创建的ZIP，无新增解析错误／存档／游戏。checkpoint2026-10-27-r127-zip-replace含审计、静态证据、截图及NEXT；本项不是任意ZIP位置／所有Mod或最终全矩阵通过。以下R127进行中记录为历史过程。

### 2026-10-27 — R127进行中：同内容目录／ZIP及replace_path实机夹具

- P3／P1恢复后完整重读00／01／02／03及MASTER §2.3.2／2.4；产品与安装仍R1233be8cbcc…，无游戏。备份10文档／双DLL／五输入到checkpoint2026-10-27-r127-zip-replace。
- 本机launcher明确zip扩展及loose-files支持；私有60文件夹具继承R125实际新key／geo1500，将17份州定义全部改文件名并声明replace_paths，目录和deflate ZIP逐文件完全相同。实际挂载／replacement与当前对象、数量／正式绘制尚待验证，不将格式配置当实机通过。准备时误将state_regions.md当txt的断言已修正，仅私有准备脚本，无产品改变。

### 2026-10-27 — R126：真实省份归属交换及道路端点重绑定实机通过

- 最终夹具除metadata，仅改州定义地理字段及两份历史owned_provinces；19份当前省份定义几何见证完整交换，geo1500／16与state0／7正确重绑，全部918陆路精确符合预期端点置换，其中12条改变、无额外／缺失。16个相同省份数值ID的物理见证不同；没有生产硬编码地图表。
- 两个有效局完整市场及两州数量与owned精确相同：原归属25864.09759／3597.24016／2932.73646、9019笔；交换后25742.04225／3070.78798／4024.27395、9022笔。市场／两州正式地图和local_goods实见，交换后UI切换前后整份探针输出相同。
- 初版夹具漏改历史owned_provinces，私有一致性校验拒绝；正常退出后只修夹具，失败保留，生产校验未放宽。中心像素颜色识别不精确，方法已拒绝；私有数值几何见证不进入产品。40612／6848／64968均正常退出0，原输入／列表恢复，原43源资产不变，无新存档／新增解析错误／游戏。
- 无产品源码／DLL变化，仍3be8cbcc…／46613962B；不重复46项。province-audit.json／owned-comparison.json及NEXT.md保存完整边界；增删／分并省、bitmap邻接、ZIP／replace_path及其他最终门槛继续，目标active。下方为本轮中间记录。

- 产品仍R1233be8cbcc…，无源码／DLL变化。两个真实目录夹具只在州文件及metadata不同，交换Midlands／Lancashire的完整11／8省份、geo及对应地理hub，保留所有物理样条／locator、历史与其他州；目标是逐physical key核对新道路端点，不用固定省份ID表。
- 原归属对照40612正常退出0、五输入保持／列表恢复；current州0／定义0／geo1500与州7／定义14／geo16，原生省份定义及运行归属11／8吻合。稳定9019笔、市场2586409759fixed、州0 359724016／州7 293273646均与正式一致，两州地图和列实见。
- 初次进入游戏后早期探针8922笔与稳定9019不同，保留两份，不把不同时点当同输入计算错误。省份中心像素颜色识别精确检查失败，拒绝该方法；当前19个唯一数字几何见证只用于私有跨场景对照，不进生产身份表。交换后场景继续；证据checkpoint2026-10-27-r126-province-membership。

### 2026-10-27 — R125：真实地理编号变更及当前几何绑定实机通过

- 无产品源码／双DLL变化，仍R1233be8cbcc…／46613962B。私有真实目录Mod在新key／州0／定义0基础上将geo20→1500，同步6点、12连接（3条反向规范化）及5类locator；坐标、strip ID、省份及历史保持，当前副本新点／key全部存在、对应旧身份全部消失。
- 自主64280/start04:58:46.1312722+08，VFS首根实际Mod／25mounts；正式州353800627fixed／国内341011600、8980笔与owned精确一致，918陆／93海，市场25790.44463／223段及州0 local_goods／234方向段实见。外部市场总量未独立打印，不作完整市场对账。
- 正常WM_CLOSE退出0／退休2，原输入字节保持、启用列表恢复、原43源资产未改，无新存档／解析错误／游戏。证据checkpoint2026-10-27-r125-geo-renumber/geo-audit.json及NEXT.md；只覆盖一个州一致重编号，不扩为省份拓扑／ZIP／replace_path或完整改图。历史0.12仍未解释，整体active。

### 2026-10-27 — R124：新局三阶段只读输入对照；未复现初始化数量变化

- 产品仍R1233be8cbcc…／46613962B，源码与双DLL未改／未构建／未部署。私有owned探针在关闭进程句柄后导出供需／州与市场／原版州对／海陆连接及当前几何；选择国家前、选择后、Start后74481行／2918052B逐字节相同，均8958笔／州360622169fixed。当前样本未支持“初始化输入变动”假设，历史0.12原因保持未确认。
- 13392会话正式owner始终0、未构建生产模型，不能作为正式数量验收。Oct27复查时进程已消失，末尾有原生退休事件，但退出码／操作者及原因未观测，不声称我方正常关闭或崩溃。原存档／设置／ini／continue字节未改，临时Mod列表按hash守卫恢复原字节，无游戏、新增解析错误0。
- 证据checkpoint2026-10-26-r124-initialization-snapshots/{initialization-audit.json,live,NEXT.md}；下一步独立geo／省份拓扑与ZIP／replace_path等实际改图门槛，整体active。

### 2026-10-26 — R123：国内分配枚举稳定性修复及真实州身份Mod实机

- 当前完整ID排序owned节点／陆路／港口边，修复同内容枚举改变等成本分配／物理路径；七种排列、平行段、海路顺序／cold与warm回归先FAIL后PASS。完整46 PASS、资源／GUI／GPU／自检560420／默认隐藏通过，主体3be8cbcc…（46613962B）已按deploy.ps1安装／实际加载。
- 真实目录Mod新key、州433→0／定义17→0／geo20保持，当前35004局正式州357465715fixed、9053笔与owned精确一致；两原生步后917陆／93海／9030笔、国内23017.375、市场25879.45727／219段与州244段实见。正常退出0，原输入及启用列表保持／恢复，无新存档／新增解析错误／游戏。
- 旧R120另一新局正式360976587／external360964587fixed差0.12，原audit.py失败保留；当前旧算法重放也一致，故历史差异未确认与排序同因。初版私有fixture中间BOM导致Svealand缺失／335错误，生产正确拒绝；只修私有fixture，失败日志保留。
- checkpoint2026-10-26-r123-state-identity含源码／DLL事前备份、current-audit.json及NEXT.md。完整改图、省份／geo重编、ZIP／replace_path及其他最终门槛继续，整体active。

### 2026-10-26 — R122：真实Mod目录覆盖及同key几何实机通过

- 产品源码／双DLL未改，当前仍90e577c3…。两私有Mod覆盖同一splnet，只改London港口strip364035中间point277150326的X，所有key／strip／控制ID／端点保持。实际VFS顺序upper→lower与反向均核对，当前内存X4071.94775／4061.94775分别采用胜者，撤销后4053.94775恢复。
- 生产同源owned重放目标strip有货流且ribbon含该新点；三会话正式市场218段／25963.19216、国内23098.111及曲线变化实见。PID48336／14144／67328均paused1/date59883360，WM_CLOSE退出码0；存档／设置／ini／continue未变，临时content_load原字节恢复，无新增错误，无游戏。
- 完成的是实际目录覆盖和单控制点同key几何，不把它扩成州key／ID／省份拓扑、ZIP／replace_path或进程内热重载兼容；下一步这些身份场景及其余验收。证据map-mod-audit.json／STATUS／PROFILE，整体active。

### 2026-10-26 — R121：三条港口缺段来源调查

- 无产品改动，安装／dist保持R12090e577c3…。当前只读副本显示：西密克罗尼西亚207／台湾936／北婆罗洲608的既有港口分别连海3002／3157／3056，均不在相应原版lane集合；本机州定义naval_exit_id3157／3056／3072与lane集合吻合。不能把另一海区港口补到原版路线上。
- 自启66308/start13:13:43.6375042+08，观测paused1/date59883360，owned重放93海／918陆／9038笔及三条最终量0.586／418.679／0。之后进程终止，日志有析构事件但未捕获退出码／操作者，不计正常退出或三处屏幕验收；原存档／设置／ini字节不变，当前无游戏。
- 原版025E8070仅查lane海区type3 key，未找到安全补段来源；未改游戏文件／启用新ABI或硬编码例外。下一单元实际Mod覆盖／同key几何及身份变化验收；港口完整图形仍DOING。证据checkpoint2026-10-26-r121-port-geography及STATUS／PROFILE。

### 2026-10-26 — R120：原生世界析构事件接入并部署／实机

- 精确EXE／独立Capstone沿全部州DB写点及析构调用链定位00811660原生世界聚合析构；机器120持有其对象，进入后逐项销毁含world1C00州管理器，晚处才清stateDb。选外层入口提前退休，void(this)由原生机器析构调用及释放wrapper交叉核对；不在析构线程读引擎数据库／跑模型／写GUI。
- NativeSnapshotMailbox新增匹配自有nativeWorld的退休事件，先推进epoch、换新resident request并清旧结果，防暂停首命令饿死和旧结果晚到；UI先退休不重复加代数。当前地址完全相同的私有例before epoch1退出1、after epoch2退出0；相关快照回归PASS。
- 精确签名与world析构hook正式接入，正式采集要求退休／完成点钩子均成功。完整46 PASS、资源／GUI归档／自检560422／默认隐藏通过；90e577c3…（46338126B）安装=dist=实机。自启400/start12:40:50.3461912+08，UI57852在直接重载／菜单退出／正常关闭时实际发出epoch2／3／4，采集58948首暂停快照分别world2/rev8、world3/rev12交接，无需推进。218市场段同量恢复且uploads218不变，州433列／248段及实际窗口缩放通过；退出码0、原存档／设置／ini未变，无游戏。
- 同址是helper回归，实机地址均变化；周期日志非每条提交与退休之间原子性证明。事件其他路径／并发边界／新设备及完整A24继续，目标active。证据及R118精确回滚checkpoint2026-10-26-r120-retirement-events/{retirement-audit.json,NEXT.md}。

### 2026-10-26 — R119：同进程重载／菜单／法国新游戏／缩放窗口场景通过

- 上轮R118为PROGRESS；本轮完整重读01／02／03及TEST_PROCEDURE，按真实读档／菜单／新游戏核对世界代数、旧帧撤回与原生池恢复，不把身份比较当退休事件／ABA证明。当前源码／dist／安装仍aff98195…，无产品修改。
- 自启47988/start11:06:15.4045767+08，同存档world2恢复218段且uploads218；菜单phase4→6→0撤帧，新开法国world3市场2／9878.27752／141段，8990笔模型与原存档9038不同；载回UK world4／25963.19216／218段，州433列／248段恢复。世界2／3／4每次原生池校验恢复前日志无SUBMIT，日志粒度不能证明逐帧退休事件。
- 实际最小化／恢复及2560×1440→1920×1080→原客户区通过，法国量及路线保持。正常退出，全程暂停／日期59883360，原存档／设置／ini字节不变、相关Mod错误0；无游戏。产品未改，不重复构建／部署／46项旧回归；完整事件／ABA／新设备及最终性能仍待，目标active。证据checkpoint2026-10-26-r119-world-lifecycle/{lifecycle-audit.json,live,NEXT.md}。

### 2026-10-26 — R118：国内连接运量不再受缺失绘图端点阻断，已部署／实机

- 已复现R117缺目的港口图形便删除真实国内连接：最小例目标500000／满足0／未满足500000；修复后满足500000／未满足0。当前原版kind0连接提供模型连通性，缺几何独立记录，不补造海岸端点；仅绘制有端点见证的唯一原版链。
- 精确EXE汇编及原版SHIPPING_LANE_BREAKDOWN_TRAVEL_DISTANCE提示链确认CShippingLane+60为原生1e5距离（未宣称公里），支持同海区合法零距离；国内海路模型使用该距离，原版国际路径保持严格完整端点要求。完整46 PASS、embedded／GUI实际归档／自检560400／默认隐藏通过。
- aff98195…（46324762B）已安装并自主实机61904：93海／918陆、partial3／unavailable0、9038笔模型；三条原缺端点边最终分配0.586／418.679／0，分别10／1／3个原版片段。市场25963.19216／218段，州433列／248段，中文筛选／空结果、20重开uploads359不变、稳定66秒uploads607不变。正常退出，原存档／设置／ini未改，无游戏。稳定首采跨上传期失败、错误检查初稿误包含既有基础资源错误均记录WORKLOG；实际本Mod错误0，基础token与R117一致。备份／证据／R117回滚checkpoint2026-10-26-r118-domestic-port-endpoints，完整目标继续active。

### 2026-10-26 — R117：国内物流／陆路及可编辑资源接入并实机

- 最新裁决MASTER §2.3.2：国际原版＋国内明确标注的模型分配，拒绝硬编码地图表。当前省份→完整州／当前连接与样条直接构网，原版进出口计入余额后MCMF分配国内剩余供需；目标／满足／未满足与Freight账独立，过滤不反馈。
- 统一46 PASS；国内变化缓存漏比先FAIL后修复。最终87b6d408…（46295447B）安装=dist，代码段与46 PASS的f0dccd6e…逐字相同，最后仅资源标题缩短；最终GUI实际归档／embedded／自检560369／默认隐藏通过。当前无游戏。
- 原版2865.08116＋国内23098.111＝25963.19216／218段海陆同屏；州433列与248段方向实见，衣物／空结果／hover／隐藏／关闭重开通过。Freight账30585.31／23098.11／7487.20。稳定145秒零上传、最终20重开uploads218不变，不等于完整性能验收。
- native-gui／本地化及原创asset／shader明文落盘，.defaults对照与保留编辑已跨升级／重启验证。一次测试//外层shader注释报错已记录并改#重验，最终相关错误0；测试注释已恢复。全部自启会话正常退出，存档／设置／ini字节不变。3条原生港口缺目的端点继续查，不补假线；改图／完整生命周期／里程／P95及最终验收未完。证据STATUS／PROFILE与checkpoint2026-10-26-r117-editable-resources-and-native-connections/NEXT.md。

### 2026-10-26 — R116：正式采集去掉旧地图／模型输入依赖

- 新当前typed州名册保留完整运行句柄/真实market，derived region显式未知；原生定义/geo/当前样条独立复制。正式CopyNativeSnapshot移除embedded regions/geography、旧orders/rails前置门；模型解码仍离线/调试保留。只读probe复用生产roster。
- 旧源码无地图请求reads0提前失败已实际复现；新入口进入原访问守卫。未知Mod key/缺旧表/不可读counter不阻止native roster，类型/完整句柄/市场/复制变化/退休仍拒绝。完整45 PASS、embedded/自检560470/默认隐藏通过。
- 主体f1ebed56…（43064787B），安装=dist=实际新采集，默认存档1147州/53商品/3192原版选择；市场2865.08116/112段实见。当前采集不读旧订单/铁路；只读新roster与完整商品对账通过。正常退出我方始终暂停会话，原存档/设置/ini不变，无游戏。
- 尚未把实际改图场景判为通过；下一步真实Mod覆盖/州定义与同key几何，开放权限、生命周期/设备、铁路长度和性能/最终验收。详见STATUS/PROFILE R116与checkpoint2026-10-26-r116-native-capture-map-independence。

### 2026-10-25 — R115：原版贸易正式后端与自主实机

- 新native_trade_view接正式worker/会话/文档/市场与地区列，数量原生1e5，与模型账分开；使用原版已选路径/当前端点和样条，缺几何保留数量且不补画。native/native去重忽略无关旧orders/rails，保留完整原生内容比较。
- 45 PASS、资源/GUI实际归档/GPU双向与负对照/独立加载/默认隐藏通过。主体11b3fadd…（43666727B）、引导71dae36d…（779879B），安装=dist=实际加载R115。市场0总量2865.08116/112段、state433贸易列/114段实见；中文搜索/空结果/hover/显示隐藏/关闭重开通过。
- 独立只读对账精确相同；全部3928原版候选映射闭渠连接0（当前3个未建定义底层连接仍在），巴拿马/苏伊士截图无跨地峡贸易箭头。一次原生日期+6后orders变化而原版事实相同，正式帧保持；首稳定视图70秒零新增上传，20次实际重开均缓存命中/总uploads315不变，内存未持续增长。
- 我方测试会话正常退出，原存档/设置/ini字节不变；无游戏，保留安装R115。完整改图仍受旧regions/rails采集门限制、真实铁路长度--，开放/权限变化、新设备/全生命周期/3×60秒性能和最终验收继续。详见STATUS/PROFILE及checkpoint2026-10-25-r115-native-formal-view。

### 2026-10-25 — R114：原生事实会话去重修复

- 旧源码最小用例复现“同订单关闭原生海峡仍判等价”，新精确owned比较覆盖当前中心/商品/原版选择/完整海峡/当前地图/端点；枚举重排保持缓存，语义顺序变化刷新。无engine访问/新hook。
- 新会话回归与完整入口42 PASS/零失败、最终构建/embedded/自检560363/默认隐藏通过。候选8d717ce7…（40051850B）未部署/实机，安装R10251ec975f…保持，无游戏。正式native视图/海峡可视修复继续，详见STATUS/PROFILE R114与checkpoint2026-10-25-r114-native-snapshot-dedupe。

### 2026-10-25 — R113：已选原版样条确定方向组装

- 新native_trade_geometry.h只把当前中心/原版所选几何连成唯一端点序列，保留原生56位strip/control/闭渠约束，不选新路。自主只读885条实选route中861条成链，对应2911笔记录；24条/63笔缺原版港口，保留未知。会话正常退出，存档/设置未变。
- 完整入口42 PASS/零失败；仅probe/测试使用新组合，dist仍6a83d933…，安装R10251ec975f…不变。正式native视图继续；先修复新复现的会话去重漏比nativeTrade问题，详见STATUS/PROFILE R113与checkpoint2026-10-25-r113-native-route-geometry。

### 2026-10-25 — R112：候选原生完成点/selector/刷新实机验收

- 临时实装同R111候选6a83d933…，自启44104实际完成点复制/372多候选原版选择器/owned UI交接通过：3192目的地全部已选。正常映射扫描码Space推进一个原生步并重停，新25395订单被采用；外部数量对账继续精确PASS。
- 原版物流页按预期显示--/frame0/没有模型回退，不当作正式海峡修复。正常退出后存档/设置/ini逐字节未变；只经checkpoint rollback/deploy.ps1恢复实装R102，dist候选保留，无游戏。
- 无源码改动/不重复测试，实际运行同41 PASS构建；正式worker/UI/端点方向及完整改图/寿命/性能仍待，详见STATUS/PROFILE R112和checkpoint2026-10-25-r112-native-capture-live。

### 2026-10-25 — R111：当前原版贸易中心/商品/航线缓存精确关联

- 依原版01221F180/013C4E70读取当前市场区中心与world+800完整查找，原生890细分/8B8分组逐项精确对账；不按旧组成员把当前中心改回缓存代表。实机发现5区中心差异，旧关联2333/2513项吻合，新关联4200→2513全部精确吻合；421笔原版缺缓存路线明确不可用。
- 新native_trade_routes.h绑定自有完整州句柄/数量/原版可选索引；新元数据进入完成点owned快照和缓存身份。41 PASS/零失败、资源/GUI/自检560418/默认隐藏vtx0通过；候选6a83d933…未部署，安装51ec975f…不变。
- 自主6176会话只读核对成功；正常Space尝试未推进，paused1/date59883360始终不变，运行态未验。正常退出后原存档/设置逐字节不变。正式原生worker/UI/端点方向/候选实际采集/海峡可视仍待，详见STATUS/PROFILE R111及checkpoint2026-10-25-r111-native-trade-association。

### 2026-10-25 — R110：原版港口连接与自主只读实机验证

- 当前定位点/transform读取、原版边界/type3 key/端点同分留首映射已实现并接owned快照，不调用原版高亮/评分，不制造最近海岸连接；已知空与未知分开，hub eligibility进入缓存身份。
- 用户授权自主验证后，正常启动R102/PID46096，暂停/日期不变，实读57组/4200商品记录/当前样条/12海峡定义/港口PASS；三条未建运河无运行记录。纠正代表州不一定是成员、注册类型实为CGameObjectLocator两项实机发现；旧源码负对照/回归覆盖。
- 修正后完整构建+统一入口40 PASS/零失败、资源/GUI/独立加载560441亮像素/默认隐藏vtx0通过。候选5c613bd6…未部署，正式native绘制/统计仍待。正常退出自启暂停会话，原存档/设置逐字节未变；当前无游戏。详见STATUS/PROFILE R110及checkpoint2026-10-25-r110-native-ports。



### 2026-10-25 — R109：原版出口方向/完整目录/内容缓存身份

- 原版提示及完整汇编证明逐州表owner出口到primary peer；source/destination明确，不假定common ID副本相等，原版数量保留100000精度，正反向分开。快照携带完整州roster；商品元数据改完整目录，不因没有生产/消费订单而漏掉贸易商品。
- native数值/方向/候选/所选路线/完整海峡身份与当前样条、海峡定义、地图geo锚点全部进入缓存内容身份；无关枚举顺序归一，原版稳定同价的候选/邻接顺序保留。同key换几何、闭渠、1e-5数量变化覆盖；旧源码两项负对照复现，当前回归通过。
- 完整构建+统一入口39 PASS/零失败，资源/GUI/独立加载560437亮像素/默认隐藏vtx0通过。候选f1ab7c9b…未部署，安装仍51ec975f…。正式原版端点/港口与worker/UI尚待，海峡故障未实机修复；未操作游戏。详见STATUS/PROFILE R109，备份checkpoint2026-10-25-r109-native-trade-identity。



### 2026-10-25 — R108：原生海峡定义/地区映射与闭渠几何约束单元完成

- 新native_sea_geography.h按当前已初始化TLS context复制CStraitDefinitionDatabase全部定义（含未建运河），按原版路径引用复制当前省份邻接/地区geo ID，并绑定完整运行州ID。运行海峡额外保留定义ID，避免三种ID混用；无getter/TLS初始化/锁/高亮调用。
- SelectNativeSeaConnections只映射原版明确选中候选：受海峡定义管辖的海区对须出现在该路径当前运行海峡记录中，不能凭省份集合/样条存在画线。关闭捷径、开通后原版改选另一候选、仍走绕行/关闭后旧句柄/同ID改地图锚点均有回归；无名称黑名单、无重算寻路/分配。
- 元数据已接完成点owned快照，连接选择函数与扩展只读probe通过离线检查，正式worker/UI仍未消费；新分支未实机。完整构建/统一入口39 PASS、最终地图锚点例针对性PASS、资源/GUI/selftest560446/默认隐藏vtx0通过。dist40e15298…/37209345B未部署，安装仍51ec975f…；当前无游戏，不启动/操控/退出。用户所见海峡故障未验收修复。
- 证据与恢复入口：checkpoints/2026-10-25-r108-native-straits及STATUS/PROFILE；下一步原版端点/港口与商品方向/完整目录、同源worker统计和正式绘制闭环。日志及MASTER_PLAN已同步。


### 2026-10-25 — R107：复制当前原版样条及地图尺寸，接入贸易快照

- 新native_splines.h复制原版当前点/分段/连接三表和实际地图宽高；完整56位分段ID与29位点ID分开，后台仅转换原有顺序/坐标，密集绘制边ID不截断原生ID。空连接保持空，同ID改变控制点保留新形状；几何存在不等于运河开放。
- 完成点CopyNativeSnapshot将几何与贸易副本一起联合复读、校验world/revision并交接，worker无引擎指针；外部只读probe同源复用。新分支未实机，正式路线/商品统计适配仍待，禁止旧模型回退继续有效。
- 完整构建及统一入口38 PASS/无FAIL/退出0；追加四种类型/重复ID例的针对性回归PASS。embedded/GUI结构与归档/独立加载560417亮像素/默认隐藏vtx0通过。dist候选d7ba1188…/36523135B未部署，安装仍51ec975f…/35151385B；无游戏进程，未启动/输入/退出。运河故障未修复验收。
- 回滚/静态汇编/日志：checkpoints/2026-10-25-r107-native-splines。Rtti明确旧记录中的“StraitTemplate”实为CStraitDefinition；下一步从该定义库取得含未建运河的真实海区对，与当前路径/运行海峡绑定，不靠原版名称黑名单。


### 2026-10-25 — R106：原版选择器接入采集；候选构建通过，未部署

- 独立汇编复核013BDBE0单个原生数组指针入参/RAX保留候选指针，严格较小代价胜出/同价留首；013BD930/013BD840只读距离/当前海峡费用，00CF6B70的有效数据库/完整handle路径只读。精确EXE hash及四个入口签名加入profile，Game绑定只由已验证的R101完成点采集回调调用；不调用原版高亮、不改界面选择。
- 新tools/test_native_trade_selector.py不启动/加载游戏：在独立测试进程保留RIP相对布局，仅映射原函数代码和合成只读输入所需数据页，运行21例，覆盖非首候选、稳定同价、完整海峡代数和当前双向费用改变胜者，调用后输入/费用逐字不变。代码页RX，其余保留页未提交；执行前核对完整EXE SHA256。该结果是原生代码隔离harness，不冒充游戏内ABI/寿命验收。
- CopyNativeSnapshot在完成点/访问所有权和目录/订单/铁路复制后调用生产原版贸易解码与选择器，联合复读及世界/日期/current再次核验；EconomySnapshot持有自有nativeTrade副本，worker无引擎指针。缺选择器即未就绪；FreightEngine对带原版数据但尚无几何适配的输入显式拒绝，禁止静默回退旧分配器/旧统计。正式几何/商品统计仍待接通，不能部署此阶段候选作为运河修复。
- 统一native/build/tests-round106.log退出0、37条PASS/无FAIL；最后新增访问门用例单独重编译PASS（snapshot-round106.log）。新增关闭/过期海峡调用前拒绝、回调中数据变更拒绝及原版快照零模型回退回归通过。完整主体构建及embedded/GUI结构与实际归档/新主体运行目录/selftest（560421亮像素）/默认隐藏vtx0全部通过。
- **dist候选**主体7116233d5758238c81b69c46b917b732b9e16fab4f4dbaeb214735f1f8256c89（36032042B）；**实际安装仍**51ec975f886d1f8859da720e7bb905740391b293c18294bfa4dd2aa54d362e43（35151385B）。引导仍71dae36d…/779879B。没有部署/自动启动/输入/暂停/退出游戏；本轮未检测到victoria3。运河故障未修复验收，产品状态不以候选测试代替。
- 备份/原生汇编/隔离harness/结果见E:/Code/v3-analysis/checkpoints/2026-10-25-r106-native-selector及native/build/*round106.log。下一步几何按原版当前路线和海峡通行结果映射，并核对州间记录方向/完整商品目录再接正式UI。额外静态发现025E7F00港口评分要求Interface1或hidden1，**不得在GameState2/hidden0完成点直接调用整套025E8450几何helper**；当前新增选择器无此断言，未启用该几何调用。


### 2026-10-25 — R105：原版世界市场/州间记录复制器与选路交接接口完成

- 新增native_world_trade.h：按现有完整州目录/原生商品目录长度复制实际海峡typed表、world中的贸易组/目的地/全部路径候选、明确海峡ID/省份区间/距离，以及逐州细分贸易定点记录。候选与当前路线分开；单候选按原版直接返回，多候选缺少已验证原版选择器时保持未选择，不取第一条。
- 可选选择器仅在adapter持有已验证访问范围时调用，校验返回指针恰在当前候选数组元素边界，转成自有索引；复制完成前复读全部输入/current。没有在本轮绑定/启用真实引擎选择器调用，测试中的回调是合成接口验证，不能当原生ABI实测。worker输入仍无引擎指针。
- native_world_trade_tests接统一入口，覆盖非首候选、无选择器、外来/中间/越界指针、选择前退休、完整州/海峡代数、零ID、组/哈希/计数/省份/目录/负值/不可读/任意组件变化和源释放；针对性运行PASS。完整入口初版36条PASS/无FAIL/退出0；后加选择器接口/单候选/退休用例后针对性重编译与探针编译PASS，其他生产源码未改。
- 复用live_shipping_probe增加--world-trade <已验证商品数>，只读复用同一头并联合复读州目录。该新分支尚未在活游戏运行：本轮复核时已无victoria3进程，不推断退出原因、不启动游戏。R104b的57组/3192目的地等活进程证据不冒充新解码器的实机通过。
- 正式快照/worker/UI尚未消费新副本，实际商品流方向/数量标签、原版选择器绑定和海峡约束下的几何仍需接通与验收；用户所见旧模型海路问题未宣称修复。产品仍51ec975f…，安装=dist，本轮未重建/部署主体。回滚与证据：checkpoints/2026-10-25-r105-native-world-trade；详细原版结构续接R104 shipping-discovery.md。



### 2026-10-25 — R104b：找到原版世界市场路线/双边记录；未切换部署

- 97条CShippingLane只覆盖港口连接与条约转运，不能当全部贸易路线。已沿原版海区面板/海峡缓存找到world+898的57组世界市场路线、3192目的地记录；每条保存1–3条ProvinceSet/海峡ID/距离候选，原版013BDBE0按原生代价取当前最佳，不能固定第一条。
- 原版逐州进出口总计数与细分/分组州间记录已分别定位并只读采样；细分/分组表同总量，不误称输入/输出或混用总目标与实际记录。原版当前海峡注册表9项，尚无巴拿马/苏伊士/基尔；模板省份邻接仍可能包含闭渠连接，所以不能只把省份集合转成全邻接线。
- 当前4240继续由用户手动，未输入/推进/关闭；产品51ec975f仍未变。用户再次发现无视海峡时已明确说明正式视图还是旧模型，未虚称修复。详细结构/函数/样本及下一步见checkpoint2026-10-24-r104-native-shipping/shipping-discovery.md新增R104b；native路径与真实贸易量正式替换继续进行。



### 2026-10-25 — R104：原生航线只读层完成；正式切换继续

- 用户确认海路、商品筛选与统计一起改用原版贸易数据，已写MASTER_PLAN §2.3.1覆盖旧模型口径。新增native_shipping.h有界复制CShippingLane typed注册表、完整handle、省份区间、国家/端点/条约及原版商船需求；不调用寻路或修改原版高亮。省份集合不是有序路径，原版商船需求不是商品运量，约定周转运量也不冒充实际交付。
- 新增native_shipping_tests并接统一入口：高slot/代数、零ID、类型/容量/范围/哨兵/重叠、不可读/变更/退休与自有副本寿命通过。完整native/build/tests-round104.log退出0、35条PASS、无FAIL；最后字段改名/零ID补例的针对性重跑通过。
- 用户手动进入存档后授权只读核对。PID4240/start2026-10-25 00:06:02/module7ff6b6960000/phase4，生产解码头复用探针成功复制97航线（93港口连接kind0、4商品转运kind1），逐字节复读通过；另核对条约27两条商品条款与海区邻接。仅外部顺序观测，不当作原子访问窗口或正式绘制验收。
- 产品仍51ec975f…/35151385B、引导71dae36d…，安装=dist，本轮未构建/部署；新读取层尚未接正式UI。用户再次报告航线无视运河时，已明确说明仍在显示旧模型，故障未修复验收。下一步原生几何/实际进出口关联与快照/worker/UI同源切换。游戏人类操控，不输入/推进/退出。
- 证据/回滚：checkpoints/2026-10-24-r104-native-shipping，shipping-discovery.md/static/shipping-assembly.txt/shipping-live.txt/treaty27-live.json/province-live.json；后续代码继续前保存对应原文件。



### 2026-10-24 — R104进行中：正式海路与商品统计转原版贸易数据

- 用户确认路线、商品筛选和工作量统计一起使用原版贸易数据，MASTER_PLAN §2.3.1记录优先覆盖。旧静态海路分配不能证明运河已开通，不以只加巴拿马黑名单结案。
- 当前精确EXE已静态定位CShippingLane typed数据库、构造/刷新、ProvinceSet区间集合及原版海路可视化链；集合不是有序点列，+58/+60在单位确认前不标货运量。准备有界只读解码和异常/变更回归，未启用新原生调用。
- 产品仍51ec975f…，本阶段未构建/部署，正式视图尚未切到原版数据，运河故障尚未验收。游戏继续用户手动；checkpoint2026-10-24-r104-native-shipping保存原文件及静态证据。



### 2026-10-24 — GitHub 公开仓库 SSH 上传成功

- 用户创建空远端后，git push -u origin main 成功，189 个已整理文件经用户 SSH 上传至 https://github.com/NoFires/victoria3-logistics-system 。main 已跟踪 origin/main。
- GitHub 未认证 API 确认 private=false / visibility=public / default_branch=main；git ls-remote 与本地 HEAD 6eadb085d3adba5389b0c46e36f5a92a0c779d8f 一致。本次追加发布结果再提交同步。
- 未上传 dist/native/build/日志/缓存或外部私有证据，未改产品源码/部署/游戏状态；许可和派生数据事项仍保留于上传说明。



### 2026-10-24 — GitHub SSH 已恢复，等待创建空远端仓库

- 用户添加公钥后，ssh -T 实际返回 Hi NoFires / successfully authenticated。用户指定使用 Git 命令，停止浏览器创建路径。
- 本地首次提交 2c81d8e，189 文件；origin 已设为 git@github.com:NoFires/victoria3-logistics-system.git。git ls-remote 仍返回 Repository not found；Git 协议不能创建 GitHub 仓库，需要先建立同名公开空仓库，再 git push -u origin main。
- 当前未推送、未创建远端；不把 SSH 成功当作上传成功。产品源码/DLL/游戏未改。



### 2026-10-24 — GitHub 公开上传准备：用户指定仓库名与 SSH

- 用户授权建立公开仓库 victoria3-logistics-system，并使用其 GitHub SSH；覆盖上一轮私有上传/命名建议。README 和上传说明同步名称；不修改内部 Ironroad 标识或产品行为。
- 本机 Git 2.49 可用、gh 未安装；默认及显式 ~/.ssh/id_rsa 连接 GitHub 均返回 Permission denied (publickey)，GitHub 未接受该公钥。未创建远端、未推送，需用户提供正确私钥路径或添加当前公钥并确认用户名。
- 修改前备份 E:/Code/v3-analysis/checkpoints/2026-10-24-github-public-upload；本轮只准备文档与本地 Git，不重建或部署 DLL，不操作游戏。许可与派生数据事项仍如实保留，不代选开源许可证。



### 2026-10-24 — GitHub 整理：完整源码索引与点子路线图

- 用户要求整理所有代码文件与点子以备上传 GitHub。推荐 Ironroad / ironroad-v3，未替用户创建远端、初始化 Git 或推送；保持原有目录与源码，避免破坏构建路径。
- 更新 README 至 R103 已知状态，清除早期未接实时数据/双DLL未实机等过时介绍；新增 docs/CODE_INDEX.md、IDEAS.md、GITHUB.md，汇总模块、全部文件、设计方向、上传范围及剩余许可事项。
- 完整开发快照排除 build/dist/缓存/日志，保留必需 data 与第三方代码；生成逐文件 SHA256 清单并校验 ZIP 一致性和文档链接。未改产品代码，无需重建或部署；产品状态和游戏操控保持原有约束。
- 备份与源码包：E:/Code/v3-analysis/checkpoints/2026-10-24-github-organization。公开前仍须作者选项目许可证、补齐第三方许可并核对派生数据分发范围；当前建议先私有上传。




### 2026-10-24 — R103：VFS目录替换/排除表与实际搜索字节已复制

- 用本地逆向技能按EXE7f53e326…核对03B4A8D0登记与03B4AF80枚举；replace表fs+48/54、stride40为root/path两字符串，exclude表+60/6C、stride20。匹配为大小写敏感字节比较：替换目录精确相等、排除原始前缀，不擅自添加路径分量边界。
- DecodeNativeVfsConfiguration扩展原解码器，实际searchPaths独立保留，规则/挂载整体复读；SSO/heap/UTF-8、数量容量/地址/终止/源变更与退休异常回归PASS，现有统一入口已含该测试。纯副本目录搜索上限支持多规则/优先级，无引擎调用。
- 用户会话57776/context0只读两次PASS：24挂载、0替换、1排除(gui/gui_editor)；地图目录允许24源，排除目录0源。未操作游戏，不修改其Mod状态。产品51ec975f不变；有效目录/ZIP枚举、源内容/地图派生与改图场景仍待。证据checkpoints/2026-10-24-r103-vfs-rules。

### 2026-10-24 — R102：近景海上路线高度修复已部署

- 用户确认拉近消失/拉远可见，两图保留。旧shader以max(GetHeight,0)贴海床；本机原版边界shader按_WaterHeight夹高。正式代码仅加jomini_water.fxh include及水位下限，保留0.35抬高/平面混合/深度测试，不改路径与经济。
- actual vertex shader GPU流输出64例覆盖海/岸/陆、多水位及平面过渡，验证坐标/UV；旧shader实际FAIL（水位19/海床-20时路线0.35），修复PASS；海床/半UV两个负对照拒绝。GPU检查接统一入口；初次入口因MSYS2 Python无Pillow失败，固定进入脚本前解释器后完整重跑34行PASS/0 FAIL。
- 51ec975f…已build/资源/GUI归档与布局/selftest560437亮像素/默认隐藏vtx0通过，并经deploy.ps1部署，安装=dist。用户保存退出后部署，未自动重启；用户随后明确确认海路高度修复成功。备份/日志/两张原图/安装清单checkpoints/2026-10-24-r102-water-height。

### 2026-10-24 — R101：完成点整份采集/常驻更新与暂停重载顺序修复

- 整份目录/订单/铁路复制移离UI计时器，仅普通命令完成callerCD2BC1、033BD750原析构前持原版scope时采集，自有副本邮箱交UI再交worker。常驻请求捕获最后暂停命令，运行态仍30秒节流；新版本覆盖ready，旧结果/错误拒绝。
- 两个实机反例均保留：519864首值偏3.18，269206重载先复制新源再由UI清掉。修复为原生先绑定world、UI采用且Ensure保留新world副本；针对性回归与完整入口PASS。
- 最终eed941b0…已deploy.ps1部署，安装=dist；自主52300（22:24:53）native61820/UI35204，暂停初始与直接重载world2均20,267,075.76，195路线恢复；中英格兰货运列/115路线可见。3轮hover进出0新采集/0null、9次汇总总量相同。
- 用户要求人类操控后停止自动输入；无自动运行/关闭脚本，保留52300。交接时原autosave hash/大小/mtime、ini均与测试前一致，未解暂停/保存。当前运行态更新未验，不扩展为可靠世界事件/ABA或性能通过。日志/回滚checkpoints/2026-10-24-r101-snapshot-handoff。

### 2026-10-24 — R100：模拟完成阶段线程/调用契约实机验证

- 原版job的vtable0441A6A8：+8=03B292C0尝试执行，+10=03B29270等待状态2完成；007C2540、00E78020与StateTrades嵌套013460B0/01346260均先执行再逐个等待，不能把+8误称wait。StateTrades任务044E6F38经01339950进入world+1C00/0133C0C0。
- 新增native_access.h只读诊断与异常回归，接统一入口；probe_access默认关闭，只有精确EXE/hash/入口字节通过后可挂0139E220，原调用转发一次，最多16次标量记录，不采订单、不发worker/GUI动作。主体d590417c…已构建并经deploy.ps1安装，完整回归/资源/GUI/自检560432亮像素/默认隐藏均PASS（38行PASS、0 FAIL）。
- 自主56932实机：16次均在模拟线程45624（UI38656），access2/hidden0/owns1/context1前后一致，每次date+6、machine/world/DB相同。限时脚本正常WM_CLOSE退出；原autosave hash/大小/mtime不变，原ini逐字节恢复，诊断现默认关闭。证据与R98旧DLL备份checkpoints/2026-10-24-r100-simulation-completion；P4-T01仍DOING，下一步验证普通命令完成阶段及暂停首份采集，再接自有副本邮箱。

### 2026-10-24 — R99：原版访问锁与模拟批处理边界取证

- 原版GameStateAccess全局0514E580、HiddenGameStateAccess0514E570、TLS[0]+5AE0持有标记/+92C上下文及033BD640/750构造析构链已按同一EXE核对；锁为0513E058→0513DCA0→root+590。00E765E0在并行批处理中主动解锁/回锁，故不能用窗口线程或取得单一锁作为原子采集证明；未新增原生调用/产品钩子。
- 自主49800（17:44:11）暂停300次与限时最低速度运行2200次外部只读观测，访问值1/2/3及线程持有标记发生切换。窗口内R98接受25394订单候选、整组109ms，无WARN/错误world失效；owner=0，不能据此宣称正式卡片/箭头本轮通过。采样非原子，仅作诊断证据。
- 限时脚本正常WM_CLOSE退出，原autosave大小/hash/mtime均未变。产品仍59e6d3d4…，不重复无变化构建。证据/回滚入口checkpoints/2026-10-24-r99-access-window；下一步验证CPartialDailyTickCommand实际执行链0139E220→007E0D80→007C8BD0的完成阶段与线程，再设计自有副本交接；P4-T01保持DOING。

### 2026-10-24 — R98：完整州订单安全复制已部署；未暂停更新取得实机证据

- 当前主体59e6d3d486a168f77d0dd7a7bcb632b5700755cce35ea43e64f5cb94a86b98c4（34,717,331 B），引导71dae36d…；源码构建=dist=安装。完整统一原生入口、GUI结构/实际归档、资源、自检560422亮像素、默认隐藏vtx0均PASS；native/build/{build,tests,gui,selftest,debug,deploy}-round98*.log。新增native_economy_tests接统一入口。
- 按本主题重读01/02/03全文。原0..2047枚举、按样本挑vtable及直接读取订单已替换：按当前DB完整slot表取全32位handle，验证CState精确类型、slot/代数、唯一对象和既有地区key/真实市场，批量安全复制订单。counter+8数据/+10容量/+14长度/+20位图与目录长度分配经当前EXE独立汇编/反编译核对，/100转milli-orders保持；容量/位图/负数/不可读拒绝，不伪装零。
- 回归覆盖slot2049/full handle0xAB000801、州/市场0、错type/slot/重复对象、容量/目录/位图边界、负值、不可读、跨州复制期间变化、退休及多解码器联合候选晚期变化。恢复旧2048上限的负对照实际FAIL，正常实现PASS。旧自动field/vtable探针的完整源码已随修改前checkpoint保留；可选历史诊断保留，主路径不依赖猜测或原生lookup调用。
- EconomyReadWindow在商品目录、订单、铁路均复制后复核联合观察。暂态铁路读取失败撤回scope并重试，不能仅凭复制失败制造新world。此机制拒绝已观察到的变化，**不构成原子模拟epoch/完整访问窗口证明**；可靠世界事件/地址复用及P4-T01仍未完成。
- 外部只读生产解码器在原R97暂停46736通过1147州/25396订单/43商品/34铁路，复制94ms；进程句柄关闭后模型重放20,267,075.76与R97一致。46736后续已不存在，正常关闭尝试未找到窗口、无新crash目录，不推断退出原因。最终R98自主47012暂停采集在DLL内订单31–47ms、整组93–110ms，正式卡片与195路线可见；这是采集耗时，不是P95性能验收。
- 最终R98以最低速度限时运行，截图从1836-01-01到01-07。窗口内订单25396→25391，新候选world1/rev7接受；工作量20,267,075.76→19,815,791.98，目标/已分配/未满足30945.01/14716.40/16228.60，195路线可见。该次采集整组93ms、worker138.546ms；窗口内无WARN、无错误world失效。仅一次实际变化，不能扩大为所有未暂停/建拆/原子性或性能通过。
- 47012由限时脚本正常WM_CLOSE退出，未保存；原autosave SHA256 fecc16f78294e294eda3211fdec9e9b89c2fd325685cd20992fd0a2a8398e804、9,176,649 B及mtime均未变，无需恢复。当前无游戏进程。备份/原eaf2安装DLL/原存档与静态/测试/实机证据：checkpoints/2026-10-24-r98-economy-copy；state-counter-static.txt、economy-production-decoder-live.txt、unpaused-{actions,report}.json、unpaused-09.png、59e6-session-only.log、tested-build.json。
- 剩余：验证真正原子访问窗口与可靠世界事件；实际铁路建拆/分市场里程与距离标定、改图实际资源/内容身份、新设备/不同存档、三轮性能及最终A01–A27。稳定复核不能代替上述门槛，目标继续。

### 2026-10-24 — R97：修复周期刷新和悬停清空统计；已部署并实机通过

- 用户报告货运刷新不稳定。先复现相同输入仍清帧与悬停使仍有效统计失效，两份失败日志refresh-round97-{before,hover-before}.log保留；修复后相关会话用例及完整统一入口PASS。
- 采集先确认当前roster仍属session，再逐值比较同world订单、商品全部属性、铁路集合和主市场。内容未变保留已发布快照/revision/frame，不取消worker、不清空GUI；不以hash代替相等。元数据失败清roster后不能走去重捷径，synthetic/retired/stale不匹配。
- 悬停不再发布null。UI统计专用校验要求同一allocation对象、同world/revision、相同基础筛选，仅忽略hoverGood；地图仍使用严格FrameIsCurrent。搜索、市场/州切换、关闭、换世界及同版本数据替换继续撤下旧数据。新增字段变化、悬停未完成、关闭/退休、synthetic及同版本替换反例。
- 当前源码构建=dist=安装：主体eaf2a3190398893fce1802f8d1083561849359f9aabc0b766b4d4d9d4da36d17（34,911,634 B），引导71dae36d…不变。完整build/tests、GUI结构/实际归档、嵌入资源、GPU、自检560437亮像素、默认隐藏vtx0均PASS，deploy.ps1部署；证据native/build/*round97.log。 实机自有ironroad解析命中0；宽泛关键字仍命中6条原版city/fleet资源消息，逐条与R96相同，未改动这些资源，分类见gui-error-classification.json。
- 自主暂停28748实机：市场155.109秒/五次相同采集无null发布、无GUI重写；20轮移入/移出共40次实际hover转换，卡片始终20,267,075.76，地图195↔146/127条路线，统计无清空。地区中英格兰75秒/两次采集保持owner15/gen46/frame1/115路线，uploads530不增，列值可见。空搜索正确0.00、清空查询恢复，关闭撤回旧scope。
- 直接Load同一autosave，world1→2，新快照rev19实际接受，池530槽重新校验后恢复195条路线；world2-recovered.png确认卡片/箭头可见且停用提示消失。去重未拦新world。仅覆盖同设备同存档；不扩大为所有设备/异存档/未暂停更新通过。
- 28748已正常WM_CLOSE退出；全程暂停1836-01-01、未保存，autosave仍9,176,649 B、修改时间2026-10-23 23:07:24。当前无游戏进程。备份/原1666安装DLL/全部证据：checkpoints/2026-10-24-r97-refresh-stability；refresh-report.json、hover-actions.json、state-stable-report.json、eaf2-session-only.log与截图可定位。
- 本单元解决已复现的无变化刷新及hover统计闪烁；真正数据变更仍按安全协议撤回并重算，未暂停采集原子性、实际铁路建拆/市场铁路统计、改图内容身份、新设备/异存档及完整性能仍未完成，不将本轮暂停测试替代这些验收。

### 2026-10-24 — R96：真实可用铁路接入，最终构建已部署并实机核对

- 当前源码构建=dist=安装：主体1666187dda0f9e1d0944c0af9f91024c36c1200ae3f42692fe625bfcea8d3fb9（34,865,300 B），引导71dae36d…。完整原生入口、嵌入资源、原版GUI结构/实际归档、GPU、D3D11自检和默认隐藏均PASS；证据native/build/*round96.log。首轮新增测试缺windows.h的编译失败已修复并完整重跑，不隐去失败日志。
- 生产只读采集取得2329潜在铁路连接、34原版当前可用铁路连接/34物理边；64位原生连接key与物理strip ID分开，格式3追加4271个一对多映射，原几何字节保持。当前type4表示原版当前可用铁路，不能扩大为全部已铺设线路。所有worker输入均为自有副本，失败撤回。
- 最终1666自主暂停会话35744中，市场0正式卡片20,267,075.76、195条ribbon可见；外部复制后关闭进程句柄的模型重放逐值一致，557条shipment使用18条已确认铁路边。仅在自有副本移除铁路的对照为17,857,680.87，未写游戏内存。稳定窗口379.219秒累计uploads保持195；这不替代P95/模拟速度验收。
- 同一会话直接重载日志确认world2重采集及195槽恢复；最终截图前进程已不存在，无新crash目录，不推断退出原因，不将日志扩大为该构建重载后的可见验收。测试未推进日期/保存。当前无游戏进程。
- 市场铁路卡片仍--：缺分市场汇总与距离标定；未暂停复制原子性、实际建拆、改图资源内容身份、新设备/异存档及完整性能仍未完成。证据/回滚：checkpoints/2026-10-24-r96-built-rail，1666-market-rails-visible.png、1666-model-replay.txt、1666-stable-uploads.json、1666-final-freight.log。用户随后报告刷新不稳，转R97优先修复。



### 2026-10-24 — R96候选：真实铁路读取与连接/物理样条映射

- 新native_rail.h已接UI线程经济采集：复制CTravelConnection当前类型4的铁路，按64位native key→物理strip列表绑定；复制变化/错误类型/图不符/过期拒绝。生产解码器在暂停61752外部只读PASS：2329潜在连接、34已建连接/34物理边，全世界长度771.864332地图单位（非km/非某市场卡片）。
- 原数据生成器丢弃links，导致一条连接两段样条无法正确绑定；格式3追加4271连接映射，几何字节与格式2完全相同。Python映射异常/原版源测试、新native rail测试及原生loader异常用例已加；完整入口首轮仅新增测试缺windows.h编译失败，已补并单独PASS，重跑中。
- 候选dist主体1666187d…（34,865,300 B），引导71dae36d…；资源/原生GUI归档检查PASS，尚未部署，安装仍8115373d…。原自主61752正常退出，未保存/推进日期。备份checkpoints/2026-10-24-r96-built-rail。市场铁路单位/分区汇总、实际铁路变化场景、完整生命周期/改图/性能仍待验。

### 2026-10-24 — R95：实际VFS挂载顺序解码与实机核对

- native_vfs.h复制冻结挂载链，按函数身份识别目录/ZIP，核对搜索顺序与复制期间变化；只输出自有字符串。异常回归接统一入口并通过（tests-round95.log）；live_vfs_probe从窗口线程TLS取得当前context，两次只读实测24项PASS。
- 已定位独立replace_path/排除规则，尚未复制或接资源解析/运行时派生图，不能宣称改图兼容完成。奥尔良地区45的8条相连样条全部铁路，当前builtRailEdges为空使其模型零分配；真实铁路仍缺。
- 备份/证据checkpoints/2026-10-24-r95-active-vfs。旧59720恢复时已不存在；新48816全程暂停1836-01-01，正常WM_CLOSE退出、未保存。DLL未变，hash=dist=安装，EXE与Ghidra副本hash一致。

### 2026-10-24 — R94：跨市场数据/筛选实机核对

同一8115373d…构建实测不列颠0→法国州455→法兰西市场2→不列颠0；法国总量1,862,020.64、煤157,972.82/8.5%，返回后英国总量17,857,680.87正确恢复，无旧值残留。州455最终分配为0且未满足234.08，其模型可达性原因尚需核对；UI零值不能替代路由正确性验收。源码/DLL未变，截图/日志见STATUS与checkpoint 2026-10-24-r94-cross-market。

### 2026-10-24 — R93c：读档恢复已取得实机证据，补设备失效闸门

原型39c18330…在同一进程完成菜单→重载world2与游戏内直接载入world3，均恢复180条箭头且uploads仍180；停用提示随后撤下。追加同世界设备/graphics失效拦截、swapchain重建前设备身份比较，避免缓存命中绕过检查。主体8115373d…（33,718,124 B）已安装=dist；完整检查、同设备菜单恢复及最小化/实际resize实机通过，新设备重建未验收；详见STATUS。

### 2026-10-24 — R93：世界代数闸门与原生池恢复候选

永久布尔停用改为world代数；新世界当前真实frame就绪后，渲染入口校验类型资产/注册成员、对象/路径/VB/CB、设备身份及复制稳定性，再清旧请求并恢复池。新增异常/过期恢复回归接入统一入口，完整检查PASS。候选主体39c18330…（33,714,028 B）尚未部署；实机恢复待验，不提前将A24判通过。备份2026-10-24-r93-world-recovery，详见STATUS。

### 2026-10-24 — R92：跨读档原生对象证据与停用提示

只读核对“存档→菜单→重载”期间92类型/371注册对象及path/VB/CB指针逐字一致；明确类型回调注册与world所有权不同，未据此解除安全闸门。补独立中英“读档后地图货流不可用”提示，数值仍按新世界发布。完整检查PASS，主体ccecb9ee…（33,044,323 B）已安装=dist，英文搜索与停用提示实机通过；已恢复中文，详见STATUS/WORKLOG及checkpoint 2026-10-24-r92-native-lifetime。

### 2026-10-24 — R91：需求账本地化及布局

底部订单账/物流需求账拆分标题与数值，模型状态本地化；不改变分配或原版经济。完整原生入口、GUI归档/资源、D3D11自检与默认隐藏通过；主体10840747…（33,039,715 B）经 deploy.ps1 安装，安装=dist，自主重启54564可读性复测通过。备份2026-10-24-r91-ledger-layout，详见 STATUS。

### 2026-10-24 — R90：真实市场身份及原版面板闭环

原版 State.GetMarket 链与1147州只读解码一致（真实305市场），替代错误的 +0x48 分组；异常/代数/复制期间变化回归接入统一入口并通过。主体93b6ec29…实机通过市场0卡片、中文搜索/分类/空结果、hover、显示隐藏、原版商品详情/右键以及切州更新；截图和原始日志见 STATUS R90 与 checkpoint 2026-10-24-r90-native-markets。读档恢复、改图、真实铁路与完整性能仍未验收。

### 2026-10-24 — R89：异步快照不丢失与地区紧凑数字（实机复测中）

- R88自主启动R87已实机发布州433工作量和69条原生ribbon，截图/日志在checkpoints/2026-10-24-r88-phase1-scope；但周期刷新后请求合并会丢掉快照，故不能判稳定通过。
- 新worker回归先失败（filter coalescing must retain newest pending snapshot）。Request现保留最新不可变快照，后续过滤任务携带；Reset清除，worker按shared_ptr身份避免重复复制/指纹扫描。复现与既有reset/cancel/缓存/会话测试通过。
- 大数货运列挤到价差：地区行采用k/M一位小数并加原版最小字体省略保护，市场数值精度及模型数据保持。新增格式回归先失败再通过。
- 完整构建/统一原生入口/GUI归档/资源检查PASS。用户授权自主实机，旧自动输入停止已撤销；测试会话始终暂停、日期未变，正常WM_CLOSE退出后经deploy.ps1安装主体c2b457d032d099d0a9d62e6f2fb9c86a23688c349b16b4fe8c06630b6de72465（32,968,986 B），安装=dist。自主重启pid55652，复测中。备份2026-10-24-r89-snapshot-mailbox。
- 市场身份静态发现：State.GetMarket实际沿所有者/特殊市场分支进入市场数据库，再用实际市场id取得GUI包装；旧+0x48聚类不是该getter的取值链。保持市场闸门，下一步只读复核真实链与分组。



### 2026-10-24 — R88：无人值守与一期边界同步

用户授权自主启动游戏并测试。R87完整原生/加载部署/运行目录入口零失败（tests-round87-full.log）；正常EXE入口-continuelastsave启动pid29272，待加载。Freight一期只保留revision 1需求账，满足量来自最终货流，不另建供给池/游戏商品；旧独立供给阻塞撤销，端到端验收仍未完成。Ghidra当前可用，静态EXE与安装哈希一致；市场resolver确认是惰性GUI包装注册表，禁止主动调用持锁/分配函数。



### 2026-10-24 — R87：实机地区链路已触发；共享枢纽分解修复已部署

- R86实机已识别州433/local_goods与关闭/切页，分配失败报cannot decompose allocated flow；截图布局正常，货运--，尚无正式数值。
- 新回归复现同枢纽多收货州被覆盖；修复为保留全部terminal并推进耗尽项，覆盖共享收/发枢纽、短缺、乱序。分配/缓存/会话/真实地图尺度检查PASS。
- 移除实机拒绝的两处on_start诊断控件并加GUI回归；归档/资源检查PASS。用户保存退出后经deploy.ps1部署主体2a56d04a…（32,946,396 B），安装=dist，新版实机待验。
- 备份2026-10-24-r87-shared-hub；实机日志/截图/只读探针与regression.txt已保存。市场身份桥及其余未决项不变，完整证据见STATUS/WORKLOG R87。

### 2026-10-24 — R86：地区货运后端首次接入运行时（实机待验）

- 地区后端已接运行时：接受快照构建州roster，UI定时器解码/仲裁，按州商品写工作量；先隐藏旧行，再复核owner/world/revision/source/frame/州ID并揭示。关闭/切州/元数据失败/读档撤旧。
- 地区不依赖GUI市场ID桥；新增回归覆盖GUI market拒绝但地区继续工作、州不可读。修复州诊断512B缓冲区被按1024B读取，指针槽数按缓冲大小推导。
- 完整原生测试、GUI/归档/资源、D3D11（560397亮像素）和默认隐藏PASS。主体d78dda6e…（32,939,385 B）已部署，安装=dist；实机数值待用户手动验。备份2026-10-24-r86-state-backend，完整证据见STATUS/WORKLOG R86。

### 2026-10-24 — R85：截图布局修复与市场商品列头

- 州表计入继承的55px商品图标，固定价格单元格边界；表头/行统一55|50|50|65|90|90|80|60=540。短标题本地价/市场价/价差保留完整提示，货运60。
- 市场表55|210|85|95|95=540；结余文本固定85×55居中，补齐分隔线和原版商品图标列头，商品/价格/结余使用原版排序。
- 构建、GUI回归、C++/Python归档一致与嵌入资源检查PASS；主体e72464e89b5662dae9d3fa628771958fbbb3b5e4b68e1a20e52979a2d693e5ca（32,340,108 B），引导71dae36d…；经deploy.ps1部署，安装=dist。实机视觉尚未验证，用户手动操作。
- 备份checkpoints/2026-10-24-r85-ui-layout（保留介入时R84p源码与3109a87f…主体）；证据native/build/{build,gui,deploy}-round85.log。新增宽度回归在修改前复现615px溢出，修改后PASS。




### 2026-10-23 — R84n/R84o：市场页列序定稿与州面板表头占满

- 市场页列序改为 `市场价格｜结余｜工作量｜占比`：原版结余列在 `last_column` 之后无法前移，遂让它在**我们页签内隐藏**（与两列依赖度同样处理），再由我们的片段用原版脚本函数 `Goods.GetMarketBuyAndSellOrdersBalance` 与原版格式键（`MARKET_PANEL_BALANCE_GOLD/PLAIN/BLUE`）自绘结余数值。
- 结余列头按用户要求改成**游戏自带图标按钮**（`sort_button` + `sort_button_icons/sort_balance.dds`，`alwaystransparent`，tooltip `SORT_BY_BALANCE_TOOLTIP`）。教训：`MARKET_PANEL_BALANCE` 是数值格式键（`[...|=Dv]`），当列头会渲染成空白；临时加的 `IRONROAD_BALANCE` 已撤回。
- 列间插入 `vertical_divider = {}`；市场页表头 `55|210|90|95|95`、行 `210|80|90|95|95`。
- 州面板表头/行重排为 `156|276|48`（表头 `39×4|96,96,84|48`，行 `48,48,60|96,96,84|48`，均 480），价格列 96/96/84 不再省略。
- 主体 `d232efff…`（32,277,114 B）已部署（安装＝dist），完整入口零失败（`native/build/entry-round84o.log`），实机 error.log 无 GUI 报错；回滚材料 `checkpoints\2026-10-24-r84o-balance-icon`。

### 2026-10-23 — R84k–R84m：列头对齐、市场页只留结余、州字段结构验证

- 州面板列头/列按“分组总和相等”对齐：行 `52+52+78 | 80+80+78 | 60 = 480`，表头 `40+40+40+62 | 80+80+78 | 60 = 480`（182/238/60 逐段相同）；脚本量过确认。
- 市场页只保留结余列：结余列头用原版键 `MARKET_PANEL_BALANCE`；行内两列“依赖度”加 `visible = "[Not(InformationPanel.IsTabSelected('ironroad_logistics'))]"`，只在原版页签显示。Python/C++ 镜像逐字一致。
- **州字段结构诊断**：`state field probe` 采集 1147 州 × `0x000..0x1FC` 共 128 个 dword，按 `state+0x48` 分组检验。实机 `candidates=1`，唯一命中 `+0x048`（52 组，值域 31..116）→ 州对象无第二套市场 id，`state+0x48` 是结构上唯一成立的州→市场键；缺口在面板侧 `panel+0x70`。
- 主体 `20927cbf…`（32,276,090 B）已部署（安装＝dist），完整入口零失败（`native/build/entry-round84l.log`），回滚材料 `checkpoints\2026-10-24-r84l-align`。

### 2026-10-23 — R84f–R84i：GUI 属性错误清零、列宽按实测收紧、表头格式统一

- 移除 `ironroad_logistics.gui` 中 crate `icon` 的 `margin_right`（引擎不处理，每次启动报两条错）→ 改用 `position/size`，并在 `test_native_gui.py` 加断言防回归。实机新会话 `error.log` 中相关命中 0。
- 用脚本实测原版列宽：`states_panel` 表头 540／行 480；补丁后行曾变 524（宽 44 → 新列被裁）。改为行 52/52/78/92/92/54 + Freight 60 = 480；表头 42/42/42/58 + 100/100/90 + 60 = 534，并把被压成省略号的“价格差异”列加宽回 90。
- 市场页两列 90→70、wrapper 180→140，标签缩短为“工作量／占比”（EN `% of freight`）。
- 两处新增表头由 `textbox` 改为原版同款 `sort_button`（`align = hcenter|nobaseline`，省略 `onclick` 引擎接受）。
- 主体 `64ab24f4…`（32,072,202 B）已部署（安装＝dist）。**约束**：原版商品行固定 `@panel_width`=540，`last_column` 已被价格块 210 + 右锚定结余 80 占满，“结余列前移 + 列头”需先让原版价格块让出宽度，已提交用户裁决。

### 2026-10-23 — R84e：重复 null 发布修复（发布频率约 65/s → 只在变化时）

- 复现：日志最近 400 行有 **399 条** `publish document=0`（每帧一次）；来源是 `DrawUi()` 在调试面板隐藏（默认）且无原生 owner 时的两处 `PublishLogisticsDocument(nullptr)`。
- 修复：`PublishLogisticsDocument` 增加发布键去重（文档指针＋`owner`／`world`／`revision`／`panelUnavailable`／`goodsUnavailable`／`worldRetired`），首次必发以保证初始 waiting/作用域文本写入。键抽到 `native/src/publish_key.h`。
- 测试：新增 `native/test/publish_key_tests.cpp` 并接入 `build_tests.ps1`（未发布键必不相等、null→null 相等、null 跟在文档后必发、六个作用域字段各自变化必发）；实跑 PASS。
- 部署与实测：主体 `01d2d708…`（32,079,882 B），安装＝dist，完整入口零失败；同一存档**整个会话只有 4 条发布**且都紧跟 `real economy snapshot accepted`，其后 `UI-thread variables published` 正常——证明没有把变量刷旧。30 秒窗口日志 +1.3 KB（修复前约 1,950 条）。证据 `native/build/round84e-publish-dedupe.txt`。

### 2026-10-23 — R84d：实机读档往返（A24）与三处结论收口

- 实机（用户手动 回主菜单→重载 autosave，我方不发送输入）：`world changed: invalidating economy snapshot, next world=2`；新 world 重采集 `states=1147 orders=25396 goods=43 markets=52 primary=33`，与读档前一致；全程 `owner=0`、无旧文档/旧帧提交、无崩溃。**安全失效 PASS。**
- A24 缺口定位：`InvalidateEconomy` 置 `m_nativeWorldRetired=true` 后**全仓库无 `store(false)`**，9 处原生路径以其为门、范围文本变成 `restart required` → 读档后正式视图不恢复、仍需重启，正是目标禁止的做法。解除需原生对象跨 world 寿命证据。
- 州探针在**完整采集**上复跑，结果与加载中那次逐字相同（12 个指针全 `0x624690`）→ 指针否定结论成立。`+0x374` 三次观测两次未填 → **弃用**。
- 存档侧：`meta` 确认玩家国家＝**大不列颠(GBR)**、1.13.11；`gamestate` 是二进制格式，暂不作证据源（探测脚本 `native/build/inspect_save.py`）。

### 2026-10-23 — R84c：州→市场对象指针诊断（否定）与地区列的连带依赖

- 新增 DLL 内一次性诊断 `state market probe`（`m_stateMarketProbeDone`，`InvalidateEconomy` 重置）：枚举 anchor 州 `+0x00..0x400` 指针字段，打印目标 vtable 的模块内 RVA。实机结果：anchor `id=0`、`state+0x48=112`，**全部 12 个指针都是容器 vtable `0x624690`，没有 `0x045850B0`** → 州与市场是整数 id 关系，指针捷径不成立。
- 连带发现：`CopyNativeStateRoster` 用 `orders[].market`（源自 `state+0x48`）建 roster，`NativeStateOwner` 以其命中为激活前提 → 地区列与市场卡片**同一闸门**，不是两条独立任务。
- 工具限制：Ghidra 对 getter `0x225AC90` 一带无函数体、脚本禁用；文档中 resolver `0x10571F0` 实际是构造 `GetSuppressionEffectDescriptors` 字符串的函数（地址记录已过期）。
- 主体 `1c3428e5…`（32,070,154 B）已部署（安装＝dist），完整入口零失败（`native/build/entry-round84c.log`）；证据 `native/build/round84c-state-market-probe.txt`，回滚材料 `checkpoints\2026-10-23-r84c-state-market`。

### 2026-10-23 — R84b：市场身份规范化拆解与只读诊断构建

- 静态复核 `MarketPanel.GetMarket` getter `0x225AC90` 取 `panel+0x70` 作 ID（PROFILE／NATIVE_GUI_PLAN），确认该字段属引擎市场 ID 空间。
- 用探针枚举 GUI 市场注册表（`module+0x059684A0`，布局取自 `native_goods_filter.h`）：`count=45`，id 集合 `0..25,30,31,36,49–54,59–62,65,88,106,109,110,113,0xFFFFFFFF`；快照的 `33/35/112` 全不在其中，面板市场 `0` 在其中。面板市场对象 `+0x374=33`（= 快照 primary），其余 44 个该字段 `0xFFFFFFFF`。仅单样本，Ghidra 该 vtable 目标多未识别为函数，故**不据此激活**。
- 新增只读诊断：`market_panel.h` 的 `simulationMarket` + `ReadGuiMarketSimulationCandidate()`（fail closed）；`overlay.cpp` 记 `native market identity gui=… sim_candidate=… in_snapshot=…` 与完整 `real economy market ids:`。`NativeMarketOwner` 激活条件未改。新主体 `960d42fe…`（32,065,546 B），18 项回归与 `verify_embedded` PASS，**未部署**（游戏仍在跑）。
- 记录环境依赖：完整测试入口要求无 `victoria3` 进程（`test_freight_deploy.py` → `deploy.ps1` 守卫）。
- 第二样本否掉了 `+0x374`：另一会话/DLL 诊断行 `native market identity gui=0 sim_candidate=4294967295 in_snapshot=0 snapshot_markets=52`（该字段未填），而上一会话是 33。同时实证 GUI 注册表是**惰性 GUI 本地空间**（未开面板 `count=2`，开面板后 `14`，曾达 `45`），与快照 `31..116` 基本不相交。结论：R53 的 `state+0x48 = market id` 属聚类推断、需重验；正解是沿**州对象→市场对象指针**取市场 id（与 `panel+0x70` 同空间）。诊断构建 `960d42fe…` 已部署（安装＝dist），完整入口零失败（`native/build/entry-round84b.log`、`native/build/round84b-market-identity.txt`）。

### 2026-10-23 — R84：构建可复现、完整入口零失败、双DLL实机加载证据

- `native/build.ps1` 两处链接加 `-Wl,--no-insert-timestamp`；连续两次完整构建字节一致（引导 `71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e`、主体 `4ac3a0e8873dbe75ce945438a1ce476c8583e29d1f3f8d610245ac19e29f2980`）。与 R83 旧构建逐字节相比只差 6–7 字节（PE 时间戳、派生校验和、调试目录副本），证明自 R83 起源码未漂移。
- 完整入口零失败（`native/build/entry-round84.log`）：18 项原生回归＋引导／部署／运行目录、verify_embedded、原生 GUI 归档与运行时、flow GPU、D3D11 selftest（560376 亮像素）、默认隐藏 vtx0。无游戏进程时经 deploy.ps1 安装，安装＝dist＝源码构建。
- 实机（R83 构建，22:09:19 pid 59636，用户手动启动）：双 DLL 加载路径、系统导出转发、`freight` 目录配置／日志／数据、嵌入资源先于 VFS 冻结与 .asset 枚举挂载、ImGui 默认关闭均取得进程内证据；进入存档只读读到 1147 州／25317 订单／43 商品／52 市场。证据 `native/build/round84-stage2-session.log`、checkpoint `2026-10-23-r84-baseline`。
- 未完成：市场面板未打开，`publish document=0` 全程，正式文档与地区列仍未上屏；刚部署的可复现构建尚未实机启动；改图／Freight 供给／真实铁路／生命周期／性能不变。
- 追加（22:23:44 pid 48420，用户手动开面板）：新构建第三次实机加载正常；新增只读活进程解码器 `native/test/live_panel_probe.cpp`（复用生产解码头，不 attach／不写）证实 `phase=4`、vtable 全 MATCH、slot 9=`market_panel`、slot 14=`states_panel`；市场置顶时 `marketOpen=1 visibleGoods=40`，州面板读到 `state=433 local_goods`。真正缺口是**市场身份**：`panel+0x70=0` 不属于快照 `state+0x48` 的 52 个市场 id，owner 正确拒绝，`publish document=1` 为 0。`NativeStateOwner` 已实现但未接入 overlay。证据 `native/build/round84-panel-probe.txt`。

### 2026-10-23 — R83：freight目录与引导／主体拆分

- 用户确认EXE旁version.dll引导＋freight/freight.dll主体，目录／配置／日志统一freight命名；替代旧单DLL要求，已构建并经deploy.ps1安装。原有回归、独立引导／迁移／安装回滚测试、资源／GUI校验和D3D11／默认隐藏PASS；完整测试脚本曾因测试环境失败，修正后相关新增测试单独通过。
- 新双DLL实际游戏启动及早期挂载仍未验；未操作游戏。当前hash／证据见STATUS R83；备份2026-10-23-freight-loader含旧安装DLL，旧配置保留待首次启动迁移。

### 2026-10-23 — 新增独立安装／运行目录要求

- 用户要求DLL放入 `common\Victoria 3\binaries` 下独立文件夹，配置、日志和缓存一并集中；默认目录名 `IronroadV3`，详见MASTER_PLAN §3.1.1。
- 当前源码／部署仍为EXE旁version.dll＋LocalAppData运行文件；只更新需求和待办。子目录加载入口待验证，根引导＋主体双DLL与原单DLL要求的取舍未裁决，不擅自改变部署结构。备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-isolated-runtime-requirement`。

### 2026-10-23 — 文档精简与逐条对照

- 完整核对四份原始文档，合并重复要求／重读指引；保留用户覆盖要求、P0–P7和A01–A27，逐条去向见 [DOCUMENT_COMPARISON.md](docs/transport/DOCUMENT_COMPARISON.md)。
- 历史日志原文移至 [AGENTS-through-R82.md](docs/transport/history/AGENTS-through-R82.md) 与 [MASTER_PLAN-through-R82.md](docs/transport/history/MASTER_PLAN-through-R82.md)，完整旧文件备份于 `E:\Code\v3-analysis\checkpoints\2026-10-23-docs-trim`。不更改源码、DLL或实机验收结论。
- 后续本节仅追加本轮结果／证据入口，详细实验记 `WORKLOG.md`；每个工作单元仍必须同步 STATUS。
