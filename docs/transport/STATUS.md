# 当前状态

### 2026-11-06 — R204：默认关闭ImGui的官方等价harness复验

- 在无游戏进程、清除`IRONROAD_DEBUG_UI`／`IRONROAD_READBACK`／`IRONROAD_FRAME_TIMING`后运行`native/build/tests/harness.exe dist/version.dll`（等价`tools/test_debug_default.ps1`）：exit=0，`loaded dist/version.dll`、导出对等、`ImGui ready (960x960)`、`present #1 hr=0x00000000`、`present #2…#6 imgui=1 vtx=0`、`drawdata lists=0 vtx=0`，无`FRAME PERF`；loader.log`loaded freight.dll (error=0)`。独立harness中`native profile not ready; arrow hooks disabled`属预期（进程非victoria3）。证明默认关闭调试面板、无帧计时采集。
- 边界：PowerShell脚本本身仍因沙箱未执行；此为同参数同二进制的手工等价复验。证据：checkpoint2026-11-06-r201-lifecycle-graphics（default-visibility-harness.log、default-visibility-loader.log）。

### 2026-11-06 — R203：1.15市场物流页hover作用域实机

- 市场物流页打开后`freight view gen=2 world=1 rev=4 ribbons=185`（owner10）。悬停首个商品行（谷物）→`freight view gen=3 ribbons=34 compute=0.732 ms`、`native session gen=3 owner=20 ribbons=34`、`DRAW PERF paths=34 uploads=33 reused=1 visible=34 dropped=0`；市场卡片总量13982.47687／510.99MU不变。移出→`freight view gen=4 ribbons=185 compute=0.008 ms`、`owner=10 ribbons=185`（缓存命中）。证明1.15下hover只改地图贡献并在移出后恢复，不改数量。
- 边界：本轮只覆盖市场页单商品hover；分类／显示隐藏／搜索（键盘输入不可用）／切州切市场／双语未在1.15重做，完整A18–A19矩阵仍PARTIAL。证据：checkpoint2026-11-06-r201-lifecycle-graphics（hover-grain.png、hover-restored.png、hover-log.txt）。

### 2026-11-06 — R202：1.15部署矩阵与改图身份（州定义geo 164→901实机绑定）

- 部署矩阵（同一`64913eea…`／`c8020412…`构建，游戏关闭时逐例准备，每例启动后保持运行≥50秒）：主体缺失→loader.log最后一行`LoadLibraryExW failed (error=126)`、无payload日志、游戏进程存活；主体损坏（256字节零）→`error=193`、游戏存活；整套卸载（version.dll＋freight.dll移开）→无新loader条目、游戏以原版界面存活；重新安装→`loaded freight.dll (error=0)`且`loader attached`/渲染pass日志出现。全部用例后dist＝安装逐文件哈希相同，EXE`473186e8…`、存档`458e7780…`、content_load`f8c89f83…`字节保持。
- 改图身份：1.15目录Mod覆盖`map_data/state_regions/02_east_europe.txt`，仅将`STATE_BEKES`的`id`由164改为901（与base文件相比仅3字节不同，保留BOM/其余内容）；content_load以绝对路径启用，测试后恢复原字节（`f8c89f83…`）并删除Mod目录。实机pid11200：`state=1025 key=STATE_BEKES definition=150 geo=901 valid=1`——运行州key与定义索引不变、地图geo身份按Mod变为901（无Mod时为164）；快照`states=1157 goods=54`正常，市场物流页`native market owner selected panel=0 tab=ironroad_logistics`、`freight view gen=2 world=1 rev=4 ribbons=185 compute=95.082 ms`、`DRAW PERF paths=185 uploads=185 reused=0 visible=185 dropped=0`，页面显示13982.47687／510.99MU。证明正式模型／视图按改图后的geo身份重建，未复用改前缓存。
- 同key样条几何：1.15目录Mod覆盖`gfx/map/spline_network/spline_network.splnet`，仅将控制点`268437460`的X由4072.966306→4082.966306（+10，文件其余字节相同）。实机pid15148：运行时样条网络`points=33765 strips=4281 connections=4279 width=8192 height=3616`（计数与base一致），`point=268437460 x=4082.966309 z=2789.185547`、`expected x=4082.966309 delta=0.000000`——同一key/点ID下几何按Mod更新；快照states=1157 goods=54，市场物流页`owner selected`、`freight view gen=2 ribbons=185 compute=103.525 ms`、`DRAW PERF uploads=185 visible=185 dropped=0`。content_load恢复原字节、Mod目录删除。
- 边界：本轮改了一个州的地图id与一个样条控制点；省份归属交换、ZIP／replace_path、逐strip重排及运行中改图缓存失效仍未在1.15验证；部署矩阵的“目录不可写”及真实设备重建未做；最终同构建A01–A27仍未完成。证据：checkpoint2026-11-06-r201-lifecycle-graphics（r201_deploy_matrix.json、deploy-matrix.log、r201_mod_test.json、mod-test.log、mod-panel.png、mod-fixture/、r201_spline_test.json、mod-test-spline.log、spline-fixture/）。

### 2026-11-06 — R201：稳定视图缓存门槛与1.15重载池恢复修复（缩放往返通过）

- 缓存／压力：市场物流页保持打开65秒，`SUBMIT … uploads=346`持续不变、无新`DRAW PERF`批次，即稳定视图零新增几何上传。随后20轮关闭→重开市场→物流页，逐轮`DRAW PERF paths=185 uploads=0 reused=185 failed=0 pool=346 … repeats_skipped=1…20 state=complete gen=6…44`：20轮零上传、池恒346（≤8192）、每帧上传0（≤16）、`peak_attempts=16`、`peak_frame_us=685`（软2 ms预算内）、`overrun_frames=0`。内存采样20轮：RSS 8,155,156,480–8,156,925,952B（Δ1.9MB），private 15,801,597,952–15,888,187,392B，末6轮在15.86–15.89GB间波动无单调增长，末4轮完全相同→平台。
- 生命周期实测发现真实1.15缺陷：`overlay.cpp`仍从1.13地址`0x0592d5c8`读图形身份，1.15该地址实读为无效值`0x404048404c302440`；而`arrow_lease.h`校验器按1.15读`0x05c080b8`（实机`0x57a50a50000`）。首轮“菜单→载入autosave_exit”重载后，池恢复被误判`native graphics identity changed`，物流页显示`IRONROAD_MAP_RETIRED`且无箭头（世界2模型帧185条仍生成）。修三处读取为`0x05c080b8`后重建`64913eeaadccdbc4dff3ceee484e76b6710f0fe171f68e9ae9e6d46cf70f27d5`／49,737,579B（引导仍`c8020412…`），官方等价原生入口再次35/35、零FAIL；游戏关闭时原子部署，安装＝dist。
- 修复后实机重载：世界1→菜单→载入world2，`native world retirement event next world=2`、快照world2 rev5/rev7、UI handoff accepted；打开物流页时`native pool recovered world=2 slots=185; old requests cleared`、`freight view gen=7 world=2 rev=7 ribbons=185 compute=91.031 ms`、`DRAW PERF paths=185 uploads=0 reused=185 failed=0 pool=185 visible=185 dropped=0`，页面显示13982.47687／510.99MU且无未就绪横幅、箭头可见。无需重启游戏即恢复正式视图。
- 缩放：视图开启状态下客户区2560×1440→1920×1080→2560×1440，无池退休、无旧帧残留，数值与185条箭头保持（截图resize-1920x1080／resize-restored）。最小化→恢复由本轮启动/恢复重复覆盖。
- 三对性能协议（同存档／相机／速度III／2560×1440，`IRONROAD_FRAME_TIMING=1`；每臂重载同存档→可选物流页→暂停热身30秒→运行65秒）：关闭→打开P95为17.712→17.992、17.642→18.049、17.659→18.064 ms，增加0.281／0.407／0.405 ms均≤2；模拟速度11.803→11.731、11.779→11.797、11.821→11.767 date单位/秒，最大降幅0.61%≤5%；窗口61.5–62.6秒，帧流连续且thread/world/swap身份一致。三对门槛在本存档／相机／速度III通过，不外推其他速度／设备。
- 边界：设备重建未做；三处海路末端／海峡权限／改图及最终同构建A01–A27仍未完成。证据：checkpoint2026-11-06-r201-lifecycle-graphics（PERF-ARMS.md、perf-analysis.log、r201_perf_arms.json、before overlay.cpp、source-change.diff、cycles.json、draw-perf-cycles.log、build/tests日志、重载与缩放截图）。

### 2026-11-06 — R200：1.15港口接入解码器迁移，阶段1官方等价入口35/35

- 上一轮唯一失败的`native_port_approach_tests`是R193诊断解码器的1.13夹具。本轮把`native_port_approach.h`整体迁移到1.15：州地区`state+0xF10`（vtable`0x046ba948`，id+0x10／geo+0x40／valid+0x44）、市场经`DecodeNativeStateMarket`、world链`0x05c07c68→+0x608 machine→+0x120 world`、manager=`world+0x5028`（vtable`0x046b1d48`）、省份运行表`machine+0xB0`（vtable`0x046ac1c0`，州int32+0x18）、省份模板注册表`0x5bd7308`（数据+0x2A0／计数+0x2AC，vtable`0x046ab568`，地区+0xC8）、海军连接表`manager+0x58`（行0x40，定义kind+4=5，样条key+0x20类型3，节点定义flags+4／anchor+8／province+0xC／linked+0xE）。陆侧节点由linked省份解析到运行州判定，海侧物理省份取另一节点province；只有物理海区与所选州地区不同才产生查询。重复id、两侧州歧义、错误kind／样条类型、依赖不可读、复制中变化与退休均拒绝。
- 新1.15夹具替换旧1.13夹具：类型化州槽位、owner→country→market、省份运行表、模板注册表、一条type5海军接入；逐项断言返回的州／市场／两地区／两锚点／逻辑与物理省份／样条，并覆盖重复id、非港口kind、非type3样条、缺海区、缺州地区、退休及源销毁后自有值保持。
- 官方等价原生入口从仓库根运行35/35 PASS、零FAIL（上一轮34/35）；本地16套保持通过。`verify_embedded`、`test_native_gui`（4项）、`test_flow_gpu`（含负对照）、`test_freight_loader`（有效／缺失／损坏两类引导）全部PASS。PowerShell脚本入口（`build_tests.ps1`、`selftest.ps1`、`test_native_gui_runtime.ps1`、`test_debug_default.ps1`、`deploy.ps1`）受沙箱限制未运行，部署仍按既有`.tmp`+rename原子替换（游戏关闭时），环境允许时应补跑。
- 新构建主体`86cdaadadbb88aa5a0d11bea060b4c7f14a9c457f465dea70fe5c2b135cfaa7b`／49,737,579B，引导`c80204122791ade4a33f5b61ea28ebb3a96f444bbc6ef215e71a6245af67a107`／779,879B；部署后逐文件哈希安装＝dist，EXE仍`473186e8…`。最小化实机pid39776：loader识别1.15.0、三组hook安装、快照world1 rev2/3/4 states=1157 goods=54。
- 本构建实机双页面复验：州面板东盎格利亚本地价格页→`native state owner selected state=434 tab=local_goods`、`native session gen=2 frame=1 owner=15 ribbons=161`、`publish document=1`、`freight view gen=2 ribbons=161`、`DRAW PERF visible=161 dropped=0 failed=0 overrun_frames=0 peak_frame_us=685`，物流列显示0.00／173.175／5.00／10.72399／27.534／273.58／138.28等真实值；市场物流页→`native market owner selected panel=0 tab=ironroad_logistics`、`native session gen=4 frame=1 owner=10 ribbons=185`、`freight view gen=4 ribbons=185 compute=1.146 ms`、`DRAW PERF visible=185 dropped=0`，页面显示贸易量13982.47687、已铺铁路510.99 MU与逐商品数值。日期保持1836-01-13暂停，未存档。
- 边界：港口接入解码器仍是默认关闭的`probe_access`诊断，未接正式worker；三处海路末端／海峡权限／改图字段仍需在1.15独立验证；性能3×60秒、上传、生命周期及最终同构建A01–A27矩阵未做。证据：checkpoint2026-11-06-r200-port-approach-1.15（build／tests-full日志、双页面截图、live-session日志、源码diff、NEXT.md）。

### 2026-11-06 — R199：1.15港口—州接缝修复（lane纯海段＋港口合成路径）

- 用户报告本地价格路线图在“海外→伦敦周边→中英格兰”处再次中断。只读复现（同一运行游戏）：水果最终货流Bahamas→中英格兰、Jamaica→中英格兰；相邻边端点坐标不连续5处。海段lane几何终点是港口陆侧点`0x8005dc`(4072.5,2779.9)，模型图端点却被掩码归一化到州主枢纽`0x5dc`(4091.0,2801.5)；同时lane几何已包含港口样条，模型又单独加港口边，渲染出现回折/断口。
- 根因：1.15 port access样条（type3）连接海锚与港口陆侧节点，而该陆侧节点没有任何陆路连接；1.13的keyless port path曾把海锚直接连到主枢纽。迁移时图端点做了掩码归一化但几何未同步，且lane几何重复包含港口样条。
- 修复：`native_trade_geometry.h`新增seaOnly与startAnchor/endAnchor：模型lane用纯海段几何，图端点取几何实际链端点；`native_domestic_model.h`港口边改为合成bit63路径＝原版港口样条控制点＋（掩码匹配时）主枢纽见证点连接，路径/图端点/长度一致；无port transfer的旧式数据回退原选择器；同海区零距离lane用两条港口样条成链。
- 实数据复核：水果货流全部相邻边坐标连续（breaks=0）。Bahamas→中英格兰：海段终点(4053.762,2759.913)→港口合成路径到枢纽(4091.022,2801.467)→陆路→…；Jamaica侧港口边同样连续。该连接点是模型假设（港口点到州枢纽的直线连接），两端均为当前见证点。
- 构建exit0，本地16套测试全PASS。官方等价入口（35项，从仓库根运行）现仅剩1项RUNFAIL：native_port_approach_tests（R193诊断解码器仍为1.13布局，需迁移1.15旅行网络）。native_trade_view_tests已全通过：修正了seaOnly完整度短路误用于legacy选择器的问题（单端口缺链现正确归partial），arrow_lease夹具已按1.15 RVA更新；harness等原为运行目录相对路径问题。阶段1只差native_port_approach迁移。
- 主体`7d31584a31302f17b179a4dc4afd259458edef5f2cc0ffdd1d1fde4e3ec94619`／49,735,019B＋引导`fa9ebd38…`（随主体哈希重建）已部署，安装＝dist；游戏实载1.15.0并出帧：市场owner10 ribbons185、伦敦周边state5 owner15 ribbons232（DRAW PERF overrun0）。本轮UI相机导航成本高，像素级港口接缝复看改由数据级坐标连续证据覆盖；不做像素验收声明。随后partial完整度修复重建为`3c931a306a406a7321cf385a4e220a4bee173f672066175efe9227d9c09d050f`／49,736,555B并再次部署（安装＝dist，引导`fa9ebd38…`不变），源码＝构建＝安装。

### 2026-11-06 — R198：州面板1.15迁移与双页面实机验收（两处`--`消除）

- 用户报告本地价格页物流列也显示`--`。定位：`state_panel.h`仍是1.13值（handler+0xD8/+0xE0/+0xEC、states_panel vtable 0x045CC848），1.15下解码失败、州owner从不激活。1.15实读：states_panel仍在slot14，vtable=`0x0485DB38`，tab=`local_goods`，state=433，bar history kind=14；已迁移解码器，并同步迁移`market_panel_tests`/`state_panel_tests`夹具（handler+0xE8/+0xF0/+0xFC、state vtable），两测试加入本地统一套件后16套全PASS。
- 重建主体`d36205ed0a2a372e73335d12c1a9b2147fbbef5e614c3f332d7f1b142fc34c57`／49,618,850B。用户授权自主操作后正常关闭游戏（WM_CLOSE）→原子替换部署（安装＝dist）→重新启动，自动操作UI完成双页面验收。
- 州页：打开本地价格页→`native state owner selected state=433 tab=local_goods`，session frame=1 owner=15，物流列显示0.00／45.422／5.64／17.85593等数值（截图`r198-state.png`）；关闭州面板后打开市场物流页→`native state owner closed`、`native market owner selected panel=0 tab=ironroad_logistics`、`native session gen=71 frame=1 owner=10 ribbons=195`、`DRAW PERF visible=195 dropped=0`，市场页显示贸易量14662.19344／已铺铁路510.99 MU与地图箭头（截图`r198-market.png`）。两处`--`均消除。
- 面板栈注意：州面板与市场面板同时开时，州owner(15)在我方同一轮询中晚于市场owner(10)设置，会覆盖地图视图；属既有owner优先级行为，不是1.15回归。纯市场验收需先关州面板。
- 副作用披露：自动化聚焦使游戏从暂停自动恢复，在下达暂停键前以速度6推进约3个月游戏时间（1836-01-13→1836-04-08），随后已暂停并最小化；未主动存档。用户会话经济状态可能已变化，需用户确认是否读档。
- 仍未做：开态商品行逐项值对照、完整GUI手动矩阵、性能门槛；三处海路末端、权限、改图、生命周期等原有未完成项不变。

### 2026-11-06 — R197：1.15贸易表语义修正与无主州处理（离线实数据验证，待部署）

- 用户报告物流显示`--`。实机日志显示：面板打开时worker报`err=native detailed and grouped exports differ`（gen21/22），随后快照失败`native state market unavailable`。定位为两处1.15语义差异，而非渲染问题。
- 贸易表：1.15 detailed(0x898)与grouped(0x870)键集合完全相同（4350对全部一致），但数值逐项不同（same=0／differ=5729）；grouped在目的组内近似常量（3451/3478组单一值）、detailed按州（780组混合值），说明grouped不再是与detailed等值的聚合表。`BindNativeTradeRoutes`不再用detailed==grouped作等值校验，改为结构校验＋逐州detailed绑定＋组代表路由；注释记录该实机证据。
- 无主州：用户局中`STATE_TEXAS`（运行id93）owner=`UINT32_MAX`且无特殊市场区，`DecodeNativeStateMarket`无结果；`DecodeNativeTradeStates`现仅在该明确情形跳过该州（不伪造市场），其他市场失败仍整份拒绝。新增native_economy_tests用例：跳过无主州、特殊区不可读仍拒绝、无效owner仍拒绝。
- 离线实数据验证（同一运行游戏，PROCESS_VM_READ）：roster 1157／skip1；trade 67组／9海峡／4350对；bindings 4350（withGroup4350／withRoute4348）；facts shapes1550／shapedBindings4348；domestic 27108订单／419陆路／810州内／32铁路／371港口接入／411枢纽／96航线；model shipments=3759（海1535／陆2224）、builtRail=719.251863 MU、target／allocated／unmet=144803.446／34307.144／110496.302。不是DLL worker帧或绘图验收。
- 构建exit0，14套原生测试全PASS（含更新后的native_trade_routes_tests与native_economy_tests）。新主体`3ddf32cd03ae366a3c0701b8305fb34665bb59a25388cb4f94545aa7f8d95233`／49,618,850B，引导`c8020412…`／779,879B。用户保存退出后，在无游戏进程状态下原子替换部署，安装＝dist；以`-continuelastsave`重新最小化启动，loader识别1.15.0、hooks安装。
- **实机开页验收通过**（同一新构建pid35212）：快照复制states=1157（无主州已跳过）／goods=54；用户打开市场面板物流页后`native market owner selected panel=0 tab=ironroad_logistics`，session gen=2 ready=1 err=，`publish document=1`，`freight view gen=2 world=1 rev=4 ribbons=196 compute=98.813 ms synthetic=0`，`SUBMIT … submitted=14 uploads=14`，`DRAW PERF paths=196 … visible=196 dropped=0 native_slots=196 peak_frame_us=2255 overrun_frames=1`，session tick owner=10 frame=1 ribbons=196 err=；随后用户关闭面板（owner closed）。这证明1.15下开态面板解码（market／tab／visibleGoods／history kind=9）、无主州跳过后的快照、贸易绑定、国内模型、worker出帧与196条ribbon提交均实际工作，`--`不再出现。
- 边界：本批次`peak_frame_us=2255`超过2ms软预算1帧，compute=98.8ms为worker计算时间，不构成性能验收；未做开态商品行逐项值核对（owner激活已要求visibleGoods可用）与完整GUI手动矩阵；三处海路末端、权限、改图、生命周期等原有未完成项不变。

### 2026-11-05 — R196：1.15数据管线全链实机通过；正式面板唯余开页验收

- 本轮把1.15迁移接到正式生产路径并实机验证。UI交接失效的直接原因：`market_panel.h`仍用1.13 handler偏移（+0xD8／+0xE0／+0xEC）；1.15实读handler+0xE8=self、panels+0xF0、count+0xFC=56，改为这三个偏移后解码通过。构建exit0，14组原生测试套件全部PASS（native_domestic／shipping／splines／sea_geography／world_trade／economy／state_market／goods_catalog／snapshot／ports／rail／trade_routes／trade_geometry／access）。
- 候选主体freight.dll SHA256 e612657d4b691cd4a6a96605450508c953194d28825950a7afb1431ee0b5950b／49,713,595B，引导version.dll c80204122791ade4a33f5b61ea28ebb3a96f444bbc6ef215e71a6245af67a107／779,879B。沙箱禁止PowerShell，本轮在游戏关闭状态下按旧轮次同规则用.tmp+rename原子替换部署（安装＝dist，逐文件哈希相同）；未走deploy.ps1，环境允许时应补跑确认。EXE SHA256 473186e8…／99,988,600B未变。
- 最小化实机pid 46532：loader记录native profile ready: Victoria 3 1.15.0与三组原生hook安装；native snapshot copied thread=27756 world=1 revision=2/3/4 states=1157 goods=54（328–359ms）；native snapshot UI handoff revision=4 states=1157 orders=0；real economy collected markets=305 goods=54 primary=0；real economy snapshot accepted。该会话段WARN／ERROR／failed为0。本轮游戏启动即最小化，Present未发生、ImGui定时器未初始化，UI交接直到一次离屏Present后才发生；窗口以离屏方式恢复取得首帧后立即最小化，现保持最小化且Rect在屏幕外。
- 正式worker仅在原版市场／州面板持有视图时请求构建（`LogisticsSession::Poll`仅当m_effectiveOwner且脏时Request）。本会话未打开面板，故session tick ready=1 frame=0 err=、无模型帧；这不是失败，是既有正式界面契约。原版商品筛选行只在面板打开后由引擎创建，面板开页→worker→箭头仍待一次UI开页验收。
- 同会话只读重放（复用生产头，PROCESS_VM_READ，关闭句柄后计算）：catalog 54、states 1157、splines 33765点／4281段／4279连接、8192×3616、shipping 98、world trade 66组／9海峡／4463对／1157中心、sea 12定义／84省、domestic 26755订单／419陆路／810州内／32铁路／371港口接入／411枢纽／94航线；模型shipments=3533（海1429／陆2104）、builtRail=719.251863 MU、target=136989.210／allocated=30890.397／unmet=106098.813。此为本机同源生产代码重放，不是DLL worker帧或绘图验收。
- 1.15面板链静态复核（同一冻结EXE、Ghidra）：CDataMarket vtable 0x480C528、CDataGoods vtable 0x480D108（构造0x200AD50保持+0x10定义、+0x18 state／market打包）、CMarketPanel vtable 0x4849150（构造0x24E7F40：market+0x70、search串+0x88、类别容器+0x110）、handler bar在+0xE8（closed+0x40、history数据+0x20、count+0x2c）；实读闭态count=0／closed=1与解码器一致。开态行值与history kind=9仍需开页实测。
- 未保存、未改经济；不改权限／分配语义。下一步唯一实机动作：打开市场面板物流页，核对native market owner selected、worker帧、ribbon与既有数量口径；随后补齐开态行／history验证。证据checkpoint2026-11-05-r196-v115-adaptation。

### 2026-11-05 — 逆向工具链接入 Claude Code MCP（工具环境，非产品变更）

按用户要求将本机既有逆向桥接注册进项目级 `.mcp.json`，复用原 Codex／DSH 同一命令、端口和环境：`ghidra`（ghidra-mcp 6.0.0 headless；用 `launcher_dsh.py` 懒加载默认组 listing／function／program／xref，列表 109 工具，其余用 `load_tool_group` 按需取）、`x64dbg`（x64dbg-mcp-server 2.3.0，23 工具，`X64DBG_MCP_TIMEOUT=15000`）、`wrath-analysis`（5 工具，`WRATH_WORKSPACE=E:\Program\Development\WrathPerformance`）。未新增下载、未改全局 PATH、未改 `~/.codex/config.toml` 与 DSH `cordis.patch.yml`。

以 MCP stdio 客户端按该文件逐个握手成功（ghidra 109／x64dbg 23／wrath-analysis 5 工具）；另对 ghidra 实际调用 `list_open_programs`，后端当前程序为冻结的 1.15.0 `victoria3.exe`（路径 checkpoint2026-11-05-r195-v115-compat/v115，x86:LE:64，image140000000，198937 函数），与 R195 分析目标一致。这只证明桥接与后端可用，不是新的 ABI／产品证据。

`.mcp.json` 含本机绝对路径，已加入 `.git/info/exclude`，不进入公开仓库。当前正在运行的会话启动时尚未有该文件，故这些工具要重启 Claude Code 并在项目 MCP 服务器提示中批准后才可用。x64dbg 工具需先运行 `Start-x64dbg.cmd` 让插件监听 127.0.0.1:27042；Ghidra headless 后端由启动器自动确保 127.0.0.1:8089，不自动附加游戏。未启动游戏、未操作进程、未改产品源码或 dist。

### 2026-11-05 — R195进行中：1.15渲染／GUI静态移植及定向回归

12组箭头接口在旧、新精确EXE均重新定位；新增7个发现器反例／正例测试，涵盖不同子类上传实现、缓冲字段不一致／歧义、this别名、调用破坏与新分配对象误判，定向通过并接统一入口。原始指令确认新版供给工厂分配E0、基类字段和float3／20B顶点ABI保持、常量buffer2C、this／path及this／passContext两参保持；原版2945240的domain／stage句柄来源独立确认。已移植这些renderer RVA／签名及资源租约身份，shader常量1与当前原版字段对齐。尚未改全局EXE准入hash，混合迁移阶段不能进入原生调用；未重建或安装中间DLL。

当前两份原版GUI哈希／原生回调和插入锚点已核对，更新C++与Python准入。首轮GUI布局回归发现新版市场价格控件215px使旧540px布局溢出5px；仅在自有物流页派生控件收窄至210，保留原版价格／趋势／导航表达式。修正后4组GUI结构／回调／布局／图例检查PASS；实际flow shader GPU定向检查PASS，包括近景海路高度、正反方向／八档宽度与反例。不是新版进程实际加载或全GUI手动验收。

新旅行网络定位world+5028的原生manager：陆地与海军节点／连接已拆为两对独立容器，旧RTTI读取不可复用。继续核对新版州市场、经济／贸易、访问完成点和native VFS及新网络结构，尚无1.15完整构建／统一回归／部署／实载结论；安装仍卸载，不push。证据checkpoint2026-11-05-r195-v115-compat，暂停R194完整保留。


### 2026-11-05 — R195进行中：Victoria3 1.15.0版本适配

用户在临时卸载后明确要求适配新版本。当前本机EXE为1.15.0／99988600B／SHA256473186e8d8deba993651e0ae4cf02a003b02f5accb99b096a6a0a8bd9506cbaa，当前无游戏、两份安装DLL仍卸载。按新任务继续兼容迁移；不将旧1.13.11验收扩成新版本结论。原始00–03按主题重读，继续local-reverse-engineering：先确认目标／hash，独立恢复、检查ABI／字段／寿命，再分层构建／测试／部署。

源码／测试／文档184份、旧交付双DLL和新EXE均已冻结于checkpoint2026-11-05-r195-v115-compat。暂停前R194的7份源码修改及新helper完整保留为R194-pending.patch和逐文件副本，当前迁移回到R193已验收功能基线，未盲目恢复旧dist。首次留档脚本遇路径键正反斜杠不一致而在改动前拒绝；归一化键后成功。

Ghidra当前新导入本次冻结EXE，x86:LE:64／image140000000；代码候选均绑定本次hash。通用箭头发现已独立找到构造／交叉构造、SetPath、类型解析与分配器；旧“两个子类上传函数地址相同”规则失败，当前两子类不同上传实现都读取对象+60，供给箭头分配尺寸C0→E0／常量2C，需按原始机器码验证而不放松唯一性。Append反编译遗漏参数，原始指令表明传入RDX仍由context函数消费，不能据伪代码改成单参。

旧CTravelConnection／CTravelNode／定义的RTTI在新版不再对应，出现NTravelNetwork::CNode／CConnection与STravelNodeDefinition／STravelConnectionDefinition，陆路采集需要按新结构实现。州、国家市场、TLS所有权等已有多处字段变化候选，未启用未经验证读取或调用。私有RTTI／函数指令对齐取得81份唯一候选，仍仅定位线索；全量临时指令缓存首实现占用过高已停止自身分析进程并改为逐函数释放，重跑完成，未动游戏或用户会话。

当前没有1.15构建／回归／部署／实机通过结论。下一步逐项验证正式渲染／UI／VFS／快照、州市场商品与新旅行网络，再移植并执行新版本测试。全部原始证据留本机checkpoint，不进入玩家资源或硬编码地图表。


### 2026-11-05 — 游戏更新后按用户要求临时卸载 DLL Mod

用户已暂停实施目标，并要求游戏更新后暂时卸载。初查自有目标游戏进程34872仍在运行，按deploy.ps1守卫等待；用户确认保存退出后重新核对无victoria3进程，执行deploy.ps1 -Uninstall退出0。已移除binaries\version.dll引导及binaries\freight\freight.dll主体；原先无version.dll.ironroad-backup，无需恢复其他代理。卸载前已将两份实际安装DLL备份到checkpoint2026-11-05-temporary-uninstall/installed-before；全部42份其余freight文件的路径／长度／SHA256卸载前后完全一致，配置、日志、缓存及可编辑资源保留。未启动游戏、未修改EXE或存档；当前EXE SHA256=473186e8d8deba993651e0ae4cf02a003b02f5accb99b096a6a0a8bd9506cbaa。仓库dist产物保留，未重新安装。

R194暂停前的同步采集／局部接入源码修改仍在工作区，尚未完成完整构建、回归或实机验收，未部署；R193历史59 PASS不覆盖这些未完成修改。实施目标保持暂停，新版本ABI／兼容性未验证，不因本次卸载恢复开发或安装。证据：E:/Code/v3-analysis/checkpoints/2026-11-05-temporary-uninstall/{before.json,installed-before,uninstall.log,uninstall-audit.json}。


### 2026-11-03 — R193：动态物理港口接入原版查询实机核对

- R192同参重查是进展，本轮实际执行新的局部港口请求。新增native_port_approach.h只复制当前typed运行州、原版市场、当前旅行连接／节点／省份owner、海区与真实spline数值；不创建ID→key表，不选第一个港口候选。原版14144FEF0确实使用有效且非排除海区的首个省份作为规范表示，此为海区表示规则。不同完整owner但同真实市场可保留，其他市场排除；同海区不追加请求，所有候选自有值且变更／退休拒绝。
- 首版误将state+48分组当市场，代码复查发现与现有state_market.h明确约束冲突；此前“6份同市场／7份排除”结论撤回。正式改用已有DecodeNativeStateMarket的typed国家／特殊市场区及市场对象校验，新增不同分组仍同市场的反例，修正后定向PASS并完整重建回归。首版构建／统一入口与私有probe日志保留为历史，不继承为最终候选证据。私有首probe还因将13份读取嵌入已有32MiB总窗口触发预算拒绝，改为各自独立窗口，不放宽边界。两次旧产品元数据会话9928／23872均正常退出0、五输入保持、probe_access始终0。
- 默认关闭probe_access中复用经验证的原版wrapper／GameState2／hidden0／owns1／context1与同次heap窗口，读取同次已选完整州、原country与默认options；按query源侧物理→逻辑海区／目的侧逻辑→物理海区构造明确24B单候选数组和56B选项。当前typed pathfinder索引仅验证登记，不把基础矩阵内容作权限或结果回退。复用独立40B结果调用／原生Free，暂移父trace，不计入自然查询；原输入／原选择／自有输入／scope／heap／manager／港口重核，异常停止后续请求，最多128次。不接正式worker、权限批次或几何。
- 15组私有原始机器码harness用自有输入数组执行真实查询／selector／move／allocator wrapper，覆盖新端点、反向、无路、原版海峡拒绝及原有选项／重查边界，输入和原输出字节保持、临时分配回收PASS。底层heap／country／base-path／restricted服务仍替代，不冒充游戏heap／通行实测。新typed输入读取测试进入统一入口，包含逐依赖读失败、错误类型／身份／计数／角色、多个候选、市场隔离、复制中变化／退休与源销毁。
- 最终完整build.ps1退出0、build_tests.ps1退出0／59 PASS／零FAIL，embedded PASS、默认debug零UI顶点PASS；主体f328f68461fd998c2d2eb7abcee55ed125eabe5f23fa516a3cbca69a3fc67c09／51363876B，引导71dae36d…／779879B，经deploy.ps1安装并实际载入，两份dist＝安装＝本次模块。产品路径模型仍R185行为；本轮新增诊断与输入复制，不声称三末端已可视修复。
- 最终自有41396普通续玩，victoria3.exe SHA7f53e326…／静态image140000000／实载base0x7ff748fa0000，phase4／暂停／date59883804。自然4010查询／103完整port选择／17重建栈及103同参独立重查保持；206端点元数据观察均有效。当前13份错配州普查按真实市场得到eligible=7／foreign=6，逐项与原版keyless港内连接的实际owner及生产州roster市场一致。额外新局部请求1份、1个country，涉及当前state=[936]；每份均与同次原版选择、动态物理端点、原country／world／线程及build诊断值对应，原输入／结果值保持，独立分配并原生释放1份，已知无路0份。完整范围和距离记录于approach-audit.json；诊断build数值不是正式epoch，释放返回不是OS物理内存回收测量。
- 与R192同一保护存档／日期的4010自然查询按完整country／源组／目的组／forced／pairs／selector多重集合对照，原权限、最终端点／省份范围／距离全部相同，不依赖调用顺序；这是当前暂停场景的原版寻路结果保持，非完整经济或性能对比。自有会话无GUI／相机／推进／保存输入，加载模块于存活时归档，正常WM_CLOSE退出0；五原始输入逐字节保持，临时probe_access1原子恢复0。相关资源错误0、六条Unexpected token同基线，当前无游戏、不提交／push、GUI手动。用户说明R188中断源于停电非正常关机，按外部中断记录，不能当正常退出或DLL崩溃证据。
- 动态物理港口参数与新本国局部原版查询子项完成。下一步把有效局部结果绑定当前lane完整身份／真实几何／world-map-request及退休更新，再接模型／worker／正式绘制；其他市场的物理港口不能仅凭共享地区放行。三末端仍未修，完整权限生产方、原子交接、改图、GUI／生命周期／性能与同构建A01–A27继续，goal ACTIVE。证据checkpoint2026-11-03-r193-dynamic-port-queries/{before,metadata-live,metadata-live-2,port-approach-corrected.log,approach-contract.log,trade-contract-cases.json,source-change.diff,build.log,tests.log,embedded.log,debug-default.log,deploy.log,live/loaded-modules.json,live/observation-counters.json,live/metadata-probe.log,live/approach-audit.json,live/replay-audit.json,live/ownership-audit.json,live/stack-audit.json,live/session-freight.log,live/close.json,live/after.json,config-restored.json,audit.json,NEXT.md}。


### 2026-11-03 — R193进行中：动态物理港口与本国接入查询

R192同参独立查询／释放是实际进展。本轮新增仅测试／默认关闭诊断使用的typed当前州→物理港口海区输入复制器，保持运行州／定义／geo／实际spline身份分离，不创建ID→key表。异常／读失败／变更／退休／源销毁与分属州市场隔离用例已接统一入口，定向PASS；尚无本轮完整构建回归结论。原版14144FEF0确实取有效非排除海区的首个省份作为规范表示，不把它解释为首个港口候选。物理港口候选全部保留。

原产品06e7272e…的自有9928、23872两次暂停VM_READ核对均正常退出0、五输入保持，probe_access始终0；用户说明的停电属于R188外部中断，不能充当DLL崩溃或正常退出证据。第一次私有probe将13份新decoder复用到已有总32MiB观察窗口，后续读取被预算拒绝；改为每份独立观察窗口后13份全部有效，并逐项与真实原版keyless港内连接的实际owner相同。首版误把state+48分组当市场，导致“6份同市场／7份排除”的结论已撤回；207→206、608→607与936→936的实际owner对照仍成立，市场必须以已有DecodeNativeStateMarket的typed国家／特殊市场区分支重新核对。已修正源码并增加不同分组但同真实市场仍可接入的反例，定向重新PASS；首版构建／回归日志保留，不能作修正后候选验收。该事实不自动证明原版航线本身无路，也不允许借其他市场港口补画。

正在同原版wrapper持有的已验证scope／heap窗口内，使用动态候选、原country、自有CPdxArray24与默认options56执行有界新请求；原生基础缓存仅核对索引登记、不作结果／权限回退。原输出／输入／scope／heap／当前港口重核，独立buffer经原Free释放；异常停后续请求。未接正式worker／权限或绘制，三处末端仍未修，不push／GUI手动。源码前备份及只读证据checkpoint2026-11-03-r193-dynamic-port-queries；产品安装仍R192，完整目标ACTIVE。


### 2026-11-03 — R192：103次原版同参独立重查及原生释放实机通过

- R191实际调用链与持有的scope是进展，本轮复核既有R100／R101／R106及代码：CD2BC1已用于正式采集及原版selector，不另建锁接口；完整经济输入writer完成／原子性仍待。本轮采用范围更明确的自然1414500F0→14141F1D0返回窗口：原caller仍持有相同候选数组／options，当前原版函数已完成且scope2／hidden0／owns1／context1与同次heap验证成立。该依据是完整ABI／实际原caller栈／借用寿命及原版调用协议的组合，不是只凭瞬时flags给新线程授予权限；不扩展为不同港口请求或整份快照安全证明。
- 源码前备份checkpoint2026-11-03-r192-native-query-replay/before。默认关闭probe_access下最多128次同参重查，调用既有原始trampoline并使用独立40B输出，完整七槽含scratch／options原样保留。重查时暂时移走自有父trace，避免污染自然query／classifier／port记录；独立数据必须不与原buffer别名、allocator为当前已验证CPdxNewDeleteAllocator，释放入口14B签名匹配，RAII在复制之后经原生Free(this,data)释放。原请求／选项值、原输出值、scope／heap不一致即停止后续重查；不发布路线、权限或worker任务，不调用经济写入接口。释放是原版回收调用返回，不是向操作系统归还物理内存的测量。
- 私有原始机器码harness将R189的11组每组重查一次，同时保留原输出：结果相同、输入及原输出字节保持、两份非空buffer独立、均经真实原版allocator wrapper释放，所有临时分配回收PASS。底层heap／country／base-path等服务仍替代，独立结果不冒充原生游戏heap证据。源码只改overlay诊断；未为低影响日志／分支复制镜像测试，真实ABI的额外用例留外部checkpoint，既有统一入口完整运行。
- 完整native/build.ps1退出0、统一build_tests.ps1退出0／58 PASS／零FAIL，embedded PASS，默认debug独立backend3664零UI顶点PASS。主体06e7272e0b0c1e901117e0ecdcd248815e55006bce23234e45f028f1c1ba88cf／51072951B，引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B；deploy.ps1安装，dist＝安装＝自有32164实际模块。两条旧“绝不新增查询”注释随后同等行数对齐新诊断范围，没有改编译token或行为、未因此重复完整测试。
- 自有32164普通续玩，无GUI／相机／推进输入；EXE SHA7f53e326…／base7ff748fa0000，phase4／暂停／date59883804／world41e732e9000。实际DLL COFF符号独立解析9个自有计数器，双读稳定：自然4010＝3907普通＋103国内，copy数一致；额外attempt103／matched103，独立日志103份全部matched1／independent1／allocated1／freed1，逐条query／country／thread／build／两端／距离／range数对应自然记录，17个country、7份同省合法零距离；实际已知无路0。103个原版port ownership及17条相同ExecuteGameCommand调用栈保持，临时屏蔽父trace未改变原有计数和绑定。
- 相关模块清单在存活时归档；自有32164正常WM_CLOSE退出0，五原始保护输入逐字节保持，临时probe_access1原子恢复0。日志超过8MiB后启动轮转，原old文件与启动前prefix逐字节相同，finish按原守卫归档；相关资源错误0，六条Unexpected token同基线。当前无游戏，未保存／提交／push，GUI手动。实时和退出后的最终session日志均严格核对通过，无本轮未解释的构建／回归／实机失败。
- 同一原生借用请求的额外调用／独立输出／实际释放子项完成。三港口缺段尚未修，不能把这103次相同参数结果当三个新接入段的结果。下一步在已验证原wrapper窗口读取当前已选完整州的typed物理港口／海区，构造自己的有界单候选数组和默认options，调用同一原版逻辑验证真实本国接入请求；禁止固定state／sea／key表、借异国许可或基础缓存补路。正式权限批次生产方、world／map／request与退休／原子交接、改图、完整GUI／生命周期／性能和最终同构建A01–A27继续，goal ACTIVE。证据checkpoint2026-11-03-r192-native-query-replay/{source-change.diff,replay_contract.py,replay-contract.log,trade-contract-cases.json,build.log,tests.log,embedded.log,debug-default.log,deploy.log,live/loaded-modules.json,live/observation-counters.json,live/replay-audit.json,live/ownership-audit.json,live/stack-audit.json,live/session-freight.log,live/close.json,live/after.json,config-restored.json,audit.json,NEXT.md}。


### 2026-11-03 — R192进行中：原版相同参数重查及独立输出释放

R191实际17条调用栈是进展。本轮复核已有R100／R101／R106完成点与任务证据：CD2BC1已用于正式采集，不新增锁钩子；完整writer完成证明仍待。先只在自然1414500F0→14141F1D0返回窗口、完整原请求仍由原caller持有且scope2／owns1／context1／hidden0及同次heap已验证时，默认关闭probe_access诊断最多128次相同参数独立重查。原借用参数／原输出不变，临时屏蔽自有父trace归类，重查不冒充自然调用；独立40B输出经原版分配器释放，异常即停后续重查。不接不同港口请求或正式路径。源码前备份checkpoint2026-11-03-r192-native-query-replay/before；11项原始机器码重复调用／独立所有权／真实wrapper释放用例PASS（底层heap等仍替代），完整构建中。安装仍f42f9e6b…／71dae36d…，候选无统一回归／部署／实机结论。不push／GUI手动、三末端仍待。


### 2026-11-03 — R191：实机确定贸易重建由ExecuteGameCommand作用域包围，间隔调用并非任务join

- R190的103次buffer／heap实机核对是实际进展。本轮转访问窗口主题，完整重读01／02／03，适用原始指针仅在已验证窗口借用、未知ABI保持关闭、分层证据和worker自有值等约束。精确EXE SHA7f53e326…／image140000000再次校验；Capstone按完整指令边界验证PE相对引用，不将位移匹配或首个caller当执行证据。
- 静态找到14140D830的3条直接caller：140FF0A67、14133E9D6、14140D763；FF0A30另经FF5DF0注册，RTTI是接收CGamestateGenerationContext的函数包装。这只是候选，不是本次启动路径。源码前备份checkpoint2026-11-03-r191-trade-access-window/before/overlay.cpp；默认关闭probe_access的原重建观察器增加24帧有界Windows调用栈与直接return PC，在进入MinHook trampoline前捕获，避免用其搬移prologue作原版unwind依据。原函数仍只转发一次，不增加查询／锁调用／经济写入或正式发布。
- 完整native/build.ps1退出0、统一build_tests.ps1退出0／58 PASS／零FAIL、embedded PASS，默认debug独立backend10168零UI顶点PASS。当前主体f42f9e6bc1b6ec1499258bdaa7db3735dd6cacb2fd47b8b4aec4520b77d69750／51038667B，引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B；经deploy.ps1安装，dist＝安装＝41000实际两模块。原R190产品已由实际安装文件冻结到delivery-before，不恢复旧dist。
- 自有41000普通续玩、无GUI／相机／推进输入；EXE base7ff748fa0000，phase4／暂停／date59883804／world4e273f45000。4010自然查询＝3907普通＋103国内，计数及copy完全对应；103条ownership继续全部ready／same_allocator／moved1。17次非空国内重建均thread7712／scope2,1,1，深度14未达24上限，各有相同11个EXE内frame；直接caller为14140D768，真实链为140CD2810(return CD2AB6)→140FC1FE0(return FC2198)→14140D670(return140D768)→14140D830，而非生成回调。原始PC及所有记录都保留，未从静态候选猜路径。
- 沿这条实际链确认140CD2810的标记ExecuteGameCommand／gamecommand.cpp，140CD28BE以GameState2／context1／source gamecommand.cpp:60调用1433BD640，storage为RBP-20；140CD2BBC对同一storage调用1433BD750。现有生产AccessScopeEndThunk及IsNativeCapturePoint已经使用其返回CD2BC1；应复用这个完成点，不能因本轮发现再新增通用锁接口或重复完成钩子。此次实机栈补强实际trade调用被该作用域包围的证据，不等于所有输入writer已join。
- 重要修正：FC1FE0每阶段间的1407BEB90不是任务等待。其完整体构造7BE8E0、析构7BEA60：受运行标记／计时条件控制，构造可调用1433BD820暂时清owns、置access3并释放root+590／SRW，析构1433BD8F0重新取得并恢复scope；另有计时／诊断记录。不能将它作为join屏障。实际shipping入口在FC2180这次间隔调用返回后，经FC2193进入，下一次间隔调用在FC2198；这只是已证调用顺序，仍须核对其间transitive调用及原版任务完成条件。E765E0也存在内联release／reacquire，单凭ctor／dtor或瞬时owns标记不授予完整原子租约。
- 自有41000正常WM_CLOSE退出0；五原始保护输入逐字节保持，临时probe_access1原子恢复0；相关资源错误0，六条Unexpected token同基线，当前无游戏。未保存／提交／push，GUI手动。私有首个research参数误传VA被numpy拒绝，已加RVA范围守卫；一处按猜测文件名读assembly不存在，改用实际extent名；组合dump第二个无pdata的候选被assert拒绝，未放松边界或将不完整体作证明。
- 实际调用链及作用域包围子项完成，三海路末端仍未修、主动搜索与正式逐lane权限生产方仍未启用。下一步复用现有CD2BC1完成点，沿ExecuteGameCommand及真实transitive任务等待／恢复路径补齐访问证明，并将当前world／map／request与真实权限生产方连接；不再重复buffer-only观察或把yield当join。改图、完整GUI／生命周期／性能及最终同构建A01–A27继续，goal ACTIVE。证据checkpoint2026-11-03-r191-trade-access-window/{source-change.diff,callback-types.json,callers-*.json,*-assembly.txt,build.log,tests.log,embedded.log,debug-default.log,deploy.log,live/loaded-modules.json,live/observation-counters.json,live/ownership-audit.json,live/stack-audit.json,live/session-freight.log,live/close.json,live/after.json,config-restored.json,audit.json,NEXT.md}。


### 2026-11-03 — R191进行中：原版贸易重建调用栈与访问窗口

R190的103次真实buffer转移／heap观察是实际进展。本轮按访问窗口主题完整重读01／02／03，沿同hash EXE完整汇编确认国内重建有3条直接caller，一条经callback注册，不能按首个caller推断本次启动路径。源码前备份checkpoint2026-11-03-r191-trade-access-window/before/overlay.cpp；只在原默认关闭probe_access观察器增加24帧有界调用栈及明确return PC，原函数仍只转发一次、无新搜索或正式发布。完整构建进行中，候选尚无统一回归／部署／实机结论；安装仍R19030596be5…／71dae36d…。继续核对acquire／yield／任务join；三末端未修，不push／GUI手动。


### 2026-11-03 — R190：103次自然结果buffer转移与原版线程heap实机核对

- R189原始机器码／21项隔离用例解决了结果移动语义，本轮将核对接入既有默认关闭probe_access诊断。源码前备份checkpoint2026-11-03-r190-natural-buffer-transfer/before；新增ObserveNativeThreadAllocator读取当前TLS索引（有界4096）、初始化标记／heap并双读，验证当前CPdxNewDeleteAllocator的VT及allocate／release方法；ObserveNativeTradeBuffer只双读24B descriptor，保留data／capacity／count／allocator，不解引用未来buffer或调用分配器。
- 原版九槽wrapper的栈内trace保存其同次七槽query返回descriptor与heap观察，外层返回后核对scope／完整输入／结果内容及两阶段heap、buffer；借用地址只在同一自然调用栈中使用并记录数值，不交worker／权限批次。原函数仍各转发一次，没有新增搜索、原生分配释放、经济写入或正式路径发布。零count持capacity合法；错误方法／未初始化／无heap／索引或地址越界／任何字段不可读／复制期间变化／源销毁等回归接入既有native_access_tests，定向PASS。
- 完整native/build.ps1退出0、统一build_tests.ps1退出0／58 PASS／零FAIL，embedded PASS、默认debug独立backend42592零UI顶点PASS。当前主体30596be5262b6c4e7b5c197c5865cf81770184c7e0e58b7aafee52b9474ae4af／51027697B，引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B；deploy.ps1安装，两份dist＝安装，实际28600模块路径／hash匹配。EXE SHA7f53e326…／运行基址7ff748fa0000，未跨构建复用探针RVA。
- 自有28600普通续玩、无GUI／相机／推进输入，phase4／暂停／date59883804／world370ba76a000。完整4010自然query＝3907普通＋103国内，与实际DLL自有atomic双读计数和copy数完全相等；103条port selection各有唯一同sample／query／thread／build／country的ownership记录，全部ready1／same_allocator1／moved1，query_data＝final_data非空、capacity／count相同，旧destination均空。103次均在thread40024，TLS索引0／TLS15764b1a8a0／heap15779400000／TEB39875b9000一致。结果持有者确实接过原buffer；没有将该指针当跨调用身份或world代数。
- 独立模块清单已在进程存活时归档。自有28600正常WM_CLOSE退出0，五原始保护输入逐字节保持，临时probe_access1原子恢复0；相关资源错误0，六条Unexpected token同基线，当前无游戏。未保存／提交／push。私有首版日志reader将历史前缀及安装消息计入selection而assert拒绝，按本次原始prefix及sample字段隔离后103对103严格匹配，并在退出后的最终session日志重复通过；未改原始日志或放松结果断言。
- 自然结果转移／线程heap观察子项完成，明确不等于原生主动搜索的访问租约、完整写任务join、world-map-request epoch、真实分配释放与退休证明。下一步核对原版包围贸易重建的scope acquire／yield／reacquire／release及任务完成窗口，不能仅凭这组同值字段启用查询。三海路末端仍未修，正式逐lane权限生产方未绑定；地图兼容、完整GUI／生命周期／性能和最终同构建A01–A27继续，goal ACTIVE。证据checkpoint2026-11-03-r190-natural-buffer-transfer/{source-change.diff,native-access.log,build.log,tests.log,embedded.log,debug-default.log,deploy.log,live/loaded-modules.json,live/observation-counters.json,live/ownership-audit.json,live/session-freight.log,live/close.json,live/after.json,config-restored.json,audit.json,NEXT.md}。


### 2026-11-03 — R190进行中：自然查询buffer转移与线程heap观察

R189的21项原始机器码用例是实际进展，本轮继续同一调用所有权主题。源码前备份checkpoint2026-11-03-r190-natural-buffer-transfer/before。新增仅观察的当前TLS索引／heap就绪与精确allocator方法核对、24B buffer描述符双读；边界／读失败／变化／源销毁用例进入既有native_access_tests并定向PASS。默认关闭probe_access的自然端口wrapper及其嵌套query记录同次descriptor转移，原函数仍各转发一次，无额外搜索／分配／释放、无正式权限或路径发布。完整构建进行中，尚无本候选统一回归／部署／实机结论；安装仍950d3f32…／71dae36d…。不push、GUI手动，三海路末端及完整目标继续。


### 2026-11-03 — R189：原版结果转移与分配器契约，21项隔离机器码用例通过

- 接续R188完整4010查询是实际进展，本轮离线核对主动查询前的结果所有权，不启动游戏、不操作GUI、不push。精确EXE SHA7f53e326…／image140000000再次校验。停电后的Ghidra当前无打开程序；状态查询明确返回Program not found／空列表，改以同hash本机PE的完整原始汇编及RTTI核对，不将历史连接当当前可用。
- 原版14512CDC0对象RTTI为CPdxNewDeleteAllocator，VT144419230；slot+8的14061BE50接受this／u64字节数／第三槽对齐，实际只转发字节数至14417D960，未消费第三槽。slot+10的14061BE90接受this／data，空指针不调用后端，非空尾调14417D850。不得跨构建复用此地址。14417D960依赖GS:58当前TLS索引1458F8C90及线程heap字段38／初始化44；14417D850从指针所属区域元数据与GS:30核对归属并分支回收。不能用DLL malloc/free或“任意线程调用operator new”代替原版分配器。
- 关键纠正：R183／R186称copy的140D97AC0实际有消费源对象的移动语义。同allocator时先释放旧destination，再转移data／capacity／count并清空source；不同allocator时按目标容量复制或经140C6C8C0／140639FF0增长，source count清零但仍持原buffer供其自身析构。1414502E3调用后，141450306–319只在临时source仍非空时释放；同allocator移动分支不会释放已交给外层的buffer。旧文档“复制后释放内部结果”须按本条解释，不得据此监听一次预期不存在的临时free或重复释放外层内存。
- 新私有隔离harness执行完整原始move／reserve／rebind／allocator wrapper机器码，仅替代底层heap与memcpy：同allocator空／非空／替换／零count有capacity、不同allocator复用／扩容及null-free共10例PASS，逐指针检查最终存活与回收，无重复释放／保留分配。再将原11项完整贸易wrapper／selector／filter harness中的假copy及allocator wrapper替换为这些真实方法，11例PASS，全部结果与R183逐项一致、临时分配全部释放、输入字节保持。合计21项隔离用例，不是实际游戏heap／TLS／主动查询安全证明。
- 原始机器码／工具与fixture全部留外部checkpoint2026-11-01-r189-native-result-allocator，不加入玩家DLL或仓库测试；没有修改产品源码、构建、部署或运行游戏，未重跑无变化的产品统一入口。当前dist＝安装仍950d3f32…／71dae36d…，58 PASS／资源／默认debug为R188同产品历史证据；五保护输入仍一致、probe_access0、无游戏。最初私有脚本名inspect.py遮蔽stdlib导致导入失败，改inspect_allocator.py；无pdata叶函数最初无范围被assert拒绝，按完整14B／精确反汇编范围处理，未放松哈希／ABI边界。
- 结果所有权的静态／隔离子项完成；下一步在已验证的自然外层调用栈内核对同次结果buffer转移及实际allocator／TLS上下文，不从render／worker另行调用。正式查询仍需真实访问窗口、完整world／map／request身份、原生完成／退休及输出寿命证据；不将上述fixture或指针相等冒充租约。三海路末端仍未修，逐lane权限生产方、改图、GUI／生命周期／性能／最终A01–A27继续，goal ACTIVE。


### 2026-11-01 — R188：4,010次自然贸易查询完整覆盖，局部港口查询并未自然产生

- 仅扩大默认关闭probe_access诊断额度：普通检查128→8192／复制32→4096，国内仍512／256；原函数仍只转发一次，country／options／同次分类／scope复核不变。没有主动搜索、正式权限发布、路线补画或经济写入。源码前备份checkpoint2026-11-01-r188-complete-trade-observation/before；不push／GUI手动。
- 完整native/build.ps1退出0、统一build_tests.ps1退出0／58 PASS／零FAIL、embedded PASS、默认debug独立backend94292零UI顶点PASS。主体950d3f324047d94ef2b0686e047b37eeb512790d902a3584fa650de2752419c6／50968135B，引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B；deploy.ps1部署，dist与安装一致。只改变诊断额度，R185业务逻辑不变；旧用户缺段确认不扩为本构建完整GUI验收。
- 自有95000普通续玩，phase4／暂停／date59883804，临时probe_access1。当前DLL符号表解析7个自有atomic地址，绑定进程创建时间／实际模块路径与hash，PROCESS_VM_READ两次计数相同：总4010＝普通3907＋国内103，分别与全部日志copy一一相等；port选择103／103。样本0–4009连续、283个country、已知无路0，无本轮额度截断或复制丢失。模块基址7ffdc2c80000由当时枚举／计数读取得到；事后完整modules.py因进程已不存在拒绝，未伪造模块清单。
- 当前动态PORT_MISMATCH／MISSING_PORT／OWNED_LANE关联三处缺口，按同country／海省对／完整options核对：前两对完全没有自然请求，第三对也无对应country，仅另一country有1份。故本次启动中扩大被动观察已足够，不能借其他国家结果／基础矩阵补画，也不能把缺查询当原版无路。三海路末端仍未修，下一步核对完整原生查询的真实分配释放及调用访问窗口，或找到原版对应端点路径来源；不继续只加配额。
- 恢复工作时95000已不存在，未发送WM_CLOSE、未kill、无close.json或退出码。用户明确说明停电造成电脑非正常关机，记为外部中断；本次正常退出验收未完成。Application事件查询未发现本次Victoria3／freight崩溃条目、最新游戏crash目录仍10月27日，这不能替代退出码。已归档现存日志并复核原始五输入逐字节保持，临时INI原子恢复原probe_access0；相关资源错误0，六条Unexpected token同基线。当前无游戏，没有保存／提交／push。
- 自然调用覆盖核对子项完成，不代表三个局部查询已有原版结果或安全主动调用已证。正式world／map／request身份、访问租约／退休、逐lane权限生产方、改图、完整GUI／生命周期／性能及最终同构建A01–A27继续，整体goal ACTIVE。证据checkpoint2026-11-01-r188-complete-trade-observation/{build.log,tests.log,embedded.log,debug-default.log,deploy.log,live/observation-counters.json,live/all-natural-queries.json,live/approach-query-audit.json,live/port-census.log,live/owned-lanes.log,live/session-freight.log,live/after.json,config-restored.json,audit.json,NEXT.md}。


### 2026-11-01 — R188进行中：完整贸易自然调用覆盖与港口接入查询

- 上轮R187新增自有海区缓存复制并以真实差异否定基础矩阵等于最终查询，是实际进展。本轮继续三处末端：只将现有probe_access诊断的普通查询copy上限32→4096、检查上限128→8192，国内仍256／512，保留全部country／options／同次分类和scope校验。所有限制仍有界，不追加原生寻路，不修改正式路径或经济；不push／GUI手动。
- 源码前备份checkpoint2026-11-01-r188-complete-trade-observation/before；完整native/build.ps1已退出0，当前候选两DLL待完整统一回归，尚无该候选部署／实机结论。安装仍R186719a2d83…，R187也未改过产品。
- 私有只读coverage reader从本次交付DLL的COFF符号表导出自有atomic计数器地址，绑定实际模块路径／hash／进程创建时间后双读；不跨构建复用RVA，不读写引擎任务状态。后续以总调用／copy计数与解析日志核对覆盖，再由当前动态港口错配记录查同country／同海省对的完整自然查询，不借其他国家权限或基础缓存补画。

### 2026-11-01 — R187：原版海区基础缓存自有复制与最终查询差异实机核对

- 上轮R186的完整州选择为实际进展，本轮继续三处海路端点。新增native_sea_cache.h仅由既有native_access_tests使用，未接overlay／正式采集／worker。当前root→world+4500的typed CSeaProvincePathfinder、原版u16索引Robin探测、24B三角行、32B条目和i64距离／原始省份集合复制为自有值；不调用搜索／getter／引擎分配器。空缓存只是观察到空条目，不证明失败搜索，源＝目的属于原版另一个分支，显式不由此helper伪造；非空零距离保留。
- 精确EXE SHA7f53e326…／image140000000；原版14141FA80缓存分支与14141EA10填充、RTTI VT044EB4D8和1413BAAF0构造核对。构造先发布owner+100指针，再调用填充，不能将指针存在或矩阵尺寸正确当完成见证。PE原始xref扫描的1412C2224候选是将call操作码误当disp的假引用，汇编排除；真实构造调用为1413BABAF。后续必须绑定构建完成／退休／world-map-request及原子访问窗口，当前纯复制器不授予这些能力。
- 新测试进入原统一入口：方向与原版三角列、Robin碰撞／存在于错误位置不可被原版探测、类型／重复索引／key／数量／地址／范围／距离／读失败、复制中变化、scope退休与源销毁；定向PASS。完整build_tests.ps1退出0／58 PASS／零FAIL、embedded PASS。未改产品调用点、未重建或部署两份DLL；主体仍719a2d8313a57f86316449bbbb34b023a839d598e6a308d8dc61fa960bcebab4／50968135B，引导71dae36d…／779879B，dist＝安装＝before冻结，R186默认debug是同产品历史证据。
- 自有3236普通续玩，复用R186默认关闭的自然观察临时probe_access1；实际模块为上述同产品、EXE base7ff63dec0000。当前自然135份最终查询（32普通＋103国内）与两个只读probe的cache scope同world4575261c000，phase4／暂停／date59883804，无GUI／相机／推进输入。以84个不同有向海省对复制128份非同省查询对应基础缓存，重复复制逐字节值一致：83份与最终查询的距离及全部范围顺序相同，45份不同＝30份缓存空但最终有路＋15份非空缓存距离更长；国内相同67／不同29，另7同省由原版本身已知零分支处理，未伪装为此缓存读取。
- 此差异否定“原生基础缓存就是最终可通行贸易结果”的假设，不是将空条目当无路的理由。完整14141F1D0尚有候选／限制及141420800结果比较等逻辑；不能据上述值推断具体45份都走哪一分支，也不能绕过country／options／通行取得全路线。初比较脚本曾按全128必相同断言，sample0缓存空与实际正距离导致拒绝；纠正为逐项记录差异，保留失败说明，不把错误假设改成源数据。
- 当前624份州港绑定动态错配13行／6地区，对应5个不同配置出口↔物理港口海区对，全部读得原生缓存。三处缺口对应的3对均两海省／一条当前海样条，空权限几何检查无governed adjacency；另一个错配对35987↔36029有直连几何key，但原版缓存为长路径／448997480、九条几何key且空权限检查挡6个有向governed邻接。这是不能按现有direct key补路的具体反例。三处局部缓存条目不等于完整商业选择见证，尚未据此补画／改变模型或放行海峡。
- 自有3236只读后正常WM_CLOSE退出0，五原始保护输入逐字节保持，临时probe_access1原子恢复原0；扩展相关资源错误0、六条Unexpected token同基线。当前审计无游戏，未保存、提交或push。私有query probe首生成误套模板tail而被assert拒绝，后续编译找不到未生成文件；按实际owned-lane尾部修正并加生成失败守卫后编译通过。Ghidra未定义两个小函数时改读完整汇编，未创建假函数或放松结果。
- 海区基础缓存的自有复制／实机条目核对子项完成，三海路末端与正式权限仍未修。下一步查完整贸易函数对当前这3对局部接入的真实选择／country／options，及原生构建完成、退休与身份交接；不把基础矩阵作最终路线回退。地图兼容、生命周期／GUI原生提交／性能及最终同构建A01–A27继续，整体goal ACTIVE。源码前备份与全部证据checkpoint2026-11-01-r187-native-sea-cache/{before,delivery-before,native-access.log,tests.log,embedded.log,cache-type.json,cache-getter-assembly.txt,cache-constructor-assembly.txt,cache-builder-callers.json,source-change.diff,live/natural-queries.json,live/query-cache-scope.log,live/cache-ports-scope.log,live/cache-query-comparison.json,live/loaded-modules.json,live/close.json,live/after.json,config-restored.json,audit.json,NEXT.md}。


### 2026-11-01 — R187进行中：原生海区缓存复制与港口缺口只读核对

- 上轮R186完成完整州／同次查询实机绑定，是实际进展。本轮继续三处海路末端，按现有主题与local-reverse-engineering精确EXE边界研究，不操作GUI、不push。源码前备份checkpoint2026-11-01-r187-native-sea-cache/before；产品源码／两份DLL未改，安装仍719a2d83…／71dae36d…。
- 精确EXE及14141FA80原始缓存分支核对后，新增只在native_access_tests使用的native_sea_cache.h：当前root／world+4500、typed CSeaProvincePathfinder、原版u16→index Robin碰撞查找、24B三角行、32B条目与i64距离／省份集合复制。数据自有，未调用引擎／分配／搜索；空条目只表示观察到空缓存，不证明失败搜索或通行。异常／越界／错误类型／身份重复／原版探测不可达／复制中变化／退休／源销毁用例定向PASS，已在原统一入口目标内。
- RTTI与构造调用确认VT044EB4D8；1413BAAF0先写owner+100缓存指针，再执行14141EA10填充，因此不能把非空指针当完成见证。PE初扫描的另一候选1412C2224经汇编判为把call操作码当disp产生的假xref，未当真实writer。后续正式消费仍需原生构建完成／寿命窗口与失效绑定，当前helper未接runtime或worker。
- 完整统一回归仍在运行，无本轮最终PASS／实机结论。正在准备只读比较当前自然贸易返回与原生缓存，以及配置出口到物理港口的动态错配对；私有probe模板首生成错误因实际tail不同被assert拒绝，已按实际owned-lane模板修正并编译，不涉及产品代码或游戏。不能据已有direct key／无governed pair就补画；完整目标ACTIVE。

### 2026-11-01 — R186：完整州选择与嵌套贸易返回实机交接，103次国内查询覆盖完整

- 上轮R185是实际源码／部署及用户缺段确认进展；本轮继续原版海路端点绑定。R185的85984在新轮库存中已不存在，未由我方关闭。源码前备份checkpoint2026-11-01-r186-port-selection/before，沿既有local-reverse-engineering精确EXE／ABI／所有权门槛；手动GUI与不push继续。
- 再次核对1414500F0及141419FEF原始caller：完整九槽void ABI，输出ProvinceSet、两个u16海省、两个完整u32州、距离i64、两份u32候选州组与country32；caller返回后才释放候选。原版将同次选中海省映射为完整州。新增CopyNativePortSearchInputs／Selection，保留完整句柄、零、重复和顺序；最终路径／海省／距离必须逐项等于同次嵌套贸易结果，完整州必须属于对应组。已知无路、未知读取失败及合法非空零距离分开；新边界／读失败／变更／方向／源销毁回归进入原native_access_tests。
- 默认关闭probe_access下新增自然wrapper观察，只转发九槽原函数一次、不额外查询／分配／释放。TLS要求同完整country、同次恰好一次合法嵌套贸易调用，与外层前后world／machine／stateDB／date／phase／access／hidden／owns／context／paused及候选匹配，再复制最终州输出并查typed当前完整州；借用指针不出调用栈，日志只含自有数值。国内查询记录额度96→256、检查上限仍512；普通仍32／检查128。未向正式worker／权限批次／几何发布，顺序观察不授予访问租约／ABA安全。
- 完整native/build.ps1退出0、统一build_tests.ps1退出0／58 PASS／零FAIL，embedded PASS、默认debug独立backend95408零UI顶点PASS。当前主体719a2d8313a57f86316449bbbb34b023a839d598e6a308d8dc61fa960bcebab4／50968135B，引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B；经deploy.ps1安装，源码／dist／安装／自有93548实际模块对应。EXE SHA7f53e326…／image140000000／加载base7ff63dec0000。只改变默认关闭的诊断与自有复制器，R185业务逻辑未改；R185用户GUI确认不冒充本轮完整GUI验收。
- 普通EXE续玩、临时probe_access1，phase4／暂停／date59883804。完整取得135份查询＝32普通＋103国内，103份外层完整州选择与各自内层query编号／country／world／线程／构建诊断代数／日期／访问／海省／距离／全部范围逐项相同；17次非空原版重建的逐次classification数与query／port记录数完全相等，补齐R184漏7次。没有跳过，7份非空零距离合法，实际已知无路0，额外限制／受限变化仍未覆盖。
- 同进程PROCESS_VM_READ生产shipping解码复制并释放读取句柄后比较：93条当前lane完整句柄／country／两端州／距离64／全部范围与最终构造记录一致；每条按同world／同batch／完整州对／距离／范围都只有1份外层选择结果相符，多义0。93条全部与原始查询方向相反，明确原版最终构造的方向转换，不能沿用查询两端顺序。此为同批值与方向的强对照，不是跨调用身份cookie／原生集合epoch／原子租约；三条历史部分海路均取得完整州／海省记录，尚未据此补画或改变权限。
- 自有93548没有市场／相机／推进输入，正常WM_CLOSE退出0。五原始保护输入保持，临时probe_access1退出后原子恢复原0；额外校验当前临时INI仍为我方预期后才恢复，避免覆盖外部编辑。扩展相关资源错误0、六条Unexpected token同基线，当前审计无游戏。未保存、提交／push；完整构建／回归／本轮实机没有未解释失败。
- 完整州选择的同次自有复制及实机对照子项完成。下一步追踪这三个实际端点的原版图形来源，并完成查询／最终lane的正式身份及world／map／request版本、访问窗口／退休更新与集合寿命；不得将采样代数充当正式generation。真实逐lane权限生产方仍未绑定，三海路末端未修；受限重算／已有lane更新、改图、GUI／原生提交与生命周期／性能／最终A01–A27继续，整体goal ACTIVE。
- 证据E:/Code/v3-analysis/checkpoints/2026-11-01-r186-port-selection/{before,delivery-before,delivery,port-wrapper-assembly.txt,port-caller-assembly.txt,native-access.log,source-change.diff,build.log,tests.log,embedded.log,debug-default.log,deploy.log,live/session-freight.log,live/owned-lanes.log,live/port-selection-audit.json,live/loaded-modules.json,live/close.json,live/after.json,three-port-records.json,config-restored.json,audit.json,NEXT.md}。原始数据与诊断ID不进入玩家地图表／运行依赖。


### 2026-11-01 — R186进行中：原版完整州选择与同次贸易返回交接

- 上轮R185是实际源码进展并已取得用户缺段修复确认。本轮继续完整目标中的海路端点绑定；新轮开始未检测到游戏进程，未关闭任何用户会话。不push、手动GUI继续。源码前备份checkpoint2026-11-01-r186-port-selection/before；当前安装仍R18508e1e433…，原版正式通行生产方与三处末端仍待。
- 汇编／原始caller再次核对1414500F0九槽void ABI：输出ProvinceSet、两个u16海省、两个完整u32运行州、距离i64、两份u32州组、country32；原版按选中海省从同次映射取完整州，caller在返回后才释放州组。新增两份自有复制器，保留完整句柄／0／重复／顺序，最终输出必须逐项等于同次嵌套贸易结果并属于对应候选组；已知无路与未知／读失败分开，非空零距离合法。原native_access_tests的复制／边界／变更／寿命／方向用例定向PASS。
- 现有默认关闭probe_access下新增自然wrapper观察代码，只转发原函数一次并保留全部九槽；TLS同次恰好一个合法贸易查询、完整country、同世界／DB／日期／访问／线程上下文和前后候选匹配后才记录完整州输出，结果仅自有诊断。国内查询记录额度96→256补足R184103次中漏7次；检查额度仍512。未接正式权限／worker／几何，不把前后观察当租约或ABA证明。
- 完整构建正在进行，新增observer尚无完整统一回归／部署或实机证据；不能继承R18558 PASS到本轮修改。后续先构建／统一检查，再在可用自有暂停会话只读核对完整州及原版最终构造方向；三海路末端和全目标继续ACTIVE。

### 2026-11-01 — R185：同市场分属州装卸修复已部署，当前新会话保留

- 用户新报分属地区道路缺段，本轮优先陆路。新反例在修复前FAIL：任意州接入点可独立装卸，货流绕过实际州内道路。新增自有landHubs，从当前typed主陆地节点role mask1复制完整控制点／省份完整运行州／region，核对原版方向、当前样条及联合复读；按同market／definition／geo共用有见证的主装卸点，进出必须走实体道路。不借州门中转、不按ID→key或最近点选枢纽；新字段进入缓存身份。这个位置是国内观察性模型假设，无主点见证区域仍保留旧接入，不称全图完成。
- 01／02／03按数据／分配／地图主题重读，沿local-reverse-engineering精确版本边界。没有增加新hook／主动查询／经济写入。输入／输出反向、缺真实道路、外市场枢纽、坏／冲突主点及同revision缓存回归进入已有view测试；typed复制／反向原生定义／锚点及region异常／源销毁进入已有domestic测试。完整build.ps1退出0、统一build_tests.ps1退出0／58 PASS／零FAIL，embedded与默认debug独立backend93064零UI顶点通过。
- 当前主体08e1e43388ece6d7e75d327a18e32dc44c46e454f725ff381991cc47af086932／50815294B，引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B；经deploy.ps1部署，dist＝安装＝自有81652实际加载，EXE SHA7f53e326…／image140000000／加载base7ff63dec0000。普通续玩、不加debug_mode、probe_access始终0。
- 同一实际输入释放读取句柄后做自有回放：531主枢纽、25420条余额的target／available逐项保持，target140740221 milli；分配84807060→84922349、未满足55933161→55817872，9424笔按州／商品独立守恒。库尔努尔91501→91504道路（仅本存档诊断标签）从0变21笔；普通完整样条48197接缝broken0。生产facts／view／ribbon函数回放中该段quantity13041700（1e5展示单位），市场0共271段／271条、马德拉斯527共44段／44条，都包含该段。不是当前GUI点击／GPU像素验收。
- 自有95032／81652全程观察phase4／暂停／date59883804，无市场／相机／推进输入，正常WM_CLOSE退出0；各自退出后五保护输入保持，扩展资源错误0、六条Unexpected token同原基线。随后检测到新进程85984（18:18:04），不视为我方测试会话，不操作／关闭／替换DLL，保留运行。未保存、提交或push。文档首归档脚本因断言“没有游戏”遇新进程而拒绝，尚未写最终记录；本次按保留新会话纠正，不把该拒绝当产品回归。
- 2331同市场／2701含跨市场诊断道路没有端点落在别的连接内部，排除首路口假设，未添加无关拆边代码。首探针误传历史goods49被拒绝，按当前catalog53重跑通过。私有PASS打印遇GBK错误不影响完整测试，最终ASCII核对58条（44冒号前缀＋14其他）。下方“R185进行中”是先前记录；失败原始文件均保留。
- 同市场分属州主枢纽装卸漏洞子项完成；用户在本次部署后明确反馈“现在没有发现这些问题了”“问题已修复”，本次州内缺段取得用户实机确认。该确认不外推所有地图／不同市场过境或最终矩阵；无真实货流不为视觉连续填假量。缺主节点见证／不同市场过境、更广场景继续。三海路末端、正式完整通行生产方、改图新字段、scope／写任务join／生命周期／性能及同构建A01–A27未完成，整体goal ACTIVE。备份／证据checkpoint2026-11-01-r185-split-state-land/{before,delivery-before,delivery,source-change.diff,before-test.log,after-test.log,build.log,tests.log,embedded.log,debug-default.log,deploy.log,live,live-final/formal-hub.log,live-final/loaded-modules.json,live-final/close.json,live-final/after.json,audit.json,NEXT.md}。


### 2026-11-01 — R185进行中：分属州装卸入口导致州内道路漏流，修复已回放

- 按用户新报的分属地区缺段转到陆路；未继续增加海路选择hook，不push。源码前备份在checkpoint2026-11-01-r185-split-state-land/before。自有95032普通续玩／probe_access0，暂停date59883804，只读复制后已正常WM_CLOSE退出0；五保护输入保持，无游戏。原版完整通行生产方、三海路末端和最终矩阵继续。
- 2331条当前同市场陆路及2701条含跨市场诊断道路均无“整条连接内部遇到另一连接端点”，排除最初路口猜测。实际模型允许净供需在任一州接入点独立装卸；库尔努尔同市场分属州内91501→91504实体道路因此货流0，并不是样条缺失。马德拉斯还存在不同市场飞地，不能据截图统一放行。
- 新增自有landHubs，从原版当前typed节点role mask1读取主陆地控制点及省份完整运行州、region关系；方向／锚点／原版几何、复读和世界检查保留。按同market／definition／geo将有见证的主枢纽作为州级净订单装卸位置，实际进出必须走实体道路；不以州门作中转。无已验证主枢纽的区域保留既有接入，仍是模型假设，不冒充原版逐笔运输；外市场枢纽不借用。身份比较纳入新字段。
- 新反例在修复前FAIL、修复后原native_trade_view_tests通过；原native_domestic_tests新增typed主枢纽复制／异常与源销毁用例通过，随后增加反向定义用例待统一入口。相同真实自有输入复制531个主枢纽，25420条供需余额与target140740221 milli保持；分配84807060→84922349，9424笔逐州／商品独立守恒。库尔努尔该段从0变为21笔经过，普通完整样条48197接缝broken0；不等于截图像素／所有分属地区验收。
- 完整native/build.ps1进行中；该修订尚无完整统一回归、部署或新DLL实载结论。旧b0a1c9c3…仍为当前安装，58 PASS仅属R184历史产品。首只读探针误传goods49被明确拒绝，按当前原生catalog53重跑通过；未把拒绝当模型失败。

### 2026-11-01 — R184：自然贸易请求／同次分类实机复制，最终州选择仍待绑定

- 上轮R183完成贸易函数第7选项与返回的隔离验证，是源码／证据进展；本轮继续真实原版数据闭环。新增CopyNativeTradeSearchInputs，完整复制两份u16候选及选项中的海峡完整句柄、海省对和u16选择值；保留顺序、重复及未知额外限制的原始数值，不把它们伪装为注册许可。界限／读失败／候选非法／复读变化拒绝，源销毁后自有；新异常用例已接原native_access_tests。
- 默认关闭probe_access下新增精确14141F1D0自然观察钩子，入口签名及R183完整七槽ABI；原函数只转发一次，保留第6临时槽及原返回值，不发起额外搜索。读取前后核对world+4500仍为同一manager、world／machine／stateDB／date／phase／access／hidden／owns／context以及完整请求／选项；要求原返回地址等于传入输出，且TLS内同次mode1分类恰好一次、country完整ID一致、分类输出与原谓词trace相符，再复制40B结果。线程局部见证按RAII恢复，全部输出为自有数值；不发布正式路线、权限批次、worker或统计，顺序观察不授予租约／ABA安全。
- 首候选648e9669…／50677026B完整构建、统一58 PASS、embedded及默认debug独立backend47472零顶点通过，部署／自有33448实载取得96份真实搜索，均caller RVA0122E6F8／build0。额度被先发生的普通国际查询耗尽，未覆盖国内重建；该覆盖不足已明确记录。正常暂停退出后，修订为普通查询检查128次／最多32份、国内重建检查512次／最多96份、每份最多256范围记录，保持有界且不给任一类借用旧数据。首候选及会话单独冻结，不将其证据混为最终构建。
- 最终完整native/build.ps1退出0，统一build_tests.ps1为58 PASS／零FAIL／退出0、embedded PASS、默认debug独立backend94436零UI顶点PASS。当前主体b0a1c9c3384373b6cfa7f28b8ed494c4bde744e3b3ead351c81fbe03b1d6531c／50679724B，引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B；经deploy.ps1安装，源码／dist／安装／最终自有93928实际模块对应，EXE base7ff63dec0000，静态EXE SHA7f53e326…／image140000000。
- 普通EXE续玩不加debug_mode，最终暂停会话实录128份：32普通＋96国内，后者来自13次重建／13个国家、caller RVA014502B1。全部phase4／date59883804／access2／owns1／context1，同一world；原返回缓冲allocator RVA0512CDC0，候选／选择／范围／距离复制通过，跳过0、分类拒绝0、构建绑定拒绝0。该批均额外限制数组为空、selectorFFFF、原始allowed为完整9项／denied空；只证明这批默认选项，不外推受限变更。4份非空原版结果距离0，未当无路或未知；本批没有已知无路场景。
- 同进程PROCESS_VM_READ探针复用生产DecodeNativeShipping，释放读取句柄后导出自有记录；93条当前国内lane的完整句柄／country／两端州／64位距离／全部范围与同次构造绑定记录逐项一致。96份国内查询按同world／同build／country／距离／范围比对：55份只有一条最终记录数值相同（54个不同lane），33份对应多条同值记录（33个不同lane），8份无最终同值记录。不选首个、不把数值相同当获胜查询身份，不据此猜方向或共享地区的具体港口。全部17次非空重建合计103次分类／查询，当前额度只记录96；剩余7次及国家156／208／465／689不在查询记录内，三缺口中的两条因此也未取得该层记录。
- 上述为真实自然贸易返回窗口复制及顺序只读对照，不是原子经济访问租约／完整写任务join、跨世界发布、主动查询分配／释放或最终州选择证明。当前查询只带选中海省，33份同值歧义表明后续须观察1414500F0原版选出的完整运行州／候选组输出，不能按country或地图地区找第一个州。三条末端仍为现有部分几何，未补猜测连接，也未启用军事权限替代商业权限。
- 自有33448与93928均观察phase4／暂停／同date59883804，无市场／相机／推进输入，分别正常WM_CLOSE退出0。每次临时probe_access1均在退出后原子恢复原0；五原始保护输入逐字节保持，扩展资源错误0、六条Unexpected token同原基线，当前无游戏。未保存进度、未提交／push。首次及最终完整构建／测试无未解释失败；诊断限额调整是覆盖修正，不是生产物流算法变化。
- 自然贸易请求与同次分类复制子项完成。下一步补原版完整州／港口选择输出与查询／构造／world／map／request版本的绑定，并给实际103次查询足够的诊断覆盖；访问窗口／结果寿命未证前不增加主动搜索。完整逐航线权限生产方仍未绑定／probe_access0，三处末端未修；当前手动GUI、改图、完整生命周期／性能及最终同构建A01–A27继续，整体goal ACTIVE。
- 备份及证据E:/Code/v3-analysis/checkpoints/2026-11-01-r184-natural-trade-search/{before,delivery-before,delivery-first,delivery,build.log,build-final.log,tests.log,tests-final.log,embedded-final.log,debug-default-final.log,deploy-final.log,native-access.log,source-change.diff,live,live-domestic/owned-lanes.log,live-domestic/trade-query-audit.json,live-domestic/session-freight.log,live-domestic/loaded-modules.json,live-domestic/close.json,live-domestic/after.json,config-restored.json,audit.json,NEXT.md}。私有原始数据／辅助探针不进入Git或玩家运行依赖。

### 2026-11-01 — R184进行中：自然贸易请求已捕获，补国内诊断额度

- 首候选648e9669…／50677026B完整构建及统一58 PASS、embedded／默认debug零顶点通过，经deploy.ps1部署并在自有33448实载。暂停date59883804中记录96份七槽自然贸易搜索及同次mode1分类，全部默认选项；首次额度均由caller RVA0122E6F8／build0消耗，未覆盖国内港口重建，不能当国内端点绑定完成。
- 自有33448无GUI／推进输入，正常WM_CLOSE退出0，五保护输入保持、原probe_access0已恢复，当前无游戏。证据checkpoint2026-11-01-r184-natural-trade-search/live及delivery-first；旧产品／原输入备份在before／delivery-before／protected。
- 正在将诊断额度分为普通查询32份与国内重建96份、检查次数分别128／512，源码已改；最终构建正在执行，尚无该修订的新增测试／部署结论。无主动搜索、正式数据／几何消费或经济修改，不push；整体goal ACTIVE。

### 2026-11-01 — R183：贸易搜索选项／返回契约隔离核验（未启用查询）

- 上轮R182已取得两份自然舰队结果及当前连接的实机对照，是源码／证据进展；本轮按完整目标转回实际贸易函数，未将军事访问当商业许可。继续沿01／02／03和local-reverse-engineering的精确版本、完整ABI与所有权门槛；验证原版贸易函数的选项／结果后，仍须补真实调用／访问窗口才能启用。
- 精确EXE SHA256 7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f／image140000000。14141F1D0汇编确认RCX=manager、RDX=返回存储、R8／R9=两份u16候选数组，栈第5槽为country32、第7槽为选项指针；旧1414500F0反编译仅列五参，遗漏了实际写入caller rsp+30的选项指针。第6槽在本体作为临时变量写入后使用，不能给其发明业务含义，也不能按旧五参形式调用。
- 当前wrapper的选项为两份24B数组：+0额外限制海峡完整句柄、+18额外限制海区省份对，+30为原样转发的u16搜索选择值（原调用默认FFFF，详细语义未启用假设）。函数先以country调用原版14141FDA0／mode1，将选项中确在allowed的海峡移到denied，将denied定义的两海省对与额外省份对合并；14141F910原版候选选择／141421820限制对筛选后，必要时走141420800受限搜索并比较原版距离值。原始分类结果不自动等于带选项查询的最终许可；后续采集不能丢掉这两份数组或假定总为空。
- 独立进程执行精确14141F1D0包装、14141F910候选选择及141421820限制对筛选机器码，11例通过：完整country／海峡句柄、mode1接线、第7选项、限制对、候选最小值／相同值保留首项、受限结果替代或失败、无路、同省零距离、64位值、第6槽任意比特不影响所测结果，以及输入字节保持／替身分配器下临时数组释放。分类器、底层缓存／受限搜索、country查找、数组拷贝和分配服务为替身；这不是实际外交／运河判定、原版分配器／TLS初始化或游戏中主动调用安全证明。
- 新增CopyNativeTradeSearchResult，仅现有native_access_tests调用；复制原版40B结果：数组头0..17、选中from／to在18／1A、64位距离20（十六进制偏移），保留原生范围记录顺序，不排序造路径。核对原请求候选、合法省份／重叠、范围／地址／数量界限、非负距离、复读变化；输出自有数值。必须先有已发生原版调用及请求／版本绑定的外部见证，初始化缓冲本身不能证明搜索执行。空范围＋FFFF／FFFF＋距离0为已知无路，与读取失败区分；非空零距离可合法，不能当未知。此复制器不自行验证country、world／map或许可。
- 新异常／方向／源销毁／端点／溢出／同点／无路及复制中变化用例进入已有native_access_tests；11份原始机器码输出保存后，在源进程退出后用同一复制器回放均一致，包含2份已知无路及1份同省零距离。统一build_tests.ps1退出0、58 PASS／零FAIL，embedded PASS。新增函数没有正式调用点，本轮未重建产品、未部署／启动游戏、无新hook或游戏进程内查询，不push。
- 私有脚本首版TLS getter少复制RET，独立进程异常；修正长度后又因只取第一条pdata范围，漏掉141421820的展开链后续代码，隔离执行失败。改用仓库已有ironpe.PE.extent，完整范围01421820..01421995后11例通过；没有修改通用ironpe或游戏。contract-first-error.log、contract-second-error.log、contract-trace.log及harness-before-unwind-fix.py保留，不将失败当PASS，不把替身服务扩大为真实原生寿命证明。
- 产品保持R182主体b85b773fe2b5f220822e37d593966b90b8998ecb2f97090da0117dcc942cbfc9／50409230B、引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B，dist＝安装；R182实际32488加载是历史同产品证据，本轮无新实机。五原始保护输入保持／probe_access0、当前无游戏；原overlay源码未变，未保存、提交／push。新复制器源码／测试不是已交付的正式查询功能。
- 贸易搜索选项／返回的隔离核验子项完成。下一步只在自然14141F1D0调用中观察完整请求、两份选项、原始分类／有效限制、返回与caller释放边界，并绑定原版所选州／港口与world／map／request版本；未取得原版访问窗口和分配释放证据前不增加主动搜索。三处末端未修，完整逐航线权限生产方仍未绑定；当前GUI／改图／生命周期／性能及最终同构建A01–A27继续，整体goal ACTIVE。
- 备份与证据E:/Code/v3-analysis/checkpoints/2026-11-01-r183-trade-path-contract/{before,before-manifest.json,trade-search-assembly.txt,wrapper-assembly.txt,selector-assembly.txt,blocked-filter-assembly.txt,trade_contract_harness.py,native-function-extents.json,trade-contract-cases.json,contract-result.log,decoder-result.log,native-access.log,tests.log,embedded.log,source-change.diff,audit.json,NEXT.md}。私有机器码／反编译及原始结果不进入Git和玩家运行依赖。

### 2026-11-01 — R182：自然舰队搜索结果观察已实机核对（正式路径仍未接入）

- 上轮R181新增普通路径自有复制器并通过原始重建输出回放，是源码／证据进展；本轮验证自然原生调用，没有将其当作商业权限或港口修复。沿用01／02／03与local-reverse-engineering的精确版本／ABI／生命周期约束。原版搜索1414043D0两参及完整0／1返回、141404330结果初始化及调用后总返回结果地址、调用者的结果释放顺序均经汇编核对；没有主动调用原生搜索／构造／权限函数。
- 精确EXE SHA256 7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f／静态image140000000；RTTI独立恢复TPdxRefDatabase<CTravelNode,7>主vtable144439800。140C59AF0以global1459667F0取当前DB、按完整节点ID的24位槽查表并复核完整ID，1406B9D20的有效性判断为ID!=UINT32_MAX；没有最近点／名称／硬编码ID→key映射。ObserveNativeFleetPathRequest只接受当前typed CFormationPathfinder／fleet kind1及typed country和两个有效节点引用，复制原版定义坐标、country和遍历选择；未知、回收句柄、范围／类型错、读失败或复读变化拒绝，输出全自有数值。
- 默认关闭的probe_access下新增1414043D0诊断钩子，精确入口签名核验；每个自然调用原样转发原函数一次，保留完整返回值。仅原版返回1、原结果初始为空且零总量、前后request一致及world／machine／DB／date／phase／access／hidden／owns／context观察一致时复制结果。最多观察前1024次调用、记录64份且每份不超512步；其他情况跳过。返回前同步复制、源指针不出调用栈、不发布worker／正式几何／权限批次。前后观察匹配不是原子租约或ABA证明。
- 新请求复制的完整句柄、country、typed DB／node／definition、零节点、坐标、源销毁、未知类型／回收／越界／读失败／地址溢出及复制中变更用例加入原有native_access_tests。完整native/build.ps1退出0；统一build_tests.ps1为58 PASS／零FAIL／退出0，embedded PASS、默认debug独立backend PID4980零UI顶点PASS。R181的7组重建机器码为此前证据，本轮没有把它们重记为新原生搜索执行。
- 当前主体b85b773fe2b5f220822e37d593966b90b8998ecb2f97090da0117dcc942cbfc9／50409230B，引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B；经deploy.ps1安装，dist＝安装＝自有PID32488实际双模块，EXE运行base7ff698bc0000。普通EXE续玩、不加debug_mode，临时probe_access1记录2份自然fleet搜索（thread80460／return caller RVA00D73139，原版结果均1，access2／owns1／context1）：country37，3392→3393／connection3841／distance=cost34949705；country201，3415→3416／connection3971／distance=cost37277911。均一条普通边，没有本轮长路径／自定义段实机覆盖。
- 随后在同暂停存档使用PROCESS_VM_READ独立核对这两份自有日志记录与当前typed国家／节点DB／节点定义、源节点原生邻接表和完整连接句柄；原版两端指针方向、定义端点指针／固定坐标、+60距离逐项一致，两边均runtime kind5／definition kind2，总量与记录之和一致。组合复读未见变化。此为自然军事舰队结果及顺序只读当前对象对照，不证明贸易权限相同、不等于三条shipping lane的实际端点绑定，也未证明主动调用的分配器／访问窗口／释放ABI。
- 观察phase4／暂停／date59883804保持，无市场／相机／推进输入。自有32488正常WM_CLOSE退出0，五保护输入与测试前原始字节保持；临时probe_access1在退出后原子恢复原ini／probe_access0。扩展资源错误0、六条Unexpected token同原基线，当前无游戏。未保存进度、未提交／push。delivery-before冻结R179双DLL及部署脚本，delivery冻结当前R182交付；备份不是自动回滚指令。
- 自然搜索结果的受限观察子项完成，正式collector／海路仍未接新结果，三处港口末端未修复。下一步补实际端点请求与结果／地图版本绑定、商业通行语义、访问窗口与原生结果寿命后才可正式消费；完整逐航线权限生产方仍未绑定／probe_access0。当前手动GUI、改图、完整生命周期／性能／最终同构建A01–A27继续，整体goal ACTIVE。
- 源码前备份、汇编、RTTI、全部构建／回归／实机与恢复证据E:/Code/v3-analysis/checkpoints/2026-11-01-r182-native-search-observer/{before,before-manifest.json,node-rtti.json,node-resolver-assembly.txt,search-entry.txt,wrapper-assembly.txt,caller-assembly.txt,native-access.log,build.log,tests.log,embedded.log,debug-default.log,deploy.log,source-change.diff,live/verified-paths.json,live/session-freight.log,live/loaded-modules.json,live/close.json,live/after.json,config-restored.json,audit.json,NEXT.md}。私有原始机器码／反编译／存档数据不进入Git和玩家运行依赖。

### 2026-11-01 — R181：原版路径结果机器码核验与自有复制器（未接正式调用）

- 上轮R180通过动态普查定位三处缺口为配置出口／实际港口海区错配，属于取得新证据的进展；本轮沿该证据验证原版路径结果，未缩小完整交付目标。重读01／02／03及local-reverse-engineering技能，复用既有测试与原生发现入口；最低通过标准为精确EXE重建输出／顺序／方向与自有解码一致、异常或变更拒绝，未取得权限／scope／寿命证据前不启用。
- 精确EXE SHA256 7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f／image140000000中，独立进程执行141405EF0原始路径重建、141407FB0追加及141403CE0坐标读取机器码。7组覆盖完整连接／节点ID、零节点、反向、三边顺序、64位距离／费用独立、零距离、自定义段与同点空路径，均通过且合成输入保持。使用合成前驱图和预分配输出，未执行原版搜索、权限、构造分配器或真实游戏对象寿命；没有以替身验证这些内容。
- 新增native_path_probe.h，仅由现有native_access_tests包含；CopyNativePathResult复制原版已成功精确节点搜索的自有数值结果。按原版64B布局取完整连接／from／to及固定坐标、独立距离／费用；核对预期端点、每步连续性、总数／容量／地址界限、非负／64位溢出及总账，复读头与全部记录后返回。输出不保留引擎指针。UINT32_MAX自定义转移显式拒绝，不画直线；空结果仅接受同一完整节点和坐标且零总量。复制器不验证对象数据库、国家权限或世界租约，尚不能用于正式几何。
- 异常用例加入已接统一入口的native_access_tests：缺字段／读失败、句柄方向、同ID异坐标、超量／溢出、负值／总账不符、复制期间头／内容变化、源销毁、自定义段及同点空路径。另将7份实际机器码输出原始记录保存为二进制，在原进程退出后用同一复制器回放，5份普通结果精确通过、2份自定义结果按预期拒绝。完整build_tests.ps1退出0、58 PASS／零FAIL；verify_embedded退出0。新增头不进入产品链接，无需重建或部署双DLL。
- 独立PE调用扫描补齐Ghidra空调用表：原版141404330初始化结果、调用1414043D0后始终返回结果地址，不能将非空地址当搜索成功；后续自然调用观察须在实际搜索返回点核验原版成功值。141401E90、140D72D80及140D87950为其他实际搜索调用者，国家舰队构造引用在1411304D0／141130760；仅静态候选。R180的基础权限恒true与舰队军事访问差异继续有效，不用其绕过商业权限。
- 两份产品保持R179主体f53f81d65f1058b9a066ba2b48e4ce8159fa16765f36fc5d89bce4728db588e0／50222428B、引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B，dist＝安装。没有启动／操作游戏、没有新增native调用／hook、未保存／提交／push；五保护输入与R180退出后基线逐字节一致，原版probe_access0保持。统一入口的独立D3D／加载器harness不是当前游戏实机验收。
- 结果重建／普通路径复制子项完成，三处海路末端仍未修复。下一步验证自然搜索调用的类型／国家／参数／成功值、结果所有权与访问窗口，才能谈正式端点接入。逐航线完整通行批次生产方仍未绑定；当前UI、原生scope、改图、生命周期、性能及最终同构建A01–A27继续，整体goal ACTIVE。
- 源码前备份与完整证据E:/Code/v3-analysis/checkpoints/2026-11-01-r181-native-path-result/{before,before-manifest.json,reconstruction_harness.py,reconstruction-result.log,reconstruction-cases.json,native-result-0.bin,decoder-native-result.log,native-access-result.log,tests.log,embedded.log,original-search-callers.json,reconstruct-assembly.txt,append-assembly.txt,audit.json,NEXT.md}。私有机器码／反编译／输出只留外部checkpoint，不进入Git或玩家运行依赖。

### 2026-11-01 — R180：港口出口错配普查与原生舰队寻路候选（未启用）

- 本轮只做当前数据只读核对及精确EXE静态分析，未修改产品源码／DLL、未构建／部署、不提交／push。当前主体仍R179 f53f81d65f1058b9a066ba2b48e4ce8159fa16765f36fc5d89bce4728db588e0／50222428B，引导71dae36d…／779879B；R179统一58 PASS为同产品历史证据，本轮没有新增整套PASS。R177港内路径与R179物理可达性已在生产使用，本轮发现的寻路候选仍未启用。
- 独立PROCESS_VM_READ探针复用生产解码器，在同一EconomyReadWindow内复制当前州、港口／海路／样条／naval_exit定义并复核窗口；释放读取句柄后分析自有数据。624份运行州港口绑定中13份、共6个物理地区的配置出口海区与实际港口所在海区不同。当前三条不完整lane218103887／218103899／218103903均配置出口属于成员、实际港口海区不属于成员。动态普查不使用ID→key地图表；上述句柄只是本次证据标签。外部顺序读取仍不构成原子原生访问租约。
- naval_exit模板索引来自当前typed CStateRegionTemplateDatabase；141572DE0汇编以JBE接受index==bound，故+F60是包含端点的最大索引。私有初版误用排他上限，漏四份州，保留port-census.log；修正后的port-census-final.log为最终624份记录，未将此新字段接入产品。1425E8450→1425E8690→1425E8070原版成员边界调用传入成员地区，非外侧邻居；没有可直接改成peer的静态证据。三个当前外侧直连key存在且第一省份对未命中海峡定义，不等价原版选择／通行许可，未补画外侧海段。
- 精确EXE 7f53e326…／image140000000确认NTravelNetwork::CPathFinder构造141403EF0、搜索1414043D0和结果重建141405EF0；基础vtable权限回调为常量true，不能用于绕过原版权限。CFormationPathfinder国家构造140D6CD80／vtable1444A0438的type+168=1由FLEET_CONNECTION_CHARTED_COST_MULT原版常量引用独立确认；通行回调140D6CE50同时涉及海峡谓词140BE7C10及军事地区访问14140B890，不能把军事语义自动当贸易许可，+190遍历选择也不能与type混用。
- 原版搜索参数+40为结果数组，+58原版距离／+60费用；重建候选每条64B：connection+0、fromNode+4、fromXY+8/+10、toNode+18、toXY+20/+28、distance+30、cost+38（偏移均十六进制）。这些是静态候选，尚无真实调用／结果生命周期验收；构造器涉及引擎／TLS分配，不主动调用、不挂新钩子、不向worker交指针。下一步须验证原版country／mode／访问窗口、结果方向、分配释放及寿命后，才讨论末端几何接入。
- 自有正常续玩PID7468实际加载对应双DLL，观察phase4／暂停／date59883804；无市场／相机／推进输入，正常WM_CLOSE退出0，五保护输入逐字节保持，扩展资源错误0、六条Unexpected token同基线，当前无游戏。证据及文档前备份E:/Code/v3-analysis/checkpoints/2026-11-01-r180-native-port-access/{before,audit.json,port_census.cpp,pathfinder-rtti.json,formation-pathfinder-rtti.json,pathfinder-callers.json,naval-exit-assembly.txt,trade-render-boundary-assembly.txt,fleet-cost-define-assembly.txt,live/port-census-final.log,live/loaded-modules.json,live/close.json,live/after.json}。
- 港口出口错配调查子项完成，三处末端仍未修复。真实通行批次生产方仍未绑定／probe_access0；当前UI、原生scope、改图、生命周期、性能及最终同构建A01–A27继续，整体goal ACTIVE。

### 2026-11-01 — R179：物理可达性前置分配，断路不再冒充已满足

- 先复现州级连接把不相连的港／道路节点视为同一终端，导致不可达廉价货源抢走真实替代供给；before-test.log为预期失败。国内模型现按当前实体land／localLand／原版shipping／港内路径建物理节点图，再调用既有最小费用最大流；删除分配后补路和缺转接占位。节点保留当前market／definition／geo及完整控制点，未知地区仍按完整运行州隔离；没有跨市场、最近点或ID→key表，没有新原生调用。
- 沿用州级订单聚合假设：州的净供需可从它已验证的接入节点装卸，不定位建筑内部。AllocateFreight可选自有stateAccess以独立有向收／发门共享该州供需上限；这些门不能形成中转捷径，最终route仅保留物理边。原单终端／旧图入口保持。无实际路径留下供给／未满足；已知原版海运和港内连接仅缺绘图时保留数量／不完整几何；同一已证物理点可有合法零距离货流。零距离、Freight目标／满足／未满足与原生1e5数据仍分开。
- 新增断开中转、收货／供货州不能跳节点、实际替代供应、恢复接驳、共享供给上限、残余重分配最优、空图／空接入、越界／重复／缺失／冲突接入、同点零距离及缺控制点但已知连接用例，均进入原有freight_tests／native_trade_view_tests。旧反向测试改为检验TravelEdge方向与strip方向组合；旧端口再生／混合筛选夹具原本依赖缺接驳仍满足，先验明断路未满足，再补实际路段测试原交互。after-test.log／after-test-2.log保留这两次旧预期失败，after-test-3.log及完整入口通过。完整构建、统一58 PASS／零FAIL／退出0、embedded PASS、默认debug关闭零UI顶点PASS。
- 当前主体f53f81d65f1058b9a066ba2b48e4ce8159fa16765f36fc5d89bce4728db588e0／50222428B，引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B，经deploy.ps1安装，源码／dist／安装／自有79444实际模块对应。普通续玩暂停date59883804，只读复制后关闭读取句柄，在同一自有输入上运行R178模型与新模型，25420份州／商品目标及可用供给逐项相同；国内目标140740221 milli，满足94128826→84807060，未满足46611395→55933161，差额9321766转为未满足，并非删需求或清零原版贸易。最终shipment9275→8842，1344份余额分配变化；市场0国内量23523099→14210265 milli。
- 私有探针持久化8842笔最终货流与25420份余额，独立逐州／商品汇总输入输出，所有供应上限、需求上限与守恒通过。20笔零距离货流复核当前相同definition／geo，非虚造线路。已知完整几何38838接缝broken0，伦敦344次港内连接broken0；只排除40次原版部分海路腿，非海路不完整0。3条原版部分海路为218103887(10→207，使用8次)、218103899(922→936，32次)、218103903(611→608，0次)，合计40，继续修复。初版私有报告按数据库槽位枚举lane却按完整ID排序的模型下标取腿，标签错配；修正诊断排序后最终记录正确，生产排序未变。初版physical.log保留，不引用其错误逐航线标签。
- 上述为外部顺序只读观察及自有重放，不是原子原生访问租约／当前GUI可见验收。未操作市场面板／相机／推进，不能沿用R178的界面数量作为新构建可视证据。单次外部计算新125ms／旧47ms只是此次重放，不满足渲染P95／模拟速度性能矩阵。世界／通行批次正式生产方仍未绑定，probe_access0；海峡权限、3处末端、改图、生命周期、性能及最终同构建A01–A27继续，整体goal ACTIVE。
- 自有79444观察始终暂停同日期，正常WM_CLOSE退出0，save／settings／ini／content／continue五输入字节保持，扩展相关错误0、六条Unexpected token同原基线，当前无游戏。未保存进度、未提交／push。首次私有freight测试编译误列仅有头文件的freight_ribbons.cpp，更正命令后通过；完整产品构建及统一入口无未解释失败。
- 源码及原R178两DLL部署前备份、失败／通过和逐笔证据E:/Code/v3-analysis/checkpoints/2026-10-31-r179-physical-domestic-allocation/{before,before-manifest.json,source-change.diff,before-test.log,after-test.log,after-test-2.log,after-test-3.log,freight-final.log,build.log,tests.log,embedded.log,debug-default.log,deploy.log,live/final-physical.log,live/loaded-modules.json,live/close.json,live/after.json,audit.json}；目录按开始备份日期命名，实机验收为11月1日。


### 2026-10-31 — R178：跨地区真实道路转接，缺段1395→556次

- 继续用户分属地区缺段报告，R177暂停存档只读分类为83组州／端点共1360次缺转接，另35次既有部分海路腿；其中839次可沿当前同市场道路绕行邻州。旧转接只查单地区，遗漏已存在的跨地区实体连接，没有按距离／名称造线或扩大市场权限。
- 转接节点改为(当前market／definition／geo，完整控制点)，未知地区仍按完整运行州隔离。纳入已验证kind2／3／4的当前同市场跨地区land，按原版方向把两端绑定各自地区；只有实体连接才允许跨区。不同市场、无连接的异地区重号点、未知地区不自动合并；无新海路／新ABI。绕行距离／费用／工作量入最终shipment，州级原分配数量与Freight需求账保持。更正NativeDirectedStrip的bit63及complete注释，与R177港内几何及缺转接事实一致。
- 跨地区用例先失败(before-test.log)后通过，并覆盖原版定义反向。扩大转接读取暴露旧夹具把rail key低位1绑定strip低位0，after-test.log保留失败；修正夹具768→769，不弱化生产校验，final-test.log及统一入口通过。完整构建、统一58 PASS／零FAIL／退出0、embedded PASS、默认debug关闭零UI顶点PASS。
- 主体fc6744227ae3ea510da72842cde61ca3dfbf7c2a2f7884c4a04e3fdc8ae9c8bd／50721811B，引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B，经deploy.ps1安装，dist＝安装＝自有78692实际模块。前后暂停／date59883804只读复制并关闭读取句柄后重放：69组／839次使用从缺段集合消失，其余14组／521次逐字段保持，无新缺口；总1395→556，另35次仍来自3条部分海路。937陆连接／93海连接／9275 shipment和市场0国内quantity23523099 milli保持，不是吨数。
- 按有向原始控制点复核已知完整路径58810个接缝，broken0，明确跳过556次不完整腿；伦敦564次港内连接仍broken0。此为外部顺序读取后的自有几何重放，不是原子原生访问租约、当前GUI逐像素验收或性能矩阵。本轮没有市场面板／镜头操作，不把实载当全界面验收。
- 余521次在当前已采集且允许的地面／港内图不可达：state227=16、232=10、356=5、547=453、852=4、1053=33。232／547合计463次涉及另一市场的港内端点，不因此自动放行；另58次继续查原版有效连接。当前仍州级先分配再补物理转接，缺路径保留已分配量，不等价完整物理节点优化；真正不可达与仅缺几何需区分，并将可达性约束前移。源／终端州内路径、海峡正式收集、改图、寿命、性能及最终A01–A27继续，整体goal ACTIVE，不称缺货／断路全部闭环。
- 自有54032／78692均正常WM_CLOSE退出0，观察暂停同日期；五保护输入save／settings／ini／content／continue字节保持，扩展相关错误0、六条Unexpected token同基线，当前无游戏。未保存进度、未提交／push。私有探针首编误加-municode导致wWinMain链接失败，去除后成功，不涉及产品构建失败；未启用kind0／1或虚拟节点新路径。
- 证据及源码前备份E:/Code/v3-analysis/checkpoints/2026-10-31-r178-transfer-gap-audit/{before,source-change.diff,before-test.log,after-test.log,final-test.log,build.log,tests.log,embedded.log,debug-default.log,deploy.log,gap-baseline.json,audit.json,live/gaps.log,live/raw.log,validation/seams.log,validation/loaded-modules.json,validation/close.json,validation/after.json}。


### 2026-10-31 — R177：原版无key港内路径已接入生产／绘制，伦敦564次控制点连续

- 按R176证据实现NativePortTransfer：只接受当前kind5／definition type5／keyMAX且typed节点角色为陆港1与海军8／9／10、两端同当前region、陆港省份完整运行州与州E78一致的原版连接。复制完整连接ID、原版方向anchor、固定起点及原版[累计距离,X,Z]路径；数组／类型／角色／地区／省份／路径端点与距离校验和outer reread均保留。其他kind5不作为陆路放开，无新引擎调用、无硬编码地图表。
- 模型按同地区同市场转接图接入港内路径，端点通过与原版坐标同一floor量化校验后对齐当前精确样条控制，内部点保留原版路径。复制几何使用自有bit63命名空间并保存在NativeDomesticModel，正式ribbon读取自有路径，不修改原生SplineDB。sea筛选包含港内接入段；距离来自原版末记录1e5单位，费用系数1是模型假设，数量仍按最终货流一次，Freight需求账不递归／不重复。
- 初始港内接入用例先失败后通过；新增角色／省份／地区／数组／负距离／端点错误及源销毁用例、正反向路径／真实距离／正式ribbon用例通过。补货运引擎指纹中的localLand和portTransfers；同revision路径变化用例先失败(cache-before.log)，修复后进入统一套件通过。完整构建、统一58 PASS／零FAIL／退出0、embedded PASS和默认debug零顶点PASS。
- 主体732a7e5e3bbf254830f8bb64374d73f08de97310e174c226614beec78f0baa7e／50666726B，引导71dae36d…／779879B，经deploy.ps1安装，dist＝安装＝实际73120模块。普通续玩／暂停／date59883804中，外部只读生产解码及复制后重放取得372条港内路径、全部有绘图几何、最终使用1282次，伦敦使用564次。逐有向控制点核对伦敦564次，broken0；缺转接由R175的2642次降到1395次。顺序外部观察不是原子原生快照，数量改善不等于其他1395次全部已修复。
- 正式市场页及伦敦海陆流线实见(london-flows／london-port-close.png)，卡片26483.90517／铁路510.99 MU保持；港口附近原版旗标遮挡部分局部画面，564次端点连续的精确结论来自只读重放，不把截图当逐像素接缝证明。第一批UI脚本首次前台guard拒绝点击，PowerShell仍继续后续步骤；截图核对后改为每步退出码守卫重执行，未将首次步骤当成功验收。
- 自有73120全程观察暂停／同日期，正常WM_CLOSE退出0；五受保护输入字节保持，扩展相关错误0、六条Unexpected token与既有基线一致，无游戏。未保存进度、未提交／push。原版港内连接接入子项完成；其他缺转接、源／终端州内路径、海峡正式权限、新地图／寿命／性能及最终A01–A27仍继续，不宣称全部道路断点消失。
- 证据E:/Code/v3-analysis/checkpoints/2026-10-31-r177-native-port-transfer/{before-test.log,cache-before.log,after-test.log,build.log,tests.log,embedded.log,debug-default.log,deploy.log,live/continuity.log,live/loaded-modules.json,live/london-port-close.png,live/close.json,live/after.json}；源码前备份before。

### 2026-10-31 — R176：找到原版无样条key的港内连接及实际路径

- 延续港口断点调查，当前源码／两DLL仍R175 e20043d3…／71dae36d…，本轮无产品代码／构建／部署变化、不push。Ghidra确认当前victoria3.exe／image140000000，独立PE精确SHA7f53e326…核验；R175完整58 PASS仅作为同产品历史证据，未重复整套测试。
- 当前原版splnet没有8390108→1503连接；旧gen_splines.py按geo后缀＋locator／gap阈值生成短桥，不能直接用于当前运行地图。进一步独立PE RIP引用扫描及.pdata边界找到141580FF0：陆路后缀3映射角色1，海军同region陆地区域映射角色10，分别写CTravelNodeDefinition+40完整anchor、+58当前region指针、+60角色位集；保留当前+44省份和+48／50原版固定坐标，不建立硬编码ID地图表。该静态语义由汇编归档交叉核对，未主动调用引擎。
- 只读探针在正常暂停会话54216枚举6112个当前CTravelConnection，复读字段并检查当前世界后释放进程句柄。实际372条港内连接均kind5、definition key UINT64_MAX、path_count1；陆路角色bit1与海军bit8／9／10共享当前region。伦敦原版连接为8390108(role400)→1503(role2)，definition type5，原版起点固定坐标407200000／277900000、终点407200000／278100000，路径第一／末记录均[200000,407200000,278100000]。这是引擎当前保存的实际连接／路径，不是最近点或自造直线；外部顺序观察仍不是原子原生scope证明。
- 修正调查中的窄角色掩码：初版仅含bit8／9，漏掉此处沿海角色bit10，报告bridge0；按141580FF0核对后加入bit10，372条全部匹配，初版与最终port-scan日志均保留。1412A0910及141729250跳表证明definition type2–6走统一运行kind5分支，因此kind5不能统一解读成不可用；也不能因本病例而把全部kind5作为陆路。生产必须单独识别同region的真实港内连接，验证两个typed端点、当前省份／州／市场、路径与角色，再复制自有几何。
- 自有54216仅正常续玩、全程观察暂停／date59883804，没有界面输入／推进／保存；正常WM_CLOSE退出0，五输入字节保持，扩展相关错误0、六条Unexpected token与基线一致。实际模块与安装／dist匹配。归档反编译首尝试超过Windows命令长度206而未执行，随后用编辑工具写入外部静态证据成功；Ghidra xrefs／字符串空结果不作为无调用／无字符串证明。
- 本单元完成港内连接来源定位，正式断点仍未修复。下一步将该原版路径单独接入原子快照／自有几何／转接及距离统计，补异常、跨市场／分属、路径变化与回归，再实机验收伦敦海路→中英格兰连续。禁止仅按keyMAX、kind5或固定角色后缀造线。证据E:/Code/v3-analysis/checkpoints/2026-10-31-r176-port-node-relation/{static-decompilation.json,node-builder-assembly.txt,node-vtable-refs.json,connection-predicate-table.json,port_probe.cpp,port-audit.json,live/port-scan-final.log,live/loaded-modules.json,live/close.json,live/after.json}。

### 2026-10-31 — R175：同地区同市场分属州的物理转接，剩余断点继续

- 用户截图补充分属地区仍割裂。R174按完整运行州隔离localLand，导致相同物理地区内另一个分属州的可用道路不能进入转接。新增同地区同市场分属州用例先失败（before-test.log），再修复。按当前market＋region definition＋geo三元身份组织转接，沿用完整运行州作为订单身份；无地区见证退回精确本州，未知不授权共享。没有写死州ID→地图表。
- 转接同时纳入当前同地区同市场的跨运行州land连接，可以实际跨分属边界再返回，而非把两个逻辑州强制合并。不同市场／不同地区／未知定义用例拒绝补接；保持原数量账，路径仍为现有可用样条，缺线继续不完整。新增两段跨属地实体路段用例验证最终四腿路径。经济分配仍为原有州级观察模型，不把该模型转接宣称原版逐笔贸易路线。
- 完整构建及统一58 PASS／零FAIL／退出0，embedded PASS。当前主体e20043d34ec5c1ef0d051fcec89ffcfa16a50ba1751a949c9852561b074d6b5d／49780394B，引导71dae36d…／779879B，dist＝deploy.ps1安装＝实际26228模块。原有未提交源码保持本地，不push；没有本轮重新执行独立默认debug测试。
- 同一原存档／date59883804／暂停，独立只读复制后模型重放识别60组同市场分属地区；转接腿使用16373次、缺转接2642次，R174同存档观察为15690／2936。外部顺序复制不是原子原生快照，计数改善不等同所有路径修复。经原版地点查找器定位海得拉巴，再打开英国市场物流页，印度道路流线及26483.90517／510.99 MU卡片实见（india-flows.png）；镜头范围和色彩不同于用户截图，未逐断点同镜头对照，不将其当全部可视验收。
- 自有26228会话观察始终暂停／同日期，正常WM_CLOSE退出0，原save／settings／ini／content／continue五输入字节保持；扩展相关资源错误0，六条Unexpected token与基线一致。当前无游戏。测试未保存推进、未修改游戏经济。初始Ghidra复核025E8070只说明原海路的海军锚点规则，字符串搜索没有返回，不据此建立海陆锚点关系或启用新ABI。
- 本单元修正转接范围过窄，但仍有2642次缺转接（非唯一断点数），伦敦海军／陆路港口位置差异尚未解决；其他不连续场景、真实港口关系、海峡权限和最终矩阵继续。证据E:/Code/v3-analysis/checkpoints/2026-10-31-r175-split-state-transfer/{before-test.log,after-test.log,build.log,tests.log,embedded.log,live/split.log,live/india-flows.png,live/loaded-modules.json,live/close.json,live/after.json}；源码前备份before。

### 2026-10-31 — R174：州内陆路转接补入最终路径；伦敦海陆锚点仍待连接

- 用户报告海路到伦敦周边再到中英格兰时箭头断开。静态定位DecodeNativeDomesticSources丢弃同州道路，BuildNativeDomesticModel以整州节点分配，已选各段的物理端点不一定相接。新增海路→州内道路→跨州陆路用例先失败（before-test.log），不是仅按截图猜测。
- 采集localLand保留当前已验证可用的同州type2／3／4连接，单独纳入内容身份，不制造自发自收货流；模型按连续两段的真实进出控制点，在该州当前连接上用同费率选择州内转接段并缓存。最终shipment插入有向自有样条，数量及目标／满足账保持，新增距离、费用、工作量计入shipment。无当前道路时插入不完整标记、保留已有片段及数量，不造直线。该转接是国内观察模型，不称原版逐笔已选路径。
- 新端点逐段连续及缺道路用例、原版逐行解码器的同州道路／铁路保留用例通过；原有混合方式用例自身无州内连接，调整为明确断点的三段路径断言，原有数量／筛选仍通过。完整构建和统一58 PASS／退出0、embedded PASS；主体9a795e01572823c0995709d7210d50aba4d37b1002a7e69c209d5f6a0794775e／49592982B，引导71dae36d…／779879B。经deploy.ps1安装，dist＝安装＝实际71336模块。本轮没有重新执行独立默认debug测试，不沿用旧构建结论。
- 当前存档独立只读复制后关闭句柄的重放取得1394条州内连接，模型最终货流使用15690次转接腿，另2936次缺合法转接；这是顺序外部观察，不是原子原生快照或完整可视验收。伦敦运行州5的四条localLand对应控制点1500–1504，实际使用0次。海军港口锚点8390108坐标4072.53589／2779.91675，陆路端点1503为4072.21387／2781.49634，既非同一控制ID也非同坐标；因此新增州内路径仍不能单独接通用户的伦敦海陆例子。禁止将最近点／硬编码ID绑定或随意直线升级为正式连接。
- 实机仅核对正常加载与生产采集，没有完成伦敦连接可视验收。自有71336始终所观察日期59883804／暂停，正常WM_CLOSE退出0；save／settings／ini／content／continue五输入字节保持，相关扩展资源错误0、六条Unexpected token与既有基线一致。当前无游戏；用户不push要求继续，未提交／推送。
- 下一步查原版当前港口的海军锚点与陆路港口节点之间的实际关系／几何，完成该端点后再对伦敦→中英格兰实际连续路径验收。海峡权限等原有未完成项继续，不能宣称本Bug全修复。证据E:/Code/v3-analysis/checkpoints/2026-10-31-r174-land-continuity/{before-test.log,after-test.log,build.log,tests.log,embedded.log,live/continuity.log,live/anchors.log,live/loaded-modules.json,live/close.json,live/after.json}；源码前备份before。

### 2026-10-30 — R173：恢复构建／统一回归及正常存档加载，当前会话保留

- 用户明确恢复正常启动游戏测试，覆盖此前资源占用期的离线／不测试限制；不push要求继续有效。本轮未继续GUI队列改码，先验证R169–R172累计改动。完整native/build.ps1退出0、统一build_tests.ps1为58 PASS／零FAIL／退出0，新增完整航线绑定、快照邮箱及会话退休用例均进入原有套件并通过；verify_embedded和test_debug_default也退出0。默认关闭ImGui的独立实际backend记录零顶点。未发生需修复的编译或测试失败。
- 经deploy.ps1安装，当前主体c82435daf55b0a00f7fbcb1a374a6e5c0d0492f4d44c05f842f022c9f9a8ec99／48822550B，引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e／779879B，dist＝安装。正常EXE续玩入口、不加debug_mode，实际PID39984加载两份对应DLL；初始phase4／暂停／date59883804。原ini保持probe_access0，未强启未验证的生产方。
- 初始市场0物流页截图实见原版贸易＋国内模型量26483.90517、已铺铁路510.99 MU、逐商品数量／占比及地图线，生产日志显示owner10／frame1／220条ribbon。此为当前构建加载及既有正式路径回归，不是原生完整通行批次已接入：正式overlay仍未提供新批次，海峡故障、受限重建、已有航线更新及全生命周期矩阵继续。
- 随后的悬停／关闭步骤期间，实际日期推进至59885904且暂停状态在0／1间变化，截图相机、面板及日志owner也变化，来源未确定；本轮脚本没有主动解暂停或推进命令。停止进一步自动输入／退出，保留仍在运行的39984，不把这段混合操作视为受控关闭重开／悬停不改统计／性能验收，也不声称正常退出。关闭截图实际为海区面板，不能按文件名当作成功关闭复现。
- 截至只读audit，原存档／settings／ini／content／continue五输入哈希均与启动前一致，两份dist／安装／实际模块一致，采样error.log中扩展相关错误0；会话未结束，这不替代最终退出后检查或所有游戏错误对照。原存档保护基线仍8008fc75…。未修改经济、未保存进度、未提交或推送。证据E:/Code/v3-analysis/checkpoints/2026-10-30-r173-offline-changes-validation/{build.log,tests.log,embedded.log,debug-default.log,deploy.log,audit.json,live/loaded-modules.json,live/logistics.png,live/session-observations.jsonl}。
- 当前目标继续：真实原生收集／退休事件驱动、GUI排队写入和原生提交边界仍待，三处港口末端／改图／性能／最终同构建A01–A27未完成。后续可继续只读核对和源码研究；对当前疑似同时操作的会话不争夺输入、不强制关闭或替换DLL。

### 2026-10-30 — R172：批次退休接入会话／帧／统计校验（未构建／未测试）

- 按用户最新要求继续仅本地离线修改，不构建、不运行测试、不操作游戏、不部署、不push。上一轮邮箱检查是源码进展，本轮延续到已交worker及地图的会话结果。所有R169–R172改动仍未经编译／执行，不继承R167历史PASS。
- LogisticsSession在适配层保留自有原子代数及期望值，worker仍只收到原有不可变数值快照。SetSnapshot拒绝过期或不完整见证；HasEquivalentSnapshot要求同token／同代数，避免相同内容跨集合寿命跳过重新绑定。Overlay将采集结果的见证沿PublishRealEconomy传给会话；旧无见证路径保留。
- SnapshotIsCurrent、FrameIsCurrent、StatisticsAreCurrent及Frame／Status增加代数复核，退休后可在下一次相应检查时拒绝已有结果，不等待UI合并。Poll在提交工作前撤下退休快照／帧并取消worker；消费回复及发布文档前再次复核，原有owner选择保留，当前替代快照可恢复相同面板。未将批次退休冒充世界变化，未改国内分配、国际数量或游戏经济。
- 在已接统一入口的logistics_session_tests写入待执行用例：真实worker产出后的帧／统计退休、相同数字不同token、同内容定时采集、退休后替换恢复、未完成回复拒绝、不完整见证和世界退休不可绕过。用例尚未运行，静态阅读与diff空白检查不是并发／渲染／UI验收。
- 原生收集器仍未填充批次及驱动失效token；已有航线更新、受限重建、首次绑定前退休／ABA、GUI已排队写入及原生提交边界的完整失效时序仍待核对。原子检查与后续使用之间不是全链事务，不宣称所有竞态消失或海峡已修复。备份E:/Code/v3-analysis/checkpoints/2026-10-30-r172-session-lane-retirement/before；资源可用后先构建并执行相关统一回归。

### 2026-10-30 — R171：逐航线批次的邮箱发布／取出失效检查（未构建／未测试）

- 本轮延续只离线改代码、不测试、不push要求。前一轮完成可选采集入口，属于源码进展；本轮补其返回后到邮箱和UI交接间的失效检查。未编译、未运行测试、未操作游戏、未部署／推送；当前修改仍全部保留本地，没有新增PASS。
- 将R170任意current回调简化为共享自有atomic代数见证；输入同时匹配world／revision／generation，生产方必须单调推进代数、冻结批次，不得恢复旧值或跨集合寿命复用token。NativeCaptureResult仅在适配层保留token及期望代数，worker的EconomySnapshot继续只含数值副本。
- Publish先保留既有请求身份和版本顺序检查，再拒绝已退休批次并通过既有错误通道报告；TakeError和Take复核未取副本，NativeCaptureMatches复核已取的保留副本。迟到旧版本在触及当前结果前拒绝，避免旧失败清除新结果；无批次见证的历史调用仍按原路径处理，缺一半见证则拒绝。
- 已在统一入口现有native_snapshot_tests写入待执行用例：发布后未取即退休、取出后退休、返回到发布间退休、错误检查到取出间退休、新版本重试、旧结果不清除新结果、缺token／缺代数及源释放后的自有存储。只做文本审阅，尚未执行这些用例，不能宣称并发或生命周期验收。
- 正式原生收集器尚未绑定新输入，也尚未驱动这些token；首次绑定前退休、已有航线更新、受限重建、已发布worker／原生帧即时撤回及全部竞态仍需完成。顺序原子读取不是全链事务，不以本轮邮箱校验替代这些要求；现行海峡显示故障未验收。备份E:/Code/v3-analysis/checkpoints/2026-10-30-r171-lane-epoch-publication/before；资源可用后先构建并执行R169–R171相关回归。

### 2026-10-30 — R170：采集入口接入自有逐航线批次（未构建／未测试）

- 延续用户仅离线改代码、暂不测试及不push的最新要求。本轮只改源码与记录，未编译、未运行测试、未启动／操作游戏、未部署／推送；上一轮是源码进展，本轮继续推进可选交接入口。两轮源码均未取得新增PASS，现有DLL保持历史产物。
- NativeLaneAccessInput仅在适配层持有不可变自有批次、独立采集代数及只读自有epoch的current回调；世界／版本／代数不匹配或缺少有效性见证即拒绝。CopyNativeSnapshot新增默认空的可选输入，采集前、绑定时和全部复制完成返回前核对有效性；按本次复制的航线及海峡注册表执行R169完整绑定，worker只获得复制后的逐航线允许列表，不获得回调或引擎指针。
- 明确拒绝同时传入逐航线批次与旧国家权限回调，避免混用两个时点的语义。旧调用点参数不变，正式overlay仍未提供新输入；未启用任何未经验证的原生函数，现行注册回退仍未修复。持有shared_ptr不延长批次的有效代数，生产方必须冻结批次并独立维护失效事件。
- 在既有native_snapshot_tests增加待执行用例，覆盖世界／版本／代数不匹配、见证缺失／退休、源释放后自有所有权、零身份、冲突来源，以及无效批次在读取引擎前拒绝。仅文本审阅；整份成功采集及复制中失效路径尚缺执行证据，不把入口拒绝用例等同完整快照验收。
- 下一步仍需原版创建和已有航线更新的正式收集、首次绑定前退休／ABA、受限重建与当前集合代数；返回后到邮箱发布的批次退休、当前帧撤回也未闭环，不能只凭本次返回前复核启用。资源可用后先构建并执行R169／R170相关回归，再补真实生产方接线及最终验收。源码前备份E:/Code/v3-analysis/checkpoints/2026-10-30-r170-snapshot-lane-access/before。

### 2026-10-30 — R169：逐航线通行结果交接与消费代码（未构建／未测试）

- 用户本轮要求仅离线改代码，暂不测试以保留资源；随后明确不需要push。本轮未编译、未运行任何测试、未启动或操作游戏、未部署、未推送，改动保留本地。现有dist／安装未由本轮更新，R167的58 PASS不适用于新增源码。
- 新增native_lane_access.h，将既有自有分类／已选航线值类型从诊断头移入共享头；BindNativeLaneAccess按独立传入的world／revision／collection generation、完整lane句柄、国家、两端州、64位距离及全部省份范围匹配当前副本。完整允许／拒绝集合必须无重复且恰好覆盖当前海峡注册表；缺失／重复／过期或同句柄内容变化整批拒绝。输出为按完整航线句柄索引的自有允许列表，不写死地图身份、不按国家借用其他航线的结果。
- NativeDomesticSources新增可选laneStraitAccess，国内几何消费优先使用逐航线结果；字段存在但缺某条非空航线时拒绝，不回退国家表／注册表。该字段加入NativeTradeEquivalent，权限变化可使缓存不等价；国内模型数量口径不变。原生采集器尚未填充新字段，现行注册回退仍存在，本轮不宣称海峡已修复。
- 在已接统一入口的native_access_tests与native_trade_view_tests写入待执行用例：世界／版本／代数不匹配、句柄与港口／距离／成员变化、源销毁、完整分类、同国两航线不同结果、缓存失效与缺记录拒绝。只做文本审阅，未执行复现或回归，未取得任何新增PASS。
- 交接函数不证明引擎访问租约或原子性；未来生产方必须从独立当前采集状态取得expected，并在发布前再次确认。原版创建／已有航线更新的完整收集、首次绑定前世界退休、受限重建、批次寿命与正式采集接线仍待；不得把诊断采样代数直接当正式集合代数。备份E:/Code/v3-analysis/checkpoints/2026-10-30-r169-lane-access-handoff/before；下一步待资源可用后先构建并运行相关回归，再继续生产方接线。

### 2026-10-30 — R168：按用户要求同步当前开发源码到GitHub

- 用户明确要求更新完成后push。核对现有main／origin及docs/GITHUB.md的既有公开上传范围，使用已有NoFires SSH远程，不新建仓库、不改写历史。fetch确认基线与origin/main一致后，将自前次发布以来的物流源码、测试、GUI／shader与记录整理为源码提交a688e34c709423a7c56ffece7ab309f1edd744dc：90文件、10466增加／199删除；其中包括当前R167默认关闭的分类／最终航线创建观察与回归。
- 提交前核对范围、文本文件与高置信凭据模式、索引内容及git diff --cached --check；dist、native/build、日志、存档、外部v3-analysis与本地凭据不进入提交。Git原有autocrlf给出LF→CRLF提示，未执行checkout或批量改写源文件。编译产物仍按既有规则独立交付，不作为Git源码上传。
- git push origin main成功，随后git ls-remote返回a688e34c709423a7c56ffece7ab309f1edd744dc，与本地HEAD完全一致，工作区干净；该成功发生在本记录之前。本记录及GitHub说明作为后续文档提交同步，最终远端核对留在外部checkpoint。当前产品仍R167的a400dd88…／71dae36d…，统一58 PASS为同产品证据；本单元只改记录，不重跑无变化的产品测试，不宣称正式海峡接入或完整目标完成。
- 现行约束已改为每次完成更新后核对测试和提交范围并推送已有远程，同时保留源码修改前checkpoint备份。后续继续原版结果的世界／请求交接、失效与正式几何消费。证据E:/Code/v3-analysis/checkpoints/2026-10-30-r168-git-sync/以及R167的git-candidate-audit.json／git-staged-check.txt。

### 2026-10-30 — R167：完整通行分类绑定原版最终创建航线的实机观察

- 核对原版141419DA0→14140DF60→14140FAA0：反编译器漏列创建包装函数第九参数，汇编确认其为原版距离值，再按引用交给原始构造器；最终州方向按构造器的实际from／to参数保存，不按候选遍历方向猜测。独立进程执行精确EXE的14140DF60原机器码，10例覆盖完整州ID、反向／零州、64位大距离、类型与无效返回；构造器、有效性和引擎服务为替身，不是实际分配器／数据库／寿命证明。
- 默认关闭的probe_access新增14140D830国内重建调用范围和14140FAA0九指针构造器观察。只转发自然原函数一次；同一嵌套线程范围内收集完整分类，国家／模式不符、同次重建分类变化、复制失败或采样超限即拒绝。构造前最终输入、返回的完整句柄对象和重建结束时当前类型化DB对象三次核对国家、完整州、原版距离和全部压缩省份范围；不选候选、不按country跨调用复用权限、不保留指针到worker、不发布正式结果。
- CopyNativeLaneSelection新增到既有probe头，源销毁、完整句柄／端点顺序、距离64位／零／负、非法与变化范围、复制中变化等用例接入已有native_access_tests。完整构建后复核增加异常／超限时整次失效及当前对象类型检查，按相同参数重编overlay并经ResourcesOnly守卫重链；最终统一58 PASS／零FAIL／退出0及embedded PASS。当前主体a400dd882243ccc0c8e439ba4324c74b9bb4bedbe2aa37049cfbc633624db3bb／48590180B，引导71dae36d…／779879B；源码、dist、安装及实际49244模块对应。
- 同一原存档会话49244记录185条不同完整lane句柄、17国家、33次重建代数，全部matched=1且绑定拒绝0：date59883804有93条，后续date59883912有92条。原版英国lane218103823/from5/to547/distance447262046在generation1创建；后续generation286为lane234881040/from5/to551/distance469386716，两者省份集合也不同。这将孟买→西孟加拉的变化绑定到同进程实际原版创建结果，未强制选港。当前记录均为mode1的九条允许／零拒绝，尚未验证受限状态下的最终创建绑定。
- 外部独立只读lane探针构建成功，但执行返回4，随后确认49244已不存在；日志有世界退休、未见新crash目录，退出原因及退出码未知，不能称本轮由我方正常WM_CLOSE退出，也不能称完成外部逐航线复核。会话后续存在UI发布与日期推进，仅记录原生日志事实，不把未知操作者或这些操作外推为交互／性能验收。finish核对五输入保持、扩展相关资源错误0、六条Unexpected token与基线一致；最终恢复原ini SHA256bbc678fb…／probe_access0，当前无游戏。
- 本单元完成原版最终航线生成边界的观察核对，仍未将诊断数据交给正式几何。世界／请求原子交接、旧代数拒绝、已有航线更新与受限重算、三处港口末端及最终矩阵继续；不宣称海峡故障已修复。证据E:/Code/v3-analysis/checkpoints/2026-10-30-r167-native-lane-binding/{before,caller-assembly.txt,create-assembly.txt,constructor-assembly.txt,create_harness.py,creation-harness-result.txt,build.log,incremental-build.log,tests.log,embedded.log,after.json,live/binding-records.json,live/session-freight.log}。
- 用户最新要求完成更新后push到Git。现有仓库main跟踪origin/main，远程为NoFires/victoria3-logistics-system，fetch后基线0 ahead／0 behind；修正本文件旧“无Git”约束，仍保留源码前checkpoint备份。按既有公开范围整理此前未提交源码、测试和记录，排除dist／native/build／日志／外部私有证据；实际提交与远端验证单独记录。

### 2026-10-30 — R166：原版完整海峡分类输出的自然调用复制与实机开关核对

- 在既有默认关闭的probe_access下增加原版14141FDA0观察钩子，按精确EXE及13字节入口签名核验后转发原始四参调用一次；借用数组仅在返回后、调用方尚持有时复制。新native_strait_probe.h保留完整句柄及顺序、排除调用前已有前缀，限制长度并复读描述符／内容；重复、交叉允许拒绝、非法句柄、不可读和观察到的变化均拒绝。线程局部逐调用trace与同次140BE7C10自然返回逐项比较，异常离开通过RAII恢复前级trace。无主动权限调用、无指针交worker、无正式权限发布。
- 生产复制器的异常输入与源销毁测试加入已有native_access_tests统一入口。完整构建后首轮58 PASS；复核补上异常trace恢复／地址边界与hidden复核，按相同编译参数重编overlay并经ResourcesOnly守卫重链，最终统一58 PASS／零FAIL／退出0、embedded PASS、默认debug零顶点通过。当前主体23b812d88ea7d92a6374cf91f9b7422e3cbad8cb7ae9e90a57fb878a368faf51／48381144B，引导71dae36d…／779879B；源码、dist、安装及实际23560模块对应。
- 第二独立暂停会话23560中，通过原版tag/date与博斯普鲁斯面板构造未保存的限制和恢复，接受377份完整分类、283国家、14线程；全部caller RVA0141F236／mode1，数组与同次自然谓词返回精确一致，复制拒绝0。47国家均有同world／同完整海峡1的允许→拒绝→允许；英国country1在classifier sample5／6862／9709、日期59883804／59888160／59892864分别为[9允许,0拒绝]→[8允许,1拒绝]→[9允许,0拒绝]。不是仅看UI控制位。该人为日期测试不代表正常模拟速度或所有运河建筑／外交分支。
- 288份观测为access2／owns1／context1，另89份为access2／owns0／context0；原版分类的自然调用具有不同线程／上下文，不能据此授权快照线程主动调用。完整分类复制仍不等于世界／请求原子交接或航线绑定；诊断map只作有界变化日志，不能直接充当正式权限缓存。国内注册回退及三处港口末端未修复，下一步在原版选路／航线生成边界绑定完整结果与world／revision／generation并验证过期拒绝。
- 首局67280的快捷脚本未改变日期，截图未建立预期海峡状态，三段audit实际FAIL；该局只接受283份初始分类，不能当开关成功。稍后进程在明确close前已消失，日志有世界退休、未见新crash目录、Application/WER查询无返回条目，但退出原因／退出码未知。第二局改逐步确认控制台、tag和日期，完成上述三段，并暂停WM_CLOSE正常退出0。两局finish均核对五输入保持，最终恢复原ini SHA256bbc678fb…／probe_access0；扩展相关游戏资源错误0，六条Unexpected token与既有基线一致，不是全部debug错误完全一致。当前无游戏。
- 证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r166-native-classification-live/{before,build.log,incremental-build.log,final-tests.log,embedded.log,debug-default.log,source.diff,after.json,live/second/classification-audit.json,live/second/session-freight.log,live/second/restricted.png,live/second/reopened.png,live/second/close.json}。原版完整分类复制子项完成，正式海峡修复及最终交付仍继续。

### 2026-10-30 — R165：原版海峡分类输出与追加语义的隔离验证

- 在独立进程核验精确EXE SHA2567f53e326…，复制原版14141FDA0分类机器码及140BE7C10／140BE7E10权限机器码，保留相对布局；合成世界、海峡、国家、关系／有效性替身和预分配输出缓冲区。原版分类输出R8为允许、R9为拒绝，保留完整32位句柄及原版遍历顺序；36例覆盖三海峡六种排列、两种模式与三个国家（含带代数句柄），10例底层权限对照也通过。输入逐字节不变，输出计数与哨兵边界符合预期。
- 额外验证原版分类函数会保留已有输出前缀并追加，每次调用不清空：连续两次调用正确累加；空注册表不触碰原有输出。正式采集若假设其输出天然替换或复用旧数组，将混入旧结果；实际调用方14141F1D0先构造空数组的现有静态证据与此一致。该fixture容量足够，未走原版分配器增长；外交／条约和建筑依赖仍为替身，不是完整原生ABI／实际游戏寿命证明。
- 进一步静态核对14140D830调用141419DA0的stack参数与胜出结果到14140DF60构造链，指出绑定当前航线须同时携完整州、路径与距离，不能只按country缓存零散谓词；尚未安装这些新钩子或接正式权限结果。现有正式国内注册回退问题继续。
- 首次独立夹具将浮点常量地址误写为过大RVA，VirtualAlloc断言在执行原版代码前拒绝；修正到0486ADB0及对应页后全部通过。无游戏操作、产品源码／DLL／构建／部署变化，当前仍8d2f0d92…／71dae36d…；R164的58 PASS仅历史同产品证据。下一步在原版合法调用边界复制完整分类并绑定航线，验证世界／请求代数与一致性，再进入正式几何消费。
- 证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r165-native-classification-output/{classification_harness.py,result.txt}。本单元是隔离输出语义验证，不是实机海峡修复或全部目标完成。


### 2026-10-30 — R164：原版海峡函数完整允许→拒绝→允许实机时序

- 修正默认关闭的probe_access观察器：记录某国家／海峡／模式的最新返回变化，移除相反旧返回的去重项，并将事件总数限制8192（仍限150000次自然调用／4096组合、忙锁丢样）。不主动调用原版、不保存引擎指针、不把诊断结果发布给正式物流。完整构建、统一入口58 PASS（含R161换港回归）、embedded PASS；当前主体8d2f0d9291765da8091f688ee84620027b02226eb0f3e8a947a604400ad2a6ac、引导71dae36d…，dist＝安装＝实际48556加载。
- 两个独立暂停测试会话均使用游戏自带tag/date命令和原版博斯普鲁斯面板：切TUR，日期1836-07-20后设置封锁／仅控制国，日期1837-02-01后重新开放；所有改变仅在未保存的诊断会话。最终48556中，同英国country1、海峡完整句柄1、mode1，自然原版返回在sample46为1、92204为0、126825为1，脚本断言三段顺序通过。这是原版实际调用证据，不是合成权限回调或仅看UI字段；不推广到运河建筑、所有外交分支或快照线程主动调用安全。
- 第一会话54560首个前台guard拒绝按键，而旧PowerShell批次仍继续后续动作，未构造出预期状态；检查截图后使用逐步check=True脚本恢复。该局只取得1→0，第三段未记录，不作成功证据。随后正常关闭并复用同构建新会话48556，以受控脚本完成全部三段；driver.log与两次trace保留。未经验证的首次操作不当作模型／权限变更验收。
- 两个自有会话均暂停WM_CLOSE退出0；finish核对原存档／设置／临时ini／content／continue字节保持，结束后恢复原ini SHA256bbc678fb…／probe_access0。扩展相关错误0、六条Unexpected token与基线一致；不宣称全部debug日志相同。本轮未验证该新构建正式地图／性能、原子结果收集或寿命，完整交付继续。
- 原版函数的真实权限翻转子项完成，正式物流仍用旧注册回退、未接入诊断数据，海峡显示故障尚未修复。下一步验证自有结果收集与世界／请求代数、同原生完成点交接、过期拒绝和性能，不能将诊断集合直接升级为正式权限表。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r164-strait-return-transition/{build.log,tests.log,live/second/return-trace.json,live/second/session-freight.log,live/second/after.json,live/second/driver.log}。


### 2026-10-30 — R163：受限海峡原版自然调用的真实允许→拒绝

- 沿用当前e65d7a42…主体／71dae36d…引导，临时probe_access1及-debug_mode，在自有暂停会话通过原版console切TUR，通过原版博斯普鲁斯海峡面板操作。原存档海峡已有非军事状态及冷却至1836-07-11，初次封锁点击被拒；经help date核对游戏自带日期命令后，仅在未保存会话将日期设1836-07-20，再通过面板设置封锁／仅实控国。此为人为构造的诊断状态，不是正常模拟推进或生产经济改动。
- 第一会话18820取得面板状态但没有拒绝诊断记录，正常退出后以同构建新会话71424重新执行；第二局自然原版mode1结果中，当前海峡完整句柄1／定义4先有284国允许，后有47国拒绝，47国均观察到允许→拒绝，包括英国country1；控制国country95有允许、未见拒绝。首次拒绝sample87002，未外部主动调用引擎；原生调用本身有真实受限分支证据，但不代表异线程主动调用安全或原子采集。
- 外部顺序只读字段在受限态为[2,3,0,0]，marker0；随后日期命令改至1837-02-01，面板重新开放，字段恢复[0,0,0,0]。重复允许结果被现有诊断去重，未取得重新开放后的第三次函数返回trace；不得宣称完整允许→拒绝→允许函数时序验收。只读脚本首次误将read返回的单值当四元组，TypeError后改逐字节读取成功，未写进程。
- 两个自有暂停会话均WM_CLOSE正常退出0；finish核对原save／settings／临时ini／content／continue字节保持，最后恢复原始ini（probe_access0、bbc678fb…）。扩展相关错误0，六条Unexpected token与基线一致；不是全部debug日志无变化的结论。未保存日期、国家或海峡修改，无产品源码／构建／部署变动，未重跑统一入口。
- 当前自然结果尚未接正式物流，现有注册即允许缺口仍未修复；下一步基于真实拒绝病例验证结果收集、世界／权限代数及提交一致性，保留ABI与调用窗口保护，继续运河建筑分支、末端几何及最终验收。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r163-strait-transition/{transition-audit.json,live/second/session-freight.log,live/second/restricted-controls.json,live/second/reopened-controls.json,live/second/restricted.png,live/second/reopened.png}。


### 2026-10-30 — R162：无扩展原版复现印度初始港口重算

- 在确认用户游戏已退出后，通过deploy.ps1 -Uninstall卸载当前双DLL，正常启动当前续玩存档。实际进程71508仅加载C:/Windows/System32/VERSION.dll，无freight.dll，证明此次对照未加载扩展。原版暂停初始date59883804，lane218103823/country1/from5/to547（孟买）；此存档已由用户推进并保存，SHA2568008fc75…，不同于R159初始fecc16f…，不混作同文件。
- 先推进到59883834仍为孟买，后推进至59883930观察到原版lane234881040/from5/to551（西孟加拉）；这复现了无DLL条件下的初始孟买→后续西孟加拉重算。端点改变发生在两次观察之间，不能精确断言具体重算tick，也不能据此证明DLL采集毫无额外延迟；但该现象不依赖扩展，不能将初始孟买本身归因DLL写入。外部探针只读身份枚举，内部路径selector／权限为测试回调，不作为真实选路或原子快照证明。
- 私有会话助手首次SetForegroundWindow失败，后AppActivate与AttachThreadInput也未获得前台，守卫拒绝发键；改为Alt激活前台后操作成功。失败的首次advanced-audit仍处原日期，不当作已推进。全程仅操作自己会话，暂停后WM_CLOSE正常退出0，未保存推进。
- 经deploy.ps1恢复主体e65d7a4296e90c1e17f4edeaa7709d36bb9f32e36f7650b3c961f7c67e628437和引导71dae36d…，安装＝dist。五输入（save/settings/ini/content/continue）字节均保持，本轮无产品源码或构建，未测试恢复后再次实载；当前无游戏。R161换港发布回归已通过，当前原版端点选取不作强制干预；下一步关注真实额外延迟、海峡权限和末端缺几何。调查子项完成不等于完整交付或初始化所有问题关闭。
- 证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r162-stock-port-refresh/{live/stock-modules.json,live/initial-audit.txt,live/advanced2-audit.txt,live/week-audit.txt,live/close.json,after.json}。


### 2026-10-30 — R161：原版港口换点的生产缓存与发布回归

- 在现有 native_trade_view_tests.cpp 增加端到端换港用例：保持同world／同revision／同商品订单与市场，旧国内lane直接接旧物理港口，新lane更换完整句柄与目的港，通过当前陆路继续到相同需求州；新旧港口几何同时保留在输入，避免通过删资源伪造旧线消失。
- 验证初始帧确含旧港口；仅lane变化即可使NativeTradeEquivalent不等价、生产FreightEngine重建facts；同最终数量的新帧只采用新港口并消除旧港口贡献。正式LogisticsSession拒绝等价快照判断，SetSnapshot后旧FrameIsCurrent立即false，worker完成后的新帧落在新港口。最终专项测试编译与整份native贸易视图／会话套件退出0、PASS；test-result.txt留存。首次通过后补强初始旧线存在断言并重跑通过。
- 本轮只改测试，无产品源码／DLL／部署变化，仍为e65d7a42…主体及71dae36d…引导。用例位于已接统一入口的既有测试中；用户16276游戏仍在运行，未执行会部署的完整build_tests.ps1。该隔离回归不证明原版为何初始孟买、不证明采集30秒节流的实际延迟，也不证明无DLL原版行为；只完成自有快照进入生产缓存／worker／发布后的换港子项。后续仍需原版重算时机与采集／发布时间线、海峡权限、三处港口末端及最终验收。
- 未操作用户游戏。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r161-port-retarget-regression/{before,native_trade_view_tests.exe,test-result.txt}。


### 2026-10-30 — R160：用户推进后印度原版航线改为西孟加拉

- 用户补充初始显示孟买、等待游戏时间推进后变为西孟加拉，并确认当前显示正常。对用户正在运行的16276仅执行外部PROCESS_VM_READ，不操作、暂停或关闭游戏；当前原版国内lane201326608/country1/from5/to551，当前州551为STATE_WEST_BENGAL。R159另一个初始续玩会话原版lane50331663为from5/to547（STATE_BOMBAY）；两次不同进程、不同模拟阶段的观察不能证明同一原子变化或DLL因果，但证明变化不仅是显示标签，原版航线身份与端点本身不同。
- 当前生产日志已记录revision6／9053模型货流和235可见路线，用户确认从西孟加拉出发；只读模型重放同为9053笔。外部比较器的selector／权限是测试回调，不代表原版选择调用或正式绘制逐边trace。国内lane完整内容已参与缓存身份比较，现有运行采集有30秒节流；是否造成该场景额外滞后、初始为何保存孟买、无扩展原版是否同样延迟，仍需同存档对照，不宣称初始化Bug已修复或DLL无因果。
- 不将孟买长期端点判为预期最终行为，不硬编码切到孟加拉。本轮无产品源码修改、构建或部署；当前用户会话保持控制权。后续复现先保存原版重算时间／lane代数、DLL采集与正式发布的时间线，确认哪一层滞后再修改。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r160-port-refresh/audit.txt，以及R159初始audit.txt。整体goal继续。


### 2026-10-30 — R159：英国市场印度港口端点报告的原版对照

- 用户指出英国市场印度海运画到葡萄牙孟买，预期孟加拉；随后确认原版航运线路面板也列孟买，并询问是否理论上应为孟加拉。当前原版 GUI sea_region_panel.gui 使用 ShippingLane.GetEndState；静态原版渲染 1425E90F0→1425E8450 取 CShippingLane+48／+4C 两州，现有解码读取同字段，不宜硬改到特定地区。
- 在自行启动的普通续玩进程 176 中，独立 PROCESS_VM_READ 比较器读取当前州定义 key、owner、market、center 及航线：STATE_BOMBAY 546 的 owner30/market11，547 的 owner218/market0；两者同地区不同运行身份。原版国内 lane50331663/country1/from5/to547，未把546当原生端点。本机孟加拉州16／551也在market0，但当前航线未以它们为此连接端点。比较器的路径selector和权限是测试回调，身份枚举不据此证明实际选路／原子性；日志中9038模型货流是隔离重放。
- 该证据排除了直接用“孟买名称／地图地区”认定端点属于葡萄牙，也说明不能按首都直觉改为孟加拉。原版141419DA0静态链比较候选州对成本，具体本存档为何胜出仍未运行追踪。共享地区的实际地图港口落点是否跨入葡萄牙控制省份仍待独立核对；残缺端点的旧逻辑仍是另一待处理问题。本轮未改产品源码、DLL或部署。
- 先前R158中断会话未完成海峡开关验收，未将其称成功；继续前确认无游戏并将临时freight.ini恢复原始bbc678fb…／probe_access0。R159取证后进程176已在收尾前结束，退出原因／退出码未取得；随后发现新进程16276/start11:45:52，非本单元启动，不操作或关闭。finish检查因存在新游戏而拒绝，不能声称本单元正常WM_CLOSE或完成五输入退出审计。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r159-india-ports/live/{audit.txt,coverage-probe.cpp,launch.json}。


### 2026-10-30 — R157：原版海峡结果对当前国内国家的覆盖核对

- 将默认关闭的 probe_access 海峡诊断扩展为最多前 150000 次自然调用、最多 4096 个国家／海峡／模式／结果组合的去重记录；观察锁忙时丢样，不阻塞等待，不保存指针或向正式快照发布。统一入口 58 PASS、verify_embedded PASS；主体 e65d7a4296e90c1e17f4edeaa7709d36bb9f32e36f7650b3c961f7c67e628437，引导 71dae36d…，源码／dist／安装／本次实际模块一致。
- 当前暂停存档、同一进程 69436：诊断日志记录 284 个国家与 9 条海峡共 2556 对，均为 mode1／允许，最后首次记录样本序号 35558。独立外部只读比较器复用当前解码器，读取 17 个国内航线国家及 9 条当前海峡，153 对全部在诊断记录中出现。比较器的选路和通行回调均是测试值，只用于枚举身份，不是原版权限结果或真实选路；日志与外部读取为顺序观察、并非原子快照。不能外推其他存档／国家、世界重载、权限改变或拒绝结果。
- 独立比较器首次链接缺 freight.cpp，第二次缺 log.cpp，补齐后编译通过；记录原始失败过程。本轮只观察暂停初始存档，未实测当前构建正式物流页面或性能。测试会话正常 WM_CLOSE 退出 0，存档／设置／内容／continue SHA256 不变；临时诊断设置已恢复原始 freight.ini SHA256 bbc678fb…／probe_access=0。扩展相关错误 0，六条 Unexpected token 与基线一致，未证明全部日志完全相同。
- 本结果支持继续验证自然结果收集，但不是证明可安全缓存：仍需原版国家／海峡完整句柄验证、同步边界、权限变化、旧世界失效和性能。正式权限回调未启用，受限海峡显示及三处港口末端尚未修复。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r157-native-coverage/{tests.log,live/coverage-probe.txt,live/coverage-audit.json,live/session-freight.log,live/after.json}。


### 2026-10-30 — R156：原版海峡判断自然调用频率实机观察

- 沿用已安装 R155 主体 a341e425…；按该 DLL 的 nm 符号和 PE ImageBase 得到诊断调用计数器 RVA 0x2bc350，先核验完整 DLL SHA256 与实际模块，再以外部 PROCESS_VM_READ 读取，无附加、暂停线程或写进程。该数字仅用于此精确构建的诊断，不写入产品映射。
- 同一自启存档进程 76216 在 phase4/暂停/date59883360：02:55:35.918 为 42333，02:55:52.357 为 51216，02:56:11.436 为 61512，02:56:17.025 为 64518。各次读数是顺序观察；暂停下持续增长表明前 32 条日志远非全部自然调用，但不证明每个国家／海峡已覆盖，也不证明哪一条结果与快照原子一致。
- 之后以速度 1 短暂推进并恢复暂停：date59883360→59883384，02:56:27.387 计数 86496；此区间包括界面操作和暂停，不能当稳定帧率或单一模拟速度基准。大量调用意味着正式被动收集必须限制额外开销，并验证国家覆盖、权限变化与世界退役；不能直接复用诊断日志作为权限缓存。
- 自有进程正常 WM_CLOSE 退出 0，未保存推进；原存档、设置、content_load、continue 的 SHA256 不变，临时 probe_access=1 已恢复原始配置 SHA256 bbc678fb…／0。扩展相关错误 0，六条 Unexpected token 与基线一致；未证明全部日志一致。本轮无源码／DLL构建或部署，沿用 R155 58 PASS；海峡问题、港口末端和最终交付仍未完成。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r156-permission-cadence/live/{count.py,counts.jsonl,session-observations.jsonl,session-freight.log,after.json}。


### 2026-10-30 — R155：原版海峡判断自然调用的实机诊断

- 在 overlay.cpp 的既有 probe_access 调试开关下增加默认关闭的观察钩子：仅转发游戏已发起的 140BE7C10 调用一次，限前 32 条采样；核对精确版本及函数入口字节，不主动调用、保存引擎指针或把结果用于物流。构建后统一入口 58 PASS、verify_embedded PASS；主体 a341e42542373b64bd2511d1bdcc438e4da7036ec86574eac0e269a1383e5d50，引导 71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e，dist＝安装＝实际加载。
- 暂停原存档的自启会话 71040 中，原版线程 9848 自然调用 32 次，caller RVA 141FE52、mode 1，涉及两个国家和九条注册海峡；样本均 marker=0、控制字节全零且返回允许。前后顺序观察均 phase4/access2/owns1/context1，同世界同日期。此会话快照复制线程为 68512，与样本线程不同；同一标志不证明完整调用期、同步、对象寿命或复杂分支可在快照线程安全运行。
- 诊断启用期间市场物流页和商品值可见，暂停会话 WM_CLOSE 正常退出 0，原存档、设置、内容与 continue 输入字节不变；临时 probe_access=1 的配置在退出后恢复原始 SHA256 bbc678fb…／probe_access=0。扩展相关错误 0，六条 Unexpected token 与既有基线一致；不是完整错误日志零差异证明。
- 本次真实调用仅为直接放行样本，不覆盖已建运河、拒绝、外交或条约，正式权限回调仍未绑定，海峡显示问题与三处港口末端仍未修复。下一步采集受限及状态变化时原版判断和其寿命，并验证无额外调用的完整结果收集／失效方案。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r155-natural-permission-probe/{tests.log,build.log,live/probe-audit.json,live/session-freight.log,live/market.png}。


### 2026-10-30 — R154：当前候选双 DLL 实装及原版物流页实机核对

- 经 deploy.ps1 部署现有已通过 R150 离线 58 PASS 的候选，游戏进程 58312 实际加载主体 23914d6503a31fee5ca4521436ae019b46a0ff6ae83028228a8a263b1cc95d66 和引导 71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e；源码未改，本轮未重跑完整测试。之前安装主体 178e039d… 已备份于 before/，当前源码对应的 dist 与安装两份 DLL 哈希一致。
- 自启原存档保持暂停、日期 59883360；实际原版市场物流页显示贸易量 25963.19216、铁路 510.99 MU 与逐商品数据。日志记录 1147 州、53 商品、3192 原版选择、918 陆路／93 国内海路、9038 模型货流、3 条部分海路和 218 可见箭头；这不证明每条几何完整或海峡权限正确。截图及该会话日志留存 live/。
- 自己的暂停会话经 WM_CLOSE 正常退出 0；原存档、设置、freight.ini、content_load.json、continue_game.json 的 SHA256 均未变。游戏日志中未检出 ironroad／freight／native-gui 相关错误，六条 Unexpected token 与既有基线一致；未逐项证明其他日志行完全一致。
- 该测试验证本候选一次启动与当前市场页，不代表海峡权限、港口端点、完整生命周期或 A01–A27 通过；国内权限回调仍未绑定，不能宣称海峡修复。当前无游戏。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r154-candidate-live/。


### 2026-10-30 — R153：国内原版搜索的附加限制参数核对

- 复核本机静态 EXE SHA256 7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f。国内调用链 1414500F0 在常规直线路径将两个临时数组清空，将其组成的结构指针作为第七参数传给 14141F1D0；后者先以 mode 1 调 14141FDA0，按 140BE7C10 的返回值分类，再将第七参数第一数组中的海峡从允许组移入拒绝组。汇编证据分别为 domestic-call-assembly.txt 和 search-classification-assembly.txt。由此可知该国内调用的空附加数组不会额外移除已允许海峡；不外推其他调用者或全部初始化分支。
- 这是静态参数核对，不代表最终实际选中哪条路径。原版判断仍依赖运河建筑、国家关系和调用时上下文；140742920 可初始化 TLS 并获取锁，相关真实生命周期与访问范围仍未证明，正式权限回调仍禁用。国内现有注册回退仍可能错误显示受限海峡，三处港口末端仍缺，不能称修复完成。
- 本轮未改产品代码、构建、部署或操作游戏。下一步围绕原版在真实航线生成期间的自然调用时机与权限数据生命周期取证，不能拿空附加数组或模拟测试替代完整权限和路径验证。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r153-domestic-route-arguments/。


### 2026-10-30 — R152：原版权限两层调用的隔离组合验证

- 在独立进程校验精确EXE 7f53e326…后同时执行原版140BE7C10与140BE7E10原始机器码，以合成对象和替身提供两岸国家、类型有效性与从属关系，未接触游戏进程。10例覆盖同一限制下本国／外国、完整国家ID、两种模式、模式相关限制，以及分别只在一侧施加限制；输入逐字节未变。替身不执行真实外交、条约或建筑查询。
- 汇编复核14141F1D0在调用14141FDA0前设置DL=1，后者向140BE7C10传原版海峡、国家和该模式；这明确国内路径生成链中该调用的参数模式。静态分析与合成对象验证不证明实际调用期、原生对象寿命、TLS／锁、外交条约分支或线程安全；真实回调仍未绑定，海峡权限问题仍未修复。x64dbg桥状态查询超时并终止等待，没有附加或暂停游戏。
- 本单元未改产品源码、构建、部署或启动游戏；候选dist仍R150的23914d65…，安装仍R147的178e039d…，R150同候选58 PASS为历史测试。三处港口末端及最终验收继续。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r152-native-permission-chain/{chain_harness.py,result.txt,mode-assembly.txt}。


### 2026-10-30 — R151：只读存档数据上的完整快照交接模拟

- 自主暂停会话74536使用外部PROCESS_VM_READ探针复用当前CopyNativeSnapshot生产源码：仅在探针内模拟访问门标志、路线selector及全允许／全拒绝权限回调，复制当前存档数据形成world1／revision1和2的两份自有结果；17个国家、模拟权限612次，两结果内容身份不同。第三次在回调内模拟请求退役，整份复制被拒绝。
- 两份结果均在关闭进程读取句柄后计算国内物流，9038笔货流逐项比较来源、目的地、市场、商品、数量及每条模型边方向完全相同，总量91297724 milli-orders。原版selector并未调用，测试对多候选故意返回首项，仅用于交接，不是合法选路证明；权限也为测试值。外部顺序观察和模拟访问标志不构成引擎访问租约或原子快照，不扩展为真实函数、实际权限／海峡修复或港口验收。
- 实机实际模块仍已安装R147主体178e039d…；候选dist仍R150主体23914d65…，未部署。会话WM_CLOSE正常退出0，五输入字节未变、扩展相关错误0、Unexpected token基线一致。本单元未改产品源码／DLL，沿用R150同候选58 PASS历史。原版140BE7C10真实调用依赖、权限变化／旧世界并发、三处港口末端及最终验收继续。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r151-owned-snapshot/{live_snapshot_probe.cpp,live/snapshot-probe.txt,live/close.json,live/after.json}。


### 2026-10-30 — R150：权限结果变化的复制期复核

- R148权限采集桥此前只判断一次；合成回调在海峡控制位不变时由允许变拒绝，旧代码实际FAIL。现在在同一调用方提供的访问范围内复核所有国家／海峡结果，允许→拒绝、拒绝→允许、第二次不可用均拒绝整个结果；保留已知拒绝和未知的区别。所读字段复读与世界退休检查仍保留。
- 两次观察不能证明未读取的外交／条约对象未改变或ABA安全，也不证明原版函数无副作用、锁等待或完整ABI；回调仍未绑定真实游戏，额外调用的实机性能未测。完整合成世界的CopyNativeSnapshot成功链、真实权限变化与三处港口末端仍待完成。
- 完整构建、统一入口58 PASS／零FAIL及embedded通过。候选dist主体23914d6503a31fee5ca4521436ae019b46a0ff6ae83028228a8a263b1cc95d66未部署／实机；安装仍R147的178e039d…，源码／dist／安装不一致。未操作游戏，现有生产海峡行为未改变。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r150-permission-consistency/{before,build.log,full-tests.log,record.py}。


### 2026-10-30 — R149：权限采集桥接入快照交接，生产回调仍关闭

- CopyNativeSnapshot增加可选权限回调；仅在现有完成命令访问门内、国内及原版贸易输入复制之后调用R148采集桥，失败则拒绝整个快照，成功将自有国家权限表随同世界／revision交给R147模型。默认不传回调，现有游戏入口未绑定原版函数，实机行为未切换。
- 原有native_snapshot_tests加入无效访问范围和退役时权限回调不得运行的断言；R148隔离测试覆盖合成允许／拒绝／变化，R147模型测试覆盖国家结果与缓存变化。尚未用完整合成世界对CopyNativeSnapshot整条成功链做端到端测试，也未验证真实140BE7C10的外交、条约、建筑／锁及对象生命周期；不能声称原版通行修复完成。
- 完整构建、统一入口58 PASS／零FAIL与embedded通过。候选dist主体00e333a6f0646586c69c35e33e44aeb6bc6f953830d9a22ec7eb043575c974cf未部署／实机，安装仍R147主体178e039d…；源码／dist／安装当前不一致。未操作游戏；国家权限和三处港口末端仍待，整体目标继续。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r149-permission-handoff/{before,build.log,full-tests.log,record.py}。


### 2026-10-30 — R148：原版权限采集桥隔离实现，未绑定游戏调用

- 新增DecodeDomesticStraitAccess供已验证访问范围内的权限回调使用：按国内lane国家去重，核验国家／海峡完整句柄、类型、当前定义、省份及已复制控制字段，生成自有国家→可通行完整句柄表；明确拒绝无法取得的结果，false作为已知拒绝，复制期间已读取字段变化或范围退役时拒绝结果。回调仅在对象校验后调用，不把指针交给worker。
- 已接现有native_domestic_tests统一入口：合成回调覆盖允许／拒绝／不可用、国家去重、旧海峡与旧国家代数、控制位变化及回调前退役。R147模型消费测试负责国家差异与缓存失效；这些都是离线测试，不等于原版权限函数ABI／游戏生命周期已验证。原版外交、条约、建筑及其可能变化尚未完整纳入一致性窗口，真实引擎回调未绑定，不能以此宣称海峡修复。
- 完整构建、统一入口58 PASS／零FAIL、embedded通过；候选dist主体0a104ab476a39d5dc3f808c5fcd9a8d93f1f0c24b7bf6f74d522e99aa6dc0a97未部署／实机，实际安装仍R147的178e039d…，当前源码／dist／安装不一致。未操作游戏，现有生产行为仍走旧路径。
- 下一步核对140BE7C10及其外交／条约／TLS与锁路径，保证当前访问范围和相关依赖一致后接入快照；再实际验证不同国家、权限变化、世界退役及原版路线。三处港口末端与最终同构建验收继续。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r148-permission-capture/{before,build.log,full-tests.log,record.py}。


### 2026-10-30 — R147：按国家接收原版通行结果的数据通路

- 国内自有快照新增可选的国家→可通行海峡完整句柄结果；存在结果时按lane.country选择，拒绝缺国家、无序／重复或过期海峡句柄，并将结果纳入缓存内容身份。worker只接自有数据；原版采集尚不提供该结果，当前实机仍走原有注册海峡路径，不能声称国家权限或运河修复已完成。
- 以R142已复现的注册即放行缺口为背景，新集成回归为两国家提供相反合成结果，确认几何分别更新、交换权限使缓存身份变化、最终模型量不变；缺国家和旧代数海峡拒绝。此为模型测试，不是引擎调用或实机受限国家验收。
- 完整构建及统一入口58 PASS／零FAIL，embedded、selftest560452、默认隐藏通过；deploy.ps1安装主体178e039d3ebae6aaa9039e9ba03c6fe14960416bd07da48a0d7ee8a883353c12、引导71dae36d…；自主会话3148实际加载同哈希，市场量25963.19216／510.99 MU、218提交，918陆／93海／9038笔／3部分几何。采集尚未启用新权限结果，此实机只验证旧路径未回归。
- 本局一次DRAW批次记录peak_frame_us=3006、overrun_frames=1，超过软2ms预算；不是最终三轮性能验收，保留该证据。会话WM_CLOSE退出0，五输入字节不变、扩展相关错误0、Unexpected token基线一致。下一步验证原版通行函数完整调用及安全窗口并实现采集、权限变化与旧世界拒绝；三港口末端与最终验收继续。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r147-permission-owned/{before,build.log,full-tests.log,live,record.py}。


### 2026-10-30 — R146：海峡原始控制字段进入快照与缓存身份

- 修复同一海峡句柄下控制字段变化仍被判为等价：原样复制运行对象+A8..AB与定义+4B到自有快照，并纳入完整内容比较；不把这些字节解释为国家权限。生产头与既有世界贸易测试修改前已备份。先新增用例，旧源码实际FAIL；修复后四字段逐项变化、定义标记变化及复制中控制位变化拒绝均通过，原有测试保留。
- 完整构建成功，统一入口58 PASS／零FAIL；embedded、selftest560446亮像素、默认关闭调试界面通过。通过deploy.ps1安装主体7ba956dd5b591e5d2e1e4b0d4846e18d66812a6756b594cf422b364bed6a5541，引导71dae36d…；会话62296实际模块哈希吻合，市场页25963.19216／510.99 MU，218条线段提交，国内918陆／93海／9038笔／3部分几何。截图和日志属于当前暂停存档，不扩展为受限海峡实机验收。
- 自启会话正常WM_CLOSE退出0，五项受保护输入字节不变，扩展相关错误0、六条Unexpected token与旧基线一致。国际继续用原版已选贸易路线，国内继续用原版港口lane与模型运量；海峡国家权限尚未接入，现有国内把全部注册海峡加入候选的问题和三处港口末端仍未修复。原版通行函数可能触发TLS初始化／锁及外交条约分支，须完成ABI、访问期和生命周期验证后才能调用。
- 证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r146-strait-snapshot/{before,build.log,full-tests.log,live,record.py}。完整目标继续，最终同构建权限／改图／生命周期／性能验收未通过。


### 2026-10-30 — R145：当前存档海峡权限快速分支只读核验

- 以当前EXE／暂停原存档自主会话73064，只读复用完整海峡句柄、类型及定义校验，按两次顺序观察复读9条注册海峡；其定义+4B均0、海峡+A8至AB均0。按R142原版汇编和R143隔离验证，这些字段落在原版140BE7C10直接放行分支。此推断不等于实际调用该函数，顺序复读不等于原子快照。
- 当前存档没有受限字段或需要建筑的注册海峡，不能验证受限国家／开放运河／权限变化。原版建筑查找路径中的140742920可能初始化TLS、取得独占锁，不能未经生命周期和访问条件验证而从现有回调直接调用；本轮未启用调用或修改产品代码。注册存在冒充权限的问题仍未修复。
- 会话正常WM_CLOSE退出0，五项受保护文件字节不变，扩展相关错误0、Unexpected token与基线一致；主体仍9afd，无游戏。国内物流量仍按模型计算。三处港口末端、权限变化及最终验收继续。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r145-current-strait-flags/live/{strait_flags.py,strait_flags-first.json,strait_flags.json,close.json,after.json}。


### 2026-10-30 — R144：原版国家通行分支隔离核验

- 隔离进程校验精确EXE 7f53e326…，执行原版140BE7E10机器码；国家从属及第二关系检查以受控stub代替、条约为空。11例覆盖同一限制条件下本国／外国完整ID、两种调用模式、可控从属关系变化和开放规则；结果符合原版指令且输入逐字节未变。一个限制条件下本国通过、不同国家不通过；模拟从属关系可改变结果。
- 此结果仅证明合成对象和替身函数下该函数的分支行为，未验证真实从属、条约／外交子调用、游戏访问期或对象生命周期，不能启用原版调用，也不是实际游戏可通行的判定。R142中当前国内几何将注册海峡视为可用的缺口仍在，三处末端未修复。
- 原拟临时隐藏全部权限未确认的海峡会同时丢失合法路线，无法完成用户要求；未进行该修改，继续追踪真实原版权限来源。产品源码、DLL、部署和游戏状态未变，沿用R138同核58 PASS作为历史证据。完整目标保持进行中。证据：E:/Code/v3-analysis/checkpoints/2026-10-30-r144-country-permission/{owned_country_harness.py,result.txt}。


### 2026-10-29 — R143：原版海峡检查的隔离快速分支验证

- 独立进程以精确EXE SHA256 7f53e326…校验后复制140BE7C10原始机器码，保留相对地址布局；仅用合成stub替代建筑查找与虚函数，其他引擎页面不提交。23例覆盖定义是否要求运河建筑、建筑有效性、建筑+D0低两位的四种组合，以及两侧字段的7种直接放行组合；结果符合指令，输入逐字节未变。无游戏加载、附加或调用。
- 在此隔离fixture中，已注册的运河对象仍可能因建筑对象无效或状态位返回0；这佐证R142不能把注册本身当通行。字段的游戏语义、当前实机状态、外交／条约等深分支、完整ABI和生命周期均未验证；测试依赖合成stub，不能据此启用游戏内函数调用或宣称海峡权限已修复。
- 无产品源码、DLL、部署或实机改动；国内量仍为模型值。三处港口末端、真实国家通行权限及全部最终验收继续。证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r143-native-permission-harness/{fast_path.py,fast-path-result.txt}，调用链与汇编见R142 checkpoint。


### 2026-10-29 — R142：国内海峡通行条件缺口的负对照

- 当前国内模型在构建几何时将全部当前注册海峡加入path.straits；NativeTradeStrait只保存句柄、省份对、费率和定义，没有国家通行结果。独立夹具复用生产SelectNativeSeaConnections，构造已注册海峡和相应省份／样条，复现不带国家权限输入仍选出该边。此为合成负对照，不证明当前存档存在实际被拒绝路线，也不证明原版选路必经该边。
- 精确EXE SHA256 7f53e326…静态调用链14141FDA0遍历当前海峡，并以海峡对象、国家对象及模式参数调用140BE7C10分类；后者检查定义／运行状态、两侧字段并向140BE7E10传国家对象。Ghidra省略部分参数，已用permission-assembly.txt和classification-assembly.txt核对x64寄存器。不能将费率或注册存在冒充国家可通行；具体状态语义、可安全读取或调用的访问窗口仍待验证。
- 独立测试首次运行缺工具链PATH退出1且无诊断；补PATH后退出0并保留复现脚本，编译的fixture重命名main产生无返回警告；未改生产源码／DLL、未操作游戏、未重新执行统一测试。R138同核58 PASS仍为历史证据。模型量不变；港口末端与真实权限仍未修复，整体目标继续。
- 下一步在原版安全采集窗口确定国家／海峡权限的只读或已验证原生调用与寿命，设计禁用／开放／权限改变和旧世界失效回归，再接入当前航线几何；不得按ID黑名单或仅凭注册状态补线。证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r142-strait-permission/{permission_negative.cpp,140be7c10.c,140be7e10.c,14141fda0.c,permission-assembly.txt,classification-assembly.txt}。


### 2026-10-29 — R141完成：原版寻路端点来源与当前定义只读核对

- 静态反编译14144FEF0→141572DE0：候选运行州经定义的当前naval出口索引取得海区定义，取其省份数组首个16位值建立搜索候选；14141F910在两个候选数组间寻找合法成本最小路径并写出选中的两值。14141F1D0也包含海峡处理及另一搜索分支，不能将本分析简化为所有路径都只走同一分支。关键字段经14144FEF0汇编交叉核查；未调用引擎函数、未启用hook。
- 自启暂停会话41088只读当前6州两次，三目的州207／936／608的出口海区3157／3056／3072，首海省分别36064／35988／36029，数组均一项。各值与R139另一次同存档会话中各航线成员边界吻合；此为跨会话顺序比较，非同一原子快照，也不是对搜索实际返回值的运行时追踪。外侧港口样条不因此获得补画授权，路线末端仍待解决。
- 会话正常WM_CLOSE退出0，五输入字节不变、未见扩展资源相关错误，六条Unexpected token与基线一致；未修改产品源码或部署，主体仍9afd。沿用R138完整58 PASS作为历史同构建证据；国内模型运量和国际原版贸易口径不变。完整几何、通行权限、改图及生命周期最终验收仍未完成。证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r141-native-endpoints/{endpoint-audit.json,endpoint-assembly.txt,14144fef0.c,14141f910.c,live}。


### 2026-10-29 — R140完成：国内航线生成端点的静态调用链

- 对精确EXE SHA256 7f53e326…、静态基址140000000，原版国内kind0创建链14140D830→141419DA0→1414500F0，前者将生成结果与端点传给14140DF60→14140FAA0创建航线。141419DA0遍历候选州对、比较路径成本，并复制获胜的省份集合与州ID；当前证据是静态反编译，关键调用及寄存器由objdump交叉核对，不证明运行时原子性或调用安全。
- 1414500F0将路径搜索结果中的两个16位值和其他结果写给其调用者，141419DA0将两个16位输出放在局部变量；本调用链中尚未看到将它们随胜出集合一起保存。它们与港口缺段的对应关系和权限语义仍需独立确认，不能直接认定为所需海区、擅自重跑引擎寻路或增加hook。此发现为下一步追踪实际端点来源提供具体切入点，并非港口修复。
- 无产品源码、构建、部署或游戏操作；沿用已安装9afd及R138同版本58 PASS作为历史证据。国内最终运量继续来自模型；三处港口完整几何、运河权限、地图及生命周期最终验收仍未完成。证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r140-lane-writer/{14140d830.c,141419da0.c,1414500f0.c,selection-disassembly.txt,find_calls.py,parents.py}。


### 2026-10-29 — R139完成：当前港口外侧候选边界核验，末端仍待修复

- 当前游戏只读复制原版航线、实际海区邻接、样条及海峡定义，关闭进程句柄后用当前源码头独立重放：918陆路、93国内海路、9038笔、3条部分几何；对应边模型分配量0.586／418.679／0，零是模型结果而不是未知。顺序外部观察并非原子游戏快照。
- 三处缺失港口各有可连接lane成员的外侧样条，分别有2／1／1条候选；这些边不属于原版lane省份集合，各直接边均未找到匹配的当前海峡定义，但这既不证明原版选路，也不证明所有通行权限。台湾原版lane距离为0，直接补画外侧边无法证明忠实于该lane；不能硬编码地图身份或自行选其中一条。另有两条候选位于lane内，首版独立探针重复列出同key，修正去重后保存两版证据，未发现生产重复缺陷。
- 我方暂停会话34156正常WM_CLOSE退出0；五项受保护输入字节未变，安装与dist仍为主体9afd299a…及引导71dae36d…，无游戏。未修改产品源码、重建或部署；沿用R138同版本完整58 PASS，并非本轮重新执行。未见扩展相关错误，六条原有Unexpected token与基线一致；不据此声称所有游戏日志完全相同。
- 三处港口完整末端与原版合法路线来源仍待解决；国内运量继续标注模型值，国际贸易保持原版数据。下一步核对原版是否存在可验证的端点或另一合法路径来源，保留改图、运河权限、生命周期及最终验收未完成状态。证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r139-port-boundary/{boundary-audit.json,boundary_probe.cpp,live,NEXT.md}。


### 2026-10-29 — R139进行中：港口外侧海区与原版航线边界核验

- R138完成交付引导异常隔离回归、统一58 PASS，当前9afd无游戏。重用R136已确认的三处运行州／原版航线与现有源码，先只读本机splnet：三处港口的其他海区都存在通向lane成员的type2候选，西密克罗尼西亚存在两个候选；这不是原版已选路径／通行证明。
- 正在编译独立只读探针，从当前游戏实际复制航线、相邻省份、海峡定义及样条后关闭句柄，再枚举候选及所属边界；验证结果须区分原lane成员、运行海峡和原版选择，不按本机静态ID写产品或补线。源码／双DLL／五输入先备份。



### 2026-10-29 — R138完成：实际交付引导的缺失／损坏主体隔离回归

- 当前主体9afd299a…／47581219B、引导71dae36d…／779879B，源码＝dist＝安装＝R137实际加载；产品源码和两份DLL未变、本轮未部署或启动游戏，完整统一入口58 PASS／零FAIL／退出0。

- 直接复制实际交付version.dll到含中文／空格的独立路径，工作目录放另一目录及同名无效诱饵；缺少主体和整个freight目录时引导自动建立目录，加载失败码126；损坏PE时193；实际健康主体成功加载且码0。三者均在独立进程中保持系统版本信息查询可用，不是游戏进程内缺失／损坏实机通过。

- 扩展已有tools/test_freight_loader.py：统一入口同时对重新编译引导和dist实际引导分别验证有效stub、缺失、损坏及错误码0／126／193；17个导出存在、系统版本数据逐字节一致；另外原有--runtime使用真实双DLL验证新配置优先和迁移。无玩家运行时新依赖。

- 隔离测试及统一入口后两份实际安装与dist哈希相同，原save／settings／ini／content／continue五输入及存档目录保持、无游戏。首私有审计误以为R134清单包含该测试文件而被断言拒绝，核对清单后改为对比本轮before；失败脚本保留。一次仅用于打印的私有脚本遇GBK输出错误，完整测试已结束且结果不受影响。文档归档首版误将保持原CRLF的Python源按Markdown检查而被拒绝，修正后仅重跑归档尾段。

- A26新增当前交付引导隔离子项PASS；真实游戏进程中缺失／损坏、目录不可写、完整路径／升级等矩阵仍PARTIAL。三港口末端／运河权限、改图新字段、写任务／提交原子性、真实ABA／新设备与最终同构建A01–A27继续；整体goal ACTIVE。

- 证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r138-delivered-loader/{loader-audit.json,isolated-results.json,isolated 中文 空间,full-tests.log,before,NEXT.md}。



### 2026-10-29 — R138进行中：实际交付引导缺失／损坏主体验收

- R137当前9afd／71da源码＝dist＝安装，实际卸载恢复及原版日志对照通过。P7已按MASTER§2.4完整重读00／01／02／03；备份本单元将涉及的测试、源码、文档及双DLL。现有缺失／损坏用例每次独立编译引导，尚需直接针对当前交付version.dll验证。
- 在独立临时目录使用实际双DLL引导，覆盖缺失／损坏主体、中文空格路径／不同工作目录、系统版本转发和具体加载错误；需要时只改测试并接统一入口。隔离结果不冒充游戏实机、目录不可写或全部A26；原安装与玩家输入不动。



### 2026-10-29 — R137完成：原版地点查找器对照与当前版本卸载恢复

- 主体9afd299a…／47581219B、引导71dae36d…／779879B；源码／资源／dist未变，复用R134完整55 PASS与R135同核性能。本轮通过deploy.ps1实际卸载后恢复，最终源码＝dist＝安装＝实际24108加载。

- 同原存档／中文／暂停59883360，地点查找器输入台湾并点击第一个地区结果，分别保存点击前后error.log。仅系统VERSION.dll加载的原版27220与当前扩展24108均为点击前0条、点击后新增1条PdxDebugConstants命名索引错误；完整规范化错误Counter相同、点击差分只有该行。R136的新增日志在本机无Freight时可复现，不能直接归因扩展；此结论只限本机EXE／存档／语言／操作，不推断所有游戏版本或其他命名索引错误。

- 卸载前核对无deploy.ps1会清理的旧文件／playset条目并备份42个非主体文件；卸载与重装之间全部42个配置／缓存／可编辑资源逐字节相同，再次实机启动后仅freight.log／loader.log自然追加。实际模块分别为stock系统VERSION.dll、当前version.dll＋freight.dll；恢复后的市场0物流页40商品／218段／25963.19216／510.99MU实见。

- 两次我方会话27220／24108均全程暂停、正常WM_CLOSE退出0；五项受保护输入及存档目录字节保持、无新存档和游戏进程。当前retirement2实见，无扩展资源相关报错；不能将正常退出扩为ABA／新设备通过。

- A26当前卸载／资源保留／重装实载子项PASS，完整缺失／损坏／不可写／路径／升级等矩阵仍PARTIAL。三处港口末端、运河权限、改图新字段、写任务／提交原子性与真实ABA／新设备及最终同构建A01–A27继续；整体goal ACTIVE。本轮消除了R136这条日志的扩展独有疑点，未修复三处港口缺段。

- 证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r137-stock-location-control/{stock-control-audit.json,live,runtime-before.json,before,NEXT.md}。



### 2026-10-29 — R137进行中：原版地点查找器日志与卸载恢复对照

- R136当前9afd、三个真实运行州及原生航线核对后正常退出；台湾地点搜索结果点击同秒出现一条PdxDebugConstants命名索引错误，原因未确定。P7完整重读00／01／02／03，备份双DLL、五输入、全部freight非主体文件及文档；确认deploy.ps1无遗留待清理文件，未改变用户playset。
- 同原存档／中文语言／暂停状态分别运行原版卸载与当前安装版：地点查找器输入台湾，分别采集点击前、后及正常退出错误日志，核对实际加载模块、设置／存档／资源；最后经deploy.ps1恢复双DLL并验证原版物流入口。错误复现不自动归因，结果只限本机与该操作。



### 2026-10-29 — R136完成：三处当前港口身份与界面核对，缺段仍待修复

- 主体9afd299a…／47581219B、引导71dae36d…／779879B，源码＝dist＝安装＝实际46268加载；本轮未改源码／ABI／资源、未构建或部署，复用R134的55 PASS及R135性能证据。

- 当前暂停原存档在精确EXE7f53e326…／module7ff63d2e0000下，线程TLS对应当前上下文只读两次相同：三个真实运行州207／936／608的州定义及naval_exit分别对应3157／3056／3072，均属于各自原版lane成员；可用类型3港口样条却分别连接3002／3157／3056，均不属于各自lane成员。R121曾因会话结束未完成运行时字段读取，本轮补到当前运行对象证据；两次顺序观察不是原子快照，也不证明所有Mod。

- R121历史编译的独立探针在关闭PROCESS_VM_READ句柄后重放得到918陆／93海／9038笔、partialSea3，三条对应边的模型量0.586／418.679／0；当前生产日志独立确认相同全局数。端口和几何两个生产头及其回归与R121逐字节一致，但不能把旧探针冒充本轮生产逐边trace。零是合法模型分配，缺几何不改成未知运量。

- 实际原版南海航线面板展开大清港口连接，台湾行及原版商船需求1.40实见；该字段不是货物运量。独立面板探针确认台湾936、苏禄北婆罗洲608、菲律宾西密克罗尼西亚207均打开local_goods，三处截图保留；台湾与207物流列有非零数，608当前为零。按地区名称搜索后最初点击的是同名另一运行州206，探针识别后排除并改选207；截图显示列表／现有视图，不证明原版或扩展的完整末端几何。

- Ghidra明确选择victoria3.exe复核025E8450→8690→8070：成员内部按type2查连接，边界按当前成员海区与端点州查type3；同端评分不授权跨出lane补一条海路。该调用链仍没有发现可验证的跨海区端点补充来源；保持原有已知部分、数量和用户不硬编码地图表约束，不能凭naval_exit或最近海岸造线。

- 我方46268/start2026-10-29T00:41:42.2087748+08:00一直暂停／日期59883360，正常WM_CLOSE退出0；原save／settings／ini／content／continue五输入字节和存档目录保持，当前无游戏。退休2实见。日志对R134同语言多出一条“PdxDebugConstants”命名索引错误，原因未确定；严格Counter相等首审计被拒绝并保留，错误时刻与地点查找器点击台湾结果同秒，仅是时间相关、不证明因果；没有ironroad／freight／native-gui相关错误，不能称完全无新增游戏错误。

- 三处完整末端几何仍未修复；后续优先追查其他当前原生几何来源与权限语义，不猜路线。运河实际开放关闭／权限、更多改图、并发／ABA／新设备及最终同构建A01–A27继续，整体goal ACTIVE。本轮原生日志新增项需在后续可比场景定位，不能将它归因扩展或直接排除。

- 证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r136-port-runtime/{port-runtime-audit.json,live,ghidra-1425e8070.txt,audit-first-rejected.py,NEXT.md}。



### 2026-10-29 — R136进行中：国内港口原版连接及当前定义核验

- R135当前三对性能及20轮缓存验证已完成，实际安装9afd；重读01／02／03与MASTER§2.4和原版几何约束，备份当前文档／源码／双DLL与五输入。上一轮R121只核对本机定义，未成功读取运行时naval_exit字段，也没有三地实际屏幕验收。
- 原版025E8450／8690／8070反编译再次核对，仅按当前lane成员、相邻省份和端点定义查询现有类型3样条，不能据此用其他海区或最近海岸补图。下一步自主启动同一暂停原存档，只读复制运行时定义／海区、现有lane与样条，核对原生显示；不把本机旧表或测试ID固化为产品映射。复现和原生语义明确后，才考虑可验证的最小修复及相应回归。



### 2026-10-29 — R135完成：当前同核三对性能与20轮颜色组合缓存验收

- 主体仍9afd299ab0f0aba088e9f6db68147319fbdeabd4fae74fdfaed453e31f451a69／47581219B，引导71dae36d…／779879B；源码／资源／dist／安装未变，复用R134完整55 PASS。无新增构建／部署／原生ABI／hook；当前实际54780双模块核对，timing-audit.json证据及指定性能门槛PASS。

- 沿R129／R130同原存档59883360、英国地标／2560×1440／原图形设置／原生速度3，每臂正常重载同存档，暂停预热30秒后运行65秒；六窗完整连续64.83–64.85秒，窗内无截图／输入、所有尖峰保留。三对关闭→打开P95为17.9185→19.2502、17.8774→19.1865、17.9017→19.0847 ms，增加1.3317／1.3091／1.1830 ms均≤2；模拟最大降幅0.774997%≤5。Present入口间隔不是GPU／显示延迟，根市场关闭还改变原版地图模式，只证明本存档／速度协议。

- 随后world6／rev52／date59884134暂停，20轮四分类轮换＋三颜色／单商品hover进入离开／恢复全部／Escape关闭→F4原版市场→点物流，逐轮VM_READ确认closed／market0／tab，176份观察通过。完整市场量26371.75358／510.99MU不因视图改变；与原日期的25963.19216差异来自本轮正常模拟推进，不将跨日期数量当UI不变对照。前四轮建立变体后，后16轮uploads和实际pool均1721不变；SUBMIT的cache是累计命中数，允许增长，不能当对象池。稳定66.313秒uploads1721不变，全进程private固定15880417280B；后16轮private范围15879147520–15892967424B、净减1712128B，属于本窗口平台证据，不证明所有分配器／8192实际满池。

- 172个实际完成DRAW批次账逐项相等，失败／丢弃0，池与native槽峰1721，每帧尝试峰16，Pump帧／单步峰1161／708µs、超2ms帧0。QPC含调度／driver等待，软预算不作硬实时保证；R134真实2056／2392µs超时保留为各自历史局，不删除或归到本局。

- 我方54780/start2026-10-28T23:09:37.6659011+08:00正常WM_CLOSE退出0；原存档／settings／ini／content_load／continue五输入字节及存档目录保持，无游戏。重载退休2…6及退出退休7实见，不扩为ABA／新设备通过；完整错误对R134中文无新唯一行，六条Unexpected身份保持、扩展资源错误0，重复读档的次数差异单独保留。

- 私有runner在有效closed-2结束后误对CompletedProcess调用wait，脚本失败但完整采样保留；改Popen后下一次重载超时，未进入第三轮窗口，保留截图和同PID暂停状态。实际菜单重新确认后正常重载并完成最后一对；没有重启／强杀或剔除采样尖峰。屏幕角落已有系统通知各臂保持，未交互／关闭；不排除其共同调度影响。

- 整体goal ACTIVE；当前9afd指定三对、20轮组合平台与稳定零上传子项完成，不再列为当前缺证。A25整体仍PARTIAL：更广工作负载／最终全矩阵不外推。继续三港口末端确证、运河开放关闭／权限、更多地图与铁路新字段、写任务／提交原子性与真实ABA／新设备，以及完整A01–A27和异常安装升级卸载恢复。若后续代码改变，应在最终构建重跑必要门槛。

- 证据／当前对备份：E:/Code/v3-analysis/checkpoints/2026-10-28-r135-final-performance/{timing-audit.json,budget-report.json,error-comparison.json,live/benchmark,before,NEXT.md}。



### 2026-10-28 — R135进行中：当前9afd三对帧／模拟与组合缓存复验

- R134已完成正式三配色／八档图例及双语居中、坏GUI拒绝与恢复，55 PASS；源码＝dist＝安装9afd299a…、无游戏。P7按MASTER§2.4完整重读00／01／02／03及§10，备份当前源／文档／双DLL和五输入至checkpoint2026-10-28-r135-final-performance。
- 不改产品；按同原存档／相机／速度3／设置，三对关闭与物流各暂停预热30秒＋运行65秒，连续原始帧保留尖峰，P95≤+2ms／模拟≤-5%；再至少20次组合循环和≥65秒零上传／平台。旧R130通过不替代当前9afd，预算实际软超时保留。当前仅准备，未宣称通过。



### 2026-10-28 — R134完成：正式颜色／八档线宽图例、双语居中及编辑GUI校验

- 主体9afd299ab0f0aba088e9f6db68147319fbdeabd4fae74fdfaed453e31f451a69／47581219B，引导71dae36d…／779879B；源码＝dist＝安装＝最后实际63460加载。完整55 PASS／零FAIL，embedded／GUI精确归档／selftest560464／默认隐藏vtx0及默认无FRAME PERF通过；只经deploy.ps1部署。

- 业务色保持地区输入绿／输出橙，市场热度使用完整市场固定区段商品数量尺度；单商品模式按原生商品ID取既有稳定五色，未选或移出为空地图，原生外观统一已验证建筑绿。颜色只改视图，不改国际原版记录、国内最终模型分配、需求账、完整行／卡片／市场分母、路径或宽度。八档图例显示实际宽度比例2:3:4:6:8:12:16:24及数量阈值10／50／200／500／2000／5000／20000，提示比例示意、非像素／容量／距离工作量；0不画、未知--。

- 四个必需GUI类型按真实type声明计数，跳过注释／字符串；缺失或重名拒绝整个扩展挂载、记录明确原因并保留玩家修改，提示对照freight/native-gui/.defaults。当前9afd实际坏GUI67464中，注释伪声明不能冒充类型，原版四页签／frame0、修改字节保持，正常退出后严格恢复；后续63460重新加载完整GUI通过。16个按钮文字框改为100%边界／居中／鼠标透传，中英截图及实际点击验证通过。

- 当前9afd英文67696与中文63460各21项颜色／商品／方向／关页重开检查通过；市场218、单商品等待0／谷物hover123／移出0，州433全部248／输入107／输出141，原生绿与业务色恢复原段数。市场量25963.19216／510.99MU、州433量3548.63121全程保持。英文原版首行Artillery为66段，中文所选第二行加工食品为22段；这是原版本地化排序，不把两次选择当同商品对照。中文额外显示／隐藏实际截图与日志通过，数量不变。每个正常会话仅一次国内分配918陆／93海／9038笔，三处partialSea仍保留已知数量及可确认片段。

- 首颜色与缺类型回归先FAIL后PASS；red-gui.exe断言退出仅属独立测试，非游戏崩溃。初英文bd5c、缩短文案b671仍存在旧文字锚点偏移，居中静态例先FAIL，修正16文字框后PASS并重跑55完整入口；最终居中重包的非资源.o逐字节不变，assets862ecc6e…不变。前四局旧候选不冒充当前9afd实机证据，原DLL／源码／失败日志和截图均保留。

- 七个我方会话全部正常退出0；原存档／settings／ini／content_load／continue五输入最终字节相同，无新存档、当前无游戏。英文退出仅删除受控language字段，逐字段证明后恢复原中文字节。完整规范化error Counter逐局等于R132同语言基线，未新增扩展错误；每局retirement2实见，不扩为ABA／新设备通过。

- 整体目标active；三港口末端、运河开放／关闭及权限变化、更多地图／铁路新字段、写任务与提交并发／ABA／新设备、最终同构建帧／模拟三对、压力平台及完整A01–A27继续。颜色／图例／当前双语布局及坏GUI子项已完成，P5／A18–A19和A25整体仍DOING／PARTIAL。R130旧核性能不能替代当前9afd。

- 证据及R133回滚：E:/Code/v3-analysis/checkpoints/2026-10-28-r134-native-legend/{legend-audit.json,live,logs,before,NEXT.md}。

### 2026-10-28 — R134进行中：正式颜色／八档线宽图例及编辑资源契约

- 居中复验前发现仅缩短文案仍沿用无明确文字框的旧锚点。新增GUI静态用例先FAIL后PASS，16按钮标签统一100%文字框／align=center／鼠标透传；新完整入口包含GUI静态回归，55 PASS／零FAIL。资源重包后主体9afd299a…／47581219B，全部非资源.o与先前逐字节相同，assets862ecc6e…未变；已deploy并实际启动英文67696，当前在做居中版中英交互复验，尚未将该阶段判为完成。


- 初英文24612已实际加载bd5c449a…／47579683B，市场单商品等待0／首两行123与132／移出0，Native与Heat218；州433量3548.63121不变，首行原版tooltip确认为Artillery、66段，三配色保持；完整248／图例八档及首末阈值、关页／重开通过。正常退出0，英文只删除受控language字段，原中文字节与五输入已恢复。
- 旧英文地区All modes等标签偏长已缩为In／Out／All与Click Trade，中文提示同步缩短；只重包本地化，C++／GUI布局／shader保持。当前dist最终候选b671255dfadea26ffc52d02832ba0dd022ab23faa3933f28c9aaf5502c7a886f，安装仍首英文bd5c449a…；最终51入口与附加检查正在串行运行，尚未部署／实机最终候选。


- 上轮R133为progress：地区地图筛选及当前中文19场景通过，当前309a安装＝dist、无游戏。按P5／颜色主题完整重读01／02／03、MASTER3.2／6与实际FlowWidth阈值及当前样式；备份源码／测试／GUI／文档及双DLL至checkpoint2026-10-28-r134-native-legend。
- 采用业务色（地区流向、市场固定全市场量尺度热度）、单商品稳定ID色高亮、统一原生建筑绿三选项，不新增原生调用／任意tint。未选商品时高亮模式为空图并提示；颜色不改数量、几何、需求账、分配或分母。图例展开显示八档实际宽度比例及数量阈值，默认折叠；较长中文按钮缩短，选中商品单元表示选择状态。
- 通过标准：先覆盖颜色／缓存／统计不变、hover进出及scope拒绝；必需自定义GUI类型缺失／重名（排除注释字符串）保留编辑并拒绝挂载；完整构建／统一入口／归档及harness串行通过后deploy，当前双语实际控件／图例、切换／关闭、原输入保护。候选PASS不冒充实机或全目标通过。

### 2026-10-28 — R133完成：地区商品／方向／运输方式地图筛选

- 主体309aed48c5880adf61d83b6883200715dd86291277c72a6c0da7ceaa2cbcb9ba／47369073B，引导71dae36d…／779879B，源码＝dist＝安装＝实际18592加载；完整50 PASS、资源／GUI精确归档／selftest560382／默认隐藏vtx0通过。地区新增商品、输入／输出和陆路／海路地图筛选，州各商品完整数量、国内分配、国际原版记录、需求账及全市场分母保持；切州／关页／商品详情撤帧并重置。
- 当前原存档暂停59883360／world1／rev4，州433量3548.63121保持；全部248段，输入107／输出141，陆路65／海路183；加工食品22段，输入陆路1／输入海路21。州5恢复完整282段／5506.40802，返回州433及关闭重开248段。丝绸运量0时合法空地图，恢复全部248；原版加工食品详情无自定义线，原版Back恢复完整州视图，市场0恢复40商品／218段／25963.19216／510.99MU。 正式控件和中文口径提示实际可见；第二行是加工食品，前一条commentary与私有grain*文件标签误称谷物，原版详情已核实，原文件保留，不将其作为实机谷物证据。
- FreightFilter／LogisticsSelection增加mapDirection与mapTransport，商品沿用map-only good选择；当前国内edge保留陆／海来源，混合路线只画所选方式的原区段，含进出港段，按货流计数量一次。热基准／scopeDrawable／完整州行保持。两个SameFilter包含新图筛选，统计验证排除map-only字段；旧地图帧立即失效，旧同作用域统计可继续发布。legacy非原生输入拒绝新方式／方向能力，不猜其交通类型。
- 既有GUI Set调用写三个固定key，值带world／完整州ID／重开代数。当前context先发布、默认值后发布、readyID最后放行；换州／关页重置，旧context／未知商品／非法方向／重复项不提交，不按首候选猜测。沿用同一次精确键probe，不增加GUI变量全量字符串复制或新ABI／getter／hook，worker不持engine指针。
- 新方向用例先FAIL后PASS；新增混合陆海／组合空结果、缺几何保留数量、所有统计／热基准不变、缓存区分并复用同facts、旧上下文／关闭／原版返回及当前州rows可发布回归。完整最终入口50 PASS／零FAIL；只有资源换行从初候选CRLF修回原LF，最终重新跑完整入口及所有归档／harness检查。
- 原生运行仅一次国内分配：918陆连接／93海运连接／9038笔，3条partialSea仍保留已知数量。23个完成DRAW批次、池峰635、每帧最多16、帧／步骤峰702／247µs、超2ms为0、失败丢弃0；这是本局构建观测，不是帧P95／8192满池或最终性能三对。R132旧局软超时仍保留。
- 初资源归档精确检查因私有writer使用平台CRLF而FAIL，修回仓库LF后通过；首默认检查与opt-in harness共写日志而FAIL，最终统一入口及附加检查全部串行通过。原候选／资源／错误日志保留initial-candidate。首私有audit误填DLL长度47369317被断言拒绝，按实际build日志／stat47369073更正，hash未变，不掩盖该脚本失败。
- 我方18592/start2026-10-28T18:13:12.4452137+08:00，EXE7f53e326…／module7ff63d2e0000，实际两模块与dist一致。正常WM_CLOSE退出0／原五输入字节及存档目录保持／无游戏；完整规范化error Counter与R132中文恢复局相同，新增扩展错误0；退出retirement2实见，不扩为ABA／新设备通过。整体目标active，P5／A18–A19及A25整体仍DOING／PARTIAL。新增控件英文布局、较长中文“全部…”标签省略及商品选择指示待完善；颜色模式／线宽图例、旧玩家编辑GUI升级契约仍待。三港口末端／运河权限、地图新字段／并发／ABA／新设备、最终同构建三对及完整A01–A27继续。
- 证据／R132回滚：E:/Code/v3-analysis/checkpoints/2026-10-28-r133-state-filters/{state-filter-audit.json,live/main,logs,initial-candidate,before,NEXT.md}；下方进行中为历史。

### 2026-10-28 — R133进行中：地区地图筛选

- R132为progress（同核双语及实际卸载恢复PASS，ui-audit.json）；当前6094安装及dist不变、无游戏。按P5重读01／03与MASTER3.2／6，备份源码／测试／GUI／工具／文档及双DLL至checkpoint2026-10-28-r133-state-filters。
- 语义：商品／方向限制地图货流，运输方式只显示其实际区段；州各商品完整数量、国内分配、国际记录、全市场分母、铁路及需求账保持。海运含原版港口连接；混合路线不改路、不重新分配。控件使用既有GUI变量Set／Clear，带当前world／state／重开代数隔离，不能复活旧作用域。
- 最小通过标准：方向／混合方式／商品组合及缓存身份、未知值与旧上下文、关闭和换州回归；GUI原版绑定与宽度保持；完整构建／统一入口／归档验证后经deploy部署并实际切换，候选测试不冒充实机。当前仅计划与备份完成，无新ABI。

### 2026-10-28 — R132完成：同构建中英UI及实际卸载／恢复验收

- 当前主体6094ea6e…／47173344B、引导71dae36d…保持，74份native源未变，复用R131完整50 PASS，不重建。实际英文／中文搜索、分类、连续hover、商品右键／详情／原价与提示、州433→5→433及返回市场通过；对应商品ID和数量逐项相同。实际卸载后仅系统VERSION.dll加载、原版四页签；通过deploy.ps1恢复同对DLL并再启动，中文物流页40商品／218段／25963.19216／510.99MU实见。四局正常退出0、五输入字节恢复、无新存档／无游戏。
- 英文clothes／CLOTHES两项1565.19777，luxury clothes一项500.81954，与中文衣物／高档衣物的ID及原生1e5定点数量一致；四分类9／15／13／3项的数量之和精确等于全市场25963.19216。连续hover／离开不改统计；空结果合法0，清空恢复。州433／5／433为3548.63121／5506.40802／原值，248／282／248段，两种语言完全相同；Overview撤帧、市场恢复218段、中文隐藏／显示通过。国内仍是明确标注的物流模型分配，国际保留原版量，缺几何不抹掉已知运量。
- 英文原生grain搜索也返回Paper／Fine Art／Grain，沿用原生匹配；Paper右键未执行税／关税／扩建，原版详情／Local Prices／Applications及历史返回实见。首返回实际仍Local Prices，继续两步才到物流；首MU提示被前提示遮挡，移出后重采。只计已核实的最终画面；5秒日志旧行不冒充逐帧取消证据。
- 两次英文正常退出删除显式System.language字段。首次strict finish因settings字节变化拒绝，失败及脚本保留；精确JSON比较仅允许受控英文形状或仅删除该字段的形状，其他字段及四输入必须保持，再原子恢复原中文字节。没有改游戏原始本地化，mtime不称保持。
- 无Freight英文对照63392仅加载系统VERSION.dll，完整规范化error Counter与有Freight英文59640相同。本机英文interfaces文件缺TRANSFER_SUBJECT_OTHER_THAN_RECIPIENT、中文存在且原条约25行引用；保留三条原版错误，不将本机缺键推成上游普遍结论。两组操作序列不同，不把本次完整Counter相同扩为所有操作无错。中文及最终恢复局对R131新增唯一行0。
- 实际卸载保留freight全部非主体文件逐字节，包括玩家偏好／缓存／可编辑资源；随后只经deploy.ps1恢复6094／71da。最后30988实际加载同对，中文物流、海陆箭头及卡片可见，正常退出0。英文59640／中文6960／stock63392／恢复30988四局均正常退出0，原save／settings／ini／content_load／continue五hash及存档目录最终一致。中文与恢复局观察到retirement2，英文无该行，不扩为ABA／设备寿命通过。
- 本次三次带Freight会话实际冷批次各出现一次不可分步骤超过2ms：2030／2039／2163µs，峰值同时为帧／单步；真实池峰1201／1127／218、最多16次／帧、完成批次31／23／1，失败与丢弃0。保留真实软预算超时，不沿用R131零超时；不是帧P95／GPU时间或8192原生满池证明。
- 这只完成当前中英交互及卸载恢复子项，整体目标active。源码确认地区商品／方向／运输方式筛选控件尚未实现，颜色模式／线宽图例正式UI未齐，P5及A18–A19保持DOING／PARTIAL；最终同构建性能三对、三港口末端／运河权限、地图新字段／并发／ABA／新设备和完整A01–A27仍待。 证据：E:/Code/v3-analysis/checkpoints/2026-10-28-r132-english-native-ui/{ui-audit.json,live,stock-localization-source,uninstall-stock.log,reinstall-after-stock.log,NEXT.md}；下方进行中记录为历史。

### 2026-10-28 — R132进行中：当前构建英文原版UI验收

- R132英文59640已实际加载6094并正常退出0：clothes/CLOTHES两项1565.19777，luxury clothes一项500.81954、空结果0，4分类数量加和等于25963.19216，hover与排序不改分母；Paper右键／详情／本地价格／用途及逐历史返回、英文口径／MU提示实见。当前州433→5→433分别3548.63121／5506.40802／原值恢复，248／282／248段，Overview撤帧与市场218段恢复通过。
- 英文退出时Game只删除显式System.language字段，首strict finish的settings hash断言FAIL保留；精确JSON比较确认仅这个受控字段，其他字段不变，守卫仅接受精确English或精确删除该字段形状，原中文字节已恢复，五输入hash全相同。原版grain搜索也返回Paper/Fine Art/Grain三项，不擅自做字面再筛选。首Back采样实际仍商品LocalPrices，继续两步才回物流；首track提示被Trade提示覆盖，移出后重采验证，不计错误截图为通过。即将用恢复后的中文设置做同核对照，未改产品源码或DLL。


- 上一轮为progress：6094预算记录／20组合轮实机完成。本轮完整重读00／01／02／03，保持同6094源码／dist／安装，不重建、不新增ABI。成功标准为真实English locale下搜索分类／空结果／连续hover、商品详情／右键／原价提示、地区列及切换无旧值，而非中文界面输入英文词。
- 事前备份9文档、双DLL和原五输入至checkpoint2026-10-28-r132-english-native-ui；核对全部native源hash与R131一致。仅把实际settings的System.language从l_simp_chinese临时改l_english，其他JSON字段及四输入保持；测试正常退出后校验仅语言变化、按原字节恢复并核对五hash。恢复必须经语义／hash守卫，不能覆盖外部变化。
- 当前尚未启动／实机，原首候选R131失败仍保留；R130三对不当新6094证据，完整目标active，A25／设备／权限／地图等剩余门槛继续。

### 2026-10-28 — R131完成：原生构建预算记录与20轮组合缓存验收

- 当前主体6094ea6e34c3622efcf451a32c3695c4086846f31b2ba43020eedb5299881060／47173344B，引导71dae36d…／779879B；源码＝dist＝安装＝实际加载。完整50 PASS，embedded／GUI结构及实际归档、selftest560410、默认隐藏vtx0通过，部署只经deploy.ps1。资源hash862ecc6e…与R130相同，无新ABI／地址／hook／地图表／模型／GUI资产改变。
- Pump复用原有时钟，记录累计步骤用时、单步／逻辑帧峰值、同帧尝试峰值及超2ms帧数；完整／取消／替换时只汇总一次DRAW PERF，带world／rev／gen、数量／失败／丢弃及池上界。16次／2ms软预算／8192策略不变。cpu_us仅是Pump步骤累计QPC用时，含调度／driver等待、不含帧间等待或Request查找／worker／UI成本，并非线程CPU或GPU时间；峰值／超时计数为cache生命期累计。
- 首候选18be148d…实机37192：冷218条正常，完全相同请求重开却报reused0。严格对账及新增目标用例FAIL保留；修正诊断Hits将已经完成的保留项计为复用，待办项不计，几何／请求／预算行为不变。目标覆盖同帧多pass／换请求不重置、3ms单次超时只计一次、10秒间隙不算构建、失败抑制、8200请求→8192上传／8丢弃及最多16，修正后PASS并重跑完整入口。初候选源码／DLL／正常退出0／五输入保持及失败证据保留。
- 最终56348/start2026-10-28T05:50:49.4003851+08:00，EXE7f53e326…／module7ff63d2e0000，实际双模块核对；原存档暂停date59883360／world1／rev4不变。20轮原版分类→首商品hover→移出→恢复全部→Escape关闭／F4市场／点物流，逐次VM_READ确认面板身份，101份观察与101条完成批次逐项对账。全量25963.19216／218段、铁路510.99MU保持；hover不改卡片，分类不改全市场分母，国内模型与原版量口径保持，UK海陆线可见。
- 实际726次成功上传，失败／丢弃均0；池序列432→573→667→726，后16轮保持726，重开218条均uploads0／reused218。全逻辑帧最多16次，峰值550µs、单步骤255µs，累计Pump15.280ms、实测超2ms帧数0。实际没有填满8192；边界来自同生产cache的合成压力及UploadRibbon index守卫，不冒充游戏8192对象或人为制造超时。
- 最后稳定66.172秒，累计uploads726不变；全进程private范围14537433088–14537785344B。轮5–20范围14559342592–14592380928B，首末从14592380928降至14562492416B；池平台与该观测窗口成立，不扩为全部分配器／所有场景无泄漏。
- 我方正常WM_CLOSE退出0、原save／settings／ini／content_load／continue五输入字节及存档目录保持，无游戏；完整error对R130新增唯一行0、六条Unexpected身份保持，无扩展资源错误，不称长流程完整Counter相等。本局正常退出没有原生retirement日志，不能用exit0补成析构／ABA／设备寿命证据；首候选与最终会话均完整归档。
- 证据／R130回滚对：E:/Code/v3-analysis/checkpoints/2026-10-28-r131-ribbon-budget/{budget-audit.json,budget-report.json,live/pressure,live/first-candidate,initial-candidate,logs,NEXT.md}。目标active，A25整体PARTIAL；R130三对帧结果只归R130，当前6094预算／缓存通过不替代最终同构建三对。更多原生压力、英文locale、三港口末端／运河权限、地图新字段／并发新设备及最终A01–A27继续。下方R131进行中及更早记录为历史。

### 2026-10-28 — R131进行中：原生构建预算实测计数

- R131首候选18be148d…／47173344B已50 PASS并实载37192，冷218条／7.030ms累计Pump CPU、单逻辑帧1.274ms／最多16、超时0。实机重开218条却reused0，严格批次对账FAIL；新增用例精确复现后，完全相同请求只把已完成项计为复用，未完成项不计，目标PASS。原测试会话正常退出0／五输入不变，first-candidate及initial-candidate保留源码／DLL／失败日志。当前正在重新完整构建／回归，旧首候选不可作最终计数通过。


- 以R130实载e4e0583…为基线，无游戏；源码／双DLL／9文档事前备份至checkpoint2026-10-28-r131-ribbon-budget。按性能主题完整重读01／02／03，验证标准是实际计数与2ms软超时可追踪，而非预先承诺单次原生调用绝不超时。
- 复用Pump已有两次clock，不新增原生ABI／钩子／数据库访问；记录累计实际构建CPU（不含帧间等待）、单步／逻辑帧峰值、同帧总尝试峰值及超2ms帧数。保持16／2ms／8192和原分配／路径行为。完成／取消／替换时输出一次DRAW PERF，批次delta及world/rev/gen、已知池上界可回查，峰值是缓存生命期累计而非该批独占。
- 目标cache回归覆盖同帧多次、同帧换请求不重置预算、单步3ms超时只计一次、跨帧10秒空隙不计CPU、失败抑制及8192／8200压力计数，PASS。当前仅目标测试；完整构建／统一入口／部署／实机尚待，安装保持R130，整体A25仍PARTIAL。

### 2026-10-28 — R130完成：精确GUI键轮询与三轮性能复验通过

- 当前主体e4e0583bd51f764fe9ba0874ea183fd4fc54e68eaa12ffb880c37e63ee7b45e7／47160425B，引导71dae36d…／779879B，源码＝dist＝安装＝实际加载。完整50 PASS，embedded／GUI结构与实际归档／selftest560432／默认隐藏vtx0及默认无计时日志通过；部署仅deploy.ps1。
- 保持25ms悬停响应、逐商品键／owner协议；只按当前目录的精确键及已存hash筛选既有GUI表，碰撞再比较完整键，不复制无关字符串。仍扫描有界桶头，不冒充O(1)查找。500ms显示开关与低频身份诊断同用精确键；调试全表枚举保持。无新ABI／地址／hook／GUI资产／模型改变，worker无engine指针。旧4096项路径实际FAIL、新2项及碰撞／边界／只读不变回归PASS，目标离线计时不冒充游戏FPS。
- 实机40808/start2026-10-28T04:55:15.4458092+08:00，EXE7f53e326…／module7ff63d2e0000、实际双模块核对。沿用R129同原存档／英国地标／2560×1440／原图形设置／速度3协议，每臂正常重载date59883360，暂停预热30秒、运行65秒，六窗完整连续64.79–64.85秒、窗内无截图或输入，尖峰全保留。P95增1.0024／0.9268／0.9308 ms，均≤2；模拟降幅0.775036%／0.000177%／0.000610%，均≤5。此三对门槛PASS，R129失败保留；Present入口间隔并非GPU执行／显示延迟，关闭根市场还含原版地图表现差异，不外推全部存档／速度。
- 连续两商品hover／移出恢复、显示隐藏、中文“衣物”两行及空结果通过，基础quantity固定2637175358／1e5；清空恢复235段。中文locale中clothes在原版Goods和物流页均零，不称英文locale已验。铁路卡片510.99 MU不随筛选变化，国内继续明确模型分配且数量已知，国际保持原版记录。
- 20次实际Escape关闭→F4原版市场→点击物流页，每次只读确认closed／opened及market0／tab；累计uploads=538不变。随后稳定66.156秒仍零新增上传；全进程private范围15622623232–15623163904 B，仅作该窗口平台证据，不证明全部分配器／池压力。首版误把F4当关闭，被marketOpen1／tabdefault／visiblenone拒绝，失败脚本保留后改正确操作，未把失败计入20次。
- 我方暂停会话正常WM_CLOSE退出0，原save／settings／ini／content_load／continue五输入字节及存档目录保持、无游戏。完整error对R129新增唯一行0、六条Unexpected身份相同，扩展资源错误0；不称多次重载次数相等。退休实见[2, 3, 4, 5, 6, 7]，退出是否有独立原生退休按原日志记录，不以exit0替代ABI／设备寿命结论。
- 启动前freight.log为8512707B，超过既有8MiB轮换门槛；首finish前缀断言拒绝及随后的audit缺error.log均保留为私有归档脚本失败，未称产品失败。核对freight.old.log逐字节等于before-freight.log、当前头精确pid40808、loader原前缀不变后修正finish支持这一已验证轮换，完整当前会话与旧日志都归档。正常WM_CLOSE后native退休7实见，重载退休2…6；不扩为ABA／新设备通过。
- 证据／R129回滚对：E:/Code/v3-analysis/checkpoints/2026-10-27-r130-filtered-gui-probe/{timing-audit.json,live/benchmark,logs,NEXT.md}。目标active；三港口末端、运河权限变化、更多地图及新铁路字段矩阵、并发／新设备、真实池压力／构建超时与最终同构建A01–A27继续。下方R130进行中及R129记录是历史，以上为当前状态。

### 2026-10-28 — R130进行中：正式GUI轮询不再复制整个变量表

- R129实际P95失败保留；定位25ms悬停timer完整ProbeGuiVariables、500ms显示开关及低频身份诊断均全复制全局字符串。当前单元只在既有验证getter／UI线程窗口中按当前商品目录的精确键及已存hash预筛，不改变逐行事件／owner／过滤／发布／模型／ABI。
- 事前备份源码／R129双DLL／五输入至checkpoint2026-10-27-r130-filtered-gui-probe。原01／02／03按UI／性能主题完整重读；保持25ms响应和旧行leave不能清新行hover的逐商品键协议，不靠降采样掩盖成本。
- 原4096项夹具实际旧路径复制4096→断言FAIL；新路径2项、同分／碰撞／顺序／缺失／SSO／截断键冒充／越界及只读不变PASS。目标离线全复制7.4738ms、精确筛选0.0131ms，不冒充游戏帧时。新测试接统一入口；初次测试auto多变量类型不一致编译错误已修正。完整构建／测试／部署／实机正在继续，安装仍R129b13289d…，A25保持FAIL。

### 2026-10-27 — R129：默认关闭的原始计时链完成，三轮性能基线发现超标

- 当前主体b13289d1228bceb3c295fb37fc311069e25329013d8f249f636e9a6e1579da55／47017006B，引导71dae36d…／779879B，源码＝dist＝安装＝实际加载。完整49 PASS，embedded／GUI结构与实际归档、selftest560447、默认隐藏vtx0及默认无计时日志通过。原生ABI／资源／模型与R128不变。
- 既有Present／Present1增加IRONROAD_FRAME_TIMING=1才启用的诊断：线程自有64个原始QPC间隔批量日志，嵌套转发只计外层，TEST／非当前swap排除；world／swap／时钟变化撤旧部分批次。无逐帧日志／数据库扫描／配置修改。native helper、解析器与实际D3D入口回归接统一入口；非法开关2保持关闭。指标为Present入口间隔，包含调度／呈现节奏，不冒充GPU执行或显示延迟。
- 自主52156/start18:15:32.8660935+08/module7ff63d2e0000，实际双DLL核对。每臂重新载入同一原存档59883360、英国相同地标视角、2560×1440、原设置／速度3，暂停预热30秒后运行65秒；各有64.82–64.85秒完整连续帧，窗内无截图／输入，原始尖峰不剔除。关闭／开启三轮P95分别18.1201→27.2114、18.8955→25.5687、18.4805→27.7414 ms，增加9.0913／6.6732／9.2609 ms，**全部超过+2 ms，A25整体FAIL**。本协议日期推进降幅约0.775%／0.001%／0.001%，未超过5%，含一个原生tick量化误差，不扩为所有速度／存档通过。
- 单轮诊断控制：保留物流页、只隐藏箭头仍28.0334 ms；原版市场商品页20.6771 ms。不能将超标直接归因箭头；优先检查新增物流页绑定／刷新。根市场关闭还会改变原版市场地图呈现，报告同时保留这一作用域差异；控制组不替换三轮失败基线，不改验收门槛。
- 首次30天保护上限提前停止且鼠标边缘导致相机漂移，短样本拒绝保留；测试保护改60天（原半年自动存档未改），有效窗约32天。快速菜单批次曾未进入读档而超时；复核同一活进程并观察菜单后继续，没有重启游戏或把超时当退出。首版audit预期退出world10日志而失败；实见只到重载退休2…9，正常WM_CLOSE退出0由close.json独立证明，不虚称退出析构／flush已观测。
- 已正常退出0、当前无游戏，原save／settings／ini／content／continue五输入字节及存档目录保持；完整error新唯一行0、六条Unexpected token身份保持、扩展资源相关错误0。多次读档后错误次数不能当同协议旧核Counter相等。timing-audit.json核对证据完整性PASS，**不等于性能PASS**；私有证据／回滚R128位于checkpoint2026-10-27-r129-frame-timing。目标active，GUI性能优化、三港口末端／权限／改图新字段／并发设备／最终矩阵继续；下方进行中记录为历史。

### 2026-10-27 — R129进行中：受控帧间隔诊断与性能验收

- 以R128主体87c2c194…为基线，源码／DLL及五输入事前备份至checkpoint2026-10-27-r129-frame-timing，无游戏。按地图／性能主题重读01／02／03，核对PERFORMANCE及既有Present钩子；三港口末端仍缺明确原生关系，不补造。
- 本机当前token非管理员且无Performance Log Users／Performance Monitor Users角色；外部ETW不作为已可用证据。现有Present／Present1入口增加仅IRONROAD_FRAME_TIMING=1启用的原始QPC诊断，线程自有64间隔批次，嵌套转发只计外层、TEST与非当前swap排除，world／swap／时钟变化撤旧部分批次。默认关闭，不扫描原生数据库／不逐帧日志／不改配置。
- 原始Present入口间隔不是GPU执行或显示延迟；离线分析拒绝截断、计数／时间不一致、混身份、断档和不足要求时长，最近秩P95。目标回归正在接统一入口；完整构建、默认关闭与启用的实际harness及3×60秒受控开关／模拟速度仍待，不能提前判A25通过。

### 2026-10-27 — R128完成：当前已铺铁路统计及实际拆除／重载验证

- 主体87c2c1947d88a48bc003062a6e7efb46e52cd99593674e33b3105646622b89cb／46997736B，引导71dae36d…／779879B，源码＝dist＝安装＝最后实际加载R128。完整46 PASS、embedded／GUI结构及实际归档、selftest560391、默认隐藏vtx0通过。当前无游戏，原存档／设置／ini／content_load／continue五输入与存档目录保持。
- 当前type4已建铁路独立复制为owned builtRails，包含州内及跨市场记录；模型land连接筛选不变。worker按同一当前市场两端、完整56位物理strip去重，累计当前样条二维折线地图单位MU；正式卡片及中英提示已接通。商品／搜索／hover／货量不决定基础设施长度；已知空为0.00 MU，缺来源为--，未校准公里保持未知。无新ABI／地址／hook／引擎调用。
- 原存档实读34条已建连接（24州内），市场0为22条唯一strip；独立Python由原坐标求510.9929662664673 MU，正式510.99 MU。原版UI铁路2→1不变，1→0删除5连接（含1州内），29条／市场17strip、387.7629690309868 MU，正式387.76 MU。普通游戏变更后的市场／国内模型量与各自owned精确一致；没有用未知量替代国内模型物流。
- 同进程正常重载原存档，world1退休／world2恢复34连接、22strip及510.99 MU和原数量；我方正常关闭exit0／退休3。最终重新安装R128并启动PID60588核对实际双DLL、510.99 MU和完整中文提示后，正常退出0。拆除仅在我方未保存世界进行，没有改写原存档。
- 初次严格audit误用R127新游戏流程作同存档错误基线，FAIL及原脚本保留。临时经deploy.ps1部署R123，同存档／相同两次拆除及重载后，完整normalized error Counter与R128逐项／次数相同；最终R128启动Counter也等于旧版同存档启动。没有把已有空key报文称零错误。初局55440下次观察时已不在，退出原因／操作者／退出码未捕获，晚间另有一组已有报文，保留限制，不称整局零新增或我方正常退出。
- rail-audit.json PASS；证据与恢复入口：E:/Code/v3-analysis/checkpoints/2026-10-27-r128-built-track/{NEXT.md,rail-audit.json,live,logs}。先前R127改图场景属先前构建证据，新铁路字段未重跑全部改图矩阵。港口三末端、运河开放权限、更多省份／bitmap改动、渲染并发／新设备／完整生命周期、公里标定、3×60秒性能及最终同构建验收继续；历史R1200.12未解释，目标active。下方R128进行中记录为历史过程，以本完成记录为准。

### 2026-10-27 — R128进行中：已铺铁路独立统计

- 最新诊断状态：R128两局已实际显示510.99MU、真实拆2→1不变／1→0后387.76MU、同进程重载恢复510.99MU；第二局正常退出0，五输入保持。完整错误Counter对R127新游戏基线增加既有空key报文组，严格audit初版FAIL保留，正在用R123同存档／相同操作对照，临时安装R1233be8cbcc…、PID48192；仓库dist仍新87c2c194…，诊断结束需经deploy.ps1恢复新版本，不盲目回退源码。初局55440退出码／来源未知，保留事实不称正常退出。最终审计／文档尚未完成。

- 当前候选87c2c1947d88a48bc003062a6e7efb46e52cd99593674e33b3105646622b89cb／46997736B已完整构建、46 PASS、embedded／GUI结构及实际归档／selftest560391／默认隐藏vtx0通过，并经deploy.ps1安装；旧R123只作回滚基线。新55440会话已启动，实际存档／列与单位提示尚待验收。先在新用例复现已有7地图单位却未进入正式文档的FAIL，checkpoint2026-10-27-r128-built-track保留事前备份与before日志。
- 当前type4独立复制含州内／跨市场，不给模型新增自环／跨市场边；worker按市场两端／唯一物理strip求当前样条二维折线MU，商品／hover／运量无关，未校准公里保持未知。显式零／未知、去重、满位strip、市场、变更／缓存／异常与格式用例已加入既有统一测试文件；目标decoder PASS，视图回归／完整构建／部署／实机还在进行。

### 2026-10-27 — R127：真实目录／ZIP及replace_path覆盖场景通过

- **当前产品保持R123**：主体3be8cbcc92fa39160fa9f700c97fdb8e46e9f1e962d74c4494fcd528c00ae074／46613962B，引导71dae36d…／779879B，安装＝dist＝实际加载；73源hash一致，无产品／ABI／钩子／构建／部署改变，复用46 PASS。
- 相同60文件的目录与deflate ZIP继承R125新key／geo1500；17份州定义全部改名，metadata声明replace_paths。两局实际冻结VFS均25挂载／1replacement／州目录allowedRoots=1，分别首根directory与zip；当前州0／定义0／geo1500及州7／定义14／geo16、11／8省、918陆路逐条完全一致，当前93国内海运保持模型量，非旧ID→key表。
- 目录53280/start06:16:06.4887472+08：9030笔，市场2590031259fixed／国内2311645000、州0 359098837／州7 299463152；正式市场219段、两州231／207段。ZIP25876/start06:27:42.2081083+08：9062笔，市场2596938903／国内2309998100、州0 364445907／州7 301342341；正式222／243／188段。全部与各自稳定owned精确相同，截图实见，UI切换前后整份探针逐字不变。两有效局module7ff63d2e0000／paused1／date59883360、实际双DLL路径／hash核对；新局经济未固定seed，不把跨局差异归因归档格式。
- 最初ZIP在分析目录58968/start06:18:48.9730124+08异常早退，原exception／debug／session日志保留，无退出码或模块枚举、不称正常退出。当前EXE静态归档分支38A4B90按manager mod根+basename打开，原生字面mod根来自38A1710；同ZIP移入用户mod根即正常挂载。原finish因早退缺完整Unexpected基线而拒绝，未放宽产品或审计；不宣称任意绝对ZIP位置受支持。准备脚本state_regions.md断言及恢复脚本也保留。
- 两有效局均我方正常WM_CLOSE退出0／退休2；五输入保持／启用列表恢复、原资产不变，无新存档、normalized error Counter新增0。临时用户ZIP按本轮创建记录及hash守卫移除，只保留私有checkpoint副本；当前无游戏。zip-replace-audit.json PASS，证据checkpoint2026-10-27-r127-zip-replace/{scenario.json,static,live,user-zip-cleanup.json,NEXT.md}。
- 本项完成目录／受支持用户根ZIP及真实replace_path代表场景；增删／拆并省份／bitmap邻接、旧存档迁移、全部Mod兼容、港口三末端、开放权限、生命周期／渲染并发／新设备、铁路单位、3×60秒性能及最终同构建验收继续。历史R1200.12仍未解释，目标active。以下R127中间记录以以上为准。

### 2026-10-27 — R127进行中：真实ZIP／replace_path覆盖验收

- 当前安装＝dist主体R1233be8cbcc…／46613962B，无产品修改／构建／部署，46 PASS保持既有证据。完整重读原始四文档；备份及私有夹具在checkpoint2026-10-27-r127-zip-replace。
- 本机launcher声明zip支持；同内容目录与deflate ZIP包含60文件，17州定义文件全改名、metadata声明replace_paths=[map_data/state_regions]。应由实际冻结VFS规则及当前州／道路／模型／正式UI证明覆盖，尚未实机，不扩展为所有改图兼容。

最新授权（2026-10-25）：用户要求自主验证，允许自主启动/操作测试，覆盖此前手动操控限制；不强制结束可能有未保存进度的会话。当前事实以最上方记录及其证据为准。

### 2026-10-27 — R126：真实省份归属／地理集合交换与全部当前陆路端点重绑通过

- **当前产品未变**：源码／dist／安装／实际加载保持R123主体3be8cbcc92fa39160fa9f700c97fdb8e46e9f1e962d74c4494fcd528c00ae074（46613962B），引导71dae36d…（779879B），73份native源hash一致，无新ABI／钩子／构建／部署，复用46 PASS。私有夹具不是玩家依赖。
- control／swapped最终仅州定义地理字段、两份历史owned_provinces及metadata不同；交换现有两州11／8完整省份及geo／hubs／naval exit，州key／历史顺序和其他效果、建筑／人口作者输入、物理样条／locator／province bitmap不变。当前state0／def0与state7／def14保持，geo1500／16交换为16／1500，省份数11／8→8／11。19份唯一当前定义数值几何见证完整交换；16个相同native Province ID指向不同物理见证，不能当跨图永久身份。
- 全部918条当前原生陆路记录（含当前已建铁路）按physical key比较，精确等于州0↔7的端点置换，12条端点改变，无多／缺／其他变化；校验没有写入任何游戏状态或用名称／旧表补路。几何见证是私有只读观察，不进入生产身份算法或新ABI。Bitmap中心采样颜色方法精确匹配失败，已拒绝并保留失败，未用最近颜色放宽。
- 有效对照40612/start05:23:02.7114236+08：9019笔、市场2586409759fixed／国内2306758200，州0 359724016／州7 293273646，与正式全部精确相同；正式市场226段、两州234／188段实见。有效交换64968/start05:49:20.0878096+08：9022笔、市场2574204225fixed／国内2296533100，州0 307078798／州7 402427395，与正式全部精确相同；市场215段、两州196／247段实见。两局module7ff63d2e0000／paused1／date59883360、真实VFS首根对应Mod／25 mounts及实际双DLL路径／hash核对。独立新局经济未固定seed，不把跨局数值差异全归因于交换。
- 交换后两州及市场切换前后整份稳定probe输出逐字相同，统计没有反馈改变模型。control初次Start后早期8922笔与稳定9019不同，早期文件保留，不混为同输入计算故障；历史R1200.12仍未解释。初版交换6848/start05:40:11.4435351+08漏同步显式历史owned_provinces，私有probe拒绝province current owner mismatch；未开正式UI，正常退出后仅修夹具和生成器，不改变／放宽产品，原失败数据保存。
- 三会话均我方正常WM_CLOSE退出0／退休2；原save／settings／ini／continue字节保持、content_load按hash守卫恢复，原43源资产不变，无新存档。完整normalized error Counter对control／R125新增0，当前无游戏。证据checkpoint2026-10-27-r126-province-membership/{scenario.json,province-audit.json,owned-comparison.json,live,failed-fixture,NEXT.md}。
- 此项完成两个现有完整省份集合的真实归属变化与当前道路全量重绑，不扩为新增／拆分／合并省份、改province bitmap邻接、旧存档迁移、所有Mod或ZIP／replace_path。港口三末端、开放权限、全生命周期／渲染并发／新设备、铁路单位、3×60秒性能和最终同构建矩阵继续，目标active。以下为本轮中间记录，以以上最终状态为准。

- 产品保持R1233be8cbcc…／46613962B，未改源码／DLL。私有control／swapped两个目录Mod只在00_west_europe和metadata不同；Midlands／Lancashire完整11／8省份集合及匹配geo／city／port／farm／mine／wood／naval_exit字段交换，州key／历史及物理样条／locator／省份bitmap保持。明确是两个完整领土集合交换，不是增删省份／修改bitmap邻接。
- 原归属40612/start05:23:02.7114236+08已实测：当前state0／def0／geo1500、peer7／def14／geo16；完整当前ProvinceDefinition roster的region指针与machine省份表state18对应，11／8匹配。外部复制关闭handle后建owned模型；稳定9019笔、市场2586409759fixed（国内2306758200）、state0 359724016、state7 293273646与正式全量／两州量一致。市场／两州local_goods及地图实见；我方正常WM_CLOSE退出0／退休2，原输入保持／列表恢复，无游戏。交换后的采集与端点重绑定尚待，不把对照PASS当整项完成。
- 初次Start后探针8922笔／市场2586492759与后续稳定值不同，稳定复读与生产相同；早期文件保留，不认定不同时点为计算故障。原生省份数值ID不假定跨定义变化稳定；读取当前定义的19份唯一数值几何见证（私有观察，不用于生产）用于后续集合对照。中心采样pixel颜色方法18／19而非精确全匹配，保留失败并拒绝它，未以最近颜色／模糊匹配放宽。当前数据和几何字段没有新增到生产ABI。
- checkpoint2026-10-27-r126-province-membership含事前备份、scenario、probe、当前对照日志／图片／失败。下一步实际swapped局验证同一geometry witness及physical key对应的新runtime owner，再记录完整解析错误和原输入保护；其他全图／权限／设备／并发／里程／性能及最终验收继续，目标active。

### 2026-10-27 — R125：真实geo20→1500、匹配当前样条及定位器的目录Mod场景通过

- **产品未变**：当前源码／dist／安装／实际加载保持R123主体3be8cbcc92fa39160fa9f700c97fdb8e46e9f1e962d74c4494fcd528c00ae074（46613962B），引导71dae36d…（779879B）；73份native源与R123一致，没有新ABI／钩子／构建／部署，复用该构建46 PASS。真实Mod是私有验收夹具，不是玩家依赖或公开产物。
- 以R123的新key／州0／定义0为基础，仅将当前州geo20改1500，匹配6个样条点ID、12个connection key和5类generated locator ID；3条陆连接因新端点大小关系翻转key及strip顺序，坐标、33780点／4273strip／4271连接数量、strip ID、省份归属及州历史保持。生成器和独立既有decode复核全表。fortification locator的id20坐标不属于此州，未将任意同数值当同身份替换。实际43个覆盖文件全部只在私有目录。
- 自启64280/start04:58:46.1312722+08/module7ff63d2e0000，实际双DLL路径／hash已读；全程paused1/date59883360。VFS首根为geo Mod、25 mounts。生产同源只读副本当前state0／definition0／geo1500，6个新点及12个新key全部存在、对应旧ID／key全部消失，derived region仍未知；没有内嵌旧图回退。owned模型918陆／93海／8980笔、当前州353800627fixed（国内341011600）与正式发布／模型计数精确相同。
- 正式市场25790.44463／223段，州0 local_goods逐商品物流列及234方向段实见，陆路和海路同屏；外部222段不调用多候选选择器，不当作正式段数。此探针未独立打印marketQuantity，因此不把截图市场总量当完整外部市场对账。州7当前陆连接及145笔目标货流采用当前图；三条既有缺港口完整几何继续保留，不补造末端。
- Start前owned8892笔／州353799427fixed，Start后8980笔／州353800627fixed（增加0.012），当前正式采用后者。未保存完整前阶段solver输入，不将该变化归因于某字段或用于宣称旧R1200.12已解释／修复。旧精确失败仍保留；R124另一个新局阶段输入相同也不扩为普遍规律。
- 正常我方WM_CLOSE退出码0、退休事件2；原save／settings／ini／continue字节保持，无新存档。content_load按预期hash恢复原字节，原43源资产hash不变，最终normalized error Counter对R123vanilla新增0；当前无游戏。证据checkpoint2026-10-27-r125-geo-renumber/{scenario.json,geo-audit.json,live,NEXT.md}，含事前源码文档／双DLL备份及原失败边界。
- 完成的是**一个州的实际geo与匹配样条／locator重编号**。省份成员／拓扑变化、ZIP／replace_path、旧存档迁移和所有Mod兼容尚未验收；港口三末端、开放权限、全生命周期／渲染并发／新设备、铁路单位／3×60秒性能及最终同构建矩阵继续，目标active。

### 2026-10-27 — R124：初始化输入三阶段比较完成，历史0.12未复现／未解释

- 产品源码及双DLL保持R123主体3be8cbcc…／46613962B（安装＝dist），无构建／部署，73份native源hash与R123一致。私有探针仅PROCESS_VM_READ，关闭进程句柄后导出owned供需、完整州／市场、原版州对数量、当前海陆连接、完整点／strip／connection；不调用引擎、不attach／暂停／写内存。
- 自主13392/start2026-10-26T19:49:12.8109610+08，三个暂停/date59883360阶段：未选择国家、已选GBR、点击Start后，74481行／2918052B逐字节一致，均8958笔、州360622169fixed／国内348832600fixed、市场国内23181.142。当前样本没有初始化输入变化；不能反推旧R120差异原因。核对实际日志后，当前生产rev2–5复制从Start之后开始，早先“可能用选择国家前副本”为待证假设，未获本次支持。
- **本会话未开正式面板**：生产owner0／没有native domestic allocation，不作为生产／external数量验收。Oct27复查进程已不存在、日志最后有原生退休事件2；退出码／操作者及原因未取得，不称我方正常WM_CLOSE，也不据此推断崩溃。采集会话日志已归档，相关资源错误0，完整normalized error Counter对R123vanilla新增0。
- 五输入均匹配测试预期；原save／settings／ini／continue保持，临时content_load按期望hash恢复原字节，当前无游戏。证据checkpoint2026-10-26-r124-initialization-snapshots/initialization-audit.json及NEXT.md。历史0.12原因仍UNRESOLVED，需要同一份owned生产输入可重放的失败夹具，不能用新局巧合一致代替修复；其余geo重编／省份拓扑、ZIP／replace_path、港口三末端／权限、设备／并发／铁路单位／P95和最终验收继续，目标active。

### 2026-10-26 — R123：国内模型确定性修复已部署，真实身份Mod与运行刷新对账通过

- **最终当前源码构建＝dist＝安装＝实机**：主体3be8cbcc92fa39160fa9f700c97fdb8e46e9f1e962d74c4494fcd528c00ae074（46613962B），引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e（779879B）。只改native_domestic_model.h及native_trade_view_tests.cpp：按当前完整ID排序owned节点／陆路／港口边，使等价内容的同成本选路／分配稳定；不是ID→key表或新选路器。七种排列、state0／平行实体段／海路顺序／冷重算与热缓存先FAIL后PASS。完整tests-round123.log46 PASS／零FAIL／退出0，embedded／GUI结构及归档／GPU／自检560420／默认隐藏vtx0通过，经deploy.ps1安装。
- 自主35004/start19:27:39.2788735+08/module7ff63d2e0000，实际双DLL路径／hash已读。真实目录Mod新key／state0／definition0／geo20／7当前陆路，VFS第一根为私有Mod、25 mounts。初始date59883360：州357465715fixed（国内346126500）、9053笔、918陆／93海与生产一致；正式市场25881.45827／219段、州0 local_goods及244方向段实见。外部231段未调用多候选选择器，不与正式段数冒比。
- 正常Space启停后date59883372（两个原生步），生产rev7交接，当前陆路917、海93、9030笔／国内23017.375与owned一致；州量保持3574.65715、正式市场更新25879.45727／219段，未沿用旧量。原版量与国内模型独立，缺图3条仍有已知模型量，未补造末端。
- 正常WM_CLOSE退出0，退休事件2；原save／settings／ini／continue字节不变，无新存档。临时content_load经hash守卫恢复原字节；原37文件／134引用源资源未改，最终Mod解析错误Counter对vanilla新增0。当前无游戏。current-audit.json精确通过，checkpoint保留前后源码／DLL与原失败证据。
- **历史差异仍未解释**：旧R120 identity-fixed66860中正式360976587fixed与外部360964587fixed差12000fixed（0.12），原audit.py与pre-fix-audit-failure.log保持失败。当前同暂停世界的旧算法重放也与新版本一致，不能声称排序修复已解决该历史差异。确定修复的是独立复现的等价枚举故障；当前新局静止与刷新数值通过。完整geo／省份拓扑、ZIP／replace_path、旧存档迁移、港口三末端、开放权限、渲染并发／新设备、铁路单位／3×60秒P95及最终矩阵仍待，整体active。

以下为本轮中间记录（最终以以上为准）：

- 当前dist／安装／最近实际加载仍R120主体90e577c3…（46338126B）及引导71dae36d…；本轮生产修改尚未构建／部署。无游戏，原save／settings／ini／continue／content_load逐字节保持／恢复，无新增存档。
- 私有37文件／134引用目录Mod将STATE_MIDLANDS重命名为STATE_R123_MIDLANDS，并前移州定义及历史块；当前生产解码得到运行州0／定义0／geo20／market0（原版433／17／20／0），derivedRegion明确未知，新key不存在embedded data。正式UI州0 local_goods、240方向段实见，7条当前陆路连接存在；地图身份／显示单元通过，geo／省份拓扑／ZIP／replace_path仍未测。
- **数量未通过**：identity-fixed外部owned州360964587fixed（国内347033600）与正式发布360976587fixed相差12000fixed＝0.12；生产9036笔，外部8946笔。保留audit.py精确失败，不放宽断言。尚未确定采集时点或模型顺序的责任。
- 已独立复现模型确定性故障：同一内容反转states枚举，NativeTradeEquivalent=true，分配目的地9→10；统一native_trade_view_tests新增冷／热缓存与实际strip回归，改前实际FAIL。正将当前完整ID排序应用于owned图构建，不改变原版路径选择／ID绑定；该故障与实机0.12差异是否同因仍待核对。
- vanilla62192与identity-fixed66860均始终暂停、正常WM_CLOSE退出0；另初版fixture50600因生成脚本将BOM留在文件中间导致Svealand解析缺失和335新增错误，解码拒绝endpoint mismatch，正常退出0后仅修私有fixture重验。完整失败日志保留。证据checkpoint2026-10-26-r123-state-identity；源码／测试已事前备份，后续构建／部署／数值实机验收继续，目标active。

### 2026-10-26 — R122：真实目录Mod覆盖／同key几何及撤销对照通过

- **无产品变化**：源码及双DLL仍为R120主体90e577c382bca48bbb9bb31cedd90e720659b6fe624b4f264a0594d2c08d5c27（46338126B）／引导71dae36d…（779879B），安装＝dist。当前原启用Mod列表为空；私有lower／upper测试目录仅覆盖同一gfx/map/spline_network/spline_network.splnet，原版文件未改、不作为玩家依赖或公开产物。两种覆盖各只改变London港口connection298634505466509059／strip364035内point277150326的X＋8／＋18；全部key、strip、点ID、端点和其他坐标逐项保持。
- 三次正常启动：lower-upper PID48336/start17:50:35.9300278+08、upper-lower PID14144/start18:07:13.0089212+08、vanilla PID67328/start18:11:20.5641874+08；EXE module均7ff63d2e0000，观测paused1/date59883360。实际VFS前两根分别upper／lower及lower／upper，26 mounts；撤销后24 mounts且无私有Mod根。当前生产解码副本的X依次4071.94775／4061.94775／4053.94775，Z2767.87549及controls8390108,277150325,277150326,8691408始终相同；结果按文件内容／实际优先级更新，无原版内嵌几何回退。
- 关闭外部只读process handle后的生产同源facts／view／ribbons重放：市场目标strip始终有正货流，ribbon包含相同的新坐标点；原版＋国内总25963.19216、国内23098.111、9038模型shipment三次一致。外部probe不调用372个多候选selector，因此其209 segments不冒充正式全选择数量。**正式UI三次均实见218段／25963.19216，London港口曲线随覆盖变形并在原版对照恢复**；截图已查看。截图动画／地图锁定状态差异不用于像素精度量测，精确坐标来自只读／owned链。
- 三次均我方WM_CLOSE正常退出码0并命中原版退休事件；原save／settings／freight.ini／continue_game字节未变。临时content_load.json按期望hash守卫原子恢复每次原字节，原Mod列表现为空；原splnet SHA91c0957b…不变，当前无游戏。Mod会话error.log去时间戳逐行Counter与最终vanilla完全相同，新增行0；相关资源错误0，原六条基础Unexpected token保留。最后实读模块路径与core hash一致。
- 本单元没有生产源码改动，不重复46项完整构建回归或部署；这是**实际目录覆盖、同key／同端点单控制点变化、撤销恢复**验收。州key／定义ID／运行ID变化、省份拓扑、ZIP／replace_path、进程内图更新和所有Mod兼容仍未完成，不能把此局部结果扩为完整改图PASS。
- 下一单元：可撤销的真实州定义／key和ID变化场景；其余港口三末端、开放权限、全生命周期／渲染并发／新设备、铁路单位、3×60秒P95与模拟及A01–A27／部署回滚继续，目标active。证据checkpoint2026-10-26-r122-active-map-mods/{scenario.json,map-mod-audit.json,mods,live,protected.json,NEXT.md}；私有Mod包含本机原版派生资产，不上传。

### 2026-10-26 — R121：国内港口缺段来源核对完成，尚未补画

- **无产品／DLL变化**：当前仍R120主体90e577c382bca48bbb9bb31cedd90e720659b6fe624b4f264a0594d2c08d5c27（46338126B）／引导71dae36d…（779879B），源码构建＝dist＝安装。私有只读probe复用生产解码；无源码改变不重复完整46项测试。
- 自启66308/start2026-10-26T13:13:43.6375042+08/module7ff63d2e0000，观测paused1/date59883360。联合复读／关闭process handle后的owned重放93海／918陆／9038笔及三条量0.586／418.679／0保持；州key由当前typed对象确认，未固化为运行地图表。
- **精确发现**：50331727目的州207=STATE_WEST_MICRONESIA，定义578／geo555，既有type3 key298545170150208259连海3002，lane无3002、含3157。50331739的936=STATE_FORMOSA，定义530／geo510，key299077746094624259连3157，lane仅3056。50331743的608=STATE_NORTH_BORNEO，定义561／geo541，key298730712737305859连3056，lane集合3052／3125／3072。本机安装定义naval_exit_id依次3157／3056／3072，与lane吻合；本机文件佐证不替代VFS覆盖证明，运行时124出口字段的新探针未成功执行。
- Ghidra／完整汇编复核025E8450→8690→8070只按lane成员及邻接尝试当前海区与端点州type3 key，无另一海区补段分支；0124D460确经州E78取定义。GetNavalExitStateRegionDesc→01DB03A0→01572DE0定位定义124出口引用；只研究，没有调用getter／写文件生成器。现有港口存在不授权改原版路径，完整末端仍DOING。
- 会话后续终止，最后日志505501078有原生析构world2；当前无victoria3，未取得退出码／操作者，不宣称正常退出、崩溃或三处屏幕通过。Application Error／WER查询未命中本次Victoria3，不证明退出原因。原存档／设置／ini字节未变，相关资源错误0、六条基础Unexpected token与R117相同。证据checkpoint2026-10-26-r121-port-geography/{port-audit.json,static,live/port-replay.txt,live/keys.json,live/after.json}。
- 下一单元推进**实际Mod覆盖／同key几何及州身份变化验收**；三处完整港口仍须找到确证的当前原生几何，不能自算海路／近岸补线。其他生命周期／并发／新设备、真实铁路单位、3×60秒性能及最终矩阵继续，目标active。

### 2026-10-26 — R120：真实原生世界析构事件已接入并部署／实机

- 从当前精确EXE完整RIP引用定位4处stateDb写点；133b580为CStateManager析构，沿真实call找到更早的00811660原生世界聚合析构（机器120指针、world1C00州管理器／1C10数据库）。采用外层入口，在管理器释放前只处理自有代数／邮箱；不新增引擎调用／数据库读取／GUI访问。Ghidra反编译及独立Capstone ABI／RTTI／caller保存于checkpoint2026-10-26-r120-retirement-events/static，.pdata数据引用不当作调用证据。
- 当前实现绑定签名与void(this)原始调用，退休／完成点钩子均安装才启用正式采集。Mailbox按自有nativeWorld比较，析构即退epoch／清结果／替换新resident request，阻止旧发布并让新暂停命令可采集；UI先变代数不重复加。完全同地址私有回归旧epoch1/退出1，新epoch2/退出0；foreign／重复／晚到／首暂停／UI先行回归PASS。
- **源码构建＝dist＝安装＝实机**：90e577c382bca48bbb9bb31cedd90e720659b6fe624b4f264a0594d2c08d5c27（46338126B），引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e（779879B）。完整46 PASS／零FAIL／退出0，embedded／GUI结构和实际归档／自检560422／默认隐藏vtx0通过，经deploy.ps1安装；实际双DLL路径／hash已复制，非只测候选文件。
- 自启400/start12:40:50.3461912+08/module7ff63d2e0000，完成点58948／UI及析构57852；始终paused1/date59883360。直接原autosave重载时事件489793796先退epoch2，UI489794015采用新epoch，首暂停copy489795750/world2/rev8并于489795875交接；world2最终rev10／218段、25963.19216实见，uploads218不变。随后菜单退出事件490048656退epoch3，phase0/owner0/frame0；Continue原存档首copy490174328/world3/rev12与490174484交接，最终rev14／218段同量／uploads218。两次未重复增加代数，不依赖后续解除暂停命令。
- world3市场／州433 local_goods物流列与248方向段恢复；实际最小化及客户区2560×1440→1920×1080→原尺寸通过。正常WM_CLOSE时第三事件490522906退epoch4；close.json确认退出码0，无游戏。原autosave／设置／ini内容hash未变，相关Mod错误0，六条基础Unexpected token与R117基线剥时间戳后相同。没有强制结束／存档写入／配置恢复伪装。
- retirement-audit.json检查事件2／3后没有旧epoch snapshot copy／UI handoff，pool recovered前周期SUBMIT行0，退出事件4后无提交。实现已有每条箭头前epoch guard；周期日志不能证明检查与原生提交之间的并发原子性。实机指针均变化，实际同地址复用未观测，合成ABA证明的是事件策略；对象析构以外全路径／渲染并发／新设备仍待，A24整体PARTIAL。实际改图／开放权限／三条完整末端／铁路真实单位／3×60秒性能和最终矩阵继续，目标active。证据／源码／日志／R118回滚checkpoint2026-10-26-r120-retirement-events，见NEXT.md。

### 2026-10-26 — R119：当前aff98195同进程重载／菜单／新游戏及客户区生命周期场景通过

- 当前源码／dist／安装仍下方R118同一构建，无产品修改；完整重读01／02／03及TEST_PROCEDURE，复核现有世界代数／原生池恢复及phase退场机制。先同存档重载，再菜单／新游戏，核对旧世界禁止提交和新世界实际显示；真实退休事件／地址ABA及新设备仍不能由身份比较证明。
- 备份后自启我方47988/start11:06:15.4045767+08/module7ff63d2e0000，全程paused1/date59883360。英国初始world1→直接同存档重载world2，原生池校验218个保留槽后恢复218段、uploads218保持，市场25963.19216一致；未靠重启进程恢复。首外部45秒watch只采到原身份，错过点击时段，不将它当转换证据；转换由生产world2采集／失效／恢复日志及实际画面证明。
- 菜单确认框autosave_exit未勾选；只读25ms观察phase4/stateDb0→6→0，菜单ready0/frame0/owner0。随后新开法国沙盒：phase0→6→1→6→2→6→4，不同machine/world/stateDb；正式开始后world3，市场2／9878.27752／141段实见，8990模型／25404本地记录／3748原版州对，与英国9038／25396／3569不同。法国数据与当前UI采用，未沿用英国数值；本次是新游戏，不冒称第二份已存文件。
- 在法国实际最小化／恢复，再以SetWindowPos将真实客户区2560×1440→1920×1080→2560×1440，前后卡片9878.27752／141段与对齐保持，原窗口矩形恢复；这不是设备移除／新设备重建测试。随后正常Load原英国autosave，观察stateDb0／phase6→2→4及不同当前指针，world4/rev21恢复25963.19216／218段、州433本地物流列与248方向段实见；市场切回0，无法国旧作用域残留。
- audit提取三次失效至相应pool recovered(world2／3／4)之间periodic SUBMIT日志均0；world4恢复使用326保留槽，英国缓存命中不加上传。该日志与画面证明上述场景，不把周期日志扩展成逐帧无退休空窗或地址ABA证明。正常WM_CLOSE退出，无游戏；原autosave／设置／ini逐字节未改，仍只有原autosave，无新存档。相关Mod错误0，六条基础Unexpected token剥时间后与R117基线完全相同。
- **源码及两DLL未改／未构建／未部署**，仍R118主体aff98195056c0e42f4535ed3c9666f13b69f7d88dd11f06b42585fe545170a85（46324762B）及引导71dae36d…（779879B）；不重复运行已通过的同构建46项回归。证据checkpoint2026-10-26-r119-world-lifecycle/{lifecycle-audit.json,live,source-after.json,NEXT.md}。A24保持PARTIAL：可靠引擎退休事件／ABA、新设备重建仍未完成，实际改图／开放权限／三条完整末端／铁路单位／性能及最终矩阵继续，目标active。

### 2026-10-26 — R118：国内真实连接保留模型运量，已部署并自主实机通过

- 当前源码修复R117将缺目的港口绘图key误当不连通的问题；三条原版kind0连接可参与供需分配，不补造海岸坐标。能唯一确定方向的现有原版片段可画，完整可绘量不包含部分几何货流；缺图形不等于未知运量。
- 改前最小回归目标500000／满足0，改后满足500000／未满足0。原版提示和0131A590→01419540／01419390精确汇编确认+60为原生1e5旅行距离，支持合法零；未声称公里。负距离拒绝，已知空原版路径不授予连接，国际路径默认完整性约束不变；部分几何额外覆盖闭渠／歧义分叉拒绝。模型海路原生距离与陆路样条距离形成模型度量，不当物理公里。
- **源码构建＝dist＝安装＝实机**：主体aff98195056c0e42f4535ed3c9666f13b69f7d88dd11f06b42585fe545170a85（46324762B），引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e（779879B）。完整tests-round118.log46 PASS／零FAIL／退出0，embedded、GUI结构与C++实际归档、自检560400亮像素、默认隐藏vtx0通过；经deploy.ps1部署。
- 自启61904/start10:44:26.1938907+08/module7ff63d2e0000，全程paused1/date59883360。生产完成点及独立自有重放一致：918陆路、93海路、partialSea3／unavailableSea0、9038笔模型（8278涉及陆／2171涉及海，可重叠）。三条原缺端点连接50331727／50331739／50331743最终边分配milli586／418679／0，现有原版片段10／1／3；第三条为模型最终合法零，不填未知。三条逐边量来自进程句柄关闭后的自有重放，生产全局数量与连接数相同，但三条各自屏幕尚未验收。市场0原版2865.08116＋国内23098.111＝25963.19216／218段，州433列与248方向段实见；衣物1565.19777／空结果0与撤图、Freight账30585.31／23098.11／7487.20保持。
- 20次真实关闭／F4重开，逐次只读核对market0/tab，uploads359不变；private首／中／末14602153984／14592372736／14555389952B。稳定州视图14样本／66.28秒uploads607不变，private约14655889408→14656368640B。首稳定测试从地区切换上传期就开始，断言失败已保留stable-first-failure.log；重采静止窗口通过，不掩盖失败。这些不替代3×60秒P95／模拟速度协议。
- 正常WM_CLOSE退出；原autosave／设置／ini逐字节未变，无游戏。初版错误检查将所有Unexpected token误当本Mod错误，实际6条全部是city/fleet基础资源，去时间戳与R117基线逐项相同；本Mod关键词错误0，after.json保存完整对照。证据／源码／日志／图片／R117双DLL回滚：checkpoint2026-10-26-r118-domestic-port-endpoints，见NEXT.md。完整3条港口末端图形仍不可用，不补造；实际改图／同key几何、开放权限、完整生命周期／设备、铁路真实单位、性能与最终A01–A27继续，目标active。

### 2026-10-26 — R117：国内模型物流、当前陆路及可编辑资源已接入并部署；3条港口端点待续

- **当前源码构建＝dist＝安装**：主体87b6d408faa070f97dbd372a43df5ba51c35a0167bb2fc612f4f151d84b2c88e（46295447B），引导71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e（779879B）；游戏已正常退出。用户裁决见MASTER §2.3.2：国际原版记录保留，国内按物流计算并标注“国内模型分配”；不依赖硬编码ID→key地图表。
- 同一R101完成点复制当前本地供需、国家338引用的kind0港口连接、当前陆路；陆路端点由定义省份44→machineB0运行省份18解析完整州句柄，64位定义key只查当前样条表，保留56位strip和当前坐标。原版进出口先计入州余额，复用MCMF分配剩余同市场供需；显式当前州终端替代旧region/hub索引，州内聚合不推断建筑位置／州内成本。原版量1e5、国内最终milli×100同单位展示但分别保存；目标／满足／未满足及Freight账独立。过滤／hover不改分配或分母，国内数据进入完整hash／精确等价比较。
- 可编辑文件首次释放至freight/native-gui/{gui,localization}与freight/assets/gfx；各自.defaults留当前默认值，未改文件升级、自定义保留，按实际内容生成启动ZIP。坏GUI保留且拒绝；玩家说明freight/EDITING_RESOURCES.txt。实机验证GUI／shader修改保留、实际归档采用修改、早期挂载及未修改本地化随升级更新；最后测试注释已按hash守卫移除，默认明文恢复。
- 构建与检查：tests-final-round117.log统一46 PASS／零FAIL／退出0，新增源复制／海陆分配／断路／进出口余额／州与hover／缓存内容／文档账回归；旧“国内变化仍命中原版缓存”先实际FAIL再修复。首生产f0dccd6e…实机后仅缩短本地化货流标题，-ResourcesOnly形成最终87b6d408…；.text/.rdata/.pdata逐字相同证明同一代码，最终GUI／实际归档／embedded及D3D11自检560369、默认隐藏vtx0重跑PASS。
- 实机：我方始终暂停/date59883360。首f0dcc会话65252/start01:36:11.0665222和最终87b6会话30716/start01:48:03.2771832、57704/start01:59:56.4865947均正常加载／完成点采集，当前1147州／53商品／25396本地订单／93国内港口／918同市场陆路（908类2、10类4）。后台8974笔国内shipment，8272涉及陆路／2083涉及海运；90港口可组成完整原版形状。市场0原版2865.08116＋国内23098.111＝25963.19216，218段同屏海陆；州433本地价格列与248段输入／输出可见。中文衣物筛选1565.19777、空结果0且无箭头、hover／隐藏统计不变；Freight账30585.31／23098.11／7487.20同屏。
- 稳定／重开：首代码相同构建州433稳定145秒／30样本零新增上传（累计762）；最终87b6实际20次关闭／F4重开，每次只读确认market0与ironroad_logistics，累计uploads218不变。private第1／10／20轮14562799616／14591586304／14565076992B，局部未持续增长；这不是3×60秒P95或模拟速度验收。
- 失败如实保留：一次可选live_vfs_probe因窗口filesystem TLS未初始化返回5，不作挂载证明；资源采用实际ZIP逐字比对＋生产早期挂载日志＋实机显示。测试脚本把HLSL的//注释放到.shader外层导致3条解析错误；正常退出后恢复，改用外层#注释重验，最终相关error.log命中0。三轮原存档／设置／ini字节不变；无强制结束／推进／保存。
- **未完成**：3条国内港口有原版连接但缺目的端点图形（50331727：10→207，geo100/555；50331739：922→936，497/510；50331743：611→608，552/541；均hubs1/1、ports1/0），未伪造连接／运输。实际改图／同key几何／开放权限、全生命周期／设备、真实铁路长度、3×60秒P95与模拟速度及最终A01–A27继续，目标active。备份／源码／日志／图片／R116回滚位于checkpoint2026-10-26-r117-editable-resources-and-native-connections，入口NEXT.md；FREIGHT_MODEL与PROFILE已同步。

### 2026-10-26 — R116：正式原生采集解除旧地图／订单／铁路前置门，默认存档实机通过

- CopyNativeSnapshot不再需要request.geography/regions，也不再先解码旧模型生产/消费或DecodeNativeRails。新DecodeNativeTradeStates从当前完整typed州表复制运行完整句柄和已验证原生市场；derived region明确UINT32_MAX，当前引擎州定义ID/geo身份由NativeSeaGeography独立复制。原版贸易/选择器、当前端点和样条仍在同一R101完成点访问窗口/联合复读中交接；没有增加engine调用、换访问线程或回退模型。live_shipping_probe复用同一生产roster，不再把所有州market伪填0。
- 私有before/after实际重放：无旧地图请求在旧源码reads0/missing current snapshot request提前失败，退出1；新源码reads1进入原访问守卫/outside completed command scope，退出0。州名册用例覆盖缺旧表、未知Mod州key、旧counter不可读，以及完整句柄/错slot/重复/类型/market0/越界/变化/退休拒绝；加入原有统一economy/snapshot测试。初稿一例误将合法代数变化当坏handle，改成真正错slot；负对照最初混用两目录pragma-once头导致重定义，改用隔离完整旧header树后成功复现，均未放松生产校验。
- 完整构建f1ebed5606eb00bae67ff10e35b7935060ca4d18865265be4ccfb907279eacd5（43064787B）；统一45 PASS/零失败/退出0，embedded、自检560470亮像素及默认隐藏vtx0通过。资源内容862ecc6e…与R115相同，没有GUI/shader变更。游戏关闭后经deploy.ps1安装，当前**安装=dist=R116**，引导71dae36d…（779879B）；相比R115主体减少601940B。
- 自主正常启动我方PID32276/start00:24:06.1455456+08/module7FF63D2E0000，完成点线程67776。新代码实际日志legacy_orders/rails=not_collected；1147当前州/53商品/57组/3192目的地/3928候选，原版3192全选，3569细分记录。原生采集四次141/156/157/157ms（仅观测，不当作性能验收）。市场0卡片2865.08116/112段仍实见，无旧模型输入；新roster只读probe实读完整57组、4200逐商品/2513分组商品精确绑定、当前8192×3616/33780点/4273段/4271连接；外部多候选仍不调用原版选择器，不扩大其证据。
- 我方全程paused1/date59883360，无推进/保存；正常WM_CLOSE退出，原autosave/设置/ini字节不变，无相关GUI/shader错误。开始时曾观察到既有PID65156/start00:01:56，未输入或结束；其后查询已消失，才部署/启动我方会话。当前无游戏，保留安装R116，R115精确回滚材料备份于本单元rollback。
- 完成的是“正式native不依赖嵌入地图/旧订单/铁路”及默认存档回归；实际修改地图的州key/定义ID/同key换几何、开放/权限变化、全生命周期/设备、真实铁路物理长度与3×60秒性能/最终A01–A27继续。未把fixture或默认地图当完整Mod兼容。证据checkpoint2026-10-26-r116-native-capture-map-independence/{before-repro.log,after-repro.log,live,logs,source-after.json,NEXT.md}。按§2.4完整重读01/02/03；R115与本轮R116均为PROGRESS，目标保持active。

### 2026-10-25 — R115：原版贸易正式视图接通，当前候选自主实机通过

- native_trade_view将原版逐商品出口/当前中心/已选方向链接入worker、缓存、会话、正式文档及市场/州变量；原生整数1e5精度，不构造模型allocation/view。缺原版端点/缓存/几何保留已知数量，不补画或伪装未满足需求。正式标签为“原版贸易数据/贸易量”；模型Freight需求账控件从正式页移除，模型层仅调试/离线保留。world/revision、取消、未知good/作用域、溢出与数据源切换仍严格拒绝。
- 卡片在hover前汇总，商品占比固定全市场分母，原版匹配good ID用于中文筛选；零订单州从当前native roster识别。native/native输入忽略无关legacy orders/rails，同时保留R114全部原生内容精确去重；原生X/Z直接转绘制点，避免近零Z反射损失。等量相向流使用单条Balanced双向动画，GPU读回/负对照通过。
- 最终构建11b3faddc9c2ff9fdbb940c2eab2ae2638fd48f087b0db245b8def1a96ef9f7d（43666727B），统一入口45 PASS/零失败/退出0；embedded、GUI结构/实际归档、GPU正常/双向/三负对照、独立加载560394亮像素与默认隐藏vtx0均通过。引导71dae36d…（779879B）。已由deploy.ps1安装，**安装=dist=实机加载的R115**。
- 自主正常启动PID15712/start22:39:01.4193294+08:00/module7FF63D2E0000；窗口/UI线程28468、已持有完成点线程64784。原版选择3192/3192，57组/3928候选/3569细分记录，53商品；当前地图8192×3616/33780点/4273段/4271连接/12海峡定义/9运行海峡。市场0卡片2865.08116、112段正式箭头实见；独立VM_READ逐商品求和精确相同。当前三个未建定义0/1/2底层各有2个海连接，独立枚举全部3928原版候选经同生产映射，拒绝652个有向附带海峡邻接、**闭渠连接返回0**；没有外部调用选择器。巴拿马/苏伊士截图无跨地峡贸易箭头，英国近景港口/海路仍可见；当前闭渠样本通过，不外推未来开放/权限变化实机验收。
- 中文“衣物”筛到2项/191.95477，空结果0.00/0段；日用品860.1734。hover 112→35段但卡片与逐行占比不变；隐藏/显示、关闭撤帧、恢复缓存通过。中英格兰state433本地价格列显示当前原版贸易量，scope118.00421/有海图形112.79952，114段输入/输出图形。仅在我方临时会话正常推进date59883360→59883366后重新暂停，不保存；订单25396→25395、原版事实相同，revision2/gen12正式帧保持，不再因legacy变化闪空。
- 稳定首视图70秒uploads保持112；后续长窗口也无新增。修正验收脚本移动鼠标后立即点击/落到上一控件的问题，逐次只读确认页签后20次实际关闭/重开全部恢复112段、allocReuse/viewReuse=1、总uploads保持315；private首14962966528→末14950445056，后15次范围14935498752..14976573440B，RSS约+4.6MB，没有持续上升趋势。此为当前重开样本，不替代完整筛选压力及三轮渲染/模拟性能。
- 正常WM_CLOSE退出我方已暂停会话；无游戏进程，autosave/设置/ini逐字节不变，相关GUI/shader错误命中0。当前R115保留安装；回滚材料checkpoint2026-10-25-r115-native-formal-view/rollback（旧R102，仅通过其中deploy.ps1回滚）。证据live/{loaded-modules.json,independent-native-audit.log,reopen-20.json,after.json,freight-r115.log,*.png}及logs。
- 尚待：真实开放/关闭及权限变化、跨市场/商品详情/全生命周期、新设备资源恢复、完整改图链路及3×60秒性能/最终A01–A27。采集仍先受embedded regions/rails门影响，原版几何已读当前图不等于完整改图兼容；正式已建铁路长度仍--。本轮属于PROGRESS，目标保持active，不把局部实机通过当整项目完成。

### 2026-10-25 — R114：会话去重漏比原版贸易事实修复完成（未部署）

- 私有最小用例使用同world/相同goods/orders/rails，只关闭原生海峡：旧HasEquivalentSnapshot错误返回等价，实际退出1/REPRO；修复后的针对性及统一用例通过。此前FreightEngine完整native fingerprint无法补救，因为会话在交worker之前就可能跳过整份新输入。
- 新native_trade_identity.h对原版owned当前中心、州/市场、细分/分组数量、完整海峡/费用/端点、组/候选/所选索引、全部当前几何及海峡定义/地区/邻接/港口做精确内容比较，不用指针或仅hash判等。无关容器枚举顺序归一；正常相同key顺序不分配排序副本。原版候选/控制点/邻接等语义顺序保留。重复相同内容保持原帧；海峡/路线变化不能被相同订单吞掉；model/native来源切换也不等价。
- 新回归接现有logistics_session_tests/完整入口：关闭/回收海峡、当前中心、原版改选/未选、1e-5量、同key改几何/定义/地区/端点、端口、缺失/已知空与顺序归一等均覆盖；完整42 PASS/零失败/退出0（tests-round114-final.log），原模型会话/hover/关闭/缓存回归不变。最终完整构建、embedded与独立加载560363亮像素/默认隐藏vtx0通过；资源hash仍1bf9c374…，GUI资源/逻辑本单元未改。
- dist候选主体8d717ce7f324088ddaff029e478feee06b293d16ffb8f20634f8198b42700406（40051850B），引导71dae36d…/779879B。初稿ebaebb77…不是最终产物；均未部署。当前无游戏/未启动，本机安装仍R10251ec975f…/35151385B；R112证明的是此前6a83d933…实际capture/selector，本轮变更未实机，不跨hash宣称新候选运行验收。
- 下一步正式native facts/view/统计/绘制/UI接通；R113唯一方向链可用，缺原版端点/当前缓存保持未知。数值使用原生1e5贸易量，不能套旧模型1e3或把吨/运力/距离工作量互换；颜色/线宽/原版过滤与固定分母继续。海峡可视、改图/生命周期/性能未完成。证据checkpoint2026-10-25-r114-native-snapshot-dedupe/{before-repro.log,after-repro.log,session-repro.cpp,dedupe-proof.md,logs,source-after.json,NEXT.md}。

### 2026-10-25 — R113：原版所选几何的确定方向组装完成（未接正式绘制）

- 新native_trade_geometry.h将原版当前中心/所选ProvinceSet与已验证的通用港口连接规则组合，仅按原始56位strip及control连成唯一端点序列；不作分支选路。闭渠约束、已知空连接、完整句柄/原版顺序、接缝与曲线控制点保留；不确定连接/分支/断开/退役明确不可绘制。它是自有几何组合，不宣称025E8450本来就是世界贸易renderer。
- 自主R102只读51568（16:25:54，module7FF63D2E0000，thread38216/context0）实读并在关闭句柄后的owned数据上审计：2974已选贸易记录对应885条当前native route，861条唯一序列对应2911笔记录/7794个strip出现次数；24条缺原版端点港口对应63笔记录，全部保留不可绘制。无真实样本出现分支；372多候选目的地外部仍不调用selector，不以此推断其几何。正常退出自有暂停/日期不变会话，存档/设置逐字节未变。
- 回归覆盖闭渠绕行/原版改选开放/关闭及回收handle、相反方向、同低32位不同56位strip、海图接缝/曲线、诱导邻接三角分支负对照、空港口/非法控制点与取消。新测试接完整入口，42 PASS/零失败/退出0；probe编译/实读通过。本单元尚未被正式DLL调用，dist仍同R1116a83d933…，安装R10251ec975f…不变，无重复部署/独立加载测试。
- 新发现LogisticsSession::HasEquivalentSnapshot漏比nativeTrade，可将订单不变但海峡/原版选择/几何变化跳过；已列TASKS，后续R114私有用例实际复现（关闭海峡仍判等价），修复后再接正式视图。native worker/统计/UI、海峡可视/全改图/寿命/性能继续；证据checkpoint2026-10-25-r113-native-route-geometry/{geometry-proof.md,logs,session-after.json,source-after.json}。

### 2026-10-25 — R112：候选完成点采集/原版多候选选择器/实际刷新自主验收通过（正式视图仍待）

- 无游戏时按deploy.ps1临时安装R111候选6a83d933…（38482263B），正常入口自主启动44104（16:06:52，module7FF63D2E0000）。实际原生完成点线程33112，UI66340；暂停初始revision2/3/4整份复制219/203/188ms，1147州/25396订单/53商品/34铁路，57组/3192目的地/3928候选/**3192已选**/3569州对/9运行海峡。372多候选目的地已在R101持有范围内实际调用原版013BDBE0并转成owned索引，当前几何/海峡定义/地区/定位点一起交接；外部探针仍不调用selector。完整候选采集/选择/交接本存档实机通过，不再沿用“新候选未加载”。
- 使用正常键盘Space映射扫描码39（R111 scan0无效的原因已解决），精确前台/PID/开始时间守卫下限时推进一个原生时间步59883360→59883366并立即重新暂停。新revision6实际25395订单/53商品被UI采用（整份266ms），当前外部生产解码/绑定继续4200→2513项全部精确一致。此耗时不代替P95/模拟速度协议，未持续推进或保存。
- F4及截图定位点击原版物流页，candidate-native-guard.png实见卡片/商品值为--、未接通提示，worker error=native trade geometry unavailable/frame0/ribbons0：原生事实没有偷偷回退模型。此为预期拒绝验收，不是正式UI/海峡可视修复。一次DPI矩形守卫在鼠标输入前拒绝，DPI-aware后正确点击；无猜测坐标连点。
- 正常WM_CLOSE退出自有已重新暂停会话后，原autosave/设置/ini逐字节未变（settings mtime因原版正常退出更新、内容不变）；无游戏。候选日志已存checkpoint，随后只经其rollback/deploy.ps1恢复**实装R102**双DLL，hash核对PASS。现在安装主体仍51ec975f…/35151385B，引导71dae36d…/779879B；仓库dist保持6a83d933…候选，没有拿旧dist覆盖源码产物。
- 本单元无源码变化，不重复构建测试；实机绑定R111完整构建/41 PASS的同一hash。下一步同源native worker/统计/绘制/UI及可确定端点/方向；原生已选ProvinceSet/海峡约束、当前中心缺缓存不可回退。完整改图/生命周期/性能和用户海峡可视故障仍未完成。证据/恢复checkpoint2026-10-25-r112-native-capture-live/{capture-proof.md,candidate-session.log,candidate-validation.json,native-update-scan-code.json,candidate-native-guard.png,session-after.json,rollback-verified.json,rollback,logs,source-after.json}。

### 2026-10-25 — R111：当前原生贸易中心/商品/缓存航线关联完成，自主只读实机精确对账（未接正式视图）

- 原版01221F180→013C4E70按州B48的当前CMarketArea+48查world+800，再按目的中心查组路线，不能拿旧组成员推断当前端点。新owned centers、独立8B8分组出口表接完成点快照/内容身份；当前组查找表与48字节组roster逐项核对key/指针对应，worker不持指针。新BindNativeTradeRoutes按当前目的中心将890细分出口精确求和，与8B8完整对账，只保留原版可选group/route索引，不选路/分配/生成港口或方向。
- 实机发现5个当前市场区中心与路线缓存代表不同（23:534/551、57:610/614、81:378/228、8:114/398、89:457/458）。初稿错误要求8B8 peer必须是缓存代表，被实读拒绝后按原版调用链纠正；完整句柄/类型校验保留。不把数量相同当别名证据，不从旧成员补路线。按旧成员汇总仅2333/2513项吻合；当前中心下4200细分商品汇总成2513分组商品逐项精确相同，1e-5精度无舍入。
- 自主正常启动仍安装R102/PID6176（15:40:50，module7FF63D2E0000，thread43648/context0），外部生产解码/owned绑定通过：57组/3192目的地/3928候选/2820单候选，3569细分州对/4200商品，2014分组州对/2513商品；绑定3569笔中跨中心2974已选/421原版缓存缺失/167外部未选，7同中心。421是记录笔数，不是物理航线条数；372多候选目的地仍未外部调用原版selector。地图/12海峡定义/9运行海峡数据沿R110通过，不能代替海峡可视验收。
- 截图确认暂停1836-01-01；首次前台守卫在输入前拒绝，随后正常Space/短按尝试均未让原生paused/date变化，运行态验收未达成，不宣称无输入。读取仅VM_READ/线程查询，无attach/写内存/nativecall/保存。正常WM_CLOSE退出我方仍暂停且日期不变会话后，原存档/设置逐字节不变；当前无游戏。
- 最终完整构建与统一入口native/build/{build,tests}-round111.log退出0，41 PASS/零失败；embedded、GUI结构/实际归档、自检560418亮像素、默认隐藏vtx0通过。候选6a83d9330f398f4231440629ead822eb45c5e862097cff79fe73af5d0b0d6eec（38482263B）未部署；安装仍51ec975f…（35151385B），引导71dae36d…（779879B）。
- 下一步实际候选完成点采集/原版多候选selector自主验收，然后原版端点几何/可确定方向、同源worker统计/正式绘制/UI。025E8450已验证调用者仍为CShippingLane，不能将R110传入组代表的诊断港口计数当原版世界贸易端点已证明。正式guard未解除；用户海峡故障未实机可视修复，完整改图/生命周期/性能仍待。备份/证据checkpoint2026-10-25-r111-native-trade-association/{association-proof.md,association-assembly.txt,static,trade-table-comparison.json,area-endpoints-live.json,area-association-confirmation.json,group-lookup-live.json,session-after.json,logs,source-after.json}。

### 2026-10-25 — R110：原版港口连接读取/映射完成，自主只读实机核对通过（未接正式绘制）

- 新native_ports.h只读复制当前原版定位点条件：CGraphicalMap+290管理器、当前高度调整transform表与原版5类hub实例，按原版02607620条件判断；同时核对运行地区+90模板与当前州definition/geo一致。stateHubs按完整州句柄保存，进入内容身份和R101联合快照；worker只拿自有数值。SelectNativeRouteConnections沿原版ProvinceSet/邻接边界、type3样条key及端点同分留首规则选择连接，保留原版已知空连接，不找最近海岸、不调用带Interface前提的港口评分/高亮helper。独立本机splnet记录校准type3 key。
- 用户最新授权自主验证，已用已安装R102正常入口-continuelastsave自主启动PID46096（15:02:25，module7ff63d2e0000），全程phase4/paused1/date59883360；仅PROCESS_VM_READ/THREAD_QUERY_INFORMATION观测，未输入/推进/保存/调用原版选择器。唯一SDL_app窗口可隐藏，按PID/标题/类精确绑定其thread62036/TLS context0，不取首个线程/其他context；目录53实读验证。
- 实机发现并纠正两项旧假设：57组中893/868/602/361/356的代表州均不在其成员列表，按原版013BF640独立使用代表/成员的行为移除这一项错误要求，保留完整句柄/唯一性等验证；旧源码负对照及新回归覆盖。实际注册hub均为CGameObjectLocator/VT047295F0，slot50=00624690恒true，+90为category（实测12），不是parsed CMapObjectType的validity flag；按原生RTTI/汇编及当前元数据纠正类型门与测试。
- 同一生产解码头实读PASS：57组/3192目的地/3928候选/2820单候选已选，3569州间记录/4200逐商品记录/9运行海峡；8192x3616、33780点/4273分段/4271连接；12海峡定义/77引用海省/1147州。2820已选路线映射19046海连接成员、5592港口连接出现次数，拒绝652个有向附带海峡邻接（不是物理运河数量）。372多候选未在外部调用原版选择器、故仍未选择；没有绘制/发布正式统计。所有3569有数值记录的source/destination均命中明确成员：3562跨组、7同组。
- 实读未建定义为canal_panama(35972/35975)、canal_suez(35978/35983)、canal_kiel(36047/36030)，当前运行表均无对应记录；名称只供诊断，未用黑名单。正常WM_CLOSE退出我方启动且仍暂停的会话后，已确认无游戏进程，原autosave与pdx_settings逐字节/hash不变。没有强制结束用户会话。
- 两项实机修正后重新完整构建与统一入口native/build/{build,tests}-round110-final.log退出0，40条PASS/无FAIL；最终embedded/GUI实际归档/selftest560441亮像素/默认隐藏vtx0通过。dist主体5c613bd61457707d42550190bcd04752cca1bad7961a2021f0fef9f21a1e8ccb（38359345B），引导71dae36d…/779879B。**未部署**，安装仍51ec975f…/35151385B。最初40PASS对应初稿，不能替代修正后的-final结果；实机使用R102加载层加外部新解码器，不当作新候选DLL采集/正式视图验收。
- 下一步将实际出口记录按原生组/所选路线关联，接同源worker绘制/统计/UI口径；原版多候选选择器的实际完成点调用、完整改图/生命周期/性能仍待。用户海峡故障尚未正式绘制修复验收。证据/备份checkpoint2026-10-25-r110-native-ports/{native-port-proof.md,port-layout-assembly.txt,locator-identity.txt,group-roster-live.json,hub-types-live.json,strait-definitions-live.json,representative-negative.txt,session-after.json,logs,source-after.json}。

### 2026-10-25 — R109：原版逐商品贸易方向、完整商品目录与内容缓存身份完成（未切正式视图）

- 原版01962410的IMPORTS/EXPORTS字符串及01962ED0导出分支、01222400/0103F440调用与完整汇编交叉确认：state890细分表的owner出口到peer，peer是对方primary+8完整句柄；不假定state20的common ID等于primary8。NativeStateTradePair改为source/destination，kNativeTradeQuantityScale=100000保留1.00001等原生精度，不经过旧1000单位；正反向记录分别保留，不净额化/重复记作两次同向。原生快照增加完整自有州/地区/市场roster。
- PopulateSnapshotGoods收录整个当前已验证目录，包括没有生产/消费订单但有原版贸易记录的商品；合法空订单仍有完整目录，未知订单ID仍整份拒绝。模型权重默认值未作为原版运输单位使用；正式native adapter仍未启用。
- FreightEngine内容身份覆盖原版逐商品数值/方向、州作用域、组/目的地/全部候选与所选索引、完整海峡句柄/定义/费用/端点，以及当前地图尺寸/点/控制序列/完整56位分段与64位连接key、全部海峡定义/省份邻接顺序/地区geo锚点；不取shared_ptr地址。无关枚举顺序归一，原版候选/邻接稳定同价顺序保留。正式native视图仍无模型回退。
- 旧源码负对照实际复现目录遗漏及native/model缓存身份相同；当前针对性回归通过。同revision下单个1e-5数值、改选/关闭海峡、同key换控制点/geo、原生ID高位、空连接/缺组件等均改变身份；重排/重新分配同内容不改变身份。live_shipping_probe追加只读原版出口数值样本输出，编译通过，新分支未实机。
- 完整主体构建后统一native/build/tests-round109.log退出0，39条PASS/无FAIL；embedded/GUI结构及实际归档、独立加载560437亮像素、默认隐藏vtx0通过。dist主体f1ab7c9b452a0b139b3c376b15c8815ac9ca0c763bc469777ad947aff8f74212（38023311B）；引导71dae36d…/779879B。**未部署**，安装仍51ec975f886d1f8859da720e7bb905740391b293c18294bfa4dd2aa54d362e43（35151385B）。当前无游戏，不启动/输入/暂停/退出；海峡故障未实机修复验收。
- 下一步原版端点/港口连接、组代表与实际出口记录的原生关联，接同源worker统计/绘制/UI口径。改图完整身份、生命周期/性能仍待。证据/备份checkpoint2026-10-25-r109-native-trade-identity/{direction-proof.md,direction-assembly.txt,static,regressions.txt,logs,source-after.json}。


### 2026-10-25 — R108：全部原生海峡定义/当前地区映射与未开通运河几何约束完成（未接正式视图）

- 新native_sea_geography.h从当前线程已有TLS context读取CStraitDefinitionDatabase，包含未建运河；完整运行海峡句柄、定义ID、省份ID、引擎地区定义ID/geo ID分开校验。CMap只复制完整有界指针表及原版路径/定义引用的省份对象、邻接和地区字段，不遍历全部陆地对象。定义/省份/州/上下文与所有数据一起复读/current核对，不调用会初始化TLS/加锁的getter或高亮/编辑器helper。
- 新SelectNativeSeaConnections只接受原版明确选中的候选，按原版025E8280位运算生成当前type2连接key，输出几何成员而非另选路线。海区对若属于任一原生海峡/运河定义，须被选中路径的当前运行海峡记录允许；定义虽在而运行海峡/所选路径未经过时拒绝，即使底层邻接/样条仍存在。无巴拿马/其他名称黑名单，无Dijkstra/分配。两向邻接被拒计数不等于物理线路条数。
- 回归负对照复现“同一ProvinceSet里底层相邻且有几何就画闭渠捷径”，新约束排除捷径并保留原版长路径几何；原版在开通后改选第二候选只映射直通，当前海峡存在但原版仍选绕行也不添加捷径，关闭后旧完整句柄/回收slot/missing选择保持不可用。另覆盖当前TLS不落到其他已填context、所有身份/容量/类型/省份引用、复制变化/退休/自有寿命；同省份/定义换geo锚点与当前样条时使用新key且仍挡闭渠。三个本机原版splnet记录独立校准位运算，未用同一helper生成测试期望值。
- 生产CopyNativeSnapshot已将seaGeography附到同一原生贸易快照并纳入外层联合复读，worker无引擎指针；**正式worker/UI尚未调用连接选择函数/显示这些结果**，R106无模型回退门仍有效。live_shipping_probe可加--world-trade <目录长度> --sea-geography，唯一游戏窗口线程通过只读NtQueryInformationThread/TEB取得已有TLS，复用同一生产解码/约束；不会选其他context，不调用原版选择器，多候选仍未选择。这个新分支未实机（当前无游戏）。
- 完整构建后统一native/build/tests-round108.log退出0、39条PASS/无FAIL；最后只增地图锚点例的针对性重编译PASS（sea-geography-round108.log），扩展probe编译PASS。embedded/GUI结构与实际归档/selftest560446亮像素/默认隐藏vtx0通过。dist主体40e15298cf4929733bb5bc0f13cddcae87eb581d32210424ec9a3439c4dd224c（37209345B），引导71dae36d…/779879B。**未部署**；安装仍51ec975f886d1f8859da720e7bb905740391b293c18294bfa4dd2aa54d362e43（35151385B）。未启动/输入/暂停/退出游戏，用户所见海峡故障未宣称修复。
- 下一步原版端点/港口及实际逐商品来源/方向/完整目录，接同源worker统计/正式绘制并处理缓存身份；改图完整路径/资源身份、生命周期/性能验收仍待。证据/原文件备份checkpoint2026-10-25-r108-native-straits/{definition-key-assembly.txt,static,local-original,logs,source-after.json}。原版本机资料只在外部私有checkpoint，不进入交付/源码仓库。


### 2026-10-25 — R107：当前引擎样条复制/自有几何转换与贸易快照交接完成，正式绘制继续

- 新native_splines.h直接读取原版CGraphicalMap+2C0的点/分段/连接三表及CMap当前尺寸，保留真实控制点顺序、完整56位分段ID、29位锚点ID和完整64位连接key；不从嵌入原版图替代当前几何。BuildNativeSplineGraph仅转换已有连接/分段，按当前地图高度转换Z并给绘制边分配独立密集ID，无Dijkstra或货流分配。未知类型不伪装基础四种交通类型；已知空连接保留空，不制造线。
- R101完成点CopyNativeSnapshot把当前几何与贸易副本一起放入同一外层EconomyReadWindow，完成后联合复读/核对原生身份/日期/current；NativeWorldTradeSnapshot持有const自有geometry。UI/worker不读取引擎指针，不调用含Interface-only港口评分的整套几何helper。live_shipping_probe的--world-trade分支复用同一生产头并离线转换核对，新分支仍NOT_RUN live（本轮无游戏）。
- 新native_splines_tests接统一入口，覆盖同低32位但不同56位分段、零锚点/原生标志位、分段反向/拆分/转折、四类基础交通、实际地图尺寸/Z一次转换、同key/端点但控制点改变、空连接、类型/计数/容量/哈希/哨兵/非有限值/未知引用/重复身份、复制任意组件变化、退休及源销毁。完整入口38 PASS/无FAIL/退出0；最后新增类型/重复例后针对性重编译PASS。完整构建、embedded、GUI结构/原归档、selftest560417亮像素和默认隐藏vtx0通过。
- 候选dist主体d7ba1188990804b93a7f5a2994f65c9943f1d1c98be2d444c72c0f2cb245ad0b（36523135B），引导71dae36d…/779879B。**未部署**；实际安装仍51ec975f886d1f8859da720e7bb905740391b293c18294bfa4dd2aa54d362e43（35151385B）。未启动/输入/暂停/退出游戏。正式原版路线→几何→商品统计/UI仍未接通，运河故障未修复验收；几何表成员存在本身不能当通行证明。源码中的nativeTrade输入继续显式拒绝旧模型回退。
- 静态类型纠正：RTTI044F85A0为CStraitDefinition（此前记录的StraitTemplate仅旧名称推测）；0441C938为CMap，0464E4E8为CGraphicalMap。找到定义库构造006E5AA0/primaryVT04424140，以及0064A830按当前已初始化TLS+8648 context取0514D418+10*index/active+8；定义指针数组+80/cap88/count8C。这是下一步全部海峡/未建运河的原生定义源候选，尚未新增此库读取/调用；不取第一个有数据的context，不调用会初始化TLS/加锁的getter。
- 下一步复制实际CStraitDefinition的定义ID/海区对和当前省份/地区geoID映射，按已选原版路径/当前运行海峡约束绑定当前几何；核对州间记录方向、完整商品目录与精度再完成worker/正式UI切换。改图全部内容身份、生命周期/性能验收仍待。证据/备份checkpoint2026-10-25-r107-native-splines/{native-layout-assembly.txt,static,logs,source-after.json}，native/build/*round107.log。


### 2026-10-25 — R106：原版选择器接入采集；候选构建通过，未部署

- 独立汇编复核013BDBE0单个原生数组指针入参/RAX保留候选指针，严格较小代价胜出/同价留首；013BD930/013BD840只读距离/当前海峡费用，00CF6B70的有效数据库/完整handle路径只读。精确EXE hash及四个入口签名加入profile，Game绑定只由已验证的R101完成点采集回调调用；不调用原版高亮、不改界面选择。
- 新tools/test_native_trade_selector.py不启动/加载游戏：在独立测试进程保留RIP相对布局，仅映射原函数代码和合成只读输入所需数据页，运行21例，覆盖非首候选、稳定同价、完整海峡代数和当前双向费用改变胜者，调用后输入/费用逐字不变。代码页RX，其余保留页未提交；执行前核对完整EXE SHA256。该结果是原生代码隔离harness，不冒充游戏内ABI/寿命验收。
- CopyNativeSnapshot在完成点/访问所有权和目录/订单/铁路复制后调用生产原版贸易解码与选择器，联合复读及世界/日期/current再次核验；EconomySnapshot持有自有nativeTrade副本，worker无引擎指针。缺选择器即未就绪；FreightEngine对带原版数据但尚无几何适配的输入显式拒绝，禁止静默回退旧分配器/旧统计。正式几何/商品统计仍待接通，不能部署此阶段候选作为运河修复。
- 统一native/build/tests-round106.log退出0、37条PASS/无FAIL；最后新增访问门用例单独重编译PASS（snapshot-round106.log）。新增关闭/过期海峡调用前拒绝、回调中数据变更拒绝及原版快照零模型回退回归通过。完整主体构建及embedded/GUI结构与实际归档/新主体运行目录/selftest（560421亮像素）/默认隐藏vtx0全部通过。
- **dist候选**主体7116233d5758238c81b69c46b917b732b9e16fab4f4dbaeb214735f1f8256c89（36032042B）；**实际安装仍**51ec975f886d1f8859da720e7bb905740391b293c18294bfa4dd2aa54d362e43（35151385B）。引导仍71dae36d…/779879B。没有部署/自动启动/输入/暂停/退出游戏；本轮未检测到victoria3。运河故障未修复验收，产品状态不以候选测试代替。
- 备份/原生汇编/隔离harness/结果见E:/Code/v3-analysis/checkpoints/2026-10-25-r106-native-selector及native/build/*round106.log。下一步几何按原版当前路线和海峡通行结果映射，并核对州间记录方向/完整商品目录再接正式UI。额外静态发现025E7F00港口评分要求Interface1或hidden1，**不得在GameState2/hidden0完成点直接调用整套025E8450几何helper**；当前新增选择器无此断言，未启用该几何调用。


### 2026-10-25 — R105：原版世界市场/州间记录复制器与选路交接接口完成

- 新增native_world_trade.h：按现有完整州目录/原生商品目录长度复制实际海峡typed表、world中的贸易组/目的地/全部路径候选、明确海峡ID/省份区间/距离，以及逐州细分贸易定点记录。候选与当前路线分开；单候选按原版直接返回，多候选缺少已验证原版选择器时保持未选择，不取第一条。
- 可选选择器仅在adapter持有已验证访问范围时调用，校验返回指针恰在当前候选数组元素边界，转成自有索引；复制完成前复读全部输入/current。没有在本轮绑定/启用真实引擎选择器调用，测试中的回调是合成接口验证，不能当原生ABI实测。worker输入仍无引擎指针。
- native_world_trade_tests接统一入口，覆盖非首候选、无选择器、外来/中间/越界指针、选择前退休、完整州/海峡代数、零ID、组/哈希/计数/省份/目录/负值/不可读/任意组件变化和源释放；针对性运行PASS。完整入口初版36条PASS/无FAIL/退出0；后加选择器接口/单候选/退休用例后针对性重编译与探针编译PASS，其他生产源码未改。
- 复用live_shipping_probe增加--world-trade <已验证商品数>，只读复用同一头并联合复读州目录。该新分支尚未在活游戏运行：本轮复核时已无victoria3进程，不推断退出原因、不启动游戏。R104b的57组/3192目的地等活进程证据不冒充新解码器的实机通过。
- 正式快照/worker/UI尚未消费新副本，实际商品流方向/数量标签、原版选择器绑定和海峡约束下的几何仍需接通与验收；用户所见旧模型海路问题未宣称修复。产品仍51ec975f…，安装=dist，本轮未重建/部署主体。回滚与证据：checkpoints/2026-10-25-r105-native-world-trade；详细原版结构续接R104 shipping-discovery.md。



### 2026-10-25 — R104b：找到原版世界市场路线/双边记录；未切换部署

- 97条CShippingLane只覆盖港口连接与条约转运，不能当全部贸易路线。已沿原版海区面板/海峡缓存找到world+898的57组世界市场路线、3192目的地记录；每条保存1–3条ProvinceSet/海峡ID/距离候选，原版013BDBE0按原生代价取当前最佳，不能固定第一条。
- 原版逐州进出口总计数与细分/分组州间记录已分别定位并只读采样；细分/分组表同总量，不误称输入/输出或混用总目标与实际记录。原版当前海峡注册表9项，尚无巴拿马/苏伊士/基尔；模板省份邻接仍可能包含闭渠连接，所以不能只把省份集合转成全邻接线。
- 当前4240继续由用户手动，未输入/推进/关闭；产品51ec975f仍未变。用户再次发现无视海峡时已明确说明正式视图还是旧模型，未虚称修复。详细结构/函数/样本及下一步见checkpoint2026-10-24-r104-native-shipping/shipping-discovery.md新增R104b；native路径与真实贸易量正式替换继续进行。



### 2026-10-25 — R104：原生航线只读层完成；正式切换继续

- 用户确认海路、商品筛选与统计一起改用原版贸易数据，已写MASTER_PLAN §2.3.1覆盖旧模型口径。新增native_shipping.h有界复制CShippingLane typed注册表、完整handle、省份区间、国家/端点/条约及原版商船需求；不调用寻路或修改原版高亮。省份集合不是有序路径，原版商船需求不是商品运量，约定周转运量也不冒充实际交付。
- 新增native_shipping_tests并接统一入口：高slot/代数、零ID、类型/容量/范围/哨兵/重叠、不可读/变更/退休与自有副本寿命通过。完整native/build/tests-round104.log退出0、35条PASS、无FAIL；最后字段改名/零ID补例的针对性重跑通过。
- 用户手动进入存档后授权只读核对。PID4240/start2026-10-25 00:06:02/module7ff6b6960000/phase4，生产解码头复用探针成功复制97航线（93港口连接kind0、4商品转运kind1），逐字节复读通过；另核对条约27两条商品条款与海区邻接。仅外部顺序观测，不当作原子访问窗口或正式绘制验收。
- 产品仍51ec975f…/35151385B、引导71dae36d…，安装=dist，本轮未构建/部署；新读取层尚未接正式UI。用户再次报告航线无视运河时，已明确说明仍在显示旧模型，故障未修复验收。下一步原生几何/实际进出口关联与快照/worker/UI同源切换。游戏人类操控，不输入/推进/退出。
- 证据/回滚：checkpoints/2026-10-24-r104-native-shipping，shipping-discovery.md/static/shipping-assembly.txt/shipping-live.txt/treaty27-live.json/province-live.json；后续代码继续前保存对应原文件。



### 2026-10-24 — R104进行中：正式海路与商品统计转原版贸易数据

- 用户确认路线、商品筛选和工作量统计一起使用原版贸易数据，MASTER_PLAN §2.3.1记录优先覆盖。旧静态海路分配不能证明运河已开通，不以只加巴拿马黑名单结案。
- 当前精确EXE已静态定位CShippingLane typed数据库、构造/刷新、ProvinceSet区间集合及原版海路可视化链；集合不是有序点列，+58/+60在单位确认前不标货运量。准备有界只读解码和异常/变更回归，未启用新原生调用。
- 产品仍51ec975f…，本阶段未构建/部署，正式视图尚未切到原版数据，运河故障尚未验收。游戏继续用户手动；checkpoint2026-10-24-r104-native-shipping保存原文件及静态证据。



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
