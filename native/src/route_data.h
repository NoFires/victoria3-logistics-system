#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "game.h"

namespace v3 {

struct StateEntry {
  int id = 0;
  std::string key;
  std::string name;
  Vec3 world;
  uint32_t cityColor = 0;  // province colour of the state's city, 0 if unknown
};

struct HeightField {
  int width = 0;
  int height = 0;
  float x0 = 0, z0 = 0;
  float x1 = 0, z1 = 0;
  std::vector<uint16_t> samples;

  bool Valid() const { return width > 1 && height > 1 && !samples.empty(); }
  float Sample(float worldX, float worldZ) const;
};

// Province adjacency graph (data/provinces.bin, generated from the local map).
struct ProvinceNode {
  uint32_t color = 0;
  float x = 0, z = 0, height = 0;
  uint32_t firstEdge = 0, edgeCount = 0;
  uint32_t terrainId = 0;
  uint32_t flags = 0;  // 1 = land, 2 = sea/lake
};

struct ProvinceGraph {
  std::vector<ProvinceNode> nodes;
  std::vector<uint32_t> edges;
  std::vector<float> terrainFactors;
  std::unordered_map<uint32_t, uint32_t> byColor;

  bool Valid() const { return nodes.size() > 2 && !edges.empty(); }
  bool Load(const std::wstring& directory, std::string* error);
};

struct SplineJunction { uint32_t id; float x, z; };
struct SplineEdge {
  uint32_t id, from, to, type, first, count;
  float length;
};
struct SplinePoint { float x, z; };
struct SplineRoute {
  std::vector<Vec3> points;
  std::vector<uint32_t> edgeIds;
  float distance = 0;
};
struct SplineGraph {
  float width = 0, height = 0;
  std::vector<SplineJunction> nodes;
  std::vector<SplineEdge> edges;
  std::vector<SplinePoint> points;
  std::vector<std::vector<uint32_t>> adjacency;
  std::unordered_map<uint32_t, uint32_t> byId;
  // Native 64-bit connection key -> ordered physical strip IDs (one or more).
  // Present in format 3; runtime connection IDs belong to a different space.
  std::unordered_map<uint64_t, std::vector<uint32_t>> nativeConnections;
  bool Load(const std::wstring& directory, std::string* error);
  bool Valid() const { return !nodes.empty() && !edges.empty(); }
  SplineRoute Route(const StateEntry& from, const StateEntry& to,
                    const HeightField& heights, uint32_t typeMask = 31,
                    const std::function<bool()>& cancelled = {}) const;
};

// Loads state positions, the height field and the optional province graph.
bool LoadRouteData(const std::wstring& directory, std::vector<StateEntry>* states,
                   HeightField* heights, ProvinceGraph* graph, std::string* error);

// Straight chord sampled along the height field (stage 1 / fallback).
std::vector<Vec3> BuildStraightRoute(const StateEntry& from, const StateEntry& to,
                                     const HeightField& heights, int subdivisions);

// Land-following path over the province graph (approximation of the authored spline
// network). Returns an empty vector when either city is missing or no land path exists.
std::vector<Vec3> BuildProvinceRoute(const StateEntry& from, const StateEntry& to,
                                     const ProvinceGraph& graph, const HeightField& heights,
                                     int* nodeCount, const std::function<bool()>& cancelled = {});

}  // namespace v3
