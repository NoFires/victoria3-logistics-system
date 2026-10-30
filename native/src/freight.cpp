#include "freight.h"
#include "flow_width.h"
#include "log.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <limits>
#include <map>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <utility>
#include <tuple>

namespace v3 {
namespace {
constexpr int64_t kMaxQuantity = 1000000000000LL;
constexpr uint32_t kNone = UINT32_MAX;
struct Arc {
  uint32_t to, reverse, edge;
  int64_t capacity, original;
  double cost;
  bool forward;
};
using Network = std::vector<std::vector<Arc>>;
uint32_t AddArc(Network& net, uint32_t a, uint32_t b, int64_t quantity,
                double cost, uint32_t edge = kNone, bool forward = true) {
  uint32_t ai = uint32_t(net[a].size()), bi = uint32_t(net[b].size());
  net[a].push_back({b, bi, edge, quantity, quantity, cost, forward});
  net[b].push_back({a, ai, kNone, 0, 0, -cost, !forward});
  return ai;
}
void CheckCancel(const std::function<bool()>& cancelled) {
  if (cancelled && cancelled()) throw std::runtime_error("cancelled");
}
std::string Lower(std::string s) {
  for (char& c : s) if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
  return s;
}
// Five display bands over the fixed market reference. Zero/unavailable
// throughput maps to the lowest band; a zero reference (no freight) never
// produces a division or a fake full-scale colour.
uint8_t HeatBand(double throughput, double reference) {
  if (!(reference > 0) || !std::isfinite(reference) || !std::isfinite(throughput) || throughput <= 0)
    return 0;
  const double relative = std::min(1.0, throughput / reference);
  return static_cast<uint8_t>(std::min<unsigned>(4, static_cast<unsigned>(relative * 5.0)));
}
}  // namespace

FreightResult AllocateFreight(const EconomySnapshot& snapshot, const SplineGraph& graph,
                             const FreightPolicy& policy, const std::function<bool()>& cancelled) {
  FreightResult result;
  result.world = snapshot.world; result.snapshotRevision = snapshot.revision;
  result.graphRevision = policy.graphRevision;
  try {
    CheckCancel(cancelled);
    if ((!graph.Valid() && (snapshot.stateTerminals.empty() || graph.nodes.empty())) || graph.nodes.size() > 1000000)
      throw std::runtime_error("invalid transport graph");
    if (snapshot.orders.size() > 1000000) throw std::runtime_error("economy snapshot too large");
    for (double c : policy.costPerDistance)
      if (!std::isfinite(c) || c <= 0) throw std::runtime_error("invalid route cost");
    std::map<uint32_t, const FreightGood*> goods;
    for (const auto& g : snapshot.goods)
      if (g.key.empty() || !std::isfinite(g.weight) || g.weight <= 0 || g.weight > 1000000 ||
          !std::isfinite(g.freightPerOrder) || g.freightPerOrder <= 0 || g.freightPerOrder > 1000000 ||
          !goods.emplace(g.id,&g).second)
        throw std::runtime_error("invalid or duplicate good");
    std::map<std::pair<uint32_t,uint32_t>,std::vector<size_t>> groups;
    std::map<std::pair<uint32_t,uint32_t>,bool> uniqueOrders;
    std::map<uint32_t,std::pair<uint32_t,uint32_t>> stateIdentities;
    for (const auto& o : snapshot.orders) {
      if (!goods.count(o.good) || (snapshot.stateTerminals.empty() && o.region > UINT32_MAX/100) || o.production < 0 || o.consumption < 0 ||
          o.production > kMaxQuantity || o.consumption > kMaxQuantity ||
          !uniqueOrders.emplace(std::make_pair(o.state,o.good),true).second)
        throw std::runtime_error("invalid or duplicate state orders");
      auto identity = std::make_pair(o.region,o.market);
      auto [it,inserted] = stateIdentities.emplace(o.state,identity);
      if (!inserted && it->second != identity) throw std::runtime_error("inconsistent state identity");
      const auto& good = *goods.at(o.good);
      const bool excluded = good.localOnly || !good.tradeable || good.freightIntermediary || Lower(good.key) == "freight";
      int64_t balance = excluded ? 0 : o.production - o.consumption;
      size_t index = result.balances.size();
      result.balances.push_back({o.state,o.market,o.good,std::max<int64_t>(balance,0),
                                std::max<int64_t>(-balance,0),0,0});
      // Generate intermediary demand from the complete target, including demand
      // that supply or geography will later leave unserved. Never derive target
      // from the shipments that happen to be possible.
      auto& demand = result.balances.back().freightDemand;
      demand.target = double(result.balances.back().targetImport) / kQuantityScale * good.freightPerOrder;
      demand.unmet = demand.target;
      if (!excluded) groups[{o.market,o.good}].push_back(index);
    }
    std::map<uint32_t,uint32_t> edgeIndex;
    double flowMs = 0, decompMs = 0;
    const auto nowMs = [] {
      return std::chrono::duration<double, std::milli>(
          std::chrono::steady_clock::now().time_since_epoch()).count();
    };
    for (uint32_t i=0;i<graph.edges.size();++i) {
      const auto& e=graph.edges[i];
      if (e.from >= graph.nodes.size() || e.to >= graph.nodes.size() || e.from == e.to ||
          !std::isfinite(e.length) || e.length < 0 || (e.length==0 && (snapshot.stateTerminals.empty() || e.type!=2)) ||
          e.type > 4 || !edgeIndex.emplace(e.id,i).second)
        throw std::runtime_error("invalid transport edge");
      if (e.type == 1 && snapshot.builtRailEdges.count(e.id)) result.builtRailLength += e.length;
    }
    for (uint32_t id : snapshot.builtRailEdges)
      if (!edgeIndex.count(id) || graph.edges[edgeIndex.at(id)].type != 1)
        throw std::runtime_error("built railway references an unknown/non-rail edge");
    // A state region owns several authored hubs (region*100 + suffix). The
    // city anchor (suffix 0) can be rail-only; when that rail is not built the
    // state would look unreachable even though its port/road hub is usable.
    // Pick the best-connected allowed hub once for the whole allocation.
    std::vector<uint32_t> activeDegree(graph.nodes.size(), 0);
    for (const auto& e : graph.edges) {
      if (!(policy.allowedTypes & (1u << e.type)) || policy.closedEdges.count(e.id) ||
          (e.type == 1 && !snapshot.builtRailEdges.count(e.id)))
        continue;
      ++activeDegree[e.from];
      ++activeDegree[e.to];
    }
    const auto anchorFor = [&](uint32_t region) -> uint32_t {
      uint32_t best = UINT32_MAX, bestDegree = 0;
      for (uint32_t suffix = 0; suffix < 5; ++suffix) {
        const auto it = graph.byId.find(region * 100 + suffix);
        if (it == graph.byId.end()) continue;
        const uint32_t node = it->second, degree = activeDegree[node];
        if (!degree) continue;
        if (best == UINT32_MAX || degree > bestDegree || (degree == bestDegree && node < best)) {
          best = node;
          bestDegree = degree;
        }
      }
      return best;
    };
    const auto runGroup = [&](const std::pair<uint32_t,uint32_t>& group,
                              std::vector<size_t>& indices,
                              std::vector<Shipment>& localShipments,
                              double& localFlowMs, double& localDecompMs) {
      CheckCancel(cancelled);
      std::sort(indices.begin(),indices.end(),[&](size_t a,size_t b) { return result.balances[a].state < result.balances[b].state; });
      int64_t available=0, wanted=0;
      for (size_t i : indices) { available += result.balances[i].availableExport; wanted += result.balances[i].targetImport; }
      const int64_t limit=std::min(available,wanted);
      if (!limit) return;
      uint32_t source=uint32_t(graph.nodes.size()), sink=source+1;
      Network net(sink+1);
      const double groupFlowStart = nowMs();
      for (const auto& [id,ei] : edgeIndex) {
        const auto& e=graph.edges[ei];
        if (!(policy.allowedTypes & (1u<<e.type)) || policy.closedEdges.count(e.id) ||
            (e.type == 1 && !snapshot.builtRailEdges.count(e.id))) continue;
        double cost=e.length*policy.costPerDistance[e.type];
        AddArc(net,e.from,e.to,limit,cost,ei,true);
        AddArc(net,e.to,e.from,limit,cost,ei,false);
      }
      struct Terminal { size_t balance; uint32_t node, arc; };
      std::vector<Terminal> exporters, importers;
      for (size_t i : indices) {
        const auto& b=result.balances[i];
        auto [region,market]=stateIdentities.at(b.state);
        uint32_t anchor=UINT32_MAX;
        if(snapshot.stateTerminals.empty())anchor=anchorFor(region);
        else {
          const auto terminal=snapshot.stateTerminals.find(b.state);
          if(terminal==snapshot.stateTerminals.end() || terminal->second>=graph.nodes.size())
            throw std::runtime_error("invalid current state terminal");
          anchor=terminal->second;
        }
        // No usable authored hub retains an explicit unallocated balance.
        if (anchor==UINT32_MAX || anchor>=source) continue;
        if (b.availableExport) exporters.push_back({i,anchor,AddArc(net,source,anchor,b.availableExport,0)});
        if (b.targetImport) importers.push_back({i,anchor,AddArc(net,anchor,sink,b.targetImport,0)});
      }
      std::vector<double> potential(net.size(),0),distance(net.size());
      std::vector<uint32_t> parentNode(net.size()),parentArc(net.size());
      int64_t sent=0;
      while (sent<limit) {
        CheckCancel(cancelled);
        std::fill(distance.begin(),distance.end(),INFINITY); distance[source]=0;
        using Entry=std::pair<double,uint32_t>;
        std::priority_queue<Entry,std::vector<Entry>,std::greater<Entry>> queue;
        queue.push({0,source}); size_t visited=0;
        while (!queue.empty()) {
          auto [d,u]=queue.top();queue.pop();
          if (d>distance[u]+1e-9) continue;
          if ((++visited & 255)==0) CheckCancel(cancelled);
          for (uint32_t i=0;i<net[u].size();++i) {
            const auto& a=net[u][i]; if (!a.capacity) continue;
            double reduced=a.cost+potential[u]-potential[a.to];
            if (reduced < -1e-6) throw std::runtime_error("invalid residual potential");
            double next=d+std::max(0.0,reduced);
            if (next+1e-9 < distance[a.to]) {
              distance[a.to]=next; parentNode[a.to]=u;parentArc[a.to]=i;
              queue.push({next,a.to});
            }
          }
        }
        if (!std::isfinite(distance[sink])) break;
        for (size_t i=0;i<net.size();++i) if (std::isfinite(distance[i])) potential[i]+=distance[i];
        int64_t amount=limit-sent;
        for (uint32_t v=sink;v!=source;v=parentNode[v]) amount=std::min(amount,net[parentNode[v]][parentArc[v]].capacity);
        for (uint32_t v=sink;v!=source;v=parentNode[v]) {
          auto& a=net[parentNode[v]][parentArc[v]];a.capacity-=amount;net[v][a.reverse].capacity+=amount;
        }
        sent+=amount;
      }
      // Work with final flows, not augmentation history. Reverse residual arcs
      // can reassign a shipment whose first tentative path was cheaper locally.
      localFlowMs += nowMs() - groupFlowStart;
      const double decompStart = nowMs();
      std::vector<std::vector<int64_t>> remaining(net.size());
      for (size_t u=0;u<net.size();++u) {
        remaining[u].resize(net[u].size());
        for (size_t i=0;i<net[u].size();++i) if (net[u][i].original)
          remaining[u][i]=net[u][i].original-net[u][i].capacity;
      }
      // Split states may share an authored hub. Keep every terminal, advancing
      // past exhausted ones once, without scanning all importers for each node.
      std::vector<size_t> importerAt(net.size(), SIZE_MAX);
      std::vector<size_t> nextImporter(importers.size(), SIZE_MAX);
      for (size_t i=importers.size();i-- > 0;) {
        nextImporter[i]=importerAt[importers[i].node];
        importerAt[importers[i].node]=i;
      }
      for (const auto& origin : exporters) {
        int64_t supply=remaining[source][origin.arc];
        while (supply>0) {
          CheckCancel(cancelled);
          std::fill(parentNode.begin(),parentNode.end(),kNone);
          std::queue<uint32_t> queue;queue.push(origin.node);parentNode[origin.node]=origin.node;
          size_t destination=SIZE_MAX;
          while (!queue.empty() && destination==SIZE_MAX) {
            uint32_t u=queue.front();queue.pop();
            auto& at = importerAt[u];
            while (at != SIZE_MAX && remaining[u][importers[at].arc] == 0) at=nextImporter[at];
            if (at != SIZE_MAX) { destination = at; break; }
            for (uint32_t i=0;i<net[u].size();++i) {
              const auto& a=net[u][i];
              if (a.edge==kNone || remaining[u][i]<=0 || parentNode[a.to]!=kNone) continue;
              parentNode[a.to]=u;parentArc[a.to]=i;queue.push(a.to);
            }
          }
          if (destination==SIZE_MAX) throw std::runtime_error("cannot decompose allocated flow");
          const auto& dest=importers[destination];
          int64_t amount=std::min(supply,remaining[dest.node][dest.arc]);
          for (uint32_t v=dest.node;v!=origin.node;v=parentNode[v]) amount=std::min(amount,remaining[parentNode[v]][parentArc[v]]);
          Shipment shipment;
          shipment.source=result.balances[origin.balance].state;
          shipment.destination=result.balances[dest.balance].state;
          shipment.market=group.first;shipment.good=group.second;shipment.quantity=amount;
          for (uint32_t v=dest.node;v!=origin.node;v=parentNode[v]) {
            uint32_t u=parentNode[v],i=parentArc[v];const auto& a=net[u][i];
            const auto& edge=graph.edges[a.edge];
            remaining[u][i]-=amount;shipment.route.push_back({edge.id,a.forward});
            shipment.distance+=edge.length;shipment.cost+=a.cost;
          }
          std::reverse(shipment.route.begin(),shipment.route.end());
          double quantity=double(amount)/kQuantityScale;
          shipment.cost*=quantity*goods.at(group.second)->weight;
          shipment.freight=quantity*goods.at(group.second)->weight*shipment.distance;
          result.balances[origin.balance].allocatedExport+=amount;
          result.balances[dest.balance].allocatedImport+=amount;
          localShipments.push_back(std::move(shipment));
          remaining[dest.node][dest.arc]-=amount;supply-=amount;
        }
      }
      localDecompMs += nowMs() - decompStart;
    };
    std::vector<std::pair<uint32_t,uint32_t>> groupKeys;
    groupKeys.reserve(groups.size());
    for (const auto& entry : groups) groupKeys.push_back(entry.first);
    std::atomic<size_t> nextGroup{0};
    std::atomic<bool> abortGroups{false};
    std::exception_ptr firstError;
    std::mutex errorMutex;
    std::vector<std::vector<Shipment>> partials(groupKeys.size());
    std::vector<double> flowPer(groupKeys.size(), 0.0), decompPer(groupKeys.size(), 0.0);
    unsigned threadCount = std::max(1u, std::thread::hardware_concurrency());
    threadCount = std::min<unsigned>(threadCount, 8u);
    threadCount = std::min<unsigned>(threadCount, std::max<size_t>(1, groupKeys.size()));
    std::vector<std::thread> pool;
    pool.reserve(threadCount);
    for (unsigned t = 0; t < threadCount; ++t) {
      pool.emplace_back([&] {
        try {
          for (;;) {
            if (abortGroups.load()) break;
            const size_t j = nextGroup.fetch_add(1);
            if (j >= groupKeys.size()) break;
            runGroup(groupKeys[j], groups[groupKeys[j]], partials[j], flowPer[j], decompPer[j]);
          }
        } catch (...) {
          std::lock_guard<std::mutex> lock(errorMutex);
          if (!firstError) firstError = std::current_exception();
          abortGroups.store(true);
        }
      });
    }
    for (auto& t : pool) t.join();
    if (firstError) std::rethrow_exception(firstError);
    for (size_t j = 0; j < groupKeys.size(); ++j) {
      flowMs += flowPer[j];
      decompMs += decompPer[j];
      for (auto& s : partials[j]) result.shipments.push_back(std::move(s));
    }
    for (auto& balance : result.balances) {
      CheckCancel(cancelled);
      auto& demand = balance.freightDemand;
      demand.fulfilled = double(balance.allocatedImport) / kQuantityScale * goods.at(balance.good)->freightPerOrder;
      demand.unmet = std::max(0.0, demand.target - demand.fulfilled);
    }
    V3_INFO("freight allocation flow=%.2f decomp=%.2f shipments=%zu threads=%u", flowMs, decompMs,
            result.shipments.size(), threadCount);
    result.freightDemandRevision = kFreightDemandRevision;
    result.complete=true;
  } catch (const std::exception& e) {
    result.cancelled=std::string(e.what())=="cancelled";
    result.error=e.what();result.shipments.clear();result.balances.clear();result.builtRailLength=0;
  }
  return result;
}

FreightView BuildFreightView(const EconomySnapshot& snapshot, const FreightResult& result,
                            const FreightFilter& filter) {
  FreightView view;
  if (!result.complete || result.world!=snapshot.world || result.snapshotRevision!=snapshot.revision) return view;
  view.builtRailLength=result.builtRailLength;
  std::map<uint32_t,const FreightGood*> goods;
  for (const auto& g:snapshot.goods) goods[g.id]=&g;
  std::map<uint32_t,double> totals;
  // Demand totals are scope facts (whole market or whole state), not view
  // aggregates: a goods filter must never look like it changed demand.
  int64_t targetImport = 0, allocatedImport = 0;
  if (result.freightDemandRevision == kFreightDemandRevision) view.intermediaryDemand = FreightDemand{};
  for (const auto& balance : result.balances) {
    const bool inScope = filter.kind == FreightViewKind::State
                             ? balance.state == filter.state
                             : balance.market == filter.market;
    if (!inScope) continue;
    targetImport += balance.targetImport;
    allocatedImport += balance.allocatedImport;
    if (view.intermediaryDemand) {
      view.intermediaryDemand->target += balance.freightDemand.target;
      view.intermediaryDemand->fulfilled += balance.freightDemand.fulfilled;
      view.intermediaryDemand->unmet += balance.freightDemand.unmet;
    }
  }
  if (targetImport > 0)
    view.targetImport = double(targetImport) / kQuantityScale;
  if (allocatedImport > 0)
    view.allocatedImport = double(allocatedImport) / kQuantityScale;
  view.unmetDemand = std::max(0.0, view.targetImport - view.allocatedImport);
  // The market heat reference is computed from the complete snapshot before any
  // filter runs: every good, both directions, each physical edge once. Hovering
  // or filtering therefore never rescales the colours; the same edge keeps its
  // band in every view of this allocation result.
  std::map<uint32_t,double> heat;
  for (const auto& s:result.shipments) {
    if (s.market!=filter.market || !goods.count(s.good)) continue;
    view.marketFreight+=s.freight;
    if(filter.kind!=FreightViewKind::State || s.source==filter.state || s.destination==filter.state)
      totals[s.good]+=s.freight;
    const double quantity=double(s.quantity)/kQuantityScale;
    const double weight=goods.at(s.good)->weight;
    std::unordered_set<uint32_t> unique;
    for (auto e:s.route) if (unique.insert(e.id).second) heat[e.id]+=quantity*weight;
  }
  for (const auto& [edge,throughput]:heat)
    view.heatReference=std::max(view.heatReference,std::isfinite(throughput)?throughput:0.0);
  struct Amount { double throughput=0, quantity=0, netQuantity=0; };
  std::map<std::pair<uint32_t,int>,Amount> segments;
  std::string search=Lower(filter.search);
  for (const auto& s:result.shipments) {
    if (s.market!=filter.market || !goods.count(s.good)) continue;
    const auto& good=*goods.at(s.good);
    if ((filter.restrictGoods && std::find(filter.allowedGoods.begin(),filter.allowedGoods.end(),good.id)==filter.allowedGoods.end()) ||
        (!search.empty() && Lower(good.key+" "+good.name).find(search)==std::string::npos) ||
        (!filter.category.empty() && filter.category!=good.category) ||
        (filter.restrictCategories && std::find(filter.categories.begin(),filter.categories.end(),good.category)==filter.categories.end())) continue;
    int direction=0;
    if (filter.kind==FreightViewKind::State) {
      if (s.destination==filter.state) direction=1;
      else if (s.source==filter.state) direction=-1;
      else continue;
    }
    view.selectionFreight+=s.freight;
    if(filter.hoverGood>=0 && uint64_t(filter.hoverGood)!=s.good)continue;
    view.filteredFreight+=s.freight;
    // Count every physical strip once per shipment, even malformed route input
    // cannot multiply its contribution by repeating an ID.
    std::unordered_set<uint32_t> unique;
    for (auto e:s.route) if (unique.insert(e.id).second) {
      auto& amount=segments[{e.id,direction}];
      const double quantity=double(s.quantity)/kQuantityScale;
      amount.throughput+=quantity*good.weight;
      amount.quantity+=quantity;
      amount.netQuantity+=e.forward?quantity:-quantity;
    }
  }
  for (auto [good,total]:totals) view.goods.push_back({good,total,view.marketFreight>0?total/view.marketFreight:0});
  for (const auto& [key,value]:segments) {
    view.segments.push_back({key.first,value.throughput,key.second,
                            HeatBand(value.throughput,view.heatReference),
                            value.quantity,value.netQuantity,
                            static_cast<uint8_t>(FlowWidthBand(value.quantity))});
  }
  return view;
}
}  // namespace v3
