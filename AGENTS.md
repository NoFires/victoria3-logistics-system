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
- **数据语义**：`targetImport`、`allocatedImport`、`unmetDemand`、区段吞吐、`shipmentFreight`、费用必须分开。正式线宽只用最终 `allocated quantity`；没有数据用 `--`，不得用 0 或测试值伪装实时数据。实时分配必须标注“模型分配”。
- **Freight 中转商品**：必须以 Freight 作为中转商品计算物流需求，遵循 `MASTER_PLAN.md` §4.5；目标／已满足／未满足分开，保留来源商品贡献。R67 已实现需求核算（默认系数 1 是模型假设）；一期不另建供给池、不注册游戏商品；满足量来自最终货流，尚需端到端实机验收。
- **地图身份**：运行州ID、引擎州定义ID与派生地图ID分开；按当前实际州定义key绑定地图，改图Mod须读取其实际加载的省份／样条并绑定图版本，不能将原版图冒充兼容。
- **禁止反馈**：过滤、悬停、换色、显示/隐藏只改视图，不得修改价格、MAPI、订单、建筑、人口需求、分配结果或市场分母。
- **线程与所有权**：渲染回调不扫描州/建筑数据库、不跑分配/Dijkstra、不等模拟锁、不执行 GUI 脚本效果。worker 不持 engine 指针。异步结果必须带 `world`/`revision`/generation，过期丢弃。
- **原生安全**：未验证 ABI/生命周期保持 disabled；不跨构建复用地址、不按“第一个候选”取值、不全局改共享 vtable、不重构造已注册对象。
- **不破坏用户工作**：无 Git。改源码前备份到 `E:\Code\v3-analysis\checkpoints\<日期>-<主题>`，不盲目恢复旧 `dist`。
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

唯一当前状态为 [STATUS.md](docs/transport/STATUS.md)，详细任务、阻塞和决策分别见同目录 `TASKS.md`、`BLOCKERS.md`、`DECISIONS.md`；旧轮次中的部署／进程描述不当作当前指令。

截至R103：产品仍为51ec975f…/35151385B、引导71dae36d…，安装=dist；用户已实机确认R102海路高度修复成功。R103仅扩展未接DLL的native_vfs配置解码/只读探针与回归：挂载顺序、实际searchPaths、replace/exclude表一组复制，纯副本目录策略已按原版字节比较复核；离线异常/变更用例PASS，用户57776两次只读24挂载/0替换/1排除PASS。尚无实际有效资源枚举/地图解析/内容身份，不能称改图兼容完成；下一步实现同规则的目录/ZIP枚举及源内容复制。游戏保持人类操控，不自动输入/启动/退出；其他未验项见STATUS。

## 6. 进度日志（简要记录，最新在上）

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
