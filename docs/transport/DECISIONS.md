# 决策记录（口径 / UI / ABI / 缓存）

## D29 读档后原生访问继续停用（2026-10-23，R84d）

- 事实：读档往返已被正确识别（`world changed … next world=2`），快照安全失效并在新 world 重新采集，数值与读档前一致，无旧帧提交。但 `InvalidateEconomy` 置 `m_nativeWorldRetired=true` 之后**全仓库没有复位点**，9 处原生路径以其为门，范围文本变为 `map unavailable after reload; restart required`。
- 决策：**保持停用**，本轮不改。清除该标志等于恢复 native 对象访问与箭头提交，而跨 world 的原生注册／注销／对象寿命尚无证据；按“未验证 ABI/生命周期保持 disabled”，宁可在读档后要求重启，也不冒访问已回收内存的风险。
- 后果（须如实记录）：A24 判 PARTIAL——“安全失效”通过，“无需重启即可恢复正式视图”不通过。这是当前唯一自知的、与总目标冲突的妥协点。
- 解除条件：取得原生箭头对象跨 world 的注册／注销／寿命证据（例如可证明注册表在 world 替换时清空且旧对象不可达），或改为每 world 重建注册并验证旧句柄不再被触碰；随后加测试与实机读档往返复验。

## D28 市场身份未验证前不得激活正式视图（2026-10-23，R84b）

- 事实：GUI 市场注册表是**惰性、GUI 本地的空间**（未开面板 `count=2`，开面板 `14`，另一会话 `45`；id 集中在 `0..25` 段），而快照 `state+0x48` 是 `31..166` 段，两者基本不相交。`+0x374` 曾被当作候选（上一会话 gui 0 → `33`），但第二样本给出 `0xFFFFFFFF`（未填），该假设已被否掉。
- 决策：**只记录、不启用**。`market_panel.h` 只把 `+0x374` 作为 `simulationMarket` 候选暴露给日志，`NativeMarketOwner` 的 `markets.count(panel.market)` 条件保持不变，继续 fail closed。
- 理由：把单样本当中性映射会把别的市场数据冒充面板市场，属于“猜测身份”，违反稳定约束；宁可保持 `--` 也不显示错误作用域。
- 结论修正（R84c）：R53 的 `state+0x48 = market id` 只是聚类推断，从未与引擎市场 id 对账；而“沿州对象指针取市场对象”这条路**已被实机证否**（州对象 0x00–0x400 内只有容器 vtable `0x624690`，无 `0x045850B0`）。因此下一步改用可核对的推导链：**面板市场 → 其国别 → 该国全部州 → 这些州的 `state+0x48`**；每一步都要独立证据，仍不得用 primary／最大市场顶替。
- 解除条件：推导链中至少两环有实机交叉验证（例如两侧国别字段一致，且该国州的 `state+0x48` 唯一），随后改采集与 owner 条件，并补“未知/歧义/变化”反例回归。`market_override` 只可用于验证下游管道，不得作为身份证据。

## D27 构建改为可复现（2026-10-23，R84）

- 现象：同一源码两次完整构建 hash 不同但尺寸相同；逐字节比对（`native/build/diff_dll.py`）只有三处共 6–7 字节不同——PE `TimeDateStamp`、由它派生的 `CheckSum`、调试目录中的时间戳副本。
- 决策：`native/build.ps1` 两处链接加 `-Wl,--no-insert-timestamp`，使 `TimeDateStamp=0`。验证：连续两次构建字节一致；`selftest.ps1` 走真实 `dist\version.dll`→子目录主体的加载链仍 PASS，说明 PE 有效、代理→主体加载不受影响。
- 理由：阶段1要求“源码、构建、安装 hash 一致”，非确定性构建让该不变量每轮都无法核对；改动只影响链接器写入的时间戳，不改代码、资源或 ABI。先确认 `tools/`、`native/src` 无 PE 时间戳依赖、无硬编码旧 hash 后才改。
- 边界：字节一致只证明源码未漂移，不构成实机通过；实机证据仍按 `native/build/*log` 单列。

## D26 双DLL与freight目录（2026-10-23，用户明确裁决／R83）

- `version.dll`依赖游戏导入名，保留于EXE同目录并转发原系统导出；主体命名 `freight.dll`，由引导线程使用绝对路径从 `binaries/freight` 加载。用户已明确允许该结构，旧单DLL数量限制及“尚未批准双DLL”被替代。
- 用户将目录由IronroadV3改为freight。主体配置／日志为freight.ini／freight.log，缓存／派生资源同目录；loader.log也在其中。旧LocalAppData仅作读取迁移来源，不覆盖已有新配置、不删除旧文件。
- 引导与主体的加载／缓存／迁移及沙箱安装验证通过；新实际游戏启动／原生早期挂载尚未验证。不能把旧单DLL实机结论扩展为当前双DLL实机通过。

## D25 正式搜索／分类复用原版匹配结果（2026-10-23，R74）

- 原版分类为字节排除数组；搜索含Unicode小写／本地化关键词匹配。正式路径复制Market.AccessMarketGoods已生成的缓存商品ID，不用英文key或自建翻译重做原版搜索。
- 缓存必须唯一market、query与当前搜索相同、dirty／writer未占用、行／商品身份及market／类别一致，且重读无可见变化；不调用getter、等待引擎锁或持engine指针到worker。dirty／缺失为unavailable并撤旧视图；干净空列表为已知无匹配，不能互相替代。
- allowedGoods只筛view，不影响allocation、全市场分母、heat或Freight需求；ID集合排序规范化，原版排序不重建视图。调试override仍选择其配置市场，筛选会员集合来自当前原版列表，不能称为另一个原版面板的结果。
- 当前只有实机dirty缓存拒绝证据和正向fixture；原版正向缓存／新DLL交互／响应延迟仍待验。重读一致不是原子快照或完整生命周期证明。

## D24 正式市场由原版面板状态取得 owner（2026-10-23，R73）

- 采集到快照不等于玩家打开物流；PublishRealEconomy只更新源，不激活正式owner。现有UI线程观察原版panel bar的IsPanelOpen谓词、选中tab和market，按原生ID选择；无可验证上下文／未知市场／关闭／其他tab一律撤正式视图，不用最大市场补缺。最大市场仅保留为诊断primary；显式override仍为视图调试选项。
- 仅读已校验类型／所有权链和MSVC字符串，不主动调用getter／RTTI／GUI效果。采样后逐字段重读拒绝可见变化，不保留engine指针；稳定读不是原子世界／完整生命周期证明。
- 重复UI采样不能重启计算；低优先级正式panel关闭不破坏其他owner的帧／文档。正式变量仅接受当前native owner帧与同world／revision／market；debug文档／旧帧显示waiting／`--`。
- 当前500ms定时器是过渡接线，不作为最终逐帧关闭标准的豁免；候选未部署，实机打开／切换／关闭／关闭延迟仍需验收。R68观测getter探针保留默认关闭，不再作为当前市场唯一定位路线。

## D23 商品元数据来自当前原生上下文（2026-10-23，R72）

- 实机53定义／主线程TLS链支持静态布局，按key对原版定义全量核对；类别0/1/2/3=military/staple/industrial/luxury。local=flags&1，tradeable=flags&2且非local，保留非本地不可交易gold语义。
- UI定时器只观察当前已初始化TLS context的slot；不getter／持引擎锁／初始化TLS／选首个slot，不在render或worker枚举。精确EXE、vtable、ID、字符串、context/catalog前后核验为接收门槛；稳定重读不是原子性／完整生命周期证明。
- 未知ID／类别或缺失／变化必须拒绝整个候选，撤正式owner／文档显示waiting／`--`，不生成goodN、默认分类或零需求。有效元数据合并不修改源订单；weight／Freight系数1仍明示为模型假设。
- 候选已接线但新DLL未部署／未实机运行；R69仍用旧默认元数据，不据回放通过认定正式总量已修正。原始字节及私有工具不进入玩家产物。

## D22 重置世界的退休门槛与保留失效指令（2026-10-23，R69）

- 修正 D06 的实现缺口：world／revision 验证不能依赖当前 snapshot 指针是否非空。ResetWorld 退休最后接受的 world；之后必须提供严格更大的 world，同 world 的任何 revision 均拒绝。设备／视图刷新不得误用 world reset。
- FreightWorker 的 reset 不再作为普通可被替换的 mailbox 输入，而是独立原子代数；调用方先记录失效，再取消旧任务，worker 下次执行请求前清缓存并退休旧 world。Cancel／连续输入不能丢失这项失效。
- Session 入队与 ResetWorld／SetSnapshot 用同一自有锁串行；接受任何新快照立即移除旧可见帧。最终发布核对 snapshot 身份、owner、filter 与 generation；渲染侧 FrameIsCurrent 同样拒绝 pending／取消／旧帧。
- 实时源 world 在失效时递增，周期刷新不跳过旧 anchor 校验。旧探针的累计样本缺少 world／market 有效性，因此只作诊断落盘，不再注入正式会话。
- 原生对象跨 world 的注册／销毁证据仍缺。检测到实际 world 失效后禁止继续访问或提交旧 native 槽，UI 明示地图需重启恢复；不调用猜测的析构／释放，也不把重启保护作为最终生命周期验收的替代品。

## D21 Freight 中转需求核算 revision 1（2026-10-23，R67）

- 按每州／市场／原商品的净输入目标生成模型中转商品 Freight 需求；`target = max(consumption-production,0)/1000 × freightPerOrder`，在分配前计算，不依赖成功 shipment 数量。
- `fulfilled = finalAllocatedImport/1000 × coefficient`，`unmet = max(target-fulfilled,0)`；系数默认 1 是项目假设，独立于 weight／距离／运价，未宣称吨、公里、货币或恢复出的原版公式。缺货／断路仍保留目标。
- `FreightBalance` 保留来源身份及三量；本地商品、标记中转商品和 canonical key Freight 不递归生成需求。无可用结果用 optional 空值，不伪造零；筛选不改变作用域需求。
- 原版第九个变量 `ironroad_demand_v1` 仅从非合成的有效文档格式化，经已有 UI 线程写窗口发布。缓存指纹包含系数及中转标志，线宽仍只认原商品最终分配量。
- 本决策只完成观察性需求核算；独立 Freight 供给／生产／价格、游戏内注册均未确定。用户范围问题仍待答复，不能把模型默认假设当作用户批准或 A27 整体完成。

> 格式：编号、日期、决策、原因、被替代项、影响。新证据推翻旧结论时保留历史并标明“已被替代”。

## D01 数据语义分层（2026-10-23 沿用既有口径）

- `consumptionDemand`、`production`、`targetImport`、`allocatedImport`、`unmetDemand`、`Shipment`、`edgeThroughput`、`shipmentFreight`、`shipmentCost` 必须分开。
- 正式线宽只用最终 `allocatedImport`/shipment quantity（orders，1000 milli-orders = 1 order）；目标需求不画实运线。
- 实时分配标注“模型分配／估算货流”，不得称为引擎逐笔运输记录。
- 影响：`freight.h`、`FREIGHT_MODEL.md`、UI 文案。

## D02 市场热度固定尺度（2026-10-23，R1 新决策）

- 五档热度按**完整市场快照**（全部商品、双向、每条物理边一次）计算 `heatReference`，每条边 `band = floor(min(1, throughput/heatReference)*5)`。
- 原因：MASTER_PLAN 第 6 节禁止按当前可见最大值跳色，要求 hover 时保留市场参考尺度；旧实现用过滤后最大值，违反该要求并有回归测试覆盖。
- 被替代：旧 `BuildFreightView` 内 `maximum` 取过滤后 segments 最大值。
- 限制：不是容量/拥堵模型；`heatReference=0` 表示无货运，UI 显示“无货运”而不是满档；阈值口径版本记录为 `kHeatScaleRevision=1`。
- 未来若改绝对档：必须新增模式并显示尺度变化，不能悄悄改变默认口径。

## D03 线宽固定绝对档（沿用）

- 阈值 10/50/200/500/2000/5000/20000 orders → 宽度 0.5/0.75/1/1.5/2/3/4/6；同档数量变化不改变 native style 身份，不重上传几何。
- 不使用当前视图最大值归一化；商品权重/距离不影响线宽。
- 这些是展示规则，不是恢复出的原版公式。

## D04 正式 UI 与原版入口（用户覆盖）

- 正式交互在原版市场/地区/商品面板；ImGui 仅调试、默认关闭，正式箭头不得依赖 ImGui 可见状态。
- 原版面板保留价格、买卖订单、建筑、图标、tooltip、右键菜单；不复制整套商品 UI。
- 保留引擎管理的窗口可见性；不给根窗口附加永久可见条件；不凭名称猜 GUI 回调。

## D05 三层缓存职责

- 路由缓存（`PreviewWorker`）按图/成本 revision + 起终点；视图聚合缓存（R1 `FreightEngine`）按快照内容指纹 + world + revision + 过滤 + 校准 + graphRevision；原生几何（`RibbonCache`）按精确点序列 + 构造时 style。
- “Dijkstra 已缓存”不等于 GPU 几何已缓存；只有数据/生命周期变化才失效；跨世界不复用原生对象。
- 视图缓存上限 32（LRU）；allocation 缓存上限 4，但 SetSnapshot 在内容/版本变化时清空，实际只保留当前快照的分配。

## D06 world/revision 契约（R1 新决策）

- `EconomySnapshot::world` 必须是**进程内单调递增**的世界代数（读档/回主菜单/地图重建都要换值）；`revision` 是同一世界内的经济快照版本。
- `FreightEngine::SetSnapshot` 拒绝更低 world 或同 world 更低 revision；同 revision 但内容指纹变化视为新快照并失效缓存；`ResetWorld()` 清空两缓存并推进 epoch，但仍保留最后 world/revision 以拒绝迟到旧数据。
- `FreightFrame` 携带 world/revision/graphRevision/epoch，消费侧必须核对后才应用。

## D07 失败与重试

- 取消不发布部分结果；无效快照的分配失败不在每帧重试（只在请求变化时重算）。
- 原生上传失败按几何身份保留失败状态，不逐帧重试；只有输入/设备/显式重试才重新入队（`RibbonCache` 既有决策）。
- 日志批量写入；启动/能力拒绝/严重错误及时保留。

## D08 证据边界

- 静态候选、离线测试、DLL 构建、资源校验、实机可见、全交互验收分别记录；前一层不证明后一层。
- 本轮 R1 的新代码只有离线测试证据，不得写“已实机验证”。

## D20 经济脚本回调观测（2026-10-23，R27 新决策）

- `GetStateProduction` 等不是普通 getter，而是 Jomini 脚本函数；直接构造脚本上下文风险高。
- 采用**观测优先**：MinHook `0x019791F0`（GetStateProduction 回调），仅记录 `ctx/result/args` 与 `ctx` 字段，随后调用原函数；不读取值、不构造上下文、不改参数。
- 触发路径：州面板“本地价格”页签逐商品调用，可作为 ctx 样本来源。
- 仅 `probe_economy=1` 时安装，默认关闭；在 ABI/所有权/线程证明前不调用容器读取器。

## D19 正式页签内容布局与快捷键验证（2026-10-23，R21 新决策）

- 正式物流页签内容必须用 **container 包裹**并设 `visible=IsTabSelected('ironroad_logistics')`；类型实例级 `visible` 不生效。
- 页签内容插入 `market_panel_details_content` **同级**（不放进默认页签独有的块内，否则父级隐藏时内容也消失）。
- 自动化验证用市场页签快捷键 **Tab 5 / SHIFT+5**；鼠标点第五页签仍受 GUI 缩放/命中影响，不作为验证手段。
- 变量文本仍遵守 D18：key/值 >=16 字符、UI 线程写入、无数据/合成输入显示 waiting/`--`。

## D18 GUI 变量写入链（2026-10-23，R12 新决策）

- 变量系统条目布局、哈希与写入 ABI 已实机验证：`MurmurHash3 x86_32(seed=0)`、引擎 `std::string::assign`、insert-or-find + 已存在时 assign 更新。
- **变量只用于布尔/枚举标志**（原版 GUI 只有 `Exists`/`Toggle`/`Set`，没有 `Get` 文本读取）；不得用变量冒充货运数值或实时数据。
- 写入仅经 debug 开关（`probe_write`）用于实验；正式 UI 使用前必须验证线程亲和性、生命周期与回滚，并保持“过滤/悬停/显示隐藏不改变经济数据”。
- 正式数值/文本需要注册独立 GUI 数据函数；在返回类型 ABI 验证前不接。
- R15 更新：**`GetVariableSystem.Get('key')` 已实机验证存在并返回写入的字符串**，因此变量可承载正式 UI 文本（无需数据函数）。但在地图加载期由 Present 线程写入会失败并导致进程退出（`entry not found after insert`），故 `native_variables` 默认关闭；写入必须移到 **UI 线程安全窗口**（如市场面板打开/更新回调）后再启用。变量仍只用于标签/占位，不伪造实时数值。
- R16 更新：隔离实验确认——`.Get` 读缺失 key 不崩溃；单个后载写入不崩溃但失败（`entry not found after insert`）；批量加载期写入崩溃。变量表在加载/首次 UI 使用时由 UI 线程占用。**在找到 UI 线程写窗口前，禁止在 Present 线程写变量**；`native_variables` 默认 0。
- R17 更新：已实现 UI 线程写窗口（窗口子类 `WM_APP+0x51` → `OnUiThreadPublish`），后载写入不再崩溃；但 insert 后的 entry 哈希匹配、key 垃圾、value 空，`.Get` 返回空。**后载变量表的 pair/所有权布局与主菜单不一致**，手工 insert 不可用；在下一次证明前，文本桥保持禁用，`native_variables` 默认 0。
- R18 更新：根因是 **SSO 短字符串在后载 insert/move 路径损坏**。改用长 key（>=16 字符）且值补齐到 >=16 字符（强制堆字符串）后，UI 线程写入 + `.Get` 端到端可用；`native_variables` 默认开启。**规则：所有正式变量 key/值必须 >=16 字符（短值用空格补齐）**，否则后载会损坏。合成输入不写入正式页签（显示 `waiting for economy data`/`--`）。
- R35 更新：insert-or-find 对**已存在 key 不更新 value**；写入后需在 entry+0x28 通过引擎 `assign` 更新 `MsvcString` value（已实测可连续更新、无崩溃）。新写入仍遵守长 key/值规则。

## D17 变量系统桥接的安全顺序（2026-10-23，R10 新决策）

- 在证明布局、hash、线程之前，DLL 不调用变量系统的任何函数，也不注册 GUI 数据函数。
- 第一步只允许 getter-only 运行时探测：门控调用单例 getter，读取现有条目（如 `ironroad_hide_flows`）的 hash/key/value，用于验证 0x48 字节条目布局并反推 hash；不写入。
- 写入实验必须满足：已知 hash、已知更新已有值的 setter、已知线程亲和性、只使用 ≤15 字符的 SSO 值（避免分配/释放旧堆字符串），并且可一键关闭/回滚。
- 变量写入只用于显示/动作状态，不得反馈经济数据或伪造实时读数；失败则关闭该绑定。
- R11 状态：只读探针已实现并实机验证 getter/对象布局（`buckets=1 count=0 level=0 load=0.90`）；尚未取得任何条目，因此条目布局/hash/setter/线程仍未知，**未写入任何变量**。

## D16 设置文件规范路径（2026-10-23，R9 新决策）

- 规范设置文件：`%LOCALAPPDATA%\IronroadV3\ironroad.ini`（runtime 根目录）。
- 首次运行迁移：若根文件不存在，依次从 `...\IronroadV3\data\ironroad.ini`、旧 `binaries\ironroad\ironroad.ini` 复制；已存在的根文件绝不覆盖。
- 读取：先读规范根文件，再回退 `...\data\ironroad.ini`（兼容旧安装）；写入只写规范根文件。
- 地图数据仍在 `...\IronroadV3\data`，日志/`readback.ppm`/缓存 ZIP 在 `...\IronroadV3` 根目录。
- 原因：R8 实机发现用户按文档改根目录 ini 无效（DLL 只读 data 子目录），属于可复现的配置陷阱。

## D15 合成测试输入的配对与宽度覆盖（2026-10-23，R7 新决策）

- 合成快照按商品把州分成成对的生产者/消费者，双方使用同一数量档，使每个商品的供给与需求平衡；奇数余量只作为额外供给。
- 八档数量（4k、25k、80k、300k、800k、3M、8M、25M milli-orders）覆盖固定宽度阈值 10/50/200/500/2000/5000/20000 orders，使调试视图能实际看到 W0–W7 多种原生宽度。
- 这样做的目的只是让调试可视化覆盖到全部宽度档；它不是游戏数据，也不能用于实时验收。真实市场仍无任何假数据。
- 若之后需要覆盖缺失的档位（当前 48-region 实测 0,2,3,4,5,6,7），继续调整数量档即可，不改变线宽映射本身。

## D14 区域 hub 回退（2026-10-23，R6 新决策）

- 每个 state region 拥有 `region*100 + 0..4` 多个 authored hub；固定用 suffix 0 会在“城市 hub 只有未建铁路”时把州误判为不可达。
- 分配器在**当前 policy 允许的边**（排除未建铁路、关闭边、未允许方式）上统计 hub 活跃度，选择活跃度最高的 hub（同度取最低 node id）；没有任何可用 hub 时把该州的供需保留为未分配。
- 这是显式的模型规则，不是恢复出的原版行为；它只改变“从该州的哪个 authored hub 接入网络”，不伪造边、不跨海吸附。
- 证据：真实地图 634 个城市 hub 中 238 个在排除未建铁路后无可用边；修正后 48-region 合成快照分配 8/96 → 32/96；全铁路已建时 1280/1296。
- 仍需：用真实已建铁路快照验证；在报告中标注该启发式。

## D13 需求统计口径（2026-10-23，R5 新决策）

- `targetImport`、`allocatedImport`、`unmetDemand` 是**作用域事实**：市场视图用整个市场，州视图用该州；不随商品搜索/hover/分类过滤变化（与市场 freight 分母一致）。
- 单位统一为 orders（整数 milli-orders 汇总后除以 1000），面板文案标注 “model”。
- `unmetDemand = max(targetImport - allocatedImport, 0)`；未满足需求单独显示，绝不折算成已运输货物或线宽。
- 线宽仍只由最终 allocated quantity 决定；目标需求增加但分配不增加时，线宽不变。
- 无快照/未就绪时显示 `--`，不得用 0 冒充“没有需求”。

## D12 多 owner 视图仲裁（2026-10-23，R4 新决策）

- owner 身份即优先级：原生市场/地区面板 10、临时商品 hover 20、ImGui 调试预览 30。同一会话共享快照与 worker，只有最高优先级的活跃 owner 生成/提交视图。
- hover 覆盖是“临时覆盖”：`Close(hover)` 后自动回到下一个活跃 owner（通常是基础面板），而不是清屏；基础面板关闭也不会清除仍活跃的调试/hover owner。
- 所有 owner 都关闭才取消 pending、清空可见 frame；快照与缓存保留，重开同一视图可复用。
- 同优先级按最近一次 `SetSelection` 决定（`serial`），避免两个同权 owner 争抢时结果不确定。
- owner 只影响“地图显示哪个视图”，不修改任何 owner 的选择状态或经济数据。
- 限制：原生面板 owner 目前只有 API/测试，正式 UI 绑定仍未实现。

## D11 合成测试快照（2026-10-23，R3 新决策）

- 调试面板可生成确定性合成快照 `BuildSyntheticSnapshot`：从本机派生地图挑选有 authored hub（`region*100`）的 region，最多 48 个；3 种商品、市场 1；偶数索引为 4000 milli-orders 产量、奇数为 4000 消费；商品权重 1。
- 仅用于在无实时数据时验证货运渲染/交互路径；文档带 `synthetic` 标记，ImGui 面板显示 TEST INPUT，原版 UI 不得接收。
- 这不是游戏数据：不得用于 A13–A15（实时快照/守恒/来源标识）的验收，也不得在报告中写成实时读数。
- 默认关闭，需显式 ini `test_snapshot=1` 或调试面板勾选；关闭后 `LogisticsSession::ResetWorld` 清空。
- 与既有“手动预览数量”一致：都是明确标注的开发测试输入。

## D10 货运会话与快照门控（2026-10-23，R2 新决策）

- `LogisticsSession` 一个实例只服务一个 owner（当前调试页 owner=2）；多 owner 优先级在更高层仲裁，暂未实现。
- 会话自身拒绝更低 world / 同 world 更低 revision 的快照；worker 引擎再做一次同样的门控。世界重置通过 `FreightWorker::Reset()` 在 worker 线程清空引擎与缓存，调用线程不触碰引擎。
- 会话关闭（面板隐藏/切页）立即取消 pending、停止发布并清空可见 frame；快照保留，重开同一视图依赖引擎缓存复用。`ResetWorld` 则连快照一起清空。
- 文档构建在 UI 线程但只使用不可变副本与已缓存 `FreightView`；不得在文档构建中读引擎或重跑分配。
- 当前没有实时快照来源：会话保持 unavailable，禁止用测试值填充原版 UI；未来调试自检输入必须显式标注且只存在于调试面板。

## D09 记录维护（用户要求 2026-10-23）

- 用户要求每次工作后更新 `AGENTS.md`（Windows 下即 `agent.md`）。每次工作单元必须追加 `AGENTS.md` 第 6 节日志，并同步本目录 `STATUS.md` / `WORKLOG.md`。
