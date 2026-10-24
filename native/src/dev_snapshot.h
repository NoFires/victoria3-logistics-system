#pragma once
#include "freight.h"
#include "route_geography.h"

namespace v3 {
// Builds a deterministic, explicitly synthetic snapshot from the installed
// derived map. This exists only so the debug panel can exercise the freight
// render path before a verified live economy source exists. It is never used by
// the original-game UI and must be labelled "TEST INPUT" wherever it appears.
// Production/consumption numbers are invented presentation values, not game
// readings; goods weights are the documented default of 1.
EconomySnapshot BuildSyntheticSnapshot(const RouteGeography& geography, size_t maxStates = 48);
}  // namespace v3
