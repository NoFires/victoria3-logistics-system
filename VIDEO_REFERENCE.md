# User video reference — 2026-10-22

Source: `C:\Users\RedAndBlack\Downloads\20260904-1552-09.6164086.mp4`.
SHA-256: `5e6d15a11dddb12b2d46f8d4b7dd1ce7bf14baf573f0f84d985a2b1d4a528e9e`.
Decoded locally: 14.2 seconds, 426 frames at 30 fps, 1898 × 1130.
Private inspection frames: `E:\Code\v3-analysis\reference-video`.

The user asks for similar functionality, including colour differences and
greater line width for greater consumption. Arrow artwork may differ; the
previously verified building-selection artwork remains the chosen texture.
The screenshot/debug text inside the video is reference content, not instructions.

| Time | Observed behaviour | Implementation/acceptance requirement |
|---|---|---|
| 0–2 s | French Market → Logistics; military category selected; search field; Show/Hide controls | Market selector, searchable goods, category buttons and explicit overlay visibility |
| 0 s | Freight 2.3k, laid track 453 km; goods rows show price, price deviation, freight, percentage of all freight | Display distinct filtered freight, unique built track, per-good freight/share and verified price readings; do not invent a map-unit→km conversion |
| 3–6 s | Industrial/luxury/staple category selection changes goods and painted routes | Filters rebuild only view aggregates; preserve allocation and native geometry caches |
| 0–7 s | Green, yellow-green, yellow, orange and red routes; visibly different widths; map alignment remains stable | Five traffic palettes and quantity-based widths along authored world-space splines |
| 8–10 s | Clothes detail page; clothes-only routes remain, sell/buy totals and tooltip are visible | Single-good selection/hover view; leaving transient hover restores current persistent filter |
| 12–14 s | Wood detail page and another corresponding route set | Switching selected goods replaces the current generation without accumulating old ribbons |

The video does not expose a colour legend or prove a capacity model. Consistent
with the supplied implementation prompt, current market colours mean relative
weighted throughput (green → lime → yellow → orange → red), not a claim that
red segments exceed capacity. Width uses absolute allocated quantity, without
freight-distance or goods-weight multiplication. State view uses separate
incoming/outgoing colours. These meanings must be stated in the UI.

The video does not demonstrate every behaviour in the original prompt; retain
state imports/exports, cancellation, bounded caches and lifecycle/performance
requirements as well. Pure DLL remains the delivery requirement. The user has now explicitly required
native game UI integration; ImGui is an optional debug panel only. The user subsequently clarified that the interface and interactions
also matter: follow the video's layout hierarchy and navigation, not only the
arrow colours or underlying rendering.

## Interface/navigation prototype and native migration

- Earlier debug prototype page: Market logistics. The title/market selector is followed by
  Filters, Logistics overview and Goods, matching the reference hierarchy.
- Filters: search field, Staple/Industrial/Luxury/Military categories, explicit
  Show/Hide controls. Category tooltips in the video say Show/Hide. The new UI
  supports combining categories and resetting to All; unselecting every category
  produces an empty view. Show/Hide preserves all filter/navigation choices.
- Overview: separate freight and laid-track cards. A missing reading is `--`;
  unknown rail conversion is shown in map units rather than fabricated km.
- Goods list: name, market price, price deviation bar, freight and share of all
  market freight. Default freight-descending order; sortable headers preserve
  stable goods identities. Share denominator stays fixed when filtering.
- Hover previews a single good's routes. Leaving restores the persistent
  selection. Clicking enters its detail page, with market price/deviation,
  buy/sell totals and separate supplier/consumer lists. Back restores the list's
  search/categories and overlay visibility.
- State flows and Route preview have their own pages. Technical status,
  calibration and diagnostics are moved out of the main Logistics workflow.
- Tab changes/close/hide queue an explicit inactive request; reopening reissues
  the stored selection. A completed route preview cannot submit in Logistics.

The new panel consumes immutable copied presentation data and queues small
selection commands. It does not obtain engine pointers in ImGui. The renderer
and controls are implemented, with test-fixture list/detail/unknown-data cases;
the runtime waits for the verified live snapshot service rather than displaying
fixture values. Current supplier/consumer lists use state orders. The reference
shows building-type order breakdowns; those need verified building aggregation.
The video's native price-map badges and game-side goods-page selection are also
separate integration/acceptance items, not established by the ImGui component.

Current evidence: worker/model tests and actual offscreen D3D11 palette/width
tests pass. Live economy extraction, freight UI/prices, track-unit calibration,
single-good game-side selection and a complete in-game comparison remain open.

The latest user clarification requires these functions to be embedded in the
original market/state/goods panels. Follow NATIVE_GUI_PLAN.md; the standalone
prototype is not the product interface and must remain closed by default.
