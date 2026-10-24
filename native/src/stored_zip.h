#pragma once
#include <cstdint>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace v3 {
struct ArchiveFile { std::string name; std::vector<uint8_t> bytes; };
inline std::vector<uint8_t> BuildStoredZip(const std::vector<ArchiveFile>& files) {
  if (files.empty() || files.size()>128) throw std::runtime_error("invalid managed ZIP entry count");
  std::vector<uint8_t> out,central;
  auto put16=[](auto& v,uint16_t n){v.push_back(uint8_t(n));v.push_back(uint8_t(n>>8));};
  auto put32=[](auto& v,uint32_t n){for(unsigned i=0;i<4;++i)v.push_back(uint8_t(n>>(i*8)));};
  auto crc=[](const auto& bytes){
    uint32_t c=0xffffffffu;
    for(uint8_t b:bytes){c^=b;for(unsigned bit=0;bit<8;++bit)c=(c>>1)^(0xedb88320u&(0u-(c&1)));}
    return ~c;
  };
  std::unordered_set<std::string> names;
  for(const auto& file:files) {
    if(file.name.empty() || file.name.size()>4096 || file.name.front()=='/' ||
        file.name.find("..")!=std::string::npos || file.name.find_first_of("\\:\0",0,3)!=std::string::npos ||
        !names.insert(file.name).second || file.bytes.size()>(16u<<20) ||
        out.size()+file.bytes.size()>(64u<<20)) throw std::runtime_error("invalid managed ZIP path/size");
    const auto offset=uint32_t(out.size()),size=uint32_t(file.bytes.size()),checksum=crc(file.bytes);
    put32(out,0x04034b50);put16(out,20);put16(out,0x800);put16(out,0);
    put16(out,0);put16(out,0x5c21);put32(out,checksum);put32(out,size);put32(out,size);
    put16(out,uint16_t(file.name.size()));put16(out,0);
    out.insert(out.end(),file.name.begin(),file.name.end());out.insert(out.end(),file.bytes.begin(),file.bytes.end());
    put32(central,0x02014b50);put16(central,20);put16(central,20);put16(central,0x800);put16(central,0);
    put16(central,0);put16(central,0x5c21);put32(central,checksum);put32(central,size);put32(central,size);
    put16(central,uint16_t(file.name.size()));put16(central,0);put16(central,0);put16(central,0);put16(central,0);
    put32(central,0);put32(central,offset);central.insert(central.end(),file.name.begin(),file.name.end());
  }
  const auto offset=uint32_t(out.size());out.insert(out.end(),central.begin(),central.end());
  put32(out,0x06054b50);put16(out,0);put16(out,0);put16(out,uint16_t(files.size()));put16(out,uint16_t(files.size()));
  put32(out,uint32_t(central.size()));put32(out,offset);put16(out,0);
  return out;
}
} // namespace v3
