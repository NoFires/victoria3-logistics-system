# 点子与路线图

整理日期：2026-10-24。汇总现有需求与决策，不新增未经用户确认的产品范围。当前事实以 transport/STATUS.md 为准，历史报告不能覆盖新实测。

## 核心设计

| 点子 | 当前落点 | 状态与边界 |
|---|---|---|
| 直接融入原版 UI | native_gui、logistics_ui/panel、market_panel、state_panel | 市场物流页、地区货运列已有局部实机证据 |
| 用真实运输走廊显示货流 | route_data、route_geography、freight_ribbons、game | 沿 authored spline，海岸/港口连接；不是州间直线代替真实路径 |
| 经济观察与展示隔离 | native_economy、native_snapshot、freight_worker | 自有副本交后台；不能把复读稳定等同原子性完整证明 |
| 净供需到最终货流 | freight、freight_engine | 本地满足后分配，最终流分解；共享 hub 修复有回归 |
| Freight 中转需求账 | FREIGHT_MODEL.md、freight | 一期不另建供给池/游戏商品；目标、满足、未满足与来源贡献分开 |
| 数量决定线宽 | flow_width、freight_ribbons | 只用最终 allocated quantity，缺货单独表达 |
| 筛选不改变经济 | logistics_session、goods filter | 卡片随搜索/分类，hover 只改地图，全市场占比分母固定 |
| 三层缓存与预算 | freight_engine、preview_worker、ribbon_cache | 路由/视图/几何分别缓存，8192 总池、每帧16次/软2ms预算；最终性能仍待验 |
| 可逆双 DLL 安装 | proxy、dllmain、embedded_assets、deploy.ps1 | 引导+主体，自包含资源；未知版本拒绝相关能力 |
| 改图 Mod 读取实际资源 | native_vfs、live_vfs_probe | R103已复制挂载/替换/排除规则，尚未完成有效资源解析与派生图 |

## 下一步优先项

1. **实际有效资源与地图身份**：实现同规则目录/ZIP枚举、前缀挂载翻译、源内容复制及后台地图解析。验证覆盖顺序、替换/排除、同 key 换几何；禁止用嵌入原版图冒充改图兼容。
2. **安全快照与世界生命周期**：继续验证当前构建运行态更新、可靠退休事件及地址复用；覆盖过期结果与加载时序。
3. **铁路与单位**：实际建拆、分市场唯一铁路长度、距离标定；缺证据继续显示 `--`，不凭空标 km。
4. **生命周期与性能**：不同存档、新设备重建、完整交互；同存档/相机/速度下区分渲染耗时与模拟进度，完成三轮性能与内存平台验证。
5. **交付验收**：逐项完成 A01–A27、安装升级卸载与回滚；对外说明实际支持版本及已验范围。

详细验收见 [ACCEPTANCE](transport/ACCEPTANCE.md)，任务见 [TASKS](transport/TASKS.md)。

## 已明确的范围裁决

- ImGui 保留为诊断工具，正式界面使用游戏原版面板。
- 允许引导与主体两份 DLL；配置/日志/缓存统一到 freight 目录。
- Freight 一期不要求独立供给机制；不把旧报告中的这个待办重新加入。
- 不修改原版经济，不将测试值伪装成实时数据，不将模型分配称为原版实运记录。
- 读档恢复、改图兼容和性能各自需要证据；局部成功不能替代完整验收。

原始点子与要求保留在 MASTER_PLAN.md、四份 00–03 文档及 transport/DECISIONS.md 中，本页是阅读导航。
