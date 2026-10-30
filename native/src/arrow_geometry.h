#pragma once
#include <cmath>
#include <vector>
#include "game.h"

namespace v3 {
// CArrow evaluates Catmull-Rom from knot[1] through knot[count-2]. Duplicate
// endpoint controls preserve the complete ordered source -> destination route.
// Current native spline inputs already use world Z; bypass image reflection
// for them to retain the original floats, including near-zero control values.
inline std::vector<Vec3> NativeArrowPoints(const std::vector<Vec3>& points,
    float worldHeight, float coordScale, float heightScale, float heightOffset,
    bool useHeights,bool nativeCoordinates=false) {
  if (points.size() < 2 || points.size() > 4094 || !std::isfinite(worldHeight) ||
      !std::isfinite(coordScale) || !std::isfinite(heightScale) ||
      !std::isfinite(heightOffset)) return {};
  std::vector<Vec3> result(points.size() + 2);
  for (size_t i = 0; i < points.size(); ++i) {
    const auto& p = points[i];
    Vec3 q{p.x * coordScale, (useHeights ? p.y * heightScale : p.y) + heightOffset,
           (nativeCoordinates?p.z:worldHeight-p.z) * coordScale};
    if (!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z) ||
        !std::isfinite(q.x) || !std::isfinite(q.y) || !std::isfinite(q.z)) return {};
    result[i + 1] = q;
  }
  result.front() = result[1];
  result.back() = result[result.size() - 2];
  return result;
}
} // namespace v3
