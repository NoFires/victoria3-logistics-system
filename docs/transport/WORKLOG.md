# 工作日志（按日期追加，不覆盖失败历史）

### 2026-10-24 — GitHub 公开上传准备：用户指定仓库名与 SSH

- 用户授权建立公开仓库 victoria3-logistics-system，并使用其 GitHub SSH；覆盖上一轮私有上传/命名建议。README 和上传说明同步名称；不修改内部 Ironroad 标识或产品行为。
- 本机 Git 2.49 可用、gh 未安装；默认及显式 ~/.ssh/id_rsa 连接 GitHub 均返回 Permission denied (publickey)，GitHub 未接受该公钥。未创建远端、未推送，需用户提供正确私钥路径或添加当前公钥并确认用户名。
- 修改前备份 E:/Code/v3-analysis/checkpoints/2026-10-24-github-public-upload；本轮只准备文档与本地 Git，不重建或部署 DLL，不操作游戏。许可与派生数据事项仍如实保留，不代选开源许可证。



### 2026-10-24 — GitHub 整理：完整源码索引与点子路线图

- 用户要求整理所有代码文件与点子以备上传 GitHub。推荐 Ironroad / ironroad-v3，未替用户创建远端、初始化 Git 或推送；保持原有目录与源码，避免破坏构建路径。
- 更新 README 至 R103 已知状态，清除早期未接实时数据/双DLL未实机等过时介绍；新增 docs/CODE_INDEX.md、IDEAS.md、GITHUB.md，汇总模块、全部文件、设计方向、上传范围及剩余许可事项。
- 完整开发快照排除 build/dist/缓存/日志，保留必需 data 与第三方代码；生成逐文件 SHA256 清单并校验 ZIP 一致性和文档链接。未改产品代码，无需重建或部署；产品状态和游戏操控保持原有约束。
- 备份与源码包：E:/Code/v3-analysis/checkpoints/2026-10-24-github-organization。公开前仍须作者选项目许可证、补齐第三方许可并核对派生数据分发范围；当前建议先私有上传。



### 2026-10-24 — R103：复制真实VFS替换/排除规则，推进改图实际资源源头

- 前轮R102为PROGRESS：修复源码/部署/用户实机确认均改变权威状态。继续完整目标，优先改图资源身份；保持手动操作限制，不等用户做重复动作。成功标准：精确登记/消费布局和比较语义独立汇编核对，规则/挂载同组自有复制、异常/变化回归通过，当前游戏只读复用同一解码器；不以此替代完整资源resolver。
- 按MASTER §2.4地图主题重读01/02/03约束延续，读取local-reverse-engineering技能；Ghidra确认victoria3.exe/x64/image140000000，EXE与分析副本SHA均7f53e326…。复用R95静态证据，重新调用登记03B4A8D0、pair追加028B2150、枚举03B4AF80；03B5BCA0未建Ghidra函数，改用本机PE/Capstone核对，未放松候选或改二进制。
- fs替换+48 ptr/+50capacity/+54count、stride40(root,path两MSVC字符串)，排除+60/+68/+6C stride20。Native VFS root/index定位不靠目录扫描猜测。03B4A8D0登记将反斜线换/，消费字符串比较不折叠大小写；替换按当前目录精确相等、取root在原生searchPaths中的最小index，保留owner和更高源。排除按原始字节前缀，没有路径分量边界。归档独立汇编与反编译，不把精确目录改成任意子路径prefix。
- 将DecodeNativeVfsMounts扩为DecodeNativeVfsConfiguration；真实searchPaths单独保留（仅归一化核对mountroot不能代替原生字节比较）。同组复制mounts/规则字符串并复读；仅自有数据，不调用VFS/getter、不持指针交worker。NativeVfsDirectoryRootCount为小型纯副本策略，实际物理资源枚举/前缀翻译尚未实现。
- 扩现有native_vfs_tests（已接统一入口），独立编译PASS。覆盖SSO/heap UTF-8、两表计数容量/地址边界、不可读与长度不符、表头/对象/heap/inline在复制中变化、退休与副本生命周期；策略覆盖嵌套/叠加、大小写/未知owner、真实search字节、邻名前缀/空排除。探针编译通过。
- 只读用户57776（23:07:08）两次PASS，24挂载、0替换、1排除(gui/gui_editor)，地图目录24源/排除0。用户此时已手动启动修复版；未输入/暂停/attach/写内存/启停Mod。原版无replacement活样本，需真实改图场景后续证明；不要求用户临时改配置。游戏仍运行，完整入口部署段本轮不跑；产品仍51ec975f，不重建未引用新头的DLL。
- 修改前备份/静态/探针输出与最终清单checkpoints/2026-10-24-r103-vfs-rules；native/build/vfs-round103.log。下一步有效目录/ZIP资源枚举与后台地图内容解析，生命周期/性能和其他总目标保留。同步R102用户“海路高度修复成功”至日志/总计划/矩阵，范围只覆盖报告故障。

### 2026-10-24 — R102补验收：用户确认海路高度修复成功

用户明确回复“海路高度修复成功”，将51ec975f构建的近景海路故障实机验收由待验改为通过；AGENTS/STATUS/MASTER/TASKS/ACCEPTANCE同步。没有自动操作游戏；长路径、接缝、其他地图等A10范围仍分别待验。本单元追加确认不触发重复构建/测试。

### 2026-10-24 — R102：拉近后海路消失，按实时水位修复

- 用户澄清拉近消失/拉远可见，并提供孟加拉湾两图；保持人类操控，无自动输入。按MASTER §2.4重读01/02/03全文，重点Geography and route fidelity、ABI分层、原生pass/缓存与高度规则：同图同路径、不在render改分配、测试层级不能互相替代。成功门槛：旧shader在正水位/负海床用例实际失败，修复覆盖海/岸/陆与平面过渡且坐标/UV不改，构建/资源/统一回归通过，随后用户手动复核。
- 只读查本机shader：正式building_flow.shader用max(GetHeight,0)，平面混合后+0.35；原版pdxborder.shader明确max(GetHeight,_WaterHeight)，jomini/jomini_water.fxh声明ConstantBuffer(JominiWater)含_WaterHeight。并非缩放筛选删除路线：当前渲染/几何路径无zoom过滤。截图与海床遮挡吻合，但最终游戏内证明仍待新构建。
- 修改前完整备份源shader/测试/脚本/文档和eed941两份DLL至2026-10-24-r102-water-height；用户两图与本机原版shader参照一并保留。正式shader最小两处修改：include水位定义、0下限换_WaterHeight。路径/宽度/纹理/深度测试/经济未改。
- 测试扩展现有flow_shader_tests.cpp：从真实shader提取VS Code，在WARP编译VS/GS，以stream-output直接读6个float位置/UV；4水位×4地形×4平面混合64例。旧shader实跑FAIL（water19 terrain-20 flat0：ribbon0.35 expected19.35），修复全部PASS。旧海床与半UV两个负对照均触发预期FAIL；其余8宽度/9色/双向箭头GPU测试通过。
- 测试接build_tests.ps1，首轮统一入口GPU失败于MSYS2 PATH选择无Pillow Python，与产品无关；固定调用进入脚本前Python解释器。保留tests-round102-first.log，修复后完整重跑34行PASS/0 FAIL，不把中断入口当通过。ResourcesOnly对象时效检查通过、嵌入资源/实际C++ GUI归档/布局/selftest560437亮像素/默认隐藏vtx0均PASS。
- 用户回复保存退出后确认0游戏进程，deploy.ps1部署51ec975f…/35151385B；引导71dae36d…/779879B不变，安装与dist两份hash一致，deployed-files.json记录。未自动启动/关闭/操作新会话，近景海运可见与游戏water CB绑定待用户手动复核。A10/P2-T02保持PARTIAL/DOING；不扩大为改图、性能或全目标通过。

### 2026-10-24 — R101：完成点整份副本交接、两处刷新缺口及人类接管

- 任务/约束：延续R100普通命令完成点研究，成功门槛为整份输入离开UI采集、暂停首次无额外命令取最终值、直接同档重载无变速/解暂停救活、同内容视图不闪。原始01/02/03线程/ownership/分层测试约束延用；不取得新模拟锁、不在render扫库、不传引擎指针给worker。备份2026-10-24-r101-snapshot-handoff含原源码/文档/DLL/ini/存档证据。
- 原生实现：033BD750精确签名，CD2BC1返回点、phase4/access2/hidden0/owns1/context1才借用原生范围，在原析构转发前复制53目录、全1147州订单与34铁路。native_snapshot.h邮箱及测试接build_tests，UI只drain自有副本并校验当前epoch/最新数据版本/当前身份/full anchor。
- 第一候选519864：完整入口部署通过。59996 native4896/UI20992，初始125ms给20,267,078.94；后续初始化命令微调订单而ready阻止再采，按1仍暂停取得94ms副本恢复20,267,075.76。增加resident回归，旧逻辑实际FAIL后修复PASS，UI Take保留请求、最新版本替换ready、旧成功/错误拒绝。暂停每命令复制，运行30s节流。限时脚本正常WM_CLOSE，未解暂停/保存。
- 第二候选269206：完整入口部署通过。47100 native24944/UI27620，暂停初始109/109/94ms副本已直接最终正确；市场195/州115路线可见，同内容沿用。直接重载却先按旧world1复制rev6–8，再由UI发现anchor更换退休到2并清掉副本，暂停没有下条命令而等待。保留269206-reload-race.log与等待截图，正常WM_CLOSE。
- 最终修复：native BindSource复制前识别machine/world/stateDb替换、CAS推进epoch；UI采用该epoch，Ensure只原子替换旧世界请求，保留已到的新world副本，避免Retire到Request间隙；UI变量与渲染均要求当前world。source-bind回归覆盖native先到、UI先到、迟到UIEnsure、旧源发布、源不可用、重复身份与机器替换。数字身份不等于可靠退休事件/ABA。
- 最终eed941…完整入口39行PASS/0 FAIL，资源/GUI实际归档/布局/GPU/selftest560468亮像素/default vertices0均通过，经deploy.ps1安装，两份安装=dist。52300启动22:24:53，native61820/UI35204，暂停初始rev4与直接重载world2 rev14均20,267,075.76；352203796先bind2，352204187 UI采用2，352204218接14，之后才打开面板。195池恢复/路线、停用提示消失、州433货运列115路线可见。
- 暂停按1确认speed0/paused1，同输入不撤正式视图。3轮hover/leave，0新采集/0null，9次汇总相同；第四轮前台guard拒绝输入，未计通过。用户随后要求切回人类操控，停止自动输入，无unpaused/paused/close脚本在运行；保留52300，不能再按此前自主授权操作/关闭。当前构建未做运行态更新测试。
- 交接时autosave SHA fecc16f7…/9176649B/mtime_ns1792768044907051700与save-before.json完全相同，ini SHA fb5be418…与原备份相同。证据eed941-handoff.log、eed941-handoff-files.json、eed941-hover-summary.json和截图；最终源码/构建清单另外归档。P4-T01/A24仍DOING/PARTIAL，可靠事件/ABA、实际建拆/市场km、改图/新设备/不同存档/性能继续，不把这个工作单元当全目标完成。

### 2026-10-24 — R100：模拟完成阶段诊断实现、完整入口与实机契约

- 上轮为进展：锁/并行yield证据改变采集方案。本轮先核实无游戏进程，R99/实际安装仍59e6d3d4…；按同一读取/线程主题延续已读原文，使用local-reverse-engineering。改源码前备份checkpoints/2026-10-24-r100-simulation-completion，另存安装两DLL/ini。
- 静态追到job wrapper VT0441A6A8，+8=03B292C0 try execute，+10=03B29270 wait state2，007C2540/00E78020/03B29440都先try后wait。不是+8单独join。StateTrades RTTI0549FAF0→COL049D6998→VT044E6F38→叶函数01339950→world+1C00/0133C0C0，嵌套013460B0/01346260显式等待；38个counter模板调用者含其他所有者，未据此宣称全州订单writer已枚举。独立PE反汇编与Ghidra明确program victoria3.exe，证据join/trade/command文件。
- native_access.h只复制观察标量，拒绝缺reader/TLS、所有字段坏读、未知模式、错应用type、world/DB缺失与地址溢出；不提供lease。新增测试接统一入口；初编译缺optional头，补后单测实际PASS。probe_access缺省false，加载/保存ini接线；Game::Ready全EXE哈希/新增签名前置，MinHook0139E220按Execute(void*)转发一次，原调用前后最多16条标量日志，没有采订单/保留对象/修改原版经济/发GUI或worker。未复用旧probe_economy钩子。
- 候选d590417caec94cddc077e0ca6fe209e7fffc57e2b245ce25533d81c41f9fe02a，34751412B。完整build→build_tests→embedded→GUI→GUI-runtime→GPU→selftest560432亮像素→默认隐藏vtx0→deploy全部退出0，汇总38行PASS/0 FAIL；native/build/*-round100.log及checkpoint副本。两个DLL安装=dist，原71dae引导不变。源码/hash列表tested-build.json。
- 临时配置probe_access1后正常EXE -continuelastsave自主56932（18:33:32）。加载期出现既有interface unavailable，之后暂停1836-01-01/原生date59883360，UI线程38656，独立外部只读probe确认speed按1键改索引0、paused1。复制已验证有界脚本，9×5秒截图、finally只按该PID/启动时间发WM_CLOSE，不强杀。
- 本PID日志按loader marker截取d590-session-only.log，恰好16个样本0..15，没有unreadable；模拟线程45624，与UI线程不同。每条前后access2/hidden0/owns1/context1、date差6、machine/world/DB相同。运行够16步后日志不再增加该类样本。样本是回调内顺序读，不能单独当完整快照一致性证明；本轮未在钩子复制经济。暂停基线25396订单，运行后25395订单/43商品接受，整组109/93ms；owner0，不作正式UI视觉验收。
- normalClose=true、problem空。autosave SHAfecc16f78294e294eda3211fdec9e9b89c2fd325685cd20992fd0a2a8398e804/9176649B/mtime before=after；原ini以临时文件原子替换逐字节恢复，缺省probe_accessfalse，当前无victoria3。live-report.json记录16条断言、退出、配置/存档恢复和安装hash。
- 下一步从普通命令完成阶段解决初始暂停：00CD2810 stage12 call007E1B60 atCD2B3D/returnCD2B42，scope dtor callCD2BBC/returnCD2BC1；目前只静态候选。需验证全部输入写任务/加载边界，设计只交自有副本的world/revision邮箱后再切生产采集，P4-T01与可靠world事件均未完成。R97闪烁修复和R98安全复制仍保持。

### 2026-10-24 — R99：原版访问范围与模拟完成阶段定位

- 延续P4读取/线程主题，用local-reverse-engineering skill复核当前EXE SHA7f53e326…、Ghidra显式program victoria3.exe；未改产品源码/DLL。修改前备份checkpoints/2026-10-24-r99-access-window，追加文档前另存BLOCKERS。完整静态伪代码与关键函数独立汇编分别access-static.txt/access-assembly.txt。
- GameStateAccess0514E580/Hidden0514E570、TLS[0]+5AE0/+92C、033BD460/640/750/820/8F0链与锁0513E058→0513DCA0→root+590确认。rootVT04430800的F0/F8均0061C140 no-op。00E765E0并行批处理期间主动unlock/relock，007C2C00临时HiddenAllowed；仅窗口线程、单一锁或复读不能作为原子证明。不手工套用未验证ABI/不在Draw扫描。
- 排除误识别：007D5740是startup，020808C0是Draw，00652750是析构，00662480是进入地图编辑器。主VT044365F0的相邻后一张secondary并非本命令；007D7940实际写对象+18=04436590→执行0139E220→007E0D80→007C8BD0。相邻FC3020是暂停相关命令，不能沿它误判tick。原scope-calls.json可导航但access-writers.json含错边界ADC伪命中，不作writer证据。下一步证明真实完成阶段与线程、全部写任务join及暂停/加载采集通道。
- 自主49800（17:44:11），暂停300样本22个1/own1、278个3/own0。私有只读探针纠正虚表枚举为10槽（原16槽越入相邻字符串，后6槽不当方法证据），加入pause/speed/date输出。按1键将UI速度设为最低，原生speed索引0、paused1确认后，有界脚本运行并在finally正常WM_CLOSE。2200样本access1/2/3共199/29/1972、hidden1共30、context0，原生date59883360→59883462。各字段顺序读，非原子观测；只开PROCESS_VM_READ/查询权限，不attach/暂停/写内存/调用原生getter。
- 脚本9张间隔截图，start336736250/finish336784875、normalClose=true、无error；unpaused-actions.json。freight区间19行无WARN/ERROR，接受1147州/25394订单/43商品（copy47ms,total109ms）；owner0，没有本轮正式卡片/ribbon视觉结论。窗口右侧有其他应用覆盖，未操作该应用，不将截图用于无覆盖UI验收。产品仍R98 59e6d3d4…，R97/R98已有正式UI证据单独保留。
- 原autosave SHAfecc16f7…、9176649B及mtime逐值未变，before/after JSON与副本在checkpoint；当前无victoria3。新私有有界脚本生成时曾出现Python反斜杠控制字符，执行前检查并修正为raw字符串，未执行错误脚本；末尾证据提取的括号语法错修正后成功，未丢失失败历史。不重复无源码变化的构建/回归。P4-T01保持DOING，未宣称原子/性能/最终验收完成。

### 2026-10-24 — R96候选：实际铁路状态与一对多样条身份

- 依MASTER §2.4完整重读01/02/03，重点Geography and route fidelity、Architecture and ownership、Test in layers、What success means；沿用local-reverse-engineering skill，精确EXE hash7f53e326…与Ghidra目标确认。门槛：原版铁路判定语义→运行类型→native连接键→物理strip映射一致，复制/异常/变化拒绝，UI线程采集而非renderer/worker读engine。
- 静态CTravelNode1315FF0通过铁路建筑可用写+19；1316210重建邻居计数+1B，不是hasRail。12A0910按定义类型1、端点可用/铁路/同市场等返回当前类型4，13B2110更新连接+20。native64位key定义+50；源links保持一对多，不按第一个候选/字符串猜ID。17287D0说明travel坐标floor到整数后×100000；实机两处Z整数边界源浮点精度在反变换中丢一ULP，记录量化比对边界，不冒充完整geometry fingerprint。
- 自主61752暂停原存档只读：4399nodes/6112connections，59节点铁路、34当前铁路连接；新生产解码器2329潜在全部绑定、34物理built边，771.864332全世界地图单位。私有第一版把native路径点当stripID打印作废，随后按stride18记录固定点；首版小于.001直接浮点比对不成立，静态确认floor后修正，不放宽成任意形状匹配。
- 格式3追加nativeConnections，4273物理样条全部且只归属一个源连接（4271links，两条连接各含两段），旧几何字节完全相同。原生loader校验成员/连通/端点/尾部与异常清空；Python未知/重复/断开/端点/未绑定用例接统一入口。新增native_rail_tests覆盖当前built状态、复制过期与错误图。
- 候选1666187d…已构建，资源/GUI结构/实际归档PASS；完整原生入口首轮新测试缺windows.h，补后目标PASS并重跑。安装仍8115373d…，未部署。备份2026-10-24-r96-built-rail包含原安装与修改前源码/数据；游戏已正常退出，未保存/推进日期。详细当前进度见STATUS R96。

### 2026-10-24 — R95：冻结VFS挂载表复制与Mod替换规则定位

- 主题P3地图/ABI；依MASTER §2.4完整重读01/02/03（同一连续主题前轮已读），重点Geography and route fidelity、Native discovery、Architecture and ownership、Test in layers。沿用local-reverse-engineering skill，Ghidra+独立PE/Capstone复核；不调用未证getter、不在render扫库、不交worker引擎指针。最小门槛：实际窗口线程context、冻结mount链、后端/搜索顺序与异常拒绝均可重现；不替代整项地图兼容。
- native_vfs.h纯复制解码；native_vfs_tests接统一入口，完整入口退出0（tests-round95.log，25行PASS）。live_vfs_probe实机复用同一头，TEB/TLS当前context0、24项挂载顺序，两次PASS。新增代码未接DLL，产品hash未变。
- 静态补定位fs+48路径替换表与fs+60排除规则；当前头未复制这些，未启用资源resolver/改图路线。地址及边界见PROFILE R95/STATUS R95；私有完整反编译及汇编存checkpoint，不分发原版代码。
- 新会话48816自主启动、暂停同一存档，正常WM_CLOSE关闭，未推进/保存。旧59720已不在，不推断退出原因。最初私有SplineEdge打印错用5×u32/f32/u32，未改源码/数据；按route_data.h校正6×u32+f32后8条相连边全rail，当前缺少builtRailEdges解释该州零分配，真实铁路仍待实现。

### 2026-10-24 — R94：外国州与跨市场实机证据（8115未改源码）

- 沿用暂停测试会话59720/world2、主体8115373d…（源码构建=dist=安装），日期始终1836-01-01，未保存。由不列颠市场点击地图进入法兰西奥尔良－贝里（运行州455）本地价格，原版价格/供需随州切换，地区货运列显示合法0；当前状态文档target234.08/allocated0/unmet234.08、市场总工作量1,862,020.64。其可达性与未分配原因未核对，不能据此判模型正确或错误，需结合图和余额继续诊断。
- 原版州概览的市场箭头进入法兰西市场，外部生产解码器明确panelMarket=2。市场卡片1,862,020.64，粮食203,892.12/11.0%；中文煤炭筛选卡片157,972.82，煤行8.5%（分母仍全部法国市场），原版价格33、结余-94；账本10909.89/2373.76/8536.13在过滤时保持法国作用域。法国地图ribbon由76变58（筛选），英国180条被撤下。
- 点击原版outliner不列颠市场回到panelMarket=0；原版会清搜索，卡片恢复17,857,680.87、粮食3,107,017.07/17.4%、煤390,633.01/2.2%，账本30892.46/11459.90/19432.56，恢复180ribbon；未残留法国单煤筛选数值。列表/市场切换仍由原版动作驱动，未修改订单/经济或国家控制。
- 证据checkpoints/2026-10-24-r94-cross-market/{france-state,france-market2,france-coal,britain-market0-return}.png与8115-cross-market.log；只读全州解码仍1147/1147、305市场。该工作单元为现有构建交互取证，没有新增源码测试或DLL构建。按MASTER §2.4重读01/02/03全文后转入实际资源/地图主题，目标不变：当前生效资源和图身份必须支持修改地图Mod，不能以嵌入原版图替代。


### 2026-10-24 — R93c：恢复实机PASS；设备边界补强候选

- 原型39c18330…（PID6248，已正常退出）：第一次开市场180箭头；菜单→Continue检测world1→2，新真实快照就绪后记录native pool recovered world=2 slots=180，箭头可见/停用提示消失、卡片17,857,680.87不变、uploads仍180。随后游戏内Load autosave检测world2→3，180箭头再次恢复，uploads仍180、cache360。first-world.png、world2-recovered.png、world3-direct-reload.png与39c-world1-to3.log已归档。
- 同一生产lease验证器由外部PROCESS_VM_READ探针在游戏/菜单/重载后的92类型/180对象上全部通过，未attach/暂停/写进程；文件production-leases-{first,menu,world2}.txt。该层验证复制解码，真正恢复由DLL render入口完成。未构造/析构注册对象，旧请求清空，world标签不允许旧恢复覆盖新失效。
- 审查发现同世界的设备变化也必须拦截已有缓存。候选新增每个选定pass检查native graphics身份、设备更换/移除；swapchain重建在旧设备仍被引用时比对新设备，避免释放后COM地址复用。相同设备可走原生池重新校验，新设备重建协议未启用。停用文案改为通用资源未就绪，不再把设备失效描述为读档故障。
- 当前主体8115373d80edf9a5098e295bde488a5cd270c5454f1eb4e929d186879e166c36（33,718,124 B），引导71dae36d…；deploy.ps1已安装，两个安装DLL hash=dist。build/relink/overlay-round93c.log、完整tests-round93c.log（新增lease异常/过期回归）、GUI归档/资源、自检560418亮像素、默认隐藏vtx0均PASS。自主会话59720（03:51:14启动）暂停1836-01-01，未保存。


- 最终8115构建实机：最小化→恢复，真实客户区2560×1440→1920×1080→2560×1440，全部保留180箭头及17,857,680.87卡片值、uploads180；窗口原位置/尺寸已恢复，window-sizes.json记录实际尺寸，window-{before,unminimized,resized,final}.png为可见证据。它不是设备移除测试。
- 最终8115菜单往返：world失效日志285302515，恢复285565171（world2、180slots），正式箭头重新可见、停用提示消失、uploads180/cache180；失效→恢复区间无SUBMIT汇总，代码在每个原生提交前再次检查world。8115-world2-recovered.png、8115-world2-live.log、8115-production-leases-world2.txt为证据；生产解码器92类型/180对象全通过，error.log相关GUI命中0。不要以“无汇总”单独证明逐调用安全，实际门控/会话反例及可见恢复是相互补充的证据。
- A24仍PARTIAL：已完成同设备同存档菜单/直接载入恢复及窗口操作；不同存档、同地址anchor复用的可靠世界事件、真实设备移除/新设备重建、资产替换与改图仍待完成。永久“需要重启”的方案已被上述受校验恢复替代，未证设备/资源路径继续停用。当前测试进程59720仍暂停运行，下一轮可继续验证其他市场。

### 2026-10-24 — R93：同设备原生池恢复候选（实机待验）

- 依MASTER §2.4完整重读01/02/03，复核Architecture/ownership、CArrow注册、What success means、Lifecycle与分层验收。约束：不调用析构/重构造、渲染不扫州库/等待模拟锁；恢复只能复用经当前注册与资源身份校验的池。最小门槛：菜单/重载后新world实际箭头再可见，旧world请求不能提交，完整测试与异常读取拒绝通过。
- R92实机已复现永久停用。候选将bool改为nativePoolWorld对economyWorld比较；旧验证即使晚返回，也不能让更新世界就绪。RecoverNativeWorld只在真实当前frame和正确原生pass上运行，校验同一D3D设备/原生graphics、资源工厂精确style→type、类型回调成员、arrow/path/points/VB/CB和当前world，清旧活跃批后才标记该world。host类型缓存清空再解析，不碰注册对象的构造/析构。
- 新arrow_lease.h复制式验证器与arrow_lease_tests：类型删除/替换、注册缺失、错误类型、路径/点数组/VB/CB替换、计数/整数边界、allocator、graphics更换、读取失效、复制期间变化及world过期均拒绝；接build_tests.ps1。世代闸门改动的竞态边界靠精确world标签而非store(false)。
- UI离开ingame phase立即失效；加载期不采集快照；同ID州对象指针变化也使旧anchor失效。恢复完成由UI定时器重新发布map_status。设备/资产替换尚无重建协议，校验失败继续显示停用状态，不宣称全生命周期完成。
- 候选主体39c183301c320774735e1af87cd43b3da91d8a25d5c64290517df6464b6b465a，33,714,028 B；引导71dae36d…不变。完整构建/统一原生入口（含新lease测试）、GUI归档/资源、自检560447亮像素通过；debug默认隐藏检查运行中。当前安装仍R92 ccecb9ee…；未启动游戏，实机恢复待验。日志native/build/{build,overlay,tests,gui,selftest,debug}-round93.log；备份2026-10-24-r93-world-recovery。


### 2026-10-24 — R92：生命周期只读实证；地图停用状态独立显示

- R91暂停会话54564正常返回菜单（未勾选保存），再Continue同存档；world1→2、1147州/305市场重采集及模型卡片恢复。外部只读探针按当前asset map枚举ironroad类型及注册项：前/菜单/重载三份输出逐字节一致，92类型、371对象、invalid0，path/VB/CB地址不变。该证据只覆盖同设备/同资产/同存档，不能自动证明设备重建或asset替换安全。
- Ghidra+独立Capstone：构造037FD4D0在type+58锁内向type+38的stride48回调容器注册arrow；回调0384D970重建几何/VB，0384D9E0复制捕获，类型析构037FE9C0释放回调与资源。箭头析构026D0B40未见注销该容器；因此仍禁止直接析构/重构造，永久retired gate尚未解除。完整静态/实机边界及恢复前提见私有checkpoint README。
- R91把状态改成本地化后，旧scope中的地图停用解释不再显示。R92补ironroad_map_status_v1（UI线程写长字符串，未改字符串ABI），原版物流页在retired时显示独立中英说明；正常世界不占行高。模型数据与原版经济行为不变。GUI结构回归与C++/Python实际归档通过。
- 主体ccecb9ee356fd7c6d0edcddc53a2a9016c6d2f6d78b93e46c4379aa703d8d1bb，33,044,323 B；引导71dae36d…不变。完整构建/原生统一入口、GUI归档/资源、自检560387亮像素、默认隐藏vtx0均PASS；native/build/{build,tests,gui,selftest,debug,deploy}-round92.log。deploy.ps1安装后两份DLL hash均=dist。
- 为中英文原版搜索验收，备份pdx_settings.json后仅临时把language设l_english；自主启动5636、暂停读同存档。英文CoAl搜索实机PASS：原版匹配5个商品，当前卡片537424.87=煤390633.01+工具95007.61+肥料51784.25，其余两行0；煤炭占比仍2.2%，账本仍30892.46/11459.90/19432.56，英文标题和数值完整可见。再次菜单→Continue后停用提示完整显示，模型卡片恢复，箭头保持安全停用；error.log相关GUI命中0。截图english-coal.png/english-reload-warning.png及r92-english-live.log已归档。测试会话5636已正常退出，未推进日期/保存。游戏退出时会省略默认英语language键；核对全部其他设置相同后恢复原文件，pdx_settings.json与备份逐字节一致，中文已恢复。未修改playset/Mod启用状态。R91旧测试会话54564已正常WM_CLOSE退出。
- 剩余：A24正式地图恢复、异存档/设备/改图资产替换、真实铁路与完整性能，仍未交付；不能以这次地址存活证据宣布完整生命周期通过。


### 2026-10-24 — R91：需求账布局修复；R90实机交互验收

- R90自主暂停会话50904：真实市场0的工作量17,857,680.87；粮食3,107,017.07/17.4%，煤390,633.01/2.2%。工业品筛选卡片2,722,625.91，中文“煤炭”单行390,633.01，空搜索0.00；商品占比分母始终全市场。全市场订单目标/已分配/未满足30892.46/11459.90/19432.56不因筛选改变。
- hover由180条基础ribbon切为136条粮食ribbon；卡片不变；隐藏撤下货流、显示恢复。点击煤炭进入原版详情，右键菜单正常（未触发任何经济操作）；返回/关闭撤下旧owner。西南诸郡→伦敦周边（州5）时地区列与路线换为新州，11.6k/896.9k/2.4M等无填充空格省略号。所有测试均暂停1836-01-01，未保存；正常WM_CLOSE退出，仅操作本次自主测试进程。
- R90截图及原始日志：checkpoints/2026-10-24-r90-native-markets/{market0-positive,market0-grain-hover,market0-hidden,market0-industrial-filter,market0-chinese-coal-query,market0-empty-query,native-coal-details,native-coal-rightclick,southwest-local-prices,state-switch}.png、r90-live-freight.log。旧52分组实机“市场隔离”结论仍作废。
- 发现底部英文前缀加数值被截断；R91改为独立本地化标题与数值行，保留订单和Freight模型单位分账；模型/等待状态依据现有文档上下文门显示本地化文案。仅修改自有GUI及本地化，不改engine字符串ABI、模型或价格。GUI结构检查更新覆盖该回归。
- R91主体10840747cb7c2a38258f6b202731a228c7ca38bbc15c9b70c7da4a4fb6bf765b，33,039,715 B；引导71dae36d…不变。build.ps1 -ResourcesOnly、完整build_tests.ps1、GUI结构/实际C++归档、verify_embedded、selftest（560420亮像素）、默认隐藏vtx=0均PASS。日志native/build/{build,tests,gui,selftest,debug,deploy}-round91.log。无游戏后deploy.ps1安装，两个安装DLL hash=dist；-continuelastsave自主启动54564，最终可读性实机PASS（localized-ledger-final.png）。
- 实机追加：中文“模型分配”、两组标题和全部三项值完整可见。已核验20次关闭→重开（日志280555281..280666734共20次closed和20次selected）；两轮10次私有内存分别14662627328→14677229568、14662254592→14642966528 B，未持续增长，累计几何上传保持371。CSV cycles10-verified-{a,b}.csv与r91-live-freight.log为证据。更早的快速输入轮未实际完成全部切换，作废，不计验收；调整为移动后150ms、点击后900ms再实测。此结果不代替三轮60秒P95/模拟速度门槛。
- 此处仅完成原版数据/UI局部闭环；另一市场、跨world恢复、改图、真实已建铁路、完整性能及最终A01–A27仍未全部通过。


### 2026-10-24 — R90：真实原版市场身份已接采集（新实机验收中）

- 静态原版State.GetMarket链与RTTI/独立Capstone复核：普通州经owner country取得market，特殊州经MarketArea取得market，再验证原生CMarket。GUI包装保存同一个原生market ID；不需要猜GUI与快照映射。
- 只读实机：1147/1147州生产解码器通过（0失败，3特殊分支）；中英格兰433/owner1→market0，与不列颠市场相符。旧+0x48分组52而真实市场305；1143州数值不同，35个旧分组跨多个真实市场。旧分配的“市场隔离”实机结论作废，保留历史不扩展。
- 新state_market.h复制并复核对象类型、完整typed ID代数、数据库边界、owner/area分支和复制期间变化；采集失败撤回正式数据。新增state_market_tests接统一入口，覆盖零ID、未知、越界、类型错、回收handle和变更，独立测试PASS；生产解码器只读全州PASS。
- R89现场稳定性：西南诸郡工作量可见，多个30秒刷新均重新获得owner15/frame1，原生40ribbon的uploads累计保持40（>60秒）；这只验证异步/几何复用，不证明旧市场分组正确或完整性能门槛。紧凑数字的padding省略号已改为先安全heap写入再对现有entry赋未填充短值。
- 候选主体93b6ec29e9a0114771c9245293a6b0bbaea286704485e2292ce8858cd64d985d（33,038,179 B），资源/实际GUI归档PASS；完整入口含新增州市场校验全部PASS；经deploy.ps1部署，安装=dist；自主创建的暂停会话55652已正常退出，未推进日期/保存。备份2026-10-24-r90-native-markets，all-state-market-live.txt/production-decoder-live.txt/static-assembly.txt为私有证据。



### 2026-10-24 — R89：异步快照不丢失与地区紧凑数字（实机复测中）

- R88自主启动R87已实机发布州433工作量和69条原生ribbon，截图/日志在checkpoints/2026-10-24-r88-phase1-scope；但周期刷新后请求合并会丢掉快照，故不能判稳定通过。
- 新worker回归先失败（filter coalescing must retain newest pending snapshot）。Request现保留最新不可变快照，后续过滤任务携带；Reset清除，worker按shared_ptr身份避免重复复制/指纹扫描。复现与既有reset/cancel/缓存/会话测试通过。
- 大数货运列挤到价差：地区行采用k/M一位小数并加原版最小字体省略保护，市场数值精度及模型数据保持。新增格式回归先失败再通过。
- 完整构建/统一原生入口/GUI归档/资源检查PASS。用户授权自主实机，旧自动输入停止已撤销；测试会话始终暂停、日期未变，正常WM_CLOSE退出后经deploy.ps1安装主体c2b457d032d099d0a9d62e6f2fb9c86a23688c349b16b4fe8c06630b6de72465（32,968,986 B），安装=dist。自主重启pid55652，复测中。备份2026-10-24-r89-snapshot-mailbox。
- 市场身份静态发现：State.GetMarket实际沿所有者/特殊市场分支进入市场数据库，再用实际市场id取得GUI包装；旧+0x48聚类不是该getter的取值链。保持市场闸门，下一步只读复核真实链与分组。



### 2026-10-24 — R88：无人值守与一期边界同步

用户授权自主启动游戏并测试。R87完整原生/加载部署/运行目录入口零失败（tests-round87-full.log）；正常EXE入口-continuelastsave启动pid29272，待加载。Freight一期只保留revision 1需求账，满足量来自最终货流，不另建供给池/游戏商品；旧独立供给阻塞撤销，端到端验收仍未完成。Ghidra当前可用，静态EXE与安装哈希一致；市场resolver确认是惰性GUI包装注册表，禁止主动调用持锁/分配函数。



### 2026-10-24 — R87：实机地区链路已触发；共享枢纽分解修复已部署

- 部署后自检：selftest-round87.log PASS（560397亮像素）；debug-round87.log PASS（vtx=0）。这些为独立harness验证，不代表新DLL已在游戏中发布正值。

- R86实机pid52220已加载d78dda6e…；识别州433/local_goods与关闭切页，但分配报cannot decompose allocated flow，尚无正式文档，截图货运--正确，不算数值通过。
- 新增同枢纽多收货州用例复现同错误；分解索引改为每节点保留全部收货terminal，耗尽后推进。覆盖共用收/发枢纽、短缺与乱序；分配/缓存/会话回归通过。
- 实机明确拒绝诊断widget的on_start；删两控件并加回归，GUI及归档/资源校验PASS。本次主体2a56d04a3375d97f6a1476f8db76f235b8dfef536cfa9bd9611bab37bc1c1866（32,946,396 B），用户保存退出后经deploy.ps1部署，两DLL安装=dist；新构建未实机启动。
- 用户已明确保存退出，确认无进程后部署；下一步手动启动验收。备份2026-10-24-r87-shared-hub，私有日志/截图及regression.txt；真实地图合成尺度回归PASS（781州、1801条ribbon上限场景）；首次手工编译尺度测试漏链dev_snapshot.cpp，补齐后通过。后续重新进存档验正值/切州/关闭，市场身份桥不变。



### 2026-10-24 — R86：地区货运后端首次接入运行时（实机待验）

- 需求：开始实现后端。按P4/P5重读01/02/03的所有权、数值副本、取消/缓存和分层验收约束；优先完成州ID已验证的地区链路。市场GUI ID与快照市场ID映射继续未决。
- 已接线：接受快照时构建NativeStateRoster；UI定时器解码州面板并仲裁NativeStateOwner；状态文档按州/商品格式化工作量；UI线程先关闭ironroad_state_rows_id_v1门，再写行，复核owner/world/revision/source/frame/州ID后揭示。更换快照、元数据失败、切州、关闭、读档撤销旧结果。render只访问session数值副本，不读UI owner或引擎州数据库。
- 纠正旧报告：地区链路使用运行州ID→同快照market，不需要GUI市场ID桥；R84c所称“同一个闸门”不是当前NativeStateOwner代码的必要条件。新增反例测试：GUI market=0、快照market=7时市场owner拒绝，地区owner仍能选州30并发布模型结果。
- 同时修复采集诊断的真实越界：512B raw原被按128个qword拷贝，编译器报告读取1024B；指针槽数改由缓冲区dword数推导为64。修后overlay编译无警告。未删除原有探针。
- 验证：build_tests.ps1全部通过（含扩展州不可读/失效与独立市场测试）、完整构建、GUI回归、实际C++/Python归档、嵌入资源检查通过；D3D11自检PASS（560397亮像素）、默认隐藏PASS（vtx=0），见selftest-round86.log/debug-round86.log。离线不证明游戏内正值或生命周期全部验收。
- 已部署主体d78dda6e2a99aec46d6c60235d50e04e15f78397faca59f107f7326cddbf0360（32,939,385 B），引导71dae36d…；经deploy.ps1安装，两DLL安装=dist。未启动/操作游戏。回滚checkpoint：2026-10-24-r86-state-backend（含R85 DLL）。证据native/build/{build,relink,overlay,tests,deploy}-round86.log。
- 下一步用户手动启动进入存档，打开州→本地价格，核对对应州货运列、切州/关闭、错误日志及owner=15。市场身份桥、实际改图资源/Freight独立供给/真实铁路/跨world原生寿命/性能仍未闭合。



### 2026-10-24 — R85：截图布局修复与市场商品列头

- 州表计入继承的55px商品图标，固定价格单元格边界；表头/行统一55|50|50|65|90|90|80|60=540。短标题本地价/市场价/价差保留完整提示，货运60。
- 市场表55|210|85|95|95=540；结余文本固定85×55居中，补齐分隔线和原版商品图标列头，商品/价格/结余使用原版排序。
- 构建、GUI回归、C++/Python归档一致与嵌入资源检查PASS；主体e72464e89b5662dae9d3fa628771958fbbb3b5e4b68e1a20e52979a2d693e5ca（32,340,108 B），引导71dae36d…；经deploy.ps1部署，安装=dist。实机视觉尚未验证，用户手动操作。
- 备份checkpoints/2026-10-24-r85-ui-layout（保留介入时R84p源码与3109a87f…主体）；证据native/build/{build,gui,deploy}-round85.log。新增宽度回归在修改前复现615px溢出，修改后PASS。



## 2026-10-24 — R84n/R84o：市场页列序定稿（结余前移、图标列头、分隔线）与州面板表头占满

1. **用户反馈**：市场页顺序应为 `市场价格｜结余｜工作量｜占比`；列头要占满整列；列之间要有分隔线；结余右侧空白要去掉；州面板列头也要占满列；**结余用游戏自身的图标，不要本地化键**。
2. **市场页结构**：原版 `goods_entry_button` 里结余列位于 `last_column` 之后，无法直接前移。方案：把结余列与两列依赖度一起在**我们的页签隐藏**（`visible = "[Not(InformationPanel.IsTabSelected('ironroad_logistics'))]"`），再由我们的片段在 `last_column` 内、工作量之前自己渲染结余数值——复用原版脚本函数 `Goods.GetMarketBuyAndSellOrdersBalance` 与原版三个格式键（GOLD／PLAIN／BLUE），不新增本地化文本。
3. **结余列头**：改用原版同款图标按钮（`sort_button` + `sort_button_icons/sort_balance.dds`，25×25，`alwaystransparent`，tooltip `SORT_BY_BALANCE_TOOLTIP`）。此前试用文字键 `MARKET_PANEL_BALANCE` 渲染为空白，因为该键是**数值格式键**（`[FixedPointToInt(Goods.GetMarketBuyAndSellOrdersBalance)|=Dv]`），不是标签；临时加的 `IRONROAD_BALANCE` 已撤回。
4. **分隔线与列宽**：表头与数据行在列间插入 `vertical_divider = {}`；市场页表头 `55|210|90|95|95`、行 `210|80|90|95|95`。
5. **州面板**：表头与行按分组总和相等重排为 `156|276|48`（表头 `39×4|96,96,84|48`，行 `48,48,60|96,96,84|48`，均 480）。价格列回到 96/96/84，三个价格表头不再省略；货运列 48。
6. **验证**：完整 build → 主体 `d232efff…`（32,277,114 B）；无游戏进程时部署，安装＝dist；完整测试入口零失败（`native/build/entry-round84o.log`）；GUI 归档逐字一致、`verify_embedded` PASS；实机启动后 `error.log` 无 `ironroad`／属性类报错。

## 2026-10-24 — R84l/R84m：列头对齐、市场页只留结余、州字段结构验证

1. **用户反馈**：州面板“列头和列没有对齐”，建议用“本地价格／市场价格”的占位来对齐；市场页“不需要销售订单，只需要看结余，结余也没列头”。
2. **对齐方案（按分组总和相等）**：量出原版行 480 px。改为
   - 行：`52+52+78`（图标/产量/消耗组）+ `80+80+78`（本地价格/市场价格/价格差异）+ `60`（货运）= 480
   - 表头：`40+40+40+62`（四个图标排序按钮）+ `80+80+78`（三个价格列头）+ `60`（货运）= 480
   逐段相等，因此列头会正对各自的列。Python 正则单趟替换、C++ 顺序替换（80→78 在 100→80 之前）保证两者输出逐字一致。
3. **市场页**：行内 `last_column` 之后依次是结余列与两列“依赖度”；给后两列容器插 `visible = "[Not(InformationPanel.IsTabSelected('ironroad_logistics'))]"`，只在原版页签显示；表头行末补结余列头（复用 `MARKET_PANEL_BALANCE`／`MARKET_GOODS_BALANCE_TOOLTIP`）。
4. **州字段结构诊断（关键）**：`state field probe` 一次性采集 1147 个州对象的 128 个 dword，按 `state+0x48` 分组检验“组内恒定、组间唯一”。实机输出 `candidates=1`，唯一命中就是 `+0x048`（52 组，值域 31..116）。**结论：州对象里没有第二套市场 id**，`state+0x48` 是结构上唯一成立的州→市场键；缺口在面板侧的 GUI 包装 id。
5. **构建／部署／验证**：完整 build → 主体 `20927cbf…`（32,276,090 B）；无游戏进程时 `deploy.ps1` 安装，安装＝dist；完整测试入口零失败（`native/build/entry-round84l.log`）；实机启动后采集到上述诊断结果。

## 2026-10-24 — R84k：市场页只保留结余列 + 州对象字段结构诊断

1. **用户反馈（截图）**：市场页行内 `+212` 之后还有一列数字（`11.6…`），并且“结余”没有列头。要求：不要销售订单／依赖度列，只保留结余，并给结余加列头。
2. **行结构定位**：原版 `type goods_entry_button` 里，`last_column` 之后依次是 `vertical_divider`、**结余列**（`MARKET_GOODS_BALANCE_TOOLTIP`，`minimumsize { 90 52 }`）、`vertical_divider`、**进口依赖度列**、`vertical_divider`、**出口依赖度列**。我们的两列被插在 `last_column` 内、结余列之前，所以视觉顺序是 价格／涨跌／工作量／占比／结余／依赖度×2 —— 后两列就是“多出来的条”。
3. **实现（两处镜像）**：`patch_market` 给两列依赖度的容器插入 `visible = "[Not(InformationPanel.IsTabSelected('ironroad_logistics'))]"`，使它们只在原版页签显示；`native/assets/ironroad_logistics.gui` 的表头行末尾加 `sort_button` 形式的**结余**列头（`MARKET_PANEL_BALANCE` + `MARKET_GOODS_BALANCE_TOOLTIP`，复用原版键）。`test_native_gui_runtime` 验证 Python/C++ 归档逐字一致 PASS。
4. **新增只读诊断**：`state field probe`（`m_stateFieldProbeDone`，每 world 一次）。动机：查证 `State.GetMarket` 确实存在（`map_markers.gui`：`BuildingMarker.GetBuilding.GetState.GetMarket`），说明引擎有州→市场解析；若州对象里还存着 GUI 空间的市场 id，那么该 dword 必须**在每个 `state+0x48` 分组内恒定、且组间互不相同**。实现：一次读入每个州对象 `0x200` 字节，提取 `0x000..0x1FC` 的 128 个 dword，按上述性质为每个偏移打分，打印通过 1:1 检验的候选及其值域。若某个候选值域很小（0..64），基本就是面板空间的市场 id。
5. **构建**：主体 `cdb02c78…`（32,268,410 B）；GUI 测试、C++/Python 归档一致、`verify_embedded` 全部 PASS。**未部署**（用户仍在看界面，明确回复“还在看，稍后”）。

## 2026-10-23 — R84f–R84i：GUI 属性错误清零与表头/列宽按实测收紧

用户提供了三张界面截图并逐项反馈，据此完成四轮小改动：

1. **R84f 属性错误**：游戏 `error.log` 每次启动都记 `gui/ironroad_logistics.gui:61 - Property 'margin_right'(815) not handled` 与 `:57 - Error setting properties for '' (icon)`。根因是我们自己的 crate `icon` 用了引擎不处理的 `margin_right`。改为 `position = { 190 21 }` + `size = { 58 58 }`（该文件其它 icon 本来就用 position/size），并在 `tools/test_native_gui.py` 加 `assert 'margin_right' not in fragment`。实机新会话 `error.log` 命中 0。
2. **R84g/R84h 列宽**：脚本实测原版与补丁后的表头/行宽——原版 `states_panel` 表头 540 px、行 480 px；补丁后行变成 524，比原版宽 44 px，这就是“表头好了但下面不正常”的原因（新列被裁）。改为行 52/52/78/92/92/54 + 60 = 480；表头 42/42/42/58 + 100/100/90 + 60 = 534，且把被压成省略号的“价格差异”列从 80 加宽到 90。市场页两列 90→70、wrapper 180→140，标签缩短为“工作量／占比”（`% of all freight`→`% of freight`）。
3. **R84i 表头格式统一**：用户要求“货运要和其它列头一样的格式”。原版表头是内置 `sort_button`（`align = hcenter|nobaseline` + `size` + 可选 `onclick`），我们之前用了 `textbox`。两处都改成 `sort_button`（不带 onclick，因为我们没有对应的引擎排序）。实测该控件允许省略 onclick——`error.log` 无相关报错。
4. **约束记录**：原版商品行 `button size = { @panel_width 50 }` 固定 540 px；行内其它列 250 + 原版 `last_column` 290（价格块 210 + 右锚定结余 80）= 540，已无余量。用户要求的“结余列放到工作量左边并加列头”必须让原版价格块让出宽度，属于取舍而非纯实现，需用户裁决。

## 2026-10-23 — R84e：重复 null 发布修复（发布频率约 65/s → 只在变化时）

1. **复现**：当前会话日志最近 400 行里 **399 条**是 `publish document=0 synthetic=-1`（间隔约 15 ms，即每帧）。来源是 `DrawUi()` 在调试面板隐藏（默认）且无原生 owner 时的两处 `PublishLogisticsDocument(nullptr)`（`overlay.cpp` 2479/2486 附近）。
2. **代价**：每次发布都会 `QueueLogisticsVariables` → `PostMessage(WM_APP+0x51)` → UI 线程重新推导并把 9 个 `ironroad_*` 变量逐个 `assign`，等于每帧重写同一批字符串；同时污染日志（约 65 行/秒）。
3. **修复**：`PublishLogisticsDocument` 先构造“发布键”并与上次比较，相同则直接返回。键包含文档指针、`status.owner`、`status.world`、`status.revision`、`m_nativePanelUnavailable`、`m_goodsMetadataUnavailable`、`m_nativeWorldRetired`；未发布过（`valid=false`）时必然发布，保证初始 waiting／作用域文本仍会写入。键结构抽到 `native/src/publish_key.h`，`overlay.h` 引入。
4. **回归测试**：新增 `native/test/publish_key_tests.cpp`（接入 `native/build_tests.ps1`，紧接 state panel 测试之前）。覆盖：两个未发布键不相等、null→null 相同键相等、null 紧跟在文档之后必须发布、同一对象重复发布相等、以及 owner／world／revision／panel／goods／worldRetired 六个字段各自变化都必须发布。实跑 `PASS publication dedupe key …`。
5. **部署**：完整 build → 主体 `01d2d708063614046f2e09e0e8b8562b2e52f34c322d00c560d20d5d9482b38a`（32,079,882 B）；无游戏进程时 `deploy.ps1` 安装，安装＝dist；完整测试入口零失败（`native/build/entry-round84e.log`）。备份上一版 `1c3428e5…` 到 `checkpoints\2026-10-23-r84e-publish-dedupe`。
6. **实机验证（同一存档）**：5 秒窗口日志仅 +187 B、最近 400 行仅 3 条发布；30 秒窗口 +1,314 B、最近 2000 行仅 4 条；**整个会话总共 4 条发布**，且每条都紧随 `real economy snapshot accepted`，其后 `UI-thread variables published` 正常出现 —— 证明真实变化仍会刷新、初始文本仍会写入，不是把变量刷旧。证据 `native/build/round84e-publish-dedupe.txt`。
7. 未改变的部分：市场身份仍是唯一闸门（`publish document=1` 仍为 0），地区列仍 `--`。
8. **顺带的市场身份线索（记录备查）**：原版 `game\gui\market_panel.gui` 里，市场面板的 `datacontext` 是 `MarketPanel.GetMarket`，多处用 `GetMetaPlayer.GetPlayedOrObservedCountry` —— 即“市场面板显示的就是玩家/观察国的市场”；而“州”页签（`InformationPanel.SelectTab('states')`）只是把地图模式切到市场/州模式（`MarketPanel.GetMapModeList`），**并不在面板里列出州**，所以从 GUI 侧拿不到“市场→州集合”。这进一步支持“面板市场 → 国别 → 该国州集合 → state+0x48”的推导路线，前提是能读出两侧国别字段。

## 2026-10-23 — R84d：实机读档往返（A24 取证）与三处结论收口

1. **存档侧试探（离线）**：`save games` 目录此前为空；请用户存了一次档，得到 `autosave.v3`（8.8 MB，magic `SAV0`）。`continue_game.json` 与存档 `meta` 给出**玩家国家＝大不列颠（GBR）**、版本 1.13.11 —— 这解释了“不列颠市场”就是玩家自己的市场，快照里州数最多的 `primary=33`（85 州）很可能对应它。但 `gamestate` 是 72 MB 的**二进制**格式（`SAV0`→zip→二进制 token 流），文本检索只能命中诸如 `ig_trait_farmers_markets` 这类无关串，解析成本过高，故不作为常规证据源。探测脚本留在 `native/build/inspect_save.py`。
2. **实机读档往返**（用户在游戏内手动：回主菜单 → 重新载入刚存的 autosave；我方不发送任何输入）：
   - `[268008968] WARN native market panel/filter unavailable: interface identity unavailable`（面板随世界切换消失）；
   - `[268009171] INFO world changed: invalidating economy snapshot, next world=2` —— 世界替换被识别并安全失效；
   - 新 world 重新采集：`states=1147 orders=25396 goods=43`、`markets=52 primary=33 states=85 top=33:85 35:75 112:73`，与读档前完全一致；
   - 全程 `native session tick owner=0 gen=0 ready=1 frame=0 ribbons=0`，无旧文档发布、无旧帧提交；进程存活、无 ERROR、无崩溃转储。**安全失效这一段 PASS。**
3. **A24 缺口精确定位**：`InvalidateEconomy` 里 `m_nativeWorldRetired.store(true)` 之后**全仓库再无 `store(false)`**；该标志被 9 处原生路径引用（830/839/895/1021/1078/1220 与范围文本 1666），范围文本会变成 `map unavailable after reload; restart required`。即读档后正式视图不会恢复、仍需重启。这正是总目标禁止的“以重启代替生命周期支持”。修复前提是原生对象跨 world 的注册／注销／寿命证据，按约束保持停用。
4. **州探针第二次运行（结论加固）**：本次探针落在**完整采集**上（`orders=25396 goods=43`），结果与加载中那次**逐字相同**：`anchor id=0 state+0x48=112`，12 个指针全部 `vtable_rva=0x624690`，无 `0x045850B0`。→ “州对象→市场对象指针”否定结论成立，排除加载不完整造成的假象。
5. **`+0x374` 弃用**：本会话面板已开（`marketOpen=1`，`tab="ironroad_logistics"`），GUI 注册表 11 个市场对象该字段全部 `0xFFFFFFFF`。三次观测两次未填，且写入时机不可控，不再作为候选映射。
6. **工具环境备注**：`Get-Content` 直接打印中文文档会出现 GBK 乱码（文件本身是 UTF-8 无 BOM、校验通过），读文档一律用 read/edit 工具；Python 输出需 `PYTHONIOENCODING=utf-8` 或落盘后再读。

## 2026-10-23 — R84c：州对象→市场对象指针诊断（否定）与地区列的连带依赖

1. **实现**：`overlay.h` 增 `m_stateMarketProbeDone`（每 world 一次，`InvalidateEconomy` 重置）；`CollectRealEconomy` 在选定 anchor 州后枚举其 `+0x00..0x400` 指针字段，打印目标 vtable 的模块内 RVA 与 `+8`／`+0x10` 的 id 样 dword，命中 `0x045850B0` 时特别标注。**只写日志，不参与采集与显示。**
2. **构建**：完整 build 通过，主体 `1c3428e5eacb77546ac755bd3da7217457abe2aa8240264627fd8a99c71a1530`（32,070,154 B）；备份旧主体 `960d42fe…` 到 `checkpoints\2026-10-23-r84c-state-market`，游戏关闭时经 `deploy.ps1` 部署，安装＝dist；完整测试入口零失败（`native/build/entry-round84c.log`）。
3. **实机**：以 `-continuelastsave` 启动，采集到：
   - `state market probe: anchor id=0 state+0x48=112`；
   - 12 条指针，**全部** `vtable_rva=0x624690`（每商品容器类型），**没有** `0x045850B0`；
   - 同帧 `real economy collected states=1147 orders=7786 goods=22`、`markets=52 primary=33`，说明本次采集发生在存档仍在加载时（商品只认出 22/53），但州对象已可枚举且 `+0x48` 可读。
   - 证据：`native/build/round84c-state-market-probe.txt`。
4. **结论**：该构建下州与市场的关系是**整数 id**，不存在“州对象→市场对象指针”这条捷径。R82 那类“沿对象指针取身份”的做法在这里不适用。
5. **连带发现**：`CopyNativeStateRoster` 用 `orders[].market`（源自 `state+0x48`）建 roster，`NativeStateOwner::Update` 以 roster 命中为激活前提。地区列与市场卡片因此**共用同一个未验证字段**，不能当作两条独立工作。
6. **工具限制记录**：Ghidra 对 getter `0x225AC90` 一带无函数体（`disassemble_bytes` 0 条指令），脚本执行被禁（`GHIDRA_MCP_ALLOW_SCRIPTS` 未设）；文档中的 resolver `0x10571F0` 实际是构造 `GetSuppressionEffectDescriptors` 字符串的函数，**该地址记录已过期**，不能继续引用。
7. **下一步**：改用“面板市场 → 国别 → 该国州集合 → state+0x48”的可核对推导链；先加一次性诊断把两侧的国别字段找出来。备选用 `market_override=112` 验证下游管道（标注为 override 证据，不代表身份正确）。

## 2026-10-23 — R84b-2：市场身份第二样本与结论反转

1. **部署了 R84b 诊断构建**（游戏关闭时经 `deploy.ps1`，安装＝dist＝`960d42fe…`），并跑完整测试入口：18 项原生＋引导／部署／运行目录、`verify_embedded`（同 hash）、原生 GUI 归档／运行时、flow GPU、selftest（560396 亮像素）、默认隐藏 vtx0 —— **零失败**（`native/build/entry-round84b.log`）。这同时验证了「完整测试入口要求无游戏进程」这一环境依赖：上一次游戏在跑时唯一失败项正是 `test_freight_deploy.py`。
2. **启动诊断构建并等采集完成**：新增诊断行先给出**完整**市场 id 列表。首次采集在存档仍在加载时进行（`states=1117`，59 个市场 id）；加载完成后为全量州、**52 个市场 id**：`31 32 33 … 112 116`。因此此前“两个存档”的判断应更正为**同一个存档在加载中与加载后的差异**（22:09 的 `1147/52` 与 22:22 的 `1117/59` 并不是两份存档）。
3. **第二样本**：用户再次打开 市场→物流。DLL 诊断行输出 `native market identity gui=0 sim_candidate=4294967295 in_snapshot=0 snapshot_markets=52` —— 与上一会话的 `+0x374 = 33` 不同，**该字段未填**。→ `+0x374` 假设被否，不得用于映射。
4. **注册表惰性性实证**：同一会话中未开面板时 GUI 注册表 `count=2`（`0` 与 `0xFFFFFFFF`）；打开面板后 `count=14`（`0 1 2 3 4 7 8 9 10 13 14 19 22`）；上一会话曾达 45 条。id 空间是 GUI 本地的、随视图增长；而快照 `state+0x48` 是 `29..166`。**两空间基本不相交**。
5. **探针增强**：`live_panel_probe.cpp` 支持 `argv[2]` 传入快照市场 id 列表（逗号/空格分隔），据此在面板市场对象自身字段与一层指针目标中做**精确成员匹配**。结果高度噪声（43/40 这类商品计数与 31–50 段市场 id 撞车），未能定位唯一字段；市场对象内满足 `(ptr,capacity,count)` 形状的向量只有一个（`+0x510`，count=1，值 635），不足以做“州集合对齐”。
6. **结论与下一步**：R53 的 `state+0x48 = market id` 只是聚类推断，从未与引擎市场 id 对账；现以“面板 id 0 与快照 29..166 不相交”为反证，**快照的市场分组字段需要重验**。可行正解是让 DLL 在进程内沿**州对象 → 市场对象指针**取市场 id（与 `panel+0x70` 同空间），从而彻底不需要跨空间映射。该路径只需单州诊断先验证指针存在，再改采集与 owner 条件。

## 2026-10-23 — R84b：市场身份两个命名空间的实证与只读诊断

**背景**：R84 已证面板解码链正常，但 `panel+0x70=0`（用户确认面板为“不列颠市场”）不在快照的 52 个 `state+0x48` 市场 id 中，正式 owner 因此拒绝。

1. **静态依据复核**：`PROFILE.md`／`NATIVE_GUI_PLAN.md` 记录 `MarketPanel.GetMarket` getter `0x225AC90` 正是取 `panel+0x70` 作为 ID 交给 resolver `0x10571F0`，wrapper ID 落在 `+0x10`。所以 `panel+0x70` 是**引擎市场 ID 空间**的值，不是任意字段。
2. **定位 GUI 市场注册表**：`native_goods_filter.h` 早已解析该注册表（`module+0x059684A0` → `+0x00` entries、`+0x08` capacity、`+0x0c` count、`+0x18` lock；条目 `object+0x10` = GUI 市场 id，vtable `0x045850B0`）。据此给只读探针加 `[5c]` 段并实测：
   - 注册表 `count=45 capacity=63 lock=0`；id 集合 `0..25,30,31,36,49,50,51,53,54,59,60,61,62,65,88,106,109,110,113,4294967295`；
   - 快照的 `33`／`35`／`112` **都不在**该集合中；面板市场 `0` **在**集合中；
   - 面板市场对象 = 注册表 id 0 的对象，其 `+0x374 = 33`（快照 primary 市场），而其余 44 个市场对象的 `+0x374` 全为 `0xFFFFFFFF`（惰性未解析）。
3. **结构旁证**：该对象 `+0x370` 的 qword 为 `0x00000021AF48EB08`，高 dword 正是 33；`+0x368/+0x378/+0x388` 呈 (0,5)(0,5)(0,10) 成对结构，像是按需填充的缓存条目而非计数字段。
4. **仍未定性**：只有一个样本（gui 0 ↔ 33），且 Ghidra 侧该 vtable 的目标地址多数未被识别成函数（`batch_decompile` 返回 Function not found），无法用静态反编译确认 `+0x374` 的写入语义。按“未知身份拒绝、不猜”的要求，**不据此激活正式视图**。
5. **因此本轮交付的是只读诊断**（不改任何视图行为）：
   - `market_panel.h` 新增 `simulationMarket` 字段与 `ReadGuiMarketSimulationCandidate()`，以与 `native_goods_filter.h` 相同的注册表布局读取候选；歧义/忙锁/不可读一律返回 false（fail closed）。
   - `overlay.cpp` 在市场或候选变化时打一行 `native market identity gui=%u sim_candidate=%u in_snapshot=%d snapshot_markets=%zu`，并在采集时输出**完整**的 `real economy market ids:` 列表（此前只打 top3），用于下一轮对账。
   - 不改 `NativeMarketOwner` 的激活条件，正式 owner 仍然拒绝。
6. **构建与回归**：完整构建通过，新主体 `960d42fe5f41b6f7cbfcd6ad5c84457439184390dd64a952b05e70fa87bdb855`（32,065,546 B，比上版 +24 KB）；18 项原生回归全部 PASS；`verify_embedded.py` PASS（同 hash）。
7. **发现环境依赖**：`build_tests.ps1` 末尾的 `test_freight_deploy.py` 走 `deploy.ps1`，游戏进程存在时被守卫拒绝（`Close Victoria 3 before deploying.`）。游戏运行中该步失败属预期，不是回归；已记入 AGENTS §3。当次原生 18 项与引导 valid/missing/corrupt 三项仍全 PASS。
8. **未部署**：游戏（pid 48420）仍在运行，未强替 DLL。诊断构建只在 `dist`，安装仍是 `4ac3a0e8…`。

## 2026-10-23 — R84：阶段1基线固定与阶段2双DLL实机加载证据

**环境**：无游戏进程（22:11 起）；EXE `7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f`（1.13.11）；工具链 `E:\Program\C Compiler\MSYS2\ucrt64\bin`。

1. **实机机会（阶段2）**：开工时正好捕获到一次真实启动（22:09:19 pid 59636，用户手动启动，非我方输入）。实时只读取证：
   - `binaries\freight\loader.log`＝`Ironroad freight loader: loaded freight.dll (error=0)`；
   - `Get-Process -Id 59636 | Modules` 实测三个关键模块：`VERSION.dll` → `binaries\VERSION.dll`（我们的代理，mem 532480）、`freight.dll` → `binaries\freight\freight.dll`（mem 29,667,328）、`version.dll` → `C:\Windows\system32\version.dll`（系统导出转发目标）。这条是双DLL布局的关键证据：主体确实从子目录加载，代理与真实系统 DLL 并存；
   - 主体日志：`native profile ready: Victoria 3 1.13.11 sha256=7f53e326…`（对照旧会话的 `native profile disabled: unsupported Victoria 3 build`）、`runtime data=…\binaries\freight\data`、`embedded asset pre-enumeration hook installed`、`embedded archive mounted during native MountData BEFORE VFS configuration freezes (no playset)`、`embedded archive ready BEFORE gfx/models .asset enumeration`、`native GUI enumeration; prepared=1 mounted=1`、`ImGui ready (hwnd …, 2560x1440)` 但 `drawdata display=2560x1440 … lists=0 vtx=0`；
   - 该会话无新增加载／解析错误；22:10:37 游戏自行退出（日志无异常结尾、安装目录无崩溃转储）。进程在 22:10:37 后消失，模块取证是在其存活期间采样的，事后已无法复取，故按“采样时刻”记录。
2. **进入存档后的只读读数（阶段3增量）**：`real goods metadata catalog=53 observed=43 local=2 nontradeable=1 thread=59996`；`real economy collected states=1147 orders=25317 goods=43`；`real economy markets=52 primary=33 states=85 top=33:85 35:75 112:73`；相隔 25 秒两次采样完全一致；`arrow pass` 在 frame 681 后出现，说明地图已加载（此前 `no arrow render pass observed after 301 frames` 属主菜单/加载阶段）。市场面板始终关闭：`native market owner state ready panel=4294967295 tab=default`，`publish document=0` 贯穿整段，`native session tick owner=0 … ready=1 frame=0 ribbons=0`。
3. **快照口径核对（代码）**：`overlay.cpp` 的 `PublishRealEconomy()` 在 `SetSnapshot` 成功后调用 `PublishLogisticsDocument(nullptr)` 并打印 `(diagnostic)`，注释明确“Snapshot availability does not activate a panel. Only the observed open native Logistics tab may own the formal view”。因此 `orders=25317` 只证明采集与接收成功，不证明正式 UI 有数值；地区列与市场卡片仍 `--`。
4. **构建可复现（阶段1）**：先做对照实验——直接重建 R83 源码，得 `3b6c1ce7…`／`5e340122…`，与已安装的 `c58bcaaa…`／`6887546a…` 不同但尺寸完全一致；用 `native/build/diff_dll.py` 逐字节比对，只有 3 段 6–7 字节不同：PE `TimeDateStamp`（0x88，两文件 e_lfanew 均为 0x80）、由它派生的 `CheckSum`（0xD8）、以及调试目录中的时间戳副本（0x30604／0x260404）。确认 `tools/` 与 `native/` 没有依赖 PE 时间戳的代码、也没有硬编码旧 hash，于是给 `native/build.ps1` 两处链接加 `-Wl,--no-insert-timestamp`。
5. **复现性验证**：改后连续两次完整构建（间隔 3 秒）→ 两次 hash 完全相同，`TimeDateStamp=0`：引导 `71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e`、主体 `4ac3a0e8873dbe75ce945438a1ce476c8583e29d1f3f8d610245ac19e29f2980`；嵌入资源包 `assets sha256=85b17cf3…`、10 个资源 3,785,191 B。
6. **完整入口**（`native/build/entry-round84.log`，逐步记录退出码）：`native/build_tests.ps1`（18 项原生回归＋引导有效/缺失/损坏导出、沙箱安装／升级／卸载／恢复、真实主体目录／迁移／新配置优先）、`verify_embedded.py`、`test_native_gui.py`、`test_native_gui_runtime.ps1`、`test_flow_gpu.py`（八档宽度×九色板+像素着色器+负对照）、`selftest.ps1`（560376 亮像素）、`test_debug_default.ps1`（PID 60820，vtx=0）——全部 EXIT=0，无未解释失败。
7. **部署**：确认无 `victoria3` 进程后执行 `deploy.ps1`；安装后逐一比对，引导与主体均与 dist 相同（`71dae36d…`／`4ac3a0e8…`），无遗留 `.tmp`／`.ironroad-backup`。selftest 走的正是 `dist\version.dll` → 子目录主体的真实加载链，因此新加的时间戳标志没有破坏 PE 与加载路径。
8. **备份**：`E:\Code\v3-analysis\checkpoints\2026-10-23-r84-baseline\`（`dist-before-rebuild`＝R83 两份 DLL、`live-logs`＝原始 freight.log／loader.log／freight.ini）。
9. **未做**：未启动或操作游戏（22:09 那次是用户自己启动的），未发送任何界面输入；未测改图资源、Freight 供给、真实铁路、读档往返与性能；刚部署的构建还没有实机启动证据。

10. **新部署构建实机启动（22:22:41 pid 11116，`-continuelastsave`）**：加载链与 22:09 完全一致（profile ready、`freight\data`、VFS 冻结前挂载、`vtx=0`），进程模块实测 `VERSION.dll`=binaries=`71dae36d…`、`freight.dll`=binaries\freight=`4ac3a0e8…`、`version.dll`=system32。存档读出 `states=1117 orders=28554 goods=49 markets=59 primary=118`（另一份存档）。22:23:27 游戏自身日志 `application.cpp: Quit: Quit from inside game`，无崩溃转储、无 Application 事件——属正常退出，不是 DLL 崩溃。
11. **用户自行启动（22:23:44 pid 48420）并打开面板**：加载链第三次复现；随后 `native panel hide flows=1/0`（物流页“显示/隐藏”被点击两次），`real economy markets=52 primary=33 states=85 top=33:85 35:75 112:73`、`orders=25375→25399`、两次采样一致。但 `publish document=1` 计数为 **0**，`native session tick owner=0`。
12. **只读活进程解码器**（新增 `native/test/live_panel_probe.cpp`，复用 `market_panel.h`／`state_panel.h` 生产解码逻辑；`OpenProcess(PROCESS_VM_READ)`＋`ReadProcessMemory`，不 attach、不暂停、不写、不调用任何引擎函数）：
    - `phase(machine+0x588)=4`、idler vtable／gfx vtable／handler vtable／`handler+0xd8` 自指针全部 MATCH、`panel count=56` —— 生产 gate 前提全部成立，R73 的静态链在 1.13.11 上没错。
    - slot 9 = `market_panel`，`name`/`tab` 通过生产的 MSVC 字符串校验；slot 14 = `states_panel`。
    - 州面板置顶时：`DecodeNativeMarketPanel.marketOpen=0`、`DecodeNativeStatePanel.stateOpen=1 state=433 tab="local_goods"`。把市场窗口置顶后：`marketOpen=1 market=0 tab="ironroad_logistics" visibleGoods=40` —— 原生分类＋搜索＋`AccessMarketGoods` 缓存链在实机可用。
    - 由此确认：**解码不是瓶颈，市场身份是瓶颈**。`panel+0x70 = 0`，而快照市场集合来自 `state+0x48`（52 个，含 33/35/112），`NativeMarketOwner::Update` 要求 `markets.count(panel.market)`；扫描 `market_panel` 前 0x200 字节未出现任何快照市场 id。owner 因此判定 inactive 并直接返回，`m_active` 也为 false，所以连“closed”日志都不打。行为符合“未知身份拒绝、不猜”，但结果就是正式页仍是 `--`。
    - 决定性定性：原版 `game\gui\market_panel.gui` 的 `datacontext = "[MarketPanel.GetMarket]"`、标题 `Market.GetNameNoFormatting`（单一市场，无市场选择器），**用户确认面板标题为“不列颠市场”**。所以 `panel+0x70 = 0` 是一个真实国家市场的 **GUI/面板身份**，与快照 `state+0x48` 的 52 个市场 id 属于两个命名空间（与 R82 州身份错位同类）。在取得经验证的 GUI↔快照映射前，不得用 primary/最大市场顶替，否则就是把 33 号市场的数据冒充不列颠市场。
    - 附带结论：`NativeStateOwner`（`native_state_owner.h`，`kOwnerNativeState=15`）已实现但 `overlay.cpp` 从未引用，地区列缺少运行时接线。
13. 证据：`native/build/round84-panel-probe.txt`（完整探针输出）、`native/build/round84-panel-open-session.log`（面板会话非例行日志）、checkpoint 的 live-logs。

下一步：先确定 `panel+0x70 = 0` 的语义（世界市场／未选择／GUI 注册表另一命名空间），再把 `NativeStateOwner` 接进 overlay 运行时并做地区列发布。探针工具可用于每次运行时核对，不随 DLL 交付。

## 2026-10-23 — 旧LocalAppData迁移与清理审批阻塞

- 用户明确要求迁移并删除 `C:\Users\RedAndBlack\AppData\Local\IronroadV3`。无游戏进程时，将全部15文件（34082141B）复制到安装目录 `binaries\freight`：ironroad.ini／ironroad.log及历史日志改为freight命名，data与版本化资源缓存原名保留；不改DLL。
- 全部文件在源／目标／checkpoint三处逐个SHA-256一致。完整旧目录备份及逐文件清单／校验记录：`E:\Code\v3-analysis\checkpoints\2026-10-23-freight-data-migration` 的old-runtime、manifest.json、validation.txt。
- 清理未完成：自动审批分别拒绝经范围／hash核验的批量删除与显式完整路径逐项删除，均返回blocked by policy，无更具体原因；删除均未执行，旧目录仍存在。新目录可用且已验证，剩余动作是用户手动删除上述旧目录；不能记录为旧文件已清理。

## R83 当前状态（2026-10-23）：version引导＋freight主体与独立运行目录

- 用户明确允许EXE旁保留引导，由它加载子目录主体，并将目录命名为 `freight`；正式交付改为 `version.dll`＋`freight/freight.dll`，替代原单DLL限制。代理17项导出保持原转发实现，主体用自身目录存 `freight.ini`／`freight.log`／资源缓存，引导记录 `freight/loader.log`。
- 独立引导线程以绝对路径加载主体，不依赖工作目录；缺失／损坏主体仍保持系统导出。旧配置复制迁移且不覆盖新配置，源文件保留；配置改为临时写入后原子替换。部署成对安装／移除，保护外来主体，保留配置缓存。
- 原有原生回归通过；引导中文／空格／异工作目录及缺失／损坏测试、真实主体目录／迁移／新配置优先、沙箱安装／升级／卸载／恢复外来代理测试PASS。测试初次因PowerShell模块环境与环境变量键大小写失败，修正测试环境后单独重跑通过，未将原全脚本失败记录改为PASS。
- 完整build及最终重链接、verify_embedded、GUI归档、D3D11 selftest（560412亮像素）及默认隐藏PASS。已在无游戏进程时经deploy.ps1安装，安装hash与dist一致：引导 `c58bcaaa91223d74255cd90fcd582a482c12cfc73f6c03c24c1b18768a1681ea`（779879B）；主体 `6887546a2a8d53a838278c8df7380d6da1606439a7024f7033571037260e5816`（32041860B）。
- 未启动／操控游戏；新布局实机启动／早期挂载时机尚未验证，R82地区发布／改图／Freight供给／铁路／生命周期／性能缺口不变。备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-freight-loader` 含旧安装DLL；证据 `native/build/*freight-loader*.log`，旧LocalAppData保留供迁移。

## 2026-10-23 新增需求：独立安装／运行目录

- 用户希望DLL及配置／日志／缓存集中在 `common\Victoria 3\binaries\IronroadV3\`，已写MASTER_PLAN §3.1.1及AGENTS稳定约束；目录名为当前默认选择。
- 事实：当前dllmain.cpp／deploy.ps1仍依赖EXE旁 `version.dll`，日志与缓存仍由源码写LocalAppData。子目录DLL加载入口、运行目录迁移均未实现；若需根引导＋主体双DLL，须先裁决与原单DLL要求的冲突。
- 本轮只记录要求，不改源码／DLL／部署，不移动文件或操作游戏。后续按A02–A04／A26验证加载、目录落点、迁移、权限失败及回滚。R82候选／R81安装及实机结论沿用下方记录。
- 重读00／01／02／03，重点loader、备份、证据层级；核对dllmain.cpp、embedded_assets.cpp与deploy.ps1。备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-isolated-runtime-requirement`，文档／未改产物校验见该目录validation.txt。

## 2026-10-23 文档维护：逐条对照与精简

- 完整阅读00／01／02／03，逐项对照技术要求、证据边界及后续用户要求，形成139项来源／去向表：`docs/transport/DOCUMENT_COMPARISON.md`。
- AGENTS仅保留稳定规范／命令／恢复入口及简短日志；MASTER合并重读和执行协议，保留核心约束、P0–P7与A01–A27。两处旧历史章节原文移至`docs/transport/history/*-through-R82.md`；旧§19／20.x引用在归档查原章节。
- 当前产品事实仍以下方R82记录为准：候选未部署、上次运行／安装R81、地区数据接线及改图实际资源／完整实机验收仍未完成；用户手动界面，自动输入保持停止。本次仅改文档，不改源码／DLL／部署／实机结论。
- 备份：`E:\Code\v3-analysis\checkpoints\2026-10-23-docs-trim`。验证：原文hash、历史正文、核心约束和验收行、UTF-8无BOM及文件链接；证据同备份目录`validation.txt`。详细记录见WORKLOG。

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


## 2026-10-23 — R78：参考图市场卡片、商品列与当前帧发布

- 上轮R76部署／R77统计修正为进展；恢复时查询95099／97389，均退出0，R77 13项回归／完整build／embedded确认，通过后补齐§20.75与交接，不因旧等待状态重启。完整重读01／03，核对GUI／文本桥／队列／失效／性能约束。用户Escape只停止界面自动操作，本轮未再调用Computer Use。
- 自有GUI按市场参考图增加两卡、crate／railway原版图标、Freight／share列；Show/Hide与商品入口不变，移走市场ID诊断行。原价块从已校验market_panel.gui的goods_entry_button.last_column本机派生，插入自有placeholder，保留价、趋势、偏差条。首次Python类型正则漏flowcontainer而拒绝，修正锚点后通过；缺／重placeholder与未知哈希仍拒绝。没有复制整份GUI／纹理到分发资源。
- 两种语言增加标签／口径tooltip。现有同名native Get／Exists、EqualTo_string与hover上下文用于行数据ready门控；EqualTo_string在本机game_rules／location_finder有原版用法。新回归检查价块原字节保留、价格趋势、列绑定与拒绝条件；实际C++ ZIP镜像逐字通过。
- FormatFreightGoods输出稳定商品键变量，固定全市场分母。测试涵盖50.00／71.4%、分类／hover／排序不变、missing／synthetic／重复／非法键、非有限工作量、0工作量与0市场百分比。原版发布携自有frame／applied再验，先撤goods_context、清旧行，写成功且最终帧仍当前才显示；过期摘要清为--。只写GUI表，worker无engine指针，不改经济／分配。
- 新行写入会频繁更新已有key，因此重用FindEntry返回的已校验value+0x28直接assign，避免每次更新新建pair；新键路径未改变。静态布局／旧assign实测不证明本新分支或批量写入已经运行；原生UI成本／内存平台仍待游戏。
- 13项回归及完整build退出0；test_native_gui、test_native_gui_runtime、verify_embedded PASS。候选48f0e495698bcb754bdd400a77cdc2f05c68543bf1c2d880d6f05ff5cb69cf29，31,753,550B。确认游戏未运行，备份R76安装DLL并经deploy.ps1安装，hash相同；未启动或操控游戏，未重跑不相关GPU／默认隐藏测试，不沿用旧测试宣称新候选实机通过。
- GUI源码预览native/build/r78-gui-preview，不是游戏截图。地区列／州上下文尚未接，市场新布局／parser／row发布、hover延迟／25ms性能／同帧关闭与完整生命周期NOT_RUN。备份2026-10-23-r78-market-reference-ui，MASTER_PLAN §20.76、AGENTS／STATUS／TASKS／README／NATIVE_GUI_PLAN／FREIGHT_MODEL同步，目标active。

## R77 统计层完成（2026-10-23；总目标进行中）

- 已完成参考图统计层：`marketFreight` 固定全市场占比分母，`selectionFreight` 汇总当前州／搜索／分类，`filteredFreight` 仅表示 hover 后的地图贡献。正式卡片使用 selection；hover 不改卡片、份额分母或 Freight 需求。地区逐商品工作量只统计该州最终输入／输出，每笔一次。
- 地区反例先 FAIL，修复后原有句柄 95099／97389 已确认均退出0；13项原生测试、完整build与verify_embedded PASS。候选 `3695238f83be193aa596e101360f53ec6b70c01a27ba5182b0f216aaf9d3787f`（31,174,023B），未部署／未实机；安装仍R76 `15fee7d6…`。证据 native/build/repro-round77.log、tests-round77.log、build-round77.log、embedded-round77.log。
- 这完成数值作用域，地区列、市场卡片／商品列的原版布局和逐行数据发布仍待接线。用户 Escape 停止的界面自动操作未恢复；继续代码／离线验证。备份2026-10-23-r77-reference-freight-scopes。

## R77 进行中（2026-10-23；Computer Use 已由用户停止）

- 已按新参考图调整统计层：全市场工作量、当前筛选工作量与临时悬停地图贡献分开；地区逐商品工作量只计该州输入／输出的最终货流。原生卡片格式化改用筛选工作量，保持全市场占比分母。尚未完成地区列和参考图卡片／商品列布局。
- 反例 repro-round77.log 已复现地区统计缺口；修复后构建与13项测试已启动，当前状态待核实，不能写成通过。exec session：测试 95099；构建 97389；日志 native/build/tests-round77.log、build-round77.log。恢复后先查询原句柄或日志／进程，不因观察超时重启。
- 当前安装仍为已验证部署的 R76：15fee7d63d34db078fafa1128de1afff1244c2a06b5e68896c15f044c16a5329；dist 正在重建，尚无 R77 最终交付哈希。源码修改前备份 2026-10-23-r77-reference-freight-scopes。
- 只读窗口枚举时工具报告用户按物理 Escape 停止 Computer Use；本轮未继续任何界面操作。编码／非UI验证的目标仍未完成，不把界面自动操作停止写成整个 goal 完成。

## 2026-10-23 — R76：原版筛选恢复与原生商品悬停桥

- 前轮 R75 为有效单位修复进展。完整重读 01／02／03，重点原版 UI、Freight behavior、100／250ms 合并、owner取消、线程／原生生命周期和分层证据；使用 local-reverse-engineering 与 computer-use。最低标准：原版控件保留、悬停归属／恢复／过期拒绝的模型回归、完整构建与归档一致；新事件实际触发和延迟／开销仍需实机。
- 用户回复恢复原版控件，解除上轮显隐裁决等待。新 test_native_gui 原控件片段保留检查先失败，删除 native_gui.cpp 与 tools/native_gui.py 的旧 R22 子控件隐藏块后通过；保留父容器支持 Logistics 和原版折叠／搜索／分类绑定。没有修改安装原始 GUI。
- 参照安装 map_markers.gui 的 hierarchy enter／leave＋Set／Clear／Concatenate 模式；重看 Goods.GetKey 注册／回调0x1977D60与直接getter0x1957E50（读取wrapper+0x10 definition+0x18），不新增 native getter 调用。自有商品行继承容器绑定事件，原 inner button 的价格、tooltip、左／右键保持。
- NativeMarketOwner 增加逐商品变量键／上下文与临时 owner，依据 copied goods key→ID、可见列表和 world 验证；重开／market／world 改变换上下文，多行／失败／未知／过期拒绝。关闭／切页／元数据失效同时撤基础和 hover，另一调试 owner 不受影响。回归涵盖进入、移出、迟到 leave、歧义、不可读变量、过滤商品、重开旧上下文、tab close和正式 owner 渲染类别；分配重用、全市场分母与 Freight 需求保持固定。
- Overlay 新25ms UI timer负责面板／hover／session同步，原500ms经济周期保持；重建窗口时 KillTimer两项。仅 UI 线程触碰变量和原版状态，worker只收自有ID。正式 render／publish接纳 hover，ImGui不可见仍可提交。没有将25ms频率或测试通过写成实机同帧关闭、UI开销或完整生命周期通过。
- 构建初版后收到控件裁决，再完整构建 final。最终13项回归／GUI／C++归档镜像／verify_embedded PASS；selftest 560231 bright pixels、默认隐藏 PID27524 vtx0 PASS。候选 SHA256 `15fee7d63d34db078fafa1128de1afff1244c2a06b5e68896c15f044c16a5329`，31,174,023B。测试／归档 native/build/*round76*.log，失败 visibility-repro-round76.log 保留。
- 运行状态复核发现 PID52316 退出，曾有 PID8032 可见窗口后也退出。只读枚举窗口时未取得唯一目标，未发送游戏输入；没有强制终止进程。两次无 victoria3 后备份运行日志／原R69安装DLL，执行离线测试与deploy.ps1，安装哈希与候选一致。sky启动报窗口事件handler未就绪，复核未启动；实机新候选 NOT_RUN。
- 用户随后给出地区本地价格／市场物流参考图，确认 Freight 是工作量。固定新的展示要求到 MASTER_PLAN §3.2.1，原图保存私有 references/2026-10-23-freight-ui；未复制截图数值。市场筛选卡片与全市场占比分母分开，hover只临时改地图。R76没有实现这些新列／卡片，下一步继续该主线，保留Freight需求／供给与真实铁路缺口。
- 全部源码／DLL编辑前备份 2026-10-23-r76-native-hover；AGENTS／STATUS／TASKS／NATIVE_GUI_PLAN／README／MASTER_PLAN §20.74同步，目标active。

## 2026-10-23 — R75：原版摘要工作量单位修正

- 前一轮规范文档维护为有效进展；继续原目标，完整重读 01／02／03，重点原版 UI、线程／所有权、Freight behavior、分层测试，复核 MASTER_PLAN §4.4 及当前模型／显示代码。最低标准为当前显示路径的单位反例先失败、修复后通过，并区分费用、工作量和中转商品需求。
- 初查正式筛选入口发现历史 R22 仍隐藏物流搜索／分类，与总计划不符；按新 AGENTS §1.1 已请求用户裁决，显隐补丁保持原状。调查 JSON 显示三块原版控件被改写，原 binding 数量仍相同；原 GUI／归档测试全部 PASS，说明计数／镜像一致并不证明控件能操作。等待选择时推进独立且由 §4.4 已明确的单位修复。
- `AllocateFreight` 将 amount/kQuantityScale 转为 orders 后计算 freight=q×weight×distance；BuildFreightView.marketFreight 和 LogisticsDocument.allFreight 直接累加，overlay 却再次除以 kQuantityScale。先将该显示边界提取为 FormatFreightWork 并保留原换算，7 orders×10 map units=70 的测试失败；GUI 新单位标签断言也先失败，日志 repro-round75.log／gui-repro-round75.log 保留。
- 移除第二次换算；正式标签和 README 改为 weighted order-distance，FREIGHT_MODEL 记录单位。测试涵盖：运价 3 使费用 210 而工作量仍 70；商品权重使工作量 120 而 Freight 需求 11／7 不变；筛选后全市场工作量仍 70；缺失／无 world／合成／负值／非有限数为 --，合法零为 0.00。不修改分配器或 Freight 需求公式。
- 最终 native/build_tests.ps1（13 项）与 native/build.ps1 均退出 0，test_native_gui.py、test_native_gui_runtime.ps1、verify_embedded.py PASS。候选 SHA256 `64485541f14b7252297a0fa96a85bee3c0114e5fbd328c6d0af76e8001d020f8`，30,921,468 B。未部署／未运行新候选，安装仍 R69；未运行会覆盖现有日志的 selftest／debug_default，未强制结束游戏。
- 等待期间通过 Ghidra 只读初查注册／析构静态链，私有 arrow-lifetime-r75-static.md；构造注册与析构资源释放不能证明注销／world 生命周期，未新增任何析构调用或解除安全停用。静态结果不作为 ABI／寿命实测。
- 源码／DLL 修改前备份 E:\Code\v3-analysis\checkpoints\2026-10-23-r75-freight-work-units；AGENTS、STATUS、MASTER_PLAN §20.73 与口径文档同步。源码 diff／交接证据留 native/build；正式控件显隐待裁决，hover／逐帧关闭、Freight 供给／铁路和完整生命周期仍未完成，goal active。

## 2026-10-23 — R74：复用原版搜索／分类缓存的商品ID

- 补齐明确市场 ID 0 与旧版“未指定市场”的区别：原版 owner／SelectMarket 显式保留 0，只有未指定选择才自动回退；会话反例覆盖 market 0／primary 7，不能据此认定当前存档市场 0 有效。最终 tests-round74-final.log／build-round74-final.log 均退出 0，13 项回归通过。
- R73为有效实现进展；本轮完整重读01／03、复核原版market_panel.gui及模型／owner。静态GetFilters SAnyType确认为GoodsCategory字节数组；225A630为!contains，225A730单选／恢复时维护排除项并标wrapper dirty。Searchbar panel+0x80/string+8，225DB90把原版Unicode归一化query写wrapper+0x328；搜索还匹配本地化关键词，不能用goodN／英文key重做。
- 恢复AccessMarketGoods→1CDFAC0→1CD8650→1CD8690，返回wrapper+0x60缓存，1CDAAF0／1CDA600构建原版筛选结果；registry global59684A0，wrapper vt45850B0，row vt4552218／definition+0x10／state+0x18／market+0x1C。关键字节／dirty／writer／返回偏移用Capstone复核。没有调用getter／原生筛选器或取锁。
- 只附加本次PID52316，实际registry两项0／FFFFFFFF、dirty1、0rows，类型／header／query空布局符合静态；已detach并确认stopped无目标、游戏Responding。实际解码器回放将此样本拒绝为pending，而非已知空结果。**正向原版筛选列表及新DLL交互未验证**，不能据内存fixture宣称搜索已实机通过。
- 新增native_goods_filter.h安全复制原版缓存ID；唯一market／query相同／dirty与writer未占用／类型及行归属／分类／重读门槛，返回nullopt与空vector分开。MarketPanel状态只在物流页签读取筛选；pending撤旧正式owner并显示waiting，已知空保留合法市场统计、无ribbon。没有把Unicode query送到模型英文匹配。
- 新增FreightFilter／Selection allowedGoods，仅BuildFreightView提取贡献；Engine／Session比较与缓存身份含规范化ID，native owner对相同集合不重启worker。fixture覆盖中文query、dirty／busy／旧query／分类未同步／错误归属／重复market／中途变更；模型与会话验证分配复用、固定分母／heat／Freight目标、已知空与缺失、ID排序缓存不失效。13项回归PASS、完整build／verify_embedded PASS，候选1a97a6823e2fc72fa7f1f2178807803a4001fc89faee3ccd8df3fdc8b0223cfd（30,916,271B），未部署，运行仍R69。
- 备份2026-10-23-r74-native-filter-view；最初清单漏了overlay两文件，发现后按本轮五处明确差异逆向重建R73末态到checkpoint；统一diff自动重放尝试失败后未强行接受，未回退工作源码／DLL。其余文件均修改前备份。私有native-filter-r74-*不分发；native/build/*round74*.log及changes-round74.patch；MASTER_PLAN §20.71、AGENTS、STATUS、模型／UI／profile／验收同步。500ms响应、正向cache、正式hover、Freight供给／铁路／生命周期仍未完成，目标active。

## 2026-10-23 — R73：原版市场所有权／状态与正式视图接线

- 上轮R72为实机字段验证及源码接线进展；本轮重读01／02／03全文、NATIVE_GUI_PLAN／PROFILE、状态与仲裁代码。窗口工具仍无Victoria3，PID52316 Responding；未反复启动或要求用户操作。继续无人值守，使用local-reverse-engineering。
- 静态RTTI／构造调用链：CInGameInterfaceIdler global592D0E0→gfx→CIngameInterfaceHandler→嵌入bar+0xD8→vector slot9 CMarketPanel；恢复tab+0x30／market+0x70。原版IsPanelOpen注册30F500→2129E50→21255E0尾调用2125610，predicate为history count>0、closed byte+0x40为0、最后0x30条目kind9。Capstone分开检查tail helper，避免只看首个unwind extent漏掉逻辑。
- x64dbg仅附加自有测试PID52316，清除本次自动TLS断点并按paused状态恢复；读整条链、bar self/count56/history0/closed1、market_panel/default/IDffffffff。再次读取root一致；已detach并确认stopped无目标、游戏Responding／日志继续。未调用getter／RTTI cast／GUI效果；原始字节／反编译私存market-panel-r73-*，实际C++解析器回放得到available1/ingame1/open0/default/ffffffff。没有打开态／图像证据。
- 新增market_panel.h安全读／类型／所有权／字符串／变化拒绝；native_market_owner.h桥接现有Session，快照就绪不再自动选择最大市场，只有原版物流页签打开时按面板ID激活。关闭／切页／未知ID／缺元数据撤正式owner，重复采样不替换worker请求，其他owner保留。正式变量限定当前原生owner帧和world／revision／market；debug／旧帧不进入正式数值。
- 新13th原生测试覆盖closed／open谓词、最后history项、保留tab但已关闭、相位／坏类型／指针／中途变化；会话测试验证市场8代替primary7、重复采样不重算、切换取消旧帧、关闭无迟到、reopen、override、未知／缺元数据／合成拒绝、新world恢复、关闭base保留debug帧。13项最终回归PASS；完整build后加强正式变量与其他owner文档保护／错误诊断，再完整build-final-checked；verify_embedded PASS。
- 最终候选e0a2e7863db4f016091888b4e6d885678fae3149db8a12e6e4f22b58c7a3d1a6、30,376,351B，未部署／未实机跑新DLL；运行仍R69。500ms UI定时器是当前接线延迟，不冒充逐帧即时关闭，打开／切换／关闭与完整生命周期仍未验收。
- 备份E:\Code\v3-analysis\checkpoints\2026-10-23-r73-native-panel-owner；native/build/*round73*.log／changes-round73.patch；MASTER_PLAN §20.70、AGENTS、STATUS、DECISIONS／ACCEPTANCE／TASKS、UI／PROFILE／README同步。总目标active，Freight独立供给、铁路、正式hover／筛选等继续保留。

## 2026-10-23 — R72：实机商品目录／TLS与正式采集接线

- R71为有效进展，本轮继续无人值守；完整重读01／02／03及模型／profile，使用local-reverse-engineering。新建隐藏x64dbg48784、健康2.3.0、初始stopped无目标；仅附加自有测试PID52316，确认当前模块／哈希和运行R69。附加异步，早期run返回409非paused；自动TLS断点暂停后清除本次新会话断点并正常恢复；后续Nakama首次C++异常栈正常恢复，未skip。观察后detach；为主线程TLS再次短附加也已detach，确认stopped无目标、游戏Responding／日志继续。没有显式经济数据写入，调试器附加／自动断点会短暂改变执行，不作性能样本。
- 全53定义ID／vtable一致、53定义和向量重读一致；按key对本机goods定义53/53类别／local／tradeable一致，不由文件顺序推ID。local15/16/17，非本地不可交易gold51，automobiles46 flags10仍可交易；heap key luxury_furniture50确认。主线程51844 TEB→TLS array→EXE storage init1/token0x100000000/index0/slot0与静态getter一致。一次手工算错的无关地址1字节读取已弃，不作为证据。
- 实际C++解码器和线程解析器各用私有实机字节回放PASS；首轮私有编译因缺工具链PATH失败，补PATH后成功，非解码失败。重复定义字节不单独证明heap字符串不变或原子性。
- 新增goods_snapshot.h：仅读当前已初始化context，token／slot核验；原生ID映射真实key／分类／local／tradeable。CollectRealEconomy限既有UI线程、安全RPM、采集前后目录比较；去掉goodN默认分类，未知／缺失／变化撤正式owner／文档，全部正式数值含Freight变为waiting／`--`，不归零源订单；不getter／锁／初始化TLS。worker无原生指针。
- 测试含slot3隔离（slot0故意错误）、未初始化／越界／inactive context、token变更、合并源订单／world／revision不变、未知ID／类别拒绝；12项原生回归PASS。完整build后补Freight行live门槛，再完整build-final／verify_embedded PASS；最终候选05b58a2d1b85c61ac952b4c38b9744be16c915f4b84c6b6092dd827a2156293b，30,159,766B，未部署／未跑新DLL，当前R69仍旧元数据。回放不代替DLL运行、原子快照／生命周期／可见层。
- 备份E:\Code\v3-analysis\checkpoints\2026-10-23-r72-live-goods；私有goods-context-r72-notes.md／原始json／回放工具不分发；证据native/build/*round72*.log。MASTER_PLAN §20.69、AGENTS、STATUS、DECISIONS、模型／profile、TASKS／BLOCKERS同步，目标active。

## 2026-10-23 — R71：无人值守推进与商品元数据研究

- 用户回复暂时人机脱离，无需人操作；不再等待人工显示窗口，PID52316 仍 Responding，未终止／暂停／输入或替换运行DLL。
- 本轮完整重读01／02／03、MASTER_PLAN相关段与FREIGHT_MODEL／PROFILE；使用local-reverse-engineering skill。Ghidra当前victoria3.exe、整函数Capstone独立核对flags／key getter／singleton epilogue，证据私有goods-catalog-r71-static.md。初次把ironpe.xrefs位移位置当指令起点产生错误解码，未作为证据；随后统一使用unwind extent整函数解码。未猜文件序号映射。
- Goods定义0x70、ID+0x10、key+0x18、flags+0x40；数据库vector+0x80、count+0x8C、上限128。singleton getter有TLS初始化／锁／cleanup，未主动调用或选第一个slot。x64dbg状态等待120秒后报插件未上线；未附加游戏，未写运行内存。
- 新增header-only copied-memory decoder，返回自有值，类型／ID／字符串／重复key／复制期间变更失败即清空；12th原生测试覆盖SSO／heap、分类优先级、坏指针／ID／类型、字符串错误、数据库和heap中途变化。当前未连接任何正式采集入口，不把离线测试当作ABI／访问窗口验证。
- FreightGood追加tradeable（默认true兼容旧fixture），排除不可交易跨州分配／Freight目标，分类加入fingerprint；测试原始消费不变与同revision分类变化使旧分配失效。真实adapter仍goodN／默认标志，正式元数据排除缺口明确保留。
- 完整build／12项原生回归／verify_embedded PASS，日志native/build/*round71.log；候选698c8b8dc5486c4fc3f3e96086da208deeaf115b1f2d4b352c22ab0d21b08d56、29,840,576B；未部署，运行中仍R69。游戏运行不做standalone selftest/debug_default。
- 源码／文档／旧候选备份E:\Code\v3-analysis\checkpoints\2026-10-23-r71-goods-metadata；MASTER_PLAN §20.68、AGENTS、STATUS、模型／profile同步。总目标保持进行中，后续安全元数据采集、Freight供给／正式UI／生命周期仍未完成。

## 2026-10-23 — R70：真实正式提交／热态采样与文档重复发布修复

- 前轮R69有源码／回归／部署成果，计为进展。本轮检查部署哈希一致且无游戏进程后，通过Start-Process直接启动当前已部署R69、continuelastsave、Hidden，PID52316。启动成功证明此前失败是窗口工具启动接口问题；没有再次使用同一失败接口循环重试。
- 进程Responding、真实采集1117／28554／49、59市场primary118／77州；owner10、37 ribbons、world1、revision1→6、compute约133ms。ImGui启动0顶点，持续SUBMIT scope0/1/0。window tool list_windows/list_apps未返回游戏，因为本次Hidden启动未暴露窗口；已异步请用户显示窗口，未发UI输入／未强制结束或替换运行DLL。
- 日志副本 runtime-round70.log；独立提取 runtime-summary-round70.json：36个SUBMIT样本，冷16uploads，热态35样本170.453秒uploads37恒定，submitted7966→303596。初次摘要断言因包含冷阶段16而失败，改为明确报告冷阶段及最后一次上传变化后的连续热态区间，保留所有原始样本。
- 6次采集对应349次文档发布，确认LatestWorker::Poll返回retained reply，Session每500ms重建同一document。新增断言先FAIL（repro-round70.log），再在snapshot获取和最终提交两处用m_lastApplied去重；不清除当前render frame，不抑制新revision／owner／reopen。
- 重读01／03全文并复核MASTER_PLAN §10性能／缓存／日志；完整build、11项native回归、verify_embedded PASS。候选R70 `f3a9a4befc8db63e6f2562ba4a71be6e38b447055aecc38d98756ef440c5e7e3`（29,840,576 B），未部署；运行中仍R69 `17268f5e…`。未执行会写游戏日志的standalone selftest/debug_default。
- 备份 E:\Code\v3-analysis\checkpoints\2026-10-23-r70-runtime-publication；新增证据与审查diff在native/build/*round70*。MASTER_PLAN §20.67、AGENTS、STATUS、PERFORMANCE／ACCEPTANCE同步。完整地图可见／市场探针／读档／Freight供给范围仍未验收。

## 2026-10-23 — R69：世界重置与异步发布门控修复

- 本轮重新完整读 01／02／03，重点 Architecture／Language-neutral control flow／Performance requirements／生命周期；复核 MASTER_PLAN §11 和 Engine／Worker／Session／Overlay。前轮 R68 是已完成静态证据与关闭态探针，计为进展。
- 进程检查无 victoria3；sky.list_windows 无游戏窗口；本轮 sky.launch_app 仍报 accessibility window-opened handler did not become ready。未继续重复启动，转为修复独立生命周期缺口。
- 新测试先运行旧实现，`repro-round69.log` 得到 FAIL: reset retires the previous world even while no snapshot exists，确认真实反例。
- Engine／Session 独立保存 world／revision 门槛，reset 退休整个 world，高 revision 也不能复活；worker reset 使用独立原子失效代数，后续 Cancel／合并不会吞掉清缓存指令。Session 串行提交与 reset，快照更新立即撤帧，发布前复核 snapshot 身份、owner、filter、generation；处理 mailbox 外层异常。
- render 通过 FrameIsCurrent 检查上传／提交；修复 OnRenderPass 的 ImGui 可见性残留门槛。实时 source world 失效时递增，刷新前检查旧 anchor；旧 reader 累计样本不再注入正式快照，仅作落盘诊断。
- 原生对象跨 world 的注册／析构仍未知，新增临时停用门槛：检测 world 替换后不再访问／提交旧 native 槽，UI 提示重启恢复地图。数字层可接受新 world；不能把此安全保护或 anchor 回退检测算作完整 A24。
- 最终 11 项原生回归 PASS（tests-round69-final.log）；完整构建 build-round69-checked.log、GUI runtime、verify_embedded、selftest（560192 bright pixels）、debug_default（PID 46344，0 UI vertices）均 PASS。修改 overlay 最后增加强制 anchor 校验后已重新完整构建，未沿用过时对象。
- 候选／已部署 DLL `17268f5ea3074e6072b3b68d7b64c29b837987cf0ebd1ff36b59fdb0e3bb3bba`（29,839,552 B），deploy.ps1 成功；旧安装 R67 `52009510…` 保存为 checkpoint/installed-version.dll，源码与 R68 候选同目录备份。
- 记录同步 MASTER_PLAN §20.66、AGENTS、STATUS、DECISIONS、TEST_PROCEDURE。备份 E:\Code\v3-analysis\checkpoints\2026-10-23-r69-world-reset；实机市场／真实 native 可见性／读档验收仍 NOT_RUN，总目标保持 active。

## 2026-10-23 — R68：市场 getter 静态契约与关闭态探针

- 重读 01／02／03 原始文档，重点 Native discovery、Architecture and ownership、Known symptoms；复核 MASTER_PLAN §8–9、UI 方案与 native profile，保持未验证能力关闭。
- 使用 local-reverse-engineering 技能。Ghidra 初始 server_status 为 disconnected；list_instances 发现本机 8089 后端，随后 list_open_programs 成功，显式选择 victoria3.exe（另有无关程序，未操作）。分析副本与安装 EXE SHA256 相同 `7f53…f811f`。
- Ghidra 确认 GetMarket getter `0x225AC90` 的参数与 wrapper 引用计数；resolver `0x10571F0` 可加锁／分配，constructor `0x1CD6DC0` 写入 wrapper ID+0x10。MCP disassemble_function 返回空，改用本机 PE＋Capstone 独立解码 getter 和脚本 callback，复核寄存器、栈和返回值，未将空结果算验证成功。
- 新增默认 0 的 `probe_panel_market` 及持久化，getter hook 只有 hash／前缀匹配时可启用；原函数一次调用、out／RAX 不变，调用窗口内安全复制 panelId／wrapperId，至多 16 个样本，不保留指针，不改变正式选择。
- build／verify_embedded／debug_default PASS（PID 38840，0 UI vertices）；候选 `09b219a9e596f999bd3e2182672b2e551b2fa85e450f266b2297bdf64af27fd0`（29,877,463 B）。本轮未部署、未启动游戏，已部署 R67 保持不变。
- 静态证据 `E:\Code\v3-analysis\panel-market-r68-static.md`；日志 `native/build/*round68.log`；源码／DLL 备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-r68-market-getter`。动态 ID 对照、切换、线程／生命周期仍 NOT_RUN；下一步以最小实机样本证伪，不直接启用正式路径。

## 2026-10-23 — R67：Freight 中转需求核算与原版 UI 文本桥

- 本轮重新完整读 01／02／03 原始文档，重点货流、线程／所有权、缓存及 UI 生命周期；复核模型／原版 UI 方案／PROFILE 与源码。目标：将 §4.5 的需求核算做成真实接口边界测试，保留未满足目标、不重命名工作量充数。
- 实现：分配前构建逐州／市场／商品 Freight 目标；分解最终货流后填写已满足／未满足；独立转换系数默认 1（项目假设），local-only 和 Freight 自身不递归。复制 view／document 保留总量及商品贡献，系数／标志入缓存指纹。原版增加第九个长 key 变量 `ironroad_demand_v1`，不接收合成数据，缺失为 `--`。
- 验证：完整 build、11 项原生回归及追加几何身份测试、test_native_gui、native_gui_runtime、verify_embedded、selftest（560181 bright pixels）、debug_default（0 UI vertices）PASS；测试已接既有 harness，无新增孤立测试入口。
- 失败记录：首次新增测试用了 float 的 INFINITY／NAN 与 double 混合初始化，编译失败；改为显式 double 数组后 PASS。归档测试首次与 DLL 构建并发，读取旧 DLL 后资源不一致；这是依赖排序错误，构建完成后重跑原测试 PASS，未放宽断言。
- 部署：游戏关闭，deploy.ps1 成功。候选／安装哈希 `52009510d0ff6c64af6e03b77ab109c7d9c6d5bafd56ce101116a2b6fb1c7bd9`，29,863,251 B；旧版本／源码备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-r67-freight-demand`。
- 实机：computer-use 技能初始化后，`sky.launch_app` 两次均报 `accessibility window-opened handler did not become ready`；随后 list_windows 无游戏窗口，进程检查无 victoria3。停止重试启动，新行实机 NOT_RUN，未终止游戏或改存档。
- 剩余：独立 Freight 供给机制、模型内 vs 游戏内商品范围仍未定；异步问题尚无答复。只完成需求核算层，不据此标记整个 A27 或总目标完成。其余市场／铁路／hover／生命周期缺口保留。

## 2026-10-23 — R66：Freight 中转商品需求入册

- 用户原话：“注意用Freight作为中转商品来计算物流需求，写入master_plan”。
- 对照 `MASTER_PLAN.md` §4.1–4.4、`FREIGHT_MODEL.md`、`native/src/freight.h`：现有 `Shipment::freight` 是货运工作量，未定义独立中转商品需求。
- 已在总任务书新增 §4.5、P4 链路与 A27；同步 AGENTS、STATUS、TASKS 与模型说明。明确公式／供给机制尚待设计，保留观察性边界和 allocated quantity 线宽规则。
- 检查：六份 Markdown UTF-8 无 BOM、需求引用一致；DLL 哈希未变。无代码／构建／实机验证，不能把需求入册写成模型已实现。
- 备份：`E:\Code\v3-analysis\checkpoints\2026-10-23-r66-freight-intermediate`。

## 2026-10-24

### 02:46–03:05 R65：正式 UI 市场对比截图（UK=79）

**实机**
- 自动模式：`model allocation (market 118, auto)`、`11728.46`、`16440.38 / 1862.16 / 14578.22`，37 ribbons。
- `market_override=79`：`model allocation (market 79)`（无 `, auto`）、`601.13`、`7465.20 / 451.99 / 7013.21`，47 ribbons（r65-uk-ribbons.png）。
- 当前缩放下地图未清晰看到 ribbon（market 79 含海外州，分布分散），页签数值对比已确认作用域切换。

**验证**
- 候选/已部署 `200f08d2d738d829afa47097758f14d6250cfcd20c9945cda324d714bd4de134`；`verify_embedded` PASS；`build_tests` 11 项 PASS；测试后 `market_override=0`。

## 2026-10-24

### 02:11–02:45 R64：脚本函数注册表探针（负结果）

**研究**
- Ghidra 反编译 `0x3AB6D00`/`0x3A7E440`/`0x3A7A9D0`：注册表在固定全局 `0x1459FD648`，条目 0x78 字节，size 在 `+0x08`（实测 141），名字不在 `+0x40`。
- 新增 `probe_registry`（默认 0）：读表头/条目、尝试名字偏移；实机 `hdr=... 8d ... sizeA=141 sizeB=0`，未提取出任何条目名（偏移/字符串布局不符），未发现 market 函数。

**结论**
- 该注册表路径未能给出市场函数地址；玩家市场自动识别仍未完成。探针默认关闭。

**验证**
- 候选/已部署 `200f08d2d738d829afa47097758f14d6250cfcd20c9945cda324d714bd4de134`；`verify_embedded` PASS；`build_tests` 11 项 PASS；测试后 `probe_registry=0`。

## 2026-10-24

### 01:51–02:10 R63：全新缓存重装演练

**步骤**
- 备份并删除 `%LOCALAPPDATA%\IronroadV3`；`deploy.ps1 -Uninstall` → `deploy.ps1` 重装同一候选。
- 启动：DLL 自动重建缓存（`data/`、`assets-*.zip`、`native-gui-*.zip`、`ironroad.ini`、日志）。
- F4+Shift+5：页签正常显示 `model allocation (market 118, auto)`、`11728.46`、`16440.38 / 1862.16 / 14578.22`、`Markets (id:states): 118:77 ...`（r63-fresh.png）。
- 日志：`real economy markets=59 primary=118`、`freight view ribbons=37 compute=141.431 ms`；无错误/变量失败。

**验证**
- 候选/已部署 `c69340f169f680346a1643409995bfc602f38d6ff57b3d643ab2324d11b3a16a`（29,771,621 B）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 备份保留在 `%TEMP%\IronroadV3-backup-*`（未使用）。

## 2026-10-24

### 01:26–01:50 R62：页签市场概览行

**实现**
- 采集时生成前五市场摘要（`id:states`），经互斥串保存；新 GUI 变量 `ironroad_markets_v1`（第 8 个变量）发布。
- `ironroad_logistics.gui` 新增一行：`Markets (id:states): ...`，让玩家在游戏内直接看到可选市场并设置 `market_override`。

**实机**
- 页签显示 `Markets (id:states): 118:77 164:75 35:74 33:68 34:58`；`UI-thread variables published` 无失败；37 ribbons、136.2 ms（r62-markets.png）。

**验证**
- 候选/已部署 `c69340f169f680346a1643409995bfc602f38d6ff57b3d643ab2324d11b3a16a`（29,771,621 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS；`test_native_gui.py` PASS。

## 2026-10-24

### 01:06–01:25 R61：完成审计与实现报告刷新

**更新**
- `COMPLETION_AUDIT.md` 重写为 R61 状态：逐项列出当前证据与缺口（正式页签实时数据、market 作用域/override、内存/性能、回滚；缺口：玩家市场自动识别、铁路、hover/filter、真实读档）。
- `IMPLEMENTATION_REPORT.md` 顶部新增 “Current status (R61)”：当前候选哈希、实时市场作用域数据、已知缺口；旧章节保留为历史。

**验证**
- 文档 UTF-8 校验通过；无源码改动，DLL 仍为 `0cb83a45…5672d56`。

## 2026-10-24

### 00:51–01:05 R60：自动市场标注

**实现**
- 页签状态：自动选择时显示 `model allocation (market N, auto)`；`market_override>0` 时显示 `model allocation (market N)`，让玩家一眼看出市场是自动挑的还是手动锁定的。

**实机**
- 自动模式：`model allocation (market 118, auto)`、`11728.46`、`16440.38 / 1862.16 / 14578.22`（r60-auto.png）；37 ribbons、138.8 ms。

**验证**
- 候选/已部署 `0cb83a456e3b6237db2b36574848ac0e6f87ef3c96890d7edc5ff11d51672d56`（29,752,211 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。

## 2026-10-24

### 00:31–00:50 R59：市场提示日志 + 设置持久化补齐

**实现**
- 采集日志新增前三市场：`real economy markets=59 primary=118 states=77 top=118:77 164:75 35:74`。
- `SaveSettings` 补写 `probe_market` 与 `market_override`（此前保存会丢）。

**实机**
- `top=118:77 164:75 35:74`；37 ribbons、137.8 ms；`market_override` 仍为 0。

**验证**
- 候选/已部署 `065bbf34db83f4843aafc29095e7f4692d5a6b1629866ebcc45a4efa0f66aa85`（29,750,126 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。

## 2026-10-24

### 00:11–00:30 R58：README 用户文档刷新

**更新**
- 修正标题乱码（`鈥?` → `—`）。
- “Native UI integration and live freight data are still being completed” 等过时描述改为当前状态：正式市场第五页签、实时市场作用域数据、Show/Hide、商品列表。
- 新增 “Logistics tab semantics”：order-cost 口径、target/allocated/unmet 定义、`model allocation (market N)` 标注、`market_override` 用法、模型分配非游戏订单/MAPI 的声明。
- Controls 段更新为正式页签 + 调试面板两套入口。

**验证**
- README 全文件 UTF-8 有效；无源码改动，DLL 仍为 `a98e45b5…163a394`。

## 2026-10-23

### 23:46–00:10 R57：市场归属核对（发现默认启发式偏差）

**探针**
- `probe_market` 增加 `eco market id=` 行：17/19=**79**（威尔士/高地=UK）、132=71（莫斯科）、350=83、201=64、205=33、432/436=164。
- 变量表探针：游戏变量表仅含我们的 7 个 `ironroad_*` 键，无市场上下文变量。

**结论**
- 默认 primary=118（州数最多，77 州）**不是 UK 市场**；UK 市场为 **79（30 州）**。
- 因此“州数最多”默认对本存档偏差；`market_override=79` 可得到 UK 市场（R54 已验证 7465.20/451.99/7013.21）。
- 玩家市场自动识别仍未完成；文档/页签标注市场 id 以提示。

**验证**
- 候选/已部署 `a98e45b56e08d7f8960e308797d2aaf351fb3e63925e8d2491909f5ab163a394`（29,632,396 B）；`verify_embedded` PASS；`build_tests` 11 项 PASS；测试后开关恢复默认。

## 2026-10-23

### 23:21–23:45 R56：脚本注册探针（负结果）

**实现**
- 新增 `probe_register`（默认 0）：MinHook `0x3AB6D00`（脚本/GUI 注册助手），记录名称含 “market” 的注册及回调地址。

**实机**
- 钩子安装成功（`script register probe installed at 00007ff71a196d00`），但**未观察到任何 market 注册**；game 启动/注册早于钩子安装或该助手不是脚本函数注册路径。

**结论**
- 无法用该路径取得市场面板 market；玩家市场自动识别仍未完成，继续使用“最大市场 + `market_override`”。

**验证**
- 候选/已部署 `227793796c7413948ca97af290a258f69b25a6a1b078983eae2f428756046e1c`（29,630,860 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS；测试后 `probe_register=0`。

## 2026-10-23

### 23:01–23:20 R55：面板开关循环稳定性

**实机**
- 连续 10 次 F4 打开/ESC 关闭市场面板（偶数次插入 Shift+5 切物流），进程存活、无错误/无 `world changed`。
- 周期刷新继续：rev 1→2、gen 1→2、37 ribbons、135–137 ms。
- 结束后重新打开：页面正常显示 `model allocation (market 118)`、`16440.38 / 1862.16 / 14578.22`（r55-cycles.png）。

**验证**
- 候选/已部署 `f02169a82b3153c262577ba85a6505cf40a9e968d64c537956cc33e2308a66ea`（29,625,098 B）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 无源码改动；测试后关闭游戏、开关恢复默认。

## 2026-10-23

### 22:41–23:00 R54：market_override 可配置市场

**实现**
- 新增 ini 设置 `market_override`（默认 0）：0 表示沿用“州数最多”启发式；>0 时锁定该市场 id（若存在）。

**实机（market_override=79，UK 本土市场）**
- `real economy markets=59 primary=79 states=30`；`freight view ribbons=47 compute=143.469 ms`。
- 页签：`model allocation (market 79)`、`Freight (order-cost): 601.13`、`Target/allocated/unmet: 7465.20 / 451.99 / 7013.21`（r54-override79.png）。

**结论**
- 玩家市场的自动识别仍未完成（默认取最大市场 118/77 州）；`market_override` 提供可控修正，页签标注实际市场 id。

**验证**
- 候选/已部署 `f02169a82b3153c262577ba85a6505cf40a9e968d64c537956cc33e2308a66ea`（29,625,098 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS；测试后 `market_override=0`。

## 2026-10-23

### 22:06–22:40 R53：market 作用域突破（state+0x48 = market id）

**发现**
- 用已验证的同市场样本（威尔士 id17、苏格兰高地 id19）对比莫斯科 id132：`state+0x48` 前两者均为 0x4f(79)、后者 0x47(71)。
- 对全部 1117 州统计：**59 个不同值**，聚集明显（118×77、164×75、35×74…）→ 判定 `state+0x48` 为 **market id**。

**实现**
- 采集时读取 `state+0x48` 作为 `StateOrders.market`（不再统一 market=1）；统计各市场州数。
- `EconomySnapshot.primaryMarket` = 州数最多的市场；`LogisticsSession::FirstMarket`/`PublishRealEconomy` 优先使用它。
- 页签状态改为 `model allocation (market N)`。

**实机**
- `real economy markets=59 primary=118 states=77`；`freight view ribbons=37 compute=136.396 ms`。
- 页签：`model allocation (market 118)`、`Freight (order-cost): 11728.46`、`Target / allocated / unmet: 16440.38 / 1862.16 / 14578.22`（r53-final.png）。

**注意**
- primary 取“州数最多”的市场是启发式，不保证等于玩家市场；下一步需从首都州/GUI 上下文取玩家市场。

**验证**
- 候选/已部署 `371d2db7ed57dcd20fc11ced2d76ba901a3881629d1ada0a45ad4e249dabc42f`（29,618,209 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。

## 2026-10-23

### 21:41–22:05 R52：全局作用域标注 + 州面板映射观察

**实现**
- 正式页签状态从 `model allocation` 改为 **`model allocation (global)`**，如实标注当前快照为全局作用域（market 分组未完成）。

**实机**
- F4+Shift+5：页签显示 `model allocation (global)`、`Freight (order-cost): 126318.85`、`Target/allocated/unmet 519674.48 / 93784.10 / 425890.38`（r52-global.png）。
- 地图点击打开州面板：`威尔士`（positions id 17）、`苏格兰高地`（id 19）名称与 positions 表一致；尝试通过州面板“本地价格”页签触发读取器以核对运行时 id，坐标未命中、未取得 `eco` 日志，映射仍未由 reader 证实。

**验证**
- 候选/已部署 `5db7d577ce9375a2104fd705e1867f2ed1029aa493aeea89fa9c38d0346559a4`（29,312,141 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，开关恢复默认。

## 2026-10-23

### 21:06–21:40 R51：市场容器头探针（负结果）+ 验收矩阵更新

**探针**
- 新增 `probe_market`（默认 0）：采集后输出 id 16/17/18/132/432/205/201/350 的 `state+0xC8/+0x78` 容器头 8 qword。
- 结果：16/17/18 与 132/432 的容器头**没有共享的市场指针/ID**；`h[0]/h[3]` 为静态地址、`h[1]` 为每州数组、`h[4]` 为每州标志位，均不可作为市场分组键。
- 结论：market 作用域仍无法从 state/容器直接获得；维持全局 `market=1` 并在验收矩阵标注。

**文档**
- `docs/transport/ACCEPTANCE.md` 重写为 R51 状态：A03/A07/A13/A15/A21/A22/A23/A25/A26 PASS，A06/A10/A12/A16/A17/A18/A19/A20/A24 PARTIAL；明确 market/rail/hover/owner 缺口。

**验证**
- 候选/已部署 `a25f78ae7006562a826fa3992339d64743d3877e4173656dd7cd91aa6fd7ea45`（29,311,629 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS；测试后 `probe_market=0` 等全部恢复。

## 2026-10-23

### 20:41–21:05 R50：GUI 补丁镜像修复与全量回归

**发现**
- `tools/test_native_gui_runtime.ps1` 失败：`AssertionError: gui/market_panel.gui`。
- 根因：Python `patch_market`（`tools/native_gui.py`）自 R21/R22 起未同步 C++ 补丁：内容块结构不同、缺少 FILTERS/搜索栏/过滤器容器的物流页签隐藏。

**修复**
- 将 Python `patch_market` 逐字对齐 C++（container 包裹、`ironroad_market_logistics_content = {}`、FILTERS/搜索栏 `visible`、过滤器容器 `And(...)`）。
- 运行时 GUI 归档校验恢复：**exact Python/native patch agreement**。

**全量回归**
- `test_native_gui.py` PASS；`test_native_gui_runtime.ps1` PASS；`test_flow_gpu.py` PASS；`selftest.ps1` PASS（560340 bright pixels）；`test_debug_default.ps1` PASS（默认零 UI 顶点）；`verify_embedded` PASS；`build_tests` 11 项 PASS。

**验证**
- DLL 未改动（`5e74477a…4691fc`）；测试后关闭游戏。

## 2026-10-23

### 20:06–20:40 R49：世界失效演练（安全注入）

**实现**
- 抽出 `InvalidateEconomy(why)`：清 collected/anchor/vtable、`ResetWorld`、清空文档；`CheckWorldLifecycle` 与调试注入共用。
- 新增 ini 调试键 `test_world_change`（默认 0）：采集完成后注入一次失效，验证真实代码路径。

**实机（test_world_change=1）**
```
real economy collected states=1117 orders=28554 goods=49
TEST world-change: invalidating economy snapshot
real economy collected states=1117 orders=28554 goods=49
freight view gen=3 world=1 rev=2 ribbons=989 compute=139.145 ms
```
- 失效 → 重新采集 → 新 revision/gen，约 5 秒内恢复。

**真实重载自动化**
- 暂停菜单“载入游戏”入口打开的是国家预览界面；为避免误开新局，未点击“开始”，未修改存档。

**验证**
- 候选/已部署 `5e74477a73f50e0924f72b449ab2986f8b619d698d623a91e23544e9984691fc`（29,309,069 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`test_world_change=0` 等全部恢复。

## 2026-10-23

### 19:41–20:05 R48：内存平台与周期刷新稳定性

**测量**
- 打开市场物流页签，周期刷新运行中，每 10 s 采样 WorkingSet，共 18 次（3 分钟）。
- 采样：7286.6 → 7302.4（峰值）→ 7278.0 MB；warm 区间 **7278–7302 MB（±24 MB，约 0.3%）**，后段下降，无持续上升趋势。
- 刷新日志：rev 5→6、gen 5→6，989 ribbons，compute 137.1/139.9 ms。

**结论**
- 无内存泄漏迹象（3 分钟平台）；注意 WorkingSet 含游戏基线（约 7.3 GB），mod 的增量未单独隔离。

**验证**
- 构建未改动，候选/已部署 `b19e8308fb1ac83cddda35a4c2b8ff73812e7039f4d5a7017d988798268eee97`；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，开关恢复默认。

## 2026-10-23

### 19:06–19:40 R47：回滚部署演练（A26）

**步骤**
1. `deploy.ps1 -Uninstall` 移除 DLL（保留 `%LOCALAPPDATA%\IronroadV3` 缓存/配置）。
2. 启动 `victoria3.exe -gdpr-compliant -continuelastsave`：**无 DLL 时正常进入地图**（r47-nodll.png，无物流页签/箭头）。
3. 关闭游戏，`deploy.ps1` 重新安装 DLL（SHA-256 与候选一致）。
4. 再次启动：`real economy collected states=1117 orders=28554 goods=49`、`freight view gen=1 ribbons=989 compute=144.216 ms`，mod 正常。

**结论**
- 部署/回滚可逆，缓存不影响回滚；回滚后重新部署功能恢复。
- A26 回滚演练通过（单 DLL 交付验证）。

**验证**
- 构建未改动，候选/已部署 `b19e8308fb1ac83cddda35a4c2b8ff73812e7039f4d5a7017d988798268eee97`；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，开关恢复默认。

## 2026-10-23

### 18:41–19:05 R46：周期刷新 + 重载演练尝试

**实现**
- 每 30 秒（60 个 500ms tick）把 `m_economyCollected` 置 false，触发重新采集；新快照 revision+1，worker 后台重算后替换视图。
- 实机（无 probe）：rev=1 → rev=2、gen=1 → gen=2，均 989 ribbons，compute 141.7/144.9 ms。

**重载演练尝试**
- 暂停菜单可用 Esc 打开；自动化点击定位到错误行（进入“切换国家”界面而非载入对话框）。
- 未选择任何国家、未保存、未修改存档；随后关闭游戏。

**结论**
- 周期刷新工作正常；存档重载/主菜单往返的端到端演练仍需更可靠的操作方式（待补）。

**验证**
- 候选/已部署 `b19e8308fb1ac83cddda35a4c2b8ff73812e7039f4d5a7017d988798268eee97`（29,303,893 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，全部开关恢复默认。

## 2026-10-23

### 18:11–18:40 R45：世界变化的生命周期失效

**实现**
- 采集时记录首个州对象指针（anchor）与 vtable。
- `CheckWorldLifecycle()` 每 ~2 秒复核 `lookup(anchorId)`：指针/vtable/id 任一不匹配 → 判定世界已替换，置 `m_economyCollected=false`、清 anchor/vtable、`LogisticsSession::ResetWorld()`、发布空文档（页签回到 waiting，等待重新采集）。
- 防误报：20 秒实机运行无 `world changed`，数据保持（1117 州 / 28554 订单 / 49 商品，989 ribbons，137 ms）。

**未完成**
- 真机存档重载/主菜单往返的端到端演练未执行（暂停菜单自动化点击不稳定）；代码路径已就绪。

**验证**
- 候选/已部署 `f447b9b754b9dceab3b93a027be0c413b155152d5053e2442f2458dfabd9349c`（29,302,357 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，全部开关恢复默认。

## 2026-10-23

### 17:31–18:10 R44：默认真实数据（无需任何 probe 开关）

**实现**
- 新增 `Overlay::CollectRealEconomy()`：UI 线程定时器内一次性枚举全部州（id 0..2047，按 vtable 频率 + 容器数据探测选出真正的 state 类），读取 128 商品的生产/消费，构建 `EconomySnapshot` 并 `PublishRealEconomy`。
- 注册表尚未就绪时返回并每 5 秒重试（日志 `real economy probe pending` / `real economy retry`）。
- 移除了真实数据对 `probe_economy` 的依赖：`PollHideFlows` 始终轮询会话/发布文档。

**实机（probe_economy=0）**
- `real economy collected states=1117 orders=28554 goods=49` → `snapshot accepted` → 会话 owner=10、frame=989 ribbons。
- 物流页签（无任何调试面板/开关）显示：`model allocation`、`Freight (order-cost): 126318.85`、`Target / allocated / unmet: 519674.48 / 93784.10 / 425890.38`。
- 日志：`freight allocation flow=913.80 decomp=12.30 shipments=7016 threads=8`、`freight view gen=1 ribbons=989 compute=140.695 ms`。

**验证**
- 候选/已部署 `0a21fdb1890e53cec79b8a81512a76745c1c57e0d93304848bcf086e2242dad0`（29,300,253 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，全部开关恢复默认。

## 2026-10-23

### 17:01–17:30 R43：分配并行化（741→127 ms）

**实现**
- 把 `AllocateFreight` 的按组（market, good）求解体抽成 `runGroup` lambda；用 `std::thread` 池（最多 8 线程）+ 原子任务索引并行执行。
- `result.balances` 的写按组互斥（每组独立索引），`partials[j]` 各自收集 shipments，join 后合并。
- 线程内异常用 `exception_ptr` 捕获并在主线程 `rethrow`，恢复 cancelled/错误语义（首版未捕获导致 `terminate: cancelled`，已修）。

**实机**
- `freight allocation flow=893.20 decomp=12.62 shipments=7016 threads=8`（线程 CPU 累计）。
- 墙钟：`allocation=126.94 ms view=6.44 ms total=133.38 ms`，ribbons=989。
- 相比 R42 的 741 ms，**约 5.8× 加速**；UI 全程可交互。

**验证**
- 候选/已部署 `df10b6c29abb77cfe587c8b65d44a65f1800583ebb84056d93404f0f156b0f06`（29,182,830 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`probe_economy=0` 等全部恢复。

## 2026-10-23

### 16:21–17:00 R42：分配性能剖析与首个优化

**剖析**
- `FreightEngine::Build` 增加分段计时：`allocation` vs `view`。
- 实机：`allocation=741.30 ms view=10.73 ms total=752.03 ms`。
- 在 `AllocateFreight` 内细分：`flow=729.03 ms decomp=11.16 ms shipments=7016`。

**优化**
- 分解阶段原先对每个 BFS 节点线性扫描全部 importer（O(nodes×importers)/shipment）；改为 `importerAt[node]` O(1) 查表。
- 结果：分解耗时降到 11 ms；分配总耗时 769→741 ms（瓶颈明确在 **min-cost flow 增广**）。

**结论**
- 下一步优化方向：减少增广次数（按 exporter/importer 聚合/贪心运输）、并行 per-good 分组、或更快的最短路。

**验证**
- 候选/已部署 `f85067d9fb6df0f33ad8f09eb8c588337c8c151aec6c281b59f19c33f16bed87`（28,966,428 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`probe_economy=0` 等全部恢复。

## 2026-10-23

### 15:46–16:20 R41：真实数据 ribbon 上屏（关键打通）

**修复**
- `ApplyPendingFreight` 原先要求 ImGui 面板可见（`m_visible`）才提交 ribbon；改为**原生 owner 活跃时也提交**（`status.owner == kOwnerNativePanel`），pump 取消条件同步调整。

**实机**
- 真实帧：`freight view gen=1 world=1 rev=1 ribbons=989 compute=768.119 ms synthetic=0`。
- Diagnostics：`native pool 989/8192`、`visible yes 999`、`route uploads 989`、`last freight 989 ribbons`。
- 地图上出现真实数据的细线 ribbon（宽度档低，因为全局 1117 州摊薄了单条流量）。

**性能**
- 分配+视图 compute **768 ms**（后台 worker，非渲染线程）；UI 全程可交互。
- 989 条 ribbon 上传/渲染正常，8192 池未溢出。

**验证**
- 候选/已部署 `b6582a238252bb0abedb69c19f05a21008adb31552d0e04a053cb0cc423630b4`（28,929,209 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`probe_economy=0` 等全部恢复。

## 2026-10-23

### 15:06–15:45 R40：market 字段追踪（负结果）

**实验**
- 对英国本土 16/17/18、海外 205 与外国 132/432 的 state 对象 0x200 字节做全量对比（qword 与 dword），寻找“英国州相同、外国不同”的字段。
- 进一步读取 0x80/0xa8/0xd0 指向对象的 0x80 字节并做同样对比。

**结果**
- **没有任何字段**满足“16/17/18 相同且与 132/432 不同”（qword 与 dword 均无）。
- 说明 market 不是 state 的直接字段，也不是这三个指针目标中的直接值；更可能通过**所有者国家/省份**间接获得，或 16/17/18 的 id 映射与假设不符。

**结论**
- 需要先验证州身份（运行时名称/所有者），或 hook 市场面板的 market 对象；当前快照仍为全局 market=1。

**验证**
- 候选/已部署 `a6fe068d0d76205cf2d701066ce7e38f37a78959add8b4bfc272f02659df6a2d`（28,928,697 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`probe_economy=0` 等全部恢复。

## 2026-10-23

### 14:31–15:05 R39：绕过可见页限制，枚举全部州

**实现**
- reader 钩子内新增一次性**全州枚举**：记录首个真实州的 vtable，遍历 id 0..2047，用 `lookup(id)` + vtable 相等 + `state+8==id` 三重校验，再按已知容器布局（state+0xC8/+0x78、位图+数组）读取 128 个商品；值过滤 `<1e12`。
- 枚举结果写入 `econStates` 并直接 `writeDump()`/发布快照，不再依赖“本地价格”页签的可见页。

**实机**
- `# states=1117`（与 registry 数量一致，但值为有效量级）：`totalProd=88,936,587,381`、`totalCons=94,231,898,026`（≈889,366/942,319 单位，1e5）。
- 物流页签：`Freight (order-cost): 126318.85`、`Target / allocated / unmet: 519674.48 / 93784.10 / 425890.38`。
- 分配率从 5.7% 提升到 **18.0%**；数据覆盖不再受首屏 100 行限制。

**结论**
- 真实数据采集问题已解决；剩余低分配率主要来自 market=1 混样与铁路缺失。

**验证**
- 候选/已部署 `d98d815b8f46520b056168489d2d2b1725d3f47099c1957880f2b52448e1566a`（28,922,465 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`probe_economy=0` 等全部恢复。

## 2026-10-23

### 13:56–14:30 R38：州对象字段扩展转储；本地价格页签非市场作用域

**实现**
- reader 钩子对 12 个州 id（16/17/18/19/21/23 英国本土 + 201/205/132/350/432/436）转储 state 前 0x200 字节（64 qword），一次每州。

**实机**
- 实际只出现 6 个州：205/201/350/132/432/436；**英国本土 16/17/18/19/21/23 均未被 reader 访问**。
- 说明商品面板“本地价格”页签不是当前市场，而是按价格/销量排序的**全球子集**（首屏 100 行，含殖民地/外国州）；不能用它推断市场归属。
- 州对象 0x00–0x100 未见明显 market 字段（0x80/0xa8/0xd0 为每州数据指针，0xb0 为类别计数）；0x100 之后仍需分析。

**结论**
- 需要**已知同市场样本**（英国本土州 16–23）才能定位 market 字段；下一轮从州面板/州列表触发这些州，或静态分析州对象的 market 链接。

**验证**
- 候选/已部署 `ad0a4dc14f10a34ae70e966522ff89fcbf48c69148a546f769accdda48b14fce`（28,920,642 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`probe_economy=0` 等全部恢复。

## 2026-10-23

### 13:21–13:55 R37：state→region 映射确认；发现跨市场混样

**发现**
- 运行时州 id **等于** `data/positions.txt` 的 region id：
  201=Timbuktu、205=Windward Coast、432=Ankara、436=Azerbaijan、132=Moscow、350=Western Cuba。
- 790/940 在 positions **不存在** → 无 authored hub，模型不可达。
- 州对象前 0x100 指针/CString 扫描未发现 `STATE_` 字符串（key 不在州对象直接字段）；但映射已不需要 key。
- **关键**：本地价格页签列出的 100 个州横跨多国（Timbuktu/Moscow/Ankara...），说明该列表是**全球州**而非单一市场；我们把它们全部标成 market=1，导致跨市场混样与低分配率（5.7%）。

**结论**
- region 映射正确；缺的是**州的市场归属**。下一步需定位州对象的 market 字段/指针，按市场分组后再建快照。

**验证**
- 候选/已部署 `8feb3b81c2e8f625aac88302cd5259c082d70ccd1ee60cfe21cd8d5d16c56ea1`（28,904,528 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`probe_economy=0` 等全部恢复。

## 2026-10-23

### 12:51–13:20 R36：Freight 口径/标签修正

**问题**
- `LogisticsDocument::allFreight` 是 `shipment.freight` 之和（milli-orders × cost），不是订单数；页签标签 “Freight (orders)” 误导，数值 23,108,927.60 偏大。

**修正**
- 显示值改为 `allFreight / kQuantityScale`（→ 23,108.93，订单×成本口径）。
- GUI 标签改为 `Freight (order-cost):`；不改变 `allFreight` 字段本身，测试断言不受影响。

**实机**
- 物流页签显示：
  `model allocation`
  `Freight (order-cost): 23108.93`
  `Target / allocated / unmet: 259398.15 / 14746.49 / 244651.66`
  `Built rail: --`。
- 分配率仍低（14,746/259,398 ≈ 5.7%），指向 state→region 映射/铁路缺失问题。

**验证**
- 候选/已部署 `75cc6de5ef452761f4897cf223cad79ec5ff11d6554821d3c301aa93f5a50f74`（28,895,740 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`probe_economy=0` 等全部恢复。

## 2026-10-23

### 12:06–12:50 R35：正式 UI 显示真实模型分配（关键里程碑）

**根因**
- `SetGuiVariable` 的 insert-or-find 对**已存在 key 不更新值**：首次写入 `waiting for economy data` 后，后续真实值被忽略，页签一直显示 waiting。

**修复**
- 写入后在 entry 上通过引擎 `assign` 更新 value 字段（entry+0x28），实现“存在则更新”。
- 实机：真实文档持续发布、变量持续更新，**进程无崩溃**。

**实机结果**
- 物流页签显示：
  `model allocation`
  `Freight (orders): 23108927.60`
  `Target / allocated / unmet: 259398.15 / 14746.49 / 244651.66`
  `Built rail: --` + 货物列表。
- 这是**真实经济数据 → 模型分配 → 原版市场面板**的完整链路首次可见。

**验证**
- 候选/已部署 `77a3efd064b28d5b2cb27dd23f168a2d53a45f2674644b3811711906f3939b0c`（28,895,740 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`probe_economy=0` 等全部恢复。

## 2026-10-23

### 11:26–12:05 R34：真实快照接入会话，生成模型分配帧

**实现**
- `writeDump` 同时把采集数据映射为 `EconomySnapshot`（state=stateId、region=stateId、market=1、good=0..127，1e5→milli-orders 除以 100），调用 `PublishRealEconomy`（synthetic=false）。
- `PublishRealEconomy` 设置 `kOwnerNativePanel` 选择（market=第一市场），让会话在正式 owner 下构建视图；`PollHideFlows` 每 500ms `Poll()` 并发布 document。
- `DrawUi` 在原生 owner 活跃时不再用 `PublishLogisticsDocument(nullptr)` 清除正式文档。

**实机**
- 采集 100 州后：`real economy snapshot accepted: orders=3082 goods=49 market=1`。
- 会话状态：`owner=10 gen=1 ready=1 frame=1 ribbons=116 err=` —— **真实快照成功生成 116 条模型分配 ribbon**。
- 正式页签文本本轮截图仍显示 waiting（文档发布/时序问题待查）；模型分配帧已存在。

**验证**
- 候选/已部署 `bd63bf36916931a098f9d6b2d3d1f590b22562272e5deb83e868b1b3a6d6a733`（28,892,668 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏（含误开的第二个实例），`probe_economy=0` 等全部恢复。

## 2026-10-23

### 10:51–11:25 R33：真实市场快照采集与落盘（economy_dump）

**实现**
- 读取器钩子内新增采集器：对 reader 观察到的每个州（当前市场）读取全部商品的生产/消费，去重后缓存；每 25 个州写一次 `%LOCALAPPDATA%\IronroadV3\economy_dump.txt`。
- 过滤无效数据：`good < 128` 且 `value < 1e12`（高位商品索引读到指针垃圾，如 1.4e14/1e18；有效量级远低于阈值）。
- dump 格式：`# states=N scale=100000` + `P/C <state> <good> <value>` + 总计行。

**实机**
- 100 个州采集完成，总量：
  - `totalProd=24,007,540,245`（≈240,075 单位）
  - `totalCons=36,843,839,954`（≈368,438 单位）
- 单州/单商品值与 UI 市场总量同量级；单位 1e5 再次确认。
- 这是**当前市场**（本地价格页签列出的州）的真实数据，可直接映射为 EconomySnapshot。

**验证**
- 候选/已部署 `474e638c41f971e3d5ab46f68320e77056c5c6e53c31c611cc1d388306d1f6c4`（28,703,482 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`probe_economy=0` 等全部恢复。

## 2026-10-23

### 10:21–10:50 R32：州枚举验证（registry vs reader 路径）

**发现**
- 州对象 id 位于 **state+0x8**（实机确认：205/790/201 均命中）。
- registry 暴力枚举（0..2047 + id 校验 + vtable 过滤）得到 1117 个“州”，但总生产/消费达到 4.5e18 级，明显无效；说明仅凭 id/vtable 无法证明对象是有效州容器。
- **reader 路径**（商品面板“本地价格”逐州调用）给出的州对象有效：
  - state 205：prod 8 项/296,415，cons 17 项/240,237
  - state 790：prod 8 项/976,950，cons 16 项/722,112
  - state 201：prod 11 项/8,264,509，cons 38 项/11,239,733
- 结论：真实快照应从 **reader 观察到的市场州集合** 构建（本地价格页签列出的即当前市场州），不要用 registry 盲枚举。

**实现**
- 读取器钩子增加每州全商品统计（前 3 个新州）与 registry 枚举对照实验（`probe_economy` 下）。

**验证**
- 候选/已部署 `d90dc2e590d856a7afc09b042d2922344bcc5fde3e0ff58e41ebf99022cacd7f`（28,654,634 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`probe_economy=0` 等全部恢复。

## 2026-10-23

### 09:46–10:20 R31：实机生产/消费数值读取与单位验证（1e5）

**实机数值**
- 直接读取 work：`state=432 good=31 prod=1,200,000`；`state=436 prod=400,000, cons=20,850,000`（钢）。
- 去重州后累计（前 100 个州）：`prod=3,424,477,556`、`cons=5,942,248,942`。
- 商品面板“不列颠市场”同商品显示 `销售订单 36.6K`、`购买订单 61.2K`。
- 比例：`3.424e9 / 36600 ≈ 9.36e4`、`5.942e9 / 61200 ≈ 9.71e4` → **CFixedPoint 缩放为 1e5**（5 位小数），与 UI 市场总量一致（差异来自 100 州子集）。

**实现**
- 读取器钩子内用 registry + 位图/数组直接读 `state+0xC8`（生产）与 `state+0x78`（消费）；按州去重累计，每 100 个州记录一次总和。
- 验证了静态公式与实机数值一致；真实快照的数据源已打通。

**验证**
- 候选/已部署 `eded5217d23162222fce2a2e53a2cbb2bc2eddde78904e0f0580eaad7bcdcb30`（28,643,288 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`probe_economy=0` 等全部恢复。

## 2026-10-23

### 09:06–09:45 R30：静态解码州经济容器布局（关键突破）

**静态分析**
- 用 Ghidra headless（`IronroadEconomy-r2` 项目，victoria3.exe）反编译 `0x0103F3C0`、`0x0103F440`、`0x0195FF00`、`0x0195BA00`，输出 `E:\Code\v3-analysis\container-r30.txt`。
- **indexed 路径**：`goodId = *(int*)(ctx+0x10 + 0x10)`；容器位图在 `container+0x20`，数组指针在 `container+8`；值 = `*(uint64_t*)(array + goodId*8)`（位图置位时）。
- **keyed 路径**：键来自 `*(int*)(*(arg+0xC0)+0x10)`；数组在 `container+0x30`、条目步长 0x20，按 `FUN_14173DA20(arg)` 选 qword。
- **读取器**：`state = registry(ctx+0x18)`；生产容器 = `state+0xC8`，消费容器 = `state+0x78`；值写入 `out[0]`。

**实现**
- `probe_economy` 下新增直接读取实验：读取器钩子内调用 registry `0x007D40C0`，读取 `state+0xC8/0x78` 的位图与数组，记录 goodId/value。
- 最终实机触发了一次 tooltip 状态，未捕获到 `eco direct` 样本（商品面板点击落点/提示框遮挡）；公式已由静态反编译确认。

**验证**
- 候选/已部署 `7e32789a92c75c0bcdf4885cb26f93f71d78ca722fa892b6c75f27124e3b6281`（28,540,190 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`probe_economy=0` 等全部恢复。

## 2026-10-23

### 08:36–09:05 R29：生产读取器/容器读取器钩子；定位 indexed 路径

**实现**
- 在 `probe_economy` 下增加三个观测钩子：
  - `0x0195BA00` 生产读取器（ctx,out）→ 记录 out 前后；
  - `0x0103F3C0` keyed 容器读取（container,out,arg）；
  - `0x0103F440` indexed 容器读取（container,out,arg）。
- 修正读取器 detour 返回类型为 `void*` 并原样返回原函数 rax（此前用 bool 可能截断返回值）。
- 全部仅记录、随后调用原函数，不修改参数/不构造值。

**实机**
- 商品面板“本地价格”实际走 **indexed** 路径：同一 `out`、8 个不同 `arg`（步长 0x70），`container=state+0xC8` 附近的容器。
- `out` 词为 0/end 指针/堆指针/代码指针，形如向容器追加的条目向量；具体数值条目/步长尚未解析。
- keyed 路径未触发；生产读取器的 out 在调用后 [0] 从指针变为 0，疑似值被移出/消费。

**验证**
- 候选/已部署 `450abd873cbdca0716a574ff5010c5b8e0f210161f1113faa0b690e9651b5b2f`（28,534,558 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`probe_economy=0`、`test_hide_variable=0`、`probe_variables=0`、`probe_write=0`、`native_variables=1`。

## 2026-10-23

### 08:06–08:35 R28：经济回调 ctx/args/result 结构 dump

**实机**
- 通过物流页签的货物行打开原版商品面板，再点 **本地价格**：`GetStateProduction` 对多个州逐次触发，state id 来自 `ctx+0x18`（本轮样本 205/790/201/351/484 等）。
- 捕获结构：
  - `ctx` 6 个 qword：`[0]` 代码/类型指针、`[1]=3`、`[2]` 指针、`[3]=0xffffffff_<stateId>`、`[4]` 指针、`[5]=0`。
  - `args` 向量：`data` 中 count=7、元素步长 0x20，begin 在 `data+0x20`；每次调用有不同对象指针。
  - `result`：4 个 qword 的值容器（vtable/代码指针/子对象指针），数值仍被包装，尚未解码。
- 这确认了州 id/参数向量/结果容器的实际形状；商品 id 与定点数编码仍待定位。

**验证**
- 候选/已部署 `098293db77f3e9cca9ad18e2bfde54f23ac111396ef1876e6ef007224f6d99b1`（28,521,158 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`probe_economy=0`、`test_hide_variable=0`、`probe_variables=0`、`probe_write=0`、`native_variables=1`。

## 2026-10-23

### 07:36–08:05 R27：GetStateProduction 脚本回调观测钩子（经济桥突破）

**实现**
- 新增调试设置 `probe_economy`（默认 0）：在 `base+0x019791F0`（GetStateProduction 注册回调）安装 MinHook detour，仅记录 `ctx/result/args`（含 `ctx+0x18` 州 id、`ctx+0x10/0x20` 对象、args 向量计数），随后调用原函数；不读取/不构造值。
- 钩子安装日志：`economy production probe installed at ...`。

**实机**
- 打开州面板的 **本地价格** 页签会按商品逐行调用 `GetStateProduction`；日志捕获多组：
  `economy production cb ctx=... result=... args=...`（旧格式构建 7236）。
- 这确认了脚本回调的实际调用时机与参数形状，为后续“捕获 ctx → 提取州 id/商品 → 调用容器读取器”提供入口。
- 因鼠标坐标缩放，后续点击未能稳定复现州面板；richer ctx 字段日志（state/p10/p20/args count）已构建但尚未抓到样本。

**验证**
- 候选/已部署 `9b8a8b31ff14c339141c53c68d667dab7332ba5ae6f448ce7ff49c3ab0ea6726`（28,518,598 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`probe_economy=0`、`test_hide_variable=0`、`probe_variables=0`、`probe_write=0`、`native_variables=1`。

## 2026-10-23

### 07:16–07:35 R26：`ironroad_hide_flows` 驱动调试货运视图隐藏

**实现**
- UI 线程每 500 ms 通过 `ProbeGuiVariables`（只读）轮询 `ironroad_hide_flows`，更新 `m_hideFlows`；状态变化记录 `native panel hide flows=0/1`。
- `ApplyPendingFreight` 在 `m_hideFlows` 为真时调用 `DeactivateRibbons` 并停止提交调试货运 ribbon；纯视图，不改数据/分配。

**实机**
- `test_hide_variable=1` 启动：日志出现 `TEST ironroad_hide_flows written`，随后 `native panel hide flows=1`，确认 UI 线程轮询与状态切换生效。
- 未打开调试货运视图做肉眼 ribbon 消失验证（需切换到 prototype 视图）；标志与分支已接通。

**验证**
- 候选/已部署 `c544f1e910fbc94117764c18fb5511682f9329a8e9cff134b98dae5cc9a38831`（28,513,281 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`test_hide_variable=0`、`probe_variables=0`、`probe_write=0`、`native_variables=1`。

## 2026-10-23

### 06:56–07:15 R25：程序化验证 `ironroad_hide_flows` 与按钮状态

**实现**
- 新增调试设置 `test_hide_variable`（默认 0）：当为 1 时，UI 线程在发布 7 个变量后写入一次 `ironroad_hide_flows='1               '`（堆字符串）。
- 用于绕过鼠标坐标问题，验证原版 GUI 的 `Exists/Toggle` 绑定与按钮 enabled 状态。

**实机对比**
- 基线（无变量）：`显示` 按钮灰暗（disabled），`隐藏` 按钮紫色（enabled）。
- 写入变量后：`显示` 紫色（enabled），`隐藏` 灰暗（disabled）。
- 两张裁剪图 `r25-baseline-row.png` / `r25-hideflows-row.png` 直观显示状态翻转；日志 `TEST ironroad_hide_flows written from UI thread`。

**验证**
- 候选/已部署 `43db0537e1df90a4b9ecadb7debde8fdd5e2477f6962f07636adf9f3f9d2b855`（28,505,886 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 测试后关闭游戏，`test_hide_variable=0`、`probe_variables=0`、`probe_write=0`、`native_variables=1`。

## 2026-10-23

### 06:36–06:55 R24：物流页签重开生命周期 + 商品列表打开原版商品面板

**实机**
- 物流页签点击“显示/隐藏”按钮的坐标仍受 GUI 缩放影响，自动化点击落到商品行；点击商品行**成功打开原版商品面板**（钢：国际/本地市场、用途页、消费税/关税、销售/购买订单，`r24-show.png`），证明货物列表与原版商品详情接通。
- 重开生命周期：Esc 关闭面板 → F4 打开 → Shift+5 切物流页签，内容、变量文本、按钮与货物列表均正常，无崩溃（`r24-reopen.png`）；日志 `UI-thread variables published` 正常。

**结论**
- 生命周期 A23/A24 的“关闭再打开”部分通过；按钮点击自动化仍受坐标影响，非功能缺陷。
- 本轮无源码改动，候选保持 R23。

## 2026-10-23

### 06:16–06:35 R23：物流页签布局微调（消除行重叠）

**改动**
- 重写 `ironroad_logistics.gui` 的顶部布局：状态、Freight、Target 标签、值行、按钮、Built rail 各自独立一行，显式 `size`（530x26/40）与 `multiline`；按钮行单独 flowcontainer。
- 保持 `default_header_2texts` 标题、显示/隐藏按钮逻辑与“货物”列表不变。

**实机**
- F4+Shift+5：页面干净，无重叠：
  `物流` / `waiting for economy data` / `Freight (orders): --` / `Target / allocated / unmet:` / `-- / -- / --` / 显示/隐藏 / `Built rail: --` / `货物` 列表。
- 证据：`r23-layout.png`。

**验证**
- 候选/已部署 `8f1a686cea6889eb9f84ffc03c5a3fbebe0935f4c3f3cc28e3910b9e070e780f`（28,498,718 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。

## 2026-10-23

### 05:56–06:15 R22：物流页签隐藏商品过滤器/搜索栏，内容布局清理

**改动**
- 在 `PatchNativeMarketGui` 中为商品专用的 `section_header_button`（FILTERS）、`search_bar` 与过滤器 `flowcontainer` 增加 `Not(IsTabSelected('ironroad_logistics'))` 条件；仅在物流页签隐藏这些控件，其他页签行为不变。
- 物流页签现在从内容顶部开始：`物流`、`waiting for economy data`、`Target / allocated / unmet: -- / -- / --`、显示/隐藏、`Freight: --`、`Track: --`、`货物`列表。

**验证**
- F4 + Shift+5 实机截图：过滤器/搜索栏消失，物流内容无商品头图遮挡（`r22-clean.png`）。
- 候选/已部署 `5a0ed5faffd7a09bb76f8d1424b41c5a10ccfe8a0db042af898fa836cf213252`（28,498,718 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 余留：Target/Allocated/Unmet 行与按钮行有轻微重叠，需布局微调；真实数据仍缺。

## 2026-10-23

### 05:26–05:55 R21：物流页签内容与 `.Get` 文本最终实机确认

**修复**
- 直接给类型实例加 `visible` 不生效（内容在默认页签也显示）。改为 **container 包裹**：
  ```
  container = {
      visible = "[InformationPanel.IsTabSelected('ironroad_logistics')]"
      using = default_fade_in_out
      ironroad_market_logistics_content = {}
  }
  ```
  容器可见性在默认页签验证为 false（内容隐藏），物流页签为 true。

**页签切换**
- 鼠标悬停页签提示快捷键 **Tab 5 / SHIFT+5**；发送 `Shift+5` 后“物流”页签高亮，内容显示：`waiting for economy data`、`Target / allocated / unmet: -- / -- / --`、`Freight: --`、`Track: --`、显示/隐藏按钮、“货物”列表。
- 至此正式 UI 的页签内容、`.Get` 文本与变量写入链全部实机打通。

**验证**
- 候选/已部署 `336208e29500851c3eb845ab7ddaabe81b90bd3e6e3c71c473160032dea22b80`（28,472,094 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 证据：`native/build/r21-shift5.png`（物流页签内容）、`r21-tabs-crop.png`（物流高亮）、`r21-container.png`（默认页签隐藏）、`tests-round21b.log`。

## 2026-10-23

### 05:06–05:25 R20：标题点击切页实验失败，回退干净构建

**实验**
- 临时在 `window_header_name` 的 `raw_text` 后插入 `onclick = SelectTab('ironroad_logistics')`，希望通过点击大标题切到物流页签。
- 实机点击标题（约 physical 420,165）后，选中的是 **成员** 页签（显示国家旗列表），不是物流；说明该 `onclick` 未作用到预期标题元素，或点击落点命中了成员页签区域。标题点击测试失败。
- 已回退该注入；正式构建保持 R19 的“块内内容 + 仅第五页签可见”。

**状态**
- 候选/已部署 `9d5b1dee32e10a820bd5d3c38000c1bbfd53eecae0312a52f3588734fd54fdbe`（28,472,606 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 证据：`native/build/r20-titleclick.png`、`r20-session.log`、`tests-round20.log`。

## 2026-10-23

### 04:46–05:05 R19：页签内容容器定位修复；`.Get` 文本实机可见

**发现**
- R18 之前的 `ironroad_market_logistics_content` 插在 `market_panel_details_content` **块外**（同级），即使 `IsTabSelected` 为真也不会进入页签内容容器。
- 改为插入 `market_panel_details_content` 的**大括号内部**（首个子元素）后，内容与页签选择正确关联。

**实机验证**
- 临时把可见性改为 `Or(default, ironroad_logistics)` 后 F4：页签内容区显示“物流”、`waiting for economy data`、`Freight: --`、`Track: --`、显示/隐藏按钮，与商品内容并存（同容器叠加，属测试态）。
- 生产构建恢复为仅 `IsTabSelected('ironroad_logistics')`；第五页签点击的自动化仍未命中（点击会落到地图/其他控件），但内容容器位置已正确。

**状态**
- 候选/已部署 `a8a83cc94fdb48a22a5f68659194d44e59bfb16c3c1ef0ca079b8dfabaa2015e`（28,472,606 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 证据：`native/build/r19-inside.png`、`r19-session.log`、`tests-round19.log`。

## 2026-10-23

### 04:16–04:45 R18：后载变量文本桥打通（UI 线程 + 堆字符串）

**发现**
- UI 线程发布消息已可用；之前 key 垃圾/value 空的根因不是线程，而是 **SSO 短字符串在地图后载的 insert/move 路径被破坏**（entry 里存了指针但 cap=15）。
- 改为 **长 key（>=16 字符）+ 值填充到 >=16 字符**，强制走堆字符串路径后，7 个变量全部正确写入并由探针读回：
  `ironroad_status_v1='waiting for economy data'`、`ironroad_freight_v1='--              '` 等。

**实现**
- `QueueLogisticsVariables` 采用长 key；值不足 16 字符用空格补齐；去重后经 `WM_APP+0x51` 在 UI 线程写入。
- 原版市场面板标题临时 `.Get` 验证：`STATUS=waiting for economy data FREIGHT=--`，确认端到端可读；随后移除标题调试。
- 正式 GUI 页签改用 `[GetVariableSystem.Get('ironroad_*_v1')]` 绑定；`native_variables` 默认开启（合成输入仍显示 `waiting for economy data`/`--`，不显示测试值）。

**未完成**
- 仍未能点击“物流”页签验证页签内容（点击坐标/布局问题）；默认页签强制显示测试也未渲染出内容。文本链本身已由探针+标题 `.Get` 验证。

**验证**
- 候选/已部署 `387c7924df718ad042a77eaeffa4fb1571ca95e112ef78e9feb85931bd7eaa71`（28,471,070 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 证据：`native/build/r18-getcheck.png`（标题 `.Get` 成功）、`r18-tabvisible.png`（页签未显示）、`r18-session.log`、`tests-round18.log`。

## 2026-10-23

### 03:56–04:15 R17：UI 线程写入窗口实现；后载 map 的 pair 布局仍不匹配

**实现**
- 复用已有的窗口子类 `Overlay::WndProcThunk`：新增 `WM_APP+0x51` 处理，在 **UI 线程**执行 `OnUiThreadPublish()`。
- 渲染线程只把 7 个 `ir_*` 字符串放入 mutex 保护的 pending 槽并 `PostMessage`；UI 线程取出后调用 `SetGuiVariable`。
- 5 秒稳定期后触发一次发布，验证消息路径；后载写入不再崩溃（进程存活，日志 `UI-thread variables published`）。

**新问题**
- 写入的 entry 哈希与我们的 7 个 key 完全匹配（`0dc66b10`、`2e7cc4e3`…），但探针读到的 **key 字节是垃圾、value 为空**；`GetVariableSystem.Get('ir_status')` 也返回空。
- 说明主菜单可用的 pair/所有权布局在**后载变量表**上不成立（engine insert 未按预期复制/移动我们的 MsvcString）。这不是线程问题，而是对象/布局问题。
- 已回退：移除标题调试；`native_variables=0`；`SetGuiVariable` 路径仍仅调试开关可达；GUI 恢复 pending。

**验证**
- 候选/已部署 `2bccef97b95aafaaa8d9cc37690e015da4545c601fab1c6ec99dd30939928d96`（28,466,083 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 证据：`native/build/r17-uivalues.png`、`r17-fixed.png`、`r17-session.log`、`tests-round17.log`。

## 2026-10-23

### 03:36–03:55 R16：单个后载写入不崩溃但失败；线程亲和性确认

**隔离实验**
- 保留标题 `.Get` 调试、`native_variables=0`：地图正常加载（`map=True`），说明 `.Get` 读取缺失 key 不会崩溃。
- 关闭“发布全部 key”，只保留一个后载单 key 写入（`ir_status`）：地图正常加载，进程存活，但写入失败：`post-load variable write failed: entry not found after insert`。
- 复现：在地图加载期批量发布全部 `ir_*` key 会失败并导致进程退出（R15 的崩溃）。
- 探针观察：某次会话加载后变量表有 36 条（`buckets=64 level=8`），当前会话启动为 0 条；说明变量表在加载/首次 UI 使用时会被重建或由 UI 线程占用。

**结论**
- 变量写入链本身可用（主菜单/安静时刻已验证），但**地图加载期和加载后的 Present 线程写入都不安全**：轻则 `entry not found`，重则进程退出。
- 需要真正的 **UI 线程写入窗口**（hook 市场面板打开/更新回调或 GUI tick），在这些回调内写；在此之前 `native_variables` 默认 0。
- 代码回退：发布全部 key 的调用已移除；后载单 key 探针只在 `native_variables=1` 时一次性尝试并记录失败；标题调试已移除。

**验证**
- 候选/已部署 `c87c5ae6119bb7b5497dd12582cfe2e646c50d7dfe1f40b843f549dcd948086e`（28,460,700 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 证据：`native/build/r16-single.png`、`r16-session.log`、`tests-round16.log`。

## 2026-10-23

### 03:16–03:35 R15：`GetVariableSystem.Get` 验证成功；渲染线程写入地图期不安全

**重大能力验证**
- 临时把调试文本插到原版市场面板标题区，F4 打开市场后读回：
  `VAR_EXISTS=1 VAR_GET=long-value…`
  → **`GetVariableSystem.Get('key')` 存在并返回写入的字符串**，变量可以用于正式 UI 文本显示（无需注册数据函数）。
- 该测试也再次确认变量写入链（Murmur3/assign/insert）在安静时刻可用。

**尝试正式文本桥**
- 新增 `PublishLogisticsVariables()`：把 `ir_status/ir_freight/ir_target/ir_allocated/ir_unmet/ir_track/ir_synthetic` 写入变量，GUI 用 `[GetVariableSystem.Get(...)]` 显示。
- 在地图加载期间由 Present 线程写入时失败：`entry not found after insert`（连续多个 key），随后游戏进程退出。判断为**渲染线程写入与 UI 线程/GUI 初始化竞争**，地图加载会重建/占用变量表。
- 按 D18 回退：`native_variables` 默认 **0**；GUI 恢复 pending 文本；移除临时标题调试插入。文本桥代码保留但默认禁用，等 UI 线程安全窗口。

**验证**
- 候选/已部署 `3ffd86e7b10a51a292972660f9a69f689dc164aa28f4eefd38788d2475eb244a`（28,457,628 B，clean）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 证据：`native/build/r15-header.png`（`.Get` 成功读回）、`r15-bridge2.png`、`r15-session.log`（写入失败）、`tests-round15.log`。

## 2026-10-23

### 02:56–03:15 R14：自动进入地图 + F4 打开市场；页签点击仍待校准

**重大自动化进展**
- 发现并验证启动参数 **`-continuelastsave`**：直接加载最近存档进入地图，绕过“载入游戏”对话框（该对话框的“载入”按钮对合成点击长期无响应）。命令：`victoria3.exe -gdpr-compliant -continuelastsave`。
- F4 绑定确认可用：`keybd_event(VK_F4=0x73, scancode 0x3E)` 能可靠打开原版市场面板；`PostMessage` 不行。
- 读取 `pdx_settings.json`：`GUI.scale = 1.1`，点击坐标与截图位置存在缩放偏移，是页签点击失手的可能原因。

**未完成**
- 仍未点中市场第五页签“物流”：直接点 (585,232) 会落到地图（打开海区面板并关闭市场）；点 (532,211) 会展开“每月价格报告/过滤器”。临时 debug 文本（Exists/Get）均已移除，正式 GUI 保持干净。
- `GetVariableSystem.Get` 是否存在仍未验证（`.Get` 调试行曾在 GUI 中，但未成功进入页签内容；已移除）。
- 变量写测试在 `probe_variables=1 && probe_write=1` 下正常执行（本会话读回 `long-value-2`）。

**验证**
- 候选/已部署 `e728cd5e6d13e5acf8d3c460e986df3bf79d120a5450efd283b7947ebbfd0e6b`（28,448,007 B，clean GUI）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 证据：`native/build/r14-session.log`、`r14-f4c.png`（F4 打开市场）、`r14-clean585.png`/`r14-clean532.png`（点击落点）、`tests-round14.log`。

## 2026-10-23

### 02:36–02:55 R13：原版市场面板自动化与“物流”页签实机确认

**发现**
- `game/input_profile/default.profile`：`open_market` = **F4**（scancode 61）；市场面板也可从左侧 sidebar 的贸易按钮打开（`trade_overview_btn.dds`）。
- 自动化从 sidebar 打开市场面板成功；截图确认第五页签 **“物流”** 与 `商品/贸易/粮食安全/成员` 并列存在，页签物理坐标约 (590, 230)，页签内容区含“显示/隐藏”按钮。

**未完成**
- 为测试变量文本绑定，曾在 GUI 加两行临时 debug（`GetVariableSystem.Exists` 守卫的文本、`GetVariableSystem.Get(...)` 原文）；因面板关闭/点击坐标问题未真正切到“物流”页签内容，未验证 `.Get` 是否存在。最终构建前已移除 debug 行，正式 GUI 保持干净。
- 发现配置错误：`probe_write=1` 但 `probe_variables=0` 时写测试不会执行（`ProbeVariablesIfEnabled` 提前返回）。下轮需两者同时开启，或在 UI 点 “Probe now”。
- F4 的 `PostMessage(WM_KEYDOWN)` 未触发面板；下轮改用 `SendInput`/`keybd_event` 并确保前台。

**验证**
- 候选/已部署 `a59fdbfc26e23837b5db8acd5537fa41a4b5c216ea0e5aebbf64a15cee035977`（28,448,007 B，debug 行已移除）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 证据：`native/build/r13-market5.png`（市场面板+物流页签）、`r13-session.log`、`r13-tabs3.png`、`tests-round13.log`。

## 2026-10-23

### 02:21–02:35 R12：全局字符串映射探针、Murmur3 反推、变量写入打通

**只读扩展**
- 发现同一 string→string map 模板的第二个实例（静态地址，运行时 5639 条），用同一探针读取：`buckets=8192 count=5639 level=15 load=0.90 entries=5639`，键/值读取全部正常，验证了 0x48 条目布局。
- 用 40 条真实 key/hash 对反推哈希：**MurmurHash3 x86_32, seed=0**，40/40 完全匹配（FNV/CRC/djb2 等均不匹配）。
- 静态确认 map 写入调用点：`0x3BB3C30(data, len, seed)` 即 Murmur3，`0x79C0D0(map,out,hash,pair)` 为 insert-or-find；`0x61F280` 为引擎 `std::string::assign(dst,src,size)`（含分配/释放）。

**写入实验（gated）**
- `WriteGuiVariableProbe()`：签名校验 getter/insert/assign → Murmur3(key) → 已存在则 `assign` 更新值，否则用 `assign` 构造键/值对并 insert（引擎接管字符串）。
- 实机 1（短键）：`ir_probe` = hello-1 → hello-2，成功；map 变为 `buckets=8 count=1 entries=1`，探针读回 `value='hello-2'`。
- 实机 2（长键，堆字符串）：`ironroad_probe_long_key` = long-value-1 → long-value-2，成功；读回 `hash=dbf7bbbc value='long-value-2'`，无崩溃。
- 关闭游戏，`probe_variables=0`、`probe_write=0`。

**关键限制**
- 原版 GUI 对变量系统只用 `Exists`/`Toggle`/`Set('key','true|false')`，**没有任何 `Get` 文本读取**；变量只能当布尔/枚举标志用，不能直接显示货运数字。数值/文本摘要仍需注册自己的 GUI 数据函数。
- 写入路径当前仅调试开关可用；正式 UI 使用前需证明线程/生命周期与回滚策略。

**验证**
- 候选/已部署 `05f54995d632fa1b458acf3b4f6a05c1bfbfe6c806b13954e784f234f3e9c43e`（28,448,007 B）；`verify_embedded` PASS；`build_tests` 11 项 PASS。
- 证据：`native/build/r12-probe-read.log`、`r12-write.log`、`tests-round12.log`。

## 2026-10-23

### 02:10–02:20 R11：GUI 变量系统 getter-only 探针 + 实机验证

**改动**
- 新增 `native/src/variable_probe.{h,cpp}`：校验 `kRvaVariableSystem`（0x3A92920）的入口签名后调用 getter；用 `VirtualQuery` 校验对象/数组/字符串指针；读取 map 头部（+0x08 entries、+0x10 count、+0x14 mask、+0x18 level、+0x1C load）与 0x48 条目（hash/占用/MsvcString 键/值）；**从不写入**。
- `offsets.h`：加入 `kRvaVariableSystem` 与 8 字节签名；该签名并入 profile gate。
- `overlay`：新增 `probe_variables` 设置、Diagnostics “GUI variable probe (read-only)” 复选框 + “Probe now”、每 2 秒轮询且仅在新 key/数量变化时记录；`m_varProbeSummary` 显示对象/桶数/条目数。
- 构建/部署候选 `b171d15d9cabb8a532e71f471b069bf060b642792cb679aebc6e1304346120a7`（28,382,723 B）。

**实机结果**
- profile ready（新签名通过）、ImGui ready 2560×1440。
- 日志：`GUI variable map obj=00007ff71c0dd800 buckets=1 count=0 level=0 load=0.90 entries=0`——对象地址 = base+0x59FD800，头部字段与静态解剖完全一致；无崩溃。
- 未取得条目：为验证 0x48 条目布局，需要先让原生 GUI 创建 `ironroad_hide_flows`（打开市场“物流”页签并点击显示/隐藏）。本轮自动化已加载 autosave，但未能可靠打开市场面板/物流页签，因此 `count` 仍为 0。
- 测试后关闭游戏、`probe_variables` 恢复 0。

**验证**
- `native/build.ps1` 成功；`verify_embedded.py` PASS。
- `native/build_tests.ps1`：11 项 PASS（`tests-round11.log`）。

**风险/边界**
- 探针在渲染线程调用 getter，且遍历时 GUI 线程可能修改 map；目前只读、短窗口、可关闭，仍属调试能力，不进入正式功能。
- 尚未验证条目内容、hash 算法、setter 与线程亲和性，不能写变量。

## 2026-10-23

### 02:36–02:50 R10：GUI 变量系统静态解剖（只读）

**目标**：为原版 UI 数据桥找到“从 C++ 写入/读取 GUI 变量”的可验证路径。本轮只做静态分析，不调用任何引擎函数。

**发现**
- `GetVariableSystem` 是 Jomini 函数：注册包装（RVA 0x5AE6C0）→ 通用注册助手 0x3AB6D00(name, 0, callback) → 回调 0x3A98400（忽略参数）→ 返回 0x3A92920()。
- 单例 getter 0x3A92920 惰性构造对象（RVA 0x59FD800），构造器 0x7B79D0；对象 **非多态**，0x28 字节头部 + 分配器。
- 对象本质是 **string→string 哈希表**：条目 0x48 字节（hash、占用/探测、MsvcString 键、MsvcString 值）；已定位重哈希 0x79B4E0/0x79B480、insert-or-find 0x79C0D0（移动键/值字符串，已存在时不更新值）、字符串比较 helper。
- 未定位：条目 hash 算法、更新已有值的 setter、GUI `.Exists/.Toggle/.Get/.Set` 与该对象的映射、线程亲和性。

**结论**
- 仍不能在 DLL 中写变量或注册数据函数；下一步只做 getter-only 运行时探测（门控、可关闭），读取现有 `ironroad_hide_flows` 条目验证布局与 hash。
- 证据：`E:\Code\v3-analysis\var-system-r10-notes.txt`；`var-system-r10*.txt` 反编译输出。

**验证**
- 本轮无源码改动；`native/build_tests.ps1` 11 项仍 PASS（R9 结果）。
- 未部署/未实机（仅静态）。

## 2026-10-23

### 02:20–02:35 R9：修复设置文件路径并实机验证迁移

**问题**
- `RuntimeDataDirectory()` 实际是 `%LOCALAPPDATA%\IronroadV3\data`；此前 README/文档说设置位于 `%LOCALAPPDATA%\IronroadV3\ironroad.ini`，但 DLL 只读 `...\data\ironroad.ini`。用户按文档放在根目录的设置会被忽略（R8 实测）。
**修正**
- `native/src/overlay.cpp`：新增 `m_settingsPath`。有 runtime data 时使用其父目录（即 `%LOCALAPPDATA%\IronroadV3\ironroad.ini`）为规范路径；`LoadSettings` 先读规范路径，再回退旧 `...\data\ironroad.ini`；`SaveSettings` 写规范路径并创建父目录。
- `native/src/embedded_assets.cpp`：首次运行时把旧的 `data\ironroad.ini`（或更旧 `binaries\ironroad\ironroad.ini`）迁移到根目录规范路径，不覆盖已存在的根文件。
- `native/src/overlay.h`：新增 `m_settingsPath`。

**实机验证（当前候选）**
- 构建并部署 `bef613d4907bddc570517a18219327a7302560c2c4772978d3ffddf3fe6b34bf`（28,123,676 B）。
- 删除我们先前误放的根 ini、把 `data\ironroad.ini` 设为 `test_snapshot=1`，启动游戏：
  - 根 ini 由迁移自动生成；
  - 日志 `settings: type='ironroadBuildingOutput' … mode=0` 与 `synthetic freight test snapshot restored (48 orders)`；
  - `native GUI enumeration; prepared=1 mounted=1`、`ImGui ready 2560x1440`；无崩溃。
- 关闭游戏，`test_snapshot` 恢复 0；规范根 ini 保留（含迁移后的设置）。

**附带静态研究（未调用）**
- 只读 Ghidra 确认 `GetVariableSystem` 注册包装调用 `FUN_143ab6d00(name, 0, FUN_143a98400)`；回调 `FUN_143a98400` 返回 `FUN_143a92920()`，后者返回全局 VariableSystem 对象地址（RVA `0x59FD800`）。尚未验证方法 ABI/线程/值类型，未在 DLL 中调用；仅作为后续原生 UI 数据桥的研究线索。

**验证**
- `native/build.ps1` 成功；`verify_embedded.py` PASS。
- `native/build_tests.ps1`：11 项 PASS（exit 0）。日志 `native/build/tests-round9.log`。
- 证据：`native/build/r9-session.log`、`native/build/dll-build-round9.log`。

**风险**
- 迁移只发生一次且不覆盖用户根文件；若两个文件都存在且内容不同，规范根文件优先。
- `%LOCALAPPDATA%\IronroadV3\data\ironroad.ini` 仍保留但不再被持续使用；文档已更新。

## 2026-10-23

### 02:03–02:20 R8：首次实机验证当前候选（调试货运视图）

**过程**
1. 在游戏关闭时运行 `deploy.ps1`，部署当前候选 `c6f77995…9d7b70`（候选=已部署）。
2. 发现设置实际路径：`RuntimeDataDirectory()` 返回 `%LOCALAPPDATA%\IronroadV3\data`，因此 `ironroad.ini` 在 `...\IronroadV3\data\ironroad.ini`（此前写到了根目录，未被读取）。修正后设置 `test_snapshot=1`。
3. 直接启动 `victoria3.exe -gdpr-compliant`（Steam 运行中）。启动日志：
   - loader attached、profile `1.13.11` hash 匹配；
   - GUI 归档 prepared/mounted、嵌入资源 MountData、native GUI enumeration prepared=1 mounted=1；
   - native hooks installed、ImGui ready 2560×1440；
   - `synthetic freight test snapshot restored (48 orders)`；无崩溃。
4. 通过 `Insert` 打开调试面板；一次误点击命中主菜单 Continue，加载了 autosave（未修改/未保存游戏），随后进入地图（1836 年，暂停）。
5. 切换到调试 UI prototype 页后：
   - `freight view gen=2 world=1 rev=1 ribbons=58 compute=0.004 ms synthetic=1`；
   - 解析 `ironroadTrafficLimeW6`、`ironroadTrafficGreenW0/W2/W3/W4` 等多宽度类型；
   - 持续 `SUBMIT scope=0/1/0`；结束前累计约 153k 次提交、71 次上传、605 cache hits；
   - 截图可见地图上的多宽度彩色 freight ribbon（cyan/lime/red 等），以及文档的 TEST INPUT、Freight 16.15M、Target 111.63k / Allocated 38.95k / Unmet 72.68k、热度参考 25000。
6. 原版市场“物流”页签可见，但仍显示“实时货运数据尚未接通 / 货运量 -- / 已铺铁路 --”（预期，正式数据桥未接）。
7. 用 WM_CLOSE 正常退出游戏；把 `test_snapshot` 恢复为 0；`readback.ppm` 与日志已保存。

**证据**
- `native/build/r8-session.log`（运行日志副本）
- `native/build/r8-freight1.png`（2560×1440：调试面板 + 地图 ribbon）
- `native/build/r8-crop.png`（Britain freight ribbon 放大）
- `native/build/r8-panel.png`（主菜单+面板）
- `native/build/r8-diag.png`（另一帧/纸地图）

**结论与边界**
- 实机证明：当前 DLL 可加载、GUI/样式/hook/ImGui 正常；合成 TEST INPUT 的文档和 ribbon 提交链路在真实渲染进程中工作并可见。
- **不是**实时数据、正式原版 UI 或完整验收：A13–A15、A18–A19、A25 未通过；正式 UI 数据桥仍未实现。
- 误加载 autosave 不应视为用户存档被修改：游戏未保存，测试后正常退出。

## 2026-10-23

### 01:50–02:02 R7：合成输入覆盖全部宽度档 + 配对供需

**改动**
- `native/src/dev_snapshot.cpp`：把每个商品的州分成成对的生产者/消费者，双方使用同一数量档，使每个商品总供给=总需求（奇数余量仅作为额外供给）；使用八档数量（4k/25k/80k/300k/800k/3M/8M/25M milli-orders）覆盖固定宽度阈值 10/50/200/500/2000/5000/20000 orders。
- `native/test/dev_snapshot_tests.cpp`：新增“合成 ribbon 覆盖多个原生宽度档、至少达到宽档”的断言；实测 48-region 产生宽度档 **0,2,3,4,5,6,7**（7 档）。
- `native/test/freight_scale_tests.cpp` 的“全铁路可分配 > 90%”在配对供需下重新成立（此前单侧数量不等导致假失败）。

**验证**
- `native/build_tests.ps1`：11 项 PASS，exit 0。日志 `native/build/tests-round7.log`。
- 合成快照（48 region）：58 ribbons、freight 16,153,537、compute 4.18 ms；宽度档 0/2/3/4/5/6/7。
- 规模（配对合成）：无铁路 48 region 15 shipments/58 ribbons/3.8 ms；649 region 248/655/25.8 ms。全铁路：48 region 41/156/15.9 ms/99.5% allocated；649 region 627/1801/271.8 ms/96.5%。
- `native/build.ps1`：成功；DLL `c6f7799570a279110b08d54d62b5ce3bbea845b32e05c5737e29f8f6b19d7b70`（27,962,872 B）。
- `python -B tools/verify_embedded.py`：PASS。
- 未部署、未实机。

**风险**
- 数量档与配对是调试输入设计，不代表真实市场；649-region 冷构建 271.8 ms 是合成上界。
- 宽度档只覆盖 7/8（缺 W1）；需要时调整数量档即可，不改变线宽映射。
- 仍未验证 NPC/GPU 上传与实机可见性。

## 2026-10-23

### 01:38–01:50 R6：区域 hub 回退修正 + 规模/连通性测量

**发现（真实派生地图）**
- 原分配器固定用 `region*100`（城市 hub）连接州。排除未建铁路后，634 个城市 hub 中有 **238 个没有任何可用边**；许多州因此被人为判为不可达，48-region 合成快照仅 8/96 allocated。
- 每个州 region 拥有 `region*100 + 0..4` 多个 authored hub；`+3` 是港口 hub（带 type 4 海岸转换）；部分州的城市 hub 只有铁路，而港口/道路 hub 可达。

**修正**
- `native/src/freight.cpp`：在 policy 允许的边（排除未建铁路、关闭边）上统计各 hub 活跃度，取最高活跃度 hub（同度取较低 node id）；没有任何可用 hub 时保留未分配。
- 回归：新增“城市 hub 仅未建铁路、港口 hub 有道路”的用例，确认走道路并满足 target；关闭该道路后需求保留为未满足。

**规模/连通性测量（离线，真实地图，合成快照）**

| 快照 | 未建铁路 | 全铁路视为已建 |
|---|---|---|
| 48 regions | 8 shipments / 39 ribbons / 3.3 ms / 32·96 | 22 / 117 / 10.9 ms / 88·96 |
| 128 | 28 / 137 / 4.4 ms / 112·256 | 61 / 295 / 22.6 ms / 244·256 |
| 256 | 47 / 221 / 6.1 ms / 188·512 | 125 / 609 / 43.0 ms / 500·512 |
| 512 | 111 / 513 / 13.3 ms / 444·1024 | 253 / 1206 / 82.6 ms / 1012·1024 |
| 649 | 153 / 641 / 17.6 ms / 612·1296 | 320 / 1472 / 105.0 ms / 1280·1296 |

- 无铁路时活跃图有 1229 个连通分量（giant 1328/3384 nodes），781 个州中仅 393 个至少一个 hub 在 giant；未满足主要来自“未建铁路 + 网络碎片”。
- 全铁路已建时分配接近 99%；热视图 0 ms、hover ≤0.7 ms；1472 条 ribbon 按 16/帧约需 92 帧上传（未实机测）。

**验证**
- `native/build_tests.ps1`：11 项 PASS（exit 0）。日志 `native/build/tests-round6b.log`。
- `native/build.ps1`：成功；DLL `5bca95e09e2526676dffbff74e528e2e7b4ac1f92e6bd6777fbdbd4643aa5ddc`（27,935,041 B）。
- `python -B tools/verify_embedded.py`：PASS。
- 未部署、未实机。

**失败/事故记录**
- 用 PowerShell `Get-Content | Set-Content` 批量替换中文文档导致 `AGENTS.md`、`STATUS.md`、`WORKLOG.md` 编码损坏（UTF-8 无效）；已用 UTF-8 写入重建。以后禁止用该方式改写中文 Markdown。

**风险**
- hub 选择是模型启发式（最高可用度），可能改变起运/到达的区内节点；需真实已建铁路快照复核。
- 合成快照没有真实已建铁路，高未满足比例是预期结果，不代表真实市场。

### 01:36–01:42 R5：需求/分配/未满足统计 + 热度图例

- `FreightView` 新增 `targetImport` / `allocatedImport` / `unmetDemand`（orders）；作用域为整个市场或该州，不随商品过滤变化；unmet=max(target−allocated,0)。
- `LogisticsDocument` 透传；面板显示三项统计与“未满足不画成已运输”；热度图例显示参考尺度/版本，无货运时明确无尺度。
- 测试：缺货 target 5/allocated 3/unmet 2；完整市场 6/6/0；过滤前后不变；州视图按州汇总。
- 验证：10 项 PASS；DLL `d873648e…ace7bc9e`（27,933,520 B）；未部署、未实机。

### 01:35 R4：货运会话多 owner 仲裁

- 会话支持原生面板 10 / hover 20 / 调试 30；有效 owner 最高优先级（同级最近更新）；关闭 hover 恢复基础视图；关闭一个 owner 不清除其他；全部关闭才清空可见 frame，快照保留。
- 测试：多 owner 回归；10 项 PASS；DLL `2d5812b1…4dd916e1`；未部署、未实机。

### 01:28 R3：合成测试快照 + 货运 ribbon 提交路径

- `dev_snapshot` 生成确定性合成快照（3 商品、有 authored hub 的 region、市场 1）；文档 `synthetic`、面板 TEST INPUT；不进入原版 UI。
- overlay `ApplyPendingFreight()` 将最新 `FreightFrame` 分帧上传并提交到共享原生池；切页/关闭停止提交；Diagnostics 增加显式开关（`test_snapshot` ini）。
- 真实地图：48 orders→24 ribbons（R6 后 39），3 ms 级；10 项 PASS；DLL `403f6b12…e2008cdd`；未部署、未实机。

### 01:22 R2：UI→货运文档会话 + overlay 接线 + 经济静态复核

- 新增 `LogisticsSession`（快照门控、worker 消费、文档发布、关闭取消、重开复用）；`FreightWorker::Reset()`；文档构建使用缓存视图。
- overlay 调试页接线；新增 `logistics_session_tests`；9 项 PASS；DLL `971cf5c6…5e7e`。
- 静态确认 `GetStateProduction/Consumption/Imports/Exports` 为脚本函数回调，不是简单 getter；不得据此 hook（私有笔记 `economy-r2-notes.txt`）。

### 01:13 R1：固定市场热度尺度 + 双层缓存

- 热度改为按完整市场快照的固定参考尺度；hover/search 不改变颜色。
- 新增 `FreightEngine`（allocation + view/ribbon 缓存、内容指纹、world/revision 拒绝、ResetWorld）与 `FreightWorker`（LatestWorker）；新增 `freight_engine_tests`，`freight_tests` 增补热度回归。
- 8 项 PASS；DLL `45fce3c8…c51c402`；未部署、未实机。

### 01:07 R0：接管基线

- 完整重读 MASTER_PLAN 与四份原始文档；核对 README/审计/模型/GUI/测试文档、源码、`native/PROFILE.md`、构建/部署脚本、本机环境。
- 基线 7 项测试 PASS；游戏 1.13.11、EXE `7f53…f811f`、候选/已部署 DLL `a99f…c3e74`、市场 GUI `1f68…ec3e`；无 Git。


### 2026-10-24 — R96：真实可用铁路接入，最终构建已部署并实机核对

- 当前源码构建=dist=安装：主体1666187dda0f9e1d0944c0af9f91024c36c1200ae3f42692fe625bfcea8d3fb9（34,865,300 B），引导71dae36d…。完整原生入口、嵌入资源、原版GUI结构/实际归档、GPU、D3D11自检和默认隐藏均PASS；证据native/build/*round96.log。首轮新增测试缺windows.h的编译失败已修复并完整重跑，不隐去失败日志。
- 生产只读采集取得2329潜在铁路连接、34原版当前可用铁路连接/34物理边；64位原生连接key与物理strip ID分开，格式3追加4271个一对多映射，原几何字节保持。当前type4表示原版当前可用铁路，不能扩大为全部已铺设线路。所有worker输入均为自有副本，失败撤回。
- 最终1666自主暂停会话35744中，市场0正式卡片20,267,075.76、195条ribbon可见；外部复制后关闭进程句柄的模型重放逐值一致，557条shipment使用18条已确认铁路边。仅在自有副本移除铁路的对照为17,857,680.87，未写游戏内存。稳定窗口379.219秒累计uploads保持195；这不替代P95/模拟速度验收。
- 同一会话直接重载日志确认world2重采集及195槽恢复；最终截图前进程已不存在，无新crash目录，不推断退出原因，不将日志扩大为该构建重载后的可见验收。测试未推进日期/保存。当前无游戏进程。
- 市场铁路卡片仍--：缺分市场汇总与距离标定；未暂停复制原子性、实际建拆、改图资源内容身份、新设备/异存档及完整性能仍未完成。证据/回滚：checkpoints/2026-10-24-r96-built-rail，1666-market-rails-visible.png、1666-model-replay.txt、1666-stable-uploads.json、1666-final-freight.log。用户随后报告刷新不稳，转R97优先修复。



### 2026-10-24 — R97：修复周期刷新和悬停清空统计；已部署并实机通过

- 用户报告货运刷新不稳定。先复现相同输入仍清帧与悬停使仍有效统计失效，两份失败日志refresh-round97-{before,hover-before}.log保留；修复后相关会话用例及完整统一入口PASS。
- 采集先确认当前roster仍属session，再逐值比较同world订单、商品全部属性、铁路集合和主市场。内容未变保留已发布快照/revision/frame，不取消worker、不清空GUI；不以hash代替相等。元数据失败清roster后不能走去重捷径，synthetic/retired/stale不匹配。
- 悬停不再发布null。UI统计专用校验要求同一allocation对象、同world/revision、相同基础筛选，仅忽略hoverGood；地图仍使用严格FrameIsCurrent。搜索、市场/州切换、关闭、换世界及同版本数据替换继续撤下旧数据。新增字段变化、悬停未完成、关闭/退休、synthetic及同版本替换反例。
- 当前源码构建=dist=安装：主体eaf2a3190398893fce1802f8d1083561849359f9aabc0b766b4d4d9d4da36d17（34,911,634 B），引导71dae36d…不变。完整build/tests、GUI结构/实际归档、嵌入资源、GPU、自检560437亮像素、默认隐藏vtx0均PASS，deploy.ps1部署；证据native/build/*round97.log。 实机自有ironroad解析命中0；宽泛关键字仍命中6条原版city/fleet资源消息，逐条与R96相同，未改动这些资源，分类见gui-error-classification.json。
- 自主暂停28748实机：市场155.109秒/五次相同采集无null发布、无GUI重写；20轮移入/移出共40次实际hover转换，卡片始终20,267,075.76，地图195↔146/127条路线，统计无清空。地区中英格兰75秒/两次采集保持owner15/gen46/frame1/115路线，uploads530不增，列值可见。空搜索正确0.00、清空查询恢复，关闭撤回旧scope。
- 直接Load同一autosave，world1→2，新快照rev19实际接受，池530槽重新校验后恢复195条路线；world2-recovered.png确认卡片/箭头可见且停用提示消失。去重未拦新world。仅覆盖同设备同存档；不扩大为所有设备/异存档/未暂停更新通过。
- 28748已正常WM_CLOSE退出；全程暂停1836-01-01、未保存，autosave仍9,176,649 B、修改时间2026-10-23 23:07:24。当前无游戏进程。备份/原1666安装DLL/全部证据：checkpoints/2026-10-24-r97-refresh-stability；refresh-report.json、hover-actions.json、state-stable-report.json、eaf2-session-only.log与截图可定位。
- 本单元解决已复现的无变化刷新及hover统计闪烁；真正数据变更仍按安全协议撤回并重算，未暂停采集原子性、实际铁路建拆/市场铁路统计、改图内容身份、新设备/异存档及完整性能仍未完成，不将本轮暂停测试替代这些验收。



### 2026-10-24 — R98：完整州订单安全复制已部署；未暂停更新取得实机证据

- 当前主体59e6d3d486a168f77d0dd7a7bcb632b5700755cce35ea43e64f5cb94a86b98c4（34,717,331 B），引导71dae36d…；源码构建=dist=安装。完整统一原生入口、GUI结构/实际归档、资源、自检560422亮像素、默认隐藏vtx0均PASS；native/build/{build,tests,gui,selftest,debug,deploy}-round98*.log。新增native_economy_tests接统一入口。
- 按本主题重读01/02/03全文。原0..2047枚举、按样本挑vtable及直接读取订单已替换：按当前DB完整slot表取全32位handle，验证CState精确类型、slot/代数、唯一对象和既有地区key/真实市场，批量安全复制订单。counter+8数据/+10容量/+14长度/+20位图与目录长度分配经当前EXE独立汇编/反编译核对，/100转milli-orders保持；容量/位图/负数/不可读拒绝，不伪装零。
- 回归覆盖slot2049/full handle0xAB000801、州/市场0、错type/slot/重复对象、容量/目录/位图边界、负值、不可读、跨州复制期间变化、退休及多解码器联合候选晚期变化。恢复旧2048上限的负对照实际FAIL，正常实现PASS。旧自动field/vtable探针的完整源码已随修改前checkpoint保留；可选历史诊断保留，主路径不依赖猜测或原生lookup调用。
- EconomyReadWindow在商品目录、订单、铁路均复制后复核联合观察。暂态铁路读取失败撤回scope并重试，不能仅凭复制失败制造新world。此机制拒绝已观察到的变化，**不构成原子模拟epoch/完整访问窗口证明**；可靠世界事件/地址复用及P4-T01仍未完成。
- 外部只读生产解码器在原R97暂停46736通过1147州/25396订单/43商品/34铁路，复制94ms；进程句柄关闭后模型重放20,267,075.76与R97一致。46736后续已不存在，正常关闭尝试未找到窗口、无新crash目录，不推断退出原因。最终R98自主47012暂停采集在DLL内订单31–47ms、整组93–110ms，正式卡片与195路线可见；这是采集耗时，不是P95性能验收。
- 最终R98以最低速度限时运行，截图从1836-01-01到01-07。窗口内订单25396→25391，新候选world1/rev7接受；工作量20,267,075.76→19,815,791.98，目标/已分配/未满足30945.01/14716.40/16228.60，195路线可见。该次采集整组93ms、worker138.546ms；窗口内无WARN、无错误world失效。仅一次实际变化，不能扩大为所有未暂停/建拆/原子性或性能通过。
- 47012由限时脚本正常WM_CLOSE退出，未保存；原autosave SHA256 fecc16f78294e294eda3211fdec9e9b89c2fd325685cd20992fd0a2a8398e804、9,176,649 B及mtime均未变，无需恢复。当前无游戏进程。备份/原eaf2安装DLL/原存档与静态/测试/实机证据：checkpoints/2026-10-24-r98-economy-copy；state-counter-static.txt、economy-production-decoder-live.txt、unpaused-{actions,report}.json、unpaused-09.png、59e6-session-only.log、tested-build.json。
- 剩余：验证真正原子访问窗口与可靠世界事件；实际铁路建拆/分市场里程与距离标定、改图实际资源/内容身份、新设备/不同存档、三轮性能及最终A01–A27。稳定复核不能代替上述门槛，目标继续。

