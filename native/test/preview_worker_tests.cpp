#include "preview_worker.h"
#include "flow_width.h"
#include <cstdio>
#include <cstdlib>
using namespace v3;
void Check(bool ok,const char* label) {
  if (!ok) { std::printf("FAIL: %s\n",label); std::exit(1); }
}
template<class Worker> auto Await(Worker& worker, uint64_t generation) {
  const auto deadline=GetTickCount64()+5000;
  auto reply=worker.Poll();
  while ((!reply || reply->generation!=generation) && GetTickCount64()<deadline) {
    Sleep(1); reply=worker.Poll();
  }
  Check(reply && reply->generation==generation,"worker completed requested generation");
  return reply;
}
int main() {
  SettleGate gate;
  Check(gate.Remaining(0)==INFINITE,"idle worker has no busy loop");
  gate.Change(0); Check(gate.Remaining(99)==1,"quiet request settles at 100ms");
  Check(gate.Remaining(100)==0,"quiet request ready");
  gate={}; gate.Change(0); gate.Change(90); gate.Change(180); gate.Change(240);
  Check(gate.Remaining(249)==1 && gate.Remaining(250)==0,"continuous churn cannot exceed 250ms max wait");

  HANDLE began=CreateEventW(nullptr,TRUE,FALSE,nullptr);
  HANDLE release=CreateEventW(nullptr,TRUE,FALSE,nullptr);
  std::atomic<unsigned> calls=0;
  DWORD computeThread=0;
  LatestWorker<int,int> mailbox([&](const int& input,const auto& cancelled) {
    computeThread=GetCurrentThreadId(); ++calls;
    if (input==4) throw std::runtime_error("test compute failure");
    if (input==1) {
      SetEvent(began);
      while (!cancelled()) {
        if (WaitForSingleObject(release,1)==WAIT_OBJECT_0) break;
      }
    }
    return input*10;
  });
  mailbox.Submit(1);
  Check(WaitForSingleObject(began,5000)==WAIT_OBJECT_0,"first calculation entered");
  Check(!mailbox.Poll(),"render consumer can poll while calculation is in progress");
  auto latest=mailbox.Submit(2);
  auto reply=Await(mailbox,latest);
  Check(reply->value==20 && calls==2 && computeThread!=GetCurrentThreadId(),"superseded compute cancels and only worker publishes newest result");
  mailbox.Cancel(); Check(!mailbox.Poll(),"clear synchronously removes completed reply");
  ResetEvent(began); mailbox.Submit(1);
  Check(WaitForSingleObject(began,5000)==WAIT_OBJECT_0,"second blocked calculation entered");
  mailbox.Cancel(); SetEvent(release);
  latest=mailbox.Submit(3); reply=Await(mailbox,latest);
  Check(reply->value==30,"close/clear cannot resurrect blocked old result");
  latest=mailbox.Submit(4); reply=Await(mailbox,latest);
  Check(reply->error=="test compute failure","worker exception publishes a diagnostic");
  latest=mailbox.Submit(5); reply=Await(mailbox,latest);
  Check(reply->value==50 && reply->error.empty(),"worker recovers after a calculation exception");
  mailbox.Stop(); CloseHandle(began); CloseHandle(release);

  auto geo=std::make_shared<RouteGeography>();
  std::string error;
  Check(geo->splines.Load(L"data",&error),error.c_str());
  Check(LoadRouteData(L"data",&geo->states,&geo->heights,&geo->provinces,&error),error.c_str());
  PreviewWorker worker(geo);
  PreviewRequest request; request.from=15; request.to=16; request.style="ironroadBuildingOutput";
  auto first=Await(worker,worker.Request(request));
  Check(first->value.error.empty() && first->value.ribbons.size()==1 && !first->value.routeCacheHit,"real authored UK route built asynchronously");
  const auto& points=first->value.ribbons[0]->points;
  Check(points.size()>4 && points[0].z>0,"native controls produced by worker");
  auto again=Await(worker,worker.Request(request));
  Check(again->value.routeCacheHit && SameRibbon(*first->value.ribbons[0],*again->value.ribbons[0]),"unchanged route uses routing cache and exact native identity");
  request.style="ironroadBuildingInput"; request.heightOffset=2;
  auto styled=Await(worker,worker.Request(request));
  Check(styled->value.routeCacheHit && styled->value.ribbons[0]->points[0].y==points[0].y+2 &&
      styled->value.ribbons[0]->style==FlowWidthStyle(request.style,request.previewConsumption),"style/calibration reuse route search but produce correct native geometry");
  request.previewConsumption=5000;
  auto wide=Await(worker,worker.Request(request));
  Check(wide->value.routeCacheHit && wide->value.ribbons[0]->style!=styled->value.ribbons[0]->style &&
      wide->value.ribbons[0]->points.size()==styled->value.ribbons[0]->points.size(),
      "consumption changes native width variant without rerouting");
  request.from=16; request.to=15; request.heightOffset=0;
  auto reversed=Await(worker,worker.Request(request));
  const auto& backward=reversed->value.ribbons[0]->points;
  Check(backward.front().x==points.back().x && backward.back().z==points.front().z,"reverse request retains source-destination orientation");
  request.from=-999;
  auto invalid=Await(worker,worker.Request(request));
  Check(!invalid->value.error.empty() && invalid->value.ribbons.empty(),"unknown stable ID never touches engine");
  Check(geo->splines.Route(geo->states[0],geo->states[1],geo->heights,31,[]{return true;}).points.empty(),"authored search accepts cancellation");
  std::puts("PASS: settle/max-wait, nonblocking handoff, newest generation/cancel/clear, real-map background routing and style-independent route cache");
}
