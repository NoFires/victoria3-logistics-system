# 交付说明（Victoria 3 1.15.0）

> **R264当前安装（2026-11-10）**：主体27cefa0a5f601068360aa9be7d7fbadbeca5169b26a69d403a6b9372a1bdb7e0（52,197,771B）/引导71dae36d…，安装=dist。链路：R262 需求账日志行＋G1 真档 41 商品对账（R263 州/第二市场，G1 关闭）在 0ea4caf3 基础上构建本主体；R264 同构建 A25 三对（P95 +0.379/+0.430/+0.394 ms≤2、模拟零降幅）＋20 轮（零上传/池 206/内存平台）重绑通过。官方入口 72 PASS。**已知限制（BLOCKERS R262）**：Gora UI + UI Mod 使物流页签不渲染；另一存档/返回菜单生命周期子项绑旧构建。回滚与证据见 STATUS R259–R264 与 checkpoint2026-11-09-r261-perf/PERF-ARMS.md、checkpoint2026-11-09-r262-g1-matrix。

> **R257当前安装（2026-11-09）**：主体0ea4caf37f40fe410fb2b4c07b612f805e8e4855c971cb76693eac8d44b189f7（52,200,309B）/引导71dae36d…，安装=dist。共享地区海侧方向补到0缺失/38部分；岸端保持未完成，不借其他州港口陆路。正式完整71条PASS/部署/新档原生计数与页面通过，五输入保持/无游戏。两条新补海段当前量0，不产生可见线；完整路径/最终同构建矩阵仍待。dc85回滚副本在checkpoint2026-11-09-r257-sea-endpoints/before，未实机回滚演练。

> **R256当前安装（2026-11-09）**：主体dc85e9e5f9fbb4a4877196f4a525349ee688641852200d1fa89280fd18c2ea2f（52,190,069B）/引导71dae36d…，安装=dist。悬停市场或州本地价格的物流数值可看该商品国内Freight目标/满足/未满足；复用原版提示，默认系数1明示，合法零/未知分开。完整官方71条PASS/部署，隐藏新1.15市场/州/分类搜索/排序提示通过，退出0/五输入恢复；英文仅静态验证，完整同构建性能/地图/生命周期/设备/海端/回滚仍待。回滚1ac双DLL及源码在checkpoint2026-11-09-r256-good-demand-ui/before，未演练。

> **R255当前安装（2026-11-09）**：主体1ac0783b85c0eb5947cb42b9235d913a74f2f788a09960c44be81a7cbe189a39（52,198,543B）/引导71dae36d…，安装=dist。共享快照校验合并读取，完整官方检查/资源/自检/默认debug/部署通过，SPACE实机复制188–219ms、等待态后当前数值恢复；原输入和全部存档保持。仍非零卡顿，完整新构建A25/地图/生命周期/设备/交互/回滚待验。回滚bc967双DLL在checkpoint2026-11-09-r255-copy-cost/before，未声称实机回滚通过。

> **R254补验（2026-11-09）**：产品仍bc96745c…/71dae36d…，5改图正例当前绑定/41行物流/206原生批次及替换缺文件差分负例补证；原输入/全部存档恢复、无游戏。负例自然退出1与清理竞态留档，不算正常启动通过。暂停重复采集已减少，但单次约0.4秒同步采集、静止编辑最终刷新及完整生命周期/几何/设备/UI/交付仍待，见STATUS R254。

> **2026-11-09 R262当前安装**：主体 `27cefa0a5f601068360aa9be7d7fbadbeca5169b26a69d403a6b9372a1bdb7e0`（52,197,771 B）/引导 `71dae36d…`，安装＝dist。R260–R262 链：R260 任务书刷新；并行会话 R256–R258 交付逐商品原版提示、快照校验读合并（~2×）、共享地区海侧方向恢复（2 unavailable→0）；R259 生命周期重绑（0ea4caf3）；R262 新增基础作用域需求账日志行并完成 G1 真档 41 商品逐商品对账（求和=总账、逐行守恒）。官方入口 72 PASS。**已知限制（BLOCKERS R262）**：第三方 UI Mod `Gora UI +` 启用时铁道路物流页签不渲染。回滚与证据见 STATUS R259–R262 与 checkpoint2026-11-09-r262-g1-matrix、checkpoint2026-11-09-r254-lifecycle、checkpoint2026-11-08-r252-perf。

> **R248改图补验**：相同13f2aeb9…已通过省份交换ZIP及同key样条X+10的当前绑定/正式物流发布/206原生批次，数值可见、正常退出、配置与全部存档保持。其他改图夹具与逐点像素/设备/前台性能/海端仍未全部完成，见STATUS。

> **R247生命周期补验**：相同13f2aeb9…已通过同档重载、另一份1.15同国家存档、返回主菜单与尺寸/最小化/缩放；输入保持/正常退出。R246缓存65秒/20轮也已通过。A24真实设备丢失、跨国家新游戏、当前完整前台性能/改图及海端仍未全部完成，见STATUS。

> **2026-11-08 R245安装（已被R251取代）**：主体13f2aeb9…（51,924,020B）/引导71dae36d…，官方71 PASS、资源/自检/默认关闭/部署及同构建新档市场/州物流 -- 修复验收通过。数值恢复不等于海端全修复：仍2 unavailable/36 partial；完整A25/改图/生命周期未重绑。回滚和证据见STATUS R245。

> **2026-11-08 R239 当前构建（最终）**：主体 `26b70987b66df6e209f27b4b6afa3930c861f9c4da4fe1f1149c7ca5f6aa2cf2`（51,840,048 B），引导 `71dae36d…`，安装＝dist。当前构建证据链：官方 build/build_tests 71 PASS/embedded/selftest/默认debug；实机启动与数据身份（R230–R236 链）；市场/州物流列原版排序＋居中 k/M，切州自动重排实机通过；同构建 A25 三对（P95 +0.456/0.421/0.563 ms ≤2，模拟最大降 0.66% ≤5%）＋20 轮缓存（uploads=0 reused=206 pool=206 repeats_skipped=1…20）；部署矩阵缺失126/损坏193/卸载无条目/重装0（四例存活、退出0，R237）；生命周期重载 world2 池恢复 slots=206＋冷重建、缩放往返与最小化恢复零退休（R238）；改图 geo/样条/ZIP 三夹具探针精确绑定（geo=901、X+10 delta=0）、页面 206 ribbons 冷建、content_load 恢复（R239）。明示未完成：A24 设备丢失重建需真实 GPU/显示变化；1 条 unavailable＋38 条 partial 海路缺当前港口接续；跨国家新游戏分支与逐像素多相机矩阵未在本构建重复。详见 STATUS/PERFORMANCE/ACCEPTANCE 与各 checkpoint。

> **2026-11-07 R235 当前测试安装**：主体`cfd8f6324ed973c42321ec4a4dbc9c61226a821372c683c3d0f66d760816a352`（50,891,737B），引导71dae36d…。cfd8f632…／71dae36d…已完整构建/统一69条PASS/资源/自检560375/默认debug/官方部署。先复现真实BGRA晚帧红蓝交换，再按Format修复；隐藏新档实际Format87/颜色正确。19动作与清理后前台/指针保持；原版查找器定位伦敦，远近景海路可见，伦敦周边物流列/方向与海陆筛选256/123/133/62/194段，统计固定/零新增上传。WM_CLOSE退出0，设置首轮自动恢复/五输入当时exact。Escape尚未关州页、海段实际方向/全端点及完整矩阵待续；中断后其他会话改过输入，已分开记证；不push，goal ACTIVE。

> **2026-11-07 R234 当前测试安装**：主体`67d8f802a1c8476396f1966ffd9437e6999f4117dce7434bb7e0af44bacfcf1e`（50,880,559B），引导71dae36d…；源码/构建/安装一致，完整67条PASS/资源/selftest560445/default-debug/官方部署通过。新增仅显式隔离桌面启用的开发测试支持，玩家默认无指针hook。隐藏新档实际原版物流/分类/中文搜索/空结果/隐藏显示、208ribbon及GPU读回通过，真实桌面/指针保持；正常退出/五输入恢复。读回通道与完整最终矩阵仍待，详见STATUS R234。

> **2026-11-07 R232 同构建实机补充**：当前主体03182e3b…/引导71dae36d…未变。新档真实物流页/分配与208条正式ribbon、分类/隐藏数量、20次重开0上传/池318、65秒后台稳定通过，退出0/五输入恢复。完整端点/前台性能/生命周期/改图仍待。旧后台对齐窗口自动点击会移动指针，已停用；后续自主验收先验证隔离，详见STATUS R232。

> **2026-11-07 R231 当前测试安装**：主体 `03182e3b26aa5b4f216729cf446e71e78d0f562dd872bfe3854e744cf6912138`（50,835,433B）＋引导 `71dae36d…`，源码/构建/安装对齐。完整统一60条PASS，资源/自检560410亮像素/默认debug/官方部署通过；同构建新档最小化加载与正式采集4290/4290选路通过，5输入保持并退出0。只读371港口/342共享keyless陆路已核对；UI/实际分配/完整矩阵仍待，不把原采集成功扩成绘图或性能验收。详见STATUS R231。

> **2026-11-07 R229 当前测试安装**：主体 `716fd6892523eab6bc14fc17dfe4d2f78556815febf6e0e0ae43b8929ebaf1d8`（50,835,433B）＋引导 `71dae36d…`，源码/构建/安装对齐。完整统一60条PASS、embedded、selftest560412亮像素、默认隐藏通过；47保留文件与5输入保持。仅接有当前原版中转引用的无key地面段，无引用起终区连接/海峡/完整实机矩阵仍待。以下旧构建结果不能外推，完整目标未完成。

> **2026-11-07 R228 当前构建**：源码与 dist 主体 `bbee96cb61931355b9c8dfb6d82b91cb883cc0dc84c7a8214dd894b4cc93d3ae`（50,651,488B），引导 `71dae36d…`。完整统一 build_tests.ps1 exit0（60条PASS、零FAIL），embedded／selftest（560442亮像素）／默认debug零顶点均通过；经 deploy.ps1 安装 exit0，安装＝dist（bbee96cb…／71dae36d…），47个保留文件与5项受保护输入逐字节保持；真实游戏加载未验收。新增有效重合港内段的绘图处理；下方 R227/R226 数字仅作历史，不替代本构建实机/性能验收，完整目标仍 ACTIVE。

> **2026-11-07 R227 候选，未部署**：源码及 `dist/freight/freight.dll` 已更新为 `21a82419e8c8fbf843cc8fb75ea81bedf9860abbac689e7485da9ff0d53eb9a2`（50,650,464 B），引导仍为 `71dae36d…`。修复主城直线接港及无见证端点的海路猜向，改用当前 kind6 实际港内连接。完整统一 build_tests.ps1 exit0（60条PASS、零FAIL），embedded／selftest（560390亮像素）／默认debug零顶点均通过；实际安装仍 R226 `404abb88…`／`71dae36d…`，与当前 dist 不同。下方旧构建实机和性能结果不能用于本候选；状态与恢复入口见 STATUS R227。

> **2026-11-06 历史已验收构建（R225）**：正式线已合入三项经实机验证的更优功能——完整贸易 getter
> `D039C0`＋每州当前 hub `DA2500`、商品缓存代数／失效检查、`state+0x8C0` 分组表＋
> `detailed==grouped` 等值绑定。当前构建主体 `404abb88…`／50,220,030 B，引导 `71dae36d…`；
> 旧已验收 `2028e03b…`／`c8020412…` 保留为回滚副本。
> 同一 1.15 存档含 5 个多 hub 区域（2/3/4 个中心），实机 `selected=4290/4290`、201 ribbons、
> 贸易量13982.47687／铁路510.99MU不变，绑定未报 `native detailed and grouped exports differ`。
> 完整等价入口35/35、两个目标 EXE 机器码 harness、embedded／GUI／GPU／loader、selftest
> （960×960／560,432亮像素）通过；同构建三对 P95 `+0.894／+0.818／+0.863 ms`（≤2），模拟最大
> 降幅 `0.31%`（≤5%）；20轮稳定revision `uploads=0 reused=201/200 pool=230 overrun_frames=0`。
> 设备丢失重建（A24）仍未完成；官方 PowerShell 测试／部署／selftest 已实际执行，其余 A01–A27
> PARTIAL 见 `ACCEPTANCE.md`。
> R225 验收当时安装＝`dist`（`404abb88…`／`71dae36d…`），`freight.ini` `df06e5c0…` 保持；旧主体以
> `freight.dll.r196a-backup` 保留。

本文件是当前交付的唯一入口；状态、验收与性能细节见 `docs/transport/STATUS.md`、
`ACCEPTANCE.md`、`PERFORMANCE.md`，工作日志见 `AGENTS.md` 第 6 节。

## 1. 交付物与身份

| 文件 | SHA256 | 大小 |
|---|---|---|
| 历史回滚引导（c8020412） | `c80204122791ade4a33f5b61ea28ebb3a96f444bbc6ef215e71a6245af67a107` | 779,879 B |
| 历史 R227 未部署候选（checkpoint归档） | `21a82419e8c8fbf843cc8fb75ea81bedf9860abbac689e7485da9ff0d53eb9a2` | 50,650,464 B |
| 历史 R228 测试安装（本轮installed-before归档） | `bbee96cb61931355b9c8dfb6d82b91cb883cc0dc84c7a8214dd894b4cc93d3ae` | 50,651,488 B |
| 历史R229测试安装（R231 installed-before归档） | `716fd6892523eab6bc14fc17dfe4d2f78556815febf6e0e0ae43b8929ebaf1d8` | 50,835,433 B |
| 历史R231/R232/R233安装（R234 installed-before） | `03182e3b26aa5b4f216729cf446e71e78d0f562dd872bfe3854e744cf6912138` | 50,835,433 B |
| 历史 R234 测试安装（installed-before） | `67d8f802a1c8476396f1966ffd9437e6999f4117dce7434bb7e0af44bacfcf1e` | 50,880,559 B |
| 历史 R235 测试安装（installed-before） | `cfd8f6324ed973c42321ec4a4dbc9c61226a821372c683c3d0f66d760816a352` | 50,891,737 B |
| `dist/freight/freight.dll`（当前 R236 构建） | `26b70987b66df6e209f27b4b6afa3930c861f9c4da4fe1f1149c7ca5f6aa2cf2` | 51,840,048 B |
| `dist/version.dll`（当前构建 71dae36d） | `71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e` | 779,879 B |
| 历史回滚主体（2028e03b） | `2028e03bc288cd89ff1df23c3dd4f94b9413594636ca2781a183d916c7d9d4a7` | 50,025,206 B |
| 历史 R226 安装主体（installed-before归档） | `404abb8883100bab17c6bfb2200919ffb89c1fba26f4643c1a379cb875c57c92` | 50,220,030 B |

- 目标 EXE：Victoria 3 1.15.0，`SHA256 473186e8d8deba993651e0ae4cf02a003b02f5accb99b096a6a0a8bd9506cbaa`。
- 安装布局：`binaries\version.dll` + `binaries\freight\freight.dll`；配置 `freight.ini`、日志
  `freight.log`/`loader.log`、缓存与资源都在 `binaries\freight\`，不依赖 playset、外部 Mod、
  Python 或手工资源。
- 历史安装：2026-11-07 安装 R236 构建 `26b70987…`／`71dae36d…`（deploy.ps1 exit0）；更早 R226/R228/R235
  记录由顶部当前记录取代；旧已验收构建 `2028e03b…`／`c8020412…` 保留在回滚副本。安装时必须无游戏进程，
  以 `.tmp` + rename 原子替换。
- 回滚副本：`E:\Code\v3-analysis\checkpoints\2026-11-06-final-delivery\dist`（已验收的
  `2028e03b…`／`c8020412…`），源码对应交付提交 `3ec16a0` 一带；候选源码为本地 `main` HEAD
  （R227 修改前 HEAD `b8f098a`；R227 提交后以 `git log -1` 为准）。

## 2. 构建、测试、部署

```powershell
powershell -ExecutionPolicy Bypass -File native\build.ps1          # dist\version.dll + dist\freight\freight.dll
powershell -ExecutionPolicy Bypass -File native\build_tests.ps1    # 完整原生入口（游戏需关闭）
python -B tools\verify_embedded.py
python -B tools\test_native_gui.py
python -B tools\test_flow_gpu.py
powershell -ExecutionPolicy Bypass -File deploy.ps1                # 游戏关闭时安装
powershell -ExecutionPolicy Bypass -File deploy.ps1 -Uninstall
```

官方 PowerShell 入口已实际执行（2026-11-07 R236 最终构建）：`native\build_tests.ps1` exit0（71 PASS、
零FAIL，含排序值/64+4 ABI）；`tools\selftest.ps1` PASS（loader＋ImGui，560,386／首跑560,419 亮像素，
阈值 5,000）；`deploy.ps1` exit0（安装＝`dist`，`26b70987…`／`71dae36d…`）；`build.ps1` exit0。
日志见 `checkpoint2026-11-07-r236-final-live/official-*.log`；R225 旧日志保留在
`checkpoint2026-11-06-main-build/ps-*.log`。

## 3. 原版界面操作

- **市场物流**：打开任意市场（如“不列颠市场”）→“物流”页签：显示“原版贸易＋国内模型分配”、
  贸易量、已铺铁路（MU）、逐商品市场价格/贸易量/占比，以及地图物流带。
- **筛选**：搜索框、商品分类、“显示/隐藏”、颜色（热度/单商品/原生绿）。
- **地区本地价格**：选择州→“本地价格”页签：物流列、商品/方向/方式筛选。
- **物流列排序**：点击“物流”列表头按数量/占比排序，再次点击反向；切换州/市场后保持原方向自动重排。
- **原版功能保留**：价格、排序、提示、右键菜单、商品详情与“用途”页均按原版工作。

## 4. 模型口径（一期，观察性）

- 国际部分使用原版已选贸易路线，数量为 1e5 整数；国内部分使用最终模型的物理分配，二者独立保存、
  展示相加，不冒充吨数或运输船容量。
- 缺货与断路保留未满足需求；线宽只由最终已分配数量决定；过滤/悬停/颜色/显示隐藏只改视图。
- Freight 需求账分别记录目标/满足/未满足（页面底部 `Goods quantity` 与 `Domestic Freight demand`
  两行），满足量来自最终货流；一期不另建供给池、不注册游戏商品。
- 原版数据中确实缺少当前港口接续的航线保留数量并显式标记 partial/unavailable，不以测试值或
  编造岸线填充。

## 5. 验收与性能结果

- A01–A27 当前构建矩阵见 `docs/transport/ACCEPTANCE.md` 顶部“当前构建 `27cefa0a…`／`71dae36d…` 的
  A01–A27 映射（R278）”：**PASS 18／PARTIAL 9**，未验证项逐条写明，不因冻结升级为 PASS。
- 冻结核验（R276）：`dist` 与游戏安装目录两份 DLL 哈希＋长度逐项相同（引导 `71dae36d…`／779,879 B、
  主体 `27cefa0a…`／52,197,771 B）；`git diff d0f1b6f..HEAD -- native/src native/test native/assets
  data tools` 为空，即当前树就是产出该主体的树。注意主体长度以 **52,197,771 B** 为准（旧文档 52,212,575 B 记错）。
- 性能（同存档/相机/速度 III，三对 65 秒，**当前构建** `27cefa0a…`，R264）：六臂 P95 增加
  `0.379 / 0.430 / 0.394 ms`（门槛 2 ms），模拟速度零降幅（门槛 5 %），窗口内帧流连续，见 STATUS R264。
- 缓存/压力（当前构建，R264）：20 轮 `uploads=0 reused=206 pool=206`、池 206/8192、每帧上限 16、
  峰帧 529 µs、暂停期重开零捕获、内存末段持平。旧 `404abb88…`／`2028e03b…` 的数字仅属各自构建。
- 部署矩阵（当前构建，R266）：主体缺失 126／损坏 193／整套卸载无 loader／重装 0，游戏均正常存活且安装＝`dist`。

## 6. 已知限制（不冒充通过）

1. 1 条海路航线（MINDANAO→NORTH_BORNEO）保留 unavailable，38 条 partial 航线的一端没有当前
   港口接续；数量保留、几何标注未知。
2. 设备丢失重建（A24 剩余项）需真实 GPU/显示设备变更，未在本次会话模拟。
3. 省份增删/拆并、其他地图变体、逐像素多相机矩阵与 A01–A27 的其余 PARTIAL 项仍需样本验证。
4. **海端 38 条 partial（G2）**：全部为“该端本州无当前港口接入→取同地区同 geo 的 peer 锚点”的回退类
   （R275 实测 38/38 至少一端无 `portTransfers` 记录）。原版实际选中的岸线仍未识别，因此不补线、只标注；
   任何“37 条已修好”的说法都不成立——它们是**有标注的不完整**，不是已验证的完整。
5. **另一存档载入／返回主菜单无法自动化（G6）**：R277 九场实机证明存档列表控件对 PostMessage 与真实光标
   单击/双击均无反应、键盘通道整体无效（详情见 `BLOCKERS.md`）。该项需人工操作一次才能绑定。
