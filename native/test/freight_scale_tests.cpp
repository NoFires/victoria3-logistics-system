#include "dev_snapshot.h"
#include "freight_engine.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
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
  for (size_t size : {size_t(48), geography->states.size()}) {
    for (int rails = 0; rails < 2; ++rails) {
      auto snapshot = BuildSyntheticSnapshot(*geography, size);
      if (rails) {
        for (const auto& edge : geography->splines.edges)
          if (edge.type == 1) snapshot.builtRailEdges.insert(edge.id);
      }
      FreightEngine engine(geography);
      Check(engine.SetSnapshot(snapshot), "synthetic scale snapshot accepted");
      FreightFilter filter;
      filter.market = 1;
      auto frame = engine.Build(filter);
      Check(frame && frame->error.empty() && frame->view && frame->allocation,
            "synthetic scale frame builds");
      const auto& view = *frame->view;
      Check(view.targetImport > 0, "synthetic scale target is present");
      Check(std::abs(view.targetImport - view.allocatedImport - view.unmetDemand) < 1e-6,
            "allocated + unmet equals target");
      Check(view.allocatedImport <= view.targetImport + 1e-6, "allocation never exceeds target");
      Check(frame->ribbons.size() <= 8192, "freight ribbons stay within the global native pool");
      auto warm = engine.Build(filter);
      Check(warm->allocationReused && warm->viewReused, "repeat view is fully cached");
      if (rails) {
        Check(view.allocatedImport / view.targetImport > 0.9,
              "with rail considered built the synthetic map is mostly allocatable");
      }
      std::printf("%s states=%zu rails=%d orders=%zu shipments=%zu segments=%zu ribbons=%zu target=%.0f allocated=%.0f unmet=%.0f cold=%.2fms\n",
                  rails ? "[built-rail]" : "[no-rail]", size, rails, snapshot.orders.size(),
                  frame->allocation->shipments.size(), view.segments.size(), frame->ribbons.size(),
                  view.targetImport, view.allocatedImport, view.unmetDemand, frame->computeMs);
    }
  }
  std::puts("PASS: real-map synthetic scaling, allocation/unmet accounting, cache reuse and 8192-ribbon bound");
}
