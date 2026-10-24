# 工程知识（事实 / 假设 / 未知）

## 事实（可复查）

### 构建与运行环境
- 1.13.11 EXE SHA-256 `7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f`（2026-10-23 复核）。
- 工具链 `E:\Program\C Compiler\MSYS2\ucrt64\bin`；Python 3.12.4。
- 仓库无 Git；回滚点位于 `E:\Code\v3-analysis\checkpoints\`。
- 生产 DLL 当前候选 `45fce3c84329ca73fe651adf3988dd91991a5f823a3e396800f133965c51c402`（25,379,526 B，R1 构建，未部署）。
- 已部署 DLL `a99f8d9ecb58c76245d19c8b72bffe5bb333a7bff79dd390459fed21ec3f9c74`（22,563,873 B）。

### 原生 CArrow（来自 `PROFILE.md` + 历史实机）
- 构造 0xC0 对象用 supply 工厂；base 的 slot 15 为 no-op，真实逐对象常量上传在 supply/military vtable。
- `SetPath` 保留 path 指针并每帧读取；bucket append 必须使用 pass 本次 context；提交只能在 pass hook。
- 宽度在构造时绑定类型；离散宽度变体以类型名注册，不能运行时改全局宽度。
- 参考：`native/PROFILE.md`、`native/src/game.cpp`、`offsets.h`。

### 货运模型（当前实现）
- 每市场/商品独立 min-cost max-flow；最终流分解 shipment；残量反向边可重分配。
- 单位 milli-orders；本地先满足；未建铁路不路由；断路保留未满足；模型不反馈经济。
- 线宽固定绝对档；热度 R1 改为完整市场快照固定尺度。
- 参考：`native/src/freight.cpp`、`freight_engine.cpp`、`FREIGHT_MODEL.md`。

### 缓存/线程
- `LatestWorker`：单槽最新请求 + settle 门 + generation 取消 + 异常诊断；计算不在锁内。
- `RibbonCache`：内容哈希/SameRibbon 精确比较、全局 8192、同 style 才可重用、失败保留不逐帧重试、16/帧 + 2 ms。
- `FreightEngine`（R1）：allocation 与 view 双层；world/revision 拒绝；内容指纹；epoch。

## GUI 变量系统静态解剖（2026-10-23，R10）

- `GetVariableSystem` 是注册的 Jomini 函数：注册包装 → 通用注册助手 → 回调（忽略参数）→ 返回全局单例。
- 单例对象是非多态结构（无 vtable），构造时初始化 0x28 字节头部 + 分配器；本质是 **string→string 哈希表**，条目 0x48 字节：hash、占用标志、键 `MsvcString`、值 `MsvcString`。
- 已定位重哈希、insert-or-find、字符串比较 helper；其中 insert-or-find **只在键不存在时插入，不更新已有值**。
- 未验证：条目哈希算法、更新已有值的 setter、GUI `.Exists/.Toggle/.Get/.Set` 与该对象的映射关系、线程亲和性。
- R11 实机：签名门控的 getter 探针返回 `base+0x59FD800`；头部 `buckets=1 count=0 level=0 load=0.90 entries=0`，与静态解剖一致；无崩溃。尚未创建任何 GUI 变量，条目内容仍未知。
- R12 实机：同模板全局 map（5639 条）验证条目布局；哈希为 **MurmurHash3 x86_32 seed=0**（40/40 真实 key/hash 匹配）；引擎 `std::string::assign`（0x61F280）+ insert-or-find（0x79C0D0）打通任意长度键值的插入/更新，`ironroad_probe_long_key` 读回 `long-value-2`。
- 原版 GUI 变量 API 只有 `Exists`/`Toggle`/`Set('key','true|false')`，**没有 `Get` 文本读取**：变量只能作标志，不能直接显示数字；数值文本仍需注册 GUI 数据函数。
- R15 更正：**`GetVariableSystem.Get('key')` 实际存在**（原版未使用，但接口可用）。实机标题临时文本显示 `VAR_EXISTS=1 VAR_GET=long-value…`，证明写入的字符串可被 GUI 读取。因此正式文本可以通过“C++ 写变量 + GUI `.Get`”实现，无需数据函数。
- R15 线程边界：在地图加载期由 Present 线程批量写 `ir_*` 变量时，出现 `entry not found after insert` 并导致进程退出；变量表在 GUI/地图初始化时会被占用/重建。写入必须放在 UI 线程安全窗口，且默认关闭（`native_variables=0`）。
- R16 隔离：`.Get` 读取缺失 key 安全（不崩溃）；单个后载写入不崩溃但失败（`entry not found after insert`）；批量加载期写入崩溃。变量表在加载后由 UI 线程占用（曾观察到 36 条/buckets=64），Present 线程不是合法写者。
- R17：UI 线程写窗口可用（窗口子类 `WM_APP+0x51` → `OnUiThreadPublish`），后载写入不再崩溃；但 insert 返回的 entry 只有 **hash 正确**，key 字节为垃圾、value 为空，`.Get` 返回空。说明后载变量表的 **pair/所有权布局与主菜单不一致**，手工构造 StringPair 的写入不可靠。
- R18：根因是 **SSO 短字符串**（<=15）在后载 insert/move 路径损坏（entry 存指针但 cap=15）。改用 **key/值均 >=16 字符**（值用空格补齐）强制堆路径后，7 个变量全部正确写入并可被探针与 `.Get` 读回。正式变量命名与取值必须遵守 >=16 字符规则。
- R19：页签内容必须插入 `market_panel_details_content` **块内部**（首个子元素）；插在块外的同级元素不会进入页签内容容器，即使 `IsTabSelected` 为真也不显示。
- R20：在 `window_header_name` 的 `raw_text` 后注入 `onclick=SelectTab('ironroad_logistics')` 的标题点击实验失败——实机点击标题选中的是“成员”页签，不是物流。该注入未作用于预期元素，已回退；不要依赖标题点击切页。
- R21：类型实例上的 `visible` 不生效（内容会一直显示）；必须用 **container 包裹**并设 `visible=IsTabSelected('ironroad_logistics')`。市场面板页签提示快捷键 **Tab 5 / SHIFT+5**，`Shift+5` 可稳定切到第五页签（物流）；这是当前自动化验证页签内容的可靠方式。
- R22：商品专用的 FILTERS `section_header_button`、`search_bar` 和过滤器 `flowcontainer` 不带页签条件；需在补丁中为它们加 `Not(IsTabSelected('ironroad_logistics'))`，否则物流页签会显示商品过滤器/搜索栏。
- R27/R28：`GetStateProduction` 注册回调 RVA `0x019791F0`，签名 `void(ctx, result, args)`；州面板或商品面板“本地价格”页签会按州/商品逐次调用。`ctx+0x18` 低 32 位为州 id（高 32 位 0xffffffff）；args 向量 count=7、元素步长 0x20、begin=data+0x20；result 为带 vtable 的包装值容器，定点数值尚未解码。
- R29：生产读取器 `0x0195BA00` 为 `void*(ctx,out)`；商品“本地价格”实际走 **indexed** 容器读取 `0x0103F440(container,out,arg)`（keyed `0x0103F3C0` 未触发），同一 out、多个 arg（步长 0x70），out 写入形如条目向量的头部（0/end/堆/代码指针）。钩子必须原样返回原函数 rax，不能用 bool 截断。
- R30（静态确认）：indexed 值 = `*(uint64_t*)(*(container+8) + goodId*8)`，goodId=`*(int*)(arg+0x10)`，有效位图在 `container+0x20`；keyed 键=`*(int*)(*(arg+0xC0)+0x10)`，表在 `container+0x30`、条目 0x20；读取器 `state=registry(ctx+0x18)`，容器=`state+0xC8`（生产）或 `state+0x78`（消费），值写入 `out[0]`。
- R31（实机确认）：生产/消费值为 **CFixedPoint，缩放 1e5**（除以 100,000 得游戏单位）。100 州钢累计 3.424e9/5.942e9 ≈ 34,244.8/59,422.5，与 UI 市场 36.6K/61.2K 同量级；单位验证通过。
- R32：州对象 id 在 `state+0x8`；registry 盲枚举（id 校验 + vtable 过滤）会混入非州对象，总量异常（4.5e18），不可用；**reader 路径**（本地价格页签逐州回调）给出的州容器有效，且该页签列出的州即当前市场州集合，是快照的正确来源。
- R33：采集当前市场 100 州全商品并落盘 `%LOCALAPPDATA%\IronroadV3\economy_dump.txt`（`P/C state good value`，1e5）；高位商品索引（>=128 附近）会读到指针垃圾，需过滤 `good<128 && value<1e12`。过滤后总量 prod=24.0e9、cons=36.8e9（≈240,075/368,438 单位）。
- R35：`SetGuiVariable` 的 insert-or-find **不更新已存在 key 的 value**；更新必须在 insert 后对 `entry+0x28` 的 MsvcString 调用引擎 `assign`。实测可连续更新、无崩溃；正式页签因此能显示动态真实数值。
- R36：`LogisticsDocument::allFreight` = Σ shipment.freight（milli-orders × cost），不是订单数；UI 显示应除以 `kQuantityScale` 并标注为 order-cost。页签数值 = 23,108.93（原 23,108,927.60）。
- R37：运行时州 id 与 `data/positions.txt` 的 region id **一致**（201=Timbuktu、205=Windward Coast、432=Ankara、436=Azerbaijan、132=Moscow、350=W.Cuba）；positions 缺失的 id（790/940）无 hub。商品面板“本地价格”列的是**全球州**，不是单一市场；快照把所有州标为 market=1 会跨市场混样。州对象前 0x100 未发现 `STATE_` key。
- R38：本地价格页签实际只访问**全球排序首屏**（6/100 个样本州，无英国本土 16–23），确认其非市场作用域。州对象 0x00–0x100：0x80/0xa8/0xd0 为每州数据指针（不等），0xb0 为每州计数（1/2/3），0x88/0xd8 恒为 0x35_00000035，0x90/0xb8/0xc8/0xe0 为静态类型指针；未见 market 字段，需 0x100 之后或静态分析。
- R39：全州枚举可行：`lookup(id)` + vtable 相等 + `state+8==id` 校验后按容器布局读取，1117 个真实州、总量 889,366/942,319 单位；**不要**依赖“本地价格”页签（首屏子集）。枚举一次约 O(2048×128×2) 读取，UI 线程可接受。
- R40：market 不是 state 的直接字段：英国本土 16/17/18 vs 外国 132/432 的 0x200 字节、以及 0x80/0xa8/0xd0 目标对象 0x80 字节中，无“英国相同、外国不同”的 qword/dword。market 可能由所有者国家/省份间接决定，或 16/17/18 的 id 映射需先验证。
- R41：`ApplyPendingFreight` 曾要求 ImGui 可见才提交 ribbon；原生 owner 驱动时必须把 `kOwnerNativePanel` 视为活跃，否则真实帧永远不会上屏。实测 989 ribbons、compute 768 ms、pool 989/8192、可见。
- R42：分配耗时集中在 **min-cost flow 增广**（7016 shipments → 729 ms）；路径分解在 importer 查表后仅 11 ms；view/ribbon 仅 ~10 ms。优化需减少增广次数或并行分组。
- R43：按 (market, good) 组并行求解安全可行（balances 每组独立索引、shipments 分线程收集后合并）；**线程内异常必须用 exception_ptr 回传主线程**，否则 `CheckCancel` 抛出的 "cancelled" 会 `std::terminate`。8 线程墙钟 741→127 ms。
- R44：默认采集可行：UI 定时器内枚举 id 0..2047，按 vtable 频率 + 容器数据探测选 state 类，注册表未就绪时每 5 秒重试；成功后 `PublishRealEconomy` 激活原生 owner。**真实数据不再依赖 probe_economy**。
- R45：世界重载可用 anchor 州对象指针 + vtable + id 复核检测（每 ~2 s）；不匹配时 `ResetWorld` 并重新采集。20 s 内无误报。
- R46：周期刷新（每 30 s 重新采集，revision+1）可行；worker 后台重算约 145 ms，视图 gen 递增、ribbon 数稳定。
- R49：世界失效路径可用 `InvalidateEconomy` 统一（清采集/anchor/vtable、ResetWorld、清文档）；调试注入 `test_world_change` 可在采集后触发一次，验证重新采集与 revision/gen 递增。
- R53：**`state+0x48` 是 market id**：1117 个州只有 59 个不同值且成簇（118×77、164×75…）；威尔士(17)/苏格兰高地(19)=79，莫斯科(132)=71。采集按此分组即可得到真实市场；primary 目前取州数最多的市场（启发式），可用 ini `market_override=<id>` 锁定。
- R56：钩 `0x3AB6D00`（脚本/GUI 注册助手）未捕获 market 注册（可能注册早于安装或非该路径）；不能由此取得市场面板 market。
- R57：已知州的市场 id：17/19=79（UK）、132=71、350=83、201=64、205=33、432/436=164；**最大市场 118 不是 UK**。游戏变量表只存脚本变量（仅我们的 7 个键），无市场上下文。
- R64：脚本/数据函数注册表在固定全局 `0x1459FD648`：`+0x00` 数据指针、`+0x08` 条目数（实测 141）、条目 0x78 字节；名字字段偏移尚未确认（尝试 +0x40 失败）。
- 私有证据：`E:\Code\v3-analysis\var-system-r10-notes.txt`。探针/写入默认关闭（`probe_variables=0`、`probe_write=0`）。

## P4 经济读取链的静态证据（2026-10-23，R2）- 本机 `victoria3.exe` 中存在脚本函数名 `GetStateProduction`、`GetStateConsumption`、`GetStateImports`、`GetStateExports`；注册包装把回调交给通用脚本函数注册 API。
- 回调在脚本执行上下文中从 `context+0x18` 取对象 id，经通用对象数据库解析后读取 state 对象上的候选容器（生产/消费候选偏移不同，语义尚未用运行值证实），再通过 `context+0x20` 的虚调用参数对象求值。它们**不是**无参/纯数值 getter；直接调用需要构造脚本上下文、参数与结果容器。
- 上一轮已知的参数向量构造 `0x73A1D0` 仍不是读取 guard；本结论与 `NATIVE_GUI_PLAN.md` 一致。
- 私有证据：`E:\Code\v3-analysis\economy-r2-notes.txt`、`economy-r2-1.txt`；只读 Ghidra 副本 `IronroadEconomy-r2`。均不进入交付包。
- 下一步：从容器读取函数的引擎内部调用者反推容器布局、访问线程与稳定 ID；在此之前不得 hook/调用。
- 不可据此声称“已恢复实时经济读取”。`EconomySnapshot` 仍只能来自测试或未来适配器。

## 原版市场面板与输入自动化（2026-10-23，R13/R14）

- `open_market` 绑定在 `game/input_profile/default.profile`：**F4**（scancode 61）。
- **自动进图**：`victoria3.exe -gdpr-compliant -continuelastsave` 可直接加载最近存档进入地图，绕过“载入游戏”对话框（该对话框的“载入”按钮对合成点击长期无响应）。
- **打开市场**：`keybd_event(VK_F4=0x73, scancode 0x3E)` 有效；`PostMessage(WM_KEYDOWN)` 无效。市场面板左侧 sidebar 的贸易按钮也可打开，但面板开关/展开状态会影响可点击坐标。
- **GUI 缩放**：`pdx_settings.json` 中 `GUI.scale = 1.1`；截图坐标与输入命中位置存在偏移，第五页签“物流”（截图约 physical 585,232）尚未命中，直接点会落到地图/过滤器。下轮需按缩放校准或临时把该页签设为默认。
- 市场面板页签为 `商品/贸易/粮食安全/成员/物流`。`GetVariableSystem.Get` 在原版 GUI 中无使用；是否存在仍未知。

## 运行期路径与实机事实（2026-10-23，R8/R9）

- 运行时根目录 `%LOCALAPPDATA%\IronroadV3`；地图数据在 `...\IronroadV3\data`。
- **规范设置文件**：`%LOCALAPPDATA%\IronroadV3\ironroad.ini`（R9 起）。首次运行若根文件不存在，会从 `...\data\ironroad.ini` 或旧 `binaries\ironroad\ironroad.ini` 迁移，且不覆盖已有根文件；读取仍回退 data 副本，写入只写根文件。
- 日志：`%LOCALAPPDATA%\IronroadV3\ironroad.log`；GUI/资源缓存 ZIP 与 `readback.ppm` 在根目录。
- 当前候选 `c6f7…9d7b70` 实机启动成功：profile hash 匹配、GUI 归档 prepared/mounted、嵌入资源 MountData、native hooks、ImGui ready 2560×1440。
- 合成 TEST INPUT（`test_snapshot=1`）会加载 48 orders；调试 UI prototype 产出 58 条 ribbon、71 次上传、605 cache hits，地图可见多宽度彩色 ribbon；原版市场“物流”页签仍显示 `--`。
- 游戏在无地图的主菜单不会进入 arrow pass（日志提示等待地图）；有存档/地图后 `SUBMIT scope=0/1/0` 正常。

## 地图 hub 与连通性（2026-10-23，R6）

- 本机 `data/splines.bin`：3384 junctions、4606 edges（type 0=1425、1=2330、2=146、3=372、4=333）。
- 每个 state region 拥有 `region*100 + 0..4` 多个 hub；`+3` 为港口 hub，type 4 边连接陆地港口与 `0x800000 | region*100` 海上锚点。
- 排除未建铁路（type 1）后，634 个城市 hub（suffix 0）中 **238 个没有任何可用边**；350 个港口 hub 中仅 12 个零度。固定用 suffix 0 会把大量州误判为不可达。
- 活跃图（不含未建铁路）有 1229 个连通分量，giant 1328/3384 nodes；781 个州中 393 个至少有一个 hub 在 giant。未满足需求主要来自“未建铁路 + 网络碎片”，不是重复计费。
- 分配器 R6 起选择**最高可用度**的 region hub；这是显式模型规则（D14），不是原版事实。
- 合成快照（无铁路）649 regions：153 shipments、641 ribbons、cold 17.6 ms、allocated 612/1296；全部铁路已建：320 shipments、1472 ribbons、cold 105 ms、allocated 1280/1296。

## 假设（明确标注，未证明）

- `EconomySnapshot::world` 可作为进程内单调世界代数；需要适配器保证读档/回菜单换值（D06）。
- 市场热度用“完整市场最大边吞吐”的相对五档足以表达参考视频的强度语义；需实机对照（B05）。
- 商品权重暂为 1（模型默认），不能当作吨位；若接入真实权重需另记录来源。
- 原版 GUI 变量/动作绑定可通过已确认的市场模板扩展；静态候选未作为运行时调用（B02）。

## 未知 / 待验证

- 经济读取窗口：只读 getter、对象寿命、线程、稳定 ID、市场归属、铁路 built 状态。
- 原版 UI 变量系统写入/读取与 hover/关闭 ABI。
- 读档、回主菜单、resize/设备丢失时原生对象与缓存的安全生命周期。
- 大市场冷/热批次的真实 CPU/GPU 成本与内存平台。
- 参考视频中颜色/线宽的精确语义（仅按原始 prompt + 用户要求推断）。

## 反例与陷阱（不要重复）

- `0x73A1D0` 是参数向量构造，不是经济 guard。
- 旧 `BuildFreightView` 用可见最大值定热度，已被 R1 替换；不要再引入。
- 本机图边方向：`Graph()` 中 edge 0 是 node0→node2，不是 node0→node1；测试断言前先核对 `SplineEdge{id,from,to,type,first,count,length}`。
- 新 DLL 构建成功/资源校验通过不等于游戏内可见；R1 未部署。
