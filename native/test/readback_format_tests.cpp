#include "readback_format.h"
#include <cstdio>
#include <cstdlib>

static void Check(bool value,const char* reason){if(!value){std::fprintf(stderr,"FAIL %s\n",reason);std::exit(1);}}
int main(){
  using namespace v3;
  const std::array<unsigned char,4> rgba{140,10,13,99},bgra{13,10,140,0};
  for(auto format:{DXGI_FORMAT_R8G8B8A8_UNORM,DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
      DXGI_FORMAT_B8G8R8A8_UNORM,DXGI_FORMAT_B8G8R8A8_UNORM_SRGB}){
    const auto channels=ReadbackRgbChannels(format);Check(bool(channels),"known byte layout rejected");
    const auto& bytes=(format==DXGI_FORMAT_R8G8B8A8_UNORM || format==DXGI_FORMAT_R8G8B8A8_UNORM_SRGB)?rgba:bgra;
    const std::array<unsigned char,3> rgb{bytes[(*channels)[0]],bytes[(*channels)[1]],bytes[(*channels)[2]]};
    Check(rgb==std::array<unsigned char,3>{140,10,13},"readback must preserve encoded RGB without alpha or gamma changes");
  }
  for(auto format:{DXGI_FORMAT_UNKNOWN,DXGI_FORMAT_R16G16B16A16_FLOAT,DXGI_FORMAT_R10G10B10A2_UNORM,
      DXGI_FORMAT_R8_UNORM,DXGI_FORMAT_BC1_UNORM,DXGI_FORMAT_FORCE_UINT})
    Check(!ReadbackRgbChannels(format),"unknown/packed/float/compressed data cannot use the four-byte RGB reader");
  std::puts("PASS readback byte formats: RGBA/BGRA and sRGB preserve encoded colour; unsafe layouts refused");
}
