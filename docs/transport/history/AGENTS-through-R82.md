# AGENTS 历史日志（截至 R82）

仅为历史证据；当前约束见根目录 AGENTS.md，当前状态见 ../STATUS.md。原章节及正文保留。

## 6. 进度日志（每次工作单元追加，最新在上）

### 2026-10-23 — R82：地区列实机截图与州／地图ID混用修复

- 用户手动截图确认中英格兰本地价格表的货运列可见且--，原价／差额行仍显示；表头省略号需后续调整。截图私存references/2026-10-23-freight-ui/r81-midlands-local-prices.png，不将布局可见扩展为数据／交互通过。
- 只读OpenProcess／RPM采样PID35764：实际state_panel解码与复制字节回放通过，当前州433、market111、local_goods；没有界面输入、调试器附加／断点或引擎调用。State.GetStateRegion链与构造经Ghidra＋Capstone核对，定义key为STATE_MIDLANDS、引擎定义ID17，而派生地图ID20。
- 发现旧CollectRealEconomy错误地order.region=id，历史真实货流的地理／分配结论需重验。暂停并私存未启用的地区发布草稿；新增state_region.h按state+E78定义key绑定地图，不混用三类ID；拒绝无效类型／ID／字符串／复制变化／未知或重复地图键。当前1147/1147州映射成功，675个独立地区键。
- 全14项回归、完整build、verify_embedded PASS；候选290ee571a172264695f9889430932ca48d131e623364f55f6f660267798a8ebd（32,063,114B），未部署，游戏仍R81／Responding；未跑占用游戏日志的standalone smoke。新映射的DLL运行／正式总量、地区owner与行写入仍待接线验证。
- 用户确认原版地图，并新增“兼容修改地图的Mod”；纳入MASTER_PLAN §7.1／任务表。当前key映射仅必要步骤，实际加载mod地图／省份／样条及版本缓存尚未完成，不能称完整兼容。列宽、null发布日志、icon属性错误继续保留。
- 重读01／02／03及当前UI／采集源码，local-reverse-engineering流程；备份2026-10-23-r82-state-live-bridge（state_panel_tests旧态按本轮明确新增片段逆向重建，已记录），证据native/build/*round82.log及私有state-region-r82-notes.md／assembly.txt／live helper和原始字节。MASTER_PLAN §20.80、STATUS／WORKLOG／TASKS／PROFILE／模型同步，总目标active。

### 2026-10-23 — R81部署／实机启动与手动验收交接

- 无游戏进程时仅经deploy.ps1安装R81，dist／安装hash均234635d6c92231a7102f31044e0dafcabfac45cc776c85276e0a13279f10c1a0；旧R78安装另存r81 checkpoint/installed-version.dll。
- sky.launch_app报告window-opened handler未就绪，但核对发现游戏已实际启动PID35764／Responding，未重复启动。日志确认正确EXE profile、native GUI提前挂载及枚举，真实快照1147州／约25k订单／43商品、52市场；此为加载与采集证据，不能当列可见或交互通过。
- 用户再次按物理Escape停止Computer Use，随后明确“我手动，安排”：界面由用户操作，后续只读日志／代码工作，不再发送界面输入。未强制结束游戏、未替换运行DLL。先请用户停在任意地区本地价格页并回传完整面板，以获取当前州正向上下文和布局证据。
- 新发现：本次日志含连续重复null文档publish（采样计数6104），以及ironroad_logistics.gui icon的margin_right不支持错误；均未修复，列入下一候选。现无states_panel.gui／新state cell错误命中，但这不证明可见布局／绑定正确。原始日志私存E:/Code/v3-analysis/state-ui-r81-runtime.log，启动／实机验收未整体PASS。

### 2026-10-23 — R81：地区owner与同版本快照发布门控

- 新增NativeStateRoster／NativeStateOwner候选接口：只用已接受的自有快照复制州→市场表，拒绝冲突／无效ID；仅原版地区local_goods打开、元数据／快照有效时选择该州和实际市场。州0／市场0保留，独立owner15低于hover／debug，关闭不清其他owner。
- SnapshotIsCurrent核验自有快照指针，覆盖同world／revision替换；CanPublish同时核对owner、文档与帧的州／市场／world／revision、当前快照和FrameIsCurrent。旧州表、新帧配旧owner、切州／切页／reset／合成输入全部拒绝；ReadyId与原版signed ID字符串一致。市场变量额外要求Market文档，防止地区结果误入市场摘要。
- 新会话测试与全14项回归／完整build／verify_embedded PASS；候选234635d6c92231a7102f31044e0dafcabfac45cc776c85276e0a13279f10c1a0（31,984,223B），原生state decoder／owner尚未接运行时，地区单元格仍--。R80 smoke／GUI镜像为上一候选证据，不扩展为本候选实机通过。
- 用户已明确允许恢复游戏界面自动操作，撤销此前Escape停止；已读computer-use技能及guidance／confirmations／API，下一步部署候选并验证原版列／当前州上下文。未强制结束游戏／覆盖运行DLL。备份2026-10-23-r81-state-owner，native/build/session-round81.log／tests-round81.log／build-round81.log／embedded-round81.log。MASTER_PLAN §20.79、STATUS／WORKLOG／TASKS／UI同步，目标active。

### 2026-10-23 — R80：地区本地价格 Freight 列与独立数据格式

- 从本机hash校验后的states_panel.gui派生本地价格表，追加Freight表头／单元格；保留全部原版排序、价格／差额、tooltip、点击／右键及窗口生命周期绑定，仅压缩表内列宽以容纳新列。其他页签／非本地商品区块逐字保留，原版GUI不内嵌DLL。
- 自有单元格按州ID＋Goods.GetKey读取独立变量，ready ID必须匹配当前State.GetIDString；未就绪为--。文档增加明确kind／state身份，FormatStateFreightGoods拒绝市场文档、错误／未知／重复州、市场不符、无版本及合成输入，沿用R77输入／输出工作量、hover不改事实。
- 静态恢复State.GetIDString完整类型／回调／getter链，Ghidra＋Capstone确认其输出有符号int32十进制，修正高位ID键格式并增加反例；州0、零工作量、错作用域／hover等测试通过。14项全回归、最终相关UI回归、完整build、GUI／实际C++归档逐字镜像、verify_embedded、selftest（560208 bright pixels）、默认隐藏PASS。
- 最终候选 b257418f7861d39cc1dd6f26287576ec46d2451cea61dfb6390aad7f32a3233a（31,985,143B），未部署；安装仍R78。地区owner／运行时行发布尚未接，故新列目前只可显示--；新布局原生解析／实机可见性与州上下文正向样本未验。不启动／附加／操控游戏，不恢复Computer Use。
- 重读01／03全文，复核原版表格／参考图、模型与GUI计划；备份2026-10-23-r80-state-freight-column，证据native/build/*round80*.log／r80-gui-preview与私有state-column-r80-static.md／assembly.txt。MASTER_PLAN §20.78、STATUS／WORKLOG／TASKS／UI／模型同步，总目标active。

### 2026-10-23 — R79：地区当前州上下文的离线解码器

- 本机 EXE 与分析副本 SHA-256 一致；Ghidra＋Capstone 核对 states_panel 为面板栏 slot14，vtable 0x45CC848，State typed ID 位于 +0x70；原版 local_goods 为本地价格页签。未调用原生 getter、未启动／附加／操控游戏。
- 新增 state_panel.h 仅复制自有数字／字符串，核验 UI 所有权链、关闭／最后历史项、类型／名称／字符串及重读一致性；未知／无效／复制中切州或界面替换拒绝整体结果。明确 ID 0 与无 ID 区别；没有 runtime caller，未启用未验证路径。
- 新 state_panel_tests 已接 build_tests.ps1；新增异常输入／切换／变化拒绝与原有回归共14项 PASS，verify_embedded PASS。dist／已部署仍 R78 48f0e495698bcb754bdd400a77cdc2f05c68543bf1c2d880d6f05ff5cb69cf29，本轮仅新增离线代码／测试，未重建或部署 DLL。
- 地区列尚未交付；下一步采样当前州与 snapshot ID 对应、接 owner／逐行上下文后追加原版本地价格列。正向实机／安全窗口／全生命周期保持未验。备份2026-10-23-r79-state-context；私有 state-panel-r79-static.md／assembly.txt，native/build/tests-round79.log／state-context-round79-final.log；MASTER_PLAN §20.77、STATUS／WORKLOG／TASKS／UI／PROFILE同步，目标active。

### 2026-10-23 — R78：参考图市场卡片／商品列与逐行发布

- 市场页新增 Freight／已铺铁路卡片、商品工作量与全市场占比列；保留原版行、价格图表／提示／右键。价格块从版本校验后的本机 GUI 派生替换占位符，原价块及原版纹理不内嵌 DLL。
- 逐行变量按商品键规范化，hover不改工作量／占比；缺失／合成／重复键／无效值拒绝，零工作量0.00、零市场份额--。UI写入重验帧、先撤ready上下文，成功后才显示；失效行清理。已有变量直接assign，避免重复构造key/value对，原生写入分支仍待实机。
- 13项回归、完整build、GUI／C++归档逐字镜像、verify_embedded PASS；候选／已部署 `48f0e495698bcb754bdd400a77cdc2f05c68543bf1c2d880d6f05ff5cb69cf29`（31,753,550B），游戏关闭时经deploy.ps1安装，旧R76安装另存checkpoint。
- 新布局／逐行发布／25ms开销／同帧关闭没有实机通过证据；地区Freight列仍待当前州上下文与GUI接线。未恢复Computer Use。备份2026-10-23-r78-market-reference-ui，MASTER_PLAN §20.76／STATUS／WORKLOG／UI／模型同步，总目标继续。
### 2026-10-23 — R77：参考图统计作用域

- 全市场／筛选卡片／临时hover工作量分开，地区商品仅计本州最终输入／输出；卡片改用 selectionFreight，商品占比分母与 Freight 需求保持固定。
- 反例先失败，修复后原句柄95099／97389均退出0，13项回归／完整build／verify_embedded PASS。候选 `3695238f83be193aa596e101360f53ec6b70c01a27ba5182b0f216aaf9d3787f`（31,174,023B），未部署／未实机，安装仍R76。§20.75／STATUS／WORKLOG／模型文档同步；备份2026-10-23-r77-reference-freight-scopes。
- 地区列与市场卡片／行布局仍待接线；未恢复被用户 Escape 停止的界面自动操作，后续继续代码及离线验证。

### 2026-10-23 — R76：恢复原版筛选与商品行悬停桥

- 用户裁决恢复显示搜索／分类；删除 C++／Python 的三处子控件隐藏改写，保留原版控件与折叠逻辑。新控件保留反例先 FAIL 后 PASS，归档镜像一致。
- 原版商品行 enter／leave 使用逐商品键和面板／world 上下文；仅当前可见商品可激活临时 hover owner，移出恢复原筛选，关闭／切页／身份变化取消两 owner，保留调试 owner。过期／歧义／失败读取拒绝；正式 hover 不依赖 ImGui。
- UI 交互改用 25ms 定时器，经济采集保持 500ms 独立节奏；定时器重建时清理。13 项回归、完整 build、GUI／归档／资源、selftest（560231 bright pixels）及默认隐藏 PASS。候选／已部署 `15fee7d63d34db078fafa1128de1afff1244c2a06b5e68896c15f044c16a5329`（31,174,023 B）；无游戏进程时经 deploy.ps1 安装，安装 hash 一致。
- 原测试 PID52316、后来的 PID8032 均已退出，未结束它们。sky 启动仍报 window-opened handler 未就绪；新 DLL 实机 hover／筛选／25ms 开销与同帧关闭 NOT_RUN，不能扩展离线结论。
- 用户补充地区／市场参考图，确认 Freight 列为货运工作量；要求原版本地价格追加列、市场行 Freight／全市场占比、筛选卡片汇总且 hover 不改卡片。写入 MASTER_PLAN §3.2.1；这些新增列／卡片布局仍待实现。备份 2026-10-23-r76-native-hover，§20.74／STATUS／WORKLOG 同步。

### 2026-10-23 — R75：正式货运工作量单位修正

- 当前分配器已将 quantity 从 milli-orders 转为 orders 后计算工作量；原版摘要又除以 1000 且误标 order-cost。提取现有显示边界复现 7 orders × 10 map units 应显示 70 而非 0.07，修复二次换算，标签改为 weighted order-distance。
- 运价／权重／筛选与独立 Freight 需求的回归通过；缺失、合成、负值及非有限数显示 `--`，合法零为 0.00。13 项原生测试、完整构建、GUI／归档镜像及嵌入资源校验 PASS。
- 候选 `64485541f14b7252297a0fa96a85bee3c0114e5fbd328c6d0af76e8001d020f8`（30,921,468 B），未部署／未实机验证新单位；安装及 PID52316 仍为 R69。备份 2026-10-23-r75-freight-work-units，MASTER_PLAN §20.73、STATUS／WORKLOG／README／模型文档同步。
- 原版控件显隐冲突已按 §1.1 请求用户裁决；未修改显隐补丁。正式 hover／逐帧关闭、Freight 供给／铁路／完整生命周期仍未完成，总目标继续。

### 2026-10-23 — 文档维护：编码工作规范

- 将用户提供的四组规范整理为 §1.1–1.4，修正“代码重构svg”笔误；简化仅限任务范围，矛盾时暂停受影响工作并澄清。
- 保留项目约束、构建入口与既有日志。仅修改文档，核对备份差异、四组规则及 UTF-8 无 BOM；同步 STATUS 与 MASTER_PLAN §20.72，DLL 沿用 R74。

### 2026-10-23 — R74：原版搜索／分类结果复用

- 补齐明确市场 ID 0 与旧版“未指定市场”的区别：原版 owner／SelectMarket 显式保留 0，只有未指定选择才自动回退；会话反例覆盖 market 0／primary 7，不能据此认定当前存档市场 0 有效。最终 tests-round74-final.log／build-round74-final.log 均退出 0，13 项回归通过。
- 恢复原版分类字节排除表与Unicode／本地化搜索缓存链。候选直接安全复制Market.AccessMarketGoods缓存ID，检查唯一market／query／dirty／writer／行类型和归属／变化；不按英文key重做搜索，不调用getter或等待引擎锁。
- allowedGoods仅筛视图，排序规范化；分配、市场分母／热度和Freight需求不变。待刷新撤视图／waiting，已知空结果无线路但保留全市场统计。13项回归／完整build／verify_embedded PASS。
- 候选1a97a6823e2fc72fa7f1f2178807803a4001fc89faee3ccd8df3fdc8b0223cfd、30,916,271B，未部署。实机registry仅两项0／FFFFFFFF，均dirty1／0rows，实际解码回放正确拒绝；没有正向缓存／新DLL交互通过证据，500ms／hover／生命周期仍待验。
- 备份2026-10-23-r74-native-filter-view；私有native-filter-r74-*、native/build/*round74*.log；MASTER_PLAN §20.71、STATUS、WORKLOG及模型／UI／PROFILE同步。调试器已分离、游戏R69保持运行，继续无人值守。

### 2026-10-23 — R73：原版市场状态／正式 owner 接线

- 静态恢复信息面板栏slot9及原版IsPanelOpen谓词，实机读取PID52316整条所有权链并回放实际解析器：当前关闭／default／market无效。原始字节与反编译只存私有market-panel-r73-*，未调用getter／RTTI cast／GUI效果。
- 候选仅原版物流页签打开时激活正式owner，按面板market选择，去掉快照就绪即激活最大市场；关闭／切页／未知／数据缺失撤正式帧，重复采样不重算，保留其他owner。正式变量核验owner／当前帧／world／revision／market，非当前数据为waiting／`--`。
- 完整build／13项原生回归／实机关闭态回放／verify_embedded PASS；候选e0a2e7863db4f016091888b4e6d885678fae3149db8a12e6e4f22b58c7a3d1a6、30,376,351B，未部署，运行仍R69。500ms定时同步不是逐帧即时关闭验收，打开／切换／关闭实机交互待验。
- 已分离调试器，游戏Responding；未强制结束／替换DLL。MASTER_PLAN §20.70、STATUS／WORKLOG及UI／profile同步，备份2026-10-23-r73-native-panel-owner；保持无人值守，Freight供给／铁路／正式hover和完整生命周期继续推进。

### 2026-10-23 — R72：实机53商品／TLS验证与真实快照接线

- x64dbg读取PID52316全部53商品／主线程TLS context，按key对原版定义53/53一致；53定义及向量重读一致，实际解码器／线程解析器回放PASS。原始字节和工具只留私有分析目录。
- 候选goods_snapshot.h接UI定时器：当前已初始化context、RPM／前后核验、原生ID映射真实key／分类／local／tradeable，无goodN回退。未知／缺失／变化撤正式owner／文档，显示waiting／`--`；weight／Freight系数1仍为假设。
- 完整build／12项回归／verify_embedded PASS；候选 `05b58a2d1b85c61ac952b4c38b9744be16c915f4b84c6b6092dd827a2156293b`（30,159,766B），未部署／未实机跑新adapter，当前仍R69。MASTER_PLAN §20.69、STATUS、WORKLOG、模型／profile同步，备份2026-10-23-r72-live-goods。
- 附加自动TLS断点和Nakama首次C++异常曾短暂停止，已正常恢复／分离；确认调试器stopped、游戏Responding／日志继续。未显式写经济数据、强制结束或替换DLL；继续无人值守，原子快照／完整生命周期／新候选正式可见性仍待验。

### 2026-10-23 — R71：商品元数据静态解码与不可交易分类

- 按用户最新要求无人值守推进，不再等待人工显示隐藏窗口。PID 52316 仍运行 R69，未操作／暂停／结束或替换 DLL。
- 当前 EXE 静态确认商品 ID／key／local／tradeable 字段；新增离线 goods_catalog 解码器及 12th 原生测试，拒绝错误类型／ID／字符串／重复 key／复制期间变更，失败清空部分结果。解码器未接实时采集，访问窗口／catalog 身份仍待实机验证。
- 模型 `tradeable` 与 localOnly 分开；不可交易商品不产生跨州分配或 Freight 中转需求，分类纳入缓存身份。真实 adapter 仍用 goodN 和默认标记，正式输入排除尚未修复，不能据模型测试宣称实机语义正确。
- 完整 build／12 项回归／verify_embedded PASS；候选 `698c8b8dc5486c4fc3f3e96086da208deeaf115b1f2d4b352c22ab0d21b08d56`（29,840,576 B），未部署。证据 `native/build/*round71.log`、私有 `E:\Code\v3-analysis\goods-catalog-r71-static.md`。
- 备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-r71-goods-metadata`；MASTER_PLAN §20.68、STATUS、WORKLOG、FREIGHT_MODEL／PROFILE 同步。x64dbg 插件未上线，未附加游戏；继续安全采集适配与未验收事项。

### 2026-10-23 — R70：真实 native 提交／热态零上传与重复发布修复

- 直接进程启动 R69 成功（PID 52316、Hidden、continuelastsave）；窗口工具启动器失败不等于游戏不可运行。真实 1117／28554／49、market118、owner10、37 ribbons、rev1→6，调试 UI 启动 vtx0，持续 native SUBMIT。
- 170.453 秒热态 35 样本 uploads 恒定37，提交7966→303596；图像不可见，不能据日志通过地图可见／全交互。游戏仍运行，未强停，已请求用户显示窗口。
- 修复同一 worker reply 每500ms重复发布文档（运行样本6次采集／349次发布）；新测试先FAIL后PASS，去重保留渲染帧与新请求发布。
- 候选 `f3a9a4befc8db63e6f2562ba4a71be6e38b447055aecc38d98756ef440c5e7e3`（29,840,576 B）；build／11项native回归／verify_embedded PASS。未部署，运行中仍为 R69 `17268f5e…`，未执行会写游戏日志的 standalone selftest。
- 证据 native/build/*round70*；备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-r70-runtime-publication`；MASTER_PLAN §20.67／STATUS／WORKLOG／PERFORMANCE／ACCEPTANCE 已同步。

### 2026-10-23 — R69：世界重置／迟到快照与发布门控修复

- 新测试先复现 reset 后旧 world 被接受；Engine／Session 保留已退休 world 门槛，即使快照为空也拒绝旧 world（高 revision 不可绕过）。worker 重置改用独立失效代数，合并／Cancel 不会吞掉重置。
- Session 将入队与 reset 串行化，接受快照立即撤下旧帧，最终发布核对 snapshot 指针／owner／filter／generation；render 通过 FrameIsCurrent 复核。实时 world 改为递增，周期刷新先验证旧 anchor；旧 reader 探针改为仅落盘，禁止其未版本化累计数据进入正式会话。
- 同时修复 OnRenderPass 对 ImGui 可见性的残留依赖；源码门控已改，实机仍未验证。世界替换后 native 对象寿命未证实，因此停用后续原生箭头访问／提交并提示重启，不能把此保护算完整跨读档支持。
- `repro-round69.log` 保存修复前 FAIL；`tests-round69-final.log` 11 项 PASS；最终 build／GUI runtime／verify_embedded／selftest／debug_default PASS。候选／已部署 `17268f5ea3074e6072b3b68d7b64c29b837987cf0ebd1ff36b59fdb0e3bb3bba`（29,839,552 B），仅经 deploy.ps1；旧安装 R67 保存为 checkpoint 的 `installed-version.dll`。
- 游戏启动器本轮仍报 window-opened handler 未就绪，未发现游戏进程／窗口；保留实机 NOT_RUN。备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-r69-world-reset`。
- `MASTER_PLAN.md` §20.66、STATUS／WORKLOG／DECISIONS 与 TEST_PROCEDURE 已同步；完整生命周期与原生对象跨世界安全回收仍未完成。

### 2026-10-23 — R68：市场 getter 静态契约与关闭态探针

- 同哈希 EXE 的 Ghidra＋Capstone 确认 `GetMarket` getter 使用 panel+0x70 ID，返回 caller-owned out；wrapper+0x10 保存 ID、+8 为引用计数。resolver 持锁且可创建 wrapper，不可主动轮询调用。
- `probe_panel_market=0` 默认关闭；启用需 hash／指令前缀门控，只随原调用复制 ID，最多 16 条日志，不持指针／不改选择。实机取值、线程／生命周期尚未验证，正式自动识别仍未完成。
- 候选 `09b219a9e596f999bd3e2182672b2e551b2fa85e450f266b2297bdf64af27fd0`（29,877,463 B）；未部署，安装目录保持 R67 `52009510…`。build、verify_embedded、debug_default PASS。
- 证据 `E:\Code\v3-analysis\panel-market-r68-static.md`、`native/build/*round68.log`；备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-r68-market-getter`；MASTER_PLAN §20.65 已更新。

### 2026-10-23 — R67：Freight 中转需求核算与文本桥

- balance 新增分配前目标需求与最终 fulfilled／unmet，系数独立于 weight／距离／运价；有来源商品贡献，排除 Freight 自递归，需求汇总不受过滤影响。缺失值用 optional，不伪造 0。
- 系数与中转标志纳入缓存键；原版页签新增 `ironroad_demand_v1`，默认系数 1 为模型假设。独立 Freight 供给机制与是否注册游戏内商品仍待确定，A27 未整体完成。
- 构建、11 项原生回归、GUI／归档、verify_embedded、selftest、debug_default PASS；新增系数变化保持线宽与几何身份测试 PASS。归档首次提前读取旧 DLL 失败，待构建后重跑通过。
- 候选／已部署 `52009510d0ff6c64af6e03b77ab109c7d9c6d5bafd56ce101116a2b6fb1c7bd9`（29,863,251 B）；部署仅经 deploy.ps1。Windows 控制工具启动两次失败，游戏无进程，新 UI 实机 NOT_RUN。
- 证据 `native/build/*round67.log`；备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-r67-freight-demand`；`MASTER_PLAN.md` §20.64 已更新。

### 2026-10-23 — R66：新增 Freight 中转商品需求约束

- 用户明确要求以 Freight 作为中转商品计算物流需求；已写入 `MASTER_PLAN.md` §4.5、P4 和 A27，并更新模型说明与任务表。
- 区分 Freight 目标需求／已满足／未满足与现有 `shipmentFreight` 工作量；公式、单位换算、供给机制待设计，不将 R35–R65 的 Freight 文本算作本需求完成。
- 本轮仅文档变更；UTF-8 无 BOM 与条目一致性校验通过，未构建／部署；DLL 哈希保持 `200f08d2d738d829afa47097758f14d6250cfcd20c9945cda324d714bd4de134`。
- 回滚点：`E:\Code\v3-analysis\checkpoints\2026-10-23-r66-freight-intermediate`。历史日志日期保留；本条日期按客户端 Asia/Hong_Kong 上下文记录。

### 2026-10-24 03:05 — R65：正式 UI 市场对比截图

- 默认：`model allocation (market 118, auto)`、`11728.46`、`16440.38 / 1862.16 / 14578.22`、37 ribbons。
- `market_override=79`：`model allocation (market 79)`、`601.13`、`7465.20 / 451.99 / 7013.21`、47 ribbons。
- 当前缩放下地图 ribbon 未清晰可见（market 79 含海外州）；页签数值确认作用域切换。
- 无源码改动；候选/已部署 `200f08d2d738d829afa47097758f14d6250cfcd20c9945cda324d714bd4de134`；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r65-*`。
- 记录：`MASTER_PLAN.md` §20.62 已更新。

### 2026-10-24 02:45 — R64：脚本函数注册表探针（负结果）

- Ghidra：注册表固定全局 `0x1459FD648`，条目 0x78，size 在 +0x08（实测 141）。
- 新增 `probe_registry`：实机读取表头/条目，未提取出条目名（偏移不符），未找到 market 函数。
- 候选/已部署 `200f08d2d738d829afa47097758f14d6250cfcd20c9945cda324d714bd4de134`（29,781,483 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `tests-round64.log`。
- 记录：`MASTER_PLAN.md` §20.61 已更新。

### 2026-10-24 02:10 — R63：全新缓存重装演练

- 删除 `%LOCALAPPDATA%\IronroadV3` → 卸载/重装 → 启动自动重建缓存（data/assets/native-gui/ini/log）。
- 页签正常：`model allocation (market 118, auto)`、`11728.46`、`16440.38 / 1862.16 / 14578.22`、`Markets (id:states): ...`；37 ribbons、141.4 ms；无错误。
- 无源码改动；候选/已部署 `c69340f169f680346a1643409995bfc602f38d6ff57b3d643ab2324d11b3a16a`；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r63-fresh.png`。
- 记录：`MASTER_PLAN.md` §20.60 已更新。

### 2026-10-24 01:50 — R62：页签市场概览行

- 新增第 8 个 GUI 变量 `ironroad_markets_v1`（前五市场 `id:states`，互斥保护）与页签 `Markets (id:states): ...` 行。
- 实机：`118:77 164:75 35:74 33:68 34:58`；变量发布无失败；37 ribbons、136.2 ms。
- 候选/已部署 `c69340f169f680346a1643409995bfc602f38d6ff57b3d643ab2324d11b3a16a`（29,771,621 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；`test_native_gui.py` PASS；证据 `native/build/r62-markets.png`。
- 记录：`MASTER_PLAN.md` §20.59 已更新。

### 2026-10-24 01:25 — R61：完成审计与实现报告刷新

- `COMPLETION_AUDIT.md` 重写为 R61 状态（逐项证据 + 缺口）。
- `IMPLEMENTATION_REPORT.md` 顶部新增 Current status（当前哈希、实时市场作用域数据、缺口）。
- 无源码改动；DLL 仍为 `0cb83a456e3b6237db2b36574848ac0e6f87ef3c96890d7edc5ff11d51672d56`；文档 UTF-8 有效。
- 记录：`MASTER_PLAN.md` §20.58 已更新。

### 2026-10-24 01:05 — R60：自动市场标注

- 页签自动选择时显示 `model allocation (market N, auto)`；`market_override>0` 时显示 `(market N)`。
- 实机自动模式：`model allocation (market 118, auto)`、`11728.46`、`16440.38 / 1862.16 / 14578.22`；37 ribbons、138.8 ms。
- 候选/已部署 `0cb83a456e3b6237db2b36574848ac0e6f87ef3c96890d7edc5ff11d51672d56`（29,752,211 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r60-auto.png`。
- 记录：`MASTER_PLAN.md` §20.57 已更新。

### 2026-10-24 00:50 — R59：市场提示日志 + 设置持久化补齐

- 采集日志新增 `top=`（前三市场州数）：`top=118:77 164:75 35:74`。
- `SaveSettings` 补写 `probe_market`/`market_override`（此前保存会丢）。
- 实机 37 ribbons、137.8 ms。
- 候选/已部署 `065bbf34db83f4843aafc29095e7f4692d5a6b1629866ebcc45a4efa0f66aa85`（29,750,126 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `tests-round59.log`。
- 记录：`MASTER_PLAN.md` §20.56 已更新。

### 2026-10-24 00:30 — R58：README 用户文档刷新

- 修正标题乱码；过时的“UI/数据仍在接通”改为正式第五页签 + 实时市场作用域数据。
- 新增 Logistics tab semantics：order-cost 口径、target/allocated/unmet、`model allocation (market N)`、`market_override` 用法、非游戏订单/MAPI 声明。
- 无源码改动；DLL 仍为 `a98e45b56e08d7f8960e308797d2aaf351fb3e63925e8d2491909f5ab163a394`；README UTF-8 校验通过。
- 记录：`MASTER_PLAN.md` §20.55 已更新。

### 2026-10-24 00:10 — R57：市场归属核对

- `probe_market` 新增 `eco market id=`：17/19=**79（UK）**、132=71、350=83、201=64、205=33、432/436=164。
- 游戏变量表仅含我们的 7 个 `ironroad_*` 键，无市场上下文；默认 primary=118（州数最多）**不是 UK**。
- `market_override=79` 可得到 UK 市场（R54：7465.20/451.99/7013.21）。
- 候选/已部署 `a98e45b56e08d7f8960e308797d2aaf351fb3e63925e8d2491909f5ab163a394`（29,632,396 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `tests-round57.log`。
- 记录：`MASTER_PLAN.md` §20.54 已更新。

### 2026-10-23 23:45 — R56：脚本注册探针（负结果）

- 新增 `probe_register` + `kRvaScriptRegister=0x3AB6D00` 探针；实机安装成功，但未观察到 market 注册。
- 结论：该路径不能取得市场面板 market；玩家市场识别仍用“最大市场 + `market_override`”。
- 候选/已部署 `227793796c7413948ca97af290a258f69b25a6a1b078983eae2f428756046e1c`（29,630,860 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `tests-round56.log`。
- 记录：`MASTER_PLAN.md` §20.53 已更新。

### 2026-10-23 23:20 — R55：面板开关循环稳定性

- 10 次 F4/ESC 开关（含 Shift+5）存活、无错误/无 `world changed`；周期刷新 rev 1→2、37 ribbons（135–137 ms）。
- 重开后页签正常：`model allocation (market 118)`、`16440.38 / 1862.16 / 14578.22`。
- 无源码改动；候选/已部署 `f02169a82b3153c262577ba85a6505cf40a9e968d64c537956cc33e2308a66ea`；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r55-cycles.png`。
- 记录：`MASTER_PLAN.md` §20.52 已更新。

### 2026-10-23 23:00 — R54：market_override 可配置市场

- 新增 ini `market_override`（默认 0=最大市场；>0 锁定市场 id）。
- 实机（79）：`primary=79 states=30`、47 ribbons；页签 `model allocation (market 79)`、`7465.20 / 451.99 / 7013.21`。
- 玩家市场自动识别仍未完成；override 提供可控修正。
- 候选/已部署 `f02169a82b3153c262577ba85a6505cf40a9e968d64c537956cc33e2308a66ea`（29,625,098 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r54-override79.png`。
- 记录：`MASTER_PLAN.md` §20.51 已更新。

### 2026-10-23 22:40 — R53：market 作用域突破

- `state+0x48` = market id（1117 州仅 59 个值；威尔士/高地=79、莫斯科=71）。
- 快照按市场分组；`EconomySnapshot.primaryMarket`=州数最多市场；session/filter/selection 使用；页签 `model allocation (market N)`。
- 实机：`markets=59 primary=118 states=77`、37 ribbons、136 ms；页签 `16440.38 / 1862.16 / 14578.22`。
- primary 取“州数最多”为启发式，非玩家市场精确值。
- 候选/已部署 `371d2db7ed57dcd20fc11ced2d76ba901a3881629d1ada0a45ad4e249dabc42f`（29,618,209 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r53-*`。
- 记录：`MASTER_PLAN.md` §20.50 已更新。

### 2026-10-23 22:05 — R52：全局作用域标注 + 州面板映射观察

- 正式页签状态改为 `model allocation (global)`，如实标注全局作用域；实机显示 `126318.85 / 519674.48 / 93784.10 / 425890.38`。
- 州面板点击：威尔士/苏格兰高地名称与 positions 一致；尝试用“本地价格”页签触发 reader 核对 runtime id 未命中（坐标/触发问题），映射仍未由 reader 证实。
- 候选/已部署 `5db7d577ce9375a2104fd705e1867f2ed1029aa493aeea89fa9c38d0346559a4`（29,312,141 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r52-*`。
- 记录：`MASTER_PLAN.md` §20.49 已更新。

### 2026-10-23 21:40 — R51：容器头探针（负结果）+ 验收矩阵

- `probe_market`（默认 0）输出 8 个州的 `state+0xC8/+0x78` 容器头；16/17/18 与 132/432 **无共享 market 指针/ID**（静态地址/每州数组/标志位）。
- `docs/transport/ACCEPTANCE.md` 重写为 R51 状态：A03/A07/A13/A15/A21/A22/A23/A25/A26 PASS，其余 PARTIAL；明确 market/rail/hover/owner 缺口。
- 候选/已部署 `a25f78ae7006562a826fa3992339d64743d3877e4173656dd7cd91aa6fd7ea45`（29,311,629 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `tests-round51.log`。
- 记录：`MASTER_PLAN.md` §20.48 已更新。

### 2026-10-23 21:05 — R50：GUI 补丁镜像修复与全量回归

- `tools/test_native_gui_runtime.ps1` 曾失败（`AssertionError: gui/market_panel.gui`）；根因是 Python `patch_market` 未同步 R21/R22 的 C++ 补丁。
- 将 Python 补丁逐字对齐（container/内容块/FILTERS/搜索栏/过滤器容器）；运行时归档校验恢复 **exact Python/native patch agreement**。
- 全量回归：native_gui / native_gui_runtime / flow_gpu / selftest / debug_default / verify_embedded / build_tests 全部 PASS。
- DLL 未改动；候选/已部署 `5e74477a73f50e0924f72b449ab2986f8b619d698d623a91e23544e9984691fc`；证据 `tests-round50.log`。
- 记录：`MASTER_PLAN.md` §20.47 已更新。

### 2026-10-23 20:40 — R49：世界失效安全演练

- 抽出 `InvalidateEconomy`；新增 `test_world_change`（默认 0）调试注入。
- 实机：采集 → `TEST world-change: invalidating` → 重新采集 → `gen=3 rev=2`、989 ribbons、139 ms。
- 真实重载自动化打开国家预览界面；未继续（未改档）。
- 候选/已部署 `5e74477a73f50e0924f72b449ab2986f8b619d698d623a91e23544e9984691fc`（29,309,069 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `tests-round49.log`。
- 记录：`MASTER_PLAN.md` §20.46 已更新。

### 2026-10-23 20:05 — R48：内存平台与刷新稳定性

- 3 分钟 18 次采样：warm 7278–7302 MB（±24 MB，0.3%），后段下降、无泄漏趋势；周期刷新 rev 5→6、989 ribbons、~139 ms。
- WorkingSet 含游戏基线（~7.3 GB），mod 增量未单独隔离。
- 无源码改动；候选/已部署 `b19e8308fb1ac83cddda35a4c2b8ff73812e7039f4d5a7017d988798268eee97`；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `tests-round48.log`。
- 记录：`MASTER_PLAN.md` §20.45 已更新。

### 2026-10-23 19:40 — R47：回滚部署演练通过

- 卸载 DLL 后游戏正常进入地图（无物流页签/箭头）；重新部署后采集 1117/28554/49、989 ribbons、144.2 ms，功能恢复。
- 缓存 `%LOCALAPPDATA%\IronroadV3` 保留、不影响回滚。
- 无源码改动；候选/已部署 `b19e8308fb1ac83cddda35a4c2b8ff73812e7039f4d5a7017d988798268eee97`（29,303,893 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r47-*`。
- 记录：`MASTER_PLAN.md` §20.44 已更新。

### 2026-10-23 19:05 — R46：周期刷新 + 重载演练尝试

- 每 30 秒重新采集（revision+1），worker 后台重算；实机 rev=1→2、gen=1→2，989 ribbons，~145 ms。
- 重载演练：暂停菜单自动化点击命中“切换国家”界面（未选国家/未保存/未改档），端到端演练待补。
- 候选/已部署 `b19e8308fb1ac83cddda35a4c2b8ff73812e7039f4d5a7017d988798268eee97`（29,303,893 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `tests-round46.log`。
- 记录：`MASTER_PLAN.md` §20.43 已更新。

### 2026-10-23 18:40 — R45：世界变化的生命周期失效

- 采集记录 anchor（首个州指针+vtable+id），`CheckWorldLifecycle` 每 ~2 s 复核；不匹配则 ResetWorld + 清空文档并重新采集。
- 实机 20 s 无 `world changed` 误报；数据保持（1117 州/28554 订单/989 ribbons，137 ms）。
- 存档重载/主菜单往返的端到端演练未执行（待补）。
- 候选/已部署 `f447b9b754b9dceab3b93a027be0c413b155152d5053e2442f2458dfabd9349c`（29,302,357 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `tests-round45.log`。
- 记录：`MASTER_PLAN.md` §20.42 已更新。

### 2026-10-23 18:10 — R44：默认真实数据（无 probe）

- 新增 `CollectRealEconomy`（UI 定时器、vtable+容器数据探测、5 秒重试）；移除真实数据对 `probe_economy` 的依赖。
- 实机（probe_economy=0）：`collected states=1117 orders=28554 goods=49`，页签真实数值 `126318.85 / 519674.48 / 93784.10 / 425890.38`，989 ribbons，`compute=140.695 ms`。
- 候选/已部署 `0a21fdb1890e53cec79b8a81512a76745c1c57e0d93304848bcf086e2242dad0`（29,300,253 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r44-final.png`。
- 记录：`MASTER_PLAN.md` §20.41 已更新。

### 2026-10-23 17:30 — R43：分配并行化（741→127 ms）

- 按组（market, good）抽出 `runGroup`，8 线程池并行；balances 每组独立、shipments 分线程收集合并；线程异常经 exception_ptr 回传主线程（否则 `terminate: cancelled`）。
- 实机：`allocation=126.94 ms / total=133.38 ms`（8 线程），`flow=893.20 decomp=12.62 shipments=7016`，ribbons=989；约 5.8× 加速。
- 候选/已部署 `df10b6c29abb77cfe587c8b65d44a65f1800583ebb84056d93404f0f156b0f06`（29,182,830 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `tests-round43.log`。
- 记录：`MASTER_PLAN.md` §20.40 已更新。

### 2026-10-23 17:00 — R42：分配性能剖析与首个优化

- 分段计时：`allocation=741.30 ms / view=10.73 ms`；分配内 `flow=729.03 / decomp=11.16 ms`（7016 shipments）。
- 分解阶段改为 importer O(1) 查表，分解降至 11 ms；瓶颈为 min-cost flow 增广。
- 候选/已部署 `f85067d9fb6df0f33ad8f09eb8c588337c8c151aec6c281b59f19c33f16bed87`（28,966,428 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `tests-round42.log`。
- 记录：`MASTER_PLAN.md` §20.39 已更新。

### 2026-10-23 16:20 — R41：真实数据 ribbon 上屏

- `ApplyPendingFreight` 不再要求 ImGui 可见；原生 owner 活跃即提交 ribbon（pump 取消条件同步）。
- 实机：真实帧 989 ribbons 上屏，`compute=768.119 ms`（后台 worker），`native pool 989/8192`、`visible yes 999`，地图可见细线。
- 性能需优化（768 ms 分配）；宽度档因全局摊薄偏细。
- 候选/已部署 `b6582a238252bb0abedb69c19f05a21008adb31552d0e04a053cb0cc423630b4`（28,929,209 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r41-*`。
- 记录：`MASTER_PLAN.md` §20.38 已更新。

### 2026-10-23 15:45 — R40：market 字段追踪（负结果）

- 对比英国本土 16/17/18 与外国 132/432 的 state 0x200 字节、0x80/0xa8/0xd0 目标对象 0x80 字节：无“英国相同、外国不同”的 qword/dword。
- market 非 state 直接字段，可能经所有者国家/省份间接获得；需先验证州身份或 hook 市场面板 market 对象。
- 候选/已部署 `a6fe068d0d76205cf2d701066ce7e38f37a78959add8b4bfc272f02659df6a2d`（28,928,697 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r40-*`。
- 记录：`MASTER_PLAN.md` §20.37 已更新。

### 2026-10-23 15:05 — R39：全州枚举接入正式 UI

- 新增一次性全州枚举（lookup + vtable + id 三重校验、容器读取），绕过“本地价格”页签首屏限制。
- 实机 1117 州：totalProd=88.9e9、totalCons=94.2e9（≈889,366/942,319 单位）；页签 `Freight 126318.85`、`Target 519674.48 / Allocated 93784.10 / Unmet 425890.38`，分配率 18.0%。
- 候选/已部署 `d98d815b8f46520b056168489d2d2b1725d3f47099c1957880f2b52448e1566a`（28,922,465 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r39-*`。
- 记录：`MASTER_PLAN.md` §20.36 已更新。

### 2026-10-23 14:30 — R38：州字段扩展转储；页签非市场作用域

- 对 12 个州转储 state 前 0x200 字节；实机仅 6 个全球州被访问（205/201/350/132/432/436），英国本土 16–23 缺失 → 本地价格页签是全球排序子集，非当前市场。
- 州对象 0x00–0x100 未见 market 字段（0x80/0xa8/0xd0 每州数据指针，0xb0 类别计数）；需同市场样本或静态分析。
- 候选/已部署 `ad0a4dc14f10a34ae70e966522ff89fcbf48c69148a546f769accdda48b14fce`（28,920,642 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r38-*`。
- 记录：`MASTER_PLAN.md` §20.35 已更新。

### 2026-10-23 13:55 — R37：region 映射确认；跨市场混样

- 州 id 与 `data/positions.txt` region id 一致（201/205/432/436/132/350）；790/940 缺失无 hub。
- 州对象未直接含 `STATE_` key（无需 key）；商品“本地价格”列的是全球州，快照全部标 market=1 → 跨市场混样，低分配率主要嫌疑。
- 下一步定位州 market 字段，按市场分组采集。
- 候选/已部署 `8feb3b81c2e8f625aac88302cd5259c082d70ccd1ee60cfe21cd8d5d16c56ea1`（28,904,528 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r37-*`。
- 记录：`MASTER_PLAN.md` §20.34 已更新。

### 2026-10-23 13:20 — R36：Freight 口径/标签修正

- `allFreight` 是 milli-orders×cost，不是订单数；显示改为 `/kQuantityScale`，标签改为 `Freight (order-cost)`。
- 实机页签：`model allocation`、`Freight (order-cost): 23108.93`、`Target/allocated/unmet 259398.15 / 14746.49 / 244651.66`。
- 分配率仅 ~5.7%，指向 state→region 映射/铁路缺失；下一步定位州对象字段。
- 候选/已部署 `75cc6de5ef452761f4897cf223cad79ec5ff11d6554821d3c301aa93f5a50f74`（28,895,740 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r36-*`。
- 记录：`MASTER_PLAN.md` §20.33 已更新。

### 2026-10-23 12:50 — R35：正式 UI 显示真实模型分配（里程碑）

- 根因：insert-or-find 不更新已存在 key 的 value；首次写入 waiting 后真实值被忽略。
- 修复：insert 后在 entry+0x28 通过引擎 `assign` 更新 value；实机连续更新无崩溃。
- 物流页签显示真实数据：`model allocation`、`Freight (orders): 23108927.60`、`Target / allocated / unmet: 259398.15 / 14746.49 / 244651.66`、`Built rail: --`。
- 真实数据链路（引擎→快照→模型→原版面板）首次完整可见；候选/已部署 `77a3efd064b28d5b2cb27dd23f168a2d53a45f2674644b3811711906f3939b0c`（28,895,740 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r35-*`。
- 记录：`MASTER_PLAN.md` §20.32 已更新。

### 2026-10-23 12:05 — R34：真实快照接入会话并生成模型分配

- `writeDump` 将采集数据映射为 `EconomySnapshot`（state/region=stateId、market=1、good 0..127、1e5→milli/100），`PublishRealEconomy` 以 `synthetic=false` 注入会话并激活 `kOwnerNativePanel`。
- 实机：`accepted orders=3082 goods=49 market=1`；会话 `owner=10 gen=1 frame=1 ribbons=116` —— 真实数据生成 116 条模型分配 ribbon。
- 正式页签文本本轮仍显示 waiting（文档发布/时序待修）；`DrawUi` 已在原生 owner 活跃时避免清空文档。
- 候选/已部署 `bd63bf36916931a098f9d6b2d3d1f590b22562272e5deb83e868b1b3a6d6a733`（28,892,668 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r34-*`。
- 记录：`MASTER_PLAN.md` §20.31 已更新。

### 2026-10-23 11:25 — R33：真实市场快照采集落盘

- 读取器钩子内采集当前市场州（reader 路径）全部商品生产/消费，去重缓存；每 25 州写 `%LOCALAPPDATA%\IronroadV3\economy_dump.txt`。
- 过滤 `good<128 && value<1e12`（高位索引读到指针垃圾）；100 州总量 prod=24,007,540,245、cons=36,843,839,954（≈240,075/368,438 单位，1e5）。
- 真实快照数据就绪；下一步映射为 `EconomySnapshot` 并接入 session/正式页签。
- 候选/已部署 `474e638c41f971e3d5ab46f68320e77056c5c6e53c31c611cc1d388306d1f6c4`（28,703,482 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r33-*`。
- 记录：`MASTER_PLAN.md` §20.30 已更新。

### 2026-10-23 10:50 — R32：州枚举验证（reader 路径有效）

- 州对象 id 在 `state+0x8`；registry 盲枚举（id+vtable 过滤）得到 1117 个“州”但总量 4.5e18，无效。
- reader 路径（商品“本地价格”逐州）给出的州有效：state 205 prod 8/296,415、cons 17/240,237；state 201 prod 11/8,264,509、cons 38/11,239,733。
- 结论：真实快照应从 reader 观察到的市场州集合构建；下一步缓存该集合并生成快照。
- 候选/已部署 `d90dc2e590d856a7afc09b042d2922344bcc5fde3e0ff58e41ebf99022cacd7f`（28,654,634 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r32-*`。
- 记录：`MASTER_PLAN.md` §20.29 已更新。

### 2026-10-23 10:20 — R31：实机生产/消费数值与单位 1e5 验证

- 读取器钩子内用 registry+位图直接读 `state+0xC8`（生产）/`state+0x78`（消费），按州去重累计。
- 实机：state 432 prod=1.2M；state 436 prod=0.4M/cons=20.85M；100 州累计 prod=3,424,477,556、cons=5,942,248,942。
- 对 UI 市场总量 36.6K/61.2K 得比例 ≈9.4e4/9.7e4 → **固定点缩放 1e5**（除以 100,000 得游戏单位），经济数据源与单位验证通过。
- 候选/已部署 `eded5217d23162222fce2a2e53a2cbb2bc2eddde78904e0f0580eaad7bcdcb30`（28,643,288 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r31-*`。
- 记录：`MASTER_PLAN.md` §20.28 已更新。

### 2026-10-23 09:45 — R30：静态解码州经济容器布局

- Ghidra headless（IronroadEconomy-r2）反编译 `0x0103F3C0/0x0103F440/0x0195FF00/0x0195BA00`，输出 `E:\Code\v3-analysis\container-r30.txt`。
- 确认：indexed 值=`array[goodId]`（array@container+8，位图@+0x20，goodId=arg+0x10）；keyed 用 arg+0xC0 的键与 +0x30/0x20 表；读取器 state=registry(ctx+0x18)，容器=state+0xC8（生产）/+0x78（消费），值写入 out[0]。
- 新增读取器钩子内的直接读取实验（registry + 位图 + 数组）；实机最终触发被 tooltip 遮挡，未取到 `eco direct` 样本。
- 候选/已部署 `7e32789a92c75c0bcdf4885cb26f93f71d78ca722fa892b6c75f27124e3b6281`（28,540,190 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r30-*`。
- 记录：`MASTER_PLAN.md` §20.27 已更新。

### 2026-10-23 09:05 — R29：生产读取器/容器读取器钩子

- 新增 `probe_economy` 下三个观测钩子：生产读取器 `0x0195BA00`、keyed `0x0103F3C0`、indexed `0x0103F440`；仅记录后调用原函数。
- 读取器 detour 返回类型改为 `void*` 并原样返回原 rax（修正 bool 截断风险）。
- 实机商品“本地价格”走 **indexed** 路径：同一 out、8 个不同 arg（步长 0x70）；out 为条目向量头部，数值/商品 id 尚待解析；keyed 未触发。
- 候选/已部署 `450abd873cbdca0716a574ff5010c5b8e0f210161f1113faa0b690e9651b5b2f`（28,534,558 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r29-*`。
- 记录：`MASTER_PLAN.md` §20.26 已更新。

### 2026-10-23 08:35 — R28：经济回调 ctx/args/result 结构

- 通过物流页签货物行 → 原版商品面板 → **本地价格**，稳定触发 `GetStateProduction`（多个州逐次调用）；`ctx+0x18` 低 32 位为州 id。
- 捕获 ctx 6 qword（`[3]=0xffffffff_<stateId>`）、args（count=7、元素 0x20、begin=data+0x20）、result 包装值容器（vtable/代码指针/子对象）。
- 商品 id 与定点数值尚未解码；下一步 hook reader `0x0195BA00` 或检查 out 容器。
- 候选/已部署 `098293db77f3e9cca9ad18e2bfde54f23ac111396ef1876e6ef007224f6d99b1`（28,521,158 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r28-*`。
- 记录：`MASTER_PLAN.md` §20.25 已更新。

### 2026-10-23 08:05 — R27：GetStateProduction 脚本回调观测钩子

- 新增 `probe_economy`（默认 0）：MinHook `base+0x019791F0`，仅记录 `ctx/result/args`（含 `ctx+0x18` 州 id、`ctx+0x10/0x20`、args 计数），随后调用原函数；不取值、不构造上下文。
- 实机：州面板“本地价格”页签逐商品触发 `GetStateProduction`，日志捕获多组 `ctx/result/args`；确认经济脚本回调可用钩子观测。
- richer ctx 字段日志已构建但未抓到样本（鼠标坐标缩放导致州面板复现不稳）；下一步 dump ctx/args 并定位商品 id/数值。
- 候选/已部署 `9b8a8b31ff14c339141c53c68d667dab7332ba5ae6f448ce7ff49c3ab0ea6726`（28,518,598 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r27-*`。
- 记录：`MASTER_PLAN.md` §20.24 已更新。

### 2026-10-23 07:35 — R26：`ironroad_hide_flows` 驱动调试货运视图

- UI 线程每 500ms 通过只读探针轮询 `ironroad_hide_flows`，更新 `m_hideFlows` 并记录变化。
- `ApplyPendingFreight` 在隐藏时 `DeactivateRibbons` 并停止提交调试 ribbon；纯视图，不改数据。
- 实机 `test_hide_variable=1`：`TEST ironroad_hide_flows written` → `native panel hide flows=1`，确认轮询/标志生效；未做 prototype 视图肉眼消失验证。
- 候选/已部署 `c544f1e910fbc94117764c18fb5511682f9329a8e9cff134b98dae5cc9a38831`（28,513,281 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r26-*`。
- 记录：`MASTER_PLAN.md` §20.23 已更新。

### 2026-10-23 07:15 — R25：程序化验证显示/隐藏按钮状态

- 新增调试设置 `test_hide_variable`（默认 0，ini-only）：UI 线程写入一次 `ironroad_hide_flows`（堆字符串）。
- 实机对比：无变量时 `显示` disabled、`隐藏` enabled；写入后 `显示` enabled、`隐藏` disabled（`r25-baseline-row.png` / `r25-hideflows-row.png`），确认 `Exists/Toggle` 绑定正确。
- 测试后 `test_hide_variable=0`；候选/已部署 `43db0537e1df90a4b9ecadb7debde8fdd5e2477f6962f07636adf9f3f9d2b855`（28,505,886 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r25-*`。
- 记录：`MASTER_PLAN.md` §20.22 已更新。

### 2026-10-23 06:55 — R24：重开生命周期与商品面板联动

- Esc→F4→Shift+5 重开物流页签：内容/变量/按钮/货物列表正常，无崩溃（`r24-reopen.png`）。
- 点击商品行成功打开原版商品面板（钢：国际/本地/用途、消费税/关税、销售/购买订单，`r24-show.png`）。
- 显示/隐藏按钮的自动化点击仍受 GUI 缩放影响落到商品行；功能绑定本身未变。
- 本轮无源码改动，候选保持 `8f1a686c…780f`；证据 `native/build/r24-*`。
- 记录：`MASTER_PLAN.md` §20.21 已更新。

### 2026-10-23 06:35 — R23：物流页签布局微调

- 重写 `ironroad_logistics.gui` 顶部：状态/Freight/Target 标签/值行/按钮/Built rail 各自独立行，显式 `size` 与 `multiline`；显示/隐藏按钮单独 flowcontainer。
- 实机 F4+Shift+5：无重叠，内容为 waiting/`--`、按钮、`Built rail: --`、货物列表（`r23-layout.png`）。
- 候选/已部署 `8f1a686cea6889eb9f84ffc03c5a3fbebe0935f4c3f3cc28e3910b9e070e780f`（28,498,718 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r23-*`。
- 记录：`MASTER_PLAN.md` §20.20 已更新。

### 2026-10-23 06:15 — R22：物流页签隐藏商品过滤器/搜索栏

- 为商品专用的 FILTERS 按钮、搜索栏与过滤器容器增加 `Not(IsTabSelected('ironroad_logistics'))`；仅在物流页签隐藏，其他页签不变。
- F4+Shift+5 实机：物流页签从顶部显示 `物流`、`waiting for economy data`、`-- / -- / --`、显示/隐藏、`Freight: --`、`Track: --`、`货物`列表；FILTERS/搜索栏消失（`r22-clean.png`）。
- 余留：Target/按钮行轻微重叠；真实数据仍缺。
- 候选/已部署 `5a0ed5faffd7a09bb76f8d1424b41c5a10ccfe8a0db042af898fa836cf213252`（28,498,718 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r22-*`。
- 记录：`MASTER_PLAN.md` §20.19 已更新。

### 2026-10-23 05:55 — R21：物流页签内容与 `.Get` 最终实机确认

- 类型实例级 `visible` 不生效；改为 **container 包裹** 并设 `visible=IsTabSelected('ironroad_logistics')`，默认页签隐藏、物流页签显示。
- 页签提示 **Tab 5 / SHIFT+5**；发送 `Shift+5` 后“物流”高亮，内容显示 `waiting for economy data`、`Target/allocated/unmet: -- / -- / --`、`Freight: --`、`Track: --`、显示/隐藏按钮与货物列表。
- 正式 UI 文本桥、页签内容、变量写入链全部实机打通。
- 候选/已部署 `336208e29500851c3eb845ab7ddaabe81b90bd3e6e3c71c473160032dea22b80`（28,472,094 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r21-*`。
- 记录：`MASTER_PLAN.md` §20.18 已更新。

### 2026-10-23 05:25 — R20：标题点击切页实验失败，回退

- 临时在标题 `raw_text` 后注入 `onclick=SelectTab('ironroad_logistics')`；点击标题（~420,165）后实际选中“成员”页签，不是物流。
- 实验失败，已回退；正式构建保持 R19 的块内内容 + 仅第五页签可见。
- 候选/已部署 `9d5b1dee32e10a820bd5d3c38000c1bbfd53eecae0312a52f3588734fd54fdbe`（28,472,606 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r20-*`。
- 记录：`MASTER_PLAN.md` §20.17 已更新。

### 2026-10-23 05:05 — R19：页签内容容器修复；`.Get` 文本实机可见

- 发现 `ironroad_market_logistics_content` 必须插入 `market_panel_details_content` **块内**；块外同级元素不会进入页签内容容器。
- 临时 `Or(default, ironroad_logistics)` 可见性下 F4：内容区显示“物流”、`waiting for economy data`、`Freight: --`、`Track: --` 与显示/隐藏按钮（`.Get` 文本可见）。
- 生产可见性恢复为仅第五页签；第五页签点击自动化仍未命中。
- 候选/已部署 `a8a83cc94fdb48a22a5f68659194d44e59bfb16c3c1ef0ca079b8dfabaa2015e`（28,472,606 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r19-*`。
- 记录：`MASTER_PLAN.md` §20.16 已更新。

### 2026-10-23 04:45 — R18：后载变量文本桥打通（UI 线程 + 堆字符串）

- 根因：SSO 短字符串在后载 insert/move 路径损坏；改用 **长 key（>=16）+ 值补齐到 >=16**（堆字符串）后，7 个 `ironroad_*_v1` 变量全部正确写入，探针读回 status/freight 等。
- 标题临时 `.Get` 显示 `STATUS=waiting for economy data FREIGHT=--`，端到端确认；随后移除标题调试。
- 正式页签改用 `[GetVariableSystem.Get('ironroad_*_v1')]`；`native_variables` 默认开启；合成输入显示 waiting/`--`，不显示测试值。
- 页签内容仍未能实机可视化（点击/布局问题）；默认页签强制显示测试也未渲染。
- 候选/已部署 `387c7924df718ad042a77eaeffa4fb1571ca95e112ef78e9feb85931bd7eaa71`（28,471,070 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r18-*`。
- 记录：`MASTER_PLAN.md` §20.15 已更新。

### 2026-10-23 04:15 — R17：UI 线程写窗口实现；后载 pair 布局不匹配

- 复用窗口子类：`WM_APP+0x51` → `OnUiThreadPublish()`，UI 线程执行 `SetGuiVariable`；渲染线程只入队并 PostMessage。
- 后载写入不再崩溃（`UI-thread variables published`，进程存活），但 entry 只有 hash 与我们的 key 匹配，key 字节为垃圾、value 为空，`Get('ir_status')` 返回空。
- 结论：不是线程问题，而是**后载变量表的 pair/所有权布局与主菜单不一致**；手工 StringPair 写入不可靠。已回退默认关闭，GUI pending。
- 候选/已部署 `2bccef97b95aafaaa8d9cc37690e015da4545c601fab1c6ec99dd30939928d96`（28,466,083 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r17-*`。
- 记录：`MASTER_PLAN.md` §20.14 已更新。

### 2026-10-23 03:55 — R16：写入线程边界隔离；回退到安全默认

- 隔离：`.Get` 读缺失 key 不崩溃；`native_variables=0` 时地图正常加载；单个后载写入不崩溃但失败（`entry not found after insert`）；批量加载期写入会崩溃。
- 结论：Present 线程不是变量表合法写者；变量表在加载/首次 UI 使用时由 UI 线程占用。需要 hook UI 线程窗口（市场面板打开/更新）再写。
- 回退：移除“发布全部 key”的调用；单 key 后载探针仅 `native_variables=1` 时一次性尝试；标题调试移除；默认全部关闭。
- 候选/已部署 `c87c5ae6119bb7b5497dd12582cfe2e646c50d7dfe1f40b843f549dcd948086e`（28,460,700 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r16-*`。
- 记录：`MASTER_PLAN.md` §20.13 已更新。

### 2026-10-23 03:35 — R15：`.Get` 验证成功；渲染线程写入地图期不安全

- 临时把调试文本插到原版市场面板标题区，F4 打开后读回 `VAR_EXISTS=1 VAR_GET=long-value…`：**`GetVariableSystem.Get` 存在并返回写入字符串**，正式文本可用“写变量 + `.Get`”实现，无需 GUI 数据函数。
- 新增 `PublishLogisticsVariables()`（`ir_*` 标签）并尝试在地图加载期由 Present 线程写入：出现 `entry not found after insert`，随后进程退出；判定为与 UI/GUI 初始化的竞争。
- 回退：`native_variables` 默认 **0**、GUI 恢复 pending、移除标题调试；文本桥代码保留但禁用，等待 UI 线程安全窗口。
- 候选/已部署 `3ffd86e7b10a51a292972660f9a69f689dc164aa28f4eefd38788d2475eb244a`（28,457,628 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r15-*`。
- 记录：`MASTER_PLAN.md` §20.12 已更新。

### 2026-10-23 03:15 — R14：自动进图 + F4 打开市场；页签点击待校准

- 验证启动参数 **`-continuelastsave`**：直接加载最近存档进入地图，绕过“载入游戏”对话框。
- 验证 **F4**：`keybd_event(VK_F4=0x73, scancode 0x3E)` 可打开市场；`PostMessage` 无效。`pdx_settings.json` 记录 `GUI.scale=1.1`。
- 第五页签“物流”仍未点中：(585,232) 落到地图、(532,211) 展开过滤器；临时 debug 文本已全部移除，正式 GUI 干净。
- 候选/已部署 `e728cd5e6d13e5acf8d3c460e986df3bf79d120a5450efd283b7947ebbfd0e6b`（28,448,007 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r14-*`。
- 记录：`MASTER_PLAN.md` §20.11 已更新。

### 2026-10-23 02:55 — R13：原版市场面板自动化与“物流”页签实机确认

- 找到 `open_market` = F4（default.profile）与左侧 sidebar 贸易按钮；通过 sidebar 自动化打开市场面板，截图确认第五页签“物流”及显示/隐藏按钮存在（约 physical 590,230）。
- 曾加临时 GUI debug 文本（Exists 守卫 + `GetVariableSystem.Get(...)`）尝试验证 `.Get`；因面板关闭/点击坐标问题未真正切到“物流”内容，未得到结论；最终构建前已移除，正式 GUI 保持干净。
- 发现 `probe_write=1` 但 `probe_variables=0` 时写测试不执行（提前返回）；下轮两者同开或点 “Probe now”。F4 的 PostMessage 未触发，下轮用 SendInput。
- 候选/已部署 `a59fdbfc26e23837b5db8acd5537fa41a4b5c216ea0e5aebbf64a15cee035977`（28,448,007 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r13-*`。
- 记录：`MASTER_PLAN.md` §20.10 已更新。

### 2026-10-23 02:35 — R12：全局字符串映射验证 + Murmur3 + GUI 变量写入打通

- 只读探针读取同模板全局 map（5639 条），验证 0x48 条目布局；40/40 真实 key/hash 反推出 **MurmurHash3 x86_32 seed=0**。
- 新增写入实验 `WriteGuiVariableProbe`：签名校验 getter/insert/assign → Murmur3 → 已存在则引擎 `std::string::assign` 更新值，否则 assign 构造键值对并 insert（引擎接管字符串）。
- 实机：`ir_probe` 短键、`ironroad_probe_long_key` 长键均插入/更新成功；读回 `hash=dbf7bbbc value='long-value-2'`，map `count=1`，无崩溃。测试后关闭游戏，`probe_variables=0`、`probe_write=0`。
- 关键限制：原版 GUI 变量 API 只有 `Exists/Toggle/Set`，**无 `Get` 文本读取**；变量只能作标志，数值/文本需注册 GUI 数据函数（返回类型 ABI 未验证）。
- 候选/已部署 `05f54995d632fa1b458acf3b4f6a05c1bfbfe6c806b13954e784f234f3e9c43e`（28,448,007 B）；`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r12-*.log`。
- 记录：`MASTER_PLAN.md` §20.9 已更新。

### 2026-10-23 02:20 — R11：GUI 变量系统 getter-only 探针（实机验证对象/头部）

- 新增 `native/src/variable_probe.{h,cpp}`：校验 getter 入口签名 → 调用 → VirtualQuery 校验对象/数组/字符串 → 读 map 头部与 0x48 条目；**只读**。
- `offsets.h` 加入 `kRvaVariableSystem=0x3A92920` 及签名（并入 profile gate）；overlay 增加 `probe_variables` 设置、Diagnostics 只读探针 UI、每 2 秒轮询且仅在变化时记录。
- 构建/部署 `b171d15d9cabb8a532e71f471b069bf060b642792cb679aebc6e1304346120a7`（28,382,723 B）。
- 实机：profile ready（新签名通过）；日志 `GUI variable map obj=00007ff71c0dd800 buckets=1 count=0 level=0 load=0.90 entries=0`，与静态布局一致，无崩溃。测试后关闭游戏，`probe_variables` 恢复 0。
- 未取得条目：自动化加载 autosave 后未能可靠打开市场“物流”页签来创建 `ironroad_hide_flows`，条目布局/hash 仍未验证。
- 验证：`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r11-session.log`。
- 记录：`MASTER_PLAN.md` §20.8 已更新。

### 2026-10-23 02:50 — R10：GUI 变量系统静态解剖（只读，无代码改动）

- `GetVariableSystem`：注册包装 0x5AE6C0 → 注册助手 0x3AB6D00(name,0,callback) → 回调 0x3A98400 → 单例 getter 0x3A92920。
- 单例对象（RVA 0x59FD800，构造器 0x7B79D0）非多态；0x28 字节头部；本质 **string→string 哈希表**，条目 0x48：hash/占用/MsvcString 键(+0x08)/MsvcString 值(+0x28)。
- 已定位重哈希 0x79B4E0/0x79B480、insert-or-find 0x79C0D0（移动键值、不更新已有值）、字符串比较 helper。
- 未定位 hash 算法、更新已有值 setter、GUI 方法映射、线程亲和性。**未在 DLL 调用**；下一步只做 getter-only 探针。
- 证据：`E:\Code\v3-analysis\var-system-r10-notes.txt`。本轮无源码改动，测试沿用 R9 的 11 项 PASS；未部署/未实机。

### 2026-10-23 02:35 — R9：设置路径修复 + 迁移实机验证

- 发现：R8 实机中根目录 `ironroad.ini` 被忽略——`RuntimeDataDirectory()` 返回 `...\IronroadV3\data`，DLL 只读该子目录。
- 修正：`m_settingsPath` 规范为 `%LOCALAPPDATA%\IronroadV3\ironroad.ini`；`LoadSettings` 先根后 data 回退；`SaveSettings` 写根并建目录；`embedded_assets` 首次从 data/legacy 迁移且不覆盖已有根文件。
- 实机：部署新候选 `bef613d4907bddc570517a18219327a7302560c2c4772978d3ffddf3fe6b34bf`（28,123,676 B），删除根 ini 后启动；根 ini 自动生成、设置读取正确、`synthetic … restored (48 orders)`、GUI/ImGui 正常、无崩溃；随后关闭并恢复 `test_snapshot=0`。
- 验证：`build_tests` 11 项 PASS；`verify_embedded` PASS；证据 `native/build/r9-session.log`。
- 附带静态线索（未调用）：`GetVariableSystem` 注册包装 `FUN_143ab6d00(name,0,FUN_143a98400)`，回调返回 `FUN_143a92920()`（全局 VariableSystem，RVA 0x59FD800）。方法 ABI/线程/值类型未验证，暂不接入。
- 记录：`MASTER_PLAN.md` §20.6 已更新。

### 2026-10-23 02:15 — R8：首次实机验证（当前候选 + 调试货运视图）

- 部署当前候选 `c6f77995…9d7b70`（`deploy.ps1`，游戏关闭时），直接启动 `victoria3.exe -gdpr-compliant`。
- 启动日志确认：loader attached、profile 1.13.11 hash 匹配、GUI 归档 prepared/mounted、嵌入资源 MountData、native hooks installed、ImGui ready 2560x1440、`synthetic freight test snapshot restored (48 orders)`；无崩溃。
- 通过 `Insert` 打开调试面板，切到 UI prototype 后：日志出现 `freight view gen=2 world=1 rev=1 ribbons=58`、`resolved arrow type 'ironroadTrafficLimeW6'` 等宽度变体、`SUBMIT scope=0/1/0` 持续提交（结束前 153k 提交、71 次上传、605 cache hits）。截图可见地图上的多宽度彩色 freight ribbon，以及文档的 `TEST INPUT`、Freight 16.15M、Target 111.63k / Allocated 38.95k / Unmet 72.68k、热度参考 25000。
- 原版市场“物流”页签仍显示“实时货运数据尚未接通 / 货运量 -- / 已铺铁路 --”，符合预期（正式 UI 数据桥未接）。
- 测试期间误触原版 Continue，加载了 autosave（未修改/保存游戏）；结束时用 WM_CLOSE 正常退出。`test_snapshot` 已恢复为 0，部署仍为当前候选。
- 证据：`native/build/r8-freight1.png`、`r8-crop.png`、`r8-panel.png`、`r8-session.log`。
- 注意：这是**调试合成输入**的实机可见性，不是实时数据或正式原版 UI 验收；A13–A15、A18–A19、A25 仍需真实数据/正式界面。

### 2026-10-23 02:02 — R7：合成输入覆盖宽度档

- `native/src/dev_snapshot.cpp`：每个商品成对生产/消费（双方同数量档，供需平衡；奇数余量只作额外供给），八档数量覆盖固定宽度阈值。48-region 实测宽度档 0/2/3/4/5/6/7、58 ribbons、compute 4.18 ms。
- `dev_snapshot_tests` 新增宽度档断言；`freight_scale_tests` 的全铁路可分配 >90% 在配对下重新成立（48-region 99.5%，649-region 96.5%）。
- 规模（配对）：649 region 无铁路 248 shipments/655 ribbons/25.8 ms；全铁路 627/1801/271.8 ms；热视图 0 ms、hover ≤0.7 ms。
- 验证：`build_tests` 11 项 PASS；DLL `c6f7799570a279110b08d54d62b5ce3bbea845b32e05c5737e29f8f6b19d7b70`（27,962,872 B）；`verify_embedded` PASS。未部署、未实机。
- 风险：合成数量档不是游戏数据；649-region 271.8 ms 为合成上界；宽度档缺 W1（需要时调档）。
- 记录：`MASTER_PLAN.md` §20.4 已更新。

### 2026-10-23 01:50 — R6：区域 hub 回退 + 规模测量

- 发现：分配器固定用 `region*100`；排除未建铁路后 634 个城市 hub 中 238 个无可用边，州被误判不可达（48-region 合成仅 8/96 allocated）。
- 修正 `native/src/freight.cpp`：在 policy 允许的边（排除未建铁路/关闭边）上统计 `region*100+0..4` 活跃度，选最高度 hub（同度取低 id）；无 hub 保留未分配。回归：城市 hub 仅未建铁路时回退港口道路 hub；关闭道路后需求保留。
- 规模（合成）：649 region 无铁路 153 shipments/641 ribbons/17.6 ms/612·1296；全铁路 320/1472/105 ms/1280·1296；热视图 0 ms、hover ≤0.7 ms。无铁路活跃图 1229 组件、393/781 州有 hub 在 giant。
- 验证：`build_tests` 11 项 PASS；DLL `5bca95e0…aa5ddc`（27,935,041 B）；`verify_embedded` PASS。未部署、未实机。
- 风险：hub 选择是模型启发式；合成未满足比例不代表真实市场；105 ms 是上界样例。
- 下一步：实机自检、真实已建铁路复核、原版 UI ABI、经济读取窗研究。

### 2026-10-23 01:42 — R5：需求/分配/未满足统计 + 热度图例

- `FreightView`/`LogisticsDocument` 新增 `targetImport`/`allocatedImport`/`unmetDemand`（orders，市场或州作用域，不受过滤影响；unmet=max(target−allocated,0)）；线宽仍只用 allocated。
- 调试面板显示三项统计与“未满足不画成已运输”；热度图例显示 `heatReference` 与统计版本，无货运明确无尺度。
- 验证：10 项 PASS；DLL `d873648e…ace7bc9e`（27,933,520 B）；verify_embedded PASS。未部署、未实机。

### 2026-10-23 01:35 — R4：货运会话多 owner 仲裁

- 会话支持原生面板 10 / hover 20 / 调试 30；有效 owner 最高优先级；关闭 hover 恢复基础视图；关闭一个不清除其他；全部关闭才清空可见 frame，快照保留供重开。
- 验证：10 项 PASS；DLL `2d5812b1…4dd916e1`（27,921,596 B）。原生面板 owner 尚无 UI 调用；未部署、未实机。

### 2026-10-23 01:28 — R3：合成测试快照 + 货运 ribbon 提交路径

- `dev_snapshot.{h,cpp}` 生成确定性合成快照（3 商品、有 authored hub 的 region、市场 1），文档带 `synthetic`、面板红色 TEST INPUT；不进原版 UI。
- overlay `ApplyPendingFreight()` 将最新 `FreightFrame` 分帧上传/提交到共享原生池；切页/关闭停止提交；Diagnostics 增加 “Synthetic test snapshot (NOT live)” 开关。
- 验证：10 项 PASS；真实地图 24 ribbons（R6 后 39）/≈3 ms；DLL `403f6b12…e2008cdd`。未部署、未实机；与路线预览共享缓存需实机确认互不干扰。

### 2026-10-23 01:22 — R2：UI→货运文档会话 + overlay 接线 + 经济静态复核

- 新增 `LogisticsSession`（快照门控、worker 消费、文档发布、关闭取消、重开复用）；`FreightWorker::Reset()`；`BuildLogisticsDocument` 缓存视图重载。
- overlay 调试页接线并显示会话状态；新增 `logistics_session_tests`。
- 静态研究：`GetStateProduction/Consumption/Imports/Exports` 是脚本函数回调（脚本上下文 → 对象 id → state 容器 → 参数对象虚调用），不是简单 getter；不得据此 hook。私有笔记 `economy-r2-notes.txt`。
- 验证：9 项 PASS；DLL `971cf5c6…5e7e`。未部署、未实机。

### 2026-10-23 01:13 — R1：固定市场热度尺度 + 双层缓存

- 热度改为按完整市场快照固定尺度（hover/search 不改变颜色）；新增 `FreightEngine`（allocation + view/ribbon 缓存、内容指纹、world/revision 拒绝、ResetWorld）与 `FreightWorker`（LatestWorker）。
- `freight_engine_tests`；`freight_tests` 增补热度回归。
- 验证：8 项 PASS；DLL `45fce3c8…c51c402`。未部署、未实机。

### 2026-10-23 01:07 — R0：接管基线

- 完整重读 MASTER_PLAN 与四份原始文档；核对仓库文档/源码/PROFILE/构建部署脚本/本机环境。
- 基线 7 项测试 PASS；游戏 1.13.11、EXE `7f53…f811f`、候选/已部署 DLL `a99f…c3e74`、市场 GUI `1f68…ec3e`；无 Git。
