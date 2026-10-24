#pragma once
#include "route_data.h"
#include <memory>

namespace v3 {
// Derived map data, owned by the DLL and immutable after startup. Nothing here
// is a pointer into a game database or an engine graphics resource.
struct RouteGeography {
  uint64_t revision = 1;
  std::vector<StateEntry> states;
  HeightField heights;
  ProvinceGraph provinces;
  SplineGraph splines;
};

using GeographyHandle = std::shared_ptr<const RouteGeography>;
}  // namespace v3
