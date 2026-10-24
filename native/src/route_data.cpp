#include "route_data.h"

#include "log.h"
#include "util.h"

#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace v3 {
namespace {

bool ReadTextFile(const std::wstring& path, std::string* out) {
  std::vector<uint8_t> bytes;
  if (!ReadFileBytes(path, &bytes)) return false;
  out->assign((const char*)bytes.data(), bytes.size());
  return true;
}

float Dist2D(float ax, float az, float bx, float bz) {
  float dx = ax - bx, dz = az - bz;
  return std::sqrt(dx * dx + dz * dz);
}

}  // namespace

bool SplineGraph::Load(const std::wstring& directory, std::string* error) {
  *this = SplineGraph{};
  std::vector<uint8_t> raw;
  auto fail = [&](const char* why) { *this = SplineGraph{}; if (error) *error = why; return false; };
  if (!ReadFileBytes(directory + L"\\splines.bin", &raw) || raw.size() < 28)
    return fail("splines.bin missing or truncated");
  uint32_t magic, version, nn, ne, np;
  std::memcpy(&magic, raw.data(), 4); std::memcpy(&version, raw.data() + 4, 4);
  std::memcpy(&width, raw.data() + 8, 4); std::memcpy(&height, raw.data() + 12, 4);
  std::memcpy(&nn, raw.data() + 16, 4); std::memcpy(&ne, raw.data() + 20, 4);
  std::memcpy(&np, raw.data() + 24, 4);
  const uint64_t geometryBytes = 28ull + nn * 12ull + ne * 28ull + np * 8ull;
  if (magic != 0x47535249 || (version != 1 && version != 2 && version != 3) || !(width > 0 && height > 0) ||
      !std::isfinite(width) || !std::isfinite(height) || nn > 1000000 || ne > 1000000 ||
      np > 10000000 || raw.size() < geometryBytes || (version < 3 && raw.size() != geometryBytes))
    return fail("splines.bin invalid header/size");
  static_assert(sizeof(SplineJunction) == 12 && sizeof(SplineEdge) == 28 && sizeof(SplinePoint) == 8);
  nodes.resize(nn); edges.resize(ne); points.resize(np); adjacency.resize(nn);
  std::memcpy(nodes.data(), raw.data() + 28, nn * 12ull);
  std::memcpy(edges.data(), raw.data() + 28 + nn * 12ull, ne * 28ull);
  std::memcpy(points.data(), raw.data() + 28 + nn * 12ull + ne * 28ull, np * 8ull);
  for (uint32_t i = 0; i < nn; ++i) {
    const auto& n = nodes[i];
    if (!std::isfinite(n.x) || !std::isfinite(n.z) || !byId.emplace(n.id, i).second)
      return fail("splines.bin invalid junction");
  }
  for (const auto& p : points)
    if (!std::isfinite(p.x) || !std::isfinite(p.z)) return fail("splines.bin invalid point");
  std::unordered_set<uint32_t> ids;
  for (uint32_t i = 0; i < ne; ++i) {
    const auto& e = edges[i];
    if (e.from >= nn || e.to >= nn || e.count < 2 || uint64_t(e.first) + e.count > np ||
        e.type > (version == 1 ? 3u : 4u) || !std::isfinite(e.length) || e.length <= 0 || !ids.insert(e.id).second)
      return fail("splines.bin invalid edge");
    const auto& a = points[e.first]; const auto& b = points[e.first + e.count - 1];
    if (Dist2D(a.x, a.z, nodes[e.from].x, nodes[e.from].z) > .01f ||
        Dist2D(b.x, b.z, nodes[e.to].x, nodes[e.to].z) > .01f)
      return fail("splines.bin inconsistent endpoints");
    adjacency[e.from].push_back(i); adjacency[e.to].push_back(i);
  }
  if (version == 3) {
    size_t offset = size_t(geometryBytes);
    auto take = [&](auto& value) {
      if (sizeof(value) > raw.size() - offset) return false;
      std::memcpy(&value,raw.data()+offset,sizeof(value));offset+=sizeof(value);return true;
    };
    uint32_t count=0;
    if (!take(count) || !count || count>ne) return fail("splines.bin invalid native link count");
    std::unordered_map<uint32_t,const SplineEdge*> byEdge;
    for (const auto& edge:edges) byEdge.emplace(edge.id,&edge);
    std::unordered_set<uint32_t> bound;
    for (uint32_t i=0;i<count;++i) {
      uint64_t key=0;uint32_t size=0;
      if (!take(key) || !take(size) || !size || size>ne || (key&63)>3 ||
          nativeConnections.count(key)) return fail("splines.bin invalid native link");
      auto& refs=nativeConnections[key];
      uint32_t current=uint32_t((key>>6)&0x1fffffff),target=uint32_t(key>>35);
      for (uint32_t j=0;j<size;++j) {
        uint32_t id=0;
        if (!take(id) || !byEdge.count(id) || !bound.insert(id).second)
          return fail("splines.bin invalid native strip membership");
        const auto& edge=*byEdge.at(id);
        uint32_t a=nodes[edge.from].id,b=nodes[edge.to].id;
        if (edge.type!=(key&63) || (current!=a && current!=b))
          return fail("splines.bin disconnected native link");
        current=current==a?b:a;refs.push_back(id);
      }
      if (current!=target) return fail("splines.bin invalid native link endpoints");
    }
    if (offset!=raw.size()) return fail("splines.bin trailing native link bytes");
    for (const auto& edge:edges)
      if (edge.type<4 && !bound.count(edge.id)) return fail("splines.bin unbound native strip");
  }
  V3_INFO("authored spline graph loaded: %u junctions, %u edges, %u points", nn, ne, np);
  return Valid();
}

SplineRoute SplineGraph::Route(const StateEntry& from, const StateEntry& to,
                              const HeightField& heights, uint32_t typeMask,
                              const std::function<bool()>& cancelled) const {
  SplineRoute result;
  // Authored city anchors use the state's stable numeric ID * 100. Never snap
  // across a disconnected sea/river to manufacture a seemingly valid route.
  auto s = byId.find(uint32_t(from.id) * 100), t = byId.find(uint32_t(to.id) * 100);
  if (!Valid() || s == byId.end() || t == byId.end() || s->second == t->second) return result;
  const uint32_t start = s->second, goal = t->second;
  std::vector<float> distance(nodes.size(), INFINITY);
  std::vector<uint32_t> previous(nodes.size(), UINT32_MAX);
  using Entry = std::pair<float, uint32_t>;
  std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> queue;
  distance[start] = 0; queue.push({0, start});
  while (!queue.empty()) {
    if (cancelled && cancelled()) return {};
    auto [d, u] = queue.top(); queue.pop();
    if (d > distance[u]) continue;
    if (u == goal) break;
    for (uint32_t ei : adjacency[u]) {
      const auto& e = edges[ei];
      if (!(typeMask & (1u << e.type))) continue;
      uint32_t v = e.from == u ? e.to : e.from;
      float next = d + e.length;
      if (next < distance[v]) {
        distance[v] = next; previous[v] = ei; queue.push({next, v});
      }
    }
  }
  if (!std::isfinite(distance[goal])) return result;
  std::vector<std::pair<uint32_t, bool>> path;
  for (uint32_t u = goal; u != start;) {
    uint32_t ei = previous[u];
    if (ei == UINT32_MAX || path.size() > edges.size()) return {};
    const auto& e = edges[ei]; bool forward = e.to == u;
    path.push_back({ei, forward}); u = forward ? e.from : e.to;
  }
  std::reverse(path.begin(), path.end());
  for (auto [ei, forward] : path) {
    if (cancelled && cancelled()) return {};
    const auto& e = edges[ei]; result.edgeIds.push_back(e.id);
    for (uint32_t i = 0; i < e.count; ++i) {
      const auto& p = points[e.first + (forward ? i : e.count - i - 1)];
      if (!result.points.empty() && Dist2D(result.points.back().x, result.points.back().z, p.x, p.z) < .001f)
        continue;
      result.points.push_back({p.x, heights.Sample(p.x, p.z), p.z});
    }
  }
  result.distance = distance[goal];
  return result;
}

float HeightField::Sample(float worldX, float worldZ) const {
  if (!Valid()) return 0.0f;
  float u = (worldX - x0) / (x1 - x0);
  float v = (worldZ - z0) / (z1 - z0);
  if (u < 0) u = 0;
  if (v < 0) v = 0;
  if (u > 0.999999f) u = 0.999999f;
  if (v > 0.999999f) v = 0.999999f;
  float fx = u * (width - 1), fz = v * (height - 1);
  int ix = (int)fx, iz = (int)fz;
  int ix2 = ix + 1 < width ? ix + 1 : ix;
  int iz2 = iz + 1 < height ? iz + 1 : iz;
  float tx = fx - ix, tz = fz - iz;
  auto at = [&](int x, int y) { return (float)samples[(size_t)y * width + x]; };
  float a = at(ix, iz) * (1 - tx) + at(ix2, iz) * tx;
  float b = at(ix, iz2) * (1 - tx) + at(ix2, iz2) * tx;
  return a * (1 - tz) + b * tz;
}

bool ProvinceGraph::Load(const std::wstring& directory, std::string* error) {
  std::vector<uint8_t> raw;
  if (!ReadFileBytes(directory + L"\\provinces.bin", &raw) || raw.size() < 16) {
    if (error) *error = "provinces.bin missing";
    return false;
  }
  struct Header {
    uint32_t magic, version, nodeCount, edgeCount;
  } h{};
  std::memcpy(&h, raw.data(), sizeof(h));
  if (h.magic != 0x47505249 || h.version != 1) {
    if (error) *error = "provinces.bin header";
    return false;
  }
  const size_t nodeBytes = (size_t)h.nodeCount * 32;
  if (raw.size() < 16 + nodeBytes + (size_t)h.edgeCount * 4) {
    if (error) *error = "provinces.bin truncated";
    return false;
  }
  nodes.resize(h.nodeCount);
  const uint8_t* p = raw.data() + 16;
  for (uint32_t i = 0; i < h.nodeCount; ++i, p += 32) {
    ProvinceNode& n = nodes[i];
    std::memcpy(&n.color, p + 0, 4);
    std::memcpy(&n.x, p + 4, 4);
    std::memcpy(&n.z, p + 8, 4);
    std::memcpy(&n.height, p + 12, 4);
    std::memcpy(&n.firstEdge, p + 16, 4);
    std::memcpy(&n.edgeCount, p + 20, 4);
    std::memcpy(&n.terrainId, p + 24, 4);
    std::memcpy(&n.flags, p + 28, 4);
    byColor.emplace(n.color, i);
  }
  edges.resize(h.edgeCount);
  std::memcpy(edges.data(), raw.data() + 16 + nodeBytes, (size_t)h.edgeCount * 4);

  // terrain id -> movement factor
  std::string text;
  uint32_t maxId = 0;
  for (const ProvinceNode& n : nodes) maxId = std::max(maxId, n.terrainId);
  terrainFactors.assign(maxId + 1, 1.0f);
  if (ReadTextFile(directory + L"\\terrain_names.txt", &text)) {
    std::istringstream in(text);
    std::string line;
    while (std::getline(in, line)) {
      if (!line.empty() && line.back() == '\r') line.pop_back();
      size_t a = line.find('\t'), b = line.find('\t', a == std::string::npos ? a : a + 1);
      if (a == std::string::npos || b == std::string::npos) continue;
      uint32_t id = (uint32_t)std::atoi(line.substr(0, a).c_str());
      float f = (float)std::atof(line.substr(b + 1).c_str());
      if (id < terrainFactors.size() && f > 0.01f) terrainFactors[id] = f;
    }
  }
  V3_INFO("province graph: %u nodes, %u edges, %zu colours", h.nodeCount, h.edgeCount,
          byColor.size());
  return Valid();
}

bool LoadRouteData(const std::wstring& directory, std::vector<StateEntry>* states,
                   HeightField* heights, ProvinceGraph* graph, std::string* error) {
  states->clear();
  std::string text;
  if (!ReadTextFile(directory + L"\\positions.txt", &text)) {
    if (error) *error = "positions.txt not found next to the loader";
    return false;
  }
  std::istringstream in(text);
  std::string line;
  bool header = false;
  while (std::getline(in, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    if (line.empty() || line[0] == '#') continue;
    if (!header) {
      if (line.rfind("IRONROAD-POSITIONS", 0) != 0) {
        if (error) *error = "positions.txt has an unknown header";
        return false;
      }
      header = true;
      continue;
    }
    // id \t key \t name \t x \t y \t z \t city
    std::vector<std::string> parts;
    size_t start = 0;
    while (true) {
      size_t p = line.find('\t', start);
      parts.push_back(line.substr(start, p == std::string::npos ? std::string::npos : p - start));
      if (p == std::string::npos) break;
      start = p + 1;
    }
    if (parts.size() < 6) continue;
    StateEntry e;
    e.id = std::atoi(parts[0].c_str());
    e.key = parts[1];
    e.name = parts[2];
    e.world.x = (float)std::atof(parts[3].c_str());
    e.world.y = (float)std::atof(parts[4].c_str());
    e.world.z = (float)std::atof(parts[5].c_str());
    if (parts.size() >= 7 && parts[6].size() >= 6)
      e.cityColor = (uint32_t)std::strtoul(parts[6].c_str(), nullptr, 16);
    if (!e.name.empty()) states->push_back(std::move(e));
  }
  if (states->empty()) {
    if (error) *error = "positions.txt contains no states";
    return false;
  }

  // Optional height field (binary).
  std::vector<uint8_t> raw;
  if (ReadFileBytes(directory + L"\\heights.bin", &raw) && raw.size() > 32) {
    struct Header {
      uint32_t magic, version, width, height;
      float x0, z0, x1, z1;
      uint32_t flags, reserved;
    } h{};
    static_assert(sizeof(Header) == 40);
    if (raw.size() >= sizeof(Header)) {
      std::memcpy(&h, raw.data(), sizeof(h));
      if (h.magic == 0x4D485249 && h.version == 1 && h.width > 1 && h.height > 1 &&
          raw.size() >= sizeof(Header) + (size_t)h.width * h.height * 2) {
        heights->width = (int)h.width;
        heights->height = (int)h.height;
        heights->x0 = h.x0; heights->z0 = h.z0; heights->x1 = h.x1; heights->z1 = h.z1;
        heights->samples.resize((size_t)h.width * h.height);
        std::memcpy(heights->samples.data(), raw.data() + sizeof(Header), heights->samples.size() * 2);
        V3_INFO("height field loaded: %dx%d world [%.0f..%.0f]x[%.0f..%.0f]",
                heights->width, heights->height, heights->x0, heights->x1, heights->z0, heights->z1);
      }
    }
  }
  if (!heights->Valid()) V3_WARN("heights.bin missing or invalid; route y uses endpoint interpolation");

  if (graph) {
    std::string graphError;
    if (!graph->Load(directory, &graphError)) V3_WARN("province graph unavailable: %s", graphError.c_str());
  }
  V3_INFO("route data: %zu states", states->size());
  return true;
}

std::vector<Vec3> BuildStraightRoute(const StateEntry& from, const StateEntry& to,
                                     const HeightField& heights, int subdivisions) {
  if (subdivisions < 2) subdivisions = 2;
  if (subdivisions > 256) subdivisions = 256;
  std::vector<Vec3> pts;
  pts.reserve(subdivisions);
  for (int i = 0; i < subdivisions; ++i) {
    float t = (float)i / (float)(subdivisions - 1);
    Vec3 p;
    p.x = from.world.x + (to.world.x - from.world.x) * t;
    p.z = from.world.z + (to.world.z - from.world.z) * t;
    if (heights.Valid()) {
      p.y = heights.Sample(p.x, p.z);  // raw height sample; scaled by the caller
    } else {
      p.y = from.world.y + (to.world.y - from.world.y) * t;
    }
    pts.push_back(p);
  }
  return pts;
}

std::vector<Vec3> BuildProvinceRoute(const StateEntry& from, const StateEntry& to,
                                     const ProvinceGraph& graph, const HeightField& heights,
                                     int* nodeCount, const std::function<bool()>& cancelled) {
  if (nodeCount) *nodeCount = 0;
  if (!graph.Valid() || !from.cityColor || !to.cityColor) return {};
  auto sit = graph.byColor.find(from.cityColor);
  auto tit = graph.byColor.find(to.cityColor);
  if (sit == graph.byColor.end() || tit == graph.byColor.end()) return {};
  const uint32_t start = sit->second, goal = tit->second;
  const size_t n = graph.nodes.size();
  if (start >= n || goal >= n) return {};

  auto factor = [&](uint32_t id) {
    return id < graph.terrainFactors.size() ? graph.terrainFactors[id] : 1.0f;
  };
  auto heur = [&](uint32_t i) {
    return Dist2D(graph.nodes[i].x, graph.nodes[i].z, graph.nodes[goal].x, graph.nodes[goal].z);
  };

  const float kInf = 1.0e30f;
  std::vector<float> gscore(n, kInf);
  std::vector<int32_t> came(n, -1);
  std::vector<uint8_t> closed(n, 0);
  using Entry = std::pair<float, uint32_t>;  // f, node
  std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> open;
  gscore[start] = 0.0f;
  open.push({heur(start), start});

  bool found = false;
  while (!open.empty()) {
    if (cancelled && cancelled()) return {};
    auto [f, u] = open.top();
    open.pop();
    if (closed[u]) continue;
    if (u == goal) {
      found = true;
      break;
    }
    closed[u] = 1;
    const ProvinceNode& un = graph.nodes[u];
    for (uint32_t k = 0; k < un.edgeCount; ++k) {
      uint32_t v = graph.edges[un.firstEdge + k];
      if (v >= n || closed[v]) continue;
      if (!(graph.nodes[v].flags & 1)) continue;  // land only
      float d = Dist2D(un.x, un.z, graph.nodes[v].x, graph.nodes[v].z);
      float cost = d * 0.5f * (factor(un.terrainId) + factor(graph.nodes[v].terrainId));
      float ng = gscore[u] + cost;
      if (ng + 1e-3f < gscore[v]) {
        gscore[v] = ng;
        came[v] = (int32_t)u;
        open.push({ng + heur(v), v});
      }
    }
  }
  if (!found) return {};

  std::vector<uint32_t> path;
  for (int32_t u = (int32_t)goal; u >= 0; u = came[u]) path.push_back((uint32_t)u);
  std::reverse(path.begin(), path.end());
  if (path.size() < 2) return {};
  if (nodeCount) *nodeCount = (int)path.size();

  // Node centroids -> polyline, then two Chaikin passes to remove the zigzag.
  std::vector<Vec3> pts;
  pts.reserve(path.size());
  for (uint32_t i : path) {
    Vec3 p;
    p.x = graph.nodes[i].x;
    p.z = graph.nodes[i].z;
    p.y = heights.Valid() ? heights.Sample(p.x, p.z) : graph.nodes[i].height;
    pts.push_back(p);
  }
  for (int pass = 0; pass < 2 && pts.size() < 4000; ++pass) {
    if (cancelled && cancelled()) return {};
    std::vector<Vec3> out;
    out.reserve(pts.size() * 2);
    out.push_back(pts.front());
    for (size_t i = 0; i + 1 < pts.size(); ++i) {
      Vec3 a = pts[i], b = pts[i + 1];
      out.push_back({a.x * 0.75f + b.x * 0.25f, 0.0f, a.z * 0.75f + b.z * 0.25f});
      out.push_back({a.x * 0.25f + b.x * 0.75f, 0.0f, a.z * 0.25f + b.z * 0.75f});
    }
    out.push_back(pts.back());
    pts.swap(out);
  }
  for (Vec3& p : pts) {
    if (heights.Valid()) p.y = heights.Sample(p.x, p.z);
  }
  // Bound the point count for the native path setter.
  if (pts.size() > 512) {
    std::vector<Vec3> reduced;
    reduced.reserve(512);
    for (int i = 0; i < 512; ++i) {
      size_t idx = (size_t)((float)i / 511.0f * (pts.size() - 1));
      reduced.push_back(pts[idx]);
    }
    reduced.front() = pts.front();
    reduced.back() = pts.back();
    pts.swap(reduced);
  }
  return pts;
}

}  // namespace v3
