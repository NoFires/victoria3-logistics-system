# 当前状态

### 2026-10-24 — GitHub 公开仓库 SSH 上传成功

- 用户创建空远端后，git push -u origin main 成功，189 个已整理文件经用户 SSH 上传至 https://github.com/NoFires/victoria3-logistics-system 。main 已跟踪 origin/main。
- GitHub 未认证 API 确认 private=false / visibility=public / default_branch=main；git ls-remote 与本地 HEAD 6eadb085d3adba5389b0c46e36f5a92a0c779d8f 一致。本次追加发布结果再提交同步。
- 未上传 dist/native/build/日志/缓存或外部私有证据，未改产品源码/部署/游戏状态；许可和派生数据事项仍保留于上传说明。



### 2026-10-24 — GitHub SSH 已恢复，等待创建空远端仓库

- 用户添加公钥后，ssh -T 实际返回 Hi NoFires / successfully authenticated。用户指定使用 Git 命令，停止浏览器创建路径。
- 本地首次提交 2c81d8e，189 文件；origin 已设为 git@github.com:NoFires/victoria3-logistics-system.git。git ls-remote 仍返回 Repository not found；Git 协议不能创建 GitHub 仓库，需要先建立同名公开空仓库，再 git push -u origin main。
- 当前未推送、未创建远端；不把 SSH 成功当作上传成功。产品源码/DLL/游戏未改。



### 2026-10-24 — GitHub 公开上传准备：用户指定仓库名与 SSH

- 用户授权建立公开仓库 victoria3-logistics-system，并使用其 GitHub SSH；覆盖上一轮私有上传/命名建议。README 和上传说明同步名称；不修改内部 Ironroad 标识或产品行为。
- 本机 Git 2.49 可用、gh 未安装；默认及显式 ~/.ssh/id_rsa 连接 GitHub 均返回 Permission denied (publickey)，GitHub 未接受该公钥。未创建远端、未推送，需用户提供正确私钥路径或添加当前公钥并确认用户名。
- 修改前备份 E:/Code/v3-analysis/checkpoints/2026-10-24-github-public-upload；本轮只准备文档与本地 Git，不重建或部署 DLL，不操作游戏。许可与派生数据事项仍如实保留，不代选开源许可证。



### 2026-10-24 — GitHub 整理：完整源码索引与点子路线图

- 用户要求整理所有代码文件与点子以备上传 GitHub。推荐 Ironroad / ironroad-v3，未替用户创建远端、初始化 Git 或推送；保持原有目录与源码，避免破坏构建路径。
- 更新 README 至 R103 已知状态，清除早期未接实时数据/双DLL未实机等过时介绍；新增 docs/CODE_INDEX.md、IDEAS.md、GITHUB.md，汇总模块、全部文件、设计方向、上传范围及剩余许可事项。
- 完整开发快照排除 build/dist/缓存/日志，保留必需 data 与第三方代码；生成逐文件 SHA256 清单并校验 ZIP 一致性和文档链接。未改产品代码，无需重建或部署；产品状态和游戏操控保持原有约束。
- 备份与源码包：E:/Code/v3-analysis/checkpoints/2026-10-24-github-organization。公开前仍须作者选项目许可证、补齐第三方许可并核对派生数据分发范围；当前建议先私有上传。



### 2026-10-24 — R103：真实VFS规则组读取与目录策略通过；地图资源解析继续

- 当前产品保持51ec975f886d1f8859da720e7bb905740391b293c18294bfa4dd2aa54d362e43（35151385B）/引导71dae36d…，dist=安装。R103仅改未接DLL的native_vfs.h、其现有单元测试与live_vfs_probe，未重建/部署产品；R102用户已经明确确认海路高度修复成功。
- 相同EXE哈希7f53e326…、Ghidra victoria3.exe/image140000000按本地逆向技能复核。03B4A8D0把(root,path)登记到fs+48，CPdxArray容量+50/数量+54、元素0x40；排除表fs+60/68/6C、元素0x20。字符串MSVC布局内联16字节、length+10/capacity+18，cap>15为heap。登记将反斜线换斜线，消费比较本身不折叠大小写。
- 原版03B4AF80：排除按字节前缀，连同gui/gui_editor_extra同前缀名也排除；替换匹配当前目录精确相等，再在实际searchPaths找root、选最高优先级上限并保留owner及更高源。子目录递归仍需经过父级枚举，不能把精确规则自行改成全路径前缀。03B5BCA0逐字节字符串相等、041DA8E0有限长度字节比较已独立汇编核对。
- 新DecodeNativeVfsConfiguration将mounts/searchPaths/replacements/exclusions一起读取并复读校验，返回全自有字符串；真实searchPaths字节独立于仅供挂载核对的斜线规范化，避免把等价物理路径冒充原生root比较。NativeVfsDirectoryRootCount只解释已复制规则，不访问引擎或文件。live探针复用此生产解码头，无attach/暂停/写内存/原生getter。
- native_vfs_tests现有统一入口扩展并单独实跑PASS，覆盖内联/heap与UTF-8、空表/显式空规则、顺序、count/capacity/地址溢出、读取失败/截断、对象与字符串及两个表头在组复制期间变化、退休/源修改后副本稳定；目录策略覆盖嵌套/叠加优先级、大小写、未知owner、实际slash字节和原始前缀。游戏运行中未重复完整入口（其部署段明确要求关闭游戏）；此单元不改变产品DLL。
- 用户57776（23:07:08，windowThread36096/module7ff6b6960000/context0）只读两次PASS：24挂载（22目录/2ZIP），0 replacement，1 exclusion=gui/gui_editor；map_data/state_regions等地图目录24源，排除目录0源。原版当前场景证明配置读取，非实际Mod覆盖实测；不擅自启停Mod或输入界面。
- 缺口：尚未实现同规则的物理目录/ZIP有效文件枚举、前缀挂载翻译与递归继承验证、后台地图/省份/港口/spline解析及内容身份缓存，未接运行时自动派生图。下一步先按当前文件和native枚举链核对有效候选，再实现源内容复制，不用嵌入原版图冒充改图。
- 证据/修改前备份E:/Code/v3-analysis/checkpoints/2026-10-24-r103-vfs-rules：replace-registration.c/replace-pair-append.c/enumeration.c、rules-assembly.txt、rules-live.txt/rules-live-repeat.txt、native/build/vfs-round103.log及最终源码清单。原目标仍active，本轮为PROGRESS而非等待用户。

### 2026-10-24 — R102：近景海上路线高度修复已部署，用户实机确认成功

- 当前源码构建=dist=安装：主体51ec975f886d1f8859da720e7bb905740391b293c18294bfa4dd2aa54d362e43（35151385B），引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e（779879B）。ResourcesOnly基于R101最新已验对象重新打包链接；本轮正式C++无改动，只改shader高度。
- 用户手动确认并提供同一海域两图：拉近后海上/港口连接消失、平面远景可见。旧shader高度max(GetHeight(pos.xz),0)可能低于实际水面。参照本机原版pdxborder.shader及jomini_water.fxh的JominiWater常量，新增该include并改max下限为实时_WaterHeight；平面插值、0.35抬高与DepthEnable=yes/DepthWriteEnable=no不变，不改订单/分配/选路。
- flow_shader_tests直接提取实际VertexShader Code编译，在WARP中经VS→GS stream-output读取结果；4水位×4海床/海岸/陆地高度×4缩放平面混合=64例，原坐标/UV保持。旧shader实际行为FAIL：water19/terrain-20/flat0，ribbon0.35 vs期望19.35；修复PASS。test_flow_gpu.py同时验证原8档宽度/9色/双向箭头及旧半UV、旧海床两个负对照，均正确拒绝回退。
- GPU测试已接native/build_tests.ps1。初次完整入口34项中的新增脚本因工具链PATH选中无Pillow的MSYS2 Python而失败，记录tests-round102-first.log；固定GPU测试使用进入脚本前Python解释器后重跑整个统一入口，34行PASS/0 FAIL。embedded、实际GUI归档与布局、selftest（560437亮像素）、默认隐藏vtx0均PASS，deploy.ps1正常安装，两份安装hash=dist，deployed-files.json核对。
- 用户保存并正常退出52300后部署；未自动启动或操作游戏。用户随后明确回复“海路高度修复成功”，本构建近景海路修复取得用户手动实机确认。A10整体仍PARTIAL：本次确认仅覆盖报告的海路高度故障，不扩展为长路径/接缝/其他地图/全部相机与生命周期验收。
- 备份E:/Code/v3-analysis/checkpoints/2026-10-24-r102-water-height，含原eed941 DLL/shader/测试/文档、本机原版参考、用户两图及最终清单；日志native/build/{gpu-round102-before,gpu-round102,build-round102,embedded-round102,gui-runtime-round102,tests-round102-first,tests-round102,gui-round102,selftest-round102,debug-round102,deploy-round102}.log。其余R101与全目标未验项不变。

### 2026-10-24 — R101：整份完成点采集与暂停重载已验；当前由用户手动操控

- 当前源码构建=dist=安装：主体eed941b09eb4e2b6018d524e711d768dae0c584d567d15d7f67c00f51b94f2b2（35151385B），引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e（779879B）。完整build/tests/embedded/GUI结构与实际归档/flow GPU/selftest（560468亮像素）/默认隐藏vtx0通过，39行PASS、0 FAIL；经deploy.ps1安装。
- 生产目录/全州订单/铁路整份复制已移离UI计时器。只在精确EXE/hash/033BD750签名通过后挂原生scope析构，callerCD2BC1且phase4/access2/hidden0/owns1/context1时，在转发原析构前借用本线程已持范围；不另取模拟锁/调用引擎getter。自有副本、完整typed roster和跨解码器一致性检查保留，worker无引擎指针。
- 请求常驻：暂停每条完成命令保留最新副本，运行态30秒节流；UI drain不撤请求，晚成功/失败不能覆盖新版本。原生先按machine/world/stateDb绑定epoch，新world请求Ensure原子替换旧请求，UI采用已有epoch且不清除同world ready副本；world变动立即拒绝旧UI写入/帧提交。数字身份不是可靠退休事件/ABA证明，P4-T01保持DOING。
- 两个反例：519864首份工作量20,267,078.94，初始化后续命令调订单但ready抑制更新；暂停变速后正确20,267,075.76。269206常驻首值已正确，直接重载仍先交新源world1 rev6–8，UI后退休清空，暂停无下条命令而等待。resident-before实际FAIL/resident-after及source-bind回归PASS；候选DLL/源码/日志/截图保留，不抹去失败。
- 最终自主52300（StartTime2026-10-24T22:24:53.236934+08:00，native61820/UI35204），原存档暂停1836-01-01。初始copy rev2/3/4为125/110/110ms，UI接rev4，卡片20,267,075.76。直接同存档重载前未变速/解暂停：352203796原生先bind world2，rev12/13/14复制125/125/110ms；UI352204187采用world2、352204218接rev14，发生在随后F4/开物流页之前。卡片同值、target30892.46/allocated14817.35/unmet16075.11；195路线与native pool world2恢复可见，停用提示随后消失。
- 重载后中英格兰433本地价格货运列与115路线可见。暂停按1后paused1/speed0、同内容沿用当前视图；3轮hover/leave截图可见，区间0 native copy/0 null，9次queue汇总均20,267,075.76。第四轮因前台校验拒绝输入，不计通过；用户要求人类操控后停止自动UI，未启动限时解暂停/退出脚本，当前进程52300交给用户，不自动关闭。
- 交接文件清单eed941-handoff-files.json确认autosave SHA fecc16f7…/9176649B/mtime_ns1792768044907051700与开始一致，freight.ini SHA fb5be418…逐字节一致。日志归档eed941-handoff.log、截图eed941-world2-recovered.png/eed941-world2-state-prices.png、hover汇总eed941-hover-summary.json；备份E:/Code/v3-analysis/checkpoints/2026-10-24-r101-snapshot-handoff。当前构建运行态刷新、不同存档、可靠世界事件/ABA、真实铁路建拆/市场km、改图内容身份、新设备及三轮性能仍待完成。

### 2026-10-24 — R100：模拟完成阶段契约已实机验证；原子采集交接尚未启用

- 当前主体d590417caec94cddc077e0ca6fe209e7fffc57e2b245ce25533d81c41f9fe02a（34,751,412 B），引导71dae36d…（779879 B），源码构建=dist=安装。变更限于默认关闭的probe_access诊断、精确入口签名和诊断字段校验；R97防闪烁/R98订单采集保持。本轮未宣称P4-T01完成。
- 原版job对象vtable0441A6A8的+8=03B292C0是try execute，+10=03B29270才是等待完成（job+60：0未执行/1执行中/2完成）。007C2540、00E78020先逆序try execute，再逐个wait；StateTrades嵌套013460B0/01346260同样wait。CTickTask_UpdateStateTrades的044E6F38→01339950→world+1C00/0133C0C0确认；没有把模板counter的38个调用者全部当CState订单写入者。
- 新增native_access.h返回标量/身份观察，明确不是lease；异常测试覆盖所有字段不可读、未知模式、错应用类型、缺world/DB、TLS/module溢出，接统一入口。0139E220按实际secondary Execute(void* command)转发一次；只在probe_access=1时装钩子，最多记录16次前后标量，不读取订单/改选择/提交worker/GUI/保留对象指针。Game::Ready的完整EXE/hash和新增8字节签名先行验证。
- 全部构建/统一原生入口/资源/GUI结构与实际归档/GPU/selftest（560432亮像素）/默认隐藏vtx0 PASS，汇总38行PASS、0 FAIL。证据native/build/{build,tests,embedded,gui,gui-runtime,gpu,selftest,debug,deploy}-round100.log；首个新增测试编译曾缺optional头，补齐后相关测试和完整入口均实际通过，不将编译错误充当行为复现。
- 自主56932（2026-10-24 18:33:32），UI线程38656，模块7ff6b6960000。临时probe_access=1，原生speed索引0/paused1确认后有界运行：16条有效回调全部thread45624，前后access2/hidden0/owns1/context1，每次日期+6，machine/world/DB逐对相同。表明这次回调实际在持有原版范围的模拟线程，不能改为直接在UI计时器借用此权限。
- 当前会话暂停基线25396订单，运行后接受25395订单，整组109/93ms；唯一WARN为加载期间既有interface unavailable，未出现probe unreadable/错误world失效。owner=0，未做本轮正式面板视觉验收，也未复制完整快照于该钩子中，故不扩展为原子性/全输入写任务/性能通过。
- 有界脚本9次间隔截图，normalClose=true、无problem；原autosave SHAfecc16f78294e294eda3211fdec9e9b89c2fd325685cd20992fd0a2a8398e804、9176649 B及mtime不变；原freight.ini逐字节恢复，probe_access缺省false，当前无游戏进程。checkpoint2026-10-24-r100-simulation-completion含R98旧安装DLL/配置、源文件修改前备份、tested-build.json、d590-session-only.log（按本PID loader marker截取）、live-report.json、join/trade静态证据、save-before/after.json。
- 下一步普通命令完成/暂停首份快照：00CD2810在stage12调用007E1B60（call CD2B3D，return CD2B42）之后、原生范围析构call CD2BBC/return CD2BC1之前仍持scope；只是候选，未装新钩子/宣称ABI验证。需覆盖初始暂停和加载，确认整个输入集合的完成阶段，再接只交自有副本的邮箱与world/revision过期拒绝。可靠世界事件、实际铁路建拆/市场里程、改图内容、新设备/异存档及三轮性能继续待办。

### 2026-10-24 — R99：访问窗口研究取得原版锁/批处理证据；产品仍R98

- 当前产品无改动：主体59e6d3d486a168f77d0dd7a7bcb632b5700755cce35ea43e64f5cb94a86b98c4（34,717,331 B），引导71dae36d…，dist=安装。R97相同内容采集保留快照、hover不清统计的修复保持；R98完整州订单安全复制保持，原统一入口结果仍有效。本轮未改产品源码/构建/DLL。
- 对同一EXE（7f53e326…，静态base140000000）核对GameStateAccess全局0514E580、HiddenGameStateAccess0514E570及原版访问范围033BD640/750。锁取得顺序为0513E058→0513DCA0→root+590，TLS[0]+5AE0是当前线程持有标记、+92C是上下文。原版00E765E0在并行批处理期间主动释放并重取范围，**单一锁/窗口线程/前后复读均不足以证明原子快照**。未按此启用新锁或钩子。
- 自主测试PID49800（17:44:11），模块7ff6b6960000、窗口线程46604。暂停300次：22次access1/own1、278次access3/own0。最低速度限时运行2200次：access1共199、access2共29、access3共1972；30次hidden1。所有观测context0且machine/world/DB地址不变，原生日期样本59883360→59883462。各字段按顺序读取，**并非原子采样，不以组合样本证明锁排他或完整工作线程状态**。
- 有界脚本共9次间隔截图并正常WM_CLOSE退出；区间336736250..336784875内R98接受1147州/25394订单/43商品候选（订单47ms、整组109ms），19行日志中无WARN/ERROR/错误world失效。该窗口owner=0，没有正式面板上屏验收，不能扩展为卡片/箭头稳定性通过。R97/R98已有正式UI证据仍分别见下文。
- 原autosave SHA256 fecc16f78294e294eda3211fdec9e9b89c2fd325685cd20992fd0a2a8398e804、9,176,649 B、mtime不变，未保存；当前无游戏进程。证据：checkpoints/2026-10-24-r99-access-window/{access-static.txt,access-assembly.txt,access-paused.txt,access-unpaused.txt,unpaused-actions.json,bounded-log.txt,save-before.json,save-after.json,build-before.json}。私有access-writers.json有从错误指令边界解出的ADC伪命中，不作实际writer证据；相邻FC3020暂停命令也不是PartialDailyTick执行函数。
- 下一步：CPartialDailyTickCommand主vtable044365F0、实际secondary04436590（对象+18，不能按表在文件中的邻接次序猜）→0139E220→007E0D80→007C8BD0。验证完成阶段是否已join全部输入写任务及其线程/所有权，再设计只交自有副本的采集通道；暂停/加载首份快照路径也需覆盖。P4-T01/可靠世界事件继续DOING；改图、实际铁路建拆/市场里程、新设备/不同存档和三轮性能仍未验收。

### 2026-10-24 — R98：完整州订单安全复制已部署；未暂停更新取得实机证据

- 当前主体59e6d3d486a168f77d0dd7a7bcb632b5700755cce35ea43e64f5cb94a86b98c4（34,717,331 B），引导71dae36d…；源码构建=dist=安装。完整统一原生入口、GUI结构/实际归档、资源、自检560422亮像素、默认隐藏vtx0均PASS；native/build/{build,tests,gui,selftest,debug,deploy}-round98*.log。新增native_economy_tests接统一入口。
- 按本主题重读01/02/03全文。原0..2047枚举、按样本挑vtable及直接读取订单已替换：按当前DB完整slot表取全32位handle，验证CState精确类型、slot/代数、唯一对象和既有地区key/真实市场，批量安全复制订单。counter+8数据/+10容量/+14长度/+20位图与目录长度分配经当前EXE独立汇编/反编译核对，/100转milli-orders保持；容量/位图/负数/不可读拒绝，不伪装零。
- 回归覆盖slot2049/full handle0xAB000801、州/市场0、错type/slot/重复对象、容量/目录/位图边界、负值、不可读、跨州复制期间变化、退休及多解码器联合候选晚期变化。恢复旧2048上限的负对照实际FAIL，正常实现PASS。旧自动field/vtable探针的完整源码已随修改前checkpoint保留；可选历史诊断保留，主路径不依赖猜测或原生lookup调用。
- EconomyReadWindow在商品目录、订单、铁路均复制后复核联合观察。暂态铁路读取失败撤回scope并重试，不能仅凭复制失败制造新world。此机制拒绝已观察到的变化，**不构成原子模拟epoch/完整访问窗口证明**；可靠世界事件/地址复用及P4-T01仍未完成。
- 外部只读生产解码器在原R97暂停46736通过1147州/25396订单/43商品/34铁路，复制94ms；进程句柄关闭后模型重放20,267,075.76与R97一致。46736后续已不存在，正常关闭尝试未找到窗口、无新crash目录，不推断退出原因。最终R98自主47012暂停采集在DLL内订单31–47ms、整组93–110ms，正式卡片与195路线可见；这是采集耗时，不是P95性能验收。
- 最终R98以最低速度限时运行，截图从1836-01-01到01-07。窗口内订单25396→25391，新候选world1/rev7接受；工作量20,267,075.76→19,815,791.98，目标/已分配/未满足30945.01/14716.40/16228.60，195路线可见。该次采集整组93ms、worker138.546ms；窗口内无WARN、无错误world失效。仅一次实际变化，不能扩大为所有未暂停/建拆/原子性或性能通过。
- 47012由限时脚本正常WM_CLOSE退出，未保存；原autosave SHA256 fecc16f78294e294eda3211fdec9e9b89c2fd325685cd20992fd0a2a8398e804、9,176,649 B及mtime均未变，无需恢复。当前无游戏进程。备份/原eaf2安装DLL/原存档与静态/测试/实机证据：checkpoints/2026-10-24-r98-economy-copy；state-counter-static.txt、economy-production-decoder-live.txt、unpaused-{actions,report}.json、unpaused-09.png、59e6-session-only.log、tested-build.json。
- 剩余：验证真正原子访问窗口与可靠世界事件；实际铁路建拆/分市场里程与距离标定、改图实际资源/内容身份、新设备/不同存档、三轮性能及最终A01–A27。稳定复核不能代替上述门槛，目标继续。

### 2026-10-24 — R97：修复周期刷新和悬停清空统计；已部署并实机通过

- 用户报告货运刷新不稳定。先复现相同输入仍清帧与悬停使仍有效统计失效，两份失败日志refresh-round97-{before,hover-before}.log保留；修复后相关会话用例及完整统一入口PASS。
- 采集先确认当前roster仍属session，再逐值比较同world订单、商品全部属性、铁路集合和主市场。内容未变保留已发布快照/revision/frame，不取消worker、不清空GUI；不以hash代替相等。元数据失败清roster后不能走去重捷径，synthetic/retired/stale不匹配。
- 悬停不再发布null。UI统计专用校验要求同一allocation对象、同world/revision、相同基础筛选，仅忽略hoverGood；地图仍使用严格FrameIsCurrent。搜索、市场/州切换、关闭、换世界及同版本数据替换继续撤下旧数据。新增字段变化、悬停未完成、关闭/退休、synthetic及同版本替换反例。
- 当前源码构建=dist=安装：主体eaf2a3190398893fce1802f8d1083561849359f9aabc0b766b4d4d9d4da36d17（34,911,634 B），引导71dae36d…不变。完整build/tests、GUI结构/实际归档、嵌入资源、GPU、自检560437亮像素、默认隐藏vtx0均PASS，deploy.ps1部署；证据native/build/*round97.log。 实机自有ironroad解析命中0；宽泛关键字仍命中6条原版city/fleet资源消息，逐条与R96相同，未改动这些资源，分类见gui-error-classification.json。
- 自主暂停28748实机：市场155.109秒/五次相同采集无null发布、无GUI重写；20轮移入/移出共40次实际hover转换，卡片始终20,267,075.76，地图195↔146/127条路线，统计无清空。地区中英格兰75秒/两次采集保持owner15/gen46/frame1/115路线，uploads530不增，列值可见。空搜索正确0.00、清空查询恢复，关闭撤回旧scope。
- 直接Load同一autosave，world1→2，新快照rev19实际接受，池530槽重新校验后恢复195条路线；world2-recovered.png确认卡片/箭头可见且停用提示消失。去重未拦新world。仅覆盖同设备同存档；不扩大为所有设备/异存档/未暂停更新通过。
- 28748已正常WM_CLOSE退出；全程暂停1836-01-01、未保存，autosave仍9,176,649 B、修改时间2026-10-23 23:07:24。当前无游戏进程。备份/原1666安装DLL/全部证据：checkpoints/2026-10-24-r97-refresh-stability；refresh-report.json、hover-actions.json、state-stable-report.json、eaf2-session-only.log与截图可定位。
- 本单元解决已复现的无变化刷新及hover统计闪烁；真正数据变更仍按安全协议撤回并重算，未暂停采集原子性、实际铁路建拆/市场铁路统计、改图内容身份、新设备/异存档及完整性能仍未完成，不将本轮暂停测试替代这些验收。

### 2026-10-24 — R96：真实可用铁路接入，最终构建已部署并实机核对

- 当前源码构建=dist=安装：主体1666187dda0f9e1d0944c0af9f91024c36c1200ae3f42692fe625bfcea8d3fb9（34,865,300 B），引导71dae36d…。完整原生入口、嵌入资源、原版GUI结构/实际归档、GPU、D3D11自检和默认隐藏均PASS；证据native/build/*round96.log。首轮新增测试缺windows.h的编译失败已修复并完整重跑，不隐去失败日志。
- 生产只读采集取得2329潜在铁路连接、34原版当前可用铁路连接/34物理边；64位原生连接key与物理strip ID分开，格式3追加4271个一对多映射，原几何字节保持。当前type4表示原版当前可用铁路，不能扩大为全部已铺设线路。所有worker输入均为自有副本，失败撤回。
- 最终1666自主暂停会话35744中，市场0正式卡片20,267,075.76、195条ribbon可见；外部复制后关闭进程句柄的模型重放逐值一致，557条shipment使用18条已确认铁路边。仅在自有副本移除铁路的对照为17,857,680.87，未写游戏内存。稳定窗口379.219秒累计uploads保持195；这不替代P95/模拟速度验收。
- 同一会话直接重载日志确认world2重采集及195槽恢复；最终截图前进程已不存在，无新crash目录，不推断退出原因，不将日志扩大为该构建重载后的可见验收。测试未推进日期/保存。当前无游戏进程。
- 市场铁路卡片仍--：缺分市场汇总与距离标定；未暂停复制原子性、实际建拆、改图资源内容身份、新设备/异存档及完整性能仍未完成。证据/回滚：checkpoints/2026-10-24-r96-built-rail，1666-market-rails-visible.png、1666-model-replay.txt、1666-stable-uploads.json、1666-final-freight.log。用户随后报告刷新不稳，转R97优先修复。


### 2026-10-24 — R96候选：铁路状态已接采集，构建/回归中（未部署）

- dist主体1666187dda0f9e1d0944c0af9f91024c36c1200ae3f42692fe625bfcea8d3fb9（34,865,300 B），引导71dae36d…；安装仍8115373d…，候选未部署。完整build-round96.log退出0、verify_embedded/原版GUI结构/实际C++归档PASS；完整原生入口重跑中。首轮仅新增spline_tests缺windows.h编译失败，已补并单独PASS，保留tests-round96-first.log。
- native_rail.h接CollectRealEconomy现有UI线程窗口，输出自有set<物理stripID>。CTravelConnection+20当前类型4为铁路；定义+50是64位CSplineStripId（两29位anchor+6位类型），不是32位物理stripID/运行连接ID。原版12A0910检查端点铁路建筑状态/同市场等后返回4，13B2110把结果写+20；不调用这些原生函数。节点+19为铁路建筑可用，+1B为相邻已建立节点计数，不能混用。
- 原版source spline links存在一对多：native key900→903/type1对应物理601089+601345。gen_splines原来丢弃links，格式3现追加4271个连接映射并验证唯一成员/连通/端点/类型；几何区字节除版本号外与旧格式2完全相同。SplineGraph保留nativeConnections；格式1/2可加载但缺铁路身份，不授权新铁路读取。当前源hash91c0957b…；data/splines.bin新hash444dcfc5…。
- 外部同一生产解码器在自主暂停61752通过2329潜在铁路连接/34当前已建连接/34物理边，长度771.864332地图单位（全世界，非市场/非km）；production-decoder-live.txt。运行6112连接/4399节点、59节点铁路建筑可用；中英格兰有铁路、奥尔良无铁路。路径逐点核对原版floor后fixed-point坐标，image-Z反变换在整数边界允许小于.001的精度区间，不能拿量化结果冒充完整地图内容身份。
- 新铁路回归覆盖一连接多段、未建/移除/新增、未知/重复ID与key、错类型/端点/路径、同key形状变化、地址/计数/容量边界、复制期间变化和请求过期；接统一入口。缓存原有snapshot fingerprint已含builtRailEdges，输入变更会使分配重算，不更改原版经济。未知/失败撤回快照，未建铁路不当已建。
- 61752正常WM_CLOSE退出，暂停1836-01-01未保存/推进。候选尚未游戏内加载/正式路线验收；市场铁路卡片仍保持--（缺分市场物理汇总/距离标定），实际建拆变化与未暂停采集原子性、改图资源resolver/内容身份、新设备/异存档/完整性能仍待完成。备份/原安装DLL/静态反编译/汇编/日志/截图：checkpoints/2026-10-24-r96-built-rail。

### 2026-10-24 — R95：真实VFS顺序已复制，改图资源解析继续

- 当前主体仍8115373d80edf9a5098e295bde488a5cd270c5454f1eb4e929d186879e166c36（33,718,124 B），引导71dae36d…；两份安装DLL hash=dist。仅新增未接入DLL的解码器/诊断/测试，未重建部署产品。EXE与Ghidra副本SHA256均7f53e326…；证据hashes.json。
- 恢复时旧59720已不存在，无新crash目录，不推断退出原因。自主启动48816（09:33:35），截图确认暂停1836-01-01；窗口线程32864的TEB+58→TLS[0]，初始化位TLS+8600、context索引+8648，实测context0。外部只读/线程信息查询，无attach/暂停/写内存/原生getter调用。完成后正常WM_CLOSE退出，当前无游戏进程，未推进/保存。
- native_vfs.h复制当前context冻结挂载链24项（22目录、2ZIP），DLC/自有归档/原版/用户目录顺序与fs缓存searchPaths逐项一致。后端表可在堆上，按3个已验证代码指针识别，不能按表地址范围猜类型。输出只有自有字符串/类型，不交worker引擎地址。live_vfs_probe两次复用该解码器PASS；production-decoder-live{,-repeat}.txt。
- native_vfs_tests覆盖未初始化/未冻结、循环链、未知后端、搜索顺序/计数错、未终止/不可读/溢出字符串、源前缀长度、挂载点、复制期间变化和请求过期，接native/build_tests.ps1。完整入口退出0、25行PASS、零FAIL（native/build/tests-round95.log）。活进程探针同live_panel_probe不接无游戏统一入口。
- Mod替换规则独立于挂载链：038A0620遍历descriptor+C0/CC，03B4A8D0规范化后追加fs+48的stride40字符串对表；03B4AF80枚举时读该表、以searchPaths索引截断低优先级资源，并读fs+60排除目录。**当前解码器未复制这两表，不是最终资源resolver**。下一步复制/复核规则与实际生效资源，接自有后台派生图/内容身份/改图场景；不得以原版图冒充兼容。
- 奥尔良零分配原因：正确SplineEdge布局6×u32+f32显示派生地区45的4个hub共8条incident edges全type1铁路；当前builtRailEdges为空，freight.cpp全部关闭，无活动hub/跨州路径，target234.08保留unmet。这只解释当前模型行为，原版真实已建铁路仍待接入。orleans-edges.json为证据；初次私有错误格式打印作废。
- 备份/静态反编译/截图/日志/只读输出：E:/Code/v3-analysis/checkpoints/2026-10-24-r95-active-vfs。后续从真实铁路、规则表/资源解析继续，不重复已完成的跨市场与同设备恢复局部验收。

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



## R84o 当前状态（2026-10-24 00:30）：市场页列序/列头按反馈定稿，州面板表头占满列

- **市场页列序改为 `市场价格｜结余｜工作量｜占比`**：原版结余列在 `last_column` 之后，无法直接前移。做法是按页签让它让位——`patch_market` 给结余列容器也加 `visible = "[Not(InformationPanel.IsTabSelected('ironroad_logistics'))]"`（与两列依赖度同样处理，原版页签不受影响），再由我们自己的片段在 `last_column` 内、工作量之前**渲染结余数值**（复用原版脚本函数 `Goods.GetMarketBuyAndSellOrdersBalance` 与原版格式键 `MARKET_PANEL_BALANCE_GOLD`／`MARKET_PANEL_BALANCE`／`MARKET_PANEL_BALANCE_BLUE`）。
- **结余列头用游戏自带图标**：按反馈不再用文字/本地化键，改为原版同款 `sort_button` + `sort_button_icons/sort_balance.dds`（25×25，`alwaystransparent`），tooltip 用原版 `SORT_BY_BALANCE_TOOLTIP`（即“按结余排序。销售订单减去购买订单为结余数量。”）。临时加的 `IRONROAD_BALANCE` 键已撤回。
- **列间加分隔线**：表头与数据行都在列之间插入原版 `vertical_divider = {}`。
- **列宽按“占满列”重算**（市场页）：表头 `55 | 210 | 90 | 95 | 95`，数据行 `价格 210 | 涨跌 80 | 结余 90 | 工作量 95 | 占比 95`。
- **州面板表头占满且不再省略**：表头与行按分组总和相等重排为 `156 | 276 | 48` —— 表头 `39×4 | 96,96,84 | 48`，数据行 `48,48,60 | 96,96,84 | 48`（均 480）。价格列回到 96/96/84，“本地价格／市场价格／价格差异”不再被截断。
- **构建／部署／验证**：主体 `d232efff74ccef67a92cf77fce9c5dcd09bbd1f883e730eddf4c12e478b33bc9`（32,277,114 B）、引导 `71dae36d…`；安装＝dist；完整入口**零失败**（`native/build/entry-round84o.log`）；GUI 归档 Python/C++ 逐字一致、`verify_embedded` PASS；实机启动后 `error.log` 中 `ironroad`／属性类命中 **0**。回滚材料 `checkpoints\2026-10-24-r84o-balance-icon`。
- 仍不变：市场身份闸门（A05／A14 未闭合）、A24 需重启、改图／Freight 供给／真实铁路／性能未验收。

## R84m 当前状态（2026-10-24 00:20）：列头/列对齐修正 + **州侧市场字段结构验证（唯一）**

- **州面板列头与列已按“分组总和相等”对齐**：行 `52+52+78 | 80+80+78 | 60 = 480`；表头 `40+40+40+62 | 80+80+78 | 60 = 480`（图标组 182、价格组 238、货运 60 逐段相同），并用脚本量过确认。
- **市场页按反馈只保留结余列**：结余列头用原版键 `MARKET_PANEL_BALANCE`（`MARKET_GOODS_BALANCE_TOOLTIP`）；行内两列“依赖度”在我们的页签隐藏（`visible = "[Not(InformationPanel.IsTabSelected('ironroad_logistics'))]"`），原版页签不受影响。
- **州侧市场字段结构验证（本轮最有价值的结论）**：新增一次性诊断 `state field probe`，采集 1147 个州对象 `+0x000..0x1FC`（128 个 dword），逐个偏移检验“同一 `state+0x48` 分组内恒定、组间互不相同”。实机结果：
  ```
  state field probe: rows=1147 groups=52 candidates=1
    state field +0x048 separates market48 1:1 (groups=52 value range 31..116)
  ```
  即**全部 128 个偏移里只有 `+0x48` 能 1:1 区分市场分组**，州对象中没有第二个空间的市场 id。→ 市场身份的缺口在**面板侧**（`panel+0x70` 是 GUI 包装 id），不在州侧；此前“`state+0x48` 可能不是市场字段”的怀疑可以撤销，该字段是结构上唯一成立的州→市场键。
- **构建／部署**：主体 `20927cbf096914eda7eca49e785a986f15621e404b6cff0c205ccb67c6e6a3e4`（32,276,090 B）、引导 `71dae36d…`；安装＝dist；完整入口**零失败**（`native/build/entry-round84l.log`），GUI 归档与 C++/Python 逐字一致、`verify_embedded` PASS。回滚材料 `checkpoints\2026-10-24-r84l-align`。
- **下一步**：对 GUI 侧做同样的结构法——用多个 GUI 市场对象（同会话内 11–45 个）找“与 GUI id 1:1、值域落在 31..116”的字段（自身或一层指针目标内），即面板包装 id → 真实市场 id 的桥；这一步可以只用外部只读探针完成，不必改 DLL。
- 仍不变：A24 需重启、改图／Freight 供给／真实铁路／性能未验收。

## R84k 当前状态（2026-10-24 00:05）：市场页只留结余列 + 州字段相关诊断

- **市场页按反馈收窄为“市场价格｜工作量｜占比｜结余”**：
  - 表头在 `native/assets/ironroad_logistics.gui` 增了 `sort_button` 形式的**结余**列头（复用原版本地化键 `MARKET_PANEL_BALANCE` 与 `MARKET_GOODS_BALANCE_TOOLTIP`，无需新增文本）。
  - 行内那两列“依赖度”（`MARKET_GOODS_IMPORT_DEPENDENCE`／`MARKET_GOODS_EXPORT_DEPENDENCE`，即截图里 `+212` 右侧那串数）在**我们的页签隐藏**：`patch_market` 给它们的容器加 `visible = "[Not(InformationPanel.IsTabSelected('ironroad_logistics'))]"`，原版其它页签照旧显示。Python 与 C++ 镜像逐字一致（`test_native_gui_runtime` 通过）。
  - 这样行宽也回到原版预算内，之前那截“多余的条”随之消失。
- **新增只读诊断 `state field probe`**（一次性／每 world，`InvalidateEconomy` 重置）：采集 1147 个州对象的 `+0x000..0x1FC` 全部 dword，然后找**按 `state+0x48` 分组恒定、且组间唯一**的偏移。理由是引擎有 `State.GetMarket`（`map_markers.gui` 在用），若州对象同时存 GUI/面板空间的市场 id，该字段必然满足这个结构性质——用结构找字段，不猜偏移。每次采集后会打印候选偏移及其值域（值域小且 1:1 即疑似 GUI 空间 id）。
- **构建**：主体 `cdb02c7832e3a55b1b1ed30a2da78cae3a8bebde4712a5d2139f1c5a0837cb36`（32,268,410 B）、引导 `71dae36d…`；`test_native_gui.py`／`test_native_gui_runtime.ps1`／`verify_embedded.py` 全部 PASS。**未部署**——用户仍在看当前界面，安装仍是 `64ab24f4…`。
- 仍不变：市场身份闸门、地区列 `--`、A24 需重启、改图／Freight 供给／真实铁路／性能未验收。

## R84i 当前状态（2026-10-23 00:05）：GUI 表头格式统一、属性错误清零、列宽按实测收紧

- **修复引擎不认的 GUI 属性**：`native/assets/ironroad_logistics.gui` 的 crate `icon` 原先用 `margin_right`，游戏每次启动报两条错（`Property 'margin_right'(815) not handled` + `Error setting properties for '' (icon)`）。改为只用 `position = { 190 21 }` + `size = { 58 58 }`；并在 `tools/test_native_gui.py` 加断言禁止 `margin_right` 回归。**实机验证：新会话 `error.log` 中 `ironroad`／`margin_right`／`not handled`／`Error setting properties` 命中数 = 0**。
- **州面板列宽按实测重算**：原版行宽 = 60+60+90+100+100+70 = **480 px**；此前补丁把行改成 454+70 = 524，比原版宽 44 → 新列被裁。现改为 52/52/78/92/92/54 + Freight 60 = **480**，表头 42/42/42/58 + 100/100/90 + 60 = **534 ≤ 540**，并把“价格差异”列从 80 加宽到 90（此前被压成省略号）。
- **市场页列宽与文案收紧**：两列 90→**70**（wrapper 180→140），标签由“货运工作量／占全市场货运”缩短为“**工作量／占比**”（英文 `% of all freight` → `% of freight`），tooltip 保留完整解释。
- **表头格式统一**：两处新增表头都由 `textbox` 改为原版同款 **`sort_button`**（`align = hcenter|nobaseline`）——州面板“货运”、市场页“市场价格／工作量／占比”。实机 `error.log` 无该控件相关报错，说明 `onclick` 可省。
- **已知硬约束（待决）**：原版商品行是 `size = { @panel_width 50 }`，即**固定 540 px**；行内其它列占 250，原版 `last_column` 已用满 290（价格块 210 + 右锚定结余 80）。因此“市场价格｜结余｜工作量｜占比”四列无法同时容纳，要满足“结余在工作量左边 + 结余列头”必须牺牲原版价格块宽度，需先定取舍。
- 构建：`64ab24f4338a092b1a3da3c94a9fca17f87cf752204825b90e09d05c464f691d`（32,072,202 B），安装＝dist；GUI 补丁、C++/Python 逐字一致、`verify_embedded` 全部 PASS。回滚材料 `checkpoints\2026-10-23-r84i-header-format`。

## R84e 当前状态（2026-10-23 23:26）：修复“重复 null 发布”，发布频率降约两个数量级

- **缺陷**：调试面板默认隐藏且无原生 owner 时，`DrawUi()` 每帧调用 `PublishLogisticsDocument(nullptr)`，导致同一批 GUI 变量被反复重写。修复前实测**最近 400 行日志里 399 条**是 `publish document=0 synthetic=-1`（约 **65 条/秒**）。
- **修复**：`PublishLogisticsDocument` 增加“发布键”去重——只有文档指针或作用域相关状态（`owner`／`world`／`revision`／`panelUnavailable`／`goodsUnavailable`／`worldRetired`）变化时才真正发布。键抽到 `native/src/publish_key.h`；新增 `native/test/publish_key_tests.cpp` 并接入 `build_tests.ps1`（首次未发布状态必发、null→null 相同则跳过、null→有文档必发、任一作用域字段变化必发）。
- **实机前后对比（同一存档）**：修复后**整个会话只有 4 条**发布，且每条都紧跟 `real economy snapshot accepted`，随后 `UI-thread variables published` 照常出现 —— 说明初始 waiting／作用域文本仍会写入、真实变化仍会刷新，不是“把变量刷旧”。5 秒窗口日志仅 +187 B，30 秒 +1,314 B（修复前同窗口约 1,950 条发布）。
- **部署与回归**：主体 `01d2d708063614046f2e09e0e8b8562b2e52f34c322d00c560d20d5d9482b38a`（32,079,882 B）、引导 `71dae36d…`，安装＝dist；完整入口**零失败**（含新测试 `PASS publication dedupe key`）。回滚材料 `checkpoints\2026-10-23-r84e-publish-dedupe`（含上一版 `1c3428e5…`）。证据 `native/build/round84e-publish-dedupe.txt`、`native/build/entry-round84e.log`。
- 仍不变：市场身份闸门、地区列 `--`、`m_nativeWorldRetired` 不恢复、改图／Freight 供给／真实铁路／性能未验收。

## R84d 当前状态（2026-10-23 23:10）：实机读档往返取证；A24 判定 PARTIAL

- **实机生命周期（用户手动：回主菜单 → 重新载入 autosave）**：`[268009171] INFO world changed: invalidating economy snapshot, next world=2`，此前一条为 `WARN native market panel/filter unavailable: interface identity unavailable`；随后新 world 重新采集 `states=1147 orders=25396 goods=43`、`markets=52 primary=33 states=85`，与读档前完全一致；全程 `native session tick owner=0 … ribbons=0`（无旧文档发布、无旧帧提交），进程存活无崩溃、无 ERROR。**安全失效这一段是实机 PASS。**
- **但 A24 仍 PARTIAL，且缺口已精确定位**：`InvalidateEconomy` 把 `m_nativeWorldRetired` 置 `true` 后**从不复位**（全仓库只有 `store(true)`，没有 `store(false)`），9 处原生提交路径都以它为门，范围文本会变成 `map unavailable after reload; restart required`。也就是说**读档后正式视图无法恢复、仍需重启游戏**——这正是总目标明确不允许的“以重启代替生命周期支持”。要修就必须先取得原生对象跨 world 注册／注销／寿命证据，属未验证 ABI，按约束保持停用。
- **州探针第二次结果（关键）**：这次探针落在**加载完成后的完整采集**上（`orders=25396 goods=43`），结果与第一次完全一致——`anchor id=0 state+0x48=112`，12 个指针全为容器 vtable `0x624690`，**没有** `0x045850B0`。→ “州对象→市场对象指针”的否定结论成立，不是加载不完整造成的假象。
- **`+0x374` 正式弃用**：本次会话面板已开（`marketOpen=1 market=0 tab="ironroad_logistics"`），GUI 注册表 11 个市场对象该字段**全部** `0xFFFFFFFF`。三次观测里两次未填，不再作为候选。
- **存档侧**：`continue_game.json` 与新存档 `meta` 确认玩家国家＝**大不列颠（GBR）**、版本 1.13.11；存档为 `SAV0` 容器（8.8 MB，内含 zip，`gamestate` 72 MB 二进制格式），不是纯文本，解析成本过高，暂不作为常规证据源（`native/build/inspect_save.py` 记录了探测结果）。
- 下一步：市场身份仍缺一条可核对的 GUI↔快照桥。可行方向是“面板市场 → 其国别 → 该国州集合 → 这些州的 `state+0x48`”，但先要能用证据读出两侧国别字段；在此之前 `NativeMarketOwner`／`NativeStateOwner` 保持停用，市场与地区列保持 `--`。

## R84c 当前状态（2026-10-23 23:00）：州对象指针诊断给出否定结果，市场身份仍是唯一闸门

- **已部署并可复现的构建**：主体 `1c3428e5eacb77546ac755bd3da7217457abe2aa8240264627fd8a99c71a1530`（32,070,154 B）、引导 `71dae36d…`；安装＝dist。完整测试入口**零失败**（`native/build/entry-round84c.log`，含 selftest 与默认隐藏，均在无游戏进程时跑）。
- **新增 DLL 内一次性诊断**（`state market probe`，不改采集与显示）：选中 economy anchor 州后，枚举该州 `+0x00..0x400` 的指针字段，打印每个指针目标在模块内的 vtable RVA 与 id 样字段。
- **实机结果（否定）**：anchor `id=0`、`state+0x48=112`；扫描到的**全部 12 个**引擎对象指针都是同一容器 vtable `0x624690`（`+0x78/+0xC8/+0x118/…` 的每商品容器），**没有任何 `0x045850B0`（GUI 市场对象）**。即：这个构建里州与市场的关系是**整数 id**，不是对象指针，“州对象→市场对象指针”的方案不成立。
- **连带发现（重要）**：`native_state_owner.h` 的 `CopyNativeStateRoster` 用 `snapshot->orders[].market`（即 `state+0x48`）建立 state→market roster，且 `NativeStateOwner::Update` 以该 roster 为激活前提。所以**地区 Freight 列与市场卡片被同一个未验证字段同时卡住**——不是两条独立任务。
- **工具限制**：Ghidra 对 `MarketPanel.GetMarket` getter（RVA `0x225AC90`）与 resolver 一带**没有函数体**（`disassemble_bytes` 返回 0 条指令、`decompile_function` 报 No function），且脚本执行被禁用（`GHIDRA_MCP_ALLOW_SCRIPTS` 未开），静态确认 `panel+0x70` 语义的成本很高。另注：文档中记录的 resolver RVA `0x10571F0` 在当前程序里是构造 `GetSuppressionEffectDescriptors` 字符串的函数，说明该地址记录本身已过期。
- **下一步（已定方向）**：改用**可验证的推导链**——市场面板显示的是玩家所在国的市场（标题“不列颠市场”），若 GUI 市场对象能读出国别、州对象也能读出所属国别，则 `面板市场 → 国别 → 该国所有州 → 这些州的 state+0x48` 可推出面板市场对应的快照市场，且每一步都可核对。实现前先用一次性诊断把两侧的“国别字段”找出来（同上一个探针的做法：打印候选字段并交叉验证），不猜。**同时要修诊断时序**：本次 `state market probe` 落在加载中的那次采集（`orders=7786 goods=22`），而本会话随后稳定在 `states=1147 orders=25396 goods=43 markets=52 primary=33`（多次一致），下次应把探针门控在“商品元数据完整”的采集上，避免在未完成世界里下结论。
- **备选验证**：`freight.ini` 的 `market_override=N` 是既有调试开关，取 `N=112`（anchor 州的 state+0x48）可绕过身份问题验证“分配→文档→GUI 变量→卡片数值”的完整下游，但它只证明管道可用、不证明身份正确，需明确标注为 override 证据。

## R84b 当前状态（2026-10-23 22:50）：市场身份"两个命名空间"实证＋只读诊断构建

- **静态复核**：`MarketPanel.GetMarket` getter `0x225AC90` 正是取 `panel+0x70` 作为 ID 交给 resolver（PROFILE／NATIVE_GUI_PLAN 记录，wrapper ID 落 `+0x10`），所以 `panel+0x70` 属于**引擎市场 ID 空间**，不是任意字段。
- **GUI 市场注册表实测**（探针新增 `[5c]`，布局取自 `native_goods_filter.h`）：`module+0x059684A0` → entries／`count=45`／`capacity=63`／`lock=0`；id 集合 `0..25,30,31,36,49,50,51,53,54,59,60,61,62,65,88,106,109,110,113,4294967295`。快照的 `33`／`35`／`112` **都不在其中**，而面板市场 `0` **在其中** —— 两个 id 空间不通用已是实测事实。
- **候选映射已被第二样本否掉**：上一会话面板市场对象 `+0x374 = 33`；本次（另一会话/加载完成的存档，快照 52 个市场 id 为 `31..116`）同一字段为 **`0xFFFFFFFF`（未填）**，DLL 诊断行实测 `native market identity gui=0 sim_candidate=4294967295 in_snapshot=0 snapshot_markets=52`。所以 `+0x374` 不是稳定的市场映射，**不得使用**。
- **两个 id 空间不通用已成实测结论**：GUI 注册表 id 是 **GUI 本地、惰性创建**的空间——未开面板时 `count=2`（0 与哨兵），开面板后 `count=14`（`0 1 2 3 4 7 8 9 10 13 14 19 22`），另一会话更多时为 45 条；而快照 `state+0x48` 的市场 id 是 `29..166` 密集区间。两者基本不相交，且注册表数量随视图增长，说明它只是按需创建的包装缓存。
- **由此外推的关键结论**：`state+0x48` 只是“聚类上像市场”，从未与引擎市场 id 对过账（R53 的结论属推断）。以面板 `panel+0x70=0` 与快照 id 区间不相交为证，**快照的市场分组字段本身需要重验**，不能靠猜映射修补。
- **下一步方案（已定，待实现）**：DLL 已在进程内持有每个州对象，可直接沿州对象的指针字段找到它的市场对象（vtable `0x045850B0`）并读该对象的 id——这个 id 与面板 `panel+0x70` **同一空间**，从而用“州→市场对象”取代未验证的 `state+0x48` 分组，消除跨空间映射需求。先用单州诊断验证该指针存在，再改采集与 owner 条件。
- **本轮代码（只读诊断，零视图行为改变）**：`market_panel.h` 新增 `simulationMarket` 字段与 `ReadGuiMarketSimulationCandidate()`（歧义／忙锁／不可读一律返回 false）；`overlay.cpp` 在市场或候选变化时记一行 `native market identity gui=… sim_candidate=… in_snapshot=… snapshot_markets=…`，并在采集时输出**完整** `real economy market ids:`（此前只有 top3）。`NativeMarketOwner` 的激活条件一字未改，正式 owner 仍然拒绝。
- **构建／回归**：完整构建通过；主体 `960d42fe5f41b6f7cbfcd6ad5c84457439184390dd64a952b05e70fa87bdb855`（32,065,546 B，比已装版 +24 KB）；18 项原生回归全 PASS、`verify_embedded` PASS（同 hash）。**未部署**——游戏 pid 48420 仍在运行，未强替 DLL；安装仍是 `4ac3a0e8…`。
- **发现环境依赖**：`build_tests.ps1` 尾部 `test_freight_deploy.py` 会走 `deploy.ps1`，有游戏进程时被守卫拒绝（`Close Victoria 3 before deploying.`）。游戏运行中该步失败属预期拒绝、不是回归，已写入 AGENTS §3。
- 下一步：游戏关闭后部署 `960d42fe…`，在存档中打开 市场→物流 采集 `native market identity` 与完整市场 id 列表，与 GUI 注册表 45 个 id 对账；映射经第二证据确认后，才改 owner 的激活条件。

## R84 当前状态（2026-10-23 22:35）：阶段1基线固定＋阶段2双DLL实机加载＋市场/州面板解码实机验证

- **构建可复现**：`native/build.ps1` 两处链接加 `-Wl,--no-insert-timestamp`，`TimeDateStamp=0`；连续两次完整构建字节完全一致 → 引导 `71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e`（779,879 B）、主体 `4ac3a0e8873dbe75ce945438a1ce476c8583e29d1f3f8d610245ac19e29f2980`（32,041,860 B）。为确认源码未漂移，把本次重建与 R83 已安装构建逐字节比对：仅 3 段共 6–7 字节不同，分别是 PE `TimeDateStamp`、由它派生的 `CheckSum` 与调试目录中的时间戳副本，代码与数据完全一致（`native/build/diff_dll.py`）。
- **完整入口零失败**（`native/build/entry-round84.log`）：`build_tests.ps1` 18 项原生＋引导／部署／运行目录、`verify_embedded.py`、`test_native_gui.py`、`test_native_gui_runtime.ps1`、`test_flow_gpu.py`、`selftest.ps1`（560376 亮像素）、`test_debug_default.ps1`（vtx0）全部退出 0。
- **部署**：无游戏进程时经 `deploy.ps1` 安装，安装＝dist＝源码构建（两份 hash 逐一比对一致）。R83 旧两份 DLL 另存 `checkpoints\2026-10-23-r84-baseline\dist-before-rebuild`；旧运行日志同备份 `…\live-logs`。
- **阶段2实机证据（R83 构建，22:09:19 pid 59636，本次完整取得）**：`freight\loader.log`＝`loaded freight.dll (error=0)`；进程模块实测 `VERSION.dll` 取自 `binaries`（我们的代理）、`freight.dll` 取自 `binaries\freight\`、`version.dll` 取自 `system32`（系统导出转发生效）；主体日志 `native profile ready … 1.13.11 sha256=7f53e326…`（旧构建的 `unsupported build` 警告消失）、`runtime data=…\binaries\freight\data`、`embedded archive mounted … BEFORE VFS configuration freezes`、`embedded archive ready BEFORE gfx/models .asset enumeration`、`native GUI enumeration; prepared=1 mounted=1`、`ImGui ready … drawdata lists=0 vtx=0`（默认关闭）。该会话无新增加载／解析错误，游戏自行正常退出，无崩溃转储。
- **阶段3读数（同一次实机，只读）**：`real goods metadata catalog=53 observed=43 local=2 nontradeable=1`；`real economy collected states=1147 orders=25317 goods=43`；`real economy markets=52 primary=33 states=85 top=33:85 35:75 112:73`；相隔 25 秒两次采样一致，R82 的按州定义 key 绑定在实机成立。
- **阶段2/5 实机面板窗口（22:23:44 pid 48420，本次新部署构建，用户手动打开面板）**：DLL 首帧起加载链同上（`loader.log` 第三次 error=0、`VERSION.dll`=binaries、`freight.dll`=binaries\freight 4ac3a0e8…、system32 转发、资源先于 VFS 冻结、`vtx=0`）。用户把市场面板切到 `ironroad_logistics`、并曾同时打开州面板 `local_goods`、点击过“显示/隐藏”（`native panel hide flows=1/0`）。
- **只读活进程解码器（新诊断工具 `native/test/live_panel_probe.cpp`，复用 `market_panel.h`／`state_panel.h` 生产解码逻辑，仅 PROCESS_VM_READ，不 attach、不暂停、不写）实测**：`phase(machine+0x588)=4`；`idler/gfx/handler` vtable 与 `handler+0xd8` 自指针全部 MATCH；`panel count=56`；slot 9=`market_panel`（name／tab 校验通过，`tab="ironroad_logistics"`），slot 14=`states_panel`（`tab="local_goods"`、`state=433`）。州面板置顶时 `marketOpen=0`／`stateOpen=1`；把市场窗口置顶后 `marketOpen=1`、`visibleGoods=40`（原生分类＋搜索＋`AccessMarketGoods` 缓存链实机可用）。证据 `native/build/round84-panel-probe.txt`。
- **由此定位真正缺口（阶段3身份问题，非解码问题）**：`panel+0x70` 读出 **0**，用户已确认该面板标题为**不列颠市场**（`Market.GetNameNoFormatting`），即 0 是一个真实国家市场的面板/GUI 身份，而快照市场集合来自 `state+0x48`（`markets=52 primary=33 top=33:85 35:75 112:73`），`NativeMarketOwner` 要求 `markets.count(panel.market)` 成立才建立正式 owner；扫描 `market_panel` 前 0x200 字节也没有出现 33／35／112 任一快照市场 id。结论：**GUI 面板市场身份与 `state+0x48` 市场 id 是两个命名空间**，与 R82 的州身份错位同类；owner 正确地拒绝、不猜，`publish document=1` 计数为 0，原版物流页仍为 `--`。未取得经验证的映射前不得用 primary/最大市场顶替。
- **仍未完成**：面板 `market=0` 的语义未定，正式文档未发布，地区列未上屏；`NativeStateOwner`（`native_state_owner.h`／`kOwnerNativeState=15`）已有类但**未接入 overlay 运行时**（`overlay.cpp` 只轮询市场面板），这是地区 Freight 列的直接待办；改图实际资源、Freight 供给、真实铁路、完整生命周期与性能不变。
- **旧日志标注**：`binaries\freight\freight.log` 现含 6 段会话，前 3 段为历史证据、不可用于当前布局——20:43:49 pid29804 与 20:44:42 pid49228 的 exe 为 `f3340c72…`（不支持的构建）、runtime data 仍是 LocalAppData；20:56:57 pid35764 已是 `7f53e326…` 但 runtime data 仍为 LocalAppData；只有 22:09:19 pid59636、22:22:41 pid11116、22:23:44 pid48420 三段属于 `freight\` 目录布局。22:09 段抽出为 `native/build/round84-stage2-session.log`，22:23 面板段抽出为 `native/build/round84-panel-open-session.log`。22:22 那次以 `-continuelastsave` 由本轮启动，22:23:27 游戏日志记录 `Quit: Quit from inside game`（正常退出，无崩溃转储），随后用户在 22:23:44 自行启动。
- 备份／证据：`E:\Code\v3-analysis\checkpoints\2026-10-23-r84-baseline`（旧 DLL、live-logs 原始日志与 freight.ini）、`native/build/round84-panel-probe.txt`（活进程解码结果）。下一步先定 `panel+0x70=0` 的语义，再把 `NativeStateOwner` 接进 overlay 取得地区列（界面由用户手动操作，不自动输入）。

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

## R82 当前状态（2026-10-23；用户手动操作，游戏仍R81）

- 用户截图已确认地区本地价格Freight列可见／--，原价和差额保留，表头过窄需调整；不将其写成数值／全交互验收通过。
- 实际只读采样与C++解码回放：当前州433、market111、local_goods。静态／汇编与实测互证：运行州433→定义key STATE_MIDLANDS（引擎定义ID17）→派生地图ID20。旧adapter的order.region=id错误，旧实机货流／线路的地理正确性及分配总量需重验。
- 新state_region.h复制定义key并核验类型／身份／字符串／重读，正式采集改为按key绑定派生图；未知／歧义／变化拒绝，不猜数字ID。1147/1147实机州映射成功，675唯一键。地区owner／行写入草稿因身份矛盾暂停并私存，未启用。
- 14项回归、完整build和verify_embedded PASS。候选290ee571a172264695f9889430932ca48d131e623364f55f6f660267798a8ebd（32,063,114B），未部署；运行／安装仍R81 234635d6…，未强替DLL或结束游戏。当前候选数值／地图结果、生命周期／性能未实机验证。
- 用户明确当前原版地图并要求兼容改图Mod，已纳入MASTER_PLAN §7.1。key映射只是必要部分；实际加载地图定义／省份／样条与几何指纹／缓存尚待实现，不能因原版全量key匹配宣布mod兼容。
- 下一步：完成运行时地区行发布，修表头省略号／null日志／icon属性；绑定实际加载的map资源和图版本后做原版／改图Mod验收。备份2026-10-23-r82-state-live-bridge；native/build/*round82.log和私有state-region-r82-notes.md／assembly.txt／live.bin，MASTER_PLAN §20.80／AGENTS／WORKLOG同步。

## R81 当前状态（2026-10-23；已部署，用户手动界面验收）

- 地区独立owner15、快照州→市场表和CanPublish门控已实现为候选接口；当前州／市场从自有快照匹配，无primary回退；州0、市场0与signed ID门控保留。重复采样不重启worker，切州复用分配，关闭／切页／reset取消所属owner并保护其他owner。
- 同world／revision换快照也核验源指针；新帧不得配旧州表／旧owner，正式发布同时要求精确文档与当前帧／owner。市场变量限定Market文档，地区结果不能冒充市场摘要。原生decoder、owner与逐行写入尚未接运行时，地区列保持--，不宣称正式地区交互完成。
- 14项原生回归、完整build、verify_embedded PASS。候选234635d6c92231a7102f31044e0dafcabfac45cc776c85276e0a13279f10c1a0（31,984,223B），已通过deploy.ps1部署且安装hash一致，旧R78安装保存在r81 checkpoint。R80的smoke／GUI镜像属于上一候选，实机列宽／解析／当前州和全部生命周期／性能仍待验。
- 恢复授权后游戏已启动PID35764／Responding，GUI提前挂载／枚举与真实快照已记日志。用户再次Escape停止并明确“我手动，安排”，当前由用户操作界面，停止所有自动输入；等待用户停在地区本地价格页取得布局／当前州证据。备份2026-10-23-r81-state-owner；证据native/build/*round81.log。MASTER_PLAN §20.79／AGENTS／WORKLOG／TASKS／UI同步，总目标进行中。

### R81启动补充

- 当前runtime日志发现重复null文档publish与市场卡片icon不支持margin_right错误，下一候选修复；新state GUI尚无对应报错命中，仍不能当可见布局或交互通过。实机日志私存E:/Code/v3-analysis/state-ui-r81-runtime.log；不结束游戏／不替换运行DLL。

## R80 当前状态（2026-10-23；总目标进行中）

- 地区本地价格表的Freight列已生成：保留原版价格／差额／全部排序和原生行交互，仅调整表内列宽；其他页签／非本地商品区块不改。本机GUI的精确hash门槛、C++／Python归档逐字镜像通过，DLL不含复制的原版GUI。
- 新FormatStateFreightGoods仅接受当前州作用域文档，分离市场／州行键；拒绝未知／重复／错误市场／无版本／合成输入。GUI用State.GetIDString＋Goods.GetKey和ready州ID门控，未就绪--。静态链证明ID字符串为有符号int32十进制，已覆盖州0与高位ID、零工作量、hover不改行事实。
- **候选** b257418f7861d39cc1dd6f26287576ec46d2451cea61dfb6390aad7f32a3233a（31,985,143B）；14项全回归／最终UI回归、完整build、GUI与C++归档、verify_embedded、selftest（560208 bright pixels）及默认隐藏PASS。**未部署**，安装仍R78 48f0e495…；新增地区owner／运行时变量发布未接，新列目前保持--。
- 下一步实现州owner与FrameIsCurrent／当前州发布门控，并验证原生上下文后接线。新GUI原生解析／列宽截图、正式交互、铁路／Freight供给／原子快照／完整生命周期仍未验。未启动游戏／恢复Computer Use；本轮只有独立harness运行。
- 备份 E:\Code\v3-analysis\checkpoints\2026-10-23-r80-state-freight-column；证据native/build/*round80*.log、r80-gui-preview及私有state-column-r80-static.md／assembly.txt。MASTER_PLAN §20.78、AGENTS／WORKLOG／TASKS／UI／模型同步。

## R79 当前状态（2026-10-23；总目标进行中）

- 地区当前州桥完成静态恢复与独立离线解码器：states_panel 是 slot14，State typed ID +0x70，原版本地价格 tab=local_goods。Ghidra与Capstone互证，当前安装EXE与分析副本hash一致。
- state_panel.h 返回自有 ID／tab；拒绝错误类型／归属／字符串／历史、无效打开身份和复制期间状态变化，区分关闭与州0。新增 state_panel_tests 接入原测试入口，14项回归及verify_embedded PASS；该解码器没有运行时调用点，尚未取得当前州与快照ID实测对应，不冒充地区Freight列完成。
- DLL未改动／未重建／未部署，候选及安装沿用R78 48f0e495698bcb754bdd400a77cdc2f05c68543bf1c2d880d6f05ff5cb69cf29。未启动游戏或恢复Computer Use。下一步验证州上下文并接owner／行发布，再派生本地价格列；R78新布局可见性、Freight供给／铁路／原子快照和全部生命周期仍待验。
- 完整重读00／01／02／03，重点原版UI、访问窗口、线程／所有权与分层验收；复核模型、UI计划、PROFILE及当前测试。备份 E:\Code\v3-analysis\checkpoints\2026-10-23-r79-state-context；证据 native/build/tests-round79.log、state-context-round79-final.log；私有state-panel-r79-static.md与assembly.txt。MASTER_PLAN §20.77／AGENTS／WORKLOG同步。

## R78 当前状态（2026-10-23；总目标进行中）

- 已接参考图的市场部分：Freight／已铺铁路卡片、价格／Freight／占比表头与商品行数据；筛选卡片及固定分母沿用 R77。行内原版价格图表从本机已校验 GUI 直接派生，按钮／tooltip／右键／原版顺序保留；未打包复制的原价块或纹理。地区本地价格的 Freight 列仍未接。
- 新 FormatFreightGoods 按键规范化：工作量为 weighted order-distance、百分比取全市场；hover／分类／排序不改行事实。未知／合成／重复或非法键拒绝，无效行--；已知零工作量0.00，零分母份额--。原版 GetVariableSystem 用上下文门控，先隐藏、成功完整发布后显示，写入前后复核FrameIsCurrent，移除行清空。
- 既有 GUI 变量经已验证布局查到 value 后直接assign，不再为每个更新创建临时key/value pair；未改新键移动／所有权路径。该优化和新批量逐行写入尚未实机，UI成本及内存平台仍需验证。
- **候选／已部署** `48f0e495698bcb754bdd400a77cdc2f05c68543bf1c2d880d6f05ff5cb69cf29`（31,753,550B）；13项原生回归、完整build、GUI、实际C++归档与Python逐字一致、verify_embedded PASS。确认无游戏进程后经deploy.ps1安装，R76安装另存checkpoint。旧R76 selftest／隐藏测试不能替代本候选实机证据。
- 没有恢复用户Escape停止的Computer Use，没有启动／操控游戏；本候选游戏加载、GUI解析／布局截图、行发布、25ms轮询开销、hover与同帧关闭均未验。下一步继续地区当前州桥与本地价格列；真实铁路、Freight供给、原子快照与完整生命周期仍未完成。
- 重读01／03全文，重点原版UI、所有权／取消、性能与分层验证；备份 E:\Code\v3-analysis\checkpoints\2026-10-23-r78-market-reference-ui，证据 native/build/*round78.log 与 r78-gui-preview（源码预览，不是渲染截图），MASTER_PLAN §20.76、AGENTS／WORKLOG／UI文档同步。

## R77 当前状态（2026-10-23；总目标进行中）

- 已完成参考图统计层：`marketFreight` 固定全市场占比分母，`selectionFreight` 汇总当前州／搜索／分类，`filteredFreight` 仅表示 hover 后的地图贡献。正式卡片使用 selection；hover 不改卡片、份额分母或 Freight 需求。地区逐商品工作量只统计该州最终输入／输出，每笔一次。
- 地区反例先 FAIL，修复后原有句柄 95099／97389 已确认均退出0；13项原生测试、完整build与verify_embedded PASS。候选 `3695238f83be193aa596e101360f53ec6b70c01a27ba5182b0f216aaf9d3787f`（31,174,023B），未部署／未实机；安装仍R76 `15fee7d6…`。证据 native/build/repro-round77.log、tests-round77.log、build-round77.log、embedded-round77.log。
- 这完成数值作用域，地区列、市场卡片／商品列的原版布局和逐行数据发布仍待接线。用户 Escape 停止的界面自动操作未恢复；继续代码／离线验证。备份2026-10-23-r77-reference-freight-scopes。

## R76 当前状态（2026-10-23；总目标进行中）

- 搜索／分类显隐冲突已由用户裁决“恢复显示”。C++ 与 Python 生成器移除旧隐藏改写，完整保留原版 header／search／category 块及折叠规则；新控件保留测试先 FAIL 后 PASS，原四页签／版本拒绝／归档镜像仍通过。
- 物流商品行在继承容器上增加原版 hierarchy enter／leave、Set／Clear／Concatenate／Goods.GetKey 绑定；逐商品变量键携带上下文，迟到 leave 不会抹掉另一行。仅当前元数据／可见 ID／相同上下文可激活临时 hover owner；多行歧义／未知键／读取失败退出 hover；离开恢复基础筛选，关闭／切页／market／world／元数据失效取消基础与 hover，调试 owner 保留。
- 数字 UI 轮询由独立 25ms 窗口定时器驱动，worker 仍按 100／250ms 合并；经济采集／生命周期检查保持原 500ms 节奏。正式渲染／文档发布接纳 hover owner，ImGui 关闭不影响。尚未实测同步延迟／开销，25ms 不证明同帧关闭或安全完整生命周期。
- **候选／已部署** `15fee7d63d34db078fafa1128de1afff1244c2a06b5e68896c15f044c16a5329`（31,174,023 B）。13 项原生回归、完整构建、GUI／C++ ZIP／verify_embedded、selftest（560231 bright pixels）、默认隐藏（vtx0）PASS。确认无 victoria3 进程后经 deploy.ps1 安装，原安装 R69 DLL 和旧运行日志备份在 checkpoint。
- 运行状态发生变化：旧测试 PID52316 不存在；曾观察到可见 PID8032，其后也退出，未自动终止任何游戏进程。部署后 sky.launch_app 报 accessibility window-opened handler did not become ready，复核无游戏进程；**新候选游戏加载／实机 UI／hover／正向缓存均 NOT_RUN**。
- 用户新增参考图和明确口径，见 MASTER_PLAN §3.2.1：地区“本地价格”追加 Freight 工作量列，市场布局包含工作量／铁路卡片及商品工作量／占比列；卡片取当前筛选工作量，占比固定全市场，hover 仅影响地图。这些列／卡片尚未完成，不能把恢复筛选／事件桥当作参考图全部实现。
- 本轮重读 01／02／03，复核原版 GUI／GetKey 静态链／owner与取消。备份 E:\Code\v3-analysis\checkpoints\2026-10-23-r76-native-hover；证据 native/build/*round76*.log。下一步优先实现参考图所需的筛选汇总与当前州逐商品工作量，再接原版行数据；Freight 独立供给、真实铁路、原子快照与完整验收仍未完成。

## R75 当前状态（2026-10-23；总目标进行中）

- 修复正式摘要工作量少显示 1000 倍的问题：分配器已用 orders × goods weight × map distance 计算 `shipment.freight`，视图与文档直接求和；UI 原先再除以 1000。新摘要直接格式化工作量并使用 weighted order-distance 标签，费用／订单数／Freight 中转需求仍分开。
- 7 orders × 10 map units 显示 70.00 的反例先 FAIL 后 PASS；提高费率只改费用，提高权重只改工作量，筛选不改全市场工作量及 Freight 需求。缺失／合成／无 world／负值／NaN／Infinity 拒绝，合法零保留。13 项回归、完整构建、GUI／C++ ZIP 镜像与 verify_embedded PASS。
- 候选 `64485541f14b7252297a0fa96a85bee3c0114e5fbd328c6d0af76e8001d020f8`（30,921,468 B），未部署／未跑新 DLL；游戏 PID52316 仍 R69、Responding、无可操作主窗口，不强制结束或替换 DLL。旧摘要截图不能证明新单位已实机显示正确。
- 另核对 R22 补丁仍隐藏物流搜索／分类控件，与总任务书及 R74 筛选接线冲突；已依据新 AGENTS §1.1 提交异步裁决，等待期间未改显隐代码。原有 GUI 测试／C++ 镜像虽 PASS，却未覆盖控件显隐，证据 `native/build/filters-visibility-investigation.json`。
- 本轮重读 01／02／03 全文，复核 §4.4、Freight 模型／UI 转换边界。备份 E:\Code\v3-analysis\checkpoints\2026-10-23-r75-freight-work-units；证据 native/build/*round75.log。下一步继续正式 hover／逐帧关闭与可操作窗口下的新候选验收；Freight 独立供给、铁路和生命周期仍未完成。

## 文档维护（2026-10-23）

- AGENTS.md §1.1–1.4 已纳入编码前思考、简洁优先、精准修改和目标驱动执行，修正“代码重构svg”笔误，并限定简化范围以避免无关重构。
- 本次仅更新工作规范及维护记录；R74 候选、部署状态和未验收项沿用下文。备份：E:\Code\v3-analysis\checkpoints\2026-10-23-agent-guidelines；文档差异与 UTF-8 无 BOM 检查通过。

## R74 当前状态（2026-10-23；无人值守，总目标进行中）

- 补齐明确市场 ID 0 与旧版“未指定市场”的区别：原版 owner／SelectMarket 显式保留 0，只有未指定选择才自动回退；会话反例覆盖 market 0／primary 7，不能据此认定当前存档市场 0 有效。最终 tests-round74-final.log／build-round74-final.log 均退出 0，13 项回归通过。
- 候选搜索／分类改用原版Market.AccessMarketGoods缓存ID，不在模型中重做本地化匹配。静态确认分类为字节排除表；原版search经过Unicode小写／关键词匹配。安全reader要求唯一market、相同query、dirty／writer未占用、正确行／商品／market／分类及稳定重读；不调用native getter或取锁。
- FreightFilter／Selection新增allowedGoods视图限制，ID顺序规范化；过滤不重算allocation，不改分母、热度、goods目标或Freight目标／满足／未满足。待刷新关闭正式owner并显示waiting；明确空匹配仍有合法市场统计、无ribbon。
- 最终候选 `1a97a6823e2fc72fa7f1f2178807803a4001fc89faee3ccd8df3fdc8b0223cfd`（30,916,271B）；完整build、13项回归、verify_embedded、实机dirty缓存拒绝回放PASS。**未部署／未跑新DLL，没有正向native筛选结果或实际点击搜索／分类的证据**。
- PID52316仍运行R69且Responding；registry实测两项market0／FFFFFFFF，均dirty1、0rows。调试器已detach并确认stopped，无显式经济写入、强制结束／替换DLL。Raw记录和私有回放不分发。
- 本轮完整重读01／03，复核GUI／模型／owner及PROFILE。证据native/build/*round74*.log、E:\Code\v3-analysis\native-filter-r74-*；备份2026-10-23-r74-native-filter-view。最低通过标准为原版语义／拒绝样本与视图不反馈回归，正向缓存／新DLL交互单列待验。
- 下一步安全部署后验证原版筛选ID对应与瞬态等待；继续500ms关闭延迟的逐帧桥、正式hover、Freight独立供给、铁路／完整生命周期。原目标未完成。

## R73 当前状态（2026-10-23；无人值守，总目标进行中）

- 原生市场面板所有权链与IsPanelOpen逻辑经静态／汇编核对；PID52316读取确认当前关闭、tab=default、market无效，实际C++解码回放PASS。当前R69仍强制持有正式owner，证实关闭控制存在缺口；本轮没有图像或打开态交互证据。
- 候选market_panel.h按版本／vtable／所有权／字符串／重读校验安全复制状态；native_market_owner.h仅原版物流页签打开才激活正式视图，选择面板market而非最大市场，显式override保留。关闭／切页／未知市场／缺元数据撤正式owner；重复采样不替换worker请求，其他owner帧保留。正式数值还核验当前owner／frame／world／revision／market。
- 最终候选 `e0a2e7863db4f016091888b4e6d885678fae3149db8a12e6e4f22b58c7a3d1a6`（30,376,351B）。完整build、13项原生回归、实机关闭态字节回放、verify_embedded PASS；**未部署／未运行新DLL，打开／切换／关闭实机待验**。500ms UI定时器同步尚不满足逐帧即时关闭的完整验收。
- 游戏继续运行R69、PID52316 Responding／日志继续，调试器已detach并确认stopped无目标；没有强制结束／替换DLL、原生GUI效果或人工操作。窗口工具仍无游戏目标，不重复请求用户操作窗口。
- 重读01／02／03全文与UI／profile；最低门槛为可复查原版谓词、关闭态实机链、解码／owner回归，交互层单列。证据native/build/*round73*.log、私有E:\Code\v3-analysis\market-panel-r73-*，备份E:\Code\v3-analysis\checkpoints\2026-10-23-r73-native-panel-owner。
- 下一步安全部署时验证当前面板市场与生命周期；继续逐帧关闭桥、正式hover／筛选、Freight独立供给、铁路及完整world安全回收，不缩减原任务。

## R72 当前状态（2026-10-23；无人值守，总目标进行中）

- 实机PID52316／EXE 7f53e326…／base0x7FF7166E0000：53商品的ID／key／类别／local／tradeable按key对原版定义53/53一致；53定义及向量重复字节一致。services15、transportation16、electricity17本地，gold51非本地不可交易；主线程51844 TLS初始化／token／index0／slot0与静态链吻合。两个实际C++解析器回放PASS；无文件序号猜测或首个slot选择。
- 候选正式采集已接goods_snapshot.h，在现有UI定时器线程／精确版本门槛安全复制当前已初始化context，采集前后核对目录，填真实key／分类／local／tradeable；不getter／持引擎锁／初始化TLS。未知ID／缺失／变更则撤正式owner和文档，所有正式数值含Freight显示waiting／`--`，不归零源订单。weight／Freight转换仍假设1。
- 最终候选 `05b58a2d1b85c61ac952b4c38b9744be16c915f4b84c6b6092dd827a2156293b`，30,159,766B；完整build、12项原生回归、实机字节／TLS回放、verify_embedded PASS。**未部署／未在新DLL运行adapter或验证修改后总量**，当前仍R69 `17268f5e…`。不运行占用游戏日志的selftest／debug_default。
- 调试器附加自动TLS断点／Nakama首次C++异常曾暂停，已清除新会话自动断点并正常恢复、detach，确认stopped无目标、游戏Responding／日志继续。无显式经济数据写入、强制结束／替换DLL或人工UI操作。稳定重读不是原子快照／全部生命周期证明。
- 重读01／02／03全文及模型／profile；最低门槛为实机字段／TLS证据、实际解码回放、拒绝输入和接线回归，DLL实机层单列。证据native/build/*round72*.log、私有E:\Code\v3-analysis\goods-context-r72-notes.md；备份E:\Code\v3-analysis\checkpoints\2026-10-23-r72-live-goods。
- 下一步：正常关闭后经deploy.ps1部署候选，验证正式元数据／总量／Freight行；不要求用户操作窗口，期间继续当前市场／owner／铁路／生命周期。Freight独立供给、正式hover、完整读档仍未完成。

## R71 当前状态（2026-10-23；总目标进行中）

- 用户最新要求：暂时人机脱离，后续无人值守推进；不再以用户显示窗口为继续开发的前提。隐藏游戏 PID 52316 仍 Responding，本轮未发送 UI 输入、未暂停／结束、未替换其 DLL。
- 当前 EXE 的 Ghidra＋整函数 Capstone 确认商品 ID、key、local／tradeable 位和数据库布局。新增 `goods_catalog.h`：注入安全 reader、校验 vtable／连续 ID／128 上限／MSVC 字符串／重复 key／复制期间变化，失败不返回部分商品；结果只含自有值。**未连接正式采集器，没有实机元数据验证**。
- 模型新增独立 `tradeable` 分类，排除不可交易商品的跨州分配和 Freight 中转需求，并纳入缓存指纹；本地商品与非本地不可交易商品保持区分。现有真实采集仍使用 goodN／默认 localOnly=false、tradeable=true，此缺口尚未修复到正式输入，不宣称现有总量已正确排除这两类商品。
- 完整构建、12 项原生回归（新增 goods_catalog_tests）和 verify_embedded PASS。候选 R71 `698c8b8dc5486c4fc3f3e96086da208deeaf115b1f2d4b352c22ab0d21b08d56`，29,840,576 B；未部署，运行中仍 R69 `17268f5e…`。游戏运行时未执行 selftest／debug_default。
- 本机 x64dbg 状态调用超时／插件未上线；未附加或中断游戏。静态证据 `E:\Code\v3-analysis\goods-catalog-r71-static.md`；备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-r71-goods-metadata`；测试／构建 `native/build/*round71.log`。
- 本轮完整重读 01／02／03，重点 Freight overlay、ABI、所有权、分层验收；使用 local-reverse-engineering skill。最低通过标准为字节解码拒绝反例、不可交易分配／Freight 排除与缓存失效回归，不扩展为运行时验证。下一步验证安全元数据读取窗口／当前 catalog 身份后接入真实快照；原生 UI／读档／Freight 独立供给仍未验收。

## R70 当前状态（2026-10-23，Asia/Hong_Kong 客户端日期；总目标进行中）

- **运行中**：已部署 R69 `17268f5ea3074e6072b3b68d7b64c29b837987cf0ebd1ff36b59fdb0e3bb3bba`，PID 52316。直接进程启动＋continuelastsave 成功；原窗口工具启动器错误不再作为“游戏不能运行”的证据。由于 Hidden 启动，未暴露可操作窗口，已向用户请求显示；未终止进程，未做 UI 输入。
- 真实数据1117州／28554订单／49商品、59市场，primary118／77州、owner10、37 ribbons；world1、rev1→6、约133ms，调试 UI 启动 vtx0、native持续 SUBMIT。**提交层证据新增，图像可见层仍未验证**。
- 热态170.453秒／35样本，累计uploads恒为37、submission7966→303596（冷阶段另有16 uploads）。证据 runtime-round70.log／runtime-summary-round70.json；仅证明当前场景热态零新增上传，不能扩展为完整A20／A25。
- **候选**：R70 `f3a9a4befc8db63e6f2562ba4a71be6e38b447055aecc38d98756ef440c5e7e3`（29,840,576 B）。修复同一reply每500ms重复构建／发布文档；已应用generation在接收及最终发布去重，当前frame继续有效。新测试先FAIL后PASS；完整构建／11项原生回归／verify_embedded PASS。**未部署／未实机运行**，不替换运行中的DLL。
- 下一步：用户显示当前游戏窗口后验证原版 Freight 需求行／地图可见；游戏正常关闭后可经 deploy.ps1 部署候选，并开启默认关闭的市场 getter 探针取最小样本。Freight供给机制、正式hover／owner、铁路、跨world原生对象安全回收仍未完成。
- 本轮重读01／03全文并复核MASTER_PLAN §10缓存／日志，最小门槛为当前PID真实提交与热态上传计数证据、重复发布反例与修复回归。备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-r70-runtime-publication`。

## R69 当前状态（2026-10-23，Asia/Hong_Kong 客户端日期；总目标仍进行中）

- 修复快照重置后 world 门槛失效、worker 重置被 mailbox 合并／Cancel 丢弃，以及新快照仍留旧帧的问题；补充原生构建／上传／提交前的 FrameIsCurrent 检查与最终发布核验。修复前反例 FAIL，最终 11 项原生回归 PASS。
- 实时采集 world 不再固定 1；失效时递增，刷新前复核旧 anchor；无版本的旧 reader 累计样本只用于诊断，不进入正式数据。OnRenderPass 的 ImGui 可见性残留门槛已改为允许正式 owner。
- 检测到真实 world 替换后，native 注册对象寿命仍未验证，后续原生访问／提交保持停用并提示重启；数字层可采集新 world。此项为临时安全保护，A24 完整读档支持仍未通过。
- 最终候选／已部署 DLL `17268f5ea3074e6072b3b68d7b64c29b837987cf0ebd1ff36b59fdb0e3bb3bba`（29,839,552 B）；build／11 项原生回归／GUI runtime／verify_embedded／selftest（560192 bright pixels）／debug_default（0 UI vertices）PASS。日志 `native/build/*round69*.log`，修复前反例 `repro-round69.log`；deploy.ps1 部署成功，旧安装 R67 另存于 checkpoint 的 installed-version.dll。
- 本轮再次尝试 sky 启动游戏仍报 `accessibility window-opened handler did not become ready`；没有游戏进程／窗口，未进行实机操作。转而完成独立生命周期修复，总目标仍进行中。
- 下一步：验证默认隐藏调试 UI 下的真实 native 提交与市场 getter 探针；研究原生箭头跨 world 的注册／销毁／安全回收，解除本轮临时重启门槛后再做真实读档验收；继续 Freight 供给机制／铁路／正式 hover／owner。
- 重读记录：本轮完整重读 01／02／03，重点线程／所有权、性能缓存、生命周期和分层证据；复核 MASTER_PLAN §11、Session／Worker／Engine／Overlay 与现有测试。最小通过标准为修复前反例失败、修复后门槛／取消／新帧发布回归通过，原生实机层继续单列。

## R68 当前增量（2026-10-23，Asia/Hong_Kong 客户端日期）

- 本轮推进当前面板市场识别：同哈希 EXE 的 Ghidra＋Capstone 验证 getter `(panel, out)->out`、panel+0x70 ID 与 wrapper+0x10 ID、引用计数所有权；resolver 会持锁／构造 wrapper，因此不从定时器或 Present 主动调用。
- 新增默认关闭 `probe_panel_market=0`，仅观测原 getter 调用并复制最多 16 组 ID／线程，不改变选择、分配、经济或 owner。实机切换／线程／生命周期 NOT_RUN；此路径不能据静态证据转为正式默认行为。
- **候选** DLL `09b219a9e596f999bd3e2182672b2e551b2fa85e450f266b2297bdf64af27fd0`，29,877,463 B；完整构建／verify_embedded／debug_default PASS。**已部署**仍为 R67 `52009510d0ff6c64af6e03b77ab109c7d9c6d5bafd56ce101116a2b6fb1c7bd9`，本轮未部署或启动游戏。
- 证据 `E:\Code\v3-analysis\panel-market-r68-static.md`、`native/build/build-round68.log`、`debug-default-round68.log`；备份 `E:\Code\v3-analysis\checkpoints\2026-10-23-r68-market-getter`。
- 下一步：可操作游戏中启用探针并打开／切换市场，对照 ID、线程和生命周期，再接正式选择；期间可继续独立的模型／缓存工作。Freight 供给机制问题仍待用户范围答复，总目标保持进行中。
- 本任务重读 01／02／03 全文，重点 UI 回调／ABI、线程与所有权、安全停用；复核 MASTER_PLAN §8–9、NATIVE_GUI_PLAN、PROFILE／源码。最小门槛为可定位的静态契约与关闭态候选检查，不把它写成实机通过。

## R67 当前状态（2026-10-23，Asia/Hong_Kong 客户端日期）

- 本轮完成 Freight 中转商品的**需求核算层**：分配前由原商品净输入需求 × 独立系数（默认 1，项目假设）生成目标，最终分配后计算已满足／未满足，保留原商品贡献；不是独立 Freight 供给求解器。
- 新增原版 `ironroad_demand_v1` 行；目标／满足／未满足与原商品订单、工作量分开显示。无数据／合成输入为 `--`；系数及中转标志纳入缓存身份，不影响原商品线宽。
- 候选／已部署 DLL：`52009510d0ff6c64af6e03b77ab109c7d9c6d5bafd56ce101116a2b6fb1c7bd9`，29,863,251 B。完整构建、11 项原生回归（含新增需求反例）、额外几何身份回归、GUI／原生归档一致性、资源校验、selftest、debug_default PASS。证据 `native/build/*round67.log`。
- 新行实机可见性 **NOT_RUN**：Windows 控制工具两次启动游戏报 `accessibility window-opened handler did not become ready`，随后未发现游戏进程／窗口；未采用进程终止或强制替换。
- A27 保持未整体验收：独立 Freight 供给／生产机制、是否注册游戏内商品尚待确定；已向用户发出异步范围问题，尚无答复。默认观察性核算不授权修改原版经济。
- 下一步：确定 Freight 商品范围与供给机制；验证新 UI 行；继续玩家市场自动识别、铁路、正式 hover／筛选／owner 和真实读档。回滚点 `E:\Code\v3-analysis\checkpoints\2026-10-23-r67-freight-demand`。
- 重读记录：本轮完整读 `01_IMPLEMENTATION_PROMPT.md`、`02_NATIVE_DISCOVERY.md`、`03_TESTING_AND_TROUBLESHOOTING.md`，重点 `Freight overlay behavior`、`Architecture and ownership`、`Language-neutral control flow`、`Performance requirements`、`Known symptoms`；复核模型、UI 方案、原生 profile 与相关源码／测试。最小门槛为需求反例、缓存不串值与新文本桥构建通过；实机层单列。

## R66 最新需求更新（2026-10-23，Asia/Hong_Kong 客户端日期）

- 用户新增硬性要求：以 **Freight 作为中转商品计算物流需求**，已纳入 `MASTER_PLAN.md` §4.5、P4 与 A27；任务 `P4-T05` 为 TODO。
- 当前 `shipmentFreight` 仍是已分配货流的工作量统计，尚未实现 Freight 中转需求链。须明确转换公式、单位、供给来源及需求／分配顺序，保留来源商品贡献、目标／已满足／未满足，并防止递归与重复计数。
- 本轮仅更新需求及交接文档，未改源码／构建／部署；候选 DLL 哈希复核仍为 `200f08d2d738d829afa47097758f14d6250cfcd20c9945cda324d714bd4de134`。
- 文档 UTF-8 无 BOM 与需求条目一致性校验通过。以下 R65 内容作为历史状态保留，其中“当前缺口”段存在早期记录，后续实施应按代码和 R35–R65 证据重新核对。
- 下一步先设计并实现 P4-T05；现有玩家市场识别、铁路状态、正式筛选／hover 与真实读档验收继续保留。

更新时间：2026-10-24 03:05，Asia/Hong_Kong
总状态：进行中（未完成，不得宣称整体交付）
当前阶段：正式 UI 市场对比截图（118 vs 79）；玩家市场自动识别/铁路/hover 仍待完成
本轮目标：R65 — 正式 UI 市场对比截图
本任务重读：`MASTER_PLAN.md` 全文；四份原始文档全文；重点章节 `Architecture and ownership`、`Bring up ImGui`、`Known symptoms`；另读 `FREIGHT_MODEL.md`、`NATIVE_GUI_PLAN.md`、`TEST_PROCEDURE.md`、`AGENTS.md`、`native/PROFILE.md`。
适用约束：变量只用于标签/占位；不显示合成测试值；正式 GUI 保持干净；不改原版文件/存档；未验证 ABI 不调用。
当前实现差异：R1–R64 建立真实数据链路/全州枚举/ribbon 上屏/并行分配/默认采集/生命周期/回滚/内存/验收矩阵/market 作用域/override/市场概览/全新缓存/注册表探针；R65 市场对比截图。铁路/hover/真实读档仍待完成。

## 版本与环境

- 源码版本／未提交修改：无 Git。R65：无源码改动（正式 UI 市场对比截图）。
- 游戏路径：`C:\Program Files (x86)\Steam\steamapps\common\Victoria 3`
- 版本：Victoria 3 1.13.11（`release/1.13.11`）
- EXE SHA-256：`7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f`
- 候选 DLL：`E:\Code\v3-carrow-imgui-starter\dist\version.dll`
  - R65：SHA-256 `200f08d2d738d829afa47097758f14d6250cfcd20c9945cda324d714bd4de134`，29,781,483 B（沿用 R64 构建）；`tools/verify_embedded.py` PASS。
- 已部署 DLL：`C:\Program Files (x86)\Steam\steamapps\common\Victoria 3\binaries\version.dll`，SHA-256 `200f08d2d738d829afa47097758f14d6250cfcd20c9945cda324d714bd4de134`（与候选一致）。
- 市场 GUI：`1f682222b689666731e755bc52b412b3126f4afff40cde05b13a0757b027ec3e`（未修改）。
- 数据：`data/positions.txt`、`heights.bin`、`provinces.bin`、`terrain_names.txt`、`splines.bin`。
- 游戏/图形后端：本轮未启动游戏；历史 D3D11。
- 回滚点：已部署 `a99f…c3e74`；方向验证点 `E:\Code\v3-analysis\checkpoints\20261022-pure-dll-direction-verified`；本轮前记录点 `20261023-master-plan-reading-rules`。

## 已确认（事实 + 证据）

- 原始四文档、MASTER_PLAN 与仓库专题文档已完整读取（R0，后续按主题重读）。
- 基线：`native/build_tests.ps1` 当前 11 项全 PASS（R6 新增 `freight_scale_tests`）。
- 生产 DLL 可由当前源码构建；`verify_embedded.py` 校验资源。
- 单 DLL 加载、嵌入样式/GUI 挂载、市场“物流”第五页签/商品列表/原版详情：历史实机 + 用户确认。
- CArrow 短线、方向、八档宽度、九种颜色：离线 GPU 回归通过。
- R1：热度固定市场尺度；双层缓存（allocation + view/ribbon，32 项 LRU，world/revision/内容指纹门控）。
- R2：`LogisticsSession` + overlay 调试接线；静态确认 `GetState*` 为脚本函数回调，非简单 getter。
- R3：合成测试快照与调试货运提交路径；文档带 `synthetic`，面板 TEST INPUT。
- R4：多 owner 仲裁（10/20/30）；hover 覆盖后关闭恢复基础视图；关闭一个 owner 不清除其他。
- R5：target/allocated/unmet（orders，作用域事实，不受过滤影响）；热度图例显示参考尺度。
- R6：真实地图 634 个城市 hub 中 238 个排除未建铁路后无可用边；hub 回退使 48-region 合成分配 8/96→32/96；全铁路 649 region 分配 1280/1296。
- R6 规模：649 region 无铁路 17.6 ms/641 ribbons；全铁路 105 ms/1472 ribbons；热视图 0 ms、hover ≤0.7 ms。
- R7：合成输入改为每个商品成对生产/消费 + 八档数量；48-region 产生 58 条 ribbon、宽度档 0/2/3/4/5/6/7，freight 16,153,537，compute 4.18 ms。
- R7 规模（配对输入）：无铁路 649 region 248 shipments/655 ribbons/25.8 ms；全铁路 627/1801/271.8 ms，allocated 1,402,070/1,452,378（96.5%）；48 region 全铁路 99.5%。
- R8（实机）：当前候选已部署并启动；日志确认 profile/GUI/嵌入资源/hooks/ImGui ready 与 `synthetic freight test snapshot restored (48 orders)`；调试 UI prototype 页出现 `freight view gen=2 … ribbons=58`、`ironroadTraffic*W*` 多宽度类型解析、持续 `SUBMIT scope=0/1/0`（结束前 153k 提交、71 上传、605 cache hits）；地图上可见多宽度彩色 freight ribbon；文档 TEST INPUT/Freight 16.15M/Target 111.63k/Allocated 38.95k/Unmet 72.68k/热度参考 25000。
- R8：原版市场“物流”页签仍显示 `实时货运数据尚未接通 / 货运量 -- / 已铺铁路 --`（预期，正式数据桥未接）。
- R9：设置规范路径改为 `%LOCALAPPDATA%\IronroadV3\ironroad.ini`；首次运行从 `data\ironroad.ini` 或旧 `binaries\ironroad\ironroad.ini` 迁移且不覆盖已存在的根文件。实机验证：删除根 ini 后将 `data\ironroad.ini` 设 `test_snapshot=1`，启动后根 ini 自动生成、日志读到迁移设置并恢复 48 orders 合成快照；GUI/ImGui 正常，无崩溃。
- R10（静态）：`GetVariableSystem` 是注册的 Jomini 函数，回调返回全局单例；单例是非多态 **string→string 哈希表**（0x48 字节条目：hash/占用/MsvcString 键/MsvcString 值），已定位构造、重哈希与 insert-or-find；未定位 hash 算法、更新已有值的 setter、GUI 方法映射与线程。未在 DLL 中调用。
- R11：新增只读探针（签名门控 + VirtualQuery 校验），实机验证 getter 返回 `base+0x59FD800`，头部 `buckets=1 count=0 level=0 load=0.90`，无崩溃；探针默认关闭、测试后已停用。条目布局/hash 仍未验证（map 为空，需要先由原生 GUI 创建变量）。
- R12：全局同模板 map（5639 条）只读验证 0x48 条目布局；40/40 反推出 **MurmurHash3 x86_32 seed=0**；写入实验成功插入/更新 `ir_probe`、`ironroad_probe_long_key`（堆字符串），读回 `long-value-2`，map `count=1`，无崩溃。原版 GUI 只用 `Exists/Toggle/Set`，变量只能作标志，数值文本仍需数据函数。
- R13：实机打开原版市场面板并确认第五页签“物流”与“显示/隐藏”按钮存在（截图 `r13-market5.png`）；找到 `open_market` = F4 与左侧 sidebar 入口；尝试的变量 `.Get` 文本因未能切到页签内容而未验证（临时 debug 行已移除）。
- R14：`-continuelastsave` 可自动进图；`keybd_event(VK_F4, scancode 0x3E)` 可打开市场面板；`pdx_settings.json` 记录 `GUI.scale=1.1`。第五页签点击仍未命中（(585,232) 落到地图、(532,211) 展开过滤器）；未做 `.Get` 结论。
- R15：**`.Get` 验证成功**——面板标题临时文本显示 `VAR_EXISTS=1 VAR_GET=long-value…`，变量可读文本。尝试 `ir_*` 文本桥时，地图加载期由 Present 线程写入失败（`entry not found after insert`）并导致进程退出；已回退：`native_variables=0`、GUI pending、移除标题调试。渲染线程写变量在地图期不安全。
- R16：隔离确认——`.Get` 读缺失 key 不崩溃；单个后载写入不崩溃但失败（`entry not found after insert`）；批量加载期写入会崩溃。变量表在加载/首次 UI 使用时由 UI 线程占用。需要 UI 线程写入窗口，当前默认全部关闭。
- R17：UI 线程写入窗口已实现（`WM_APP+0x51` → `OnUiThreadPublish`），后载写入不再崩溃；但 entry 哈希匹配、key 垃圾、value 为空，`.Get` 返回空。问题转为**后载变量表的 pair/所有权布局不匹配**，非线程问题。已回退默认关闭，GUI pending。
- R18：**文本桥打通**。根因是 SSO 短字符串在后载 insert/move 路径损坏；改用长 key（>=16）+ 值补齐到 >=16（堆字符串）后，7 个 `ironroad_*_v1` 变量全部正确写入并由探针读回；标题 `.Get` 显示 `STATUS=waiting for economy data FREIGHT=--`。正式页签已改用 `.Get` 绑定，`native_variables` 默认开启；页签内容可视化仍待点击/布局验证。
- R19：页签内容容器修复——`ironroad_market_logistics_content` 必须插入 `market_panel_details_content` **块内**。临时用 `Or(default, ironroad_logistics)` 实机显示“物流”、`waiting for economy data`、`Freight: --`、`Track: --` 与显示/隐藏按钮，`.Get` 文本可见；生产可见性恢复为仅第五页签。第五页签点击自动化仍未命中。
- R20：尝试在标题 `raw_text` 后注入 `onclick=SelectTab('ironroad_logistics')`；实机点击标题（~420,165）后选中的是 **成员** 页签（国家旗列表），不是物流。实验失败，已回退；正式构建保持 R19 行为。
- R21：**物流页签内容最终确认**。直接用类型实例 `visible` 不生效；改为 container 包裹并设 `visible=IsTabSelected('ironroad_logistics')`，默认页签隐藏、物流页签显示。悬停提示 **Tab 5 / SHIFT+5**，发送 Shift+5 后“物流”高亮，内容显示 `waiting for economy data`、`-- / -- / --`、`Freight: --`、`Track: --`、按钮与货物列表。
- R22：物流页签隐藏商品专用 FILTERS 按钮、搜索栏与过滤器容器（`Not(IsTabSelected('ironroad_logistics'))`）；F4+Shift+5 实机显示干净内容（`r22-clean.png`）。Target/按钮行有轻微重叠，待布局微调。
- R23：重写页签顶部布局，状态/Freight/Target 标签/值行/按钮/Built rail 各自独立行，显式 size；实机无重叠（`r23-layout.png`），内容为 waiting/`--`、显示/隐藏、货物列表。
- R24：重开生命周期通过（Esc→F4→Shift+5，内容/变量/按钮/货物正常，无崩溃，`r24-reopen.png`）；点击商品行打开原版商品面板（钢，`r24-show.png`）。显示/隐藏按钮的自动化点击仍受坐标影响，非功能缺陷。
- R25：新增调试 `test_hide_variable`，UI 线程写入一次 `ironroad_hide_flows`。实机对比：无变量时 `显示` disabled/`隐藏` enabled；写入后 `显示` enabled/`隐藏` disabled（`r25-baseline-row.png` / `r25-hideflows-row.png`），确认 `Exists/Toggle` 绑定正确。测试后恢复 0。
- R26：UI 线程每 500ms 轮询 `ironroad_hide_flows`，日志 `native panel hide flows=1`；`ApplyPendingFreight` 在隐藏时停用/隐藏调试 ribbon（纯视图）。未做 prototype 视图下的肉眼 ribbon 消失验证。
- R27：新增 `probe_economy` 钩子（`0x019791F0` GetStateProduction 回调）；实机打开州面板“本地价格”页签时按商品逐行触发，日志捕获 `ctx/result/args` 多组。确认了经济脚本回调的调用时机与参数形状；richer ctx 字段日志已构建但未抓到样本（鼠标坐标缩放导致州面板复现不稳）。
- R28：通过商品面板“本地价格”稳定触发；捕获 ctx 6 qword（`[3]=0xffffffff_<stateId>`）、args（count=7、元素 0x20、begin=data+0x20）、result 容器（vtable/代码指针/子对象）结构。州 id 已可读；商品 id 与定点数值仍待解码。
- R29：钩生产读取器（0x195BA00）与容器读取器（0x103F3C0 keyed / 0x103F440 indexed）。实机商品“本地价格”走 indexed 路径（同一 out、8 个不同 arg、步长 0x70）；out 为条目向量，数值条目布局待解析。修正读取器 detour 返回类型为 void* 以保留原返回值。
- R30：Ghidra headless 反编译容器读取器：indexed 值 = `array[goodId]`（array@container+8，位图@+0x20，goodId 来自 arg+0x10）；keyed 用 arg+0xC0 的键与 +0x30/步长 0x20 的表；读取器容器 = state+0xC8（生产）/+0x78（消费），state 由 registry(ctx+0x18) 解析。已加入直接读取实验；实机数值样本待补。
- R31：实机数值读取成功：state 432 prod=1.2M、state 436 prod=0.4M/cons=20.85M；100 州累计 prod=3,424,477,556、cons=5,942,248,942；对 UI 市场总量 36.6K/61.2K 得比例 ≈9.4e4/9.7e4 → **固定点缩放 1e5**。经济数据源与单位已验证，下一步枚举州/商品生成快照。
- R32：州对象 id 位于 state+0x8；registry 盲枚举（1117 个“州”）总量达 4.5e18，无效；reader 路径给出的州有效（如 state 201 prod 11 项/8,264,509、cons 38 项/11,239,733）。结论：快照应从 reader 观察到的市场州集合构建。
- R33：采集器落盘成功：100 州全商品，过滤后 `totalProd=24,007,540,245`、`totalCons=36,843,839,954`（≈240,075/368,438 单位，1e5）；文件 `%LOCALAPPDATA%\IronroadV3\economy_dump.txt`。真实市场快照数据已就绪，下一步映射 EconomySnapshot。
- R34：真实快照映射并接入会话成功：`accepted orders=3082 goods=49 market=1`；会话 `owner=10 gen=1 frame=1 ribbons=116`，**真实数据生成 116 条模型分配 ribbon**。正式页签文本本轮仍显示 waiting（发布/时序问题待查）。
- R35：根因是 insert-or-find 不更新已存在 key；改为 insert 后在 entry+0x28 通过引擎 `assign` 更新 value。实机正式页签显示真实模型分配：`Freight (orders): 23108927.60`、`Target / allocated / unmet: 259398.15 / 14746.49 / 244651.66`，无崩溃。真实数据链路首次完整可见。
- R36：修正 Freight 口径：`allFreight` 是 milli-orders×cost，显示改为 `/kQuantityScale` → `Freight (order-cost): 23108.93`；分配率 14,746/259,398 ≈ 5.7%，指向 state→region/铁路映射问题。
- R37：确认运行时州 id **等于** positions region id（201/205/432/436/132/350 均命中；790/940 缺失 → 无 hub）；州对象未直接含 `STATE_` key（不需要）。**关键**：本地价格页签列出的州横跨多国，说明采集到的是全球州而非单一市场；快照把全部州标为 market=1 造成跨市场混样，是低分配率的主要嫌疑。下一步定位州的市场字段并按市场分组。
- R38：扩展转储 0x200 字节；实际只见到 6 个州（205/201/350/132/432/436），英国本土 16–23 未被访问 → 本地价格页签是**全球排序子集**而非市场。州对象 0x00–0x100 未见明确 market 字段（0x80/0xa8/0xd0 为每州数据指针，0xb0 为类别计数）。需同市场样本才能定位。
- R39：一次性全州枚举成功：**1117 州**、`totalProd=88.9e9`、`totalCons=94.2e9`（≈889,366/942,319 单位）；正式页签 `Freight 126318.85`、`Target 519674.48 / Allocated 93784.10 / Unmet 425890.38`，分配率 18.0%。数据覆盖不再受页签首屏限制；market 混样与铁路仍待解。
- R40：market 字段追踪负结果：英国本土 16/17/18 与外国 132/432 的 state 0x200 字节、以及 0x80/0xa8/0xd0 目标对象中，**没有**“英国相同、外国不同”的字段（qword/dword 均无）。market 很可能是通过所有者国家/省份间接获得；需先验证州身份或 hook 市场面板的 market 对象。
- R41：`ApplyPendingFreight` 不再要求 ImGui 可见，原生 owner 活跃即提交；真实帧 **989 条 ribbon** 上屏（`compute=768.119 ms`，后台 worker），`native pool 989/8192`、`visible yes 999`；地图可见细线（宽度档低，全局摊薄）。
- R42：分段剖析 `allocation=741.30 ms / view=10.73 ms`，分配内 `flow=729.03 / decomp=11.16 ms`；importer O(1) 查表优化落地，分解降至 11 ms。瓶颈为 min-cost flow 增广。
- R43：按组并行求解：墙钟 `allocation=126.94 ms / total=133.38 ms`（8 线程），线程 CPU 累计 `flow=893 ms`；ribbons=989；cancelled 语义经 exception_ptr 保留。相比 741 ms 约 **5.8×**。
- R44：默认配置（`probe_economy=0`）下 `real economy collected states=1117 orders=28554 goods=49`，会话 owner=10、989 ribbons；物流页签显示 `Freight 126318.85`、`Target 519674.48 / Allocated 93784.10 / Unmet 425890.38`。真实数据不再依赖任何 probe 开关。
- R45：世界变化失效：anchor 州指针/vtable 每 ~2 s 复核；20 s 实机无 `world changed` 误报，数据保持。存档重载/主菜单往返的端到端演练未执行（待补）。
- R46：30 秒周期刷新：实机 rev=1→2、gen=1→2，989 ribbons，compute 141.7/144.9 ms。重载演练的自动化点击命中“切换国家”界面（未选国家/未保存/未改档），演练待补。
- R47：回滚部署演练通过：卸载 DLL 后游戏正常进入地图；重新部署后 `collected 1117/28554/49`、989 ribbons、144.2 ms，功能恢复。
- R48：内存平台：3 分钟 18 次采样 warm 区间 7278–7302 MB（±24 MB，约 0.3%），后段下降、无泄漏趋势；周期刷新 rev 5→6、989 ribbons、~139 ms。WorkingSet 含游戏基线，mod 增量未单独隔离。
- R49：失效路径安全演练：`TEST world-change` → 失效 → 重新采集 → `gen=3 rev=2`、989 ribbons、139 ms；真实重载自动化打开国家预览界面，未继续（未改档）。
- R50：Python GUI 补丁镜像与 C++ 对齐；`test_native_gui_runtime.ps1` 恢复 PASS（exact patch agreement）；全量工具回归（native_gui/flow_gpu/selftest/debug_default/verify/build_tests）全部通过。
- R51：容器头探针负结果（16/17/18 与 132/432 无共享 market 指针/ID）；`ACCEPTANCE.md` 更新为 R51 状态（PASS/PARTIAL 明确列出）。
- R52：正式页签标注 `model allocation (global)`（实机可见）；州面板点击威尔士/苏格兰高地名称与 positions 一致，但 runtime id 未由 reader 证实。
- R53：**market 作用域突破**：`state+0x48` = market id（1117 州仅 59 个值；威尔士/高地 79、莫斯科 71）；快照按市场分组，`markets=59 primary=118`（77 州）；页签 `model allocation (market 118)`、`Target/allocated/unmet 16440.38 / 1862.16 / 14578.22`，37 ribbons、136 ms。
- R54：`market_override=79`：`primary=79 states=30`、47 ribbons；页签 `model allocation (market 79)`、`7465.20 / 451.99 / 7013.21`；默认仍为最大市场。
- R55：10 次 F4/ESC 开关循环存活、无 `world changed`；周期刷新 rev 1→2、37 ribbons；重开后页签正常（`r55-cycles.png`）。
- R56：脚本注册探针安装成功但**未观察到 market 注册**（注册早于钩子或非该路径）；玩家市场识别仍未完成。
- R57：市场归属核对：17/19=**79（UK）**、132=71、350=83、201=64、205=33、432/436=164；默认 primary=118 不是 UK，`market_override=79` 可得到 UK 市场。
- R58：README 用户文档刷新（正式页签、语义、`market_override` 用法、调试面板区分）；UTF-8 有效。
- R59：采集日志前三市场 `top=118:77 164:75 35:74`；`SaveSettings` 补 `probe_market`/`market_override`（保存不再丢失）。
- R60：页签自动市场显示 `model allocation (market 118, auto)`（r60-auto.png）；37 ribbons、138.8 ms。
- R61：`COMPLETION_AUDIT.md` 重写为 R61 状态；`IMPLEMENTATION_REPORT.md` 顶部新增 Current status（含当前哈希与缺口）；UTF-8 有效。
- R62：页签新增 `Markets (id:states): 118:77 164:75 35:74 33:68 34:58`（`ironroad_markets_v1`）；37 ribbons、136.2 ms。
- R63：全新缓存重装：删除 `%LOCALAPPDATA%\IronroadV3` → 卸载/重装 → 启动自动重建缓存（data/assets/native-gui/ini/log）→ 页签数据正常（r63-fresh.png）。
- R64：脚本注册表探针：固定全局 `0x1459FD648` 有 141 条 0x78 字节条目，但名字偏移不符，未提取出条目名/market 函数（负结果）。
- R65：正式 UI 对比：默认 `(market 118, auto)` 16440.38/1862.16/14578.22、37 ribbons；`market_override=79` 显示 `(market 79)` 7465.20/451.99/7013.21、47 ribbons（r65-uk-ribbons.png）。

## 当前缺口

1. **Freight 中转机制**：R67 需求核算和文本桥完成；独立供给／生产／价格机制未实现，游戏内商品范围待用户答复；A27 未整体通过。
2. **市场上下文**：已有按市场分组与 override；默认最大市场不等于玩家／当前面板市场，自动识别仍缺。
3. **铁路**：真实 `builtRailEdges` 未接入，原版长度仍为 `--`；hub 回退需按真实铁路复核。
4. **正式动作／owner**：已有原版文本桥及显示隐藏标志；正式 hover／筛选、关闭时释放 owner 尚未接完。
5. **实机验收**：新需求行、完整货运宽度／颜色／长路径／相机／接缝、真实读档／菜单／resize 未全量验证；内存平台与 worker 时间有历史证据，受控帧时间＋模拟进度对照尚缺。
6. **回滚／生命周期**：R47 有实际卸载重装，R49 仅失效注入；本轮新版本的实机回滚和真实读档不能据此推定通过。

## 最近验证

- `native/build_tests.ps1`：PASS（exit 0，11 项）。日志 `native/build/tests-round65.log`。
- `python -B tools/verify_embedded.py`：PASS（`200f08d2…`）。
- **实机（R65）**：auto 118 vs override 79 的页签数值对比（7465.20/451.99/7013.21、47 ribbons）；当前缩放下地图 ribbon 未清晰可见。
- 未执行：玩家市场自动识别、铁路、正式 hover、真实读档往返。
- 证据：`native/build/r65-ribbons.png`、`r65-uk-ribbons.png`、`tests-round65.log`。

## 下一步

1. 玩家市场自动识别（市场面板对象/GUI 上下文/首都州）。
2. 铁路状态接入；正式 hover/filter owner。
3. 真实读档往返演练；清理 debug 转储。

1. market 作用域与铁路接入。
2. 完整验收矩阵与性能回归；清理 debug 转储。
3. 真实重载的手动/更可靠自动化方式（可选）。

## 阻塞与用户动作

- 实机验收需要游戏可操作；无人值守时保持 NOT_RUN。
- 若需用户协助：部署候选后启动游戏、打开调试面板 UI prototype、截图/描述箭头与数值，并回传 `%LOCALAPPDATA%\IronroadV3\ironroad.log`。
