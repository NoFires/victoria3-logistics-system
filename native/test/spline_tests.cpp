#include "route_data.h"
#include "util.h"
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <filesystem>

void Require(bool ok, const char* what) { if (!ok) { std::printf("FAIL: %s\n", what); std::exit(1); } }
int main() {
  using namespace v3;
  SplineGraph graph; std::string error;
  Require(graph.Load(L"data", &error), error.c_str());
  Require(graph.nativeConnections.size()==4271,"original native connection identities retained");
  Require(graph.nativeConnections.at((uint64_t(903)<<35)|(uint64_t(900)<<6)|1)==
          std::vector<uint32_t>{601089,601345},"one native rail connection retains both physical strips");
  std::vector<uint8_t> packed;
  Require(ReadFileBytes(L"data\\splines.bin",&packed),"read native link fixtures");
  auto word=[&](size_t offset){uint32_t value=0;std::memcpy(&value,packed.data()+offset,4);return value;};
  const size_t links=28ull+uint64_t(word(16))*12+uint64_t(word(20))*28+uint64_t(word(24))*8;
  wchar_t temp[MAX_PATH]{},unique[MAX_PATH]{};
  Require(GetTempPathW(MAX_PATH,temp) && GetTempFileNameW(temp,L"v3s",0,unique),"create spline test temp name");
  Require(DeleteFileW(unique) && CreateDirectoryW(unique,nullptr),"create spline test directory");
  const std::wstring file=std::wstring(unique)+L"\\splines.bin";
  auto rejected=[&](const std::vector<uint8_t>& bytes,const char* message){
    {std::ofstream out(std::filesystem::path(file),std::ios::binary);out.write((const char*)bytes.data(),bytes.size());Require(bool(out),"write spline fixture");}
    SplineGraph invalid;Require(!invalid.Load(unique,&error),message);
    Require(invalid.nativeConnections.empty() && invalid.edges.empty(),"invalid graph not partially published");
  };
  auto invalid=packed;invalid.pop_back();rejected(invalid,"truncated native mapping rejected");
  invalid=packed;invalid.push_back(0);rejected(invalid,"trailing native mapping bytes rejected");
  invalid=packed;uint32_t unknown=UINT32_MAX;std::memcpy(invalid.data()+links+16,&unknown,4);
  rejected(invalid,"unknown physical strip in native mapping rejected");
  invalid=packed;const size_t second=links+16+uint64_t(word(links+12))*4;
  std::memcpy(invalid.data()+second,packed.data()+links+4,8);
  rejected(invalid,"duplicate native connection key rejected");
  Require(DeleteFileW(file.c_str()) && RemoveDirectoryW(unique),"remove own spline test fixtures");
  std::vector<StateEntry> states; HeightField heights; ProvinceGraph provinces;
  Require(LoadRouteData(L"data", &states, &heights, &provinces, &error), error.c_str());
  auto state = [&](int id) { return *std::find_if(states.begin(), states.end(), [&](const auto& s) { return s.id == id; }); };
  auto a = graph.Route(state(15), state(16), heights);
  auto b = graph.Route(state(16), state(15), heights);
  Require(a.points.size() > 2 && !a.edgeIds.empty(), "UK authored path exists");
  Require(a.points.size() == b.points.size(), "reverse point count");
  Require(std::abs(a.distance - b.distance) < .01f, "reverse distance");
  for (size_t i = 0; i < a.points.size(); ++i) {
    const auto& p = a.points[i]; const auto& q = b.points[b.points.size()-1-i];
    Require(std::abs(p.x-q.x)<.01f && std::abs(p.z-q.z)<.01f, "reverse preserves spline shape");
  }
  const auto& city = graph.nodes[graph.byId.at(1500)];
  Require(std::abs(city.x - 4091.022f) < .01f && std::abs((graph.height - city.z) - 2801.467f) < .01f,
          "UK engine Z is flipped from image row");
  Require(graph.Route(state(15), state(1), heights, 3).points.empty(), "land-only UK route does not cross sea");
  auto sea = graph.Route(state(15), state(1), heights);
  Require(!sea.points.empty(), "verified port transfers connect UK to Sweden over authored naval graph");
  size_t transfers=0,naval=0;
  for (auto id:sea.edgeIds) for (auto& edge:graph.edges) if (edge.id==id) {
    if (edge.type==4) ++transfers;
    if (edge.type==2 || edge.type==3) ++naval;
  }
  Require(transfers>=2 && naval>0,"international route retains actual naval strips and two shore transfers");
  Require(graph.Route(state(15), state(16), heights, 0).points.empty(), "type filter excludes all edges");
  // A bent coastal corridor must beat a longer dirt route, keep the bend, and
  // disappear when coastal links are excluded. Same route supplies distance/paint.
  SplineGraph river;
  river.nodes = {{100,0,0},{200,10,0}}; river.byId = {{100,0},{200,1}};
  river.points = {{0,0},{5,5},{10,0},{0,0},{5,20},{10,0}};
  river.edges = {{3,0,1,3,0,3,14.142136f},{0,0,1,0,3,3,41.231056f}};
  river.adjacency = {{0,1},{0,1}};
  StateEntry s; s.id=1; StateEntry t; t.id=2;
  auto r = river.Route(s,t,heights);
  Require(r.edgeIds == std::vector<uint32_t>{3} && r.points[1].z == 5, "authored coastal river is routed and painted");
  auto land = river.Route(s,t,heights,1);
  Require(land.edgeIds == std::vector<uint32_t>{0} && land.distance > r.distance, "route cost and type mask agree");
  Require(river.Route(s,s,heights).points.empty(), "same endpoint rejected");
  std::printf("PASS: native spline routing, orientation, coastal corridor, filters, disconnection, Z transform; UK %zu edges / %zu points / %.2f units\n", a.edgeIds.size(), a.points.size(), a.distance);
}
