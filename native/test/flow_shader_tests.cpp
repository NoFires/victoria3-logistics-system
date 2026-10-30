// Exercise the actual embedded pixel shader on a WARP D3D11 device. Native
// CArrow supplies transverse UV endpoints 0 and 1, not 0 and 2. Clipping this
// domain in half moves a triangle's tip to the ribbon edge; this test catches it.
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include "../src/flow_width.h"

void Require(bool ok, const char* why) { if(!ok) {std::printf("FAIL: %s\n",why);std::exit(1);} }
void Check(HRESULT hr, const char* why) { Require(SUCCEEDED(hr),why); }
std::string Read(const char* path) {std::ifstream f(path,std::ios::binary);std::ostringstream o;o<<f.rdbuf();return o.str();}
ID3DBlob* Compile(const std::string& code, const char* target) {
  ID3DBlob *shader=nullptr,*error=nullptr;
  auto hr=D3DCompile(code.data(),code.size(),"building_flow_test",nullptr,nullptr,"main",target,D3DCOMPILE_ENABLE_STRICTNESS,0,&shader,&error);
  if(error) {std::printf("%.*s",(int)error->GetBufferSize(),(char*)error->GetBufferPointer());error->Release();}
  Check(hr,"D3DCompile actual flow shader");return shader;
}
void CheckSurfaceHeight(ID3D11Device* device,ID3D11DeviceContext* context,const std::string& source) {
  const auto begin=source.find("[[",source.find("VertexShader =")),end=source.find("]]",begin);
  Require(begin!=std::string::npos && end!=std::string::npos,"vertex Code block");
  auto body=source.substr(begin+2,end-begin-2);const auto entry=body.find("PDX_MAIN");
  Require(entry!=std::string::npos,"vertex main");body.replace(entry,8,"VS_OUTPUT main(VS_INPUT Input)");
  const std::string types=R"(
struct VS_INPUT {float3 vPosition:POSITION;float2 vTexCoord:TEXCOORD0;};
struct VS_OUTPUT {float4 vPosition:SV_POSITION;float2 vTexCoord:TEXCOORD0;};
)";
  auto vs=Compile(types+R"(
cbuffer Surface:register(b0) {float TerrainHeight;float _WaterHeight;float _FlatmapHeight;float _FlatmapLerp;};
float GetHeight(float2 position) {return TerrainHeight;}
#define FixProjectionAndMul(matrix,position) (position)
)"+body,"vs_5_0");
  auto gs=Compile(types+R"(
[maxvertexcount(1)] void main(point VS_OUTPUT input[1],inout PointStream<VS_OUTPUT> output) {output.Append(input[0]);}
)","gs_5_0");
  ID3D11VertexShader* vertex=nullptr;ID3D11GeometryShader* stream=nullptr;ID3D11InputLayout* layout=nullptr;
  Check(device->CreateVertexShader(vs->GetBufferPointer(),vs->GetBufferSize(),nullptr,&vertex),"surface vertex shader");
  const D3D11_SO_DECLARATION_ENTRY declaration[]={{0,"SV_POSITION",0,0,4,0},{0,"TEXCOORD",0,0,2,0}};
  UINT outputStride=24;
  Check(device->CreateGeometryShaderWithStreamOutput(gs->GetBufferPointer(),gs->GetBufferSize(),declaration,2,
      &outputStride,1,D3D11_SO_NO_RASTERIZED_STREAM,nullptr,&stream),"surface stream output");
  const D3D11_INPUT_ELEMENT_DESC input[]={{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
      {"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0}};
  Check(device->CreateInputLayout(input,2,vs->GetBufferPointer(),vs->GetBufferSize(),&layout),"surface input layout");
  vs->Release();gs->Release();
  auto buffer=[&](UINT bytes,UINT bind,UINT cpu,const void* data) {
    D3D11_BUFFER_DESC d{};d.ByteWidth=bytes;d.BindFlags=bind;d.CPUAccessFlags=cpu;
    d.Usage=cpu?D3D11_USAGE_STAGING:D3D11_USAGE_DEFAULT;
    D3D11_SUBRESOURCE_DATA initial{data,0,0};ID3D11Buffer* result=nullptr;
    Check(device->CreateBuffer(&d,data?&initial:nullptr,&result),"surface test buffer");return result;
  };
  const float point[]={12.5f,-99.f,37.25f,.2f,.8f};
  auto points=buffer(sizeof(point),D3D11_BIND_VERTEX_BUFFER,0,point);
  auto constants=buffer(16,D3D11_BIND_CONSTANT_BUFFER,0,nullptr);
  auto output=buffer(24,D3D11_BIND_STREAM_OUTPUT,0,nullptr),readback=buffer(24,0,D3D11_CPU_ACCESS_READ,nullptr);
  context->ClearState();context->VSSetShader(vertex,nullptr,0);context->GSSetShader(stream,nullptr,0);
  context->IASetInputLayout(layout);context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
  UINT stride=sizeof(point),offset=0;context->IASetVertexBuffers(0,1,&points,&stride,&offset);
  context->VSSetConstantBuffers(0,1,&constants);
  for(float water:{19.f,0.f,-5.f,75.f}) for(float terrain:{-20.f,0.f,19.f,90.f})
    for(float flat:{0.f,.25f,.5f,1.f}) {
      const float fixture[]={terrain,water,120.f,flat};context->UpdateSubresource(constants,0,nullptr,fixture,0,0);
      context->SOSetTargets(1,&output,&offset);context->Draw(1,0);context->SOSetTargets(0,nullptr,nullptr);
      context->CopyResource(readback,output);D3D11_MAPPED_SUBRESOURCE mapped{};
      Check(context->Map(readback,0,D3D11_MAP_READ,0,&mapped),"surface height readback");
      float value[6];std::memcpy(value,mapped.pData,sizeof(value));context->Unmap(readback,0);
      const float surface=std::max(terrain,water)*(1-flat)+120.f*flat;
      if(std::abs(value[1]-(surface+.35f))>=.0001f)
        std::printf("surface case water=%.2f terrain=%.2f flat=%.2f: ribbon=%.2f expected=%.2f\n",
            water,terrain,flat,value[1],surface+.35f);
      Require(std::abs(value[1]-(surface+.35f))<.0001f,
          "ribbon must stay above the terrain/water surface at every zoom");
      Require(value[0]==point[0] && value[2]==point[2] && value[3]==1 && value[4]==point[3] && value[5]==point[4],
          "surface placement preserves route coordinates and ribbon UVs");
    }
  context->ClearState();readback->Release();output->Release();constants->Release();points->Release();
  layout->Release();stream->Release();vertex->Release();
  std::puts("PASS: actual vertex shader stays above sea/shore/land across 64 water-height and flat-map transition cases");
}
int main(int argc,char** argv) {
  Require(argc==3,"usage: flow_shader_tests shader-file RGBA-texture-file");
  const auto source=Read(argv[1]);auto pixel=source.find("PixelShader =");
  auto begin=source.find("[[",pixel),end=source.find("]]",begin);
  Require(begin!=std::string::npos && end!=std::string::npos,"pixel Code block");
  auto body=source.substr(begin+2,end-begin-2);auto entry=body.find("PDX_MAIN");
  Require(entry!=std::string::npos,"pixel main");body.replace(entry,8,"float4 main(VS_OUTPUT Input): SV_TARGET");
  const std::string prefix=R"(
struct VS_OUTPUT {float4 vPosition:SV_POSITION;float2 vTexCoord:TEXCOORD0;};
cbuffer Arrow:register(b0) {float AnimationLength;float MoveLength;float ClipLength;float TotalLength;float Width;};
cbuffer Time:register(b2) {float GlobalTime;};
Texture2D DiffuseTexture:register(t0); Texture2D MaskTexture:register(t3);
SamplerState LinearSampler:register(s0);
#define PdxTex2D(t,uv) t.Sample(LinearSampler,uv)
#define PdxTex2DGrad(t,uv,dx,dy) t.SampleGrad(LinearSampler,uv,dx,dy)
)";
  const std::string vertex=R"(
struct VS_OUTPUT {float4 vPosition:SV_POSITION;float2 vTexCoord:TEXCOORD0;};
cbuffer Flip:register(b3) {uint Reverse;};
cbuffer Arrow:register(b0) {float AnimationLength;float MoveLength;float ClipLength;float TotalLength;float Width;};
VS_OUTPUT main(uint id:SV_VertexID) {
  float2 uv=float2((id<<1)&2,id&2);VS_OUTPUT o;
  o.vPosition=float4(uv.x*2-1,1-uv.y*2,0,1);
  o.vTexCoord=float2(uv.y,(Reverse ? 1-uv.x : uv.x)*50*Width);
  return o;
})";
  ID3D11Device* device=nullptr;ID3D11DeviceContext* context=nullptr;
  D3D_FEATURE_LEVEL level;
  Check(D3D11CreateDevice(nullptr,D3D_DRIVER_TYPE_WARP,nullptr,0,nullptr,0,D3D11_SDK_VERSION,&device,&level,&context),"WARP device");
  auto vs=Compile(vertex,"vs_5_0"),ps=Compile(prefix+body,"ps_5_0");
  ID3D11VertexShader* vertexShader=nullptr;ID3D11PixelShader* pixelShader=nullptr;
  Check(device->CreateVertexShader(vs->GetBufferPointer(),vs->GetBufferSize(),nullptr,&vertexShader),"vertex shader");
  Check(device->CreatePixelShader(ps->GetBufferPointer(),ps->GetBufferSize(),nullptr,&pixelShader),"pixel shader");
  vs->Release();ps->Release();
  auto raw=Read(argv[2]);Require(raw.size()>=8,"texture header");
  uint32_t w=0,h=0;std::memcpy(&w,raw.data(),4);std::memcpy(&h,raw.data()+4,4);
  Require(raw.size()==8ull+w*h*4ull,"RGBA texture size");
  ID3D11Texture2D *texture=nullptr,*mask=nullptr,*target=nullptr,*staging=nullptr;
  D3D11_TEXTURE2D_DESC desc{};desc.Width=w;desc.Height=h;desc.MipLevels=desc.ArraySize=1;desc.Format=DXGI_FORMAT_R8G8B8A8_UNORM;desc.SampleDesc.Count=1;desc.BindFlags=D3D11_BIND_SHADER_RESOURCE;
  D3D11_SUBRESOURCE_DATA input{raw.data()+8,w*4,0};
  Check(device->CreateTexture2D(&desc,&input,&texture),"building arrow texture");
  desc.Width=desc.Height=1;uint32_t white=0xffffffff;input={&white,4,0};
  Check(device->CreateTexture2D(&desc,&input,&mask),"white mask fixture");
  ID3D11ShaderResourceView *textureView=nullptr,*maskView=nullptr;
  Check(device->CreateShaderResourceView(texture,nullptr,&textureView),"arrow SRV");
  Check(device->CreateShaderResourceView(mask,nullptr,&maskView),"mask SRV");
  desc.Width=800;desc.Height=128;desc.Format=DXGI_FORMAT_R32G32B32A32_FLOAT;desc.BindFlags=D3D11_BIND_RENDER_TARGET;
  Check(device->CreateTexture2D(&desc,nullptr,&target),"offscreen target");
  desc.BindFlags=0;desc.Usage=D3D11_USAGE_STAGING;desc.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
  Check(device->CreateTexture2D(&desc,nullptr,&staging),"readback");
  ID3D11RenderTargetView* rtv=nullptr;Check(device->CreateRenderTargetView(target,nullptr,&rtv),"RTV");
  ID3D11SamplerState* sampler=nullptr;D3D11_SAMPLER_DESC sd{};sd.Filter=D3D11_FILTER_MIN_MAG_MIP_LINEAR;sd.AddressU=D3D11_TEXTURE_ADDRESS_WRAP;sd.AddressV=sd.AddressW=D3D11_TEXTURE_ADDRESS_CLAMP;sd.MaxLOD=D3D11_FLOAT32_MAX;
  Check(device->CreateSamplerState(&sd,&sampler),"sampler");
  auto constant=[&](UINT bytes,const void* data) {D3D11_BUFFER_DESC d{};d.ByteWidth=bytes;d.Usage=D3D11_USAGE_DEFAULT;d.BindFlags=D3D11_BIND_CONSTANT_BUFFER;D3D11_SUBRESOURCE_DATA s{data,0,0};ID3D11Buffer* b=nullptr;Check(device->CreateBuffer(&d,&s,&b),"constant buffer");return b;};
  float arrowData[8]={0,1.e9f,0,50,1,0,0,0},time[4]={};uint32_t flip[4]={};
  auto arrowBuffer=constant(32,arrowData),timeBuffer=constant(16,time),flipBuffer=constant(16,flip);
  context->PSSetConstantBuffers(0,1,&arrowBuffer);context->PSSetConstantBuffers(2,1,&timeBuffer);context->VSSetConstantBuffers(3,1,&flipBuffer);
  context->VSSetConstantBuffers(0,1,&arrowBuffer);
  context->PSSetShaderResources(0,1,&textureView);context->PSSetShaderResources(3,1,&maskView);context->PSSetSamplers(0,1,&sampler);
  context->VSSetShader(vertexShader,nullptr,0);context->PSSetShader(pixelShader,nullptr,0);context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->OMSetRenderTargets(1,&rtv,nullptr);D3D11_VIEWPORT viewport{0,0,800,128,0,1};context->RSSetViewports(1,&viewport);
  for(float width:v3::kFlowWidths) for(uint32_t reverse=0;reverse<2;++reverse) {
    arrowData[4]=width;arrowData[3]=50*width;context->UpdateSubresource(arrowBuffer,0,nullptr,arrowData,0,0);
    flip[0]=reverse;context->UpdateSubresource(flipBuffer,0,nullptr,flip,0,0);context->Draw(3,0);
    context->CopyResource(staging,target);D3D11_MAPPED_SUBRESOURCE mapped{};Check(context->Map(staging,0,D3D11_MAP_READ,0,&mapped),"map GPU output");
    int tip=reverse?200:-1;double sumY=0;int count=0;
    for(int x=0;x<200;++x) for(int y=0;y<128;++y) {
      auto p=(const float*)((const char*)mapped.pData+y*mapped.RowPitch)+x*4;
      if(p[1]>.48f && p[3]>.9f && (reverse ? x<tip : x>tip)) tip=x;
    }
    Require(tip>=0 && tip<200,"arrowhead visible");
    for(int x=std::max(0,tip-1);x<=std::min(199,tip+1);++x) for(int y=0;y<128;++y) {
      auto p=(const float*)((const char*)mapped.pData+y*mapped.RowPitch)+x*4;
      if(p[1]>.48f && p[3]>.9f) {sumY+=y;++count;}
    }
    context->Unmap(staging,0);Require(count>0,"tip samples");
    double centre=sumY/count;std::printf("width %.2f %s arrow tip x=%d, transverse centre=%.2f (expected 63.5)\n",width,reverse?"reverse":"forward",tip,centre);
    Require(std::abs(centre-63.5)<4,"triangle tip must lie on ribbon centre, not a clipped side");
    Require(reverse ? (tip>60 && tip<100) : (tip>110 && tip<140),"triangle points along route direction");
  }
  struct Palette {const char* define;float red,green,blue;};
  const Palette palettes[]={
    {"OUTPUT",.06426f,.51f,.0255f},{"INPUT",.91f,.14924f,.0455f},
    {"YELLOW",.91f,.78f,.0455f},{"BLUE",.0455f,.35f,.91f},
    {"TRAFFIC_GREEN",.08f,.85f,.35f},{"TRAFFIC_LIME",.55f,.9f,.12f},
    {"TRAFFIC_YELLOW",1.f,.9f,.05f},{"TRAFFIC_ORANGE",1.f,.53f,.03f},{"TRAFFIC_RED",.95f,.04f,.12f}};
  arrowData[4]=1;arrowData[3]=50;flip[0]=0;
  context->UpdateSubresource(arrowBuffer,0,nullptr,arrowData,0,0);
  context->UpdateSubresource(flipBuffer,0,nullptr,flip,0,0);
  for(const auto& palette:palettes) for(unsigned balanced=0;balanced<2;++balanced) {
    if(balanced && (std::string(palette.define)=="YELLOW" || std::string(palette.define)=="BLUE"))continue;
    auto compiled=Compile(prefix+"\n#define IRONROAD_"+palette.define+"\n"+(balanced?"#define IRONROAD_BALANCED\n":"")+body,"ps_5_0");
    ID3D11PixelShader* coloured=nullptr;
    Check(device->CreatePixelShader(compiled->GetBufferPointer(),compiled->GetBufferSize(),nullptr,&coloured),"palette pixel shader");
    compiled->Release();context->PSSetShader(coloured,nullptr,0);context->Draw(3,0);
    context->CopyResource(staging,target);D3D11_MAPPED_SUBRESOURCE mapped{};
    Check(context->Map(staging,0,D3D11_MAP_READ,0,&mapped),"palette readback");
    float peak[3]={};
    for(int y=0;y<128;++y) for(int x=0;x<800;++x) {
      const auto* pixel=(const float*)((const char*)mapped.pData+y*mapped.RowPitch)+x*4;
      if(pixel[3]>.99f) for(int c=0;c<3;++c) peak[c]=std::max(peak[c],pixel[c]);
    }
    context->Unmap(staging,0);
    Require(std::abs(peak[0]-palette.red)<.03f && std::abs(peak[1]-palette.green)<.03f &&
            std::abs(peak[2]-palette.blue)<.03f,"actual GPU palette must match expected colour");
    coloured->Release();
  }
  auto balancedCode=Compile(prefix+"\n#define IRONROAD_BALANCED\n"+body,"ps_5_0");
  ID3D11PixelShader* balancedShader=nullptr;
  Check(device->CreatePixelShader(balancedCode->GetBufferPointer(),balancedCode->GetBufferSize(),nullptr,&balancedShader),"balanced shader");
  balancedCode->Release();
  auto image=[&](ID3D11PixelShader* shader,uint32_t reverse){
    flip[0]=reverse;context->UpdateSubresource(flipBuffer,0,nullptr,flip,0,0);
    context->PSSetShader(shader,nullptr,0);context->Draw(3,0);context->CopyResource(staging,target);
    D3D11_MAPPED_SUBRESOURCE mapped{};Check(context->Map(staging,0,D3D11_MAP_READ,0,&mapped),"balanced readback");
    std::vector<float> pixels(800*128*4);
    for(int y=0;y<128;++y)std::memcpy(pixels.data()+y*800*4,(const char*)mapped.pData+y*mapped.RowPitch,800*4*sizeof(float));
    context->Unmap(staging,0);return pixels;
  };
  const auto forward=image(pixelShader,0),reverse=image(pixelShader,1),both=image(balancedShader,0);
  size_t forwardOnly=0,reverseOnly=0;
  for(size_t i=0;i<both.size();++i){
    Require(std::abs(both[i]-std::max(forward[i],reverse[i]))<.03f,"balanced flow must display both directions on one ribbon");
    forwardOnly+=both[i]>reverse[i]+.1f;reverseOnly+=both[i]>forward[i]+.1f;
  }
  Require(forwardOnly>100 && reverseOnly>100,"balanced flow must retain distinct opposing arrowheads");
  balancedShader->Release();
  std::puts("PASS: equal opposing flow renders both directions on one physical ribbon without doubled brightness");
  std::puts("PASS: all eight consumption widths preserve arrowhead proportions/direction; nine normal and seven balanced GPU palettes verified");
  std::puts("PASS: actual building-flow pixel shader renders centred, forward and reverse arrowheads with native 0..1 ribbon UVs");
  CheckSurfaceHeight(device,context,source);
  context->ClearState();flipBuffer->Release();timeBuffer->Release();arrowBuffer->Release();sampler->Release();rtv->Release();staging->Release();target->Release();maskView->Release();textureView->Release();mask->Release();texture->Release();pixelShader->Release();vertexShader->Release();context->Release();device->Release();
}
