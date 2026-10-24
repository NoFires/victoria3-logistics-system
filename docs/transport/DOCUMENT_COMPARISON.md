# AGENTS／MASTER_PLAN 逐条精简对照（2026-10-23）

> 后续修订（R83）：用户已明确允许version.dll引导＋freight/freight.dll主体，并将运行目录定为freight；旧单DLL数量限制／LocalAppData默认已被MASTER_PLAN §3.1–3.1.1替代。下表保留精简当时的139项审查记录。

依据四份原始文档全文逐项核对。下表的 MASTER 指根目录 MASTER_PLAN.md；Axx 指其§13验收ID。相同要求合并到一个规范入口；原文保留为技术细节来源。后续用户要求单独列明，不能因原文没有而删除。

本表是本次审查记录，不是新的任务书或当前状态源。未修改四份原文、源码、DLL或任何实机验收结果。

## 原始要求逐条去向

| 序号 | 原始位置 | 要求 | 精简后位置 | 处理 |
|---:|---|---|---|---|
| 1 | 00 · What is included / Quick start | 四份原文须完整阅读；先检查现有工程 | AGENTS §1；MASTER §2.1 | 合并入口，保留 |
| 2 | 00 · What is included / Quick start | Python为离线分析／示例，不能替代进程内native桥 | MASTER §3.1、§8 | 保留开发／运行时边界 |
| 3 | 00 · What is included / Quick start | 使用本机EXE、只读发现；候选不等于安全调用 | MASTER §9.1–9.2 | 保留 |
| 4 | 00 · What is included / Quick start | 私有发现报告放在分享包外 | MASTER §9.1、§16 | 合并 |
| 5 | 00 · What is included / Quick start | Python 3.12、venv、依赖安装和发现命令 | 00原文；AGENTS §3为当前构建入口 | 不复制原文教程 |
| 6 | 00 · What is included / Quick start | 原ZIP不含地址／DLL／C++／shader／游戏资产 | MASTER §2.2、§19 | 限定为starter历史说明，不套用当前工程 |
| 7 | 00 · Validation of this handoff | 十二组候选／五项helper历史成功只适用于当时EXE | MASTER §2.2、§13.2 | 保留证据边界，不复制旧成绩 |
| 8 | 00 · Validation of this handoff | 旧项目实机成功／日志优化不保证本机性能 | MASTER §10.5、§13.2 | 合并 |
| 9 | 00 · Validation of this handoff | 展示无须修改价格／MAPI／建筑／人口需求／分配 | MASTER §4.2；AGENTS §2 | 保留 |
| 10 | 01 · Intended result / Inspect and record | 起终点选择、绘制与Clear、批量货流筛选 | MASTER §3.2–3.3、§12 P2/P5 | 操作迁至原版正式UI；调试预览保留 |
| 11 | 01 · Intended result / Inspect and record | 原生世界空间CArrow，实际路线、河流与相机对齐 | MASTER §7 | 保留 |
| 12 | 01 · Intended result / Inspect and record | UI只复制命令入队；不直接调用引擎绘制 | MASTER §8、§10.3 | 保留 |
| 13 | 01 · Intended result / Inspect and record | frame hook／simulation tick不调用Python | MASTER §3.1、§8 | 合并离线工具边界 |
| 14 | 01 · Intended result / Inspect and record | 记录EXE身份、图形后端、playset／Mod、地图与路由现状 | MASTER §9.1、§2.1 | 保留 |
| 15 | 01 · Intended result / Inspect and record | 复用已有loader／hook／ImGui／消息／reset设施 | MASTER §2.1、§15 | 合并，不从零重建 |
| 16 | 01 · Intended result / Inspect and record | 修改前checkpoint、部署备份，保留EXE和游戏资产 | AGENTS §2；MASTER §11.2 | 保留 |
| 17 | 01 · Intended result / Inspect and record | 运行discovery，逐能力验证ABI并报告就绪状态 | MASTER §9、§16 | 保留 |
| 18 | 01 · Intended result / Inspect and record | proxy导出转发、loader-lock外初始化、独立loader测试 | MASTER §3.1、§11.2、A02 | 保留 |
| 19 | 01 · Architecture and ownership | UI只拥有选择／命令及复制快照 | MASTER §8 | 保留职责表 |
| 20 | 01 · Architecture and ownership | 适配器仅在验证窗口取数据，稳定handle重新解析 | MASTER §8、§11.1、A13 | 保留 |
| 21 | 01 · Architecture and ownership | engine指针不跨窗口，worker只持自有数据 | MASTER §8；AGENTS §2 | 保留 |
| 22 | 01 · Architecture and ownership | 路由图／版本、不可变视图、原生池与提交各自所有权 | MASTER §8 | 保留 |
| 23 | 01 · Architecture and ownership | render不扫州／建筑、不分配／Dijkstra、不跑GUI效果、不等模拟锁 | MASTER §8 | 保留禁止项 |
| 24 | 01 · Native discovery and verification | 构造、SetPath、类型、append、pass、allocator、常量创建／上传槽 | MASTER §9.2 | 保留能力清单 |
| 25 | 01 · Native discovery and verification | 对象大小／对齐／注册／寿命、完整参数与返回、隐藏参数 | MASTER §9.2 | 保留 |
| 26 | 01 · Native discovery and verification | context字段／标量／点布局／count-capacity／allocator／所有权 | MASTER §9.2 | 保留 |
| 27 | 01 · Native discovery and verification | 几何／常量就绪、buffer替换释放、设备／world reset | MASTER §9.2–9.3、§11 | 保留 |
| 28 | 01 · Native discovery and verification | 不得复用异构建地址、只验prologue、选第一个或猜layout | MASTER §9.1–9.3 | 保留 |
| 29 | 01 · Native discovery and verification | 能力失败明确停用，不放松唯一性 | MASTER §9.2、§14 | 保留 |
| 30 | 01 · Bring up ImGui | 能力状态与禁用原因、起终点、预览／州／市场、筛选／hover | MASTER §3.2–3.3 | 正式入口按用户要求改为原版，ImGui仅诊断 |
| 31 | 01 · Bring up ImGui | 请求／ribbon／upload／hit／pool／耗时诊断 | MASTER §3.3、§10.5 | 保留 |
| 32 | 01 · Bring up ImGui | device和swap-chain存在后初始化，resize／minimize／reset／shutdown | MASTER §3.3、§11.1、A24 | 保留 |
| 33 | 01 · Bring up ImGui | 输入透传／相机、窗口消息、图形状态恢复 | MASTER §3.3、A06/A24 | 保留 |
| 34 | 01 · Bring up ImGui | 不得用ImGui屏幕折线作为正式地图运输线 | MASTER §3.3、§7 | 保留 |
| 35 | 01 · Bring up one native CArrow | 先按名字解析已知类型，不动原版troop定义 | MASTER §3.1、§9.2；02原文 | 保留；具体工具步骤引用原文 |
| 36 | 01 · Bring up one native CArrow | 生成外部Mod资产、launcher启用并重启 | MASTER §3.1 | 被单DLL／早期自有资源挂载覆盖，不恢复玩家playset前置 |
| 37 | 01 · Bring up one native CArrow | 稳定对象存储与retained点／context寿命 | MASTER §8、§9.2–9.3 | 保留 |
| 38 | 01 · Bring up one native CArrow | 构造可能注册，不能原地重构造或未注销先free | MASTER §9.3(5) | 保留 |
| 39 | 01 · Bring up one native CArrow | 路径非退化／有限／合法数量；用游戏allocator；不传临时host容器 | MASTER §9.2–9.3 | 保留 |
| 40 | 01 · Bring up one native CArrow | 仅验证render-side窗口、live device构建更新 | MASTER §8–9 | 保留 |
| 41 | 01 · Constant-buffer / Submission detail | base可缺uploader；比较base／military／supply虚表真实差异槽 | MASTER §9.3(1) | 保留 |
| 42 | 01 · Constant-buffer / Submission detail | no-op地址多槽，不能以首次stub匹配定槽 | MASTER §9.3(1) | 保留 |
| 43 | 01 · Constant-buffer / Submission detail | 必要时仅逐对象虚表副本，不改共享全局虚表 | MASTER §9.3(1) | 保留 |
| 44 | 01 · Constant-buffer / Submission detail | append完整context参数，仅当前callback有效，不留到Present | MASTER §9.3(2) | 保留 |
| 45 | 01 · Constant-buffer / Submission detail | 观察实际append校准pass，在bucket消费前提交，保留original完整调用 | MASTER §9.3(2–4)、A09 | 保留；明确消费前提交 |
| 46 | 01 · Constant-buffer / Submission detail | 校准军队／tooltip消失后仍独立绘制 | MASTER §9.3(3)、P2、A09 | 保留 |
| 47 | 01 · Constant-buffer / Submission detail | 每对象每选定frame/pass一次、递归保护、不双入口提交 | MASTER §9.3(4) | 保留 |
| 48 | 01 · Geography and route fidelity | 选路、货运距离和绘制使用相同图及cost policy | MASTER §7 | 保留 |
| 49 | 01 · Geography and route fidelity | 接纳authored coastal river，不以河底道路代替 | MASTER §7、A11 | 保留 |
| 50 | 01 · Geography and route fidelity | 有序edge、按方向拼样条、去重接点、保留端点转弯 | MASTER §7 | 保留 |
| 51 | 01 · Geography and route fidelity | 本机map-to-world、尺寸、接缝、wrap、高度／深度；弦仅诊断 | MASTER §7 | 保留 |
| 52 | 01 · Geography and route fidelity | 图成员／连通性／河流／费用变化更新revision，货量与几何分离 | MASTER §7、§10.1 | 保留 |
| 53 | 01 · Freight overlay behavior | shipment保留源／目的／商品／数量／路径；目标输入与已分配分离 | MASTER §4.1–4.4 | 保留 |
| 54 | 01 · Freight overlay behavior | 州输入绿、输出深红、按货量透明度 | MASTER §6 | 后续默认输入绿／输出橙；深红可配置，透明度保留可读下限 |
| 55 | 01 · Freight overlay behavior | 市场物理区段聚合一次，热度非容量瓶颈 | MASTER §4.4、§6 | 保留 |
| 56 | 01 · Freight overlay behavior | hover只商品贡献，离开恢复筛选 | MASTER §3.2、§10.3 | 保留 |
| 57 | 01 · Freight overlay behavior | 市场总量每shipment一次，不能逐段重加全程费用 | MASTER §4.4 | 保留 |
| 58 | 01 · Freight overlay behavior | 商品占比固定全市场分母 | MASTER §3.2.1、§4.4 | 保留 |
| 59 | 01 · Freight overlay behavior | Laid track为唯一已建铁路物理长度，与货运分开 | MASTER §3.2.1、§4.4 | 保留 |
| 60 | 01 · Freight overlay behavior | 全程费用与区段吞吐分离，公开强度统计口径 | MASTER §4.1、§4.4、§6 | 保留 |
| 61 | 01 · Performance requirements | 路由、视图聚合、native几何三个独立缓存 | MASTER §10.1 | 保留 |
| 62 | 01 · Performance requirements | 几何按精确点／图边版本及构造样式，非仅端点 | MASTER §10.1、§13.1 | 保留 |
| 63 | 01 · Performance requirements | 去重、排序不重建、同视图重开复用 | MASTER §10.1、A20 | 保留 |
| 64 | 01 · Performance requirements | 离散色／透明度／宽度，同档零upload、不同构造类型缓存变体 | MASTER §5.2、§6、§10.1–10.2 | 保留 |
| 65 | 01 · Performance requirements | 全局8192，不是每bank8192；仅淘汰未pin且类型兼容 | MASTER §10.2、A22 | 保留 |
| 66 | 01 · Performance requirements | 先验buffer替换／回收，不猜析构，不逐帧重试失败 | MASTER §9.3、§10.2 | 保留 |
| 67 | 01 · Performance requirements | 100ms settle／250ms最长等待、hover最终选择 | MASTER §10.3 | 保留 |
| 68 | 01 · Performance requirements | 关闭立即取消并撤提交；owner优先级／恢复避免互相清空 | MASTER §10.3、§11.1、A19 | 保留 |
| 69 | 01 · Performance requirements | 每帧16次upload、软2ms；超时代价如实记录、不阻塞等全批 | MASTER §10.4、A21 | 保留 |
| 70 | 01 · Performance requirements | 批量日志，保留及时启动／错误证据，不逐行flush | MASTER §10.5 | 保留 |
| 71 | 01 · Performance requirements | 日志3.57秒／4.9ms是历史benchmark不是游戏加速倍数 | MASTER §10.5；01原文 | 保留边界，删除任务书重复旧数字 |
| 72 | 01 · Language-neutral control flow / Deliverables | UI复制ID并代数替换→验证窗口副本→后台聚合→分帧缓存→pass提交 | MASTER §8、§10、§15 | 合并，删除重复伪代码 |
| 73 | 01 · Language-neutral control flow / Deliverables | close取消所属任务、撤提交、安全缓存保留重开 | MASTER §10.3、§11.1 | 保留 |
| 74 | 01 · Language-neutral control flow / Deliverables | 本机native实现、构建部署、能力报告、自有定义、回归、性能、rollback | MASTER §17 | 合并交付清单，自有定义内嵌DLL |
| 75 | 01 · Language-neutral control flow / Deliverables | 区分discovery／unit／loading／实机绘制，未知ABI／寿命disabled | MASTER §9、§13.2、§17 | 保留 |
| 76 | 02 · Discovery recipes | 构造RTTI→COL→vtable→唯一referrer及lambda独立交叉检查 | MASTER §9.2；02原文配方表 | 保留能力要求，配方不再重复抄写 |
| 77 | 02 · Discovery recipes | SetPath由military／supply共同callee和vertex allocator佐证 | 同上 | 同上 |
| 78 | 02 · Discovery recipes | 常量槽唯一差异、creation写字段与uploader读字段对应 | 同上 | 同上 |
| 79 | 02 · Discovery recipes | resolver由typed helper／name builder／string-key lookup链 | 同上 | 同上 |
| 80 | 02 · Discovery recipes | append由type字段／signed bucket ID，bucket list／pass由context访问链 | 同上 | 同上 |
| 81 | 02 · Discovery recipes | allocator RTTI→vtable→唯一static instance，device global来自常量creation | 同上 | 同上 |
| 82 | 02 · Discovery recipes | bucket偏移本机推导；PE unwind／xref／Capstone依赖compiler pattern | MASTER §9.1–9.2；02原文 | 引用原文，不保留第二套发现说明 |
| 83 | 02 · What success means / Not automated | CANDIDATE_FOUND不证运行映像、完整ABI、线程、寿命或GPU所有权 | MASTER §9.1、§13.2 | 保留 |
| 84 | 02 · What success means / Not automated | signature仅重复身份；报告绑EXE hash，runtime核重定位／映像 | MASTER §9.1 | 保留 |
| 85 | 02 · What success means / Not automated | NEEDS_REVIEW查失败记录／调用者，先查第一缺失依赖不放松唯一性 | MASTER §9.2、§14 | 保留 |
| 86 | 02 · What success means / Not automated | context／对象分配／完整ABI／render context／虚表必须本机验证 | MASTER §9.2–9.3 | 合并 |
| 87 | 02 · What success means / Not automated | reset／reload／shutdown／注册寿命及map spline adapter需实测 | MASTER §7、§11、A24 | 保留 |
| 88 | 02 · Creating a local arrow type | asset生成器本机shader／texture校验，不覆盖原游戏／现有文件 | MASTER §3.1、§11.2；02原文命令 | 保留边界，工具细节引用原文 |
| 89 | 02 · Creating a local arrow type | 原生逐对象颜色或独立样式，纹理／overlay／effect／构造类型一致 | MASTER §6、§9.2 | 保留 |
| 90 | 02 · Creating a local arrow type | 默认localFreightPreview及外部Mod启用步骤 | 02原文；MASTER §3.1 | 历史示例保留原文，正式运行由单DLL资源路径覆盖 |
| 91 | 03 · Test in layers（逐项） | Discovery唯一候选／构造交叉／本机报告 | A01/A08 | 保留 |
| 92 | 03 · Test in layers（逐项） | Loader导出／启动／缺能力可继续游戏 | A02/A04 | 保留 |
| 93 | 03 · Test in layers（逐项） | ImGui开关／输入相机／无帧数据库工作／resize-reset | A06/A13/A24；MASTER §3.3 | 调试层保留，正式另验原版UI |
| 94 | 03 · Test in layers（逐项） | Single ribbon短线定位 | A09/A10 | 保留 |
| 95 | 03 · Test in layers（逐项） | Native pass校准消失后仍绘制／单次提交 | A09 | 保留 |
| 96 | 03 · Test in layers（逐项） | Clear无残留／迟到任务不复活 | A19 | 保留 |
| 97 | 03 · Test in layers（逐项） | Coastal river同走廊选路／距离／绘制 | A11 | 保留 |
| 98 | 03 · Test in layers（逐项） | Reopen复用几何 | A20 | 保留 |
| 99 | 03 · Test in layers（逐项） | Filter／hover合并与恢复 | A18 | 保留 |
| 100 | 03 · Test in layers（逐项） | Unchanged view零upload／无重复批次 | A20 | 保留 |
| 101 | 03 · Test in layers（逐项） | Cold market受每帧预算约束 | A21 | 保留 |
| 102 | 03 · Test in layers（逐项） | Memory循环平台／无持续D3D错误 | A22/A23 | 保留 |
| 103 | 03 · Test in layers（逐项） | Lifecycle读档／菜单／device无旧指针或对象 | A24 | 保留 |
| 104 | 03 · Test in layers（逐项） | 先短线隔离ABI，再扩大规模但最终支持大市场 | MASTER §9.3、P2/P7 | 保留 |
| 105 | 03 · Offline helper tests | 395路径／1000不变请求等仅验证参考策略，不渲染游戏 | MASTER §13.2；03原文 | 不重复历史样本数和命令 |
| 106 | 03 · Offline helper tests | 实际native缓存缝等价回归；失败上传／中点变化／style／重复 | MASTER §13.1 | 保留 |
| 107 | 03 · Offline helper tests | pin／不兼容类型拒绝／部分批次取消／debounce／容量／资产名 | MASTER §10.2–10.3、§13.1 | 保留 |
| 108 | 03 · Performance measurement | 相同存档／相机／速度／overlay，关闭vs打开且区分冷／热 | MASTER §10.5、A25 | 保留 |
| 109 | 03 · Performance measurement | frame time与simulation分开，不只FPS | MASTER §10.5 | 保留 |
| 110 | 03 · Performance measurement | 批次requested／visible／upload／hit／fail／pool／CPU／duplicate | MASTER §10.5 | 保留 |
| 111 | 03 · Performance measurement | CPU不含分帧等待；低频process／GPU memory | MASTER §10.5 | 保留并明确CPU口径 |
| 112 | 03 · Performance measurement | DRAW PERF汇总工具只输出数字，不公开raw地址／日志 | MASTER §16；03原文命令 | 私有证据边界保留，分析教程不复制 |
| 113 | 03 · Performance measurement | 稳定／缓存hover零upload，冷批有界，几何／style仅必要上传，排序不重建 | MASTER §10.1–10.4、A20/A21 | 保留 |
| 114 | 03 · Known symptoms（逐项） | 军队移动才画→偶发append依赖 | MASTER §9.3(3)；03症状表 | 保留风险，诊断细节只留原文 |
| 115 | 03 · Known symptoms（逐项） | append崩溃→context参数／ABI／寿命／hook入口 | MASTER §9.3(2)；03症状表 | 同上 |
| 116 | 03 · Known symptoms（逐项） | 宽亮错误→常量／虚槽／重复／材质；selected不是tint | MASTER §6、§9.3；03症状表 | 同上 |
| 117 | 03 · Known symptoms（逐项） | type失败→启动挂载／名字路径／parse／effect texture | MASTER §3.1、§9.2；03症状表 | 同上 |
| 118 | 03 · Known symptoms（逐项） | 河底道路→graph缺authored river或绘制不同edge | MASTER §7；03症状表 | 同上 |
| 119 | 03 · Known symptoms（逐项） | 鼠标卡→重callback／scan／几何／blocking log | MASTER §8、§10；03症状表 | 同上 |
| 120 | 03 · Known symptoms（逐项） | buffer错误→分配／退化context／所有权／device，不能直接断言leak | MASTER §14；03症状表 | 同上 |
| 121 | 03 · Known symptoms（逐项） | 同视图内存增→不复用／bank无界／释放／stale batch | MASTER §10.2、A23；03症状表 | 同上 |
| 122 | 03 · Known symptoms（逐项） | 筛选改货流→反馈修改allocation／economy snapshot | MASTER §4.2；03症状表 | 同上 |
| 123 | 03 · Known symptoms（逐项） | native panel卡开／刷屏→visibility override／unsupported callback，查gui/error日志 | MASTER §3.2、A07；03症状表 | 同上 |
| 124 | 03 · Known symptoms（逐项） | 保留engine管理可见性，不附永久root visibility、不猜回调 | MASTER §3.2 | 保留 |
| 125 | 03 · Reporting completion | 发现／缓存模型／加载不证可见绘制／路线／内存稳定 | MASTER §13.2、§17 | 保留 |
| 126 | 03 · Reporting completion | 可逆备份最后正常集成 | AGENTS §2；MASTER §11.3、A26 | 保留 |
| 127 | 后续用户／项目独有要求 | 单DLL、自动缓存／配置、原子替换、未知版本关闭、原版资产本机派生 | MASTER §3.1；AGENTS §2 | 完整保留，覆盖外部资源Mod步骤 |
| 128 | 后续用户／项目独有要求 | 原版市场／地区／商品正式UI；ImGui默认关闭仅诊断 | MASTER §3.2–3.3 | 完整保留，覆盖原文正式ImGui |
| 129 | 后续用户／项目独有要求 | 本地价格Freight列；市场卡片／工作量／占比；hover不改卡片 | MASTER §3.2.1 | 完整保留 |
| 130 | 后续用户／项目独有要求 | 模型标识、unknown与零、milli-orders、守恒／残量重分配／隔离 | MASTER §4.1–4.4 | 完整保留 |
| 131 | 后续用户／项目独有要求 | Freight中转目标／满足／未满足、来源、反递归，独立供给／注册未定 | MASTER §4.5、A27 | 完整保留，不把R67核算当整体完成 |
| 132 | 后续用户／项目独有要求 | 最终allocated计宽、固定八档、需求反例、双向流／权重距离不改宽 | MASTER §5 | 完整保留 |
| 133 | 后续用户／项目独有要求 | 五档热度固定全市场尺度、商品色、图例、方向／文字辅助 | MASTER §6 | 完整保留 |
| 134 | 后续用户／项目独有要求 | 三个ID分离、定义key、实际加载Mod资源／几何版本，单DLL兼容改图 | MASTER §7.1 | 完整保留 |
| 135 | 后续用户／项目独有要求 | world/revision/generation/owner、同身份换快照拒绝旧结果 | MASTER §8、§10.3；AGENTS §2；STATUS R81/R82 | 保留约束和现状证据入口 |
| 136 | 后续用户／项目独有要求 | 无Git、指定目录备份、不强停／强替DLL、deploy.ps1 | AGENTS §2–3；MASTER §11 | 完整保留 |
| 137 | 后续用户／项目独有要求 | Windows UTF-8无BOM，禁止Get-Content|Set-Content批量中文改写 | AGENTS §2 | 完整保留 |
| 138 | 后续用户／项目独有要求 | 编码前假设／歧义／矛盾暂停，简洁／精准修改，目标及对应验证 | AGENTS §1.1–1.4 | 完整保留 |
| 139 | 后续用户／项目独有要求 | 用户手动UI、自动输入仍停止；候选R82／安装R81 | AGENTS §5；MASTER §2.3；STATUS顶部 | 保留，不触发任何游戏操作 |

## 旧文件结构逐项处理

| 原位置 | 精简处理 |
|---|---|
| AGENTS §1–4 | 保留工作规范、稳定约束、命令、目录保护，避免重写有效规则 |
| AGENTS §5 | 仅保留最新R82恢复摘要并指向STATUS；旧接线描述不当当前事实 |
| AGENTS §6 | 原历史正文逐字移至 history/AGENTS-through-R82.md，根文件只留本次摘要和记录规则 |
| MASTER §1 | 文件职责缩为一个入口段 |
| MASTER §2.1–2.2 | 合并阅读／裁决，明确starter历史说明适用范围 |
| MASTER §2.3 | 移除过时早期基线，指向STATUS及R82身份纠正 |
| MASTER §2.4 | 合并为唯一重读表；每主题／阶段重读及同任务例外仍保留 |
| MASTER §3–11 | 保留产品／数据／宽度／颜色／地图／线程／ABI／缓存／生命周期；只删除各处重复重读段 |
| MASTER §12 | P0–P7的行动／输出／门槛合并为表格，不减少阶段 |
| MASTER §13 | A01–A27和离线／证据层级保留 |
| MASTER §14 | 合并排错步骤／阻塞表／求助模板为直接操作规则，排错配方引用03 |
| MASTER §15 | 合并重复循环、启动动作与伪代码；完整技术约束仍在§8–11 |
| MASTER §16 | 合并两个长模板为必填字段与文件职责；保留事实／推测／决策分离 |
| MASTER §17 | 交付清单压为一段，修正原A01–A26遗漏A27 |
| MASTER §18 | 长启动提示词改为恢复入口，避免重述全任务书 |
| MASTER §19 | 旧基线／整理历史原文移到 history/MASTER_PLAN-through-R82.md；命令／目录以AGENTS为入口 |
| MASTER §20 | 原§20.1–20.80全文移入同一历史文件，根文件仅留最新维护摘要；旧编号在归档可查 |

## 验证范围

校验四份原文SHA-256未变；两处历史章节与备份正文逐字一致；A01–A27逐行不变；保留章节的实质条款无删除；P0–P7、关键数值／公式／后续要求可查；Markdown严格UTF-8无BOM、相对文件链接可解析。构建／游戏测试不适用于本次纯文档变更，也未重报历史测试为本次PASS。

完整修改前文件备份：`E:\Code\v3-analysis\checkpoints\2026-10-23-docs-trim`。历史记录的事实冲突按STATUS最新记录与实测裁决，不改写旧失败历史。
