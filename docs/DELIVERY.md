# 交付说明（Victoria 3 1.15.0）

> **2026-11-06 当前构建（R225）**：正式线已合入三项经实机验证的更优功能——完整贸易 getter
> `D039C0`＋每州当前 hub `DA2500`、商品缓存代数／失效检查、`state+0x8C0` 分组表＋
> `detailed==grouped` 等值绑定。当前构建主体 `2fac6a05…`／50,220,030 B，引导 `9369c1dc…`；
> 旧已验收 `2028e03b…`／`c8020412…` 保留为回滚副本。
> 同一 1.15 存档含 5 个多 hub 区域（2/3/4 个中心），实机 `selected=4290/4290`、201 ribbons、
> 贸易量13982.47687／铁路510.99MU不变，绑定未报 `native detailed and grouped exports differ`。
> 完整等价入口35/35、两个目标 EXE 机器码 harness、embedded／GUI／GPU／loader、selftest
> （960×960／560,434亮像素）通过；同构建三对 P95 `+0.568／+0.564／+0.714 ms`（≤2），模拟最大
> 降幅 `0.31%`（≤5%）；20轮稳定revision `uploads=0 reused=201/200 pool=230 overrun_frames=0`。
> 设备丢失重建（A24）与官方 PowerShell 脚本仍未完成，其余 A01–A27 PARTIAL 见 `ACCEPTANCE.md`。
> 实际安装＝`dist`（`2fac6a05…`／`9369c1dc…`），`freight.ini` `df06e5c0…` 保持；旧主体以
> `freight.dll.r196a-backup` 保留。

本文件是当前交付的唯一入口；状态、验收与性能细节见 `docs/transport/STATUS.md`、
`ACCEPTANCE.md`、`PERFORMANCE.md`，工作日志见 `AGENTS.md` 第 6 节。

## 1. 交付物与身份

| 文件 | SHA256 | 大小 |
|---|---|---|
| `dist/version.dll`（已验收引导，放 EXE 同目录） | `c80204122791ade4a33f5b61ea28ebb3a96f444bbc6ef215e71a6245af67a107` | 779,879 B |
| `dist/version.dll`（当前构建 9369c1dc） | `9369c1dca460dc24e480390c2de08e198641fb1cd619d0ef07085d825c8beb16` | 779,879 B |
| `dist/freight/freight.dll`（已验收主体） | `2028e03bc288cd89ff1df23c3dd4f94b9413594636ca2781a183d916c7d9d4a7` | 50,025,206 B |
| `dist/freight/freight.dll`（当前构建 2fac6a05） | `2fac6a052cee2b1bd6dec548e5f1b13e4bfede8d881717018a6d1b2bb52ece13` | 50,220,030 B |

- 目标 EXE：Victoria 3 1.15.0，`SHA256 473186e8d8deba993651e0ae4cf02a003b02f5accb99b096a6a0a8bd9506cbaa`。
- 安装布局：`binaries\version.dll` + `binaries\freight\freight.dll`；配置 `freight.ini`、日志
  `freight.log`/`loader.log`、缓存与资源都在 `binaries\freight\`，不依赖 playset、外部 Mod、
  Python 或手工资源。
- 实际安装：2026-11-06 安装当前构建 `2fac6a05…`／`9369c1dc…`，安装＝`dist`；旧已验收构建
  `2028e03b…`／`c8020412…` 保留在回滚副本。安装时必须无游戏进程，以 `.tmp` + rename 原子替换。
- 回滚副本：`E:\Code\v3-analysis\checkpoints\2026-11-06-final-delivery\dist`（已验收的
  `2028e03b…`／`c8020412…`），源码对应交付提交 `3ec16a0` 一带；候选源码为本地 `main` HEAD
  （`f306eab`，代码提交 `b5efd46`）。

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

本次交付在受限会话中使用了等价的 bash 构建/测试入口与原子部署（PowerShell 被工作区沙箱拒绝），
其内容与上述脚本一致；`selftest.ps1` 的判据（`IRONROAD_READBACK=1`、harness 回读）已用同一二进制、
同一环境变量实际通过：960x960、561,019 亮像素（阈值 5,000），证据在
`checkpoints\2026-11-06-final-delivery\readback.ppm` 与 `selftest-run.log`。最终应在可用终端补跑一次
官方脚本存档。

## 3. 原版界面操作

- **市场物流**：打开任意市场（如“不列颠市场”）→“物流”页签：显示“原版贸易＋国内模型分配”、
  贸易量、已铺铁路（MU）、逐商品市场价格/贸易量/占比，以及地图物流带。
- **筛选**：搜索框、商品分类、“显示/隐藏”、颜色（热度/单商品/原生绿）。
- **地区本地价格**：选择州→“本地价格”页签：物流列、商品/方向/方式筛选。
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

- A01–A27 当前构建矩阵见 `docs/transport/ACCEPTANCE.md` 顶部“1.15.0 当前构建矩阵”。
- 性能（同存档/相机/速度 III，三对 65 秒，当前构建 `2fac6a05…`）：P95 增加
  `0.568 / 0.564 / 0.714 ms`（门槛 2 ms），模拟速度最大降幅 `0.31 %`（门槛 5 %），窗口
  61.52–62.59 秒，见 `PERFORMANCE.md`。旧 `2028e03b…` 的 `0.593 / 0.518 / 0.671` 与 `0.46 %`
  仅属旧构建。
- 缓存/压力：稳定 65 秒零新增几何上传；20 轮稳定 revision `uploads=0 reused=201/200`、池
  230/8192、每帧上限 16、`overrun_frames=0`；3 次数据 revision 变化时 18/11 次上传、池
  201→230。旧 `2028e03b…` 的 `reused=185`／池 346 仅属旧构建。
- 部署矩阵（当前构建）：主体缺失 126／损坏 193／整套卸载无 loader／重装 0／只读目录与日志
  静默降级，游戏均正常存活且安装＝`dist`。

## 6. 已知限制（不冒充通过）

1. 1 条海路航线（MINDANAO→NORTH_BORNEO）保留 unavailable，38 条 partial 航线的一端没有当前
   港口接续；数量保留、几何标注未知。
2. 设备丢失重建（A24 剩余项）需真实 GPU/显示设备变更，未在本次会话模拟。
3. 官方 PowerShell 脚本（`build_tests.ps1`/`deploy.ps1`/`selftest.ps1`）因沙箱未执行，已用等价
   入口复验；应在可用终端补跑并归档日志。
4. 省份增删/拆并、其他地图变体、逐像素多相机矩阵与 A01–A27 的其余 PARTIAL 项仍需样本验证。
