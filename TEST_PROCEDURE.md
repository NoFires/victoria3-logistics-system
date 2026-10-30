# In-game test procedure

### 2026-10-29 — R138完成：实际交付引导的缺失／损坏主体隔离回归

当前主体9afd299a…／47581219B、引导71dae36d…／779879B，源码＝dist＝安装＝R137实际加载；产品源码和两份DLL未变、本轮未部署或启动游戏，完整统一入口58 PASS／零FAIL／退出0。

python -B tools/test_freight_loader.py现在对临时重新编译引导及存在时实际dist/version.dll分别测试有效stub／缺失／损坏，隔离不同cwd、中文空格路径，验证17导出、系统版本数据及错误码；缺失例起始无freight目录。完整native/build_tests.ps1调用该脚本，另有--runtime验证真实双DLL资源及配置迁移。

直接复制实际交付version.dll到含中文／空格的独立路径，工作目录放另一目录及同名无效诱饵；缺少主体和整个freight目录时引导自动建立目录，加载失败码126；损坏PE时193；实际健康主体成功加载且码0。三者均在独立进程中保持系统版本信息查询可用，不是游戏进程内缺失／损坏实机通过。 扩展已有tools/test_freight_loader.py：统一入口同时对重新编译引导和dist实际引导分别验证有效stub、缺失、损坏及错误码0／126／193；17个导出存在、系统版本数据逐字节一致；另外原有--runtime使用真实双DLL验证新配置优先和迁移。无玩家运行时新依赖。

隔离测试及统一入口后两份实际安装与dist哈希相同，原save／settings／ini／content／continue五输入及存档目录保持、无游戏。首私有审计误以为R134清单包含该测试文件而被断言拒绝，核对清单后改为对比本轮before；失败脚本保留。一次仅用于打印的私有脚本遇GBK输出错误，完整测试已结束且结果不受影响。文档归档首版误将保持原CRLF的Python源按Markdown检查而被拒绝，修正后仅重跑归档尾段。

A26新增当前交付引导隔离子项PASS；真实游戏进程中缺失／损坏、目录不可写、完整路径／升级等矩阵仍PARTIAL。三港口末端／运河权限、改图新字段、写任务／提交原子性、真实ABA／新设备与最终同构建A01–A27继续；整体goal ACTIVE。 证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r138-delivered-loader/{loader-audit.json,isolated-results.json,isolated 中文 空间,full-tests.log,before,NEXT.md}。



### 2026-10-29 — R137完成：原版地点查找器对照与当前版本卸载恢复

主体9afd299a…／47581219B、引导71dae36d…／779879B；源码／资源／dist未变，复用R134完整55 PASS与R135同核性能。本轮通过deploy.ps1实际卸载后恢复，最终源码＝dist＝安装＝实际24108加载。

验证原版地点查找器日志：以完全相同原存档／中文／暂停状态、实际模块分别确认stock与当前插件；输入台湾、截图并保存错误日志后点击第一个地区结果，再保存错误日志。按时间前缀规范化Counter，逐项比较差分与两组完整结果；不得仅凭时间相关认定来源。

同原存档／中文／暂停59883360，地点查找器输入台湾并点击第一个地区结果，分别保存点击前后error.log。仅系统VERSION.dll加载的原版27220与当前扩展24108均为点击前0条、点击后新增1条PdxDebugConstants命名索引错误；完整规范化错误Counter相同、点击差分只有该行。R136的新增日志在本机无Freight时可复现，不能直接归因扩展；此结论只限本机EXE／存档／语言／操作，不推断所有游戏版本或其他命名索引错误。

卸载前核对无deploy.ps1会清理的旧文件／playset条目并备份42个非主体文件；卸载与重装之间全部42个配置／缓存／可编辑资源逐字节相同，再次实机启动后仅freight.log／loader.log自然追加。实际模块分别为stock系统VERSION.dll、当前version.dll＋freight.dll；恢复后的市场0物流页40商品／218段／25963.19216／510.99MU实见。 两次我方会话27220／24108均全程暂停、正常WM_CLOSE退出0；五项受保护输入及存档目录字节保持、无新存档和游戏进程。当前retirement2实见，无扩展资源相关报错；不能将正常退出扩为ABA／新设备通过。

A26当前卸载／资源保留／重装实载子项PASS，完整缺失／损坏／不可写／路径／升级等矩阵仍PARTIAL。三处港口末端、运河权限、改图新字段、写任务／提交原子性与真实ABA／新设备及最终同构建A01–A27继续；整体goal ACTIVE。本轮消除了R136这条日志的扩展独有疑点，未修复三处港口缺段。 证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r137-stock-location-control/{stock-control-audit.json,live,runtime-before.json,before,NEXT.md}。



### 2026-10-29 — R135完成：当前同核三对性能与20轮颜色组合缓存验收

当前9afd源码／资源／双DLL与R134归档逐文件相同，复用55检查。P7完整重读00／01／02／03及MASTER§10；保护五输入，启动子进程启用既有IRONROAD_FRAME_TIMING=1并核对实际模块。

沿R129／R130同原存档59883360、英国地标／2560×1440／原图形设置／原生速度3，每臂正常重载同存档，暂停预热30秒后运行65秒；六窗完整连续64.83–64.85秒，窗内无截图／输入、所有尖峰保留。三对关闭→打开P95为17.9185→19.2502、17.8774→19.1865、17.9017→19.0847 ms，增加1.3317／1.3091／1.1830 ms均≤2；模拟最大降幅0.774997%≤5。Present入口间隔不是GPU／显示延迟，根市场关闭还改变原版地图模式，只证明本存档／速度协议。

每次采样后暂停、通过原版载入同原存档；若菜单点击未完成，先实际截图／同PID／开始时间／phase确认，不盲目继续固定点击或重启。R135一次预采样重载超时未计入窗口，实际菜单恢复后重测第三对；closed-2已完整窗口后的runner类型错误不丢弃样本。

随后world6／rev52／date59884134暂停，20轮四分类轮换＋三颜色／单商品hover进入离开／恢复全部／Escape关闭→F4原版市场→点物流，逐轮VM_READ确认closed／market0／tab，176份观察通过。完整市场量26371.75358／510.99MU不因视图改变；与原日期的25963.19216差异来自本轮正常模拟推进，不将跨日期数量当UI不变对照。前四轮建立变体后，后16轮uploads和实际pool均1721不变；SUBMIT的cache是累计命中数，允许增长，不能当对象池。稳定66.313秒uploads1721不变，全进程private固定15880417280B；后16轮private范围15879147520–15892967424B、净减1712128B，属于本窗口平台证据，不证明所有分配器／8192实际满池。

172个实际完成DRAW批次账逐项相等，失败／丢弃0，池与native槽峰1721，每帧尝试峰16，Pump帧／单步峰1161／708µs、超2ms帧0。QPC含调度／driver等待，软预算不作硬实时保证；R134真实2056／2392µs超时保留为各自历史局，不删除或归到本局。

审计SUBMIT uploads是累计上传、cache是累计命中；真实pool与native_slots读取DRAW PERF。原始QPC重新解析并与存储summary逐字段一致，阈值按预先定义三对判断；正常退出后核对五输入／存档目录／错误与退休。

我方54780/start2026-10-28T23:09:37.6659011+08:00正常WM_CLOSE退出0；原存档／settings／ini／content_load／continue五输入字节及存档目录保持，无游戏。重载退休2…6及退出退休7实见，不扩为ABA／新设备通过；完整错误对R134中文无新唯一行，六条Unexpected身份保持、扩展资源错误0，重复读档的次数差异单独保留。

整体goal ACTIVE；当前9afd指定三对、20轮组合平台与稳定零上传子项完成，不再列为当前缺证。A25整体仍PARTIAL：更广工作负载／最终全矩阵不外推。继续三港口末端确证、运河开放关闭／权限、更多地图与铁路新字段、写任务／提交原子性与真实ABA／新设备，以及完整A01–A27和异常安装升级卸载恢复。若后续代码改变，应在最终构建重跑必要门槛。 证据／当前对备份：E:/Code/v3-analysis/checkpoints/2026-10-28-r135-final-performance/{timing-audit.json,budget-report.json,error-comparison.json,live/benchmark,before,NEXT.md}。



### 2026-10-28 — R134完成：正式颜色／八档线宽图例、双语居中及编辑GUI校验

先核对当前9afd／71da实际模块、55完整检查及五输入备份。颜色未选商品用例、缺类型／重名与注释字符串用例、16按钮居中例应先红后绿；统一入口已串行包含C++ GUI实际归档及Python布局／图例阈值检查，共用harness日志不可并发。

业务色保持地区输入绿／输出橙，市场热度使用完整市场固定区段商品数量尺度；单商品模式按原生商品ID取既有稳定五色，未选或移出为空地图，原生外观统一已验证建筑绿。颜色只改视图，不改国际原版记录、国内最终模型分配、需求账、完整行／卡片／市场分母、路径或宽度。八档图例显示实际宽度比例2:3:4:6:8:12:16:24及数量阈值10／50／200／500／2000／5000／20000，提示比例示意、非像素／容量／距离工作量；0不画、未知--。

当前9afd英文67696与中文63460各21项颜色／商品／方向／关页重开检查通过；市场218、单商品等待0／谷物hover123／移出0，州433全部248／输入107／输出141，原生绿与业务色恢复原段数。市场量25963.19216／510.99MU、州433量3548.63121全程保持。英文原版首行Artillery为66段，中文所选第二行加工食品为22段；这是原版本地化排序，不把两次选择当同商品对照。中文额外显示／隐藏实际截图与日志通过，数量不变。每个正常会话仅一次国内分配918陆／93海／9038笔，三处partialSea仍保留已知数量及可确认片段。

每个locale实际展开两面板图例并观察八宽度样本与首末数量tooltip；文字须居中、控件仍可点击。关闭／重开恢复Business；原页Overview撤帧。显示隐藏应看实际地图与生产日志，desired batch计数不直接等于屏幕可见数。

四个必需GUI类型按真实type声明计数，跳过注释／字符串；缺失或重名拒绝整个扩展挂载、记录明确原因并保留玩家修改，提示对照freight/native-gui/.defaults。当前9afd实际坏GUI67464中，注释伪声明不能冒充类型，原版四页签／frame0、修改字节保持，正常退出后严格恢复；后续63460重新加载完整GUI通过。16个按钮文字框改为100%边界／居中／鼠标透传，中英截图及实际点击验证通过。

七个我方会话全部正常退出0；原存档／settings／ini／content_load／continue五输入最终字节相同，无新存档、当前无游戏。英文退出仅删除受控language字段，逐字段证明后恢复原中文字节。完整规范化error Counter逐局等于R132同语言基线，未新增扩展错误；每局retirement2实见，不扩为ABA／新设备通过。

当前21项通过不替代新设备／原子提交、最终帧性能或所有地图；详细操作、截图、模块与检查见证据及R133回滚：E:/Code/v3-analysis/checkpoints/2026-10-28-r134-native-legend/{legend-audit.json,live,logs,before,NEXT.md}。

### R133地区地图筛选复验（2026-10-28）

同309aed48…／47369073B、50 PASS。先核对红绿方向例；统一入口覆盖原版缺几何数量、混合海陆拆段、组合空结果、固定完整列／热尺度／需求及缓存、旧context／重复／未知值、关闭及统计发布。GUI Python/C++精确归档、资源、独立加载及默认隐藏检查串行运行；它们共用harness日志，不应与opt-in计时脚本并发。

正常进入原存档暂停，当前州433本地价格：全部248／输入107／输出141、陆路65／海路183，完整quantity354863121 fixed不变。第二行是加工食品（私有grain*标签误名保留），点击新物流单元后22；输入陆路1／输入海路21；点全部商品／双向／全部方式恢复。丝绸已知0为空图且完整列不变。433→5恢复282／550640802→433恢复248／354863121；Overview、商品图标进入原版加工食品详情都撤图，Back／重开恢复全部248。市场0完整40商品／218／2596319216 fixed／510.99MU恢复。全部操作只视图，不执行税／关税／扩建。

normal WM_CLOSE exit0后核对五输入／存档目录／实际双DLL、完整规范化错误Counter；5秒会话旧行不当作当前帧或逐帧延迟证明。R133中文实见通过，新控件英文／标签省略和选择指示、图例及最终完整矩阵待续。证据E:/Code/v3-analysis/checkpoints/2026-10-28-r133-state-filters/state-filter-audit.json。

### R132同构建双语与实际卸载恢复复验（2026-10-28）

先备份保护五输入，核对74源／双DLL为6094／71da及实际模块。实际English locale与原中文各进入同一暂停存档：clothes／CLOTHES／luxury clothes与衣物／高档衣物逐商品ID及1e5数量相同，四分类总和等于全市场；空结果／清空、连续hover／离开、隐藏／显示保持统计规则。原版价格排序／右键／详情／Local Prices／Applications及逐历史返回、数量和MU提示需实际截图；原版搜索语义不另作字面筛选。

以原生面板当前州433→5→433核对数值3548.63121／5506.40802／原值、248／282／248段，两语言一致；Overview撤帧／市场218恢复。观察日志5秒旧行不可作立即撤帧证明。当前地区商品／方向／方式控件和正式颜色／线宽图例尚未实现，不计为通过。

正常退出后检查精确settings：英文可能只移除显式System.language字段，须先证明除受控字段外JSON完全相同，所有其他输入保持，再按原字节原子恢复；首次严格失败和差异证据均保留，拒绝其他变化。实际卸载只用deploy.ps1，检查非主体偏好／缓存／资源不变；stock正常启动确认仅系统VERSION.dll及原版四tab，再正常退出、恢复语言并deploy同对DLL，最后实际启动核对物流可见并正常退出。R132四局均exit0，最终五输入及存档目录保持；英文三条条约缺键错误在stock复现，本机原始英文文件缺键，游戏原文件未改。证据E:/Code/v3-analysis/checkpoints/2026-10-28-r132-english-native-ui/ui-audit.json。

### R131预算与缓存复验入口（2026-10-28）

当前6094ea6e…／50 PASS。先核对同源生产cache用例的8200→8192／8丢弃、同帧多pass及换请求不复位、3ms一步超时只计一次、10秒间隙排除；完全相同请求须计已完成复用，部分待办不计，失败不重试。原首候选reused0的严格失败与红绿例都保留。

游戏默认关闭计时开关、正常原存档暂停；4分类轮换，逐轮hover首行／离开／再次点原分类恢复全部／Escape关闭→F4市场→点物流，至少20轮，用生产market_panel外部VM_READ逐次确认closed/opened/market0/tab。对DRAW PERF每批核对paths=uploads+reused+failed+dropped（complete时pending0）、visible≤paths、pool及native_slots≤8192、peak_attempts≤16；统计与内存只在低频采样。R131实见101完成批次、726上传、0失败丢弃、池后16轮平台，重开218全复用，峰值550µs／16次，实际超时0；最后稳定66.172秒uploads726不变。

cpu_us为Pump步骤累计QPC，非线程CPU／GPU；峰值／overrun_frames累计于cache生命期，实际未填8192不称原生满池。正常退出后须核对五输入／存档目录／实际双DLL、错误身份和退出码；本局exit0没有退休行，保持寿命边界。当前完整帧／模拟三对需最终同构建另测，R130旧三对不替代。详见PERFORMANCE／budget-audit.json。

### R130性能修复复验入口（2026-10-28）

当前e4e0583…／50 PASS，沿用下方R129完整协议，六臂各65秒、暂停预热30秒、原存档／相机／速度3不变。三对P95增加1.0024／0.9268／0.9308ms、模拟均<5%通过；QPC取窗验证及实际模块／原始尖峰保留。另做连续两商品hover→离开、隐藏／显示、中文“衣物”及空结果，卡片基础统计不变。中文locale的clothes与原版Goods对照均空，英文locale另验。

面板循环须Escape关闭、F4打开原版市场、点击物流tab；用生产market_panel解码的外部VM_READ探针逐次核对marketOpen=0／1、market0、tab=ironroad_logistics，不能把F4从物流切原版Goods当关闭。20次后稳定≥65秒，每5秒记录累计SUBMIT uploads与进程private、phase／paused／date。R130uploads=538全程不变；原五输入字节／存档目录及正常退出0都须最终检查。实际池压力／构建软预算超时与最终同构建矩阵继续，详见PERFORMANCE／timing-audit.json。

### R129性能复现入口（2026-10-27）

当前计时版主体b13289d…，49 PASS；仅在本次测试子进程环境设IRONROAD_FRAME_TIMING=1。按PERFORMANCE R129的同原存档／同英国相机／同设置／速度3协议，暂停预热30秒、运行65秒，外部只读当前machine日期与QPC；每臂正常重载同原存档，窗内不截图／输入，指针停固定journal标题而非地图／屏幕边缘。记录完整原始QPC及thread/world/swap身份；frame_timing.py以窗口边界选择完整连续间隔，--require-seconds 60，最近秩分位数。原生日期速度单独计算，保留tick量化与所有刷新／上传尖峰。

R129三轮帧门槛均FAIL；详见PERFORMANCE表，不得用隐藏箭头或原版商品页单次控制替换三轮基线。优化后必须重新在最终同构建跑三对。首短窗／相机漂移拒绝；自动菜单超时不是进程结束，先复核同一PID／开始时间／phase并观察实际菜单再操作，不因超时重启。最终原五输入字节／存档目录保持、正常WM_CLOSE退出0；退出退休日志缺失仍明确记录。开发者诊断不新增玩家工具依赖。

### R128已铺铁路验证增量（2026-10-27）

先跑含州内、无货流但7 MU真实已建段的用例，确认旧行为FAIL再修至PASS；现有统一入口包含去重／方向／满56位ID、当前市场／州内、跨市场及未建排除、未知与已知零、重排缓存／拆除／同key几何改变、非法句柄／链／NaN与格式检查。完整46 PASS，资源／GUI实际归档／独立加载／默认隐藏通过。

实机只读枚举当前原始type4目录并与owned逐ID核对，用独立Python对原坐标求物理strip并集和hypot；不得把货流经过边或生产长度函数当独立证明。原存档市场应510.99 MU，空搜索不改变该卡片；提示需明确MU、二维折线及非公里。初局后置probe因进程已消失失败，不能冒充筛选前后完整probe一致。

在我方未保存世界通过原版UI将铁路等级2→1→0，一级仍有当前已建连接故长度不变，零级当前删除5条，卡片387.76 MU；同进程正常重载原存档恢复510.99 MU、新world和原数量。正常退出且保护五输入及存档目录。错误Counter须用相同存档及相同拆除／重载协议的旧核作对照：本轮完整Counter相同。最初跨流程严格比较FAIL与初局未知退出／晚期报文必须保留，不得改称完整全局零新增。最终经deploy.ps1恢复R128，再实际启动核对核心／卡片／提示并正常退出0。证据checkpoint2026-10-27-r128-built-track/{rail-audit.json,live}。

## R127 actual directory/ZIP replacement (2026-10-27)

1. Back up five protected inputs and original assets. Create identical directory
   and deflate ZIP fixtures with all17state-region filenames renamed and
   game_custom_data.replace_paths=[map_data/state_regions]. On this exact build,
   put the temporary ZIP under the existing user mod root; static archive lookup
   uses that root plus basename. Never overwrite an existing user Mod.
2. Launch fresh paused worlds separately. Verify exact EXE/loaded DLL paths/hashes,
   actual first directory/ZIP mount and replacement table, allowedRoots=1 for
   state_regions. Reuse production decoders; current key/definition/geo/province
   ownership and all918land records must agree across formats,93sea links known.
3. Close the external process handle before building owned facts/views. Compare
   stable market and both state quantities exactly with formal publication, view
   the native map and local_goods columns. Re-read after UI switches. Preserve
   early startup samples separately; do not demand equal economics across fresh
   worlds without seed control or call an external probe atomic.
4. Normally close own paused sessions, archive logs/error Counters, restore the
   content list by expected hash, remove only the unchanged user ZIP created by
   the test. Verify all original bytes/assets, save directory and no process.

Current checkpoint2026-10-27-r127-zip-replace/zip-replace-audit.json passes both
supported-root formats. Initial external ZIP startup failure and strict finish
guard rejection remain preserved. No product changes or new ABI; R12346PASS
retained. This does not prove arbitrary ZIP locations/layouts or all map Mods.

## R126 actual province membership exchange (2026-10-27)

Use control and swapped private directory fixtures. Exchange two existing
complete province sets together with geographic IDs, city/port/farm/mine/wood
and naval exits. Synchronize the two explicit historical owned_provinces lists;
parser errors alone do not detect a mismatch. Preserve other history effects,
keys, ordering, buildings/population authored input, physical splines/locators
and province bitmap. Protect user files and actual installed assets first.

Read current definition-region pointers and runtime province.state18. Require
matching ownership and complete counts. Compare the same physical road keys
across cases using the expected runtime-state endpoint permutation. Do not bind
old numeric province IDs:16IDs represent different geometry in this fixture.
The19unique numeric geometry witnesses are private observations only. Centroid
pixel colors failed exact matching and must not be replaced by nearest colors.

After initialization settles, close read handles before model/view replay and
compare complete market and both state quantities with formal publication.
Inspect actual maps/local_goods at both moved locations. Retain early samples
separately; fresh worlds are not seed-controlled. Check UI scope switches do
not change copied facts, then normal-close own sessions, compare all errors,
restore test content bytes under hash guard and verify original inputs/assets.

R126 same core3be8cbcc… passes all918road endpoint records/12rebindings,19
province-set exchanges, exact market/two-state quantities and viewed UI. Three
normal exits0 including initial invalid fixture. Evidence province-audit.json.
This is not new/split/merged provinces, changed bitmap adjacency, ZIP/replace_path
or final lifecycle/device/performance acceptance.

## R125 consistent geo renumbering (2026-10-27)

Create a private reversible directory Mod changing one current geographic ID
to an unused valid ID. Update its actual spline point IDs, strip point refs,
packed connection endpoints and generated city/port/farm/mine/wood locator IDs.
Preserve coordinates, province membership, strip IDs and original assets. If
canonical endpoint order flips, reverse the connection's strip reference order.
Use the independent version4 decoder to prove whole-table equivalence under
the renaming. Do not replace every coincident number in unrelated locator tables.

Require actual frozen VFS order, current production-decoded new geo/points/keys
and absence of the corresponding old identities. Close the external process
handle before owned model/view/ribbon replay. Check exact formal state quantity
and global model counts, then inspect actual market/state sea and land ribbons.
Do not infer full external market-total agreement if the probe did not print it.
Compare all normalized errors with vanilla, normally exit the own paused game,
restore only expected test content bytes, and verify original save/settings/ini/
continue/assets and no new saves. Keep game-derived fixture assets private.

R125 same R123core3be8cbcc… passes geo20→1500,6points/12keys/5locator categories,
current state0/definition0,8980shipments, state3538.00627, formal223market/
234state ribbons. No product rebuild needed. This does not cover changed
province topology, ZIP/replace_path, all-Mod or final lifecycle/performance.

## R123 state identity and domestic enumeration (2026-10-26)

Use a reversible real directory Mod that renames a state key and moves its
definition/history blocks to change both native definition and runtime IDs.
Preserve all province membership, geo ID and spline data for this isolated
case; preserve each asset's leading BOM at byte zero. Inspect native parser
errors against a vanilla multiset and refuse malformed worlds. Protect the
original save/settings/ini/continue/content list and restore only test changes.

Read current keys/IDs from production decoders, require the renamed key to be
absent embedded data, and open that exact state in local prices. Require
formal quantity to equal owned replay, not merely visible ribbons. Advance
normal native time then pause, re-copy, and compare refreshed production model
counts/quantity. Record actual module paths/hashes and normal close/retirement.
External unselected alternative routes do not have the formal ribbon count.

Native trade-view regression must also reverse states/orders/land connections,
including scarce supply and parallel equal-cost physical strips. Fresh replay
and cached view must agree. Reverse original port lanes as well. All are in
the existing native_trade_view_tests executable and unified build_tests entry.

R123 core3be8cbcc… passed current state0/definition0/geo20 rename case:
state3574.65715,9053→9030shipments after two native steps, domestic23019.376→
23017.375, formal market219/state244ribbons. An earlier R120 fresh world had
a0.12difference; its exact audit stays failed and its cause is unresolved.
Current old-algorithm replay also agrees, so canonical ordering's proven
permutation fix must not be presented as resolving that historical discrepancy.
Evidence: checkpoint2026-10-26-r123-state-identity/current-audit.json.

## R122 active Mod geometry precedence (2026-10-26)

Use reversible private directory Mods containing the same spline file. Change
only one interior control of a current positive-flow port strip; independently
verify all connection/strip/point IDs, endpoints and other coordinates unchanged.
Back up the original content-loader bytes, save, settings, ini and continue file.
Do not edit installed map assets or require these test Mods for the product.

Launch with lower/upper, reverse the order, then remove both for a vanilla
control. Observe actual current VFS order and production-decoded point values,
not just intended metadata. Close the read handle before owned facts/view/ribbon
replay and require the target strip to carry traffic with that exact control in
its ribbon. Check formal UI quantity and visible curve in each case. External
unselected candidates mean its segment count need not equal the formal DLL's.

R122 unchanged R120core90e577c3… passed all3 cases: X4071.94775/4061.94775/
4053.94775, identical controls and market25963.19216/domestic23098.111,
formal218ribbons. All3 own normal closes exit0; full normalized parser error
multisets equal the vanilla control. Restore original content-loader bytes under
an expected hash guard, then verify every protected file and installed map hash.
Record results at checkpoint2026-10-26-r122-active-map-mods/map-mod-audit.json.
This is real directory/same-key one-point coverage across launches, not state
key/ID/province changes, ZIP/replace_path or in-process geometry reload coverage.

## R120 original world-destructor retirement (2026-10-26)

Current core90e577c3… adds the exact-build native world destructor event. Require
both retirement/capture hooks installed. A real original destruction retires the
owned epoch before any contained manager is destroyed, clears old ready results
and keeps a new-generation resident request for the first paused replacement
command. The original destructor is always called with its verified argument.

Own paused400 verified direct reload/event2, menu return/event3 and normal close/
event4. First replacement snapshots world2/rev8 and world3/rev12 reached the UI
without unpause; current Logistics218 and state433/248 restored. Both market
recoveries reused218 uploads. Minimize and real client resize/restore also passed;
close exit code0 and protected input hashes are recorded. The ABA helper uses
identical addresses; actual reused native addresses were not observed. Other
engine reset paths, check/submit concurrency and new-device recovery remain
separate gates. Periodic logs cannot establish native-call atomicity. See STATUS
R120 and checkpoint/retirement-audit.json; A24 remains PARTIAL.

## R119 current same-process lifecycle evidence (2026-10-26)

Current R118 coreaff98195… was tested without source/binary changes: load the
British autosave, open Logistics, reload it directly, return to menu with exit
autosave unchecked, start a new French sandbox, minimize/restore and actually
resize the client2560×1440→1920×1080→original, then load the original British
save again. World epochs1→2→3→4, British25963.19216/218 ribbons,
French9878.27752/141 and restored British/state433/248 were observed. New-game
data differ from the save; no second save file was created or claimed tested.

Require current capture/publication and checked native pool recovery before the
new view, not just a live process. Periodic submit logs were absent between all
three observed retire/recover events; this sampling is not every-frame proof or
an ABA-safe retirement signal. Minimize/client resize is not GPU device removal.
The exact session, captures, read-only source transitions, restored window bounds
and unchanged save/settings/ini hashes are in STATUS R119 and its checkpoint.
Full reliable retirement/new-device/map-mod/performance acceptance remains open.

## R117 current domestic/foreign baseline

Use the current hashes in STATUS. F4 opens the market; select Logistics. The
status explicitly distinguishes original trade from domestic model allocation.
Check land and domestic sea ribbons as well as international routes. Filter by
localized goods, try a known empty query, hover a row and leave, hide/show, and
close/reopen. The card follows search/category; hover does not change it or the
all-market share denominator. No match removes ribbons while retaining scope
ledgers. Select a state and Local Prices: its logistics column and incoming /
outgoing geometry must use that exact current state.

With a short/empty goods list, inspect the lower ledgers: total / original /
domestic model quantity, then domestic Freight target / fulfilled / unmet. These
are different quantities. Domestic targets include original imports/exports in
the local balance before model allocation; unserved demand is never drawn.
Recorded default-save values are evidence only, not hardcoded UI inputs.

Editable GUI/localization lives under freight/native-gui and own assets/shader
under freight/assets. Edit with the game closed, restart, compare the actual
mounted archive bytes and visible behavior, and verify that edits survive an
upgrade while untouched files update. Current .defaults is a comparison/reset
source. Shader wrapper comments use #, while HLSL Code blocks use //.

R117 code tests and default-save UI pass; three original port destination shapes
remain unavailable. Real map changes, full device/world recovery, physical rail
units and the three-round P95/simulation benchmark remain separate acceptance.

## R93 current world recovery protocol

Read current hashes/status before testing. R93 prototype39c recovered native arrows after both main-menu reload (world2) and direct in-game loading (world3), with180 retained slots and zero additional uploads. The final8115373d candidate also checks graphics/device identity on cache hits; a new device still requires a separately verified rebuilding path. Older permanent-restart instructions below are historical.

Keep an autonomous test session paused, preserve the existing save, and leave exit autosave unchecked. Record initial market/state figures, slot count and uploads. Return to menu/reload, then open the native panel: require a strictly newer world, a current real frame, successful native lease verification and newly visible arrows. The unavailable-map message must clear after UI publication. Check no native submission between retirement and recovery; record old/new generation events and copied market scope. Repeat using in-game Load without menu. Unknown/replaced type, registration, path, buffer or graphics identity must remain disabled. Test different-save/new-game data separately; same-save recovery is not that acceptance.

After recovery test minimize/restore and actual client resize, restore original window bounds, and check map alignment/figures and error.log. These window operations are not a GPU device-removal test. Final performance/complete A24 must still cover their explicit remaining scopes in MASTER_PLAN.


## R69 lifecycle boundary and current expectations

R69 fixes numeric snapshot retirement and cancellation. A world reset now
requires a strictly newer world generation; a higher revision of the retired
world must be rejected. New snapshots immediately remove the old published
frame. Worker resets survive request coalescing and Cancel. Native panel
submission is allowed with ImGui hidden; this code change still needs live
verification against the current DLL.

Native arrow registration/destruction across actual world teardown remains
unverified. When the adapter detects a world replacement, R69 stops subsequent
access/submission of old native slots and labels the map unavailable until a
game restart. Numeric snapshots can refresh for the new world. This is a
temporary capability gate, **not** a pass for reload/main-menu lifecycle. The
old R49 invalidation drill no longer proves that arrows can resume after reset.
Anchor reuse at the same address/id/vtable may also evade the current detector;
a verified world lifecycle signal is still required for full acceptance.

Current native summaries include model allocation orders and a separate Freight
intermediary target/fulfilled/unmet row (R67). Missing values remain `--`.
The older prototype instructions below are historical where they conflict with
this current behavior. Never infer game visibility from offline tests alone.

Do this once with the current build. The loader is installed in
`binaries\version.dll`; the game must not be running while you copy a new build.
This DLL embeds its styles and map data. Do not enable a launcher mod/playset for it.
The log must show `embedded archive ready BEFORE gfx/models .asset enumeration`
during startup, then `resolved arrow type 'ironroadBuildingOutput'` on the map.

Native GUI acceptance for the current integration build:

1. Startup must log `native GUI archive mounted during native MountData` before
   `native GUI enumeration; prepared=1 mounted=1`. Check error.log for GUI errors.
2. Open a market: the fifth **Logistics / 物流** tab must appear beside the four
   original tabs. Switching/back/close must retain normal original panel behavior.
3. On Logistics, original search/category filters must change the original goods
   rows. A goods row must open the original goods panel with native prices,
   buy/sell orders and building details.
4. Freight/track summaries remain `--` until the snapshot bridge exists. Show/Hide
   currently changes its native GUI variable only; it is not yet a working native
   freight-render command. Do not treat the visible shell as completed logistics.

Offline generator checks: `tools/test_native_gui.py`,
`tools/test_native_gui_runtime.ps1` and `tools/verify_embedded.py`.

Current worker/width checks (not yet established by the earlier direction-only run):

1. Draw a route, then set **Preview consumption** to 5, 100, 500 and 5000.
   Width should grow while centreline, source/destination and arrowhead alignment
   stay fixed. This slider is a manual preview, not a live economic reading.
2. Switch the five Traffic colours in the colour menu. Check green/lime/yellow/
   orange/red variants load in game. The market view will choose these bands
   automatically from relative throughput once the live snapshot is connected.
3. Drag the amount/calibration slider quickly; only the latest settled result
   should activate. Press Clear or hide/close the panel during computation:
   an old completed route must not reappear.
4. Reopen, draw the same endpoints/style/width band, and check uploads remain
   unchanged while cache hits increase. Changing amount within a band requires
   no native upload. Reversing endpoints changes direction and geometry identity.
5. A disconnected authored or province route must report no route rather than
   silently drawing a chord. Chord remains an explicit preview mode.

`native/build_tests.ps1` exercises the actual worker with the installed derived
map and the native scheduling policy with deterministic mock native calls. GPU
tests execute the real shader at eight widths and verify all nine palettes.
These checks do not establish live many-ribbon render time, world reset safety,
or complete freight/economy integration.

Direction regression (offscreen, no game interaction):

```powershell
python tools\verify_embedded.py
python tools\test_flow_shader.py
python tools\test_flow_gpu.py
```

The GPU check uses the actual pixel shader and installed building-flow texture.
It checks forward/reverse arrow tips stay on the ribbon centre with the native
transverse UV range 0..1, and rejects the former half-width sampling as a negative
control. It does not replace the in-game camera/terrain/animation checks below.

## 1. Launch and check the panel

1. Start Victoria 3 normally.
2. No separate panel should appear on startup. Press **Insert** to open the
   optional **Ironroad debug** panel, initially on Diagnostics.
3. Its UI/State prototype pages are development tools. The player-facing native
   Logistics tab is still in integration; do not treat prototype widgets as
   proof of native GUI or live freight completion. See NATIVE_GUI_PLAN.md.
4. In **Diagnostics → Status**, expected:

   | Row | Expected |
   |---|---|
   | Loader | version.dll proxy |
   | Build | 1.13.11 |
   | Native hooks | installed |
   | Arrow type | `not resolved` → `ironroadBuildingOutput` after the first map frame |
   | ImGui | ready |

   If `Build` says *unsupported*, the loader saw a different executable hash and
   disabled the arrow calls on purpose; send the log line.

## 2. Draw the first ribbon

The pass-selection correction reads the same live domain/stage handles used by
the game's own arrow submission routines. The first 12 pass callbacks log
`scope=domain/stage/view native-scope=domain/stage eligible=...`. Once a route is
active, `SUBMIT scope=...` appears at most every five seconds. Do not infer a
visible ribbon from non-null buffers or counters alone: check the captured map.
`python tools\analyze_log.py` now reports only the latest process by default,
so a prior harness run cannot make a current game run look unsupported.

On the **Route preview** page the build runs an automatic short test route (Home Counties -> Lancashire, falling back
to Svealand -> Götaland when those states are unavailable) after the first few arrow
passes once the arrow type resolves. This exercises the native pipeline even
without a click; disable it with `auto_test=0` in
`%LOCALAPPDATA%\IronroadV3\ironroad.ini` or the panel checkbox.

1. Load a save (or start a new game) and wait until the map is interactive.
2. Open **Route preview**, choose **Origin** and **Destination**, and pick **Geometry** (`Province land path`
   follows land over the province graph; `Straight chord` is the fallback/comparison).
3. Click **Draw route**.
4. Choose **Colour**: Building flow green/orange/yellow/blue or Vanilla tan.
   `deploy.ps1` installs the standalone DLL; restart after deployment.
   The default is building-flow green. The launcher enabled-mod list may be empty.
5. Swap origin/destination and redraw: both glyphs and animation should reverse,
   with the same complete route endpoints. Repeat for green and orange; both use
   the building selection artwork. A nonzero submit counter alone does not prove this.
5. Expected in the panel:
   - `Route: active`
   - `submitted` keeps increasing once per frame
   - `last: ok: 48 pts, vb=yes cb=yes len=...`
5. The ribbon should be visible above the map between the two state centres.

If nothing is visible, first make sure the camera is over the chosen states. Then use the
**Calibration** sliders for a decisive visibility test:

1. Set `Height offset` to **+500** (leave scale at 0.01). A working ribbon must now float
   clearly above the terrain.
2. If it appears, lower the offset until it sits on the ground, then run
   `tools\calibrate.py` from a real movement arrow to get `height_scale`.
3. If it does not appear even at +500, the problem is not height; check `last:` in
   Diagnostics (`vb=` must be `yes`) and the pass/submission counters.

The log records `vb`, `cb` and `len` for every build, so a missing buffer is
distinguishable from wrong coordinates.

## 3. Capture exactly what the panel sees

When the auto-test route is built, the overlay also auto-captures one frame three seconds
later (`readback.ppm`), so the first launch is verifiable without any click. You can also
click **Capture frame (readback.ppm)** in Diagnostics at any time. The overlay copies the game backbuffer (including the panel) to
`%LOCALAPPDATA%\IronroadV3\readback.ppm`; convert it to PNG or look at it directly to see
whether the ribbon is present and where.

## 4. Calibrate from a real arrow (important)

1. Move an army or a fleet over a few provinces (any native movement arrow).
2. The log records up to 40 real `CAPTURE setpath ...` lines.
3. Close the game, then run:

   ```powershell
   python tools\analyze_log.py
   python tools\calibrate.py --ini "%LOCALAPPDATA%\IronroadV3\ironroad.ini"
   ```

   `calibrate.py` computes `height_scale = median(native y / heightmap sample)` and writes
   it to the ini. Restart the game and draw the route again.

## 5. What to report back

- Whether the panel appeared.
- The Status rows (especially `Arrow type`).
- Whether the ribbon drew, and if not, the `last:` diagnostics line.
- The full log at `%LOCALAPPDATA%\IronroadV3\ironroad.log`.

## 6. Rollback

```powershell
powershell -ExecutionPolicy Bypass -File E:\Code\v3-carrow-imgui-starter\deploy.ps1 -Uninstall
```

This removes the loader (only if it is the Ironroad build) and restores an original
`version.dll` if one was ever backed up. Saves and installed mods are untouched.
