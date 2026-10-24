#include "dev_snapshot.h"
#include "logistics_session.h"
#include <cstdio>
#include <cstdlib>
#include <set>
using namespace v3;
void Check(bool ok, const char* label) {
  if (!ok) { std::printf("FAIL: %s\n", label); std::exit(1); }
}
int main() {
  auto geography = std::make_shared<RouteGeography>();
  std::string error;
  Check(geography->splines.Load(L"data", &error), error.c_str());
  Check(LoadRouteData(L"data", &geography->states, &geography->heights, &geography->provinces, &error),
        error.c_str());
  auto snapshot = BuildSyntheticSnapshot(*geography);
  Check(snapshot.world == 1 && snapshot.revision == 1, "synthetic identity");
  Check(snapshot.goods.size() == 3, "three labelled synthetic goods");
  Check(snapshot.orders.size() >= 24, "enough synthetic states for flows");
  for (const auto& good : snapshot.goods)
    Check(!good.key.empty() && !good.localOnly && good.weight == 1, "explicit default good weight");
  int producers[4] = {0, 0, 0, 0}, consumers[4] = {0, 0, 0, 0};
  for (const auto& order : snapshot.orders) {
    Check(order.market == 1 && order.good >= 1 && order.good <= 3, "synthetic market/good identity");
    Check(geography->splines.byId.count(order.region * 100) != 0, "every order has an authored hub");
    if (order.production > 0) ++producers[order.good];
    if (order.consumption > 0) ++consumers[order.good];
  }
  for (int good = 1; good <= 3; ++good)
    Check(producers[good] > 0 && consumers[good] > 0, "each good has both sides");

  LogisticsSession session(geography, FreightPolicy{});
  Check(session.SetSnapshot(std::make_shared<const EconomySnapshot>(snapshot), &error, true),
        error.c_str());
  LogisticsSelection selection;
  session.SetSelection(kOwnerDebugPreview, kOwnerDebugPreview, selection);
  const auto deadline = GetTickCount64() + 15000;
  std::shared_ptr<const LogisticsDocument> document;
  while (GetTickCount64() < deadline) {
    document = session.Poll();
    if (document) break;
    Sleep(1);
  }
  Check(document != nullptr, "synthetic snapshot produces a document on the real map");
  Check(document->synthetic, "document is marked as synthetic test input");
  Check(document->market == 1, "first market resolved");
  Check(document->allFreight > 0, "model allocates freight across the authored graph");
  Check(!document->goods.empty(), "goods are present in the copied document");
  const auto status = session.Status();
  Check(status.haveFrame && status.applied > 0 && status.ribbons > 0,
        "session produced native ribbon requests");
  std::printf("synthetic snapshot: %zu orders, %zu ribbons, freight %.2f, compute %.2f ms\n",
              snapshot.orders.size(), status.ribbons, document->allFreight, status.computeMs);
  // The frame and its ribbon identities are deterministic for the same input.
  const auto frame = session.Frame();
  Check(frame && frame->allocation && frame->allocation->shipments.size() > 0,
        "frame retains the allocation");
  std::set<unsigned> bands;
  for (const auto& ribbon : frame->ribbons) {
    const auto pos = ribbon->style.rfind('W');
    if (pos != std::string::npos && pos + 1 < ribbon->style.size())
      bands.insert(unsigned(ribbon->style[pos + 1] - '0'));
  }
  Check(bands.size() >= 4, "synthetic ribbons exercise several native width bands");
  Check(*bands.rbegin() >= 3, "synthetic quantities reach a wide band");
  std::printf("synthetic width bands:");
  for (unsigned band : bands) std::printf(" %u", band);
  std::puts("");
  std::puts("PASS: synthetic test snapshot on the real map produces routed freight and native ribbon requests");
}
