#pragma once
#include <array>
#include <cmath>
#include <string>

namespace v3 {
// Absolute allocated orders per route/segment, before goods freight weights.
// These fixed thresholds deliberately do not normalise to the hovered/filtered
// maximum: the same transported quantity keeps its width in every view.
inline constexpr std::array<double,7> kFlowWidthThresholds{{10,50,200,500,2000,5000,20000}};
inline constexpr std::array<float,8> kFlowWidths{{.5f,.75f,1.f,1.5f,2.f,3.f,4.f,6.f}};
inline unsigned FlowWidthBand(double allocatedOrders) {
  unsigned band = 0;
  while (band < kFlowWidthThresholds.size() && allocatedOrders >= kFlowWidthThresholds[band]) ++band;
  return band;
}
inline std::string FlowWidthStyle(const std::string& palette, double allocatedOrders) {
  if (!std::isfinite(allocatedOrders) || allocatedOrders <= 0) return {};
  std::string base = palette;
  if (base == "ironroadGreen") base = "ironroadBuildingOutput";
  if (base == "ironroadRed") base = "ironroadBuildingInput";
  if (base == "ironroadYellow") base = "ironroadBuildingYellow";
  if (base == "supplyRouteArrow") base = "ironroadTan";
  if (base != "ironroadBuildingOutput" && base != "ironroadBuildingInput" &&
      base != "ironroadBuildingYellow" && base != "ironroadBlue" && base != "ironroadTan" &&
      base != "ironroadTrafficGreen" && base != "ironroadTrafficLime" && base != "ironroadTrafficYellow" &&
      base != "ironroadTrafficOrange" && base != "ironroadTrafficRed") return {};
  return base + "W" + std::to_string(FlowWidthBand(allocatedOrders));
}
} // namespace v3
