# 验收矩阵 A01–A27

### 2026-10-24 — R102增量：海路高度修复用户实机确认，A10整体仍PARTIAL

主体51ec975f…已部署，按实时_WaterHeight夹高。用户原截图证明旧构建近景海运消失；实际VS GPU64场景复现旧错并通过修复、两个负对照拒绝，完整入口/资源/GUI/自检PASS。用户随后明确回复“海路高度修复成功”，该近景故障实机通过；未扩展到长路径/接缝等其他几何场景，A10整体保持PARTIAL。下方64ab24f4等旧矩阵为历史构建证据，当前版本与限制作STATUS顶部为准。

### 2026-10-24 — R90：真实原版市场身份已接采集（新实机验收中）

- 静态原版State.GetMarket链与RTTI/独立Capstone复核：普通州经owner country取得market，特殊州经MarketArea取得market，再验证原生CMarket。GUI包装保存同一个原生market ID；不需要猜GUI与快照映射。
- 只读实机：1147/1147州生产解码器通过（0失败，3特殊分支）；中英格兰433/owner1→market0，与不列颠市场相符。旧+0x48分组52而真实市场305；1143州数值不同，35个旧分组跨多个真实市场。旧分配的“市场隔离”实机结论作废，保留历史不扩展。
- 新state_market.h复制并复核对象类型、完整typed ID代数、数据库边界、owner/area分支和复制期间变化；采集失败撤回正式数据。新增state_market_tests接统一入口，覆盖零ID、未知、越界、类型错、回收handle和变更，独立测试PASS；生产解码器只读全州PASS。
- R89现场稳定性：西南诸郡工作量可见，多个30秒刷新均重新获得owner15/frame1，原生40ribbon的uploads累计保持40（>60秒）；这只验证异步/几何复用，不证明旧市场分组正确或完整性能门槛。紧凑数字的padding省略号已改为先安全heap写入再对现有entry赋未填充短值。
- 候选主体93b6ec29e9a0114771c9245293a6b0bbaea286704485e2292ce8858cd64d985d（33,038,179 B），资源/实际GUI归档PASS；完整入口含新增州市场校验全部PASS；经deploy.ps1部署，安装=dist；自主创建的暂停会话55652已正常退出，未推进日期/保存。备份2026-10-24-r90-native-markets，all-state-market-live.txt/production-decoder-live.txt/static-assembly.txt为私有证据。



## 当前构建权威矩阵（build `64ab24f4…`，2026-10-24）

判定口径：**PASS 只覆盖该行写明的范围**；同一 A 项在旧轮次的表现不自动扩展到当前构建。证据一律可定位到文件或日志行。下面各“增量”段落仅作溯源。

| ID | 检查 | 状态 | 当前证据与限制 |
|---|---|---|---|
| A01 | 原始材料与现状审计 | PASS（历史） | 四文档 + 仓库文档 + 源码已完整读过；本轮未重做 |
| A02 | 构建与导出 | PASS | 可复现构建（`-Wl,--no-insert-timestamp`，两次一致）；原生回归全 PASS（含新增 `publish_key_tests`）；`verify_embedded` PASS；GUI 归档 Python/C++ 逐字一致 |
| A03 | 部署 | PASS | 无游戏进程时 `deploy.ps1` 安装，安装＝dist（`64ab24f4…`）；卸载／回滚演练未按当前构建重做（见 A26） |
| A04 | 能力门控 | PASS（离线） | 未知 GUI hash／缓存损坏／缺资源一律拒绝（`test_native_gui.py`、`test_native_gui_runtime.ps1`） |
| A05 | 原版入口 | **PARTIAL** | 物流页签／商品列表／商品详情可用；**正式数值从未发布**（`publish document=1` 始终为 0），因为市场身份未打通 |
| A06 | UI 原行为 | PARTIAL | 原版价格／排序／右键／商品详情路径保留，并有补丁断言；未逐项实机复测 |
| A07 | GUI 生效 | PASS | 归档挂载、`prepared=1 mounted=1`；**实机 `error.log` 中 `ironroad`／`margin_right`／`not handled` 命中 0**；不支持的 `icon margin_right` 已移除并加回归断言 |
| A08 | ABI 与对象 | PASS（历史） | `PROFILE.md` 全链静态＋实机证据；当前构建未重跑 ABI 探测 |
| A09 | 单线与 pass | PASS | D3D11 `selftest`：ImGui/overlay/readback 通过、默认隐藏 `vtx=0` |
| A10 | 几何贴合 | PARTIAL | GPU 八档宽度×九色板、像素着色器、方向全部 PASS（`test_flow_gpu.py`）；相机缩放／长路径／接缝未逐项实机 |
| A11 | 河流与海岸 | PASS（离线） | `spline_tests` UK–Sweden coastal、hub 港口/海岸回退；实机未逐项 |
| A12 | 铁路与断路 | **PARTIAL** | 未建铁路排除、断路不伪造、hub 回退离线 PASS；**真实已建铁路状态仍未接入**，页签保持 `--` |
| A13 | 快照安全 | PASS（读取窗口） | 只读复制＋复制期间变化拒绝；实机 `states=1147 orders=25396 goods=43 markets=52`；**不是原子快照** |
| A14 | 数量与守恒 | **PARTIAL（州侧已结构验证）** | 模型守恒／目标·已分配·未满足分离离线 PASS；**`state+0x48` 已用结构法验证**：1147 州 × 128 个 dword 中只有该偏移能 1:1 区分 52 个市场分组（值域 31..116），即州侧市场键成立。仍缺：面板侧 `panel+0x70` 属 GUI 包装 id 空间，与 31..116 不通用，故正式市场作用域未闭合 |
| A15 | 模型来源标识 | PARTIAL | “模型分配”标注与 `--` 回退路径存在；正式发布未发生故未实机可见 |
| A16 | 需求与宽度 | PARTIAL | 固定八档宽度 GPU PASS；正式线宽要求“只由最终已分配量决定”，当前无发布可验 |
| A17 | 颜色与统计 | PARTIAL | 固定热度尺度、全市场分母离线 PASS；正式 hover 未接入 |
| A18 | Hover/筛选 | PARTIAL | 会话层 hover／关闭恢复／缓存键 PASS；原版 hover 未完成运行时验证 |
| A19 | 关闭/owner | PARTIAL | 多 owner 仲裁离线 PASS；正式 owner 因身份问题从未激活，关闭语义未实机验证 |
| A20 | 三层缓存 | PARTIAL | 路由/视图/原生缓存离线 PASS、8192 上限与 16/帧/2 ms 策略 PASS；“稳定视图 60 秒零上传”需正式发布会话才能测 |
| A21 | 冷批次 | PASS（模型层） | 真实 1117 州冷构建 127–155 ms；非逐帧计时 |
| A22 | 有界池与失败 | PASS | 离线 8192 上限与失败抑制；实机曾观测 `native pool 989/8192`、0 失败 |
| A23 | 内存平台 | PARTIAL | 历史 3 分钟 18 次采样 ±24 MB；**当前构建未重测** |
| A24 | 生命周期 | **PARTIAL** | 读档往返实测：`world changed … next world=2` 安全失效、重采集一致、无旧帧提交；但 `m_nativeWorldRetired` 置 true 后从不复位 → **恢复正式视图需重启**（D29） |
| A25 | 性能 | **NOT_RUN** | 无正式发布，P95 ≤ +2 ms ／模拟速度 −5% 门槛无法测 |
| A26 | 回滚 | PASS（历史） | 卸载后游戏正常进图、重装恢复；当前构建未重做演练 |
| A27 | Freight 中转需求 | PARTIAL | 模型核算／来源贡献／目标·已满足·未满足／非递归离线 PASS；一期需求账已明确不另建供给池/游戏商品；UI 行及完整端到端未实机验证 |

> 结论：A02／A03／A04／A07／A09／A13／A21／A22 有当前构建证据；A05／A14／A24／A25 是四条关键缺口，全部或部分受阻于“市场身份”与“跨 world 原生对象寿命”。

## 历史增量（溯源用，结论以上表为准）


## R84i 增量（2026-10-23 00:05）：GUI 属性错误清零与列宽/表头格式

构建：主体 `64ab24f4338a092b1a3da3c94a9fca17f87cf752204825b90e09d05c464f691d`（32,072,202 B）；安装＝dist；GUI 补丁、C++/Python 逐字一致、`verify_embedded` 全部 PASS。

| 范围 | 结论 | 证据与限制 |
|---|---|---|
| A07 GUI 不支持属性 | PASS（实机） | 移除 `icon margin_right` 后，新会话游戏 `error.log` 中 `ironroad`／`margin_right`／`not handled`／`Error setting properties` 命中 **0**；`test_native_gui.py` 加断言防止回归 |
| 州面板列宽 | PASS（实测尺寸） | 原版行 480 px；补丁后行 52/52/78/92/92/54 + Freight 60 = 480；表头 42/42/42/58 + 100/100/90 + 60 = 534（≤540），“价格差异”列加宽回 90 |
| 表头格式一致性 | PASS（离线 + 实机无报错） | 州“货运”与市场“市场价格／工作量／占比”均改为原版同款 `sort_button`（`align = hcenter|nobaseline`），省略 `onclick` 时引擎接受 |
| 市场页列收紧 | PASS（离线） | 两列 90→70、wrapper 180→140、标签“工作量／占比”（EN `% of freight`） |
| 结余列前移 + 结余列头 | BLOCKED | 原版商品行固定 `@panel_width`=540 px，`last_column` 已被价格块 210 + 右锚定结余 80 占满；需先决定牺牲哪块宽度 |
| 州列表 Freight 列实机外观 | 待用户确认 | 尺寸已按原版总宽重算并部署，仍需肉眼确认行内数值不再被裁 |


## R84e 增量（2026-10-23 23:26）：重复 null 发布修复

构建：主体 `01d2d708063614046f2e09e0e8b8562b2e52f34c322d00c560d20d5d9482b38a`（32,079,882 B）、引导 `71dae36d…`；安装＝dist；完整入口零失败。

| 范围 | 结论 | 证据与限制 |
|---|---|---|
| 重复 null 发布 | PASS（实机） | 修复前最近 400 行有 399 条 `publish document=0`（约 65/s）；修复后同一存档整个会话仅 4 条，且都紧跟 `real economy snapshot accepted`，随后 `UI-thread variables published` 正常出现 |
| 发布去重回归测试 | PASS | `native/test/publish_key_tests.cpp` 接入 `build_tests.ps1`：未发布键不相等、null→null 相等、null 跟在文档后必发、六个作用域字段各自变化必发 |
| GUI 变量未变旧 | PASS（间接） | 初始 waiting／作用域文本仍会写入，真实状态变化仍触发发布；当前无正式 owner 激活，故“数值残留”场景无法构造，待身份打通后复验 |
| A02／A03 构建与部署 | PASS | 完整 build、`verify_embedded`、入口零失败；无游戏进程时 `deploy.ps1` 安装，安装＝dist |


## R84d 增量（2026-10-23 23:10）：实机读档往返

| 范围 | 结论 | 证据与限制 |
|---|---|---|
| A24 世界替换识别与安全失效 | PASS（实机） | 用户手动“回主菜单→重载 autosave”：`world changed: invalidating economy snapshot, next world=2`；随后新 world 重采集 `states=1147 orders=25396 goods=43 markets=52 primary=33`，与读档前一致；全程 `owner=0`、无旧文档/旧帧提交、进程存活无崩溃 |
| A24 读档后恢复正式视图 | FAIL（未实现） | `InvalidateEconomy` 置 `m_nativeWorldRetired=true` 后全仓库无 `store(false)`；9 处原生路径以其为门，范围文本变 `map unavailable after reload; restart required` → 必须重启才能恢复，正是目标禁止的做法 |
| A24 resize／最小化／设备恢复 | NOT_RUN | 本轮未测 |
| 州对象→市场对象指针（复测） | FAIL（结论加固） | 在**完整采集**（`orders=25396 goods=43`）上复跑，12 个指针仍全为容器 vtable `0x624690`，无 `0x045850B0` |
| `+0x374` 候选 | 弃用 | 三次观测两次 `0xFFFFFFFF`（本次面板已开 `marketOpen=1` 仍全空），写入时机不可控 |
| 玩家国家／存档 | 已确认（只读） | 存档 `meta`＋`continue_game.json`：大不列颠(GBR)、1.13.11；`gamestate` 为二进制格式，暂不作证据源 |


## R84c 增量（2026-10-23 23:00）：州→市场对象指针证否

构建：主体 `1c3428e5eacb77546ac755bd3da7217457abe2aa8240264627fd8a99c71a1530`（32,070,154 B）、引导 `71dae36d…`；安装＝dist；完整入口零失败 `native/build/entry-round84c.log`。

| 范围 | 结论 | 证据与限制 |
|---|---|---|
| A02／A03 本轮构建与部署 | PASS（本轮） | 完整 build、`verify_embedded`、完整入口（含 selftest／默认隐藏）零失败；无游戏进程时 `deploy.ps1` 安装，安装＝dist |
| 州对象→市场对象指针 | FAIL（已证否） | anchor 州 `id=0`、`state+0x48=112`；`+0x00..0x400` 内 12 个引擎指针全为容器 vtable `0x624690`，无 GUI 市场对象 `0x045850B0` → 州与市场是整数 id 关系。证据 `native/build/round84c-state-market-probe.txt` |
| 市场与地区列的同一闸门 | 已确认 | `CopyNativeStateRoster` 用 `orders[].market`（源自 `state+0x48`）建 roster，`NativeStateOwner` 以其命中为激活前提 |
| 静态证据可用性 | 受限 | Ghidra 对 getter `0x225AC90` 一带无函数体、脚本执行被禁用；文档中 resolver `0x10571F0` 实为构造 `GetSuppressionEffectDescriptors` 字符串的函数（记录已过期） |
| A05／A07 正式数值、地区 Freight 列 | NOT_RUN | 仍由市场身份阻塞；`NativeMarketOwner`／`NativeStateOwner` 均未激活，`publish document=1` 为 0 |


## R84 增量（阶段1基线固定＋阶段2双DLL实机加载）

构建：引导 `71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e`、主体 `4ac3a0e8873dbe75ce945438a1ce476c8583e29d1f3f8d610245ac19e29f2980`（可复现，两次构建一致，安装＝dist）。

| 范围 | 结论 | 证据与限制 |
|---|---|---|
| A02 构建与导出 | PASS（本轮） | 两次完整构建字节一致；完整入口零失败 `native/build/entry-round84.log`；`verify_embedded` SHA256=4ac3a0e8… |
| A03 部署 | PASS（本轮） | 无游戏进程时 `deploy.ps1` 安装，安装＝dist，无遗留 `.tmp`／备份冲突 |
| A04 能力门控 | PASS（离线） | 未知 GUI hash／缓存损坏／缺资源拒绝；本轮未在新构建上实机重测 |
| 双DLL加载路径与系统导出转发 | PASS（R83 构建实机） | 进程模块 `binaries\VERSION.dll`（代理）、`binaries\freight\freight.dll`（主体）、`system32\version.dll`（转发目标）；`loader.log`＝`loaded freight.dll (error=0)` |
| 配置／资源／日志目录落点 | PASS（R83 构建实机） | `runtime data=…\binaries\freight\data`；GUI 归档取自 `freight`；该会话无 LocalAppData 写入 |
| 嵌入资源早期挂载 | PASS（R83 构建实机） | `mounted BEFORE VFS configuration freezes`；`ready BEFORE gfx/models .asset enumeration` |
| 默认关闭 ImGui | PASS（实机＋harness） | 实机 `drawdata lists=0 vtx=0`；`test_debug_default` vtx=0 |
| 经济只读采集（进存档） | PARTIAL（只读读数） | 1147 州／25317 订单／43 商品／52 市场（primary 33），两次采样一致；快照以 `(diagnostic)` 接收，不激活正式 owner，故不是正式 UI 数据 |
| A05／A07 原版物流入口正式数值 | NOT_RUN | 该次会话市场面板未打开（`panel=4294967295 tab=default`），`publish document=0` 全程 |
| 市场/州面板原生解码链（活进程只读探针） | PASS | `phase=4`、`idler/gfx/handler` vtable 与 `handler+0xd8` 全 MATCH、`panel count=56`；slot 9=`market_panel`（`tab="ironroad_logistics"`）、slot 14=`states_panel`（`state=433`、`tab="local_goods"`）；市场置顶后 `marketOpen=1`、`visibleGoods=40`。证据 `native/build/round84-panel-probe.txt` |
| 市场身份与快照一致性 | BLOCKED（已排除一条假设） | GUI 市场注册表是**惰性 GUI 本地空间**：未开面板 `count=2`（`0`,哨兵）、开面板后 `14`（`0 1 2 3 4 7 8 9 10 13 14 19 22`）、另一会话 `45`；快照 `state+0x48` 为 `31..116`，基本不相交。`+0x374` 候选被第二样本否掉（本次 `0xFFFFFFFF`，上一会话 `33`） |
| GUI 市场注册表可读性 | PASS（只读探针） | `module+0x059684A0` → count/capacity/lock 与条目 vtable 全部校验通过；`native/build/round84-panel-probe.txt` |
| 地区 Freight 列 | NOT_RUN | `NativeStateOwner` 未接入 overlay 运行时；探针已证明州面板可读 |
| 新部署构建实机启动 | NOT_RUN | 22:19 部署，尚无启动记录 |
| A01／A06／A08–A27 其余项 | 不变 | 见下方历史矩阵，本轮未重测 |


## R74 增量（候选未部署）

| 范围 | 结论 | 证据与限制 |
|---|---|---|
| 原版筛选语义／缓存定位 | PASS（静态＋未就绪样本） | 分类排除字节、Unicode／关键词路径、原版返回缓存地址；实机仅dirty1／0rows，回放正确拒绝 |
| 搜索／分类→模型视图 | PASS（fixture／模型） | 中文query沿用native IDs、空与缺失分开、过期／busy／错误身份拒绝；分配／分母／heat／Freight不变，排序复用 |
| A18原版实际搜索／分类交互 | NOT_RUN | 没有正向native缓存／新DLL运行／实际点击结果，500ms同步仍待改善 |
| A02–A03候选构建／资源 | PASS | 13项回归、完整build、verify_embedded；1a97a6823e2fc72fa7f1f2178807803a4001fc89faee3ccd8df3fdc8b0223cfd，非实机验收 |

## R73 增量（候选未部署，运行中仍R69）

| 范围 | 结论 | 证据与限制 |
|---|---|---|
| 原版面板所有权链／关闭态读取 | PASS（观察＋回放） | PID52316整条类型／bar／panel／history0／closed1／default／market无效；实际解析器回放通过；没有打开态图像 |
| 面板状态→市场／owner／取消 | PASS（离线） | 新market_panel_tests＋会话桥回归，13项原生测试整体PASS；当前面板market代替primary，关闭不影响其他owner |
| A18–A19正式打开／切换／关闭 | NOT_RUN（新DLL交互） | 源码候选已接线，未部署；500ms同步不证明逐帧立即取消；正式hover／筛选仍缺 |
| A02–A03构建／打包 | PASS（候选） | e0a2e7863db4f016091888b4e6d885678fae3149db8a12e6e4f22b58c7a3d1a6；完整build／13项回归／verify_embedded；非新DLL实机通过 |
| A24完整生命周期、A27Freight机制 | 未完成 | 仅读取UI状态不验证原生对象跨world回收，也未完成Freight独立供给 |

## R70 运行增量（运行版本 R69，候选修复版本 R70 分开）

| 项目／层级 | 结论 | 范围与证据 |
|---|---|---|
| A09 正式owner native提交 | PASS（提交层） | PID52316运行R69、调试UI启动vtx0，真实37 ribbons持续SUBMIT；不等于实机可见图像 |
| A20 热态零新增上传子项 | PASS（本场景日志） | 热态35采样／170.453秒uploads恒37，submitted增加；runtime-summary-round70.json；hover／重开／排序场景未覆盖 |
| 地图／新Freight行实机可见 | NOT_RUN | Hidden游戏窗口未被工具枚举，已请求用户显示；没有图像证据 |
| 重复文档发布回归 | PASS（R70离线） | 新测试先FAIL，修复后11项回归PASS；同一generation只发布一次，frame保留；候选未部署 |
| A24完整生命周期 | NOT_RUN | R69检测world替换后仍暂时停用native对象访问并要求重启，未完成安全跨world恢复 |

## R67 增量验收（完整矩阵历史结论不自动扩展到新 DLL）

候选／已部署 DLL：`52009510d0ff6c64af6e03b77ab109c7d9c6d5bafd56ce101116a2b6fb1c7bd9`。

| ID／层级 | 结论 | 本轮证据／剩余工作 |
|---|---|---|
| A27 需求核算与反例（模型层） | PASS | `freight_tests`：完整目标、缺货／断路、独立系数、商品贡献、市场／州隔离、零／缺失、非递归与过滤不变；`freight_engine_tests`：系数／分类缓存失效、原商品几何身份不变；`logistics_ui_tests`：复制文档、正式变量拒绝合成／无效／缺失 |
| A27 Freight 中转机制整体 | NOT_RUN | 核算层不代表独立供给机制；是否注册游戏商品／供给来源尚未确定，无完整端到端证据 |
| A02／A03 本轮构建与打包 | PASS | build、verify_embedded、11 项原生回归；仅 DLL 经 deploy.ps1 部署 |
| A07 新 UI 行静态／归档层 | PASS | GUI 原生补丁一致、资源／CRC／新变量绑定校验通过 |
| A07 新 UI 行实机可见 | NOT_RUN | Windows 控制工具启动失败，无游戏进程；不能据归档／部署推断可见 |

以下为 R51 历史矩阵，版本与范围见原记录。

版本：EXE `7f53…f811f`（1.13.11）；当前候选/已部署 DLL `5e74477a73f50e0924f72b449ab2986f8b619d698d623a91e23544e9984691fc`（R49 构建；R51 诊断构建 `a25f78ae…` 仅增加 `probe_market` 调试日志，默认关闭）。结论：PASS / PARTIAL / NOT_RUN / BLOCKED / NOT_APPLICABLE。未实测项不得写 PASS。

| ID | 检查 | 结论 | 证据 / 说明 |
|---|---|---|---|
| A01 | 原始材料与现状审计 | PASS | 完整读取；`WORKLOG.md` R0 |
| A02 | 构建与导出 | PASS | `native/build.ps1`；`native/build_tests.ps1` 11 项 PASS；`verify_embedded.py` PASS |
| A03 | 单 DLL 部署 | PASS | `deploy.ps1` 安装/卸载均验证；R47 回滚后重新部署成功；无 playset |
| A04 | 能力门控 | PASS | 未知 GUI hash/缓存损坏/缺资源测试；`test_native_gui.py` |
| A05 | 原版入口 | PASS | 市场第五“物流”页签 + 商品列表 + 商品详情；正式页签显示真实数据（R44/R49） |
| A06 | UI 原行为 | PARTIAL | 原版价格/买卖/详情保留；未逐项复测全部原版交互 |
| A07 | GUI 生效 | PASS | GUI 归档挂载；`test_native_gui_runtime.ps1` 验证 Python/native 补丁一致 |
| A08 | ABI 与对象 | PASS（历史） | `PROFILE.md` 全链证据；当前 EXE 未重跑 |
| A09 | 单线与 pass | PASS（历史） | 短线、校准对象移除后仍绘制、单次提交 |
| A10 | 几何贴合 | PARTIAL | 真实 989 条 ribbon 上屏（R41）；GPU 宽度/方向测试 PASS；相机/长路径/接缝未逐项 |
| A11 | 河流与海岸 | PASS（离线） | `spline_tests` UK–Sweden coastal；hub 港口/海岸回退；实机未逐项 |
| A12 | 铁路与断路 | PARTIAL | 未建铁路排除、断路不伪造、hub 回退（离线）；真实 `builtRailEdges` 未接入，页签显示 `--` |
| A13 | 快照安全 | PASS | 默认采集（R44）：1117 州/28554 订单/49 商品；reader 只读；世界失效（R49） |
| A14 | 数量与守恒 | PASS（模型+市场作用域） | 真实快照按 `state+0x48` 市场分组（59 市场）；primary=最大市场（118/77 州）或 `market_override`；目标/分配/未满足分离；`freight_tests` 守恒 |
| A15 | 模型来源标识 | PASS | 页签显示 `model allocation (market N)`（R53）；无数据时 `--`；市场作用域已标注 |
| A16 | 需求与宽度 | PARTIAL | 八档宽度 GPU PASS；真实 ribbon 可见但宽度档低（全局摊薄）；正式过滤/悬停未接 |
| A17 | 颜色与统计 | PARTIAL | 固定热度尺度、分母、需求统计（离线）；真实彩色 ribbon 可见；正式 hover 未接 |
| A18 | Hover/筛选 | PARTIAL | 会话层 hover/关闭恢复、缓存键测试；原版 hover 未接 |
| A19 | 关闭/owner | PARTIAL | 多 owner 仲裁离线；正式 owner 常驻（无面板可见性检测） |
| A20 | 三层缓存 | PARTIAL | 路由/视图/原生缓存离线测试；运行中命中计数可见；零上传未逐帧验证 |
| A21 | 冷批次 | PASS | 真实 1117 州冷构建：wall 127–155 ms、989 ribbons（R43/R44）；非逐帧计时 |
| A22 | 有界池与失败 | PASS | 离线 8192 上限/失败抑制；实机 `native pool 989/8192`、0 失败 |
| A23 | 内存平台 | PASS | 3 分钟 18 次采样 7278–7302 MB（±24 MB），后段下降（R48） |
| A24 | 生命周期 | PARTIAL | 世界失效注入 PASS（R49）；真实读档/主菜单往返未完成 |
| A25 | 性能 | PASS（worker） | 分配 127–155 ms、view ~7 ms（8 线程）；UI 可交互 |
| A26 | 回滚 | PASS | R47：卸载 DLL 后游戏正常进图；重装后功能恢复 |

## 说明

- “PASS（离线）”只证明模型/策略测试通过，不证明游戏内行为。
- 当前已知缺口：market 作用域（全部州标为 market=1）、真实 `builtRailEdges` 未接入、原版 hover/筛选未接、正式 owner 无面板可见性检测、真实读档往返未演练。
- 真实数据证据：`native/build/r44-final.png`（页签数值）、R41 日志（989 ribbons）、R47/R48/R49 日志。
