# Victoria 3 Logistics System

维多利亚 3 货运可视化：在原版市场和地区面板查看模型分配，并沿地图真实运输样条显示货流。

**开发中 · Windows x64 · 当前本机验证版本 Victoria 3 1.13.11**。本项目不是通用版本插件；其他游戏版本须重新验证 ABI。项目原有内部名称为 Ironroad，GitHub 仓库名为 `victoria3-logistics-system`。

## 已有功能

- 原版市场物流页、地区商品货运列，保留原版商品详情与交互。
- 真实州、市场、商品订单和当前可用铁路的读取；后台模型计算与自有快照交接。
- 区分目标、已分配、未满足需求，以及 Freight 中转需求账和货运工作量。
- 原生 CArrow 地图线路、商品悬停与筛选、几何缓存及分帧上传；ImGui 仅调试，默认关闭。
- `version.dll` 引导加载 `freight/freight.dll`，资源内嵌，无玩家 Python 或 launcher playset 前置要求。

这些是已有实现及局部实测结果，不代表全部验收通过。近景海路高度修复已由用户确认；改图资源解析、铁路里程标定、可靠世界事件、新设备/不同存档、完整性能和最终验收仍待完成。最新权威状态见 [STATUS](docs/transport/STATUS.md)。

## 数据含义

当前后端保留**原版贸易量**，国内运输按供需与当前可用连接计算**国内模型分配**，界面分别列出两项。R117接入及验收进度见STATUS；模型不是原版逐笔事件。项目不修改价格、MAPI、订单、建筑或人口需求；筛选和悬停只改变视图。

- 正式线宽使用原版贸易数量＋最终国内模型分配数量；未满足需求不画成已运输货物。
- 货运工作量 = 已分配订单 × 商品权重 × 路径长度；未经标定的长度使用地图单位。
- Freight 一期是观察性中转需求账，系数默认 1 是模型假设；不另建供给池，不注册游戏商品。
- 无数据显示 `--`；国内需求有真实输入时计算分配与未满足。商品占比分母为全市场原版贸易量＋国内模型分配数量，悬停不改变分母。

完整公式见 [FREIGHT_MODEL](FREIGHT_MODEL.md)。

## 构建、验证和部署

开发依赖：Windows、MSYS2 UCRT64 C/C++ 工具链、Python 及 [requirements.txt](requirements.txt)。当前构建脚本固定工具链为 `E:/Program/C Compiler/MSYS2/ucrt64/bin`，其他机器需调整脚本中的路径。生成地图数据需要本机合法安装的游戏。

```powershell
powershell -ExecutionPolicy Bypass -File native/build.ps1
powershell -ExecutionPolicy Bypass -File native/build_tests.ps1
python -B tools/verify_embedded.py
python -B tools/test_native_gui.py
powershell -ExecutionPolicy Bypass -File tools/test_native_gui_runtime.ps1
powershell -ExecutionPolicy Bypass -File tools/selftest.ps1
powershell -ExecutionPolicy Bypass -File tools/test_debug_default.ps1
```

完整测试入口包含部署测试，运行前须关闭游戏。部署与卸载统一使用：

```powershell
powershell -ExecutionPolicy Bypass -File deploy.ps1
powershell -ExecutionPolicy Bypass -File deploy.ps1 -Uninstall
```

构建产物为 `dist/version.dll` 与 `dist/freight/freight.dll`。部署后配置、日志和缓存集中在 `binaries/freight/`；卸载保留这些用户文件。游戏运行时不强替 DLL。完整实机验证步骤见 [TEST_PROCEDURE](TEST_PROCEDURE.md)。

R117首次启动自动释放可编辑文件至 `freight/native-gui/gui/`、`freight/native-gui/localization/` 及 `freight/assets/gfx/`，说明位于 `freight/EDITING_RESOURCES.txt`。关闭游戏后编辑，重启生效；升级保留修改，仅自动更新未改过的默认文件。各目录的 `.defaults` 留存当前发行默认值，删除某个可编辑文件后会重新生成。内容命名的ZIP是自动缓存，无需手工修改。正式地图关系从当前游戏对象读取，不使用写死的ID→key对照表。

`.shader` 文件外层遵循游戏脚本格式，注释用 `#`；HLSL `Code` 块内才使用 `//`。解析错误记录在游戏 `error.log`，不会自动覆盖你的编辑。

## 代码与点子导航

| 内容 | 入口 |
|---|---|
| 完整文件清单与模块职责 | [代码索引](docs/CODE_INDEX.md) |
| 设计点子、实现状态、后续方向 | [点子与路线图](docs/IDEAS.md) |
| GitHub 上传范围、命名与发布准备 | [上传说明](docs/GITHUB.md) |
| 总任务书、稳定约束 | [MASTER_PLAN](MASTER_PLAN.md) · [AGENTS](AGENTS.md) |
| 当前进度、任务与阻塞 | [STATUS](docs/transport/STATUS.md) · [TASKS](docs/transport/TASKS.md) · [BLOCKERS](docs/transport/BLOCKERS.md) |
| 决策、知识与证据记录 | [DECISIONS](docs/transport/DECISIONS.md) · [KNOWLEDGE](docs/transport/KNOWLEDGE.md) · [WORKLOG](docs/transport/WORKLOG.md) |

`native/src/` 是生产代码，`native/test/` 是测试和只读探针，`native/assets/` 是 GUI/shader，`native/third_party/` 是第三方依赖，`tools/` 是离线开发工具，`data/` 是构建必需的五份派生地图数据。目录保持现有布局。

## 发布状态

当前未选定项目许可证，仓库也尚缺完整的第三方独立许可文件。上传 GitHub 不等于已授予开源许可。用户已指定公开上传；项目许可证、第三方独立许可文件及派生数据/资源的可分发范围仍待完善，见 [上传说明](docs/GITHUB.md)。不包含游戏 EXE、存档或外部私有逆向证据。
