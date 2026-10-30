# Freight allocation model (work in progress)

### 2026-10-28 — R134完成：正式颜色／八档线宽图例、双语居中及编辑GUI校验

业务色保持地区输入绿／输出橙，市场热度使用完整市场固定区段商品数量尺度；单商品模式按原生商品ID取既有稳定五色，未选或移出为空地图，原生外观统一已验证建筑绿。颜色只改视图，不改国际原版记录、国内最终模型分配、需求账、完整行／卡片／市场分母、路径或宽度。八档图例显示实际宽度比例2:3:4:6:8:12:16:24及数量阈值10／50／200／500／2000／5000／20000，提示比例示意、非像素／容量／距离工作量；0不画、未知--。

正式市场热度为区段商品数量q／完整市场最大区段量，固定20%边界；旧模型加权work不承担正式热度。宽度阈值以商品数量表示，内部1e5，国内milli-orders乘100与国际原生精度精确相容；并列UI标签区分来源。Goods高亮仅抽取所选贡献；Native只换构造时样式，方向／宽度保持，不改路或重跑分配。

当前9afd英文67696与中文63460各21项颜色／商品／方向／关页重开检查通过；市场218、单商品等待0／谷物hover123／移出0，州433全部248／输入107／输出141，原生绿与业务色恢复原段数。市场量25963.19216／510.99MU、州433量3548.63121全程保持。英文原版首行Artillery为66段，中文所选第二行加工食品为22段；这是原版本地化排序，不把两次选择当同商品对照。中文额外显示／隐藏实际截图与日志通过，数量不变。每个正常会话仅一次国内分配918陆／93海／9038笔，三处partialSea仍保留已知数量及可确认片段。

当前两语言数量保持及同facts回归通过；缺端点与缺数量继续分开，不将三partialSea缺几何改成未知运量。证据及R133回滚：E:/Code/v3-analysis/checkpoints/2026-10-28-r134-native-legend/{legend-audit.json,live,logs,before,NEXT.md}。

### 2026-10-28 — R133完成：地区商品／方向／运输方式地图筛选

正式地区筛选只提取现有最终货流，不重新分配：商品与相对当前州的输入／输出限制地图货流；运输方式按当前原生陆连接／港口lane来源限制所画区段，海路包括进出港部分。混合路线可同时贡献两种方式；每种视图都只计整笔货量一次，不改变路线、供需、目标／满足／未满足、完整商品列、scope／市场分母、热尺度或铁路长度。原生量保留1e5，国内最终milli-orders仍精确乘100，口径分开。

MapDirection／MapTransport进入视图缓存，不进入分配内容身份；旧非原生模型没有确证交通来源时拒绝这项新能力，不猜几何类型。缺几何保持已知量，某商品或筛选合法零为空地图而非未知。当前中文实机一次国内分配918陆／93海／9038笔，19项切换检查及源码混合路线回归通过；三处partial port末端仍未完整补画。证据E:/Code/v3-analysis/checkpoints/2026-10-28-r133-state-filters/state-filter-audit.json。

### R128：已铺铁路基础设施统计（2026-10-27）

正式原生视图的已铺铁路与货流账分开：从当前已建type4连接复制独立owned builtRails，含州内／跨市场；模型可用land仍只含原有同市场州间连接。按两端同属当前市场的连接，对完整56位物理strip取并集，再按当前作者二维控制点折线累计hypot(Δx,Δz)。单位MU是地图单位，不是已标定公里、三维曲线弧长或运输容量。跨市场记录不计入任一市场内部长度；反向／重叠物理段只计一次。统计与实际货量、商品筛选、搜索、hover及选择州无关。

已知空铁路集合为0.00 MU，缺原生国内来源为--；native公里值仍未知。当前连接及几何进入指纹和精确比较，变更／退休触发重算。实际原存档510.9929662664673 MU→原版拆除387.7629690309868 MU→重载恢复；独立原坐标Python并集核算及正式卡片通过。此字段没有改变国内物流模型量、国际原版量或其标签。证据见STATUS R128及rail-audit.json。

## R118 domestic connectivity and rendering separation (2026-10-26)

A current nonempty original kind0 port lane grants domestic model connectivity
even when its endpoint graphics key is missing. The solver uses its verified
`CShippingLane+60` travel distance, fixed1e5, rather than requiring a complete
drawn spline length. Zero is a valid original within-region distance; negative
values are rejected. This is the original travel-distance unit, not verified
kilometres. Land retains current spline distance, so combined model distance,
work and tariff cost are model metrics, not a physical-distance measurement.

Quantity, complete drawable quantity and partial geometry remain separate. A
single witnessed port can orient a unique chain of existing original strips;
the missing shore connection is never fabricated. Without either witnessed port,
the known model allocation still exists but no uncertain line is drawn. Empty
original paths grant no connection. Original international routes retain the
strict complete-endpoint requirement. Supply/demand, final quantity and Freight
target/fulfilled/unmet remain the model values described below. The R117 figures
below describe the previous build; current validation is recorded in STATUS.

R118 coreaff98195… is installed and live-verified with93 current sea links,
918 land links and9038 model shipments. The three partial links carry final
edge quantities0.586,418.679 and0 in the independent owned replay, matching
production's global model count. Their complete shore geometry remains absent;
their individual screen views have not been accepted. The British market still
shows23098.111 domestic model quantity separately from2865.08116 original trade.
Current GUI/filter/state,20 reopen cycles and66s stable upload checks pass;
this is not full map-mod, lifecycle or P95 acceptance.

## R117 current domestic model (2026-10-26, default-save baseline verified)

Latest user instruction restores **domestic model allocation** alongside original
international trade. Original records retain native integer 1e5 precision;
domestic allocations retain milli-orders and are converted exactly by ×100 for
the common goods-quantity display. These are separate fields, not one claimed
original statistic. The market/state UI explicitly labels the domestic estimate;
the ledger shows total / original / domestic model quantity, then domestic
Freight target / fulfilled / unmet. Earlier original-only or model-work labels
below are historical where inconsistent with this section.

The adapter copies current local production `state+C8` and consumption `state+78`,
typed country-referenced domestic `CShippingLane` kind0, and current usable
`CTravelConnection` land links. Runtime province ownership resolves land endpoints
to full current state handles; definition keys and geometry bind directly to the
current native spline tables. No saved ID→key lookup, embedded vanilla map,
nearest-coast bridge, or newly solved ocean route enters this network. Land
categories2/3 are ground transport; category4 alone uses the built-rail tariff.
Unbuilt authored rail geometry is never given the built-rail tariff. Port lane
geometry retains the original province membership, rejects unavailable canal
definitions and requires a unique original endpoint chain.

For each state/good, aggregate original incoming/outgoing records at native
precision, then divide each aggregate by100 once to milli-orders:

```
effective supply = local production + original incoming quantity
effective demand = local consumption + original outgoing quantity
surplus = max(effective supply - effective demand, 0)
target domestic import = max(effective demand - effective supply, 0)
```

Only the remaining surplus/target is allocated by the existing min-cost maximum
flow solver, independently per current market/good. This avoids allocating the
already recorded native transfers again. The model aggregates within a state;
it does not locate individual buildings or estimate intra-state transport costs.
An original land/port connection is a model edge, and final edge references map
back to its exact current native strips. Cross-market model edges are excluded.
Default distance tariffs remain explicit model assumptions: ground1, current
rail0.4, sea0.25. There are no guessed tonnages, capacities or monetary prices.
No game orders, prices, MAPI, buildings or populations are written.

Freight revision1 demand remains separate, with default conversion coefficient1
explicitly a model assumption. Targets are computed before routing; final imports
fulfill them; shortages and missing/disconnected paths leave unmet demand.
Goods filtering/hover only extract final shipment contributions. The card follows
search/category before hover; shares retain the whole-market original+domestic
quantity denominator. Original quantities whose geometry is missing remain known;
missing domestic paths cannot fabricate transported quantity or a straight line.

Source adapter and domestic allocation/geometry/cache/document regression tests
pass. An external read-only paused-save replay (after closing its process handle)
read25396 local orders,93 country port lanes,918 land links;90 port shapes were
available,3 lacked original endpoint geometry. It produced8974 modeled shipments,
including8272 using land and2083 using sea (a shipment can use both). Production now matches this replay: installed/dist core87b6d408… shows
original2865.08116 + domestic23098.111 =25963.19216,218sea/land ribbons and
state433 logistics values/248direction ribbons. Domestic Freight ledger reads
30585.31/23098.11/7487.20; localized filters/empty/hover/hide and20actual reopen
cycles pass.46native checks pass on the same code; final resource/GUI/GPU-loader
checks pass.145s stable state view has zero new uploads. Three original port
destination shapes still require work; actual modified maps, complete lifetime,
rail units and full performance/final acceptance remain. See STATUS R117.

The user confirmed that the ZIP contains no existing allocation model and asked
for one to be implemented. `native/src/freight.cpp` now provides the native model
and immutable view builder. It is not yet wired to a verified live economy
snapshot or the ImGui freight controls; those controls remain disabled.

## Freight intermediary demand (phase 1: observational ledger, end-to-end acceptance pending)

Phase-1 scope is confirmed: no independent Freight supply pool or engine good is required. Fulfillment is derived from final original-goods shipments; this ledger does not change the engine economy. Earlier supply-pool blockers are superseded.

The user requires logistics demand to be calculated through **Freight as an
intermediary good**. See `MASTER_PLAN.md` section 4.5, P4 and acceptance A27.
The existing shipment freight statistic alone does not implement this requirement.
R67 adds a model-owned intermediary demand ledger to each `FreightBalance`.
Source identities are `(state, market, good)`, with world/economy/graph revisions
on the owning result. Revision `kFreightDemandRevision = 1` defines:

```text
netImportOrders = max(consumption - production, 0) / 1000
Freight.target = netImportOrders * good.freightPerOrder       # before routing
Freight.fulfilled = finalAllocatedImport / 1000 * coefficient # after final flow decomposition
Freight.unmet = max(Freight.target - Freight.fulfilled, 0)
```

The coefficient is in abstract Freight units per original imported order. Its
default **1 is a Mod assumption**, not a game field or user-confirmed calibration.
It is a copied per-good model parameter, independent of `weight`, route length
and mode tariffs. Distance/mode affect reachability and allocation; they do not
convert the target units in revision 1. This preserves targets when no feasible
route or source supply exists. Local-only goods, explicitly flagged intermediary
goods and the canonical case-insensitive key `freight` generate no further
transport demand. Nontradeable goods are also excluded (R71); `tradeable` is
independent of `localOnly`, and both classifications participate in cache identity.
No synthetic Freight orders are appended to the original
snapshot; no original goods ID is borrowed for the model commodity.

Views and documents aggregate the ledger by whole market/state before any goods
filter. Documents also expose per-good contributions. Missing/legacy/cancelled
results remain unavailable, distinct from known zero. The native panel reads a
ninth long-key variable, `ironroad_demand_v1`; it suppresses synthetic input and
displays target/fulfilled/unmet separately from original order quantities and
order-distance work. Coefficients and intermediary flags participate in cache
fingerprints. Changing a coefficient leaves original allocated widths unchanged.

**Remaining:** this is demand accounting, not a standalone Freight supply solver.
Fulfillment currently follows feasible original-goods allocations, with no
separate Freight capacity/supply or price feedback. The intended model-only versus
registered game-good scope was asked asynchronously and has not yet been answered.
Supply/production mechanism and any economic integration remain to be designed;
A27 is not fully passed. Native game visibility of the added row is not yet
verified. Existing observation/no-economic-write constraints remain in force.

## Existing allocation rules

**Real metadata (R72 candidate):** 53 exact-build live definitions and main-thread
TLS context were observed with x64dbg and replayed through the actual decoder.
All keys/categories/local/tradeable match installed definitions by key.
The UI-timer candidate observes the initialized current-thread catalog via safe
reads, checks context/catalog before and after economy collection, and populates
observed goods by native ID. Unknown IDs or missing/changed metadata reject the
candidate and withdraw formal data; no `goodN` or guessed flags remain. Source
orders are preserved, while local/nontradeable cross-state allocations and
intermediary Freight demand are excluded. No engine pointer enters the worker;
no getter, native lock or TLS initialization is used. **The new DLL adapter has
not run in-game:** active R69 still uses the older default metadata. Rereads do
not prove atomic economy collection or all lifecycle behavior. Weight and Freight
conversion retain explicit model assumptions of 1.

- Inputs contain numeric copies: world/revision, stable state/region/market/good
  identities, production and consumption in integer milli-orders (1000 = 1
  game order). No engine objects survive into the worker/model.
- Local production satisfies local consumption first. Only net surplus and net
  demand participate in inter-state allocation. Local-only goods are excluded.
- Each market/good is optimized independently with min-cost maximum flow on the
  physical spline graph. Residual reverse arcs can reassign earlier choices.
  Shipments are decomposed from final flows, never from tentative augmentations.
- The default distance tariffs are road 1, built rail 0.4, naval/coastal 0.25,
  and a verified port shore transfer 1. These are explicit Mod model rules,
  not recovered game tariffs. No capacity/bottleneck model is claimed.
- Authored rail geometry alone does not mean built track. The snapshot must
  explicitly identify built rail edges before routing may use them. Closed edges
  and transport-type restrictions affect the same graph used for allocation.
- A state region owns several authored hubs (`region*100` … `+4`). The allocation
  uses the best-connected hub that the current policy actually allows (unbuilt
  rail and closed edges excluded), so a state whose city hub is rail-only can
  still be served through its port/road hub. If no allowed hub remains, the
  balance stays explicitly unallocated. This is an explicit model rule, not a
  recovered vanilla rule; the best-hub choice is deterministic (highest allowed
  degree, then lowest node id).
- Missing anchors and disconnected routes retain unallocated supply/demand.
  No straight chord is used to satisfy a shipment.
- The model is observational and does not change market prices, MAPI, buildings,
  game buy/sell orders, needs or vanilla trade allocations.

## Quantities shown by views

R74 candidate accepts an optional native matched-goods restriction. It copies
the same row IDs cached by `Market.AccessMarketGoods`, after vanilla localized
search and category exclusion. The model never rematches that native query
against English keys. IDs are sorted for cache identity; this is a view input,
not an allocation constraint. All-market freight, heat reference, original goods
targets and intermediary Freight target/fulfilled/unmet remain unchanged.
Missing/busy/dirty/mismatched native cache means unavailable and withdraws the
formal view; a clean known empty match set draws no ribbons while preserving
whole-market statistics. Positive native cache/DLL interaction acceptance is
still pending; only the dirty-cache rejection path has copied live evidence.

`targetImport` is residual demand. `allocatedImport` is the quantity actually
assigned by this Mod model and is bounded by that target. They remain distinct.
The final live UI must label these as model allocations, not an engine-provided
record of goods travelling between states.

Shipment freight = quantity × configured good weight × physical route length.
It is an abstract weighted-order-distance quantity; it is not tonnes or currency.
Here quantity is already `shipment.quantity / kQuantityScale` in orders. The
native summary formats order-distance directly, without dividing by 1000 again.
R75 corrects the previous native summary scale and its misleading order-cost
label. Historical screenshots of that summary do not establish this unit.
R77 separates `marketFreight` (all-market share denominator), `selectionFreight`
(current state/search/category selection, before hover) and `filteredFreight`
(temporary map contribution after hover). The reference card uses selection
work; hovering never changes its total. State goods rows sum only shipments
whose source or destination is that state, once per final shipment. They do not
display another state's goods work, while their share denominator stays market-wide.
R78 publishes native goods work/share cells by validated goods key, with a fixed
whole-market denominator. A zero-work row may show 0.00; a zero-market share is
undefined and displays --. Synthetic/missing data, duplicate or invalid keys,
and invalid work never become plausible zero readings. UI publication validates
the current frame again and uses a panel/world context gate; stale rows are
cleared or hidden. The new publication path still needs in-game validation.
Shipment cost additionally applies the per-mode distance tariffs. Edge intensity
sums quantity × weight (physical throughput), not each shipment's full charge.
Market freight sums final shipment freight once. Goods shares divide by all
freight in the selected market even while a filter/hover hides other goods.
Laid track sums unique explicitly built rail edges and is independent of freight.

State views distinguish incoming (+1 / building-flow green) and outgoing (-1 /
building-flow orange) segment contributions. Market views aggregate each physical
edge and use five relative weighted-throughput bands: green, lime, yellow,
orange, red. The supplied video has no colour legend; throughput meaning follows
the original implementation prompt. This is not a capacity/bottleneck claim.
The band reference is the **complete market snapshot** (all goods, both
directions, each physical edge once), reported as `FreightView::heatReference`
with `kHeatScaleRevision`. Hovering/searching/state filters only extract
contributions from that fixed scale; they never rescale it from the visible
subset. A market with no freight keeps `heatReference = 0` and must show "no
freight", not a full-scale legend. `native/src/freight_engine.cpp` caches the
allocation and the per-filter view/ribbon aggregate (32 views, LRU) and rejects
stale `world`/`revision` snapshots; the background `FreightWorker` produces the
same immutable frames off the render thread. The caches are wired to the real
path (R35–R70); formal owner/hover and full lifecycle acceptance remain incomplete.
View filters and hover never change the allocated shipments.

Line width uses allocated quantity in orders, independently of freight weights,
route distance, unmet consumption or relative colour intensity. Fixed thresholds
10/50/200/500/2000/5000/20000 orders choose native widths
0.5/0.75/1/1.5/2/3/4/6. No current-view maximum is used for width. A quantity
change within a width band keeps the exact same native geometry/style identity.
These thresholds are explicit Mod presentation rules, not a recovered vanilla
consumption formula. Vanilla's selected-building resources have several widths,
but inspected building-line callers also choose style by state relationship.

`BuildFreightRibbons` turns copied view segments into immutable native geometry
requests on a worker. Each market edge is drawn once; glyph direction follows
net allocated movement and width represents total movement in both directions.
The renderer's shared pool retains constructor-time palette/width variants;
changing an existing game type's global width is unnecessary. Texture repeat
length scales with width to retain centred, proportionate arrowheads.

## Geography

The installed spline network has separate land and naval port anchors. The
generator verifies land hub `region*100+3` against the installed port locator,
then links it to naval anchor `0x800000 | region*100` only for a short shore gap
(at most 10 map units, locator discrepancy at most 3). This adds 333 explicit
transfer links to 4273 authored strips on the current map. 39 candidate transfers
are excluded because their positions disagree; arbitrary nearest-coast snapping
is not used. Transfer edge IDs occupy the Mod-owned `0xF0000000 | region` range.

## Development test input

`native/src/dev_snapshot.cpp` can build a deterministic synthetic snapshot from
the installed derived map (regions with an authored `region*100` hub, three
goods, market 1, 4000 milli-orders per even/odd state). It exists so the debug
panel can exercise the freight render path while no verified live economy source
exists. It is **not** game data: documents built from it carry `synthetic=true`,
the debug panel shows a `TEST INPUT` banner, and the native game UI must never
receive them. `BuildSyntheticSnapshot` is default-off (`test_snapshot=0`) and is
not evidence for live snapshot correctness (A13–A15).

## Evidence

`native/test/freight_tests.cpp` checks residual reassignment against a known
optimum, mass conservation, unmet demand, deterministic ordering, market
isolation, coastal paths, unbuilt rail exclusion, unique laid-track length,
filter denominators, hover/state direction, cancellation and stale-world rejection.
Native spline tests exercise an actual UK–Sweden naval route using verified port
transfers and show that the same route is unavailable with land-only routing.

These are model and geometry tests, not proof of live snapshot correctness or
game-visible freight rendering. Snapshot access windows, native cache lifecycle,
full UI and game acceptance tests remain required.

## R80 native state row boundary

Copied documents now retain view kind and state ID. State native rows require
that exact state-view identity and a unique matching state/market roster entry;
market totals cannot accidentally enter the state column. Keys are disjoint
from market rows and use native State.GetIDString signed-int32 decimal format.
Missing/synthetic/unversioned or ambiguous state inputs publish no rows. Valid
zero work is 0.00, while unavailable cells remain --. Work remains allocated
orders x weight x full route distance for shipments entering/leaving this state,
once per final shipment, unaffected by transient hover. GUI generation and
payload tests pass; runtime state owner/row publication remains pending.

## R82 correction: runtime state and physical region identities

The live adapter previously set order.region=runtime state ID. A user-visible
Midlands sample disproved this: runtime state433, engine definition17 and authored
region20 share key STATE_MIDLANDS. The candidate now reads/validates the native
definition key and maps it to the graph region ID; runtime state ID still owns
orders and UI scope. Multiple split states may share one physical region. Unknown,
ambiguous or changing keys withdraw the candidate rather than fabricate routes.
1147 current vanilla states matched675 geography keys with the actual decoder;
new-DLL allocations/routes are not yet verified and old live geographic totals
must be remeasured. Full map-changing Mod compatibility additionally requires the
actual active map/province/spline data and graph fingerprint, not just equal keys.
