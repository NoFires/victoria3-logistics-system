#pragma once
#include "route_data.h"
#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_set>

namespace v3 {
// Model quantities are integer milli-orders. This is a copied numeric snapshot;
// no engine pointer, CPdxVector, or mutable game-owned container may enter it.
constexpr int64_t kQuantityScale = 1000;
// Freight is a model-owned intermediary commodity, not an engine goods ID.
// Revision 1 converts net import orders before routing, without distance or
// tariff factors. No claim of available transport capacity is made.
inline constexpr uint32_t kFreightDemandRevision = 1;
struct FreightDemand {
  double target = 0, fulfilled = 0, unmet = 0; // Freight units, not goods orders
};
struct FreightGood {
  uint32_t id = 0;
  std::string key, category;
  double weight = 1;
  bool localOnly = false;
  std::string name;
  // Explicit model coefficient: Freight units / imported order. Default 1 is
  // an assumption, independently configurable from physical freight weight.
  double freightPerOrder = 1;
  bool freightIntermediary = false; // excluded from recursive logistics demand
  bool tradeable = true; // independent of localOnly (e.g. non-local monetary goods)
  bool operator==(const FreightGood&) const = default;
};
struct StateOrders {
  uint32_t state = 0, region = 0, market = 0, good = 0;
  int64_t production = 0, consumption = 0;
  bool operator==(const StateOrders&) const = default;
};
struct NativeWorldTradeSnapshot;
struct EconomySnapshot {
  uint64_t world = 0, revision = 0;
  std::vector<FreightGood> goods;
  std::vector<StateOrders> orders;
  std::unordered_set<uint32_t> builtRailEdges;
  // Market the formal panel is scoped to (0 = derive from the orders).
  uint32_t primaryMarket = 0;
  // Owned original trade facts, including exact 1e5 fixed values and selected
  // path indices. These snapshots are never eligible for the legacy solver.
  std::shared_ptr<const NativeWorldTradeSnapshot> nativeTrade;
  // Explicit current-state terminals for a model graph built from original
  // runtime connections. Values are owned node indices, not native map IDs.
  std::unordered_map<uint32_t,uint32_t> stateTerminals;
};
struct FreightPolicy {
  uint64_t graphRevision = 1;
  uint32_t allowedTypes = 31;
  std::array<double,5> costPerDistance{{1, .4, .25, .25, 1}};
  std::unordered_set<uint32_t> closedEdges;
};
struct TravelEdge { uint32_t id; bool forward; };
struct Shipment {
  uint32_t source = 0, destination = 0, market = 0, good = 0;
  int64_t quantity = 0;
  double distance = 0, cost = 0, freight = 0;
  std::vector<TravelEdge> route;
};
struct FreightBalance {
  uint32_t state = 0, market = 0, good = 0;
  int64_t availableExport = 0, targetImport = 0;
  int64_t allocatedExport = 0, allocatedImport = 0;
  // Source attribution is this balance's (state, market, good). Target is
  // computed BEFORE routing; final shipments fulfill it AFTER decomposition.
  FreightDemand freightDemand;
};
struct FreightResult {
  uint64_t world = 0, snapshotRevision = 0, graphRevision = 0;
  bool complete = false, cancelled = false;
  std::string error;
  std::vector<Shipment> shipments;
  std::vector<FreightBalance> balances;
  double builtRailLength = 0;
  uint32_t freightDemandRevision = 0; // 0 = unavailable/legacy result
};

// Successive shortest augmenting paths on the physical spline graph. Residual
// reverse arcs allow earlier allocations to be reassigned; this is not a greedy
// nearest-supplier approximation. Final positive flows are decomposed only after
// optimization, so reverse augmentations cannot leave phantom shipments.
FreightResult AllocateFreight(const EconomySnapshot& snapshot, const SplineGraph& graph,
                             const FreightPolicy& policy,
                             const std::function<bool()>& cancelled = {});

enum class FreightViewKind { Market, State };
enum class FreightTransport { All, Land, Sea };
enum class FreightColorMode { Business, Goods, Native };
struct FreightFilter {
  FreightViewKind kind = FreightViewKind::Market;
  uint32_t market = 0, state = 0;
  std::string search, category;
  int64_t hoverGood = -1;
  // Map-only filters. Full scope quantities/demand and goods rows stay fixed.
  int mapDirection = 0; // +1 imports, -1 exports, 0 both (relative to state)
  FreightTransport mapTransport = FreightTransport::All;
  FreightColorMode colorMode = FreightColorMode::Business;
  bool restrictCategories = false;
  std::vector<std::string> categories;
  bool restrictGoods = false;
  std::vector<uint32_t> allowedGoods; // copied native row IDs; empty restriction means no matches
};
struct SegmentFreight {
  uint32_t edge = 0;
  // Physical throughput in weighted orders. A route's full freight charge is
  // never copied onto every segment. State direction is relative to selection.
  double throughput = 0;
  int direction = 0;  // +1 import, -1 export, 0 market aggregate
  uint8_t band = 0;
  double quantity = 0; // allocated orders, independent of freight weight/target imports
  double netQuantity = 0; // positive along authored edge, negative against it
  uint8_t widthBand = 0;
};
struct GoodFreight { uint32_t good; double freight, share; };
// Heat is a display statistic, not a capacity/bottleneck model. The reference
// scale is the complete market snapshot (all goods, both directions): hovering
// or filtering a good only extracts contributions from that fixed scale. The
// revision records the statistic definition so a legend can state it.
inline constexpr uint32_t kHeatScaleRevision = 1;
struct FreightView {
  double marketFreight = 0, filteredFreight = 0, builtRailLength = 0;
  // Current state/search/category selection, before transient goods hover.
  double selectionFreight = 0;
  // Model demand totals in orders for the selected market (market view) or the
  // selected state (state view). They describe the whole scope and are NOT
  // reduced by goods filters, exactly like the market freight denominator.
  // allocatedImport is always <= targetImport; unmetDemand is their difference.
  double targetImport = 0, allocatedImport = 0, unmetDemand = 0;
  // Whole selected scope, unchanged by goods filters/hover. Missing is not 0.
  std::optional<FreightDemand> intermediaryDemand;
  // Market-wide maximum edge throughput in weighted orders. Zero when the
  // market moves no freight; the UI must show "no freight", never a full scale.
  double heatReference = 0;
  uint32_t heatScaleRevision = kHeatScaleRevision;
  std::vector<SegmentFreight> segments;
  std::vector<GoodFreight> goods;
};
FreightView BuildFreightView(const EconomySnapshot& snapshot, const FreightResult& result,
                            const FreightFilter& filter);
}  // namespace v3
