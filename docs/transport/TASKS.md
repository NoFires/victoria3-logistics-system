# 任务分解（P0–P7）

状态取值：TODO / DOING / PASS / FAIL / BLOCKED / NOT_RUN。完成证据必须可定位；PASS 只覆盖该任务本身，不自动覆盖后续验收。

| 任务 | 阶段 | 依赖 | 验收 ID | 状态 | 证据 / 说明 |
|---|---|---|---|---|---|
| P0-T01 通读原始材料与现状审计 | P0 | — | A01 | PASS | 2026-10-23 四文档 + 仓库文档 + 源码 + PROFILE 已读；见 `WORKLOG.md` |
| P0-T02 建立状态/交接记录 | P0 | P0-T01 | — | PASS | `AGENTS.md`、`docs/transport/*` |
| P0-T03 基线构建与测试 | P0 | — | A02 | PASS（R84 复核） | 构建已可复现：两次完整构建字节一致；完整入口零失败（18 项原生＋引导／部署／运行目录、verify_embedded、原生 GUI 归档／运行时、flow GPU、selftest 560376 亮像素、默认隐藏 vtx0），`native/build/entry-round84.log`；构建＝dist＝已安装 `71dae36d…`／`4ac3a0e8…` |
| P0-T04 差异表与优先缺口 | P0 | P0-T01 | — | PASS | `STATUS.md` / `COMPLETION_AUDIT.md` |
| P1-T01 双 DLL 自包含与资源校验 | P1 | P0-T03 | A03 | PASS（双DLL，R83–R93） | 引导version.dll＋freight/freight.dll；完整部署/导出/目录/资源测试通过，R84及以后有正常EXE入口实机加载证据；最终回滚仍另验 |
| P1-T02 未知版本/损坏缓存拒绝 | P1 | P1-T01 | A04 | PASS（离线） | `native_gui` 哈希拒绝、缓存损坏测试；实机未重跑 |
| P1-T03 卸载与回滚演练 | P1 | P1-T01 | A26 | TODO | `deploy.ps1 -Uninstall` 未按当前候选执行 |
| P1-T04 设置规范路径与迁移 | P1 | P1-T01 | A03–A04 | PASS（R83–R93） | 配置/日志/缓存统一binaries/freight；新配置优先与非破坏迁移接统一入口，实机路径见STATUS；旧根ironroad.ini结论为历史 |
| P2-T01 CArrow 单线 ABI | P2 | P0 | A08–A09 | PASS（历史） | `PROFILE.md`、方向验证回滚点；新版本未重测 |
| P2-T02 相机/长路径/通过性 | P2 | P2-T01 | A10 | DOING | R102修复近景海路贴海床：实时水位夹高，实际VS GPU64场景/两负对照与完整入口通过，51ec975f已部署；用户明确确认海路高度修复成功；长路径/接缝及其他场景仍待 |
| P3-T01 真实样条/coastal/港口 | P3 | P2 | A11–A12 | PASS（离线） | `spline_tests`、`freight_tests`（coastal/未建铁路） |
| P3-T02 路由缓存 | P3 | P3-T01 | A20 | PASS（离线+历史） | `preview_worker_tests` |
| P3-T03 视图聚合缓存 | P3 | P3-T01 | A20 | PASS（离线，R1） | `freight_engine`（allocation + view 双层、上限 32）；未接运行时 |
| P3-T04 原生几何缓存/8192 池 | P3 | P3-T01 | A20–A22 | PASS（离线+历史） | `ribbon_cache_tests`；实机大市场未测 |
| P3-T05 分帧预算 16/2ms | P3 | P3-T04 | A21 | PASS（离线） | `ribbon_cache_tests`；实机未测 |
| P3-T06 UI→货运文档会话 | P3 | P3-T03 | A18–A20 | PASS（离线，R2） | R86–R96真实市场/州快照与原版文档已实机；R97同输入采集保留当前快照/请求，悬停统计独立于地图请求门，异常/作用域变化回归通过；并非完整生命周期验收 |
| P3-T07 合成测试快照 + 货运 ribbon 提交路径 | P3 | P3-T06 | A16–A18、A21 | PASS（离线 + 实机合成，R8） | 实机 58 ribbons、71 uploads、持续 SUBMIT；可见彩色 ribbon；正式 UI 未接 |
| P3-T08 区域 hub 回退（未建铁路假不可达） | P3 | P3-T01 | A11–A12、A14 | PASS（离线，R6） | 最高可用度 hub；48-region 8/96→32/96；全铁路 1280/1296；未实机 |
| P4-T01 安全经济读取窗口 | P4 | P2 | A13 | DOING | R101生产采集已移到普通命令scope完成点，自有邮箱/常驻请求/源world先绑定已接；最终eed941暂停首值与直接重载world2实机通过。当前构建运行态更新、全输入写任务/可靠退休事件及ABA完整证明仍待，用户接管后停止自动输入 |
| P4-T02 稳定 ID/市场/铁路状态 | P4 | P4-T01 | A13 | DOING | R82州key/R90真实305市场/R96原版当前可用铁路已接；R98州ID取完整handle与全slot表，不再固定2048；实际建拆/原子性、市场铁路统计与改图实际内容仍待完成 |
| P4-T03 模型分配接线 | P4 | P4-T01、P3-T03 | A14 | DOING | R87共享hub分解/R89快照合并/R90真实市场已修复，原版市场0与地区最终货流实机可见；异市场/不同存档及原子采集完整验收待补 |
| P4-T04 字段字典/来源标识 | P4 | P4-T02 | A15 | DOING | 模型口径见FREIGHT_MODEL；R91/R92中英模型分配及独立订单/Freight账本完整可读；所有最终界面/验收口径仍需汇总 |
| P4-T05 Freight 中转商品计算物流需求 | P4 | P4-T03、P4-T04 | A27 | DOING | revision1目标/满足/未满足与来源贡献模型回归通过；R91/R92原版账本实机可见；一期不建供给池/不注册游戏商品，最终A27多场景待验 |
| P4-T06 真实商品ID／名称／分类／可交易元数据 | P4 | P4-T01 | A13–A15、A27 | DOING | 53目录/43观察商品/TLS有生产读取与异常回归；R90真实市场中英查询和分类实机通过；完整安全窗口/变化场景仍归P4-T01 |
| P5-T01 原版市场入口 | P5 | P1 | A05–A07 | PASS（R90–R92实机） | 原版物流页签、商品图标列头、列表/详情/右键、原版价格/结余保留；两种语言布局与GUI错误0已验 |
| P5-T02 原版数据/动作桥 | P5 | P4-T02、P5-T01 | A05–A07、A18 | DOING | 真实市场0卡片/逐商品工作量/全市场占比已实机，未知/空结果/hover上下文门通过；R94同构建0→2→0市场切换/法国煤炭筛选实机通过；完整最终矩阵仍待验 |
| P5-T03 注册 GUI 数据函数 | P5 | P5-T02 | A05–A07 | CANCELED | `.Get` 可用，改用“写变量 + GUI 读取”；无需数据函数 |
| P5-T03 地区面板视图 | P5 | P5-T02 | A05 | DOING | R86运行时owner与逐行发布、R90真实市场换州已验；R97中英格兰货运列及115条路线75秒/两次采集稳定，不因相同输入清空；不同市场/加载切换最终矩阵仍待验 |
| P5-T04 hover/筛选/关闭协议 | P5 | P5-T02、P3-T06 | A18–A19 | DOING（R90–R92实机局部PASS） | R90–R94搜索/分类/原版动作/关闭与跨市场局部PASS；R97实机40次hover转换无null/GUI重写、市场155秒五次刷新无清空；地区75秒两次刷新保持115路线/列值，搜索/关闭仍撤下旧作用域；完整生命周期待验 |
| P6-T01 固定宽度档 | P6 | P3 | A16 | PASS（离线） | `flow_width.h` + 测试；实机未测 |
| P6-T02 颜色模式/图例 | P6 | P3 | A17 | DOING | R1 固定市场热度尺度；R5/R8 调试图例实机可见（参考 25000）；原版 UI 未接、正式验收未测 |
| P6-T03 分母/铁路去重/统计 | P6 | P4-T03 | A14–A17 | DOING（模型及分母实机PASS） | R90–R92筛选/hover不改全市场分母，订单与Freight账独立可见；真实铁路仍--，不能将模型铁路去重测试当作实机铁路完成 |
| P7-T01 冷/热性能 | P7 | 全链路 | A25 | NOT_RUN | — |
| P7-T02 内存平台 | P7 | 全链路 | A23 | DOING（R91局部PASS） | 20次实际closed/selected日志确认，几何上传不变、两轮内存无持续增长；最终同构建完整筛选压力/长时平台待测 |
| P7-T03 生命周期/读档/resize | P7 | 全链路 | A24 | PARTIAL（R93恢复实测） | 39c原型菜单→重载world2、游戏内直接载入world3均恢复180箭头且无新增上传；最终8115373d同设备菜单恢复、最小化/实际1920↔2560客户区resize通过；异存档/可靠世界事件/新设备重建仍待验 |
| P7-T04 最终交付与回滚 | P7 | 全部 | A26 | TODO | — |

## 下一轮优先（R95当前）

1. 真实已建铁路与原版数据闭环：R95确认奥尔良只有铁路相连、当前builtRailEdges为空；需接实际建成状态，不能将铁路样条直接全部开启。用户已授权自主启动/输入，旧“仅手动”限制已覆盖。
2. 当前有效地图资源及图版本身份：R95窗口线程TLS/24项冻结挂载顺序只读解码已过；继续复制fs+48路径替换与fs+60排除规则、解析实际资源及后台派生图。原版/州key或ID变化/同key换几何均需验，不以嵌入原版图冒充兼容。
3. 真实已建铁路段及物理长度；Freight一期为需求账，独立供给池旧任务已撤销。
4. 新设备资源重建、采集一致性、三轮60秒渲染/模拟性能与最终A01–A27/安装升级卸载回滚。保持未证路径停用，不以重启替代恢复目标。

## R84 阶段1／阶段2（2026-10-23 22:19）

| 任务 | 阶段 | 验收 | 状态 | 说明 |
|---|---|---|---|---|
| 构建可复现与源码-构建-安装一致性 | P0/P1 | A02–A03 | PASS | `-Wl,--no-insert-timestamp`；两次构建字节一致；与 R83 旧构建只差时间戳／校验和 6–7 字节；安装＝dist |
| 完整测试入口一次性运行 | P0 | A02 | PASS | 零失败，`native/build/entry-round84.log` |
| 双DLL实机加载／导出转发／目录落点／早期挂载 | P1 | A02–A04 | PASS（R83 构建实机） | 模块路径、`loader.log` error=0、`freight\data`、VFS 冻结前挂载、ImGui vtx0；`native/build/round84-stage2-session.log` |
| 市场/州面板原生解码链实机验证 | P5 | A05／A07 | PASS（只读探针） | `phase=4`、vtable 全 MATCH、count=56、slot 9=`market_panel`、slot 14=`states_panel`；市场置顶 `marketOpen=1 visibleGoods=40`，州面板 `state=433 local_goods`；`native/build/round84-panel-probe.txt` |
| 原版物流入口正式数值（市场身份） | P5／P4 | A05／A07／A14 | BLOCKED（证据缺口） | 已实测两个空间不通用：GUI 注册表是**惰性 GUI 本地空间**（未开面板 2 条／开面板 14 条／曾 45 条，id 0–22 段），快照 `state+0x48` 为 31–166 段；候选 `+0x374` 被第二样本否掉（本次 `0xFFFFFFFF`）。R53 的“state+0x48=market id”属聚类推断，需重验 |
| GUI 不支持属性（icon margin_right） | P5 | A07 | PASS（实机） | 改用 position/size，`test_native_gui.py` 禁止 `margin_right` 回归；实机新会话 `error.log` 中 `ironroad`/`margin_right`/`not handled` 命中 0 |
| 州面板 Freight 列宽与表头格式 | P5 | A05 | PASS（离线实测 + 已部署） | 行宽按原版 480 px 重算（52/52/78/92/92/54 + 60），表头 534 ≤ 540，价格差异列加宽回 90；表头改为原版同款 `sort_button` |
| 市场页工作量/占比列收紧与表头格式 | P5 | A05 | PASS（离线 + 已部署） | 两列 90→70，wrapper 180→140，标签“工作量／占比”，表头改 `sort_button`；实机无相关报错 |
| 结余列前移 + 列头 | P5 | A05 | BLOCKED（需裁决取舍） | 原版商品行固定 540 px，`last_column` 已用满 290（价格 210 + 右锚定结余 80），要塞入 `市场价格｜结余｜工作量｜占比` 必须让原版价格块让出宽度 |
| 修复重复 null 发布 | P5 | A05／A07 | PASS（实机） | R84e：原每帧一次（约 65/s，日志 399/400 条），改为按发布键去重后同一存档整个会话仅 4 条且都紧跟采集事件；`publish_key_tests` 接入统一入口；证据 `native/build/round84e-publish-dedupe.txt` |
| 市场页列序与列头（市场价格｜结余｜工作量｜占比） | P5 | A05 | PASS（离线 + 已部署） | 原版结余列在页签内隐藏并由自绘数值前移；结余列头用原版 `sort_balance.dds` 图标按钮（tooltip `SORT_BY_BALANCE_TOOLTIP`）；列间 `vertical_divider`；实机 error.log 无报错 |
| 州面板表头占满与不省略 | P5 | A05 | PASS（实测尺寸） | 表头/行分组重排为 `156|276|48`（均 480），价格列 96/96/84 恢复完整文本，货运 48 |
| 州市场字段结构验证 | P4 | A14 | PASS（实机） | `state field probe`：1147 州 × `0x000..0x1FC` 共 128 个 dword，**只有 `+0x48` 能 1:1 区分 52 个市场分组**（值域 31..116）→ 州对象无第二套市场 id，`state+0x48` 是唯一成立的州→市场键 |
| GUI 包装 id → 真实市场 id 的桥 | P4 | A14／A05 | TODO | 用同样结构法在 GUI 市场对象（同会话 11–45 个）中找“与 GUI id 1:1 且值域落在 31..116”的字段（自身或一层指针目标），可只用外部只读探针完成 |
| 面板市场→国别→州集合→state+0x48 推导链 | P4 | A14 | TODO | 下一步：一次性诊断找出 GUI 市场对象与州对象的**国别字段**并交叉验证，再用“该国所有州的 state+0x48 众数”推出面板市场对应的快照市场；每步可核对，不猜 |
| 地区 Freight 列运行时接线 | P5 | A05／A14 | TODO | `NativeStateOwner`／`kOwnerNativeState=15` 已实现但 `overlay.cpp` 未引用；探针已证明州面板可读（`state=433`） |


## R82 新增正式范围：修改地图Mod兼容

| 任务 | 阶段 | 依赖 | 验收 | 状态 | 说明 |
|---|---|---|---|---|---|
| 当前加载地图资源与图版本 | P1/P3/P4 | 实际Mod加载顺序／地图身份 | A04/A10–A14/A20/A24，MASTER_PLAN §7.1 | DOING | R103当前mount/search/replace/exclude同组读取与目录优先级策略回归/原版活进程两次通过；实际目录/ZIP资源解析、地图/省份/样条内容图及改图场景验收仍待 |

## 新增：独立安装／运行目录（2026-10-23）

| 任务 | 阶段 | 依赖 | 验收 | 状态 | 说明 |
|---|---|---|---|---|---|
| DLL子目录自动加载与交付结构 | P1 | 代理导出／早期资源挂载 | A02–A04/A26 | PASS（加载层，R84） | 进程模块证实 `binaries\VERSION.dll`＋`binaries\freight\freight.dll`＋`system32\version.dll` 三者并存，`loader.log` error=0，嵌入资源先于 VFS 冻结与 .asset 枚举挂载；A26 卸载／回滚演练仍 TODO |
| 配置／日志／缓存统一目录及迁移 | P1 | MASTER_PLAN §3.1.1 | A03/A04/A26 | PASS（首启，R84） | 实机 `runtime data=…\binaries\freight\data`，该会话无 LocalAppData 写入；目录不可写／降级场景仍待验 |
