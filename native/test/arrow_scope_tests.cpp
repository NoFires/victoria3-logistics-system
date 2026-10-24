#include "arrow_scope.h"
#include <cstdio>
#include <cstdlib>

void require(bool condition, const char* message) {
  if (!condition) { std::printf("FAIL: %s\n", message); std::exit(1); }
}
int main() {
  using namespace v3;
  // Reproduce the live callback order. The old implementation selected stage 3
  // and used up the frame guard before the native arrow stage 1 arrived.
  ArrowPassContext passes[] = {
    {{0,1},{3,1},{0,1}}, {{0,1},{2,1},{0,1}}, {{0,1},{1,1},{0,1}},
    {{0,1},{3,1},{0,1}}, {{0,1},{2,1},{0,1}}, {{0,1},{1,1},{0,1}}
  };
  int submitted = 0, chosenStage = -1; ArrowSubmissionGuard guard;
  for (const auto& context : passes) {
    if (!guard.Allow(42,context,0,1)) continue;
    chosenStage = context.stage.index; ++submitted;
  }
  require(submitted == 1 && chosenStage == 1, "select native stage before frame deduplication");
  require(!MatchesArrowScope(passes[0],0,1), "handle generation is not a stage index");
  require(MatchesArrowScope(passes[0],0,3), "selection follows live handles when indices change");
  require(!MatchesArrowScope(passes[2],-1,1), "uninitialized domain fails closed");
  require(!MatchesArrowScope(passes[2],0,-1), "uninitialized stage fails closed");
  require(!MatchesArrowScope(passes[2],7,1), "unrelated domain excluded");
  auto wrappedView = passes[2]; wrappedView.view.index = 1;
  require(guard.Allow(42,wrappedView,0,1), "wrapped-map view is submitted once too");
  require(!guard.Allow(42,wrappedView,0,1), "duplicate wrapped view rejected");
  require(guard.Allow(43,wrappedView,0,1), "new frame reactivates view");
  std::puts("PASS: native arrow pass layout, live scope selection, frame deduplication");
}
