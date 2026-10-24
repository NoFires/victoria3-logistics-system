#include "arrow_geometry.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <limits>

void Check(bool ok, const char* label) {
  if (!ok) { std::printf("FAIL: %s\n", label); std::exit(1); }
}
float Cubic(float a, float b, float c, float d, float t) {
  return .5f * (2*b + (-a+c)*t + (2*a-5*b+4*c-d)*t*t + (-a+3*b-3*c+d)*t*t*t);
}
int main() {
  using namespace v3;
  std::vector<Vec3> source{{10,3,20},{11,4,21},{25,5,30}};
  auto path = NativeArrowPoints(source, 100, 1, 2, .5f, true);
  Check(path.size()==5, "three route points need five Catmull-Rom controls");
  Check(Cubic(path[0].x,path[1].x,path[2].x,path[3].x,0)==10,
      "native curve begins at requested source");
  Check(Cubic(path[1].x,path[2].x,path[3].x,path[4].x,1)==25,
      "native curve reaches requested destination");
  Check(path[1].z==80 && path[3].z==70 && path[2].y==8.5f,
      "map orientation and terrain calibration retained");
  std::reverse(source.begin(),source.end());
  auto reverse = NativeArrowPoints(source, 100, 1, 2, .5f, true);
  for(size_t i=0;i<path.size();++i) {
    auto& p=path[i]; auto& q=reverse[path.size()-1-i];
    Check(p.x==q.x && p.y==q.y && p.z==q.z,"reversing endpoints reverses full native curve");
  }
  auto shortPath = NativeArrowPoints({{1,0,1},{2,0,2}},100,1,0,0,true);
  Check(shortPath.size()==4,"two point route still creates one native cubic segment");
  Check(NativeArrowPoints(std::vector<Vec3>(4094),100,1,0,0,true).size()==4096,"allocation limit includes endpoint controls");
  Check(NativeArrowPoints(std::vector<Vec3>(4095),100,1,0,0,true).empty(),"oversize route rejected");
  Check(NativeArrowPoints({{1,0,0},{std::numeric_limits<float>::infinity(),0,0}},100,1,0,0,true).empty(),"invalid geometry rejected");
  std::puts("PASS: native cubic endpoints, two-point routes, direction reversal, map transform, allocation bounds");
}
