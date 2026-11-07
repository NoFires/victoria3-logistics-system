#pragma once
#include <dxgiformat.h>
#include <array>
#include <optional>

namespace v3 {
inline std::optional<std::array<unsigned,3>> ReadbackRgbChannels(DXGI_FORMAT format) {
  switch(format){
    case DXGI_FORMAT_R8G8B8A8_UNORM:
    case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:return std::array<unsigned,3>{0,1,2};
    case DXGI_FORMAT_B8G8R8A8_UNORM:
    case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:return std::array<unsigned,3>{2,1,0};
    default:return {};
  }
}
} // namespace v3
