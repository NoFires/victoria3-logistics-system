# Native profile — Victoria 3 1.13.11

### 2026-10-30 — R167：原版国内重建与最终构造边界（诊断）

Exact EXE7f53e326…/image140000000. RVA0140D830 void(manager RCX,countryObject RDX), signature48894C24085553565741544155; RVA0140FAA0 returns CShippingLane* RAX, nine pointer arguments: database,kind,country,treaty,fromState,toState,ProvinceSet,extraProvince,distance64. Signature4C894424184889542410534154. Wrapper0140DF60 has nine arguments, final stack argument is distance VALUE; decompiler omits it, assembly forwards its ADDRESS to raw constructor. Isolated exact wrapper10 cases verify transport with substituted constructor/services, not allocator ABI.

Default-off probe-only a400dd88… actual49244 records185 final created lanes/17countries/33 parent generations; inputs match typed returned object and full-ID DB object at natural parent return. Same-process UK547→551 retarget changes lane generation, distance and province set. No denied classifier in these creation records; no independent external lane comparison (probe exit4 after process disappeared), normal exit code unavailable. Sequential rereads and observed world/date do not prove atomic request/world ownership. Formal permission handoff remains disabled; see STATUS R167.

### 2026-10-30 — R166：自然分类输出观察（仅诊断）

Exact EXE SHA256 7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f, image140000000. Classifier RVA0141FDA0: void(countryObject RCX, mode DL, allowedVector R8, deniedVector R9); entry 48894C24085657415441554156. CPdxArray header data0/cap8/countC, handles32bit. Appends without clearing; buffers remain borrowed by original caller. R165 isolated native-code harness and R166 actual original caller RVA0141F236 confirm separate outputs and order; R166 same-call predicate trace checks full output. Only naturally issued calls are forwarded, never extra native calls.

Current probe-only core23b812d8…/bootstrap71dae36d…, actual PID23560: 377 matched records,283 countries,14 threads;47 complete allowed/denied/allowed transitions through stock Bosporus UI in unsaved debug state. Both access2/owns1/context1 and access2/owns0/context0 occur. Bounded diagnostic copies do not grant an access lease or prove atomic world/request identity, selected-lane binding, allocator ABI for extra calls, diplomacy/canal coverage or formal routing correctness. Production strait callback remains unbound. See STATUS R166 and checkpoint2026-10-30-r166-native-classification-live.

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


### 2026-10-29 — R138完成：实际交付引导的缺失／损坏主体隔离回归

当前主体9afd299a…／47581219B、引导71dae36d…／779879B，源码＝dist＝安装＝R137实际加载；产品源码和两份DLL未变、本轮未部署或启动游戏，完整统一入口58 PASS／零FAIL／退出0。

直接复制实际交付version.dll到含中文／空格的独立路径，工作目录放另一目录及同名无效诱饵；缺少主体和整个freight目录时引导自动建立目录，加载失败码126；损坏PE时193；实际健康主体成功加载且码0。三者均在独立进程中保持系统版本信息查询可用，不是游戏进程内缺失／损坏实机通过。

扩展已有tools/test_freight_loader.py：统一入口同时对重新编译引导和dist实际引导分别验证有效stub、缺失、损坏及错误码0／126／193；17个导出存在、系统版本数据逐字节一致；另外原有--runtime使用真实双DLL验证新配置优先和迁移。无玩家运行时新依赖。

隔离测试及统一入口后两份实际安装与dist哈希相同，原save／settings／ini／content／continue五输入及存档目录保持、无游戏。首私有审计误以为R134清单包含该测试文件而被断言拒绝，核对清单后改为对比本轮before；失败脚本保留。一次仅用于打印的私有脚本遇GBK输出错误，完整测试已结束且结果不受影响。文档归档首版误将保持原CRLF的Python源按Markdown检查而被拒绝，修正后仅重跑归档尾段。

A26新增当前交付引导隔离子项PASS；真实游戏进程中缺失／损坏、目录不可写、完整路径／升级等矩阵仍PARTIAL。三港口末端／运河权限、改图新字段、写任务／提交原子性、真实ABA／新设备与最终同构建A01–A27继续；整体goal ACTIVE。 证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r138-delivered-loader/{loader-audit.json,isolated-results.json,isolated 中文 空间,full-tests.log,before,NEXT.md}。



### 2026-10-29 — R137完成：原版地点查找器对照与当前版本卸载恢复

主体9afd299a…／47581219B、引导71dae36d…／779879B；源码／资源／dist未变，复用R134完整55 PASS与R135同核性能。本轮通过deploy.ps1实际卸载后恢复，最终源码＝dist＝安装＝实际24108加载。

同原存档／中文／暂停59883360，地点查找器输入台湾并点击第一个地区结果，分别保存点击前后error.log。仅系统VERSION.dll加载的原版27220与当前扩展24108均为点击前0条、点击后新增1条PdxDebugConstants命名索引错误；完整规范化错误Counter相同、点击差分只有该行。R136的新增日志在本机无Freight时可复现，不能直接归因扩展；此结论只限本机EXE／存档／语言／操作，不推断所有游戏版本或其他命名索引错误。

卸载前核对无deploy.ps1会清理的旧文件／playset条目并备份42个非主体文件；卸载与重装之间全部42个配置／缓存／可编辑资源逐字节相同，再次实机启动后仅freight.log／loader.log自然追加。实际模块分别为stock系统VERSION.dll、当前version.dll＋freight.dll；恢复后的市场0物流页40商品／218段／25963.19216／510.99MU实见。

两次我方会话27220／24108均全程暂停、正常WM_CLOSE退出0；五项受保护输入及存档目录字节保持、无新存档和游戏进程。当前retirement2实见，无扩展资源相关报错；不能将正常退出扩为ABA／新设备通过。

A26当前卸载／资源保留／重装实载子项PASS，完整缺失／损坏／不可写／路径／升级等矩阵仍PARTIAL。三处港口末端、运河权限、改图新字段、写任务／提交原子性与真实ABA／新设备及最终同构建A01–A27继续；整体goal ACTIVE。本轮消除了R136这条日志的扩展独有疑点，未修复三处港口缺段。 证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r137-stock-location-control/{stock-control-audit.json,live,runtime-before.json,before,NEXT.md}。



### 2026-10-29 — R136完成：三处当前港口身份与界面核对，缺段仍待修复

主体9afd299a…／47581219B、引导71dae36d…／779879B，源码＝dist＝安装＝实际46268加载；本轮未改源码／ABI／资源、未构建或部署，复用R134的55 PASS及R135性能证据。

当前暂停原存档在精确EXE7f53e326…／module7ff63d2e0000下，线程TLS对应当前上下文只读两次相同：三个真实运行州207／936／608的州定义及naval_exit分别对应3157／3056／3072，均属于各自原版lane成员；可用类型3港口样条却分别连接3002／3157／3056，均不属于各自lane成员。R121曾因会话结束未完成运行时字段读取，本轮补到当前运行对象证据；两次顺序观察不是原子快照，也不证明所有Mod。

R121历史编译的独立探针在关闭PROCESS_VM_READ句柄后重放得到918陆／93海／9038笔、partialSea3，三条对应边的模型量0.586／418.679／0；当前生产日志独立确认相同全局数。端口和几何两个生产头及其回归与R121逐字节一致，但不能把旧探针冒充本轮生产逐边trace。零是合法模型分配，缺几何不改成未知运量。

实际原版南海航线面板展开大清港口连接，台湾行及原版商船需求1.40实见；该字段不是货物运量。独立面板探针确认台湾936、苏禄北婆罗洲608、菲律宾西密克罗尼西亚207均打开local_goods，三处截图保留；台湾与207物流列有非零数，608当前为零。按地区名称搜索后最初点击的是同名另一运行州206，探针识别后排除并改选207；截图显示列表／现有视图，不证明原版或扩展的完整末端几何。

Ghidra明确选择victoria3.exe复核025E8450→8690→8070：成员内部按type2查连接，边界按当前成员海区与端点州查type3；同端评分不授权跨出lane补一条海路。该调用链仍没有发现可验证的跨海区端点补充来源；保持原有已知部分、数量和用户不硬编码地图表约束，不能凭naval_exit或最近海岸造线。

我方46268/start2026-10-29T00:41:42.2087748+08:00一直暂停／日期59883360，正常WM_CLOSE退出0；原save／settings／ini／content／continue五输入字节和存档目录保持，当前无游戏。退休2实见。日志对R134同语言多出一条“PdxDebugConstants”命名索引错误，原因未确定；严格Counter相等首审计被拒绝并保留，错误时刻与地点查找器点击台湾结果同秒，仅是时间相关、不证明因果；没有ironroad／freight／native-gui相关错误，不能称完全无新增游戏错误。

三处完整末端几何仍未修复；后续优先追查其他当前原生几何来源与权限语义，不猜路线。运河实际开放关闭／权限、更多改图、并发／ABA／新设备及最终同构建A01–A27继续，整体goal ACTIVE。本轮原生日志新增项需在后续可比场景定位，不能将它归因扩展或直接排除。 证据：E:/Code/v3-analysis/checkpoints/2026-10-29-r136-port-runtime/{port-runtime-audit.json,live,ghidra-1425e8070.txt,audit-first-rejected.py,NEXT.md}。



### 2026-10-29 — R135完成：当前同核三对性能与20轮颜色组合缓存验收

主体仍9afd299ab0f0aba088e9f6db68147319fbdeabd4fae74fdfaed453e31f451a69／47581219B，引导71dae36d…／779879B；源码／资源／dist／安装未变，复用R134完整55 PASS。无新增构建／部署／原生ABI／hook；当前实际54780双模块核对，timing-audit.json证据及指定性能门槛PASS。

实际EXE7f53e326…／base7ff63d2e0000、线程57988、同swap000001fb62104380；仅该测试子进程启用既有IRONROAD_FRAME_TIMING=1，不改配置／原生字段或全局环境。world1…6各臂独立连续QPC解析，退休2…7原始行保留。

沿R129／R130同原存档59883360、英国地标／2560×1440／原图形设置／原生速度3，每臂正常重载同存档，暂停预热30秒后运行65秒；六窗完整连续64.83–64.85秒，窗内无截图／输入、所有尖峰保留。三对关闭→打开P95为17.9185→19.2502、17.8774→19.1865、17.9017→19.0847 ms，增加1.3317／1.3091／1.1830 ms均≤2；模拟最大降幅0.774997%≤5。Present入口间隔不是GPU／显示延迟，根市场关闭还改变原版地图模式，只证明本存档／速度协议。

随后world6／rev52／date59884134暂停，20轮四分类轮换＋三颜色／单商品hover进入离开／恢复全部／Escape关闭→F4原版市场→点物流，逐轮VM_READ确认closed／market0／tab，176份观察通过。完整市场量26371.75358／510.99MU不因视图改变；与原日期的25963.19216差异来自本轮正常模拟推进，不将跨日期数量当UI不变对照。前四轮建立变体后，后16轮uploads和实际pool均1721不变；SUBMIT的cache是累计命中数，允许增长，不能当对象池。稳定66.313秒uploads1721不变，全进程private固定15880417280B；后16轮private范围15879147520–15892967424B、净减1712128B，属于本窗口平台证据，不证明所有分配器／8192实际满池。 172个实际完成DRAW批次账逐项相等，失败／丢弃0，池与native槽峰1721，每帧尝试峰16，Pump帧／单步峰1161／708µs、超2ms帧0。QPC含调度／driver等待，软预算不作硬实时保证；R134真实2056／2392µs超时保留为各自历史局，不删除或归到本局。

我方54780/start2026-10-28T23:09:37.6659011+08:00正常WM_CLOSE退出0；原存档／settings／ini／content_load／continue五输入字节及存档目录保持，无游戏。重载退休2…6及退出退休7实见，不扩为ABA／新设备通过；完整错误对R134中文无新唯一行，六条Unexpected身份保持、扩展资源错误0，重复读档的次数差异单独保留。

整体goal ACTIVE；当前9afd指定三对、20轮组合平台与稳定零上传子项完成，不再列为当前缺证。A25整体仍PARTIAL：更广工作负载／最终全矩阵不外推。继续三港口末端确证、运河开放关闭／权限、更多地图与铁路新字段、写任务／提交原子性与真实ABA／新设备，以及完整A01–A27和异常安装升级卸载恢复。若后续代码改变，应在最终构建重跑必要门槛。 证据／当前对备份：E:/Code/v3-analysis/checkpoints/2026-10-28-r135-final-performance/{timing-audit.json,budget-report.json,error-comparison.json,live/benchmark,before,NEXT.md}。



### 2026-10-28 — R134完成：正式颜色／八档线宽图例、双语居中及编辑GUI校验

主体9afd299ab0f0aba088e9f6db68147319fbdeabd4fae74fdfaed453e31f451a69／47581219B，引导71dae36d…／779879B；源码＝dist＝安装＝最后实际63460加载。完整55 PASS／零FAIL，embedded／GUI精确归档／selftest560464／默认隐藏vtx0及默认无FRAME PERF通过；只经deploy.ps1部署。

没有新增原生ABI／RVA／getter／hook／shader；EXE7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f不变。颜色只选既有构造时样式，不调用selected/tint，也不重构造注册对象；worker只有owned副本。第四固定颜色键使用已验证GUI Set与当前scope，生命周期及提交门保持。

当前9afd英文67696与中文63460各21项颜色／商品／方向／关页重开检查通过；市场218、单商品等待0／谷物hover123／移出0，州433全部248／输入107／输出141，原生绿与业务色恢复原段数。市场量25963.19216／510.99MU、州433量3548.63121全程保持。英文原版首行Artillery为66段，中文所选第二行加工食品为22段；这是原版本地化排序，不把两次选择当同商品对照。中文额外显示／隐藏实际截图与日志通过，数量不变。每个正常会话仅一次国内分配918陆／93海／9038笔，三处partialSea仍保留已知数量及可确认片段。

四个必需GUI类型按真实type声明计数，跳过注释／字符串；缺失或重名拒绝整个扩展挂载、记录明确原因并保留玩家修改，提示对照freight/native-gui/.defaults。当前9afd实际坏GUI67464中，注释伪声明不能冒充类型，原版四页签／frame0、修改字节保持，正常退出后严格恢复；后续63460重新加载完整GUI通过。16个按钮文字框改为100%边界／居中／鼠标透传，中英截图及实际点击验证通过。

七个我方会话全部正常退出0；原存档／settings／ini／content_load／continue五输入最终字节相同，无新存档、当前无游戏。英文退出仅删除受控language字段，逐字段证明后恢复原中文字节。完整规范化error Counter逐局等于R132同语言基线，未新增扩展错误；每局retirement2实见，不扩为ABA／新设备通过。

整体目标active；三港口末端、运河开放／关闭及权限变化、更多地图／铁路新字段、写任务与提交并发／ABA／新设备、最终同构建帧／模拟三对、压力平台及完整A01–A27继续。颜色／图例／当前双语布局及坏GUI子项已完成，P5／A18–A19和A25整体仍DOING／PARTIAL。R130旧核性能不能替代当前9afd。 证据及R133回滚：E:/Code/v3-analysis/checkpoints/2026-10-28-r134-native-legend/{legend-audit.json,live,logs,before,NEXT.md}。

### 2026-10-28 — R133完成：地区商品／方向／运输方式地图筛选

主体309aed48c5880adf61d83b6883200715dd86291277c72a6c0da7ceaa2cbcb9ba／47369073B，引导71dae36d…／779879B，源码＝dist＝安装＝实际18592加载；完整50 PASS、资源／GUI精确归档／selftest560382／默认隐藏vtx0通过。地区新增商品、输入／输出和陆路／海路地图筛选，州各商品完整数量、国内分配、国际原版记录、需求账及全市场分母保持；切州／关页／商品详情撤帧并重置。

当前原存档暂停59883360／world1／rev4，州433量3548.63121保持；全部248段，输入107／输出141，陆路65／海路183；加工食品22段，输入陆路1／输入海路21。州5恢复完整282段／5506.40802，返回州433及关闭重开248段。丝绸运量0时合法空地图，恢复全部248；原版加工食品详情无自定义线，原版Back恢复完整州视图，市场0恢复40商品／218段／25963.19216／510.99MU。

无新原生ABI／RVA／hook／getter；EXE7f53e326…，实际双模块及world1／rev4、guarded VM_READ面板核对。GUI三个固定变量值携当前scope，旧值不能跨州／重开恢复。国内918／93／9038仅一次分配，23完成批次／池635／16每帧／702µs峰／0超时。正常退出0、五输入及存档目录保持，error完整Counter与R132中文相同，retirement2实见。失败／私有商品误名限制见STATUS及E:/Code/v3-analysis/checkpoints/2026-10-28-r133-state-filters/state-filter-audit.json。

整体目标active，P5／A18–A19及A25整体仍DOING／PARTIAL。新增控件英文布局、较长中文“全部…”标签省略及商品选择指示待完善；颜色模式／线宽图例、旧玩家编辑GUI升级契约仍待。三港口末端／运河权限、地图新字段／并发／ABA／新设备、最终同构建三对及完整A01–A27继续。

### 2026-10-28 — R132完成：同构建中英UI及实际卸载／恢复验收

当前主体6094ea6e…／47173344B、引导71dae36d…保持，74份native源未变，复用R131完整50 PASS，不重建。实际英文／中文搜索、分类、连续hover、商品右键／详情／原价与提示、州433→5→433及返回市场通过；对应商品ID和数量逐项相同。实际卸载后仅系统VERSION.dll加载、原版四页签；通过deploy.ps1恢复同对DLL并再启动，中文物流页40商品／218段／25963.19216／510.99MU实见。四局正常退出0、五输入字节恢复、无新存档／无游戏。

无新ABI／地址／getter／hook。四局EXE7f53e326…／module7ff63d2e0000，三局带Freight实际模块核对，stock仅系统VERSION.dll。国内模型／国际原版与1e5量纲保持；原始VM_READ probe、逐商品ID及州433／5身份、数量对账和界面截图见E:/Code/v3-analysis/checkpoints/2026-10-28-r132-english-native-ui/ui-audit.json。冷批次实际2.030／2.039／2.163ms各一次软超时，最多16／帧，真实池峰1201；不冒充帧P95或原生8192满池。英文settings单字段退出规范化的首FAIL与严格恢复证据、stock原版缺键对照保留，详见STATUS／WORKLOG。

这只完成当前中英交互及卸载恢复子项，整体目标active。源码确认地区商品／方向／运输方式筛选控件尚未实现，颜色模式／线宽图例正式UI未齐，P5及A18–A19保持DOING／PARTIAL；最终同构建性能三对、三港口末端／运河权限、地图新字段／并发／ABA／新设备和完整A01–A27仍待。

### 2026-10-28 — R131完成：原生构建预算记录与20轮组合缓存验收

- 当前主体6094ea6e34c3622efcf451a32c3695c4086846f31b2ba43020eedb5299881060／47173344B，引导71dae36d…／779879B；源码＝dist＝安装＝实际加载。完整50 PASS，embedded／GUI结构及实际归档、selftest560410、默认隐藏vtx0通过，部署只经deploy.ps1。资源hash862ecc6e…与R130相同，无新ABI／地址／hook／地图表／模型／GUI资产改变。
- Pump复用原有时钟，记录累计步骤用时、单步／逻辑帧峰值、同帧尝试峰值及超2ms帧数；完整／取消／替换时只汇总一次DRAW PERF，带world／rev／gen、数量／失败／丢弃及池上界。16次／2ms软预算／8192策略不变。cpu_us仅是Pump步骤累计QPC用时，含调度／driver等待、不含帧间等待或Request查找／worker／UI成本，并非线程CPU或GPU时间；峰值／超时计数为cache生命期累计。
- 首候选18be148d…实机37192：冷218条正常，完全相同请求重开却报reused0。严格对账及新增目标用例FAIL保留；修正诊断Hits将已经完成的保留项计为复用，待办项不计，几何／请求／预算行为不变。目标覆盖同帧多pass／换请求不重置、3ms单次超时只计一次、10秒间隙不算构建、失败抑制、8200请求→8192上传／8丢弃及最多16，修正后PASS并重跑完整入口。初候选源码／DLL／正常退出0／五输入保持及失败证据保留。
- 最终56348/start2026-10-28T05:50:49.4003851+08:00，EXE7f53e326…／module7ff63d2e0000，实际双模块核对；原存档暂停date59883360／world1／rev4不变。20轮原版分类→首商品hover→移出→恢复全部→Escape关闭／F4市场／点物流，逐次VM_READ确认面板身份，101份观察与101条完成批次逐项对账。全量25963.19216／218段、铁路510.99MU保持；hover不改卡片，分类不改全市场分母，国内模型与原版量口径保持，UK海陆线可见。
- 实际726次成功上传，失败／丢弃均0；池序列432→573→667→726，后16轮保持726，重开218条均uploads0／reused218。全逻辑帧最多16次，峰值550µs、单步骤255µs，累计Pump15.280ms、实测超2ms帧数0。实际没有填满8192；边界来自同生产cache的合成压力及UploadRibbon index守卫，不冒充游戏8192对象或人为制造超时。
- 最后稳定66.172秒，累计uploads726不变；全进程private范围14537433088–14537785344B。轮5–20范围14559342592–14592380928B，首末从14592380928降至14562492416B；池平台与该观测窗口成立，不扩为全部分配器／所有场景无泄漏。
- 我方正常WM_CLOSE退出0、原save／settings／ini／content_load／continue五输入字节及存档目录保持，无游戏；完整error对R130新增唯一行0、六条Unexpected身份保持，无扩展资源错误，不称长流程完整Counter相等。本局正常退出没有原生retirement日志，不能用exit0补成析构／ABA／设备寿命证据；首候选与最终会话均完整归档。
- 证据／R130回滚对：E:/Code/v3-analysis/checkpoints/2026-10-28-r131-ribbon-budget/{budget-audit.json,budget-report.json,live/pressure,live/first-candidate,initial-candidate,logs,NEXT.md}。目标active，A25整体PARTIAL；R130三对帧结果只归R130，当前6094预算／缓存通过不替代最终同构建三对。更多原生压力、英文locale、三港口末端／运河权限、地图新字段／并发新设备及最终A01–A27继续。下方R131进行中及更早记录为历史。

### 2026-10-28 — R130完成：精确GUI键轮询与三轮性能复验通过

- 当前主体e4e0583bd51f764fe9ba0874ea183fd4fc54e68eaa12ffb880c37e63ee7b45e7／47160425B，引导71dae36d…／779879B，源码＝dist＝安装＝实际加载。完整50 PASS，embedded／GUI结构与实际归档／selftest560432／默认隐藏vtx0及默认无计时日志通过；部署仅deploy.ps1。
- 保持25ms悬停响应、逐商品键／owner协议；只按当前目录的精确键及已存hash筛选既有GUI表，碰撞再比较完整键，不复制无关字符串。仍扫描有界桶头，不冒充O(1)查找。500ms显示开关与低频身份诊断同用精确键；调试全表枚举保持。无新ABI／地址／hook／GUI资产／模型改变，worker无engine指针。旧4096项路径实际FAIL、新2项及碰撞／边界／只读不变回归PASS，目标离线计时不冒充游戏FPS。
- 实机40808/start2026-10-28T04:55:15.4458092+08:00，EXE7f53e326…／module7ff63d2e0000、实际双模块核对。沿用R129同原存档／英国地标／2560×1440／原图形设置／速度3协议，每臂正常重载date59883360，暂停预热30秒、运行65秒，六窗完整连续64.79–64.85秒、窗内无截图或输入，尖峰全保留。P95增1.0024／0.9268／0.9308 ms，均≤2；模拟降幅0.775036%／0.000177%／0.000610%，均≤5。此三对门槛PASS，R129失败保留；Present入口间隔并非GPU执行／显示延迟，关闭根市场还含原版地图表现差异，不外推全部存档／速度。
- 连续两商品hover／移出恢复、显示隐藏、中文“衣物”两行及空结果通过，基础quantity固定2637175358／1e5；清空恢复235段。中文locale中clothes在原版Goods和物流页均零，不称英文locale已验。铁路卡片510.99 MU不随筛选变化，国内继续明确模型分配且数量已知，国际保持原版记录。
- 20次实际Escape关闭→F4原版市场→点击物流页，每次只读确认closed／opened及market0／tab；累计uploads=538不变。随后稳定66.156秒仍零新增上传；全进程private范围15622623232–15623163904 B，仅作该窗口平台证据，不证明全部分配器／池压力。首版误把F4当关闭，被marketOpen1／tabdefault／visiblenone拒绝，失败脚本保留后改正确操作，未把失败计入20次。
- 我方暂停会话正常WM_CLOSE退出0，原save／settings／ini／content_load／continue五输入字节及存档目录保持、无游戏。完整error对R129新增唯一行0、六条Unexpected身份相同，扩展资源错误0；不称多次重载次数相等。退休实见[2, 3, 4, 5, 6, 7]，退出是否有独立原生退休按原日志记录，不以exit0替代ABI／设备寿命结论。
- 启动前freight.log为8512707B，超过既有8MiB轮换门槛；首finish前缀断言拒绝及随后的audit缺error.log均保留为私有归档脚本失败，未称产品失败。核对freight.old.log逐字节等于before-freight.log、当前头精确pid40808、loader原前缀不变后修正finish支持这一已验证轮换，完整当前会话与旧日志都归档。正常WM_CLOSE后native退休7实见，重载退休2…6；不扩为ABA／新设备通过。
- 证据／R129回滚对：E:/Code/v3-analysis/checkpoints/2026-10-27-r130-filtered-gui-probe/{timing-audit.json,live/benchmark,logs,NEXT.md}。目标active；三港口末端、运河权限变化、更多地图及新铁路字段矩阵、并发／新设备、真实池压力／构建超时与最终同构建A01–A27继续。下方R130进行中及R129记录是历史，以上为当前状态。

### 2026-10-27 — R129：默认关闭的原始计时链完成，三轮性能基线发现超标

- 当前主体b13289d1228bceb3c295fb37fc311069e25329013d8f249f636e9a6e1579da55／47017006B，引导71dae36d…／779879B，源码＝dist＝安装＝实际加载。完整49 PASS，embedded／GUI结构与实际归档、selftest560447、默认隐藏vtx0及默认无计时日志通过。原生ABI／资源／模型与R128不变。
- 既有Present／Present1增加IRONROAD_FRAME_TIMING=1才启用的诊断：线程自有64个原始QPC间隔批量日志，嵌套转发只计外层，TEST／非当前swap排除；world／swap／时钟变化撤旧部分批次。无逐帧日志／数据库扫描／配置修改。native helper、解析器与实际D3D入口回归接统一入口；非法开关2保持关闭。指标为Present入口间隔，包含调度／呈现节奏，不冒充GPU执行或显示延迟。
- 自主52156/start18:15:32.8660935+08/module7ff63d2e0000，实际双DLL核对。每臂重新载入同一原存档59883360、英国相同地标视角、2560×1440、原设置／速度3，暂停预热30秒后运行65秒；各有64.82–64.85秒完整连续帧，窗内无截图／输入，原始尖峰不剔除。关闭／开启三轮P95分别18.1201→27.2114、18.8955→25.5687、18.4805→27.7414 ms，增加9.0913／6.6732／9.2609 ms，**全部超过+2 ms，A25整体FAIL**。本协议日期推进降幅约0.775%／0.001%／0.001%，未超过5%，含一个原生tick量化误差，不扩为所有速度／存档通过。
- 单轮诊断控制：保留物流页、只隐藏箭头仍28.0334 ms；原版市场商品页20.6771 ms。不能将超标直接归因箭头；优先检查新增物流页绑定／刷新。根市场关闭还会改变原版市场地图呈现，报告同时保留这一作用域差异；控制组不替换三轮失败基线，不改验收门槛。
- 首次30天保护上限提前停止且鼠标边缘导致相机漂移，短样本拒绝保留；测试保护改60天（原半年自动存档未改），有效窗约32天。快速菜单批次曾未进入读档而超时；复核同一活进程并观察菜单后继续，没有重启游戏或把超时当退出。首版audit预期退出world10日志而失败；实见只到重载退休2…9，正常WM_CLOSE退出0由close.json独立证明，不虚称退出析构／flush已观测。
- 已正常退出0、当前无游戏，原save／settings／ini／content／continue五输入字节及存档目录保持；完整error新唯一行0、六条Unexpected token身份保持、扩展资源相关错误0。多次读档后错误次数不能当同协议旧核Counter相等。timing-audit.json核对证据完整性PASS，**不等于性能PASS**；私有证据／回滚R128位于checkpoint2026-10-27-r129-frame-timing。目标active，GUI性能优化、三港口末端／权限／改图新字段／并发设备／最终矩阵继续；下方进行中记录为历史。

本轮仅增加已存在DXGI Present／Present1钩子中的可选Windows QueryPerformanceCounter采样；没有新增游戏ABI、RVA、生命周期窗口、引擎调用、worker指针或生产地图关系。独立D3D实际钩子128间隔／7.1675739秒精确解析，非法值2不采样，默认无FRAME PERF。实际游戏线程61328、frequency10000000；world2…9各样本连续且明确分开。无退休10日志不能推断析构是否发生；当前性能失败不能包装为生命周期或设备全通过。

### 2026-10-27 — R128完成：当前已铺铁路统计及实际拆除／重载验证

- 主体87c2c1947d88a48bc003062a6e7efb46e52cd99593674e33b3105646622b89cb／46997736B，引导71dae36d…／779879B，源码＝dist＝安装＝最后实际加载R128。完整46 PASS、embedded／GUI结构及实际归档、selftest560391、默认隐藏vtx0通过。当前无游戏，原存档／设置／ini／content_load／continue五输入与存档目录保持。
- 当前type4已建铁路独立复制为owned builtRails，包含州内及跨市场记录；模型land连接筛选不变。worker按同一当前市场两端、完整56位物理strip去重，累计当前样条二维折线地图单位MU；正式卡片及中英提示已接通。商品／搜索／hover／货量不决定基础设施长度；已知空为0.00 MU，缺来源为--，未校准公里保持未知。无新ABI／地址／hook／引擎调用。
- 原存档实读34条已建连接（24州内），市场0为22条唯一strip；独立Python由原坐标求510.9929662664673 MU，正式510.99 MU。原版UI铁路2→1不变，1→0删除5连接（含1州内），29条／市场17strip、387.7629690309868 MU，正式387.76 MU。普通游戏变更后的市场／国内模型量与各自owned精确一致；没有用未知量替代国内模型物流。
- 同进程正常重载原存档，world1退休／world2恢复34连接、22strip及510.99 MU和原数量；我方正常关闭exit0／退休3。最终重新安装R128并启动PID60588核对实际双DLL、510.99 MU和完整中文提示后，正常退出0。拆除仅在我方未保存世界进行，没有改写原存档。
- 初次严格audit误用R127新游戏流程作同存档错误基线，FAIL及原脚本保留。临时经deploy.ps1部署R123，同存档／相同两次拆除及重载后，完整normalized error Counter与R128逐项／次数相同；最终R128启动Counter也等于旧版同存档启动。没有把已有空key报文称零错误。初局55440下次观察时已不在，退出原因／操作者／退出码未捕获，晚间另有一组已有报文，保留限制，不称整局零新增或我方正常退出。
- rail-audit.json PASS；证据与恢复入口：E:/Code/v3-analysis/checkpoints/2026-10-27-r128-built-track/{NEXT.md,rail-audit.json,live,logs}。先前R127改图场景属先前构建证据，新铁路字段未重跑全部改图矩阵。港口三末端、运河开放权限、更多省份／bitmap改动、渲染并发／新设备／完整生命周期、公里标定、3×60秒性能及最终同构建验收继续；历史R1200.12未解释，目标active。下方R128进行中记录为历史过程，以本完成记录为准。

本轮未新增已验证ABI、偏移、调用或hook：builtRails复用既有当前typed CTravelConnection、type4、端点／定义／样条解码，并随既有完成点联合复读交接；worker持自有值。只读外部探针独立枚举原始type4目录与复制记录ID核对，随后关闭读取句柄才做owned计算；Python按完整坐标另算长度，不调用生产长度函数。实机EXE SHA256 7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f，模块7ff63d2e0000。私有探针不是玩家依赖或新增生产ABI。

实际移除运行连接98、99、102、103、111均邻接当前州433；这些是本次观察值，不作生产固定映射。market0从22strip减为17，差123.22999723548048 MU；原生paused1/date59883360始终未推进。rail-change PID55920、R123对照48192及最终R12860588均有guarded正常退出0；55440退出缺观察，单独保留。

## R127 real ZIP and replace_path (2026-10-27, live)

No product/ABI changes. R123core3be8cbcc…/46613962B, bootstrap71dae36d…/
779879B remain dist=installed=actually loaded;73native source hashes unchanged.
Identical60-file directory and deflate ZIP inherit the renamed key/geo1500
fixture. All17state definition basenames change to r127_*; metadata requests
replace_paths=[map_data/state_regions]. Actual frozen VFS shows25mounts,
1replacement, allowedRoots=1 for that directory, first root directory/ZIP.
Current state0/definition0/geo1500, peer7/definition14/geo16,11/8provinces and
all918land records are exactly identical across formats.93domestic sea links
retain model quantities; three incomplete port geometries remain partial.

Directory PID53280/start06:16:06.4887472+08:9030shipments, market2590031259fixed
(domestic2311645000), states359098837/299463152; formal219/231/207ribbons.
User-root ZIP PID25876/start06:27:42.2081083+08:9062shipments, market2596938903
(domestic2309998100), states364445907/301342341; formal222/243/188ribbons.
All three scope quantities per session match stable owned replay exactly;
stable/final probe text is byte-identical after UI scope switches. Both paused1/
date59883360/module7ff63d2e0000, actual module paths/hash verified, own normal
WM_CLOSE exit0 and retirement2. Fresh economies are not seed-controlled.

Initial external ZIP PID58968/start06:18:48.9730124+08 terminated abnormally
before world/UI; retained exception/debug/session logs, no captured exit code
or actual module enumeration, never a normal-close claim. Current exact EXE
SHA7f53e326…/preferred base140000000: static38A1710 establishes mod manager root,
38A25B0 iterates current paths,38A4B90 ZIP branch extracts metadata by opening
that root plus basename(3B12280),38A5440 applies current replacement paths.
Same ZIP works from expected user mod root; arbitrary absolute ZIP locations
are not accepted by this evidence. Ghidra pseudocode plus locally derived RIP
xrefs remain private; no new ABI or engine calls were enabled.

Audit confirms original inputs/assets restored, hash-guarded created user ZIP
removed, no new saves/processes/normalized error rows. Initial finish.py rejected
missing baseline parser rows after early termination; do not reinterpret its
message as additional Unexpected-token errors or erase failed evidence.
Checkpoint2026-10-27-r127-zip-replace/{zip-replace-audit.json,static,live,NEXT.md}.
Not all Mods/ZIP layouts, added/split provinces/bitmap adjacency, old-save
migration, full lifecycle/device/render concurrency/performance/final acceptance.

## R126 actual province membership exchange (2026-10-27, live)

No product/ABI changes; R123core3be8cbcc…/46613962B and bootstrap71dae36d…/
779879B remain installed=dist=actual loaded, native73source hashes unchanged.
Two private real directory fixtures differ only in state geographic fields,
the corresponding two historical owned_provinces payloads, and metadata. All
physical spline/locator/bitmap files, state keys/history ordering/other effects
and building/population authored data remain identical between fixtures.

Current state0/definition0 and state7/definition14 retain identities. Their
province counts11/8 and geos1500/16 become8/11 and16/1500. Private VM_READ
probe checks definition-region pointers against actual runtime province.state18;
19unique numeric definition geometry witnesses exchange exactly. Sixteen reused
numeric province IDs represent different geometry, so old-ID binding fails this
scenario. Witnesses are observational private evidence, not a new production
layout/identity algorithm. Centroid-pixel color identification failed exact
multiset checks and was rejected, without nearest-color fallback.

All918current land records, including current type4, exactly match the expected
state endpoint permutation by physical key;12rebind, no unexpected/missing rows.
Control40612/start05:23:02.7114236+08:9019shipments, market2586409759fixed,
states359724016/293273646; formal226/234/188ribbons. Swapped64968/start
05:49:20.0878096+08:9022shipments, market2574204225fixed, states307078798/
402427395; formal215/196/247ribbons. All quantities exactly match current owned
replay; both state local_goods panels and maps viewed. Both module7ff63d2e0000,
paused1/date59883360; actual VFSroot and loaded pair verified. Final full probe
output identical after UI scope switches. Fresh worlds are not seed-controlled.

Initial6848fixture omitted historical owned_provinces changes; private consistency
probe refused it before formal UI. Only fixture/generator repaired, failure kept;
no product guard relaxed. All3 own normalWM_CLOSE exits0/retirement2, original
inputs/assets intact, content lists restored, no new saves/errors/game. Evidence
checkpoint2026-10-27-r126-province-membership/province-audit.json/NEXT.md.
Not new/split/merged provinces or changed bitmap adjacency, ZIP/replace_path,
old-save/all-Mod or final lifecycle/device/performance. Historical0.12 unresolved.

## R125 actual geo renumbered directory Mod (2026-10-27, live)

No product changes: exact R123core3be8cbcc…/46613962B remains source=dist=
installed=actual loaded, bootstrap71dae36d…/779879B. No new ABI/hook. Real
private Mod keeps new state key/runtime0/definition0 and changes geo20→1500,
with6point IDs/12connection keys/5generated locator IDs updated. Three land
keys reverse canonical direction. All coordinates/strip IDs/province members
and history remain unchanged. fortification ID20 is a different location and
is not rewritten by numeric coincidence. Existing decoder verifies full table.

PID64280/start04:58:46.1312722+08/module7ff63d2e0000, paused1/date59883360.
Actual VFSfirst root=geoMod/25mounts. Production decoders read geo1500 and all
new point/key references; corresponding old6/12 identities are absent. Owned
after-Start allocation918land/93sea/8980shipments matches production; state
353800627fixed/domestic341011600 exactly matches formal publication. Formal
market25790.44463/223ribbons and state0 local_goods/234direction ribbons viewed.
External222ribbons omit multi-candidate selector calls. External market total
was not separately printed, so only exact state/model counts are audited.
Original3partial sea endpoints remain; no fabricated geometry.

Before-Start model8892/state353799427 differs from after-Start8980/353800627
by0.012 state quantity, but complete pre-stage solver inputs were not saved.
Do not use this to explain the historical R1200.12difference. Own normal
WM_CLOSE exit0/retirement2, original inputs/43source assets unchanged, test
content list restored, no new saves/errors/game. Checkpoint2026-10-27-r125-
geo-renumber contains geo-audit.json/NEXT.md. This is one consistent geo
renumbering case; province topology, ZIP/replace_path, all Mods, old-save
migration, device/concurrency/performance and final matrix remain open.

## R123 domestic deterministic graph and real renamed-state Mod (2026-10-26)

Core3be8cbcc92fa39160fa9f700c97fdb8e46e9f1e962d74c4494fcd528c00ae074,
46613962B, bootstrap71dae36d…/779879B: source=dist=installed=actual loaded.
No new native ABI or hook. Only owned domestic graph node/land/port-lane
enumeration is sorted by current full IDs so equal-content cache identities
cannot hide different equal-cost allocations or selected physical strips.
The regression failed before the fix and passed after; complete46 checks plus
resources/GUI/GPU/selftest560420/default-hidden passed.

Private real directory Mod renames/reordersMidlands: runtime433→0,
definition17→0, geo20 unchanged; new key absent embedded data. PID35004,
start19:27:39.2788735+08, module7ff63d2e0000, actual first VFSroot=Mod/25mounts.
Initial9053 modeled shipments and state357465715fixed exactly match owned
replay. Formal state0 local_goods/244ribbons and market25881.45827/219ribbons
were viewed. Two original time steps date59883360→59883372 produced917land,
93sea,9030shipments and domestic23017.375; production and external replay
agree, market updates25879.45727, state quantity remains3574.65715.
External231ribbons omit multi-candidate selector calls and are not the formal
render count. NormalWM_CLOSE exit0, retirement2, original save/settings/ini/
continue unchanged, temporary content list restored, no new saves/errors/game.

Earlier R120PID66860 fresh-world formal360976587 vs owned360964587 differs
by12000fixed(0.12). Its exact failed audit remains. Old algorithm agrees with
new on the current fresh world too, so do not claim this historical discrepancy
was caused or resolved by canonical ordering. Current numerical checks PASS
within this world only. Original malformed fixture left a BOM in the middle,
lostSvealand and produced335new errors; decoder correctly refused it. Fixture
repair affected private test files only. Checkpoint2026-10-26-r123-state-identity
contains failures, source/DLL backups, current-audit.json and NEXT.md. Geo and
province topology changes, ZIP/replace_path, full port endpoints, permissions,
device/concurrency/performance and final acceptance remain open.

## R122 active directory Mod same-key geometry (2026-10-26, live)

No product change; exact current core90e577c3…/46338126B remains installed=dist.
Two private real content-loader Mods override the same installedsplnet. Only
point277150326 X changes by8 or18; Z2767.87548828125, connection298634505466509059,
strip364035 and controls8390108,277150325,277150326,8691408 remain identical.
Independent decoder verifies every key/control/other point unchanged. Installed
source91c0957b… stays intact; no game asset is edited or redistributed.

| Content order | Own PID/start +08 | Actual VFS first roots | Current native X |
|---|---|---|---|
|lower,upper|48336/17:50:35.9300278|upper,lower (26mounts)|4071.94775|
|upper,lower|14144/18:07:13.0089212|lower,upper (26mounts)|4061.94775|
|none|67328/18:11:20.5641874|no test roots (24mounts)|4053.94775|

All observedphase4/paused1/date59883360/module7ff63d2e0000. Private external
probe reuses the production current spline/trade/domestic decoders, jointly
rereads and closes its VM_READ handle before building ownedfacts/view/ribbons.
The target strip has positive current-market traffic and its exact changed point
occurs in the resulting ribbon. External209segments omit unselected multi-route
candidates; they are not claimed equal to formal production's218ribbons. Formal
market218/25963.19216 and changing/restored London curve were visually confirmed
in all3 sessions. Domestic23098.111/9038shipments remain equal for this sea-curve
change. Screens are qualitative; animation/map-lock differences were not used
for pixel-precision assertions. Actual loaded pair verified in final session.

All3 own WM_CLOSE exits0; original save/settings/ini/continue bytes unchanged,
temporary content_load restored atomically under expected-hash guard. Mod
error.log normalized full-row multisets equal the vanilla control (new rows0),
including the known6base Unexpected token rows. No game process remains.
This proves real directory precedence and one same-key/endpoint-preserving
geometry change across normal launches, not ZIP/replace_path, changed state
key/IDs/province topology, in-process map reload or all-Mod compatibility.
Evidence:checkpoint2026-10-26-r122-active-map-mods/{scenario.json,
map-mod-audit.json,live,protected.json}. Source unit tests remain R120's46;
production was not rebuilt merely for this validation.

## R121 domestic port gaps: current path versus spline sea endpoint (2026-10-26)

No product change from R120 core90e577c3…/46338126B. Exact EXE7f53e326…,
image140000000, Ghidra explicit programvictoria3.exe. Independent installed
splnet91c0957b… contains ports for all three states. Own66308/start
13:13:43.6375042+08/module7ff63d2e0000, observedpaused1/date59883360, production
decoder replay after closing its process handle confirms the following:

| Lane | Current state/key | Definition / geo | Existing port's sea geo | Lane member geos |
|---|---|---|---|---|
|50331727|207/WEST_MICRONESIA|578/555|3002|3091,3106,3052,3125,3088,3089,3104,3108,3158,3157|
|50331739|936/FORMOSA|530/510|3157|3056|
|50331743|608/NORTH_BORNEO|561/541|3056|3052,3125,3072|

Those existing port keys are298545170150208259,299077746094624259,
298730712737305859 respectively. None connects a sea member of its current lane.
Installed state-region files' naval_exit_id3157/3056/3072 corroborate the paths;
local files do not prove a fresh active-VFS override. No IDs/keys from this table
are introduced into the product.93sea/918land/9038shipments and final edge
quantities0.586/418.679/0 remain unchanged.

Full original025E8450/8690/8070 assembly only probes type3 for the enumerated
member sea and current state region; no fallback via another sea was found.
0124D460 uses stateE78 thenworld3C70. GUI registration021CDC0 binds
GetNavalExitStateRegionDesc wrapper01DB10E0, closure01DB03A0 calls01572DE0;
that getter reads region-definition124 and uses context databasef98/countf60.
064C090 resolves TLS context through0514F088/active0514F090 and may initialize
or lock, so it was not called. The generation command writes map definitions
and was not executed. An attempted external runtime124-field follow-up was
rejected before reads because66308 had already ended; no result is claimed.

Session termination cause/exit code were not captured. Last log505501078 has
world retirement2; no live process remains. Input file bytes are unchanged,
related parser errors0; this is not normal-close or endpoint screen acceptance.
Source evidence:checkpoint2026-10-26-r121-port-geography/{static,port-audit.json,
live/port-replay.txt,live/keys.json,live/after.json}. Full shore geometry still
needs another verified native source; no bridge/path rewrite was enabled.

## R120 original world aggregate destruction event (2026-10-26, live)

Exact EXE7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f /
image140000000. New RVA00811660 entry48895C2408488974 is the original destructor
of the native aggregate owned at machine+120. Independent current PE disassembly
of007DA820 shows RCX=[this+120] at007DA94E, call00811660 at007DA951, then free;
007F2870's release wrapper also passes only RCX at007F287E/call007F2881. Original
entry overwrites RDX before use. Hook signature void(this), original always
forwarded. No game destructor is invoked by the adapter outside its own call.

00811660 destroys the embedded CStateManager at world+1C00 through0133B580;
typed state DB is its base at+10/world+1C10. RTTI044E67B8=CStateManager,
044E6710=TPdxRefDatabase<CState,7>. Its late-clearing variants0133B580,
01344AB0 and013564A0 were found by independent RIP writes, not Ghidra's sparse
xref list.044ADFC8 is the CCountryManager base vtable, not an invented whole-world
class name. Pdata/unwind data references were excluded from caller proof. Full
ABI/caller/RTTI bytes and decompilation are in checkpoint/static.

The event compares the nativeWorld argument to the mailbox's owned identity,
never dereferences it. Matched destruction advances mod epoch before the
original body, replaces an old resident request with a new-epoch copy, clears
ready data and forgets old identity. Late publications fail pointer ownership;
first paused commands still have a request. UI-first retirement is adopted
without incrementing twice. Both event/capture hooks must install to enable
formal capture. The existing identity-change fallback remains. Same-address
reconstruction is covered by an owned helper regression, not actual allocator
reuse evidence. No engine DB scan, GUI call, solver or new simulation lock occurs
in the callback. Rendering retains its per-submission epoch check; atomicity of
check/native call versus retirement remains a separate unproved boundary.

Core90e577c382bca48bbb9bb31cedd90e720659b6fe624b4f264a0594d2c08d5c27,
46338126B, full46 PASS/GUI archive/embedded/selftest560422/default-hidden passed.
Own400/start2026-10-26T12:40:50.3461912+08/module7ff63d2e0000 live event thread
57852, completion58948. Direct reload logged event2 before UI adoption and first
new paused copy(world2/rev8); menu return event3 then Continue first copy3/rev12.
Both market views recovered218 ribbons/25963.19216,uploads218 unchanged; state433
248 and minimize/actual resize resumed. Normal WM_CLOSE produced event4 and exit
code0; originals byte-identical. Actual source addresses changed in these runs.
Other reset paths/real same-address lifetime reuse/new-device/full A24 remain
unaccepted; see STATUS and checkpoint/retirement-audit.json.

## R119 current same-process lifecycle observations (2026-10-26)

No production/source/binary change from R118aff98195…/46324762B. Own47988,
start11:06:15.4045767+08,module7ff63d2e0000,completion31784/UI65196. Current
same-save direct reload advanced world2/rev11 and recovered218 retained native
objects without additional uploads. Menu return observed phase4/stateDb0→6→0
and no active document; a new French sandbox advanced world3, accepted different
owned facts(25404local rows/3748native pairs/8990model shipments), showed market2
9878.27752/141 ribbons. Later loading the original British save advanced world4,
rev21, recovered the326 retained-object pool and market0/25963.19216/218 ribbons;
state433/local_goods/248 resumed. Original-world identities differed in external
read-only observations, not retained engine pointers in workers.

Actual minimize/restore and client2560×1440→1920×1080→original worked with the
French view. This is same-device window coverage, not device-removal/new-device
rebuilding. Periodic submit logs contain no lines in the three retire/recover
intervals, but this cannot prove an engine retirement event or address ABA
safety. Native stateDb clearing before phase change is a future discovery clue,
not permission for another access window. All own ingame observations were
paused1/date59883360; normal close preserved save/settings/ini. Full A24 remains
partial. Evidencecheckpoint2026-10-26-r119-world-lifecycle/{lifecycle-audit.json,live}.

## R118 native port-lane travel distance (2026-10-26)

Same exact EXE7f53e326…/image140000000.0131A290 writes the sixth argument to
`CShippingLane+60`;0131A590 moves that field into R9 before01419540(kind0,
call0131A819) or0141AE70(kind1,call0131A74F).01419540 supplies the original
`SHIPPING_LANE_BREAKDOWN_TRAVEL_DISTANCE` tooltip's `DISTANCE` value.01419390
uses native fixed1e5 arithmetic and current distance coefficients. The decoder
now names this field `travelDistanceFixed`, rejects negatives and preserves
valid zero within-region distance; kilometres remain unverified. No new native
call or access window is enabled. Exact disassembly, strings and decompilation:
checkpoint2026-10-26-r118-domestic-port-endpoints/static.

Missing port graphics does not retire the copied country-referenced kind0 lane.
Its nonempty current original path and native distance provide model
connectivity independently of geometry. Partial rendering requires a witnessed
port and unique original strip chain, sharing all current strait guards; no
shore bridge, branch selection or static map table is introduced. Production
build/live evidence for this revision is tracked at STATUS.

Coreaff98195056c0e42f4535ed3c9666f13b69f7d88dd11f06b42585fe545170a85
(46324762B) passed46 unified checks and loaded in own paused61904/start
2026-10-26T10:44:26.1938907+08:00. Current production capture/model reports
93sea/918land/3partial/0unavailable and9038 shipments, matching owned read-only
replay after closing its process handle. Normal close preserved save/settings/ini
bytes. Individual partial-lane screen verification remains pending; see STATUS
and checkpoint2026-10-26-r118-domestic-port-endpoints/live.

## R117 current domestic logistics sources (2026-10-26; default-save live baseline passed)

Exact EXE SHA2567f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f,
image140000000. Original025E8880 resolves market848 owner country, iterates its
CPdxArray338/cap340/count344 full shipping handles, resolves00C6F920, accepts
kind1C==0 and passes from48/to4C state objects plus ProvinceSet30 to025E8450.
This is separate from world-market state890 export records. New domestic adapter
validates the complete shipping decoder, typed countryVT04494CB8/fullID18 and
each exact country338 reference. MerchantMarineDemand58 remains unrelated to
modeled goods quantities.

Original012A0910 chooses current CTravelConnection20;013B2110 updates that value.
Currently usable ground categories2/3 and rail4 are distinct from unavailable or
other categories0/1/5/6/7. Rail4 requires definition58==1 and both endpoint rail
flags19 plus original same-market/eligibility checks. Production reads the
current category rather than reproducing these checks. Manager world5618
VT044BB380/roster5680 remains the R96 source; objectsVT044E08D8/id8/definition18/
type20/from28/to30. DefinitionVT04515E20/id10/nativeSplineKey50/type58/path68.
Runtime endpointsVT044E3D78/definition78 must match definition40/48; endpoint
definitionVT044F8A78 (RTTI CTravelNodeDefinition), province ushort44, fixed XY48.
01315FF0 and012A07E0 call0120E7D0(definition44), which resolves machineB0 pointer
array/countBC; runtime provinceVT04437A00 (RTTI CProvince), current full state18.
This binds exact current state ownership, never key-name/ordinal/nearest matches.
Current path coordinates are compared to the native spline with the original
floor-to-integer quantization, including split connection strip orientation.

All new sources join the existing completed-command owned snapshot and combined
reread; no new engine calls or worker pointers. Original foreign amounts remain
1e5; domestic local counters convert to milli-orders and original imports/exports
are incorporated before allocating residual supply/demand on current same-market
land links and original domestic port lanes. Model/state-terminal semantics,
tariffs, precision and separate Freight ledger are in FREIGHT_MODEL R117.

New source tests cover typed full handles, current province ownership, changed
geometry, masks/bounds, mutation and retirement; view/model tests cover land/sea,
unmet/disconnected, hover/state, cache identity and document separation. Original
cache-only comparison was reproduced failing on domestic change before repair.
Paused external observation59444/start2026-10-26T01:16:50.8899355+08:00 on R116:
25396 local orders/93 country port lanes/918 same-market land links(908type2,
10type4); after closing its process handle, owned replay produced8974 shipments,
8272 with land/2083 with sea;90 port shapes available and3 missing native endpoint
links. No save/date advance. Production core87b6d408faa070f97dbd372a43df5ba51c35a0167bb2fc612f4f151d84b2c88e/46295447B is now installed=dist and live verified:
market25963.19216=original2865.08116+domestic23098.111,218native ribbons;
state433 column/248incoming-outgoing ribbons; Freight30585.31/23098.11/7487.20.
Initial same-code f0dccd6e core ran46PASS and145s zero-upload state view;
final resources-only core has identical.text/.rdata/.pdata, final GUI/embedded/
selftest560369/default-hidden PASS and20actual reopen cycles with uploads218
unchanged. Player GUI/shader edits retained/actually packed and mounted before
VFS freeze, untouched localization upgraded. A test-only outer // shader comment
caused3parse errors; corrected to # and repeated with0related errors. Test
comments restored underhashguard. All ownpaused sessions closed normally with
original save/settings/ini bytes unchanged; no game remains. Three destination
port shapes, actualmap-Mod/lifetime/railunits/performance/finalmatrix remain. Raw evidence checkpoint2026-10-26-r117-editable-
resources-and-native-connections/static/original-domestic-sources.txt and live/.

## R116 native capture independent of legacy embedded maps/orders/rails

CopyNativeSnapshot request now only world/revision/override; no geography/regions prerequisite or DecodeNativeEconomy/DecodeNativeRails call. DecodeNativeTradeStates copies current full typed state registry and exact State.GetMarket chain, explicit market0, duplicate/full-generation/slot/type/bounds/combined-reread/retirement guards. Legacy NativeStateEconomy.region explicitlyUINT32_MAX for this roster: never an engine definition ID. NativeSeaGeography independently retains current definition/geo per full state and provinces; native splines/dimensions/ports and original selector remain in sameR101scope/outercopy. Model decoders retained offline/debug. live_shipping_probe uses this production roster, replacing hardcodedmarket0 diagnostic rows.

Private exactoldsource request-no-map reproduction reads0/missingcurrentrequest/exit1; newreads1/outsideverifiedscope/exit0. Negativecontrol headers isolated to avoid mixedpragmaonce definitions. New roster tests unknownModkey/absentembeddedtable/unreadablelegacycounter and rejection cases integrated existing economy/snapshot suites. Full45PASS/zeroFAIL/exit0, embedded; selftest560470/default-hidden. Coref1ebed5606eb00bae67ff10e35b7935060ca4d18865265be4ccfb907279eacd5/43064787B (601940Bsmaller), bootstrap71dae36d...779879B, assets862ecc6e...unchangedR115. Installed via deploy.ps1 after preexisting65156 process independently gone (no inputs/exit sent).

OwnnormalPID32276/start2026-10-26T00:24:06.1455456+08:00/EXEbase7FF63D2E0000/completion67776: code-specific legacy_orders/rails=not_collected capture141,156,157,157ms;1147states/53goods/57groups/3192destinations/3928alternatives/3192native selections/3569pairs. Market0Q286508116fixed and112formalribbons visible without modelinput. Newroster externalVM_READ fullworld/sea/hub/spline probe passed,4200detailed/2513groupedgoods exactbinding, map8192x3616/33780points/4273strips/4271connections; externalmulticandidates stillunknown/no nativecall. Allownsessionpaused1/date59883360;normalWM_CLOSE exit, originalsave/settings/ini bytesunchanged,relevantGUI/shader errors0,no game. Installed=distR116. Not an actual modified-map case: realModoverride/changedkeys/IDs/samekeygeometry, futureopen/permission changes, world/device lifetime, railwayphysicalunits and3x60s performance/finalaudit remain. Checkpoint2026-10-26-r116-native-capture-map-independence/{capture-gate-repro.cpp,before-repro.log,after-repro.log,live,logs,source-after.json,NEXT.md}. Goalactive/R116PROGRESS.


## R115 formal native trade view and autonomous live acceptance

NativeTradeFacts/View own numeric source world/revision, exact1e5 quantities and full56-bit selected original strips. Facts bind detailed/grouped exports via CURRENT trade centers; geometry composes proven port/sea mapping into unique endpoint chains. No model solver/allocation/view in formal native branch. Unknown geometry keeps quantities and coverage distinct from demand. Native/raw X/Z passed directly to NativeArrowPoints(nativeCoordinates=true), preventing near-zeroZ precision loss. Fixed original market denominator precedes filters/state/hover; matched native goods IDs authoritative. Session/engine ignore diagnostic legacyorders/rails only when bothsources native; retain exact native content comparison/world gates. Balanced zero-net uses one geometry with max(forward,reverse) alpha artwork;7effects/56width styles added (148total), WARP union/negativecontrol passed.

Final DLL11b3faddc9c2ff9fdbb940c2eab2ae2638fd48f087b0db245b8def1a96ef9f7d/43666727B, bootstrap71dae36d.../779879B. Full45PASS/exit0, embedded and GUI archive parity, selftest560394/default-hiddenvtx0. Installed throughdeploy.ps1, loaded samecore in ownPID15712/start2026-10-25T22:39:01.4193294+08:00, EXEmodule7FF63D2E0000/UI28468/completion64784. Native3192selected/3192destinations/3928alternatives/57groups/3569statepairs/53goods. Map8192x3616/33780points/4273strips/4271connections/12definitions/9runtimeStraits. Formal market0 Q286508116fixed (2865.08116),112ribbons, drawable217272227fixed; independentVM_READ typed-state-market export sum identical. Closeddefs0,1,2 each have2raw sea connections: all3928existing original alternatives through productionmapping returned0closedkeys,652directed governedadjacencies rejected. No external selector invocation. Panama/Suez screenshots no isthmus-crossing trade; UKclosezoom port/sea geometry visible. This validates currentclosedworld, not future liveopen/permission changes or allmapmods.

Chinese clothingfilter2goods Q19195477fixed; emptymatch0ribbons/Q0; staple86017340fixed. Hover112->35ribbons leaves Q286508116/card/shares; hide/show andclose withdraw/reopen restore. State433 local_goods nativeQ11800421/drawable11279952fixed/114input-output ribbons visible. Ownnormalboundedadvance59883360->59883366,pausedagain/no save: legacyorders25396->25395 but native unchanged; revision2/gen12 frame retained. Initialstable70s uploads112unchanged;20normalclose/reopen confirmed panelstate and eachfresh112ribbonframe,allocReuse/viewReuse1,uploads315unchanged. Privatebytes14962966528first->14950445056last;last15range14935498752..14976573440,RSS+4.6MB. Initialcycleharness omitted .1s cursor-settle, causing previous-widget clicks; fixedtest passed. Not fullfilterpressure or3x60s frame/simulation performance.

Ownpausedsession normallyWM_CLOSE exited; no game. Originalsave/settings/ini byteidentical, relevantGUI/shader errors0; currentR115 kept installed, rollbackR102 materials isolated. Pending: legacyembeddedregions/rails capturegates block complete mapmodcompat, realrailphysicalunits/length--, futureopen/permission change, fullworld/device lifecycle/performance/final audit. Evidencecheckpoint2026-10-25-r115-native-formal-view/{native-view-proof.md,live,logs,source-after.json,NEXT.md}. PreviousgoalturnR114 wasPROGRESS; currentR115 isPROGRESS, goalactive.


## R114 exact owned native snapshot handoff identity (not live)

PublishRealEconomy calls LogisticsSession::HasEquivalentSnapshot before worker/engine: old implementation ignored nativeTrade despite engine deepfingerprint, and actual private standalone sameworld/samelegacyorders/remove-currentstrait returnedequivalent (exit1REPRO,before-repro.log). Newnative_trade_identity.h+sessioncondition compares all owned native centers/state-scope/detailed+grouped1e5quantities/groups/alternatives/selected/provinces/passages/currentfullstraits/costs/geometry/sea definitions+geo+adjacency+hubs/availability exactly. No shared_ptr address/hash-only equality. Canonical irrelevant state/group/member/destination/pair/good/strait/definition enumeration; normal matched key order avoids sorting allocations. Candidate/control/adjacency/passage sequence remains semantic. Model/native source transitions not equivalent. No engine call/hook/scope change.

Private fixedinitial reproduction passes; FINAL canonical code+expanded closedcanal/sameorders/recycledhandles/currentcenters/1e5/cachedchoice/samekeygeometry/emptyvsunknown/component/reorder cases in unifiedsession suite PASS. Fullbuild+42PASS/zeroFAIL/exit0; finalembeddedhash8d717ce7f324088ddaff029e478feee06b293d16ffb8f20634f8198b42700406/40051850B; selftest560363/default-hiddenvtx0PASS. GUI/shader assetsunchanged1bf9c374..., bootstrap71dae36d...779879B. Initialcandidateebaebb77...superseded, final logs*-round114-final.log. NOTdeployed/live: installedR10251ec975f...35151385B/no game. R112actualselector evidence is forprevious6a83d933...only. Formalnativeview stillguarded; next nativefacts/view/stats/drawing/UI/currentgeometry. Checkpoint2026-10-25-r114-native-snapshot-dedupe/{dedupe-proof.md,before-repro.log,after-repro.log,session-repro.cpp,logs,source-after.json,NEXT.md}. No canalvisual/mapmod/lifecycle/performance completion.

## R113 selected native geometry chain composition/direction (tests/probe only)

BuildNativeTradeGeometry composes current013C4E70 endpoints with selectedProvinceSet and verified original generic025E8450/8070 port-key rules, retaining full56-bit strips, connection control order/endpoints and original low29naval geo*100 anchors. Only a unique complete chain (endpointdegree1/internaldegree2/allstripsvisited) is oriented; branches/disconnected/empty/missing remain unknown. No new solver/nearestport/quantity allocation. This composition is not claimed to be original world-market renderer usage; observed helper callers remainCShippingLane.

Unified42PASS/zeroFAIL and targeted closed/open/recycled canal, reverseflow, curvedseam, same-low32/full56 IDs, inducedtriangle negativecontrol, empty/missing/badcontrols/cancel tests. Autonomous externalOWNR10251568/start16:25:54.6219184+08/module7FF63D2E0000/thread38216/context0:2974single-selected positivepairs->885unique native routes,861chains->2911pairs/7794stripoccurrences;24missing-endpointport routes->63pairs, no actual valid-port branch found. Remaining external multi-selector unknown, no nativecall/formaldrawing. Paused1/date59883360 unchanged/noinput/save; normalWM_CLOSE, originalsbyte-identical. Dist6a83d933... and installed51ec975f... unchanged; no redundantDLLbuild/deploy/selftest. Checkpoint2026-10-25-r113-native-route-geometry/{geometry-proof.md,logs,session-after.json,source-after.json}. R114 must fix reproduced HasEquivalentSnapshot ignoring native content before view cutover; formalUI/allmap/lifecycle/performance pending.

## R112 real candidate capture, original selector, owned handoff and one native update (not formal route drawing)

Same R11141PASS-built payload6a83d9330f398f4231440629ead822eb45c5e862097cff79fe73af5d0b0d6eec/38482263B TEMPORARILY deployed throughdeploy.ps1, actual own44104/start2026-10-25T16:06:52.2788424+08/module7FF63D2E0000. Completionthread33112/UI66340. Initialpaused revisions2/3/4date59883360 copy219/203/188ms:1147states/25396orders/53catalog/34rails;57groups/3192destinations/3928alternatives/3192selected/3569pairs/9runtimeStraits;33780points/4273segments/4271connections/8192x3616;12defs/77referencedprovinces/1147states. All372multiple-candidate destinations really invoke original013BDBE0 under existing R101 owned completion scope, retain only range-checked selected indices, combined reread and UIhandoff. No external-selector invocation/remote write/debug attach.

NormalSpace with mapped scanMapVirtualKey=39 solves R111scan0 ineffective input. Foreground/PID/start guarded, one native dateadvance59883360->59883366, immediately re-paused; revision6new25395orders/53goods adopted UI (266mswholecopy). External productiondecoders/currentcenter binding still4200->2513exact. Not full sustained-running/performance protocol. F4+observed native Logistics-tab click with DPI-awareness: screenshotactual rows/cards--/unavailable, frame0/ribbons0/errornative trade geometry unavailable, proves no modelfallback; NOT finishednativeview/canalvisual acceptance. One DPI rectangle refusal beforemouseinput corrected.

Ownsession normalWM_CLOSE atpaused/newdate; originalsave/settings/ini bytes identical (settingsmtime changed onnormalshutdown only). ExactR102installeddualDLL restored only by private rollback/deploy.ps1, installed51ec975f.../35151385B, bootstrap71dae36d...779879B, repositorydistcandidate6a83d933... preserved; no game. No code change/redundant regression rerun this unit. See checkpoint2026-10-25-r112-native-capture-live/{capture-proof.md,candidate-session.log,candidate-validation.json,native-update-scan-code.json,candidate-native-guard.png,session-after.json,rollback-verified.json,rollback,logs,source-after.json}. Next nativeownedview/stats/drawing/UIlabels + current-center geometry/direction. Full map/lifetime/performance still pending.

## R111 current native trade centers, exact goods association and external live validation (not formal view)

Original01221F180 gets stateB48 CMarketArea, reads area48 full state-center and calls013C4E70(world+508,sourceCenter,destinationCenter). Current typed area DB05964EA0 entries20/slots2C, areaVT04430CD8/id8. No area/center UINT_MAX retained; populated center must be current typed primary state handle. Native group lookup manager2F8/world800: data808/count810/mask814/overflow818, stride18/occ4/key8/groupPtr10. Validate exact alignment/fullkey/pointer-index/count/unique correspondence with copied world898 group vector before owned lookup. Original destination key is literal current destination center, never member-derived fallback.

Independent raw state890 detailed and8B8 grouped goods tables retain exact1e5 int64/full peer IDs. Grouped peers may be OUTSIDE current route-cache anchor set: initial anchors-only guard was wrong and rejected before activation, removed after raw/source verification; typed peer/counter/uniqueness retained. New centers/groupPairs owned values enter snapshot/fingerprint. BindNativeTradeRoutes sums detailed by(source,current destination center,good), requires whole exact equality with8B8, binds optional sourceGroup/route index via current centers. No solver/reallocation/guess aliases/port geometry/travel-order proof.

Own externalR102 session6176/start2026-10-25T15:40:50.0705064+08/module7FF63D2E0000/thread43648/context0,phase4/paused1/date59883360.1147states/134cached members/57groups/3192destinations/3928candidates/2820single-selected;3569pairs/4200goods;2014groupPairs/2513groupedGoods.5current areas differ from cache anchor:23center534/cache551,57:610/614,81:378/228,8:114/398,89:457/458 (11members). Current lookup57keys has no aliases. Old membership aggregation2333/2513exact, current area48 aggregationALL4200->2513EXACT.3569bindings=2974crossSelected/421crossMissing/167crossUnselected/7sameCenter; missing counts are records, not physical routes. External multi-selector NOT invoked. No new DLL capture/formal UI acceptance.

Guarded normalSpace taps/hold attempted bounded update after screenshot paused1836-01-01, but all observed pause/date unchanged; running-state NOT verified, no no-input claim. Initial foreground refusal occurred beforeinput. Read-only memory/query handles only/noattach/nativecall/write/save. Own paused/date-unchanged session normalWM_CLOSE; original save/settings byte-identical before/after, no game now. Candidate6a83d9330f398f4231440629ead822eb45c5e862097cff79fe73af5d0b0d6eec/38482263B NOT deployed; installedR10251ec975f... unchanged. Fullbuild+41PASS/zeroFAIL, embedded/GUIstructure+actualarchive/selftest560418/default-hiddenvtx0 pass.

Port caveat:025E8450 observed callers025E8880/025E90F0 useCShippingLane; R110 world group representative port counts were diagnostic caller inputs, not proof of original world-market endpoint rendering. Need actual current-center geometry composition and determinable direction, native candidate capture/selector live, same-source formalworker/UI. Checkpoint2026-10-25-r111-native-trade-association/{association-proof.md,association-assembly.txt,static,trade-table-comparison.json,area-endpoints-live.json,area-association-confirmation.json,group-lookup-live.json,session-after.json,logs,source-after.json}. No closed-canal visual/mapMod/lifecycle/performance completion claim.

## R110 original port/hub mapping and autonomous external live validation (not formal view)

Original025E8450/8690 enumerates selected ProvinceSet/native adjacency, type2 sea keys for member neighbor, port025E8070 for outside neighbor. StateE78 definition indexes world3C70/cap3C78/count3C7C; CStateRegionVT044DF758, template90 must match current state definition/geo.02607620 checks categories3/0/2/1/4 from global051385C0 CPdxArray(data/capC8/countCC,5rows16). CURRENT registered hub objects are CGameObjectLocatorVT047295F0 (actual live all5), slot50=00624690 returns true; field90 is category12, NOT validity. Parsed CMapObjectTypeVT04725B98/IsValid029E04B0 and parsed filefield98 do not describe these live locators. Runtime finalize02C04CF0 assigns locator98 index. Current CGraphicalMapVT0464E4E8+290 manager, manager1F8/cap200/count204 outerarray, rows18hex each innerCPdxArray; runtime98 indexes it. Original02C10470 sizes inner transforms to locator instance counts. Instancearray50/cap58/count5C stride30hex, unsigned geo4 sorted/lower_bound; current transform stride28hex/X0/Z8.02607620 requires(x>=0 ORz>=0) AND not(x==0 ANDz==0). Known nulltypeVT047435C8/slot50=0061BE40 false. New decoder uses current transforms and source rereads, never authored fallback/engine getter/lock.

025E8070 type3 packing is original type2 packing with low6=3, independently calibrated actual splnet3000/1=04249F0020001903,3000/4=04249F0020006403,3000/152=04249F00200ED803.025E7F00 score is only endpoint-state buildings, no candidate key/province parameter, so every candidate for an endpoint ties; strictgreater update retains original first existing key including known empty. SelectNativeRouteConnections preserves this membership/boundary/native enumeration behavior and uses prior selected-path strait constraints. No new Dijkstra/nearest/blacklist. Whole native geometry helper still forbidden in capture access2/hidden0 (score assertsInterface1 ORhidden1). stateHubs/fullstateID->bool joins owned NativeSeaGeography and fingerprint, no worker engine pointer.

LIVE CORRECTION: 5of57 groups have representative outside members:893->{894},868->{21},602->{604},361->{373},356->{355}; original013BF640 accesses representative/members independently. Removed only unsupported anchor-in-members check; complete primary handles/known peers/unique representatives+members remain. Saved old-source regression fails, current passes. All3569positive directed pairs map source/destination to exact explicit group members (3562crossgroup/7samegroup); do not infer representative membership.

User now explicitly permits autonomous startup/operation validation. Own normalEXE -gdpr-compliant -continuelastsave PID46096/start15:02:25.0679591+08/module7FF63D2E0000, thread62036/context0,phase4/paused1/date59883360 throughout. InstalledR10251ec975f… loaded; current external production-decoder probe onlyPROCESS_VM_READ/THREAD_QUERY_INFORMATION/noattach/write/nativecall/input. Hidden uniqueSDL_app/titleVictoria 3 bound by exactpid/title/class, not first thread/othercontext. Nativegoods catalog53 checked. DecoderPASS57groups/3192destinations/3928candidates/2820single-selected/3569pairs/4200goods/9runtimeStraits;8192x3616/33780points/4273segments/4271connections;12defs/77referencedprovinces/1147states.2820selected routes map19046type2memberships/5592portoccurrences;652directed incidental strait adjacencies rejected (not physicalcanal count).372multi-candidate destinations remain unselected externally. Sequential stable reread is not a native access lease or new candidate capture/visual acceptance.

Current unbuilt defs0canal_panama35972/35975,1canal_suez35978/35983,2canal_kiel36047/36030, all absent from current runtimeStraitDB. Names diagnostic only. Own paused/unchanged session normalWM_CLOSE, no forced exit; no game process afterwards. Originalsavefecc16f…9176649B andsettings07279b75…2933B byte-for-byte unchanged. No candidate deployment.

After live corrections finalbuild/entry40PASS/zeroFAIL/exit0, finalembedded/GUIarchive/selftest560441/default-hiddenvtx0PASS. Candidate payload5c613bd61457707d42550190bcd04752cca1bad7961a2021f0fef9f21a1e8ccb/38359345B, bootstrap71dae36d…/779879B; installedpayload51ec975f…/35151385B. Formalnativeworker/UIguard still rejects pending view adapter. No canal visual/performance/lifecycle/mapMod acceptance claimed. Checkpoint2026-10-25-r110-native-ports/{native-port-proof.md,port-layout-assembly.txt,locator-identity.txt,static,group-roster-live.json,hub-types-live.json,strait-definitions-live.json,representative-negative.txt,session-after.json,logs,source-after.json}.


## R109 native directed exports, full catalog and snapshot content identity (not formal/live)

Exact EXE7f53e326…/base140000000. Original01962410 direction1 selects primary strings GOODS_PRICE_WORLD_MARKET_IMPORTS_HEADER(0454FD20), GOODS_PRICE_WORLD_MARKET_MARKET_IMPORTS_DESC(0454FCA8), GOODS_WORLD_MARKET_STATE_TO_STATE_IMPORTS(0454FDA0); else selects EXPORTS0454FBA8/0454FC78/0454FD70. Its closure01962ED0 exports branch queries01222400(selectedState+18, out, otherState primary+8, baseGood), which reads common880=>actualstate898, maskcommon88C=>state8A4, overflowcommon890=>state8A8; entries60hex/fullpeer8/counter10.0103F440 uses baseGood10 ID/mask and returns originalint64. Thus table owner exports TO peer. Import perspective queries the SAME source table, not an extra reverse flow.

NativeStateTradePair fields now source/destination; kNativeTradeQuantityScale100000, independent from model1000. No conversion/truncation to milli-orders. Snapshot owns complete NativeStateEconomy roster. Common subobjectstate18 has its own initialized ID atactualstate20; equality with primary8 is NOT proved/assumed/read by this decoder. Explicit export branch supplies all required direction evidence without this assumption. This is interpretation of original detailed trade data, not merchant-marine demand, treaty commitments or proof every represented unit physically traverses drawn lines.

PopulateSnapshotGoods now uses full current catalog, including trade goods absent from state production/consumption. Legitimately empty orders keep metadata; unknown order IDs still fail. FreightEngine fingerprint includes native presence, exact directed quantities/goods, states/region/market, groups/members/destinations/candidate order/selection/provinces/straits/distance, full current strait handle/definition/sea pair/tariff modes, all current spline dimensions/points/ordered controls/full56 segmentIDs/full64 connection keys (including known empty), and sea definitions/province adjacency order/state geo identity. Canonical sort only unordered enumerations; native tie orders preserved; pointer addresses never identity. Hashing uses owned data only; not a native access grant.

Before-source negative controls reproduce missing goods and equal native/model cache identities; current targeted tests PASS. Full production build precedes entry39 PASS/zeroFAIL/exit0. Probe compiles with raw source/destination/good/quantityFixed/scale sample output; new branch NOT_RUN live. Embedded/GUI archive+runtime/selftest560437/default-hidden vtx0 PASS. Candidate payloadf1ab7c9b452a0b139b3c376b15c8815ac9ca0c763bc469777ad947aff8f74212/38023311B; bootstrap71dae36d…/779879B. NOT installed; product51ec975f…/35151385B. No game process/input/launch/pause/exit. Formal native worker/UI guard still rejects pending endpoint adapter; no original warm-cache/visual/performance acceptance implied.

Evidence checkpoint2026-10-25-r109-native-trade-identity/{direction-proof.md,direction-assembly.txt,static,regressions.txt,logs,source-after.json}. Next original port/hub/type3 connections and group representatives→directed exports association, then same-source formal view/statistics labels. Do not call025E8450 whole geometry helper from GameState2/hidden0 capture;025E7F00 requires Interface1 OR hidden1. Full map identity/legacy economy+rail gates and lifecycle/performance remain pending.


## R108 current strait definitions, sea geography and selected-path geometry constraints (not formal/live)

Exact EXE7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f/image140000000 unchanged. New native_sea_geography.h copies already initialized TLS slot0 storage (GS58 array), init8600 bit0/context-token8648; only context uint32<16 slot0514D418+10*context/active+8. No0064A830 call (it may initialize/lock). CStraitDefinitionDatabase primaryVT04424140, ptr array80/cap88/count8C; CStraitDefinitionVT044F85A0/id10, sea uint16 endpoints44/46. Native runtime CStrait now separately retains its definition ID, not the full typed runtime handle. All definitions copied even when no runtime CStrait exists for an unbuilt canal. Values are bound to current context, never first populated slot.

CMap global058FCCF0/VT0441C938, province pointers2A0/cap2A8/count2AC, direct referenced indices1..FFFE only. CProvinceTemplateVT044DC488/region-pointerC8, adjacency header50/cap58/count5C/stride28 peeru32 at0. CStateRegionTemplateVT044E7850/definitionID10/geoID40/valid44. Copy the full bounded pointer roster but dereference only path/definition province union; current runtime states resolve full handles then use stateE78. Region pointers remain adapter-local cache keys, never worker input. Outer snapshot read window rechecks these observations with native trade, geometry, source/date/current.

Original025E8280 full independent disassembly: uint32(geo*100), naval bit23, exact raw packing with0x10000000800000 then<<6|type2, two native probes retained; no invented nearest-geometry binding. Independent authored splnet calibrations: geo3021↔3023 key0424E6E021270502;3006↔3024 key0424EA0021258E02;3024↔3026 key0424F04021275002. Type3 shore/port selection is still a separate pending adapter, not inferred from type2 membership.

SelectNativeSeaConnections requires original destination.selected index (no first-candidate default), current owned geometry and geography. It maps native selected ProvinceSet adjacency to existing nonempty type2 native keys. If a sea pair is governed by any actual strait/canal definition, only that selected path's current full-handle strait records with matching definition/sea pair permit it. Thus unbuilt/unused canal edges cannot reappear via induced adjacency even when both seas and the authored spline are present. Output is a deduplicated geometry membership, NOT an ordered route or new pathfinder; rejected adjacency count can count both directions. Current existence without selected passage is insufficient. No GUI/highlight/engine permission effect, no model solver.

Tests include a valid original detour with an unbuilt canal definition and actual shortcut geometry; stripping rules reproduces blind-adjacency shortcut, new gate excludes it. Opening plus a new second native candidate maps direct only, existing canal with original detour still excludes it, closed/recycled full handle fails. Current-context decoy, IDs/counts/types/arrays/unreadable/mutation/retirement/owned lifetime and same province+definition with changed native geo anchors/geometry covered. Build then unified39 PASS/noFAIL/exit0; final map-anchor test separately PASS. Expanded read-only live_shipping_probe --world-trade <count> --sea-geography observes unique window thread's existing context via query-only NtQueryInformationThread/TEB, no getter/selection/input; selected single alternatives can be mapped after process handle closes using owned copies. New branch NOT_RUN live; game absent. These are offline/static proofs, not game canal acceptance.

Candidate payload40e15298cf4929733bb5bc0f13cddcae87eb581d32210424ec9a3439c4dd224c /37209345B, NOT deployed. Installed51ec975f…/35151385B; bootstrap71dae36d… unchanged. Embedded/GUI archive/selftest560446/default-hidden vtx0 PASS. Formal worker/UI does not yet call the new connection selector; legacy no-model-fallback gate remains. Native port/endpoints, goods provenance/direction/full catalog/units, cache content identity and live geometry writer/lifecycle/performance gates remain pending. Evidence checkpoint2026-10-25-r108-native-straits/{definition-key-assembly.txt,static,local-original,logs,source-after.json}; original local game definitions private only, not bundled/distributed.


## R107 current native spline geometry copy (candidate; not live exercised)

Same exact EXE SHA2567f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f, image140000000. RTTI confirms global05901FA0 points to CGraphicalMap VT0464E4E8; +2C0 is current non-polymorphic spline database. Global058FCCF0 is CMap VT0441C938; map+240 is province pixel map, width50/height54 used by01581D20 to interpret original coordinates. Do not freeze8192x3616 for other maps.

Native database: point map header+0, entries stride1C, occupancy byte0/key4 masked1FFFFFFF/value8 two floats X/Z.015800F0 is a leaf missing Ghidra function metadata: complete independent disassembly015800F0..015801B2 returns row+8 or null. Segment map header28, entries stride30, occupancy4/key8 masked00FFFFFFFFFFFFFF; value10 CPdxArray<uint32 point IDs> (data10/cap18/count1C/allocator20), own ID28. Connection map header50, entries stride38, occupancy4/full64 key8; value10 CPdxArray<uint64 styled segment IDs>, own key28. Header each has table8/active10/mask14/overflow18. Original lookup RVA012E8440 uses full64 connection keys;02C60860 uses seven-byte hash/masked56 segment identities. Do not treat the connection's segment-ID list as points, or truncate segment handles to32 bits.

02CE29A0 enumerates connection segments in native order,02C60860 resolves control IDs,02CE0020 checks the prior last point and reverses when needed, avoiding duplicate joins. Control-ID flags mask29; segment style byte masks56. Same-key control changes must replace geometry. New native_splines.h owns native points/segments/connections and actual dimensions, rereads the entire observed source/current; BuildNativeSplineGraph converts only base types0..3 into existing SplineGraph geometry, dense local edge IDs mapped separately from native56 IDs. Shared segments deduplicate physically; known empty native connections remain known empty. No pathfinder/allocator/GUI/native editor helper called; existence of a key or geometry is NOT a canal-access decision.

CopyNativeSnapshot reads geometry with trade inside its existing verified command scope and outer combined window, attaches const owned geometry to nativeTrade, then checks source/date/current. UI/worker receive no native pointer. live_shipping_probe --world-trade includes the same decoder and graph conversion, remains read-only and does not invoke native selector externally. Build/entry38 PASS/noFAIL/exit0; final added four-type/duplicate tests separately PASS, probe compile PASS. Embedded/GUI archive/selftest560417/default-hidden vtx0 PASS. Candidate dist payloadd7ba1188990804b93a7f5a2994f65c9943f1d1c98be2d444c72c0f2cb245ad0b /36523135B, NOT deployed; installed51ec975f… unchanged. No game process; new geometry decoder/capture not live verified, no closed-canal or map-Mod acceptance. Formal native view adapter remains disabled/no model fallback.

Important name correction/next source: VT044F85A0 is CStraitDefinition, not CStraitTemplate. DefinitionDatabase primaryVT04424140/secondary+38 VT04424128, constructor006E5AA0 allocF60, pointer array80/cap88/count8C; definition size60/id10/key18/land40,42/sea44,46. Getter0064A830 reads current initialized TLS8600 bit0/context8648, slot0514D418+10*index, active+8. Getter can initialize TLS/lock and MUST NOT be invoked for a copy; no definition-DB copy or new call enabled yet. This is the actual generic source for all canal definitions, including currently absent runtime straits, replacing the earlier unfound template-name hypothesis. Current selected-path/strait passage constraints and province/region geoID mapping must govern final line selection, never raw induced ProvinceSet adjacency alone.

Evidence checkpoint2026-10-25-r107-native-splines/{native-layout-assembly.txt,static,logs,source-after.json}. Map-content/cache identity and full lifecycle/performance remain separate unpassed gates.


## R106 original selector binding and completed-command capture (candidate, not installed/live)

Exact EXE SHA2567f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f / image140000000. Independent full assembly confirms013BDBE0: RCX live CPdxArray pointer, RAX retained 0x38-byte path candidate; count1 direct pointer, count>1 strict signed lower013BD930 cost, ties first. 013BD930 writes only caller stack output; calls013BD840, which accumulates both current CStrait tariff modes through00CF6B70. Valid typed lookup reads global059667F8/table20/slots2C, fullhandle object8. The unavailable-database branch can log/assert, so decoder validation and native ownership are prerequisites, never optional. New profile signatures: selector4055564883EC2883, cost48895C241048896C, straitcost48895C2410488974, lookup4883EC58488B057D.

Isolated tools/test_native_trade_selector.py verifies full installed EXE hash before executing only copied native functions. Virtual reserve preserves RIP-relative offsets; only function pages RX and synthetic data pages RW are committed. It does not load the executable image, initialize engine services or launch/attach/pause/input a game. 21 exact-native-code cases PASS: direct/smallest/tie order/fullhandle tariffs and same-strait tariff-change winner reversal; borrowed arrays/database/straits/constants unchanged. This is native machine-code harness evidence, not live-game scope/lifetime proof.

Game::SelectTradePath is bound only after Game profile readiness. Overlay passes it to CopyNativeSnapshot inside original completed-command destructor hook. Existing caller00CD2BC1/phase4/access2/hidden0/owns1/context1 gate precedes all selection; array/full strait validation precedes callback, returned pointer becomes owned index; combined metadata/economy/rail/trade window and source/date/current are rechecked. EconomySnapshot now owns const NativeWorldTradeSnapshot; no engine pointer goes to workers. Missing selector rejects capture. FreightEngine explicitly refuses native snapshot until its geometry/view adapter exists, preventing fallback to legacy model results. No native highlight call or interface effect was enabled.

Unified tests-round106.log exit0,37 PASS/noFAIL. Final additional access-gate cases separately rebuilt/run PASS; world-trade mutation/unavailable-strait and worker-no-model-fallback cases PASS. Full build/embedded/native GUI/runtime-dir/selftest560421/default-hidden vtx0 PASS. Candidate payload7116233d5758238c81b69c46b917b732b9e16fab4f4dbaeb214735f1f8256c89 /36032042B in dist, NOT deployed. Installed remains51ec975f…/35151385B; bootstrap71dae36d… unchanged. No victoria3 process at recheck. New capture branch and native selector not live exercised; unopened canals and formal native statistics remain unaccepted.

Critical next-step constraint: geometry builder025E8450 reaches port scorer025E7F00, which asserts access1 OR hidden1. R101 command capture has access2/hidden0. Do not call the full native geometry helper in that scope. Current selector/cost/valid-handle chain has no such Interface precondition. Copy required raw current geometry/template data or establish a separately verified proper native scope; do not relax engine access checks. Actual source/peer direction still unlabelled pending verification. Evidence/backup checkpoint2026-10-25-r106-native-selector/{selector-assembly.txt,strait-lookup.c}, native/build/*round106.log.


## R105 owned world-market path/state-record decoder (not yet enabled in product)

native_world_trade.h reads exact-build world+898 groups (stride48), group member array+8 and route map+20, path candidates stride38 with ProvinceSet0/straitIDs18/distance30; typed current CStrait DB059667F8 VT04439720, objectsVT04490548/ID8/definitionA0 VT044F85A0, sea provinces definition44/46, tariff enum objectAC/AD. Detailed state map+890 (entries+898,mask8A4,overflow8A8), stride60/goods counter atrow10 VT044283B0. Values preserve native integer fixed units, with no planned-total/treaty substitution. State/peer direction remains unlabelled until full provenance verified. R104b raw live data is earlier evidence, not a live run of this new decoder.

Original013BDBE0 receives the native path-array address and returns a retained candidate pointer; one candidate returns directly, multiple calls013BD930 and uses strict lower cost/tie-first. Optional adapter-only callback now range/alignment-checks that pointer and stores only an owned candidate index; actual engine callback is NOT bound/enabled in product. Missing selector leaves multi-candidate selected empty. Synthetic callback tests prove interface validation only, not ABI/lifetime in-game. Native access ownership remains an external precondition; the helper does not acquire a lock or grant a lease.

New suite wired to build_tests.ps1: initial unified36 PASS/noFAIL/exit0; final callback/single-candidate/retirement tests and live probe compile passed separately. Cases include high-slot/full-generation state/strait IDs, zero state IDs, group/member uniqueness/anchors, hash bounds/entry counts, unknown destinations/straits/types, bad ranges/distances/goods masks/negative populated values, component mutation, publication retirement and source destruction. live_shipping_probe accepts optional --world-trade <verified goods count>, observes complete typed state roster and performs combined rereads; never calls native selector externally. No game was running at R105 recheck, so this branch remains NOT_RUN live. Product51ec975f unchanged.

Evidence/backup E:/Code/v3-analysis/checkpoints/2026-10-25-r105-native-world-trade; prior concrete structure/raw samples remain in2026-10-24-r104-native-shipping/shipping-discovery.md. Next enable verified native selection within R101 scope and consume current native facts/geometry in formal rendering/statistics; preserve unopened-canal and map-content identity gates.

## R104 native shipping copy (read-only live, formal integration pending)

Exact EXE7f53e326…/image140000000 rehashed. New native_shipping.h uses CShippingLane DB global05966760/VT04439410, entries+20/slots+2C/active+3C; stride10hex pointer+8. Full typed handle at object+8, primaryVT04427D98/secondary+10 VT04427D60, size68hex; country18/kind1C/from48/to4C. ProvinceSet+30 stores uint16 first/delta pairs, inclusive membership, not path order. Kind0 port connection andkind1 goods transfer are name/getter-verified (01D5290→01CD3440→01CD2E20;01D53E0→01CD3460→01CD2E40). +58 is GetMerchantMarineDemandRaw (01D4CF0→01CD33C0→01CD29C0), fixed100000 scale; +60 unlabelled. Kind1 typedref+20 tag65/full treaty28 goes via00C5A180 to CTreaty. Article inputs quantity variant1/goods variant2 are declared weekly quantities; original suspension/effectiveness remains relevant, no actual-delivery claim.

native_shipping_tests added to unified entry;35 PASS/noFAIL/exit0, final targeted rerun PASS. Diagnostic live_shipping_probe reuses same header and only PROCESS_VM_READ/QUERY: user manually opened4240/start2026-10-25 00:06:02/module7ff6b6960000/phase4;97 copied lanes=93 ports+4 goods transfer, full observed bytes reread stable. External sequential observation is not an atomic game scope. Static original renderer025E90F0(case26)→025E9F30→025E8450 selects lane ProvinceSet adjacency→025E8280 type2 native spline keys, port connectors025E8070 type3; actual native province templates/read-only samples validated separately. No native helper/highlight call, input, pause, write or exit. Product51ec975f unchanged; formal model replacement/canal verification pending.

Private evidence and detailed continuation: E:/Code/v3-analysis/checkpoints/2026-10-24-r104-native-shipping/shipping-discovery.md; static/,shipping-assembly.txt,shipping-live.txt,treaty27-live.json,province-live.json. This unit does not prove native actual trade allocation, current maritime geometry or map-Mod compatibility; next copy actual native geometry and trade records then replace official statistics without model fallback.

## R103 owned VFS configuration and directory policy (not a resource resolver)

Exact EXE7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f/image140000000 unchanged.03B4A8D0 normalizes backslash to slash in replacement root/path then03B4AA53 calls028B2150 on filesystem+48. Native array pointer+0,capacity+8,count+C; replacement entries stride40 hex contain root string0/path20. Exclusion array filesystem+60 has capacity68/count6C,stride20. Both strings have inline16 bytes,length10,capacity18,heap pointer0 when capacity>15. Existing mounts decoder extended to DecodeNativeVfsConfiguration, copying rules with mounts and actual search-path bytes and rereading all observations. It is still diagnostic-only/not included by product runtime; installed payload51ec975f unchanged.

Enumeration03B4AF80 exclusion stage uses length check and041DA8E0 byte-prefix comparison. No component-boundary or case-folding step there. Replacement directory match is exact byte equality:03B4B095 calls03B5BCA0 for NUL strings,03B4B0B9 compares equal-length views.03B5BCA0 SIMD/scalar paths return equality without case conversion;041DA8E0 compares bytes, not locale strings. For matching rules, root lookup uses actual filesystem+28 searchPaths byte equality; minimum index wins, owner included. Missing owner leaves INT_MAX (no cutoff). Child recursion still traverses parent enumeration; directory equality must not be promoted to a whole-path prefix rule. Pure NativeVfsDirectoryRootCount implements this step only; actual disk/ZIP enumeration, relative path/mount translation, map parsing and content graph identity remain unimplemented.

Unit tests cover both arrays/inline and heap strings,UTF-8/order,empty tables and empty exclusion,counts/capacity/pointer and string overflows,bad reads/mismatched terminators,all relevant copy-mutation stages,retirement,owned lifetime and rule priority/case/exact-vs-prefix behavior. Existing native/build_tests.ps1 already runs native_vfs_tests; targeted compilation/test PASS. New probe compiled and read user's PID57776/start23:07:08/windowThread36096/module7ff6b6960000/context0 twice:24 mounts,0 replacements,1 exclusion gui/gui_editor. Map directory limits24,excluded gui directory0. No attach/pause/input/write/getter/Mod changes. Actual modified-map replacement behavior is not live-proved.

Evidence/backup checkpoint2026-10-24-r103-vfs-rules/{replace-registration.c,replace-pair-append.c,enumeration.c,rules-assembly.txt,rules-live.txt,rules-live-repeat.txt}; native/build/vfs-round103.log. Next implement physical source resolution and recursive enumeration matching the engine before deriving a runtime content-identified graph.

## R101 completed-command owned snapshot handoff (production, partial lifecycle evidence)

Same exact EXE SHA2567f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f. Scope destructor033BD750 signature40 53 48 83 EC 20 48 8B, native command-runner call00CD2BBC/return00CD2BC1. Hook forwards the native destructor exactly once; only this return site with phase4/access2/hidden0/owns1/context1 can copy before forwarding, on the thread already holding that native scope. No new engine getter/scope/queue invocation, no simulation-lock acquisition from render/UI. Full catalog/state orders/rail copies use that thread's GS58 TLS and the existing bounded production decoders plus combined read-window checks. UI only drains owned data and validates source/epoch/latest data revision/full-handle anchor; model workers receive no engine pointers.

Resident requests remain after UI Take. Each actual copy has a unique monotonic data revision; newer data/error may replace ready data, stale success/failure cannot overwrite newer state. Paused completed commands always copy (the last command may precede an arbitrarily long idle); running copies remain limited to once per30s. Native BindSource advances the shared epoch before a changed machine/world/stateDB copy. Ensure replaces only older-world requests atomically, preserving a native new-world result when UI notices later. UI adopts the epoch/reset owners without retiring that result; queued GUI writes and native submissions require the current epoch. Numeric identity replacement is not a reliable retirement event or an ABA proof.

Failures retained:519864 initial20,267,078.94 stale after later initialization commands; paused speed change copied the correct20,267,075.76.269206 resident request fixed that, but reload copied the replacement source as old world1 before UI noticed/retires, discarding all ready new-source data and starving while paused. Dedicated resident test failed before fix; resident/source-bind tests and the entire unified entry pass after fix. Final payloadeed941b09eb4e2b6018d524e711d768dae0c584d567d15d7f67c00f51b94f2b2 (35151385B), bootstrap71dae36d…; dist equals installed.

PID52300/start2026-10-24T22:24:53.236934+08:00 native61820/UI35204. Paused initial rev2/3/4 copies125/110/110ms; UI accepted4. Direct same-save reload bound epoch2 at352203796; copies12/13/14 took125/125/110ms, UI adopted2 at352204187 and accepted14 at352204218, BEFORE F4/tab/speed/unpause actions. Market20,267,075.76,target30892.46,allocated14817.35,unmet16075.11;195 ribbons recovered, then state433 local Freight column/115 ribbons visible. Three hover/leave pairs:zero new native copies/zero null publications, nine queued totals identical. Fourth pair refused by foreground guard, excluded. User then requested manual control:all automation ended, no unpause/WM_CLOSE scheduled, PID remains for user. Current build running-update proof, reliable event/ABA, other-save/device/map scenarios and full performance remain pending.

Evidence checkpoint2026-10-24-r101-snapshot-handoff:eed941-handoff.log,eed941-world2-recovered.png,eed941-world2-state-prices.png,eed941-hover-summary.json,eed941-handoff-files.json; failed candidates retained. At handoff autosave hash/size/mtime and ini bytes equal pre-test. Full entry39 PASS lines/zero FAIL, selftest560468 bright pixels/default ImGui vertices0.

## R100 task joins and observed partial tick contract (diagnostic only)

Exact EXE remains7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f, image base140000000. Job wrapper VT0441A6A8:+8=03B292C0 tries queued execution, +10=03B29270 executes if needed then waits for job+60 state2; state0 queued/1 running/2 complete. 007C2540/00E78020 iterate handles first through+8 in reverse order, then all through+10. 03B29440 repeats the pattern before releasing handles. Do not label+8 alone as join.

CTickTask_UpdateStateTrades RTTI string0549FAF0/type descriptor0549FAE0/COL049D6998/VT044E6F38. Factory01343ED0 installs that table; its+38 leaf01339950 takes machine from batch+8, world+120, passes world+1C00 to0133C0C0. Nested013460B0/01346260 schedule through01348C30/01348DE0 then explicitly try/wait all handles before returning. Counter0103F6C0 has38 callers across unrelated container owners: this does not enumerate all state-order writes.

Opt-in probe_access adds exact0139E220 entry signature40 55 53 56 57 41 54 41 (full file hash also checked). Actual secondary Execute accepts the native command this pointer and returns void; caller00CD2810 passes that pointer and does not consume a result. Hook forwards it once and only logs16 pre/post scalar observations; it does not scan databases, copy orders, queue work, alter GUI or retain command/world pointers. native_access.h validates readable fields/type/mode/overflow; it grants no access lease. Default false, original user ini restored after testing.

Payloadd590417c… (34751412B) passed the unified entry and standalone checks. In PID56932/module7ff6b6960000/window thread38656, all16 observations were simulation thread45624, access2/hidden0/owns1/context1 before and after, date advanced6 each time, machine/world/state DB identities equal. This proves that callback's observed thread/contract for this bounded run, not full snapshot atomicity or every writer. Actual collector remains on the R98 path.

Next initial-paused/ordinary-command candidate:00CD2810 stage12 calls007E1B60 at00CD2B3D (return00CD2B42), then restores inner bookkeeping, then destroys its native access scope at00CD2BBC (return00CD2BC1). No new hook enabled there. Establish full input completion/initial load and an owned-copy handoff before switching production collection. See checkpoint2026-10-24-r100-simulation-completion/{join-static.txt,join-assembly.txt,trade-task-static.txt,command-assembly.txt,live-report.json,d590-session-only.log}.

## R99 game access scopes (static + external read-only observations; no new hook)

Same EXE SHA2567f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f, image base0x140000000. GameStateAccess enum at module+0514E580, source+588; HiddenGameStateAccess at0514E570/source+578. Observed scope values0 disabled,1 Interface,2 GameState,3 released/yielded. Interface assertion0443B0C0 allows Interface1 OR HiddenAllowed1. TLS array=GS:58, slot0:+5AE0 thread owns scope,+92C context. Asset filesystem TLS+8648 is unrelated.

Acquire033BD460 takes SRW0513E058 then SRW0513DCA0 then native mutex root+590, sets TLS+5AE0, releases only the outer0513E058. Ctor033BD640(storage*,access,context,source) optionally acquires, swaps global access/source and TLS context/native flag058FD79D, calls root virtual+F0. Dtor033BD750 calls+F8, restores swapped values and, if acquired here, unlocks root+590/SRW0513DCA0 and clears thread flag. Ghidra's typed qword root+0xB2 in the dtor means byte+590; independent assembly checked. Exact root primary VT04430800 has both+F0/+F8=0061C140 no-op in this build. Do not infer a separate world pointer swap from these virtuals.

Yield033BD820/8F0 temporarily releases/restores the scope. Crucially00E765E0 does the same around parallel batch execution;007C2C00 temporarily allows HiddenGameStateAccess for batch commands. Taking one native lock or reading on the window thread is not proof that every input writer has joined. No native scope invocation or new product hook has been enabled on this evidence.

CPartialDailyTickCommand factory007D7940 explicitly installs primary VT044365F0 and secondary04436590 at object+18. Actual execution0139E220 enters007E0D80 then007C8BD0 (command batches); adjacent VT entries/FC3020 belong to other commands and must not be used as its update function. Regular UI logic007D53E0→007D64B0 enqueues this command;007D5740 is startup. Graphics020808C0 is Draw with Interface scope, unsuitable for database copies. Next: prove a non-render completion phase, all relevant writes joined, exact thread/ABI and initial paused/load path before implementation.

External PROCESS_VM_READ-only PID49800/window thread46604/module7ff6b6960000: paused300 samples had access1/own1=22,access3/own0=278. Unpaused2200 samples had access1=199,2=29,3=1972,hidden1=30; context0, machine/world/DB unchanged. Reads are sequential, not atomic; mixed combinations cannot prove mutual exclusion. Bounded test normal WM_CLOSE, save hash/mtime unchanged. Product remains59e6d3d4…; no formal UI scope was open in this test. Evidence STATUS/WORKLOG R99 and private checkpoint2026-10-24-r99-access-window/access-{static,assembly}.txt, access-{paused,unpaused}.txt.

## R98 bounded state economy copy (exact 1.13.11; not an atomic epoch proof)

EXE SHA2567f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f, image base0x140000000. Native lookup007D40C0 resolves module+05964EF0, database+20 table/+2C slot count, stride10 hex and object at slot+8; object+8 is the full32-bit typed handle, low24 bits select the slot. Native state primary vtable044DE308. Production now copies the complete bounded table, validates full handles/types/duplicates, and invokes no native lookup. A test at slot2049 with handleAB000801 fails under the former2048 cap.

State production+C8/consumption+78 use counters with data+8, capacity+10, length+14, masks+20. Getter0103F440 reads only set-mask indices; allocator0103E450 sizes the array to current goods count (catalog+8C). Writer0103F6C0 updates the value and mask separately. This is precisely why rereads alone cannot establish an atomic economy epoch. Numeric values are fixed point1e5; dividing by100 produces milli-orders1e3. Negative/unreadable/out-of-catalog inputs are rejected, never changed to0.

native_economy.h returns owned state identities and orders. EconomyReadWindow rereads the combined metadata/order/rail observation set before acceptance; observed changes reject the candidate without falsely changing world generation. UI lifecycle identity checks resolve the full handle through safe copies rather than calling the native resolver or dereferencing an old pointer. A reliable world event/ABA-safe lifetime and atomic game-access window remain unverified.

Payload59e6d3d4… passed the unified entry and autonomous paused/one bounded unpaused update (1836-01-01→01-07). States1147, orders25396→25391, actual market0 card20,267,075.76→19,815,791.98 and195 visible ribbons. DLL total copy/recheck93–110ms; not P95/simulation performance acceptance. Evidence: STATUS/WORKLOG R98, private checkpoint2026-10-24-r98-economy-copy. No debugger attach, suspension or engine economic writes.

## R95 frozen filesystem capture (diagnostic only)

Exact EXE SHA2567f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f, static image base0x140000000. Getter RVA03B4D230 returns module+05144588+TLSindex*0128; initialized byte+0120. The window thread's TEB+58 points to TLS array, slot0 has initialized bit0 at+8600 and context index+8648. External read-only NtQueryInformationThread/ReadProcessMemory confirmed thread32864/context0 in PID48816, module0x7ff6b6960000. No native getter was called.

Filesystem+B8 points to rawVFS, frozen byte+FA, linked mounts+8. Mount item has data0/rootchar*8/mountPoint10/sourcePrefix18/prefixBytes20/backend28/next30. SearchPaths fs+28 is a null-terminated char** list. Mount03B2F4D0 prepends; RefreshRoots03B46FF0 copies this order. Directory backend enumerate/open/stat RVAs03B33D80/03B33FE0/03B34220; ZIP03B40A50/03B40CC0/03B410E0. ZIP backend table itself was on the heap, so validate function identity rather than table RVA. native_vfs.h copies owned strings, bounds/duplicate/bad-read/change/current-request checks, and reconciles search paths before publication. live_vfs_probe twice passed24 mounts (22directories/2ZIP); new fixture tests integrated in build_tests.ps1, full entry PASS. Payload remains8115373d…, installed=dist.

**Not a resource resolver:** Mod038A0620 passes descriptor+C0/CC replace paths to03B4A8D0, which appends normalized root/path string pairs to fs+48 (stride40/count54). Enumeration03B4AF80 consults that table and fs+60 exclusion paths (stride20/count6C), restricting matching directories to the owning root's search priority or higher. The decoder does not yet copy these tables or resolve current map content; runtime use/mapMod compatibility are not enabled by this unit. Precise nested/boundary semantics still require verification. Private evidence and original decompilations: checkpoints/2026-10-24-r95-active-vfs. Game was paused1836-01-01 and normally closed without saving. See STATUS R95 for the corrected Orleans rail-only connectivity diagnosis.

## R93c checked native-pool recovery

The world gate now compares nativePoolWorld with economyWorld. Once a current real frame is ready, the verified arrow pass checks the current asset style/type binding, constructor callback membership, retained arrow/path/point/VB/CB identity, original graphics/device identity and stable copies. It clears old active requests and marks only that exact world ready. There is no reconstruction/destruction or simulation lock. Device/graphics changes also retire cache hits; a swapchain replacement compares devices before releasing the old reference, so a recycled COM address cannot authorize the old pool.

Prototype39c recovered180 arrows after menu reload(world2) and in-game Load(world3), with uploads180 unchanged. Final8115373d (33,718,124 B, installed=dist) also passed menu reload with180 retained arrows, UI status restoration, minimize/restore and actual2560x1440→1920x1080→2560x1440 client resizing. Error.log GUI hits0; production lease decoding validated92 types/180 objects. Full regression and D3D11/default-hidden checks pass. Evidence: STATUS/WORKLOG R93c and private checkpoint2026-10-24-r93-world-recovery.

This establishes same-device/same-asset recovery for these save tests. A24 remains partial: another save, reliable world events under anchor-address reuse, new-device native-buffer rebuilding, asset replacement and active-map compatibility are still pending. Earlier permanent-restart gates below are historical, not the current implementation.


## R92 lifecycle evidence and R90/R91 UI acceptance

Real State.GetMarket identity (305 markets) replaces the invalid state+48 grouping. Native market/state figures, Chinese search/categories/empty results/hover/show-hide, native details/context menu and state switching have live evidence. R91 localized ledger values are fully readable; 20 verified close/reopen cycles reused native geometry (uploads371 unchanged), without sustained private-byte growth. This is partial acceptance, not the complete performance protocol.

R92 read-only snapshots across save/menu/reload are byte-identical for92 ironroad types/371 registered arrows, including path/VB/CB pointers. The type's stride0x48 vector holds geometry-rebuild callbacks, not proven world ownership. Constructor registration and type/arrow destructors were inspected; direct teardown remains disabled. The permanent world-retirement gate still blocks arrows after reload, while current-world numeric data resumes. R92 adds a separate localized unavailable-map message. No recovery implementation or device/asset replacement acceptance is claimed. Exact evidence and hashes: STATUS/WORKLOG R92 and private checkpoint2026-10-24-r92-native-lifetime. Historical statements below are superseded where they conflict.


## R90 market identity correction

State.GetMarket follows the country/explicit MarketArea branch and resolves the native market object, whose full ID also keys the GUI wrapper. state+0x48 is not that chain. The production read-only decoder matched all 1147 live states (3 area branches): state433 maps to country1/market0. There are305 native markets versus52 old field48 clusters; old market-scope allocation claims are invalidated. state_market.h validates typed handles, RTTI-derived vtables and reread stability. Static assembly/whole-roster evidence: private checkpoints/2026-10-24-r90-native-markets; EXE SHA-2567f53e326… unchanged. No native getter/resolver is invoked by the decoder.


## R82 actual state/region identity (important correction)

Live state433 (user's Midlands screenshot) belongs to market111 and points at
state+0xE78 to a region definition, vtable RVA0x44E7850, engine definition ID17
at+0x10, key STATE_MIDLANDS MSVC string+0x18. Authored map ID is20. Registration
0xE83A0 -> State.GetStateRegion callback0x1A06B80 -> definition0x15715A0;
constructor0x156D1D0 verifies the definition ID/name layout. Critical instructions
independently checked with Capstone. +0xE48/0x7BB480 was an early discarded
candidate, not this region chain. Runtime state IDs must never be used as map IDs.
Actual new key decoder maps1147/1147 runtime states to675 authored geography keys.
Only read-only process access, no attach/breakpoint/UI/engine calls. New adapter
is built but not deployed; prior live geographic allocation conclusions need
revalidation. Full actual-map/Mod resource and geometry revision support remains
pending. Private evidence: state-region-r82-notes.md / assembly.txt / live.bin.


## R79 state context candidate (offline only)

The exact EXE constructor0x20A8900 places states_panel at bar slot14;
name-to-kind0x20ABFC0 independently returns14. Constructor0x22539D0 sets
vtable0x45CC848 and ID+0x70. The State typed-ID setter0x22547B0 writes that
field, and method0x2254770 resolves it. Ghidra plus Capstone checked the
critical instructions; installed GUI uses local_goods for Local Prices.
state_panel.h safely copies the candidate context; state_panel_tests is wired
into build_tests.ps1 and all14 suites pass. No runtime caller or native getter
was added. Live state/snapshot ID correspondence, access window and open/close
transitions remain unverified; the Freight column is still pending. DLL remains
R78 unchanged. Private evidence: state-panel-r79-static.md / assembly.txt.


Local-only reversing notes. Every address is an **RVA** for the executable identified
below. Do not reuse these numbers for another build; the loader refuses to call anything
when the file hash or an entry-point signature differs.

- Executable: `binaries\victoria3.exe`
- SHA-256: `7f53e3260cf46b6247f6906837464944038c4f74e860274ad3d394e0c04f811f`
- Size: 96,872,568 bytes
- Branch/revision: `release/1.13.11` (`caligula_rev.txt`)
- Image base: `0x140000000` (all runtime use is `module base + RVA`)

## Derivation

R74 filter/cache research: GetFilters callback0x225B150 wraps CPdxArray of
GoodsCategory bytes at panel+0x110 (ptr+0,capacity+8,count+0xC).
IsSelected0x225A630 returns !contains(category); setter0x225A730 changes
exclusions and sets wrapper+0x320 dirty. Search getter0x225AFB0 returns
panel+0x80 CGuiSearchBar; normalized string+8 is panel+0x88. Callback0x225DB90
writes it to market wrapper+0x328 and marks dirty. Unicode lower loop0x3B1AD60
is part of vanilla normalization; native matching also uses localized keywords.
AccessMarketGoods registration0x1D9A60 ->0x1CDFAC0 ->native0x1CD8650,
cache helper0x1CD8690 and builder0x1CDAAF0/0x1CDA600. Return is cached rows
at wrapper+0x60; cache lock+0x54,dirty byte+0x320. Registry global0x59684A0
holds a sorted pointer vector (ptr+0,capacity+8,count+0xC,lock+0x18), exact
market wrapper vtable0x45850B0/ID+0x10. Cached row vtable0x4552218,
definition ptr+0x10,state+0x18 must FFFFFFFF,market+0x1C; definition uses
the R72 verified goods layout. Candidate observes writers only, never locks.
Live sample: two registry entries IDs0/FFFFFFFF, both dirty1/rowCount0;
actual decoder replay rejects as pending. Positive filtered rows are unobserved;
no new DLL execution or filter interaction is established. Evidence private
native-filter-r74-static.md/live.json/notes.md and project round74 logs.

R73 interface-state research: global RVA0x592D0E0 points to the current
CInGameInterfaceIdler (vtable0x4431500). Native resolver0x2080CD0 requires its
machine at+0x18 to have phase4 at machine+0x588, then RTTI-casts gfx at+0x10
(CIngameInterfaceIdlerGfx, vtable0x45B13D8). The candidate does no cast/call:
it validates the exact primary vtable and copies gfx+0x20 -> handler
(CIngameInterfaceHandler, vtable0x45AE328). Handler constructor0x2045AE0 embeds
InformationPanelBar at+0xD8, with self-pointer, vector+8, count+0x14 (56).
Bar constructor0x20A8900 places CMarketPanel at vector slot9, vtable0x45CC9C0,
name MSVC string+8, selected tab string+0x30, market ID+0x70.
IsPanelOpen registration0x30F500 -> callback0x2129E50 -> native0x21255E0,
tail helper0x2125610: bar history pointer+0x20, count+0x2C; open iff count>0,
bar byte+0x40==0 and last 0x30-byte history entry's kind==9. Selected-tab native
comparison0x214C120 reads panel+0x30 (setter0x214C770).
The complete closed chain was observed live in PID52316 and replayed through
market_panel.h. All original functions remain uncalled by the candidate.
New DLL execution/open transitions, same-frame close and full lifecycle remain
unverified; current candidate polls the existing UI timer at500ms. Raw evidence
is private market-panel-r73-static.md / live.json / notes.md.

R71 static / R72 live goods metadata: name registrations
`0x94460` / `0x94310` / `0x92830` identify IsLocal / IsTradeable / GetKey.
Callbacks `0x1978310` / `0x1978230` dereference wrapper+0x10, then definition+0x40:
local = bit 1, tradeable = bit 2 and not local. Direct key getter `0x1957E50`
copies definition+0x18 MSVC string (length +0x28, capacity +0x30). CGoods primary
vtable `0x4517F80`, definition size 0x70, ID +0x10, category byte +0x41. Database
primary vtable `0x4517DF0`, vector +0x80, count +0x8C (postload `0x174F8E0`,
max 128). Constructor `0x1753C10` assigns ID from database count.
Singleton getter `0x7B7290` uses TLS index and can initialize, lock and register
cleanup: do not call it as a harmless read. Its slot table starts `0x514E0D8`,
stride 16, initialized flag +8; no first-populated-slot selection is authorized.
The pure decoder in goods_catalog.h requires explicit exact-build identities and
a safe reader. R72 observed all53 native definitions and main-thread TLS context,
verified IDs/vtables plus key/category/local/tradeable by installed definition key,
and replayed the actual decoder/context resolver on copied bytes. The candidate
goods_snapshot.h/CollectRealEconomy caller observes GS:0x58 -> EXE TLS storage,
initialized bit+0x8600, token+0x8648 (low32 index<16), then only that initialized
context slot. Safe RPM and context/catalog rechecks run on the existing UI timer.
The new DLL adapter is NOT_RUN; stable rereads do not prove an atomic economy
snapshot or all object lifetimes. Evidence: private goods-catalog-r71-static.md
and goods-context-r72-notes.md.

The standalone DLL mounts its embedded archive immediately after a native
MountData call at RVA `0x3B47C40`, before VFS configuration freezes. The archive
places its type definitions under `gfx/models/military` so the normal `.asset`
parser consumes them. A diagnostic hook at `0x3B42800` verifies mounting happened
before the asynchronous `gfx/models` / `.asset` pre-enumeration cache is populated.
Its full arguments are three pointers to 16-byte string views and an int depth.
`0x3807B00` and `0x3800070` were rejected after runtime evidence showed these are
ThreadedInitTextureLookup, not model-definition parsing. ZIP mounting is performed by
the engine's context-local VFS entry `0x3B2F4D0` (ignored first argument, UTF-8
archive path in RDX, mount root in R8), followed by `0x3B46FF0`, as in the native
MountData caller. All four entry signatures join the executable hash gate.
These are static ABI findings; fresh startup registration logs provide runtime
evidence separately. No asset registration occurs in a render or simulation hook.

`tools/discover_local.py` (adapted from the supplied starter) performed name-anchored
recovery against this exact file. It walks RTTI complete object locators, `.pdata` unwind
extents, RIP-relative cross references and Capstone instruction patterns. It does **not**
execute the game. All twelve recipes returned `CANDIDATE_FOUND`; the private report is
`E:\Code\v3-analysis\arrow-candidates.json`.

The manual work on top of the discovery was reconstructing the path context and the
per-object data that the renderer consumes, from the two shipped arrow updaters and the
render pass itself.

## Entry points

| RVA | Symbol | Evidence |
|---|---|---|
| `0x037FD4D0` | `CArrowObject::CArrowObject(CPdxArrowType const*)` | unique referrer of the `CArrowObject` vtable `0x481D3E0`; the constructor-lambda cross-check agrees |
| `0x026D0A20` | supply arrow factory | `mov ecx, 0xC0; call operator new; call 0x37FD4D0; store supply vtable 0x46588E8; [obj+0xB0]=0` |
| `0x026D7150` | military arrow factory | same shape, installs `0x4658A08` |
| `0x037FD5B0` | `CArrowObject::SetPath(ArrowPath const*)` | unique shared callee of both updaters reaching the `SArrowVertex` buffer allocator |
| `0x037FCCD0` | ribbon edge generation | called by `SetPath` with `2*width*0.5`; consumes the centerline vector |
| `0x037FDC50` | vertex buffer build | called by `SetPath`; creates the GPU buffer and writes `+0xA8` |
| `0x026CEF30` | constant buffer creation | unique shared callee storing the field read by vtable slot 15 |
| `0x037E7D60` | `CAssetFactory::GetArrowType(std::string const*)` | called by the typed helper with the freshly built name in `rdx` |
| `0x037FE620` | bucket append | unique `CArrowObject` vtable slot reading `this+0x50` type and `type+0x30` bucket id |
| `0x037FD860` | arrow render pass | unique reader of the bucket container indexing it with `[rcx]`, `[rcx+8]`, `[rcx+0x10]` |
| `0x0512CDC0` | `CPdxNewDeleteAllocator` singleton | unique `.data` qword carrying vtable `0x4419230` |

Signature bytes checked by `Game::Initialize` (first 8 bytes at each RVA) are in
`native/src/offsets.h`.

## ABI

x64 Microsoft calling convention; `__fastcall` in the source is documentation only.

```c
void* __fastcall GetArrowType(void* unused_rcx, MsvcString* name);
void* __fastcall CreateSupplyArrow(CPdxArrowType* type);      // returns 0xC0 object
void  __fastcall SetPath(CArrowObject* self, ArrowPath* path);
void  __fastcall EnsureConstantBuffer(CArrowObject* self);
void  __fastcall BucketAppend(CArrowObject* self, PassContext* ctx);
void  __fastcall RenderPass(PassContext* ctx);
void* __fastcall Allocator_Allocate(void* alloc, size_t bytes, size_t align);  // vtbl[1]
```

`GetArrowType` ignores `rcx`: its first action is `call 0x38181E0`, a function-local-static
getter that never reads `rcx` and returns the asset factory. It reads `rdx` as an MSVC
`std::string`: data/SSO pointer at `+0x00`, size at `+0x10`, capacity at `+0x18`
(`cmp qword ptr [rdx+0x18], 0xF; mov ebx, [rdx+0x10]`). The loader builds that layout by
hand; names longer than 15 characters use a heap buffer.

`SetPath` first stores `path` at `self+0x70`, then calls the path interface:
`0x2674100(path, &temp)` copies `path->points` (a `CPdxVector<SVector3>`) into a temporary
with the game allocator. `.pdata`/instruction evidence for the layout:

```
02674100  test rcx,rcx ; je ret
02674108  lea rdx,[rcx+0x10]     ; source vector
0267410c  mov rcx,rax            ; destination temp vector
0267410f  jmp 0x25EAED0          ; CPdxVector::operator=
```

`0x25EAED0` uses the destination allocator (`+0x10`, vtable slot 1, align 4) and copies
`count*12` bytes; it never takes the source allocator. Element stride 12 (`SVector3`).

The render pass reads the retained path each frame:

```
02674120  test rcx,rcx ; je ret
02674125  eax=[rcx+4] ; *rdx=eax
0267412a  eax=[rcx]   ; *r8 =eax
0267412f  eax=[rcx+8] ; *r9 =eax
```

The pass builds constant buffer 0 as
`{ path[1], path[0], path[2], arrow+0xA8, type+0x68 }`, which is exactly the shader's
`{ AnimationLength, MoveLength, ClipLength, TotalLength, Width }`. The loader sets
`moveLength = 1e9` (fully drawn), `animationLength = clipLength = 0`.

## Object field map

Observed in the constructor (`0x37FD4D0`), both factories, both path updaters
(`0x26D0140`, `0x26D69B0`) and the render pass:

```
0x00 vtable
0x08 u32 0
0x0C (padding)
0x10 4x4 transform (identity; translation at 0x40/0x44/0x48/0x4C=1)
0x50 type*
0x58 u32 -1
0x5C u8  flags; ctor ORs 7; render pass requires bit 0 set
0x60 constant buffer (shared_ptr-like object created by 0x26CEF30)
0x68 vertex buffer (created by 0x37FDC50)
0x70 path*            <- retained and dereferenced every frame
0x78 CPdxVector<SVector3> centerline
0x90 CPdxVector<SVector3> ribbon edges (offset/subdivided)
0xA8 f32 total length (written by the vertex builder)
0xAC u32 0
0xB0 u64 supply/military constants uploaded as CB1
```

The supply vtable slot 15 (`0x26CF000`) uploads 8 bytes from `this+0xB0`:
`rcx=[rdx]` (device), `rdx=[this+0x60]` (CB), `r8=this+0xB0`, `r9d=8`, `jmp [rax+0xB8]`.
Zeroed `+0xB0` therefore means `IsSelected=0, IsFaded=0`, i.e. normal drawing.

## Pass context

The render pass receives one pointer with three dwords at `+0x00`, `+0x08`, `+0x10`. It
validates them against nested global containers, obtains a bucket list via `0x3828380`,
and iterates bucket entries of `0x18` bytes (`{data, capacity, count, allocator}` at
`+0`/`+8`/`+0xC`/`+0x10`) containing arrow pointers.

`BucketAppend(self, ctx)` reads the **same** triple from `ctx` (`[rdx]`, `[rdx+8]`,
`[rdx+0x10]`) and grows the bucket container to `ctx[2]` via `0x3820A10`, then appends
`self`. Submission therefore happens in the pass hook, using the pass's own `rcx` as the
append context, before the original pass runs. Submitting from `Present` is wrong: the
context is a temporary owned by the caller.

## Why the supply subclass is used

The base `CArrowObject` shares slot 15 with both subclasses but the slot target is a no-op
(`0x61C140`); the real constant-buffer uploader is `0x26CF000`, reached only through the
supply/military vtable. Constructing through `0x26D0A20` gives a correctly sized/aligned
`0xC0` object, installs the supply vtable, and zeroes `+0xB0`. The loader never writes the
game's shared vtable.

## Constants used

- Ribbon width argument: `SetPath` calls `0x37FCCD0(vec, 2*type[0x68]*0.5f, 0)`; `0.5f`
  lives at `0x486ACF0`.
- Type fields: `+0x30` i16 bucket id, `+0x68` f32 width, `+0x6C` i32 subdivisions,
  `+0x70` u8 line smoothing.
- Object size: `0xC0` (`mov ecx, 0xC0` in both factories).

## Verification boundary

The static layout above is derived from the shipped callers, not from symbols. It has not
been observed against a live renderer in this session. The first live run is expected to
confirm:

1. `GetArrowType("supplyRouteArrow")` returns a type with a sane width/subdivision/bucket.
2. `CreateSupplyArrow` returns a `0xC0` object whose vtable is `0x46588E8`.
3. `SetPath` builds a vertex buffer (`+0x68 != 0`) and total length (`+0xA8 > 0`).
4. `EnsureConstantBuffer` creates `+0x60`.
5. The pass hook appends once per frame and the ribbon is visible.

The `CAPTURE setpath obj=... type=... n=... first=(...) last=(...)` log lines from the
SetPath hook provide items 2–4 plus real world coordinates for calibration.
