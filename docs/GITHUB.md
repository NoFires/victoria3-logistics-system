# GitHub 上传说明

2026-10-30：已按用户最新要求将当前物流开发更新推送到现有main，源码提交 `a688e34`（90文件），远端完整SHA已核对一致。后续完成更新后继续核对测试／范围并提交推送；功能状态与未完成项以STATUS为准。DLL、构建目录、日志和外部私有证据仍不提交，当前同步不代表完整功能验收。

用户指定展示名：**Victoria 3 Logistics System**。仓库名：`victoria3-logistics-system`，可见性：公开，使用用户 GitHub SSH 推送。

2026-10-24 用户指定名称覆盖上一轮命名建议。已通过 NoFires 的 SSH 推送至 [公开仓库](https://github.com/NoFires/victoria3-logistics-system)，main 已跟踪 origin/main；GitHub API 已确认公开可见，远端提交与本地一致。

仓库简介可用：

> Native freight visualization for Victoria 3, with model allocations, vanilla UI integration, and spline-based map flows.

## 已整理的上传内容

- 生产源码、测试、GUI/shader、构建/部署/离线工具。
- 第三方源码、五份必需派生数据和全部现有需求/设计/决策/历史记录。
- README、完整文件索引、点子与路线图。

源码压缩包是完整开发快照，包含派生地图数据与构建版本研究记录，保留全部开发内容。压缩包根目录内容即仓库根目录；SHA256 清单位于包内 UPLOAD_MANIFEST.json。它不是玩家安装包，也不声称公开分发许可已经核实。

## 排除内容

`.gitignore` 排除 native/build、dist、Python缓存/虚拟环境、日志、dump，以及本机编辑器和凭据文件。上传时不要直接压缩整个工作目录；源码快照已经排除了这些内容。外部 v3-analysis 中的存档、原始截图/内存/反编译证据和备份不在包内。

`data/` 参与资源构建，不可直接删除；缺失时构建不完整。若将来决定公开仓库不携带派生数据，应另做可复现的生成/分发方案及验证，不能只删文件宣布完成。

## 首次上传

1. 用户已授权以 `victoria3-logistics-system` 建立公开仓库，使用 SSH 推送。本地准备与远端实际结果分别记录，未经推送验证不声称上传成功。
2. 检查 README、docs/CODE_INDEX.md 与 docs/IDEAS.md；保留 AGENTS 和当前状态记录。
3. 更换机器构建前调整工具链路径并安装 Python 依赖。文档中的绝对本机路径是历史证据索引，远端不会包含对应文件。
4. 编译产物不提交源码仓库。未来玩家 Release 应单独打包 version.dll + freight/freight.dll，并附适用游戏版本、安装/卸载说明及许可声明。

## 公开发布前尚需处理

- 项目根目录目前没有 LICENSE。MIT、GPL 等授权应由作者选择，本次没有代选。
- Dear ImGui 源文件注明 MIT 并引用 LICENSE.txt，但当前 vendored 目录缺该独立文件；MinHook 源头包含许可文字，仍应汇总其及 HDE 等组件的完整声明。尚未完成许可证补齐与审计。
- data 中是从游戏派生的数据；native/assets 是适配片段。公开分发前核对这些内容的来源与可分发范围，不能因其已在本机仓库便推定可公开。
- 尚无跨机器 CI 与完整最终验收；当前支持范围应按 README/STATUS 如实描述。

旧 PACKAGE_MANIFEST.txt 仅覆盖原始 starter 文本，不能用于校验当前完整工程；此次源码包的清单单独生成，保留旧清单作为历史记录。
