# 验收矩阵 A01–A27

### 2026-10-29 — R138完成：实际交付引导的缺失／损坏主体隔离回归

当前主体9afd299a…／47581219B、引导71dae36d…／779879B，源码＝dist＝安装＝R137实际加载；产品源码和两份DLL未变、本轮未部署或启动游戏，完整统一入口58 PASS／零FAIL／退出0。

直接复制实际交付version.dll到含中文／空格的独立路径，工作目录放另一目录及同名无效诱饵；缺少主体和整个freight目录时引导自动建立目录，加载失败码126；损坏PE时193；实际健康主体成功加载且码0。三者均在独立进程中保持系统版本信息查询可用，不是游戏进程内缺失／损坏实机通过。

扩展已有tools/test_freight_loader.py：统一入口同时对重新编译引导和dist实际引导分别验证有效stub、缺失、损坏及错误码0／126／193；17个导出存在、系统版本数据逐字节一致；另外原有--runtime使用真实双DLL验证新配置优先和迁移。无玩家运行时新依赖。

隔离测试及统一入口后两份实际安装与dist哈希相同，原save／settings／ini／content／continue五输入及存档目录保持、无游戏。首私有审计误以为R134清单包含该测试文件而被断言拒绝，核对清单后改为对比本轮before；失败脚本保留。一次仅用于打印的私有脚本遇GBK输出错误，完整测试已结束且结果不受影响。文档归档首版误将保持原CRLF的Python源按Markdown检查而被拒绝，修正后仅重跑归档尾段。

A26新增当前交付引导隔离子项PASS；真实游戏进程中缺失／损坏、目录不可写、完整路径／升级等矩阵仍PARTIAL。三港口末端／运河权限、改图新字段、写任务／提交原子性、真实ABA／新设备与最终同构建A01–A27继续；整体goal ACTIVE。 证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r138-delivered-loader/{loader-audit.json,isolated-results.json,isolated 中文 空间,full-tests.log,before,NEXT.md}。



### 2026-10-29 — R137完成：原版地点查找器对照与当前版本卸载恢复

主体9afd299a…／47581219B、引导71dae36d…／779879B；源码／资源／dist未变，复用R134完整55 PASS与R135同核性能。本轮通过deploy.ps1实际卸载后恢复，最终源码＝dist＝安装＝实际24108加载。

同原存档／中文／暂停59883360，地点查找器输入台湾并点击第一个地区结果，分别保存点击前后error.log。仅系统VERSION.dll加载的原版27220与当前扩展24108均为点击前0条、点击后新增1条PdxDebugConstants命名索引错误；完整规范化错误Counter相同、点击差分只有该行。R136的新增日志在本机无Freight时可复现，不能直接归因扩展；此结论只限本机EXE／存档／语言／操作，不推断所有游戏版本或其他命名索引错误。

卸载前核对无deploy.ps1会清理的旧文件／playset条目并备份42个非主体文件；卸载与重装之间全部42个配置／缓存／可编辑资源逐字节相同，再次实机启动后仅freight.log／loader.log自然追加。实际模块分别为stock系统VERSION.dll、当前version.dll＋freight.dll；恢复后的市场0物流页40商品／218段／25963.19216／510.99MU实见。 两次我方会话27220／24108均全程暂停、正常WM_CLOSE退出0；五项受保护输入及存档目录字节保持、无新存档和游戏进程。当前retirement2实见，无扩展资源相关报错；不能将正常退出扩为ABA／新设备通过。

A26当前卸载／资源保留／重装实载子项PASS，完整缺失／损坏／不可写／路径／升级等矩阵仍PARTIAL。三处港口末端、运河权限、改图新字段、写任务／提交原子性与真实ABA／新设备及最终同构建A01–A27继续；整体goal ACTIVE。本轮消除了R136这条日志的扩展独有疑点，未修复三处港口缺段。 证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r137-stock-location-control/{stock-control-audit.json,live,runtime-before.json,before,NEXT.md}。



### 2026-10-29 — R135完成：当前同核三对性能与20轮颜色组合缓存验收

主体仍9afd299ab0f0aba088e9f6db68147319fbdeabd4fae74fdfaed453e31f451a69／47581219B，引导71dae36d…／779879B；源码／资源／dist／安装未变，复用R134完整55 PASS。无新增构建／部署／原生ABI／hook；当前实际54780双模块核对，timing-audit.json证据及指定性能门槛PASS。

沿R129／R130同原存档59883360、英国地标／2560×1440／原图形设置／原生速度3，每臂正常重载同存档，暂停预热30秒后运行65秒；六窗完整连续64.83–64.85秒，窗内无截图／输入、所有尖峰保留。三对关闭→打开P95为17.9185→19.2502、17.8774→19.1865、17.9017→19.0847 ms，增加1.3317／1.3091／1.1830 ms均≤2；模拟最大降幅0.774997%≤5。Present入口间隔不是GPU／显示延迟，根市场关闭还改变原版地图模式，只证明本存档／速度协议。

随后world6／rev52／date59884134暂停，20轮四分类轮换＋三颜色／单商品hover进入离开／恢复全部／Escape关闭→F4原版市场→点物流，逐轮VM_READ确认closed／market0／tab，176份观察通过。完整市场量26371.75358／510.99MU不因视图改变；与原日期的25963.19216差异来自本轮正常模拟推进，不将跨日期数量当UI不变对照。前四轮建立变体后，后16轮uploads和实际pool均1721不变；SUBMIT的cache是累计命中数，允许增长，不能当对象池。稳定66.313秒uploads1721不变，全进程private固定15880417280B；后16轮private范围15879147520–15892967424B、净减1712128B，属于本窗口平台证据，不证明所有分配器／8192实际满池。 172个实际完成DRAW批次账逐项相等，失败／丢弃0，池与native槽峰1721，每帧尝试峰16，Pump帧／单步峰1161／708µs、超2ms帧0。QPC含调度／driver等待，软预算不作硬实时保证；R134真实2056／2392µs超时保留为各自历史局，不删除或归到本局。

A20／A23／A25当前指定循环／平台／帧模拟子项PASS，完整矩阵仍PARTIAL。我方54780/start2026-10-28T23:09:37.6659011+08:00正常WM_CLOSE退出0；原存档／settings／ini／content_load／continue五输入字节及存档目录保持，无游戏。重载退休2…6及退出退休7实见，不扩为ABA／新设备通过；完整错误对R134中文无新唯一行，六条Unexpected身份保持、扩展资源错误0，重复读档的次数差异单独保留。

整体goal ACTIVE；当前9afd指定三对、20轮组合平台与稳定零上传子项完成，不再列为当前缺证。A25整体仍PARTIAL：更广工作负载／最终全矩阵不外推。继续三港口末端确证、运河开放关闭／权限、更多地图与铁路新字段、写任务／提交原子性与真实ABA／新设备，以及完整A01–A27和异常安装升级卸载恢复。若后续代码改变，应在最终构建重跑必要门槛。 证据／当前对备份：E:/Code/v3-analysis/checkpoints/2026-10-28-r135-final-performance/{timing-audit.json,budget-report.json,error-comparison.json,live/benchmark,before,NEXT.md}。



### 2026-10-28 — R134完成：正式颜色／八档线宽图例、双语居中及编辑GUI校验

主体9afd299ab0f0aba088e9f6db68147319fbdeabd4fae74fdfaed453e31f451a69／47581219B，引导71dae36d…／779879B；源码＝dist＝安装＝最后实际63460加载。完整55 PASS／零FAIL，embedded／GUI精确归档／selftest560464／默认隐藏vtx0及默认无FRAME PERF通过；只经deploy.ps1部署。

当前9afd英文67696与中文63460各21项颜色／商品／方向／关页重开检查通过；市场218、单商品等待0／谷物hover123／移出0，州433全部248／输入107／输出141，原生绿与业务色恢复原段数。市场量25963.19216／510.99MU、州433量3548.63121全程保持。英文原版首行Artillery为66段，中文所选第二行加工食品为22段；这是原版本地化排序，不把两次选择当同商品对照。中文额外显示／隐藏实际截图与日志通过，数量不变。每个正常会话仅一次国内分配918陆／93海／9038笔，三处partialSea仍保留已知数量及可确认片段。

四个必需GUI类型按真实type声明计数，跳过注释／字符串；缺失或重名拒绝整个扩展挂载、记录明确原因并保留玩家修改，提示对照freight/native-gui/.defaults。当前9afd实际坏GUI67464中，注释伪声明不能冒充类型，原版四页签／frame0、修改字节保持，正常退出后严格恢复；后续63460重新加载完整GUI通过。16个按钮文字框改为100%边界／居中／鼠标透传，中英截图及实际点击验证通过。

A16–A19当前正式图例／颜色／双语切换子项PASS，A26当前坏GUI拒绝／恢复子项PASS；各项完整矩阵仍PARTIAL。七个我方会话全部正常退出0；原存档／settings／ini／content_load／continue五输入最终字节相同，无新存档、当前无游戏。英文退出仅删除受控language字段，逐字段证明后恢复原中文字节。完整规范化error Counter逐局等于R132同语言基线，未新增扩展错误；每局retirement2实见，不扩为ABA／新设备通过。

整体目标active；三港口末端、运河开放／关闭及权限变化、更多地图／铁路新字段、写任务与提交并发／ABA／新设备、最终同构建帧／模拟三对、压力平台及完整A01–A27继续。颜色／图例／当前双语布局及坏GUI子项已完成，P5／A18–A19和A25整体仍DOING／PARTIAL。R130旧核性能不能替代当前9afd。 证据及R133回滚：E:/Code/v3-analysis/checkpoints/2026-10-28-r134-native-legend/{legend-audit.json,live,logs,before,NEXT.md}。

### 2026-10-28 — R133完成：地区商品／方向／运输方式地图筛选

主体309aed48c5880adf61d83b6883200715dd86291277c72a6c0da7ceaa2cbcb9ba／47369073B，引导71dae36d…／779879B，源码＝dist＝安装＝实际18592加载；完整50 PASS、资源／GUI精确归档／selftest560382／默认隐藏vtx0通过。地区新增商品、输入／输出和陆路／海路地图筛选，州各商品完整数量、国内分配、国际原版记录、需求账及全市场分母保持；切州／关页／商品详情撤帧并重置。 当前原存档暂停59883360／world1／rev4，州433量3548.63121保持；全部248段，输入107／输出141，陆路65／海路183；加工食品22段，输入陆路1／输入海路21。州5恢复完整282段／5506.40802，返回州433及关闭重开248段。丝绸运量0时合法空地图，恢复全部248；原版加工食品详情无自定义线，原版Back恢复完整州视图，市场0恢复40商品／218段／25963.19216／510.99MU。

当前地区地图筛选及切换子项PASS；完整A18–A19仍PARTIAL。仅一次9038笔国内分配与scope数量保持；正常退出／五输入／完整错误Counter均核对。A25本局池635／16每帧／702µs／0超时不代替最终三对；R132卸载恢复属于旧6094，不能自动扩为当前309a全A26。整体目标active，P5／A18–A19及A25整体仍DOING／PARTIAL。新增控件英文布局、较长中文“全部…”标签省略及商品选择指示待完善；颜色模式／线宽图例、旧玩家编辑GUI升级契约仍待。三港口末端／运河权限、地图新字段／并发／ABA／新设备、最终同构建三对及完整A01–A27继续。 失败证据及加工食品误名更正见E:/Code/v3-analysis/checkpoints/2026-10-28-r133-state-filters/state-filter-audit.json。

### 2026-10-28 — R132完成：同构建中英UI及实际卸载／恢复验收

当前主体6094ea6e…／47173344B、引导71dae36d…保持，74份native源未变，复用R131完整50 PASS，不重建。实际英文／中文搜索、分类、连续hover、商品右键／详情／原价与提示、州433→5→433及返回市场通过；对应商品ID和数量逐项相同。实际卸载后仅系统VERSION.dll加载、原版四页签；通过deploy.ps1恢复同对DLL并再启动，中文物流页40商品／218段／25963.19216／510.99MU实见。四局正常退出0、五输入字节恢复、无新存档／无游戏。

当前搜索／分类／hover／州切换及正常关闭子项PASS；A18／A19整体仍PARTIAL，地区商品／方向／运输方式筛选与正式颜色／线宽图例缺实现。A26新增当前实际卸载→无Freight原版启动→重装→实际物流恢复PASS，完整损坏／不可写／升级等最终矩阵未齐。A25新增真实冷步骤软超时2.030／2.039／2.163ms，每局各一次、16／帧／真实池峰1201；保留而不当帧P95证明。四局保护、语言规范化首FAIL与原版英文缺键对照详见E:/Code/v3-analysis/checkpoints/2026-10-28-r132-english-native-ui/ui-audit.json。

### 2026-10-28 — R131：当前预算记录／20轮组合缓存PASS，A25整体PARTIAL

当前6094ea6e…／47173344B安装＝dist＝实际加载，50 PASS。首候选相同请求漏计218复用经实际严格FAIL及目标红绿后修正；20轮4分类hover恢复关闭重开／101完成批次，实际726上传／0失败丢弃、池后16轮平台、每帧最多16／峰值550µs、超时0，稳定66.172秒零上传。卡片／分母与510.99MU保持，海陆线可见；当前budget-audit PASS。8192边界与3ms超时是合成生产策略例，不称游戏满池／实际超时。正常退出0／五输入及存档目录保持／无新唯一错误／无游戏，本局无退休行仍不能加寿命结论。此项补充A18／A19／A23及A25预算缓存场景，不外推全部语言／压力／设备／地图；R130帧三对只属R130，当前最终同构建三对与全矩阵待验，目标active。

### 2026-10-28 — R130：A25指定三对及稳定缓存PASS，整体PARTIAL

当前e4e0583…／47160425B安装＝dist＝实际加载，完整50 PASS。保持25ms响应的精确GUI键轮询修复后，沿用R129同原存档／英国相机／速度3三对65秒完整连续QPC，P95增1.0024／0.9268／0.9308ms、模拟均<5%，指定帧与模拟门槛PASS，原失败保留。20次实际关闭重开逐次只读确认，稳定66.156秒uploads=538不变；全进程private范围15622623232–15623163904 B，只作观测窗口证据。hover／leave／显示隐藏／中文／空结果及原版Goods英文词空结果对照通过，不称英文locale通过。正常退出0、五输入／存档目录保持、无新增唯一错误／无游戏。A25仍PARTIAL：真实池压力、软2ms预算实际超时及最终同构建矩阵未完成；其余全目标门槛继续。详见PERFORMANCE／STATUS／timing-audit.json。

### 2026-10-27 — R129：A25真实三轮性能FAIL

主体b13289d…安装＝dist＝实际加载，统一49 PASS；此为诊断链通过，不能替代性能通过。同原存档／英国相机／速度3三轮各65秒完整连续QPC帧：P95增9.0913／6.6732／9.2609ms，均超过2ms；模拟本场景降幅<5%。仅隐藏箭头仍28.0334、原版商品页20.6771，下一步定位物流页绑定／刷新；控制不替换失败基线，不改门槛。正常退出0、原五输入字节及存档目录保持，无游戏。重载退休2…9有日志，退出退休10未记录，不能加验生命周期全通过。详见PERFORMANCE／timing-audit.json；其余最终矩阵继续。

### 2026-10-27 — R128限定验收：已铺铁路MU统计PASS

当前主体87c2c194…／46997736B安装＝dist＝最后实载；完整46 PASS。独立原始当前type4目录、owned条目与原坐标Python并集对账，34条（24州内）／市场22strip＝510.992966 MU，正式510.99 MU；原版实际2→1不变／1→0删除5条后387.76 MU，同进程重载恢复，正常退出0。最终重装新核卡片／完整中文提示实见、正常退出0；五输入及存档目录保持。已知零、未知、商品／hover不影响基础设施口径的相关回归通过。

同存档同两次拆除／重载的旧核完整错误Counter完全相同；初版不同流程基线FAIL及初局未知退出／晚期已有报文保留，不扩成所有会话零新增。此项增加A13／A14–A17当前铁路基础设施证据，单位仅明确地图MU；未覆盖公里标定、新铁路字段全部改图矩阵、完整运河权限／三港口末端／并发设备／3×60秒性能或最终A01–A27。完整目标继续，历史0.12未解释。详见STATUS／PROFILE和rail-audit.json。

### 2026-10-27 — R127增量：真实目录／ZIP及replace_path场景PASS

当前R1233be8cbcc…无产品变化。相同60文件、17份州定义全改名的目录与用户mod根deflate ZIP均真实挂载，replacement截断州目录至1根；当前新key／geo1500、省份归属及918陆路逐条一致。9030／9062笔各自市场及两州正式量全部与稳定owned一致，陆海运及列实见、UI切换前后完整事实不变；两有效局正常退出0，输入／资产保持、列表恢复、临时ZIP移除，无新增解析错误／存档／游戏。

该代表场景补充A04／A10–A14／A20的当前资源／图绑定／国内模型证据。最初分析目录ZIP异常早退保留，按原版归档定位用户根后通过，不称任意ZIP位置支持；无新ABI／DLL。增删省份／bitmap、所有Mod／旧存档、港口三末端、权限／完整A24／A25及最终同构建矩阵仍待，历史0.12未解释。详见STATUS／PROFILE及zip-replace-audit.json。

### 2026-10-27 — R126增量：实际省份归属／全量陆路端点重绑场景PASS

当前仍R1233be8cbcc…，无产品变更。两个现有完整11／8省份集合及对应地理／历史ownership交换后，19个唯一几何见证完整互换、16个同数字省份ID对应不同物理见证；全部918陆路按physical key精确满足新州端点置换，其中12条重绑，无多／缺。两份有效局的完整市场及两州量、9019／9022笔均与各自稳定owned重放一致，实际市场／两州地图和local_goods可见。交换后UI切换前后完整探针输出不变。

初版夹具遗漏owned_provinces由私有校验拒绝，不称生产修复；仅修夹具，失败保留。三局正常退出0、原输入／原43资产保持、列表恢复，无新存档／新增错误／游戏。本场景补充A04／A10–A14／A20的真实归属／图绑定／统计证据，不扩为增删／拆并省、bitmap邻接、ZIP／replace_path、旧存档迁移／所有Mod或最终A24／A25矩阵。历史0.12未解释，证据STATUS／PROFILE及province-audit.json。

### 2026-10-27 — R125增量：实际geo重编号／当前图场景PASS，完整改图仍PARTIAL

当前仍R1233be8cbcc…、无产品变更。真实目录Mod将当前新key／州0／定义0的geo20→1500，并同步6点／12连接／5类locator，原坐标／省份／strip身份保持。实际VFS采用Mod，生产副本无对应旧点／key；正式州353800627fixed及全局8980笔与owned精确相同，市场223段／州234段海陆同屏实见。市场总量25790.44463为正式观察值，外部未独立打印该总量，不称全市场对账。正常退出0／退休2，原输入／资产保持，列表恢复，无新增错误／存档／游戏。

补充A04／A10–A14／A20中的本州geo重编号和新图采用证据。省份拓扑、ZIP／replace_path、旧存档迁移、所有Mod兼容及其他A24／A25最终门槛继续；历史R1200.12差异仍未解释，当前局通过不能替代其修复。证据STATUS／PROFILE R125和geo-audit.json。

### 2026-10-26 — R123增量：国内枚举稳定性／当前真实州身份Mod数值PASS

当前主体3be8cbcc…／46613962B已安装及实机，完整46项／资源／GUI／GPU／自检／默认隐藏通过。等价州／陆路枚举曾改变最终分配及平行物理段，回归先FAIL后PASS；现按当前完整ID确定owned图顺序，冷重算与缓存一致。

真实目录Mod改key并令运行州433→0、定义17→0，geo20／省份／样条保持；正式州0列与244段实见，357465715fixed与owned精确同值。正常推进两个原生步后9053→9030笔、国内23019.376→23017.375与生产相同；市场25881.45827→25879.45727／219段更新。正常退出0，原输入／Mod列表保持或恢复，无新存档／新增错误。

这补充A10–A14／A20中本场景的身份、模型、视图和缓存证据，不扩为完整改图、geo重编／省份拓扑／ZIP／replace_path、设备／性能或最终同构建全矩阵PASS。旧R120另一新局0.12差异原因仍未确认，原失败保留；不能由当前新旧算法都吻合推断历史已修复。详见STATUS／PROFILE及checkpoint/current-audit.json。

### 2026-10-26 — R122增量：实际目录覆盖／同key几何场景PASS，完整改图仍PARTIAL

当前仍R12090e577c3…；真实lower／upper两Mod以正反顺序覆盖同一splnet，仅改London港口中间控制点。实际VFS根顺序、当前原生副本、owned业务ribbon点和正式218段曲线分别采用优先资源；撤销后原版恢复。三次总量25963.19216／国内23098.111且9038笔模型量一致，正常退出码0、原输入与Mod列表恢复，新增错误行0。证据STATUS／PROFILE R122和map-mod-audit.json。

这为A04／A10–A14／A20的当前目录覆盖与同key几何场景补充实机证据，不把完整改图任务改为PASS：州key／定义与运行ID、省份拓扑、ZIP／replace_path、进程内图变化仍待。A24／A25及最终同构建全矩阵保持原边界。

### 2026-10-26 — R120当前增量：原生析构退休事件已实现／实机，A24整体PARTIAL

当前主体90e577c3…／46338126B，安装＝dist＝实际加载，统一46 PASS及资源／GUI归档／自检／默认隐藏通过。精确00811660原生world析构入口在管理器释放前退休自有epoch／邮箱，保留首暂停请求；同地址合成例先FAIL后PASS，外来／重复／晚到／UI先行回归通过。

我方400实际直接重载／菜单退场／正常关闭分别事件2／3／4，前两次首暂停快照已交接且218段／25963.19216恢复，州433列248段及实际客户区缩放通过；关闭码0，原输入不变。证明的是当前析构路径与策略；本次真实地址均变化，不能声称已观察实机同址重用。周期日志无旧copy／handoff及恢复前SUBMIT，但每条检查与原生调用之间的并发边界、新设备、其他重置路径仍待，A24不改为完整PASS。详见STATUS／PROFILE及checkpoint2026-10-26-r120-retirement-events/retirement-audit.json；下方其他hash是历史证据。

### 2026-10-26 — R119当前构建增量：A24具体场景PASS，整体PARTIAL

当前主体aff98195…／46324762B（安装＝dist，同R118完整46 PASS构建），本轮没有源码／DLL变更。下方64ab24f4及旧表均为历史证据；当前完整状态取STATUS，不沿用旧“重启才能恢复”的产品描述。

| A24范围 | 当前结论 | 证据与限制 |
|---|---|---|
| 直接重载原英国存档 | PASS（本场景） | 自主47988同进程world1→2，当前采集/原生池校验后218段／25963.19216恢复，uploads218保持 |
| 主菜单退场／新游戏及返回原存档 | PASS（本场景） | 菜单phase0/frame0，法国新游戏world3市场2／9878.27752／141段／8990笔模型；Load英国world4市场0／25963.19216／218段及州433列248段。不是第二份已存文件验收 |
| 最小化及实际客户区resize | PASS（本场景） | 法国窗口2560×1440→1920×1080→原尺寸，量及箭头保持，矩形恢复；非新设备／设备移除 |
| 可靠退休事件／地址ABA／新设备恢复 | 未完成 | 周期SUBMIT日志在三次退役至恢复间均0，但不能证明逐帧退休空窗或同地址复用；新设备仍无实测／安全重建证明 |

会话正常退出，原存档／设置／ini字节不变，无新存档；证据checkpoint2026-10-26-r119-world-lifecycle/{lifecycle-audit.json,live,NEXT.md}。A24整体保持PARTIAL，其他项目不由本增量自动改判。

### 2026-10-24 — R102增量：海路高度修复用户实机确认，A10整体仍PARTIAL

主体51ec975f…已部署，按实时_WaterHeight夹高。用户原截图证明旧构建近景海运消失；实际VS GPU64场景复现旧错并通过修复、两个负对照拒绝，完整入口/资源/GUI/自检PASS。用户随后明确回复“海路高度修复成功”，该近景故障实机通过；未扩展到长路径/接缝等其他几何场景，A10整体保持PARTIAL。下方64ab24f4等旧矩阵为历史构建证据，当前版本与限制作STATUS顶部为准。

### 2026-10-24 — R90：真实原版市场身份已接采集（新实机验收中）

- 静态原版State.GetMarket链与RTTI/独立Capstone复核：普通州经owner country取得market，特殊州经MarketArea取得market，再验证原生CMarket。GUI包装保存同一个原生market ID；不需要猜GUI与快照映射。
- 只读实机：1147/1147州生产解码器通过（0失败，3特殊分支）；中英格兰433/owner1→market0，与不列颠市场相符。旧+0x48分组52而真实市场305；1143州数值不同，35个旧分组跨多个真实市场。旧分配的“市场隔离”实机结论作废，保留历史不扩展。
- 新state_market.h复制并复核对象类型、完整typed ID代数、数据库边界、owner/area分支和复制期间变化；采集失败撤回正式数据。新增state_market_tests接统一入口，覆盖零ID、未知、越界、类型错、回收handle和变更，独立测试PASS；生产解码器只读全州PASS。
- R89现场稳定性：西南诸郡工作量可见，多个30秒刷新均重新获得owner15/frame1，原生40ribbon的uploads累计保持40（>60秒）；这只验证异步/几何复用，不证明旧市场分组正确或完整性能门槛。紧凑数字的padding省略号已改为先安全heap写入再对现有entry赋未填充短值。
- 候选主体93b6ec29e9a0114771c9245293a6b0bbaea286704485e2292ce8858cd64d985d（33,038,179 B），资源/实际GUI归档PASS；完整入口含新增州市场校验全部PASS；经deploy.ps1部署，安装=dist；自主创建的暂停会话55652已正常退出，未推进日期/保存。备份2026-10-24-r90-native-markets，all-state-market-live.txt/production-decoder-live.txt/static-assembly.txt为私有证据。



## 当前构建权威矩阵（build `64ab24f4…`，2026-10-24）

判定口径：**PASS 只覆盖该行写明的范围**；同一 A 项在旧轮次的表现不自动扩展到当前构建。证据一律可定位到文件或日志行。下面各“增量”段落仅作溯源。

| ID | 检查 | 状态 | 当前证据与限制 |
|---|---|---|---|
| A01 | 原始材料与现状审计 | PASS（历史） | 四文档 + 仓库文档 + 源码已完整读过；本轮未重做 |
| A02 | 构建与导出 | PASS | 可复现构建（`-Wl,--no-insert-timestamp`，两次一致）；原生回归全 PASS（含新增 `publish_key_tests`）；`verify_embedded` PASS；GUI 归档 Python/C++ 逐字一致 |
| A03 | 部署 | PASS | 无游戏进程时 `deploy.ps1` 安装，安装＝dist（`64ab24f4…`）；卸载／回滚演练未按当前构建重做（见 A26） |
| A04 | 能力门控 | PASS（离线） | 未知 GUI hash／缓存损坏／缺资源一律拒绝（`test_native_gui.py`、`test_native_gui_runtime.ps1`） |
| A05 | 原版入口 | **PARTIAL** | 物流页签／商品列表／商品详情可用；**正式数值从未发布**（`publish document=1` 始终为 0），因为市场身份未打通 |
| A06 | UI 原行为 | PARTIAL | 原版价格／排序／右键／商品详情路径保留，并有补丁断言；未逐项实机复测 |
| A07 | GUI 生效 | PASS | 归档挂载、`prepared=1 mounted=1`；**实机 `error.log` 中 `ironroad`／`margin_right`／`not handled` 命中 0**；不支持的 `icon margin_right` 已移除并加回归断言 |
| A08 | ABI 与对象 | PASS（历史） | `PROFILE.md` 全链静态＋实机证据；当前构建未重跑 ABI 探测 |
| A09 | 单线与 pass | PASS | D3D11 `selftest`：ImGui/overlay/readback 通过、默认隐藏 `vtx=0` |
| A10 | 几何贴合 | PARTIAL | GPU 八档宽度×九色板、像素着色器、方向全部 PASS（`test_flow_gpu.py`）；相机缩放／长路径／接缝未逐项实机 |
| A11 | 河流与海岸 | PASS（离线） | `spline_tests` UK–Sweden coastal、hub 港口/海岸回退；实机未逐项 |
| A12 | 铁路与断路 | PARTIAL（R128当前连接证据） | 生产已用当前已建连接，R128实际拆除／重载及MU去重长度PASS；三港口末端／完整新字段改图矩阵待续，不将旧未接描述当现状 |
| A13 | 快照安全 | PASS（读取窗口） | 只读复制＋复制期间变化拒绝；实机 `states=1147 orders=25396 goods=43 markets=52`；**不是原子快照** |
| A14 | 数量与守恒 | PARTIAL（同源正式数量） | 原版真实市场及州作用域已闭合；国际原版1e5／国内模型量、独立对账与守恒已有证据，当前切色统计不变；全场景与原子性仍待，旧state+48推导非正式桥 |
| A15 | 模型来源标识 | PARTIAL（正式双语子项PASS） | 当前国际原版／国内模型标签及已知零／未知--实见；最终完整多场景继续 |
| A16 | 需求与宽度 | PARTIAL（R134正式图例PASS） | 八档数量阈值／实际比例中英图例及生产回归通过；宽度只取最终商品数量，GPU历史通过，最终相机／几何全矩阵待续 |
| A17 | 颜色与统计 | PARTIAL（R134正式子项PASS） | 当前9afd三颜色模式及双语图例／hover，固定全市场数量尺度与完整统计不变通过；单商品未选为空图，原生统一绿非容量 |
| A18 | Hover/筛选 | PARTIAL（R134当前双语子项PASS） | 中英各21场景：商品／方向／颜色、等待空图／hover离开恢复、完整列及分配不变；运输组合沿用R133证据，最终全矩阵继续 |
| A19 | 关闭/owner | PARTIAL（R134当前子项PASS） | 两语言关页／重开和原上下文拒绝，显示隐藏通过；当前退休日志实见，不称逐帧延迟／ABA／全寿命设备通过 |
| A20 | 三层缓存 | PARTIAL（R135当前子项PASS） | 当前9afd20轮分类／三色／hover／关闭重开，后16轮1721零新增；稳定66.313秒零上传，完整压力／失效矩阵继续 |
| A21 | 冷批次 | PASS（模型层） | 真实 1117 州冷构建 127–155 ms；非逐帧计时 |
| A22 | 有界池与失败 | PASS | 离线 8192 上限与失败抑制；实机曾观测 `native pool 989/8192`、0 失败 |
| A23 | 内存平台 | PARTIAL（R135当前20轮PASS） | 后16轮池1721稳定／private无持续增长、末66.313秒private固定15880417280B；全进程窗口不证明全部分配器／设备／实际满池 |
| A24 | 生命周期 | PARTIAL | R120已接真实world退休并恢复正式视图，历史重载／菜单／resize通过；当前七次正常退出均retirement2，真实ABA／提交并发／新设备仍待，不沿用需重启旧门 |
| A25 | 性能 | PARTIAL（R135当前指定三对PASS） | 同9afd三对65秒P95增1.3317／1.3091／1.1830ms、模拟最大降0.774997%；172批次／16每帧／1161µs峰／0超时，其他工作负载与最终全矩阵继续 |
| A26 | 回滚 | PARTIAL（R138交付引导隔离子项PASS） | 实际引导缺失／损坏126／193、正常主体0及系统版本转发在隔离进程通过；R137当前卸载重装实载、资源保留通过；游戏内异常、不可写／完整路径／升级矩阵继续 |
| A27 | Freight 中转需求 | PARTIAL | 模型目标／满足／未满足、来源贡献及非递归回归、历史原版账本可见；一期不另建供给池／游戏商品，最终当前多场景端到端继续 |

> 当前版本与逐项边界以上方R134和STATUS为准。市场身份及正式UI已接通；剩余关键缺口是端点／通行变化、全写任务与提交安全、ABA／新设备、最终同构建性能及完整交付矩阵。历史表不得提升为当前全套通过。

## 历史增量（溯源用，结论以上表为准）


## R84i 增量（2026-10-23 00:05）：GUI 属性错误清零与列宽/表头格式

构建：主体 `64ab24f4338a092b1a3da3c94a9fca17f87cf752204825b90e09d05c464f691d`（32,072,202 B）；安装＝dist；GUI 补丁、C++/Python 逐字一致、`verify_embedded` 全部 PASS。

| 范围 | 结论 | 证据与限制 |
|---|---|---|
| A07 GUI 不支持属性 | PASS（实机） | 移除 `icon margin_right` 后，新会话游戏 `error.log` 中 `ironroad`／`margin_right`／`not handled`／`Error setting properties` 命中 **0**；`test_native_gui.py` 加断言防止回归 |
| 州面板列宽 | PASS（实测尺寸） | 原版行 480 px；补丁后行 52/52/78/92/92/54 + Freight 60 = 480；表头 42/42/42/58 + 100/100/90 + 60 = 534（≤540），“价格差异”列加宽回 90 |
| 表头格式一致性 | PASS（离线 + 实机无报错） | 州“货运”与市场“市场价格／工作量／占比”均改为原版同款 `sort_button`（`align = hcenter|nobaseline`），省略 `onclick` 时引擎接受 |
| 市场页列收紧 | PASS（离线） | 两列 90→70、wrapper 180→140、标签“工作量／占比”（EN `% of freight`） |
| 结余列前移 + 结余列头 | BLOCKED | 原版商品行固定 `@panel_width`=540 px，`last_column` 已被价格块 210 + 右锚定结余 80 占满；需先决定牺牲哪块宽度 |
| 州列表 Freight 列实机外观 | 待用户确认 | 尺寸已按原版总宽重算并部署，仍需肉眼确认行内数值不再被裁 |


## R84e 增量（2026-10-23 23:26）：重复 null 发布修复

构建：主体 `01d2d708063614046f2e09e0e8b8562b2e52f34c322d00c560d20d5d9482b38a`（32,079,882 B）、引导 `71dae36d…`；安装＝dist；完整入口零失败。

| 范围 | 结论 | 证据与限制 |
|---|---|---|
| 重复 null 发布 | PASS（实机） | 修复前最近 400 行有 399 条 `publish document=0`（约 65/s）；修复后同一存档整个会话仅 4 条，且都紧跟 `real economy snapshot accepted`，随后 `UI-thread variables published` 正常出现 |
| 发布去重回归测试 | PASS | `native/test/publish_key_tests.cpp` 接入 `build_tests.ps1`：未发布键不相等、null→null 相等、null 跟在文档后必发、六个作用域字段各自变化必发 |
| GUI 变量未变旧 | PASS（间接） | 初始 waiting／作用域文本仍会写入，真实状态变化仍触发发布；当前无正式 owner 激活，故“数值残留”场景无法构造，待身份打通后复验 |
| A02／A03 构建与部署 | PASS | 完整 build、`verify_embedded`、入口零失败；无游戏进程时 `deploy.ps1` 安装，安装＝dist |


## R84d 增量（2026-10-23 23:10）：实机读档往返

| 范围 | 结论 | 证据与限制 |
|---|---|---|
| A24 世界替换识别与安全失效 | PASS（实机） | 用户手动“回主菜单→重载 autosave”：`world changed: invalidating economy snapshot, next world=2`；随后新 world 重采集 `states=1147 orders=25396 goods=43 markets=52 primary=33`，与读档前一致；全程 `owner=0`、无旧文档/旧帧提交、进程存活无崩溃 |
| A24 读档后恢复正式视图 | FAIL（未实现） | `InvalidateEconomy` 置 `m_nativeWorldRetired=true` 后全仓库无 `store(false)`；9 处原生路径以其为门，范围文本变 `map unavailable after reload; restart required` → 必须重启才能恢复，正是目标禁止的做法 |
| A24 resize／最小化／设备恢复 | NOT_RUN | 本轮未测 |
| 州对象→市场对象指针（复测） | FAIL（结论加固） | 在**完整采集**（`orders=25396 goods=43`）上复跑，12 个指针仍全为容器 vtable `0x624690`，无 `0x045850B0` |
| `+0x374` 候选 | 弃用 | 三次观测两次 `0xFFFFFFFF`（本次面板已开 `marketOpen=1` 仍全空），写入时机不可控 |
| 玩家国家／存档 | 已确认（只读） | 存档 `meta`＋`continue_game.json`：大不列颠(GBR)、1.13.11；`gamestate` 为二进制格式，暂不作证据源 |


## R84c 增量（2026-10-23 23:00）：州→市场对象指针证否

构建：主体 `1c3428e5eacb77546ac755bd3da7217457abe2aa8240264627fd8a99c71a1530`（32,070,154 B）、引导 `71dae36d…`；安装＝dist；完整入口零失败 `native/build/entry-round84c.log`。

| 范围 | 结论 | 证据与限制 |
|---|---|---|
| A02／A03 本轮构建与部署 | PASS（本轮） | 完整 build、`verify_embedded`、完整入口（含 selftest／默认隐藏）零失败；无游戏进程时 `deploy.ps1` 安装，安装＝dist |
| 州对象→市场对象指针 | FAIL（已证否） | anchor 州 `id=0`、`state+0x48=112`；`+0x00..0x400` 内 12 个引擎指针全为容器 vtable `0x624690`，无 GUI 市场对象 `0x045850B0` → 州与市场是整数 id 关系。证据 `native/build/round84c-state-market-probe.txt` |
| 市场与地区列的同一闸门 | 已确认 | `CopyNativeStateRoster` 用 `orders[].market`（源自 `state+0x48`）建 roster，`NativeStateOwner` 以其命中为激活前提 |
| 静态证据可用性 | 受限 | Ghidra 对 getter `0x225AC90` 一带无函数体、脚本执行被禁用；文档中 resolver `0x10571F0` 实为构造 `GetSuppressionEffectDescriptors` 字符串的函数（记录已过期） |
| A05／A07 正式数值、地区 Freight 列 | NOT_RUN | 仍由市场身份阻塞；`NativeMarketOwner`／`NativeStateOwner` 均未激活，`publish document=1` 为 0 |


## R84 增量（阶段1基线固定＋阶段2双DLL实机加载）

构建：引导 `71dae36d7fd52cd9c6eb6de9b63317dd697ffacd80039c82023bf28c08b7962e`、主体 `4ac3a0e8873dbe75ce945438a1ce476c8583e29d1f3f8d610245ac19e29f2980`（可复现，两次构建一致，安装＝dist）。

| 范围 | 结论 | 证据与限制 |
|---|---|---|
| A02 构建与导出 | PASS（本轮） | 两次完整构建字节一致；完整入口零失败 `native/build/entry-round84.log`；`verify_embedded` SHA256=4ac3a0e8… |
| A03 部署 | PASS（本轮） | 无游戏进程时 `deploy.ps1` 安装，安装＝dist，无遗留 `.tmp`／备份冲突 |
| A04 能力门控 | PASS（离线） | 未知 GUI hash／缓存损坏／缺资源拒绝；本轮未在新构建上实机重测 |
| 双DLL加载路径与系统导出转发 | PASS（R83 构建实机） | 进程模块 `binaries\VERSION.dll`（代理）、`binaries\freight\freight.dll`（主体）、`system32\version.dll`（转发目标）；`loader.log`＝`loaded freight.dll (error=0)` |
| 配置／资源／日志目录落点 | PASS（R83 构建实机） | `runtime data=…\binaries\freight\data`；GUI 归档取自 `freight`；该会话无 LocalAppData 写入 |
| 嵌入资源早期挂载 | PASS（R83 构建实机） | `mounted BEFORE VFS configuration freezes`；`ready BEFORE gfx/models .asset enumeration` |
| 默认关闭 ImGui | PASS（实机＋harness） | 实机 `drawdata lists=0 vtx=0`；`test_debug_default` vtx=0 |
| 经济只读采集（进存档） | PARTIAL（只读读数） | 1147 州／25317 订单／43 商品／52 市场（primary 33），两次采样一致；快照以 `(diagnostic)` 接收，不激活正式 owner，故不是正式 UI 数据 |
| A05／A07 原版物流入口正式数值 | NOT_RUN | 该次会话市场面板未打开（`panel=4294967295 tab=default`），`publish document=0` 全程 |
| 市场/州面板原生解码链（活进程只读探针） | PASS | `phase=4`、`idler/gfx/handler` vtable 与 `handler+0xd8` 全 MATCH、`panel count=56`；slot 9=`market_panel`（`tab="ironroad_logistics"`）、slot 14=`states_panel`（`state=433`、`tab="local_goods"`）；市场置顶后 `marketOpen=1`、`visibleGoods=40`。证据 `native/build/round84-panel-probe.txt` |
| 市场身份与快照一致性 | BLOCKED（已排除一条假设） | GUI 市场注册表是**惰性 GUI 本地空间**：未开面板 `count=2`（`0`,哨兵）、开面板后 `14`（`0 1 2 3 4 7 8 9 10 13 14 19 22`）、另一会话 `45`；快照 `state+0x48` 为 `31..116`，基本不相交。`+0x374` 候选被第二样本否掉（本次 `0xFFFFFFFF`，上一会话 `33`） |
| GUI 市场注册表可读性 | PASS（只读探针） | `module+0x059684A0` → count/capacity/lock 与条目 vtable 全部校验通过；`native/build/round84-panel-probe.txt` |
| 地区 Freight 列 | NOT_RUN | `NativeStateOwner` 未接入 overlay 运行时；探针已证明州面板可读 |
| 新部署构建实机启动 | NOT_RUN | 22:19 部署，尚无启动记录 |
| A01／A06／A08–A27 其余项 | 不变 | 见下方历史矩阵，本轮未重测 |


## R74 增量（候选未部署）

| 范围 | 结论 | 证据与限制 |
|---|---|---|
| 原版筛选语义／缓存定位 | PASS（静态＋未就绪样本） | 分类排除字节、Unicode／关键词路径、原版返回缓存地址；实机仅dirty1／0rows，回放正确拒绝 |
| 搜索／分类→模型视图 | PASS（fixture／模型） | 中文query沿用native IDs、空与缺失分开、过期／busy／错误身份拒绝；分配／分母／heat／Freight不变，排序复用 |
| A18原版实际搜索／分类交互 | NOT_RUN | 没有正向native缓存／新DLL运行／实际点击结果，500ms同步仍待改善 |
| A02–A03候选构建／资源 | PASS | 13项回归、完整build、verify_embedded；1a97a6823e2fc72fa7f1f2178807803a4001fc89faee3ccd8df3fdc8b0223cfd，非实机验收 |

## R73 增量（候选未部署，运行中仍R69）

| 范围 | 结论 | 证据与限制 |
|---|---|---|
| 原版面板所有权链／关闭态读取 | PASS（观察＋回放） | PID52316整条类型／bar／panel／history0／closed1／default／market无效；实际解析器回放通过；没有打开态图像 |
| 面板状态→市场／owner／取消 | PASS（离线） | 新market_panel_tests＋会话桥回归，13项原生测试整体PASS；当前面板market代替primary，关闭不影响其他owner |
| A18–A19正式打开／切换／关闭 | NOT_RUN（新DLL交互） | 源码候选已接线，未部署；500ms同步不证明逐帧立即取消；正式hover／筛选仍缺 |
| A02–A03构建／打包 | PASS（候选） | e0a2e7863db4f016091888b4e6d885678fae3149db8a12e6e4f22b58c7a3d1a6；完整build／13项回归／verify_embedded；非新DLL实机通过 |
| A24完整生命周期、A27Freight机制 | 未完成 | 仅读取UI状态不验证原生对象跨world回收，也未完成Freight独立供给 |

## R70 运行增量（运行版本 R69，候选修复版本 R70 分开）

| 项目／层级 | 结论 | 范围与证据 |
|---|---|---|
| A09 正式owner native提交 | PASS（提交层） | PID52316运行R69、调试UI启动vtx0，真实37 ribbons持续SUBMIT；不等于实机可见图像 |
| A20 热态零新增上传子项 | PASS（本场景日志） | 热态35采样／170.453秒uploads恒37，submitted增加；runtime-summary-round70.json；hover／重开／排序场景未覆盖 |
| 地图／新Freight行实机可见 | NOT_RUN | Hidden游戏窗口未被工具枚举，已请求用户显示；没有图像证据 |
| 重复文档发布回归 | PASS（R70离线） | 新测试先FAIL，修复后11项回归PASS；同一generation只发布一次，frame保留；候选未部署 |
| A24完整生命周期 | NOT_RUN | R69检测world替换后仍暂时停用native对象访问并要求重启，未完成安全跨world恢复 |

## R67 增量验收（完整矩阵历史结论不自动扩展到新 DLL）

候选／已部署 DLL：`52009510d0ff6c64af6e03b77ab109c7d9c6d5bafd56ce101116a2b6fb1c7bd9`。

| ID／层级 | 结论 | 本轮证据／剩余工作 |
|---|---|---|
| A27 需求核算与反例（模型层） | PASS | `freight_tests`：完整目标、缺货／断路、独立系数、商品贡献、市场／州隔离、零／缺失、非递归与过滤不变；`freight_engine_tests`：系数／分类缓存失效、原商品几何身份不变；`logistics_ui_tests`：复制文档、正式变量拒绝合成／无效／缺失 |
| A27 Freight 中转机制整体 | NOT_RUN | 核算层不代表独立供给机制；是否注册游戏商品／供给来源尚未确定，无完整端到端证据 |
| A02／A03 本轮构建与打包 | PASS | build、verify_embedded、11 项原生回归；仅 DLL 经 deploy.ps1 部署 |
| A07 新 UI 行静态／归档层 | PASS | GUI 原生补丁一致、资源／CRC／新变量绑定校验通过 |
| A07 新 UI 行实机可见 | NOT_RUN | Windows 控制工具启动失败，无游戏进程；不能据归档／部署推断可见 |

以下为 R51 历史矩阵，版本与范围见原记录。

版本：EXE `7f53…f811f`（1.13.11）；当前候选/已部署 DLL `5e74477a73f50e0924f72b449ab2986f8b619d698d623a91e23544e9984691fc`（R49 构建；R51 诊断构建 `a25f78ae…` 仅增加 `probe_market` 调试日志，默认关闭）。结论：PASS / PARTIAL / NOT_RUN / BLOCKED / NOT_APPLICABLE。未实测项不得写 PASS。

| ID | 检查 | 结论 | 证据 / 说明 |
|---|---|---|---|
| A01 | 原始材料与现状审计 | PASS | 完整读取；`WORKLOG.md` R0 |
| A02 | 构建与导出 | PASS | `native/build.ps1`；`native/build_tests.ps1` 11 项 PASS；`verify_embedded.py` PASS |
| A03 | 单 DLL 部署 | PASS | `deploy.ps1` 安装/卸载均验证；R47 回滚后重新部署成功；无 playset |
| A04 | 能力门控 | PASS | 未知 GUI hash/缓存损坏/缺资源测试；`test_native_gui.py` |
| A05 | 原版入口 | PASS | 市场第五“物流”页签 + 商品列表 + 商品详情；正式页签显示真实数据（R44/R49） |
| A06 | UI 原行为 | PARTIAL | 原版价格/买卖/详情保留；未逐项复测全部原版交互 |
| A07 | GUI 生效 | PASS | GUI 归档挂载；`test_native_gui_runtime.ps1` 验证 Python/native 补丁一致 |
| A08 | ABI 与对象 | PASS（历史） | `PROFILE.md` 全链证据；当前 EXE 未重跑 |
| A09 | 单线与 pass | PASS（历史） | 短线、校准对象移除后仍绘制、单次提交 |
| A10 | 几何贴合 | PARTIAL | 真实 989 条 ribbon 上屏（R41）；GPU 宽度/方向测试 PASS；相机/长路径/接缝未逐项 |
| A11 | 河流与海岸 | PASS（离线） | `spline_tests` UK–Sweden coastal；hub 港口/海岸回退；实机未逐项 |
| A12 | 铁路与断路 | PARTIAL | 未建铁路排除、断路不伪造、hub 回退（离线）；真实 `builtRailEdges` 未接入，页签显示 `--` |
| A13 | 快照安全 | PASS | 默认采集（R44）：1117 州/28554 订单/49 商品；reader 只读；世界失效（R49） |
| A14 | 数量与守恒 | PASS（模型+市场作用域） | 真实快照按 `state+0x48` 市场分组（59 市场）；primary=最大市场（118/77 州）或 `market_override`；目标/分配/未满足分离；`freight_tests` 守恒 |
| A15 | 模型来源标识 | PASS | 页签显示 `model allocation (market N)`（R53）；无数据时 `--`；市场作用域已标注 |
| A16 | 需求与宽度 | PARTIAL | 八档宽度 GPU PASS；真实 ribbon 可见但宽度档低（全局摊薄）；正式过滤/悬停未接 |
| A17 | 颜色与统计 | PARTIAL | 固定热度尺度、分母、需求统计（离线）；真实彩色 ribbon 可见；正式 hover 未接 |
| A18 | Hover/筛选 | PARTIAL | 会话层 hover/关闭恢复、缓存键测试；原版 hover 未接 |
| A19 | 关闭/owner | PARTIAL | 多 owner 仲裁离线；正式 owner 常驻（无面板可见性检测） |
| A20 | 三层缓存 | PARTIAL | 路由/视图/原生缓存离线测试；运行中命中计数可见；零上传未逐帧验证 |
| A21 | 冷批次 | PASS | 真实 1117 州冷构建：wall 127–155 ms、989 ribbons（R43/R44）；非逐帧计时 |
| A22 | 有界池与失败 | PASS | 离线 8192 上限/失败抑制；实机 `native pool 989/8192`、0 失败 |
| A23 | 内存平台 | PASS | 3 分钟 18 次采样 7278–7302 MB（±24 MB），后段下降（R48） |
| A24 | 生命周期 | PARTIAL | 世界失效注入 PASS（R49）；真实读档/主菜单往返未完成 |
| A25 | 性能 | PASS（worker） | 分配 127–155 ms、view ~7 ms（8 线程）；UI 可交互 |
| A26 | 回滚 | PASS | R47：卸载 DLL 后游戏正常进图；重装后功能恢复 |

## 说明

- “PASS（离线）”只证明模型/策略测试通过，不证明游戏内行为。
- 当前已知缺口：market 作用域（全部州标为 market=1）、真实 `builtRailEdges` 未接入、原版 hover/筛选未接、正式 owner 无面板可见性检测、真实读档往返未演练。
- 真实数据证据：`native/build/r44-final.png`（页签数值）、R41 日志（989 ribbons）、R47/R48/R49 日志。
