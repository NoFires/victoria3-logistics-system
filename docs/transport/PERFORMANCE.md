# 性能预算与测量

## R70：R69 实际存档／正式 owner 热态提交采样

- PID52316；运行DLL `17268f5ea3074e6072b3b68d7b64c29b837987cf0ebd1ff36b59fdb0e3bb3bba`；直接进程启动continuelastsave，Hidden窗口；未发送UI输入。原版native owner10、真实快照1117州／28554订单／49商品，市场118、37 ribbons、world1、rev1→6；启动ImGui vtx0。
- 采样日志36条SUBMIT，冷阶段uploads=16，之后连续35样本／170.453秒累计uploads=37不变，submitted7966→303596。记录同一场景热态零新增上传；多个wrapped view可能贡献提交计数，不用该计数推算可见线数或FPS。
- 证据 `native/build/runtime-round70.log` 与独立提取的 `runtime-summary-round70.json`。窗口不可操作，地图可见性未观察；相机／分辨率／日期未做受控对照，未测模拟tick／帧时间／hover／reopen，本结果不代表完整性能验收。
- 运行日志6次采集却发布349次实时文档，定位到retained mailbox重复消费。R70候选加入generation去重，回归证明重复Poll不再发布文档且frame仍有效；此候选尚未在游戏中执行，不能宣称已减少实际开销或给出加速比。

## 当前已实现预算（离线策略，来自源码与测试）

| 项目 | 预算 | 实现 | 证据 |
|---|---|---|---|
| 冷几何上传 | 每逻辑帧 ≤16 次 | `RibbonCache::Pump` | `ribbon_cache_tests` |
| 软 CPU 预算 | 2 ms/帧（可被单个原生调用突破） | `RibbonCache::Pump` 累计计时 | `ribbon_cache_tests` |
| 原生对象池 | 全局合计 ≤8192，非每 bank | `RibbonCache` capacity 上限 8192 | `ribbon_cache_tests` |
| UI 合并 | settle ≈100 ms，最长 ≤250 ms | `SettleGate` | `preview_worker_tests` |
| 关闭/世界变化取消 | 立即，不等 settle | `LatestWorker::Cancel`、`FreightEngine::ResetWorld` | `preview_worker_tests`、`freight_engine_tests` |
| 路由缓存 | 128 条（`PreviewWorker`） | `m_routes` | `preview_worker_tests` |
| 视图缓存 | 32 项 LRU（R1） | `FreightEngine::kViewCapacity` | `freight_engine_tests` |
| allocation 缓存 | ≤4（内容变化即清空） | `FreightEngine::kAllocationCapacity` | `freight_engine_tests` |
| 日志 | 批量写，不逐行 flush | `log.cpp` | 历史日志基准 |

## 离线冷/热样例（2026-10-23 R7，真实地图 + 合成快照，不是实际存档）

合成输入为“每个商品成对的生产/消费、八档数量”（覆盖全部宽度档）；数值为 worker 冷构建的 allocation + view + ribbon 时间。

| 快照 | 未建铁路 | 全部铁路视为已建 |
|---|---|---|
| 48 regions | 15 shipments / 58 ribbons / 3.8 ms / 38,948·111,627 orders | 41 / 156 / 15.9 ms / 111,027·111,627 |
| 649 regions | 248 / 655 / 25.8 ms / 613,446·1,452,378 | 627 / 1,801 / 271.8 ms / 1,402,070·1,452,378 |

- 热视图 0.00 ms，hover 视图 ≤0.7 ms（同快照缓存）。
- 1,801 条 ribbon 按 16/帧预算约需 113 帧完成首次上传（GPU 未实测）。
- 无铁路时未满足比例高，来自未建铁路与活跃网络碎片（1229 连通分量），不是重复计费。
- 这些是合成输入上界样例；实际市场、GPU 与模拟开销仍需按 A25 实测，不能换算成游戏帧率。

## 实机观察（R8，合成 TEST INPUT，非正式性能测量）

- 当前候选 `c6f7…9d7b70`，2560×1440，合成 48-region 快照：调试 UI prototype 生成 58 条 ribbon；冷批次累计 **71 次上传**（16/帧预算内），随后 cache hits 达 605；日志持续 `SUBMIT scope=0/1/0`，结束前累计约 153k 次提交；游戏 HUD 显示 60 FPS（非受控测量）。
- 观察期间无崩溃、无 D3D 错误；但未测帧时间分位数、模拟进度、内存平台与真实大市场，不能据此通过 A25。
- 测试后已关闭游戏，`test_snapshot` 恢复 0；证据 `native/build/r8-session.log`。

## 待测（实机，A21/A25）

必须使用同一存档、日期、相机、游戏速度、分辨率与图形设置：

1. 稳定视图连续 60 秒：几何上传应为 0。
2. 同视图/筛选循环 ≥20 次：重开复用，无新增上传；记录 allocation/view cache 命中。
3. 大市场冷构建：完整批次的上传数、未完成数、CPU 构建时间、超预算/取消数。
4. 循环后进程内存采样（低频）：应趋于平台，不随次数持续增长。
5. 帧时间中位数/P95/P99 与模拟进度/tick 分开测量；面板关闭 vs 打开。
6. 不以 FPS 单独宣称模拟无损耗；日志计数不能替代可见结果。

## 已知风险

- `BuildFreightRibbons` 在视图构建时对每条边采样高度并生成 cubic 控制点，大市场冷批次可能受 CPU 限制；需实测。
- `FreightEngine::Build` 在分配失败时会在每次请求重算（不做逐帧重试，但无效输入重复请求仍有成本）。
- 视图缓存按筛选内容逐项保存，32 项上限可能对大市场内存有影响；需实测每项大小（segments + ribbons）。

## R41 实测（真实全州数据，2026-10-23）

| 指标 | 数值 | 说明 |
|---|---|---|
| 数据规模 | 1117 州 × 49 商品 | 全州枚举快照 |
| 分配+视图 compute | **768.12 ms** | 后台 worker（非渲染线程），UI 可交互 |
| ribbon 数 | 989 | native pool 989/8192 |
| 上传 | 989 | `route uploads 989`，0 pending/failed |
| 渲染 | 可见 | 地图细线（宽度档低，全局摊薄） |

## R42 分段剖析（2026-10-23）

| 阶段 | 耗时 | 说明 |
|---|---|---|
| allocation | **741.30 ms** | `AllocateFreight` |
| ├ flow（min-cost 增广） | **729.03 ms** | 7016 shipments，增广循环主导 |
| └ decomp（路径分解） | 11.16 ms | importer O(1) 查表后 |
| view + ribbons | 10.73 ms | 989 ribbons 几何 |
| total | 752.03 ms | 后台 worker |

## R43 并行分配（2026-10-23）

| 指标 | R42 | R43 |
|---|---|---|
| allocation 墙钟 | 741.30 ms | **126.94 ms** |
| total build | 752.03 ms | **133.38 ms** |
| 线程 | 1 | 8 |
| 线程 CPU 累计 flow | 729.03 ms | 893.20 ms |
| ribbons | 989 | 989 |

**结论**：按组并行约 **5.8×** 加速；cancelled/错误语义经 exception_ptr 保留。

## R48 内存平台（2026-10-23）

| 项目 | 数值 |
|---|---|
| 采样 | 18 次 × 10 s（3 分钟） |
| warm 区间 | **7278–7302 MB**（±24 MB，约 0.3%） |
| 趋势 | 后段下降，无持续增长 |
| 周期刷新 | rev 5→6、989 ribbons、~139 ms |

**结论**：3 分钟无泄漏迹象；WorkingSet 含游戏基线（~7.3 GB），mod 增量未单独隔离。




