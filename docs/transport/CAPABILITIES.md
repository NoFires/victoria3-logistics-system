# 能力矩阵（verified / disabled / unknown）

> 能力状态绑定当前构建与游戏版本；换版本必须重新验证。`verified` 不等于“玩家可见”，证据层级见 `KNOWLEDGE.md`。

## 加载与部署

| 能力 | 状态 | 证据 | 边界 |
|---|---|---|---|
| `version.dll` 导出转发 | verified（历史） | `dllmain.cpp`、历史启动日志 | 当前候选未在游戏内重跑 |
| 文件 hash / 入口签名门控 | verified（历史） | `game.cpp`、`offsets.h`、`PROFILE.md` | 版本不符必须禁用 |
| 嵌入地图/样式 10 项资源 | verified（R1 复核） | `tools/verify_embedded.py` on `45fc…c402` | 资源内容校验不等于实机加载 |
| MountData 早挂载自有 ZIP | verified（历史+用户确认） | `NATIVE_GUI_PLAN.md`、历史日志 | 不在 VFS freeze 后晚挂载 |
| 市场 GUI 派生补丁与缓存 | verified（历史） | C++/Python 哈希向量、未知版本拒绝 | 不修改安装目录原文件 |
| 市场物流第五页签/商品列表/原版详情 | **用户实机确认** | `NATIVE_GUI_PLAN.md` | 数据/动作未接 |
| 卸载/回滚 | 脚本存在 | `deploy.ps1 -Uninstall` | 当前候选未演练 |
| 设置规范路径与迁移 | verified in-game（R9） | `r9-session.log`：根 ini 自动生成并读取 | 旧 `data\ironroad.ini` 仅作回退/迁移 |
| GUI 变量系统 getter 探针（只读） | verified in-game（R11） | `r11-session.log`：obj/头部字段匹配；未读条目 | 未验证条目/hash/setter/线程；默认关闭 |
| GUI 变量写入（调试，标志位） | verified in-game（R12） | `r12-write.log`：插入/更新长键值成功、读回正确 | 只用于标志；正式 UI 前需线程/回滚验证 |
| 变量数值/文本显示 | available（`.Get` 已验证） | `r15-header.png`：`VAR_GET=long-value…` | 写入需 UI 线程安全窗口；当前默认关闭 |
| 渲染线程变量写入（地图期） | FAIL（R15/R16） | 单 key 失败不崩溃；批量加载期崩溃 | 不得在 Present 线程写；需 UI 线程窗口 |
| UI 线程变量写入窗口 | PASS（R18，带约束） | `r18-getcheck.png`：`.Get` 显示 status/freight；探针读回 7 个长 key | key/值必须 >=16 字符；仅 UI 线程 |
| 正式页签 `.Get` 文本绑定 | PASS（R22） | `r22-clean.png`：物流页签干净内容 waiting/`--`/Freight/Track/货物 | 数据仍为占位，等待真实快照 |
| 页签切换（自动化验证） | PASS（R21） | `Shift+5` 选中第五页签，`r21-tabs-crop.png` | 鼠标点击仍受缩放影响 |
| 物流页签隐藏商品控件 | PASS（R22） | `r22-clean.png`：FILTERS/搜索栏消失 | 其他页签行为不变 |
| 页签重开生命周期 | PASS（R24） | `r24-reopen.png`：Esc→F4→Shift+5 内容正常、无崩溃 | 长时间/主菜单返回未测 |
| 商品行→原版商品面板 | PASS（R24） | `r24-show.png`：钢面板（国际/本地/用途、税/订单） | 仅点击验证 |
| 显示/隐藏按钮状态绑定 | PASS（R25） | `r25-*-row.png`：变量写入后 enabled 状态翻转 | 实际点击未自动化 |
| 隐藏标志→调试 ribbon | PARTIAL（R26） | 日志 `native panel hide flows=1`；`ApplyPendingFreight` 停用/隐藏 | 未在 prototype 视图肉眼验证 |
| 经济脚本回调观测 | PASS（R31，读路径） | 实机读取生产/消费，100 州累计与 UI 市场总量一致（1e5） | 全州/全商品枚举、市场/铁路未接 |
| 真实快照→模型→正式UI | PASS（R35） | `r35-updated.png`：页签显示真实 Freight/Target/Allocated/Unmet | 单位口径/映射/铁路待改进 |

## 原生 CArrow

| 能力 | 状态 | 证据 | 边界 |
|---|---|---|---|
| `GetArrowType(name)` | verified（历史） | `PROFILE.md`；实机解析日志 | 只认当前 EXE |
| 构造函数（0xC0 对象，supply vtable） | verified（历史） | `PROFILE.md`；单线实机 | 不重构造已注册对象 |
| `SetPath` / 顶点缓冲 / 总长 | verified（历史） | `PROFILE.md`；`CAPTURE setpath` | 非法点/退化线在调用前拒绝 |
| 逐对象常量缓冲（slot 15） | verified（历史） | `PROFILE.md`；八档宽度 GPU 回归 | 不全局改共享 vtable |
| 渲染 pass / bucket append（含 context） | verified（历史） | `PROFILE.md`；实机提交计数 | 仅在 pass hook 用本次 context |
| 每帧/每 pass 单次提交 + 去重 | verified（历史+离线） | `arrow_scope_tests` | 不得 Present 与 pass 双提交 |
| 八档宽度 / 九色样式 | verified（离线 GPU） | `test_flow_gpu.py` | 实机观感未全量测 |
| 全局 8192 原生池 / 类型兼容 LRU | verified（离线） | `ribbon_cache_tests` | 实机大市场未测 |
| 16 次/帧 + 软 2 ms 预算 | verified（离线） | `ribbon_cache_tests` | 单个原生调用可超时，须如实记录 |

## 数据与货运

| 能力 | 状态 | 证据 | 边界 |
|---|---|---|---|
| 样条图/coastal/港口转换/路由 | verified（离线） | `spline_tests`、`freight_tests` | 实机长路径/相机未测 |
| 分配模型（守恒/缺货/市场隔离/残量） | verified（离线） | `freight_tests` | 不是引擎记录 |
| 区域 hub 回退（region*100+0..4，最高可用度） | verified（离线，R6） | `freight_tests`；`freight_scale_tests` | 显式模型规则；需真实已建铁路快照复核 |
| 真实地图规模/守恒/池上限 | verified（离线，R6） | `freight_scale_tests`（649 region、有无铁路、≤8192 ribbons） | 合成输入；非实机性能 |
| 固定线宽档 | verified（离线） | `flow_width.h` + 测试 | 实机未测 |
| 市场热度固定尺度 | verified（离线，R1/R5） | `freight_tests`、`freight_engine_tests`；调试面板图例 | 不代表容量/拥堵；原版 UI 未接 |
| 需求/分配/未满足统计 | verified（离线，R5） | `freight_tests`、`logistics_session_tests` | 模型口径；未随过滤变化；原版 UI 未接 |
| 分配缓存 + 视图缓存 + world/revision 门控 | verified（离线，R1） | `freight_engine_tests` | 未接运行时 |
| 后台货运装配/owner 透传/取消 | verified（离线，R1） | `freight_engine_tests` | 未接 overlay/原版 UI |
| UI→货运文档会话（快照门控/文档发布/关闭重开） | verified（离线，R2） | `logistics_session_tests` | overlay 已接线；无快照来源，未实机 |
| 合成测试快照（调试输入） | verified（离线，R3） | `dev_snapshot_tests`；真实地图 48 orders/24 ribbons/3 ms | 不是游戏数据；仅调试面板 |
| 货运 ribbon 分帧上传/提交（调试态） | verified in-game（R8，合成输入） | `r8-session.log`：58 ribbons、71 uploads、持续 SUBMIT；截图可见彩色 ribbon | 仅调试合成输入；正式原版 UI 未接 |
| 多 owner 视图仲裁（基础/hover/调试） | verified（离线，R4） | `logistics_session_tests` | 原生面板 owner 尚无 UI 调用；实机仅调试 owner |
| 路由缓存 + 100/250 ms 合并 + 代数取消 | verified（离线+历史） | `preview_worker_tests` | 实机未全量测 |
| 实时经济读取窗口 | **unknown / disabled** | 无 | 见 `BLOCKERS.md` B01 |
| 原版 UI 数据/动作桥 | **not implemented** | 无 | 页签壳可显示，数值 `--` |

## 生命周期

| 能力 | 状态 | 证据 | 边界 |
|---|---|---|---|
| swap-chain resize | verified（历史代码） | `overlay.cpp` | 读档/主菜单/resize 实机未测 |
| 世界代数取消/过期结果丢弃 | verified（离线） | `preview_worker`/`freight_engine` 测试 | 引擎事件接线未验证 |
| 运行中安全卸载 DLL | **未验证** | — | 采用关游戏后替换 DLL |
| 内存平台 | NOT_RUN | — | 需实机循环 |
