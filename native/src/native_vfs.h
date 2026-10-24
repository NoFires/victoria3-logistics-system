#pragma once
#include <array>
#include <cstdint>
#include <utility>
#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

namespace v3 {
enum class NativeVfsKind { Directory, Zip };
// Owned strings in native search order. No engine address leaves the decoder.
struct NativeVfsMount {
  NativeVfsKind kind=NativeVfsKind::Directory;
  std::string root,mountPoint,sourcePrefix;
};
struct NativeVfsReplacement {std::string root,path;};
struct NativeVfsConfiguration {
  std::vector<NativeVfsMount> mounts;
  std::vector<std::string> searchPaths;
  std::vector<NativeVfsReplacement> replacements;
  std::vector<std::string> exclusions;
};
// Enumeration at 03B4AF80: exclusion is a literal byte prefix; replacement
// matches this directory exactly and keeps the owning root and higher roots.
// Recursive traversal and physical directory/ZIP resolution are separate.
inline size_t NativeVfsDirectoryRootCount(const NativeVfsConfiguration& configuration,const std::string& directory) {
  size_t count=configuration.searchPaths.size();
  for(const auto& path:configuration.exclusions)
    if(directory.compare(0,path.size(),path)==0)return 0;
  for(const auto& rule:configuration.replacements)if(directory==rule.path)
    for(size_t i=0;i<configuration.searchPaths.size();++i)if(rule.root==configuration.searchPaths[i]){
      if(i+1<count)count=i+1;break;
    }
  return count;
}

// 1.13.11 only: caller verifies the executable and supplies its current TLS
// filesystem context index. This copies frozen configuration, never calls VFS.
// Mounts and replace/exclude rules are copied together. This does not yet
// resolve effective files. Directory enumeration policy uses only these copies.
template<class Reader,class Current>
std::optional<NativeVfsConfiguration> DecodeNativeVfsConfiguration(
    const Reader& reader,uint64_t module,uint32_t context,const Current& current,
    std::string* error=nullptr) {
  auto fail=[&](const char* reason)->std::optional<NativeVfsConfiguration>{
    if(error)*error=reason;return std::nullopt;
  };
  if(!current() || !module || context>=16 || module>UINT64_MAX-0x05145900)
    return fail("invalid filesystem context");
  struct Copy {uint64_t address;std::vector<uint8_t> bytes;};
  std::vector<Copy> copies;size_t stringBytes=0;
  auto read=[&](uint64_t address,void* out,size_t size){
    if(!address || !size || size>UINT64_MAX-address || !reader(address,out,size))return false;
    copies.push_back({address,{(uint8_t*)out,(uint8_t*)out+size}});return true;
  };
  auto field=[&](uint64_t object,uint64_t offset,auto& value){
    return object && offset<=UINT64_MAX-object && read(object+offset,&value,sizeof(value));
  };
  auto string=[&](uint64_t address,std::string& value){
    value.clear();if(!address)return true;
    for(size_t i=0;i<4096 && stringBytes<262144;++i){
      char c=0;if(i>UINT64_MAX-address || !reader(address+i,&c,1))return false;
      ++stringBytes;
      if(!c){
        std::vector<uint8_t> bytes(value.begin(),value.end());bytes.push_back(0);
        copies.push_back({address,std::move(bytes)});return true;
      }
      value+=c;
    }
    return false;
  };
  const uint64_t fs=module+0x05144588+uint64_t(context)*0x128;
  uint8_t ready=0,frozen=0;uint64_t raw=0,paths=0,node=0;
  if(!field(fs,0x120,ready) || ready!=1 || !field(fs,0xb8,raw) ||
     !field(raw,0xfa,frozen) || frozen!=1 || !field(raw,8,node) || !node ||
     !field(fs,0x28,paths) || !paths)return fail("filesystem configuration not frozen");
  NativeVfsConfiguration configuration;auto& mounts=configuration.mounts;
  std::unordered_set<uint64_t> seen;
  while(node){
    if(mounts.size()>=512 || !seen.insert(node).second)return fail("invalid filesystem mount chain");
    std::array<uint64_t,7> item{};
    if(!read(node,item.data(),sizeof(item)) || !item[0] || !item[1] || !item[5])
      return fail("unreadable filesystem mount");
    NativeVfsMount mount;
    if(!string(item[1],mount.root) || mount.root.empty() ||
       !string(item[2],mount.mountPoint) || !string(item[3],mount.sourcePrefix) ||
       (item[3] ? item[4]!=mount.sourcePrefix.size()+1 : item[4]!=0) ||
       (!mount.mountPoint.empty() && mount.mountPoint.back()!='/'))
      return fail("invalid filesystem mount strings");
    uint64_t enumerate=0,open=0,stat=0;
    if(!field(item[5],0x38,enumerate) || !field(item[5],0x40,open) || !field(item[5],0x68,stat))
      return fail("unreadable filesystem backend");
    if(enumerate==module+0x03b33d80 && open==module+0x03b33fe0 && stat==module+0x03b34220)
      mount.kind=NativeVfsKind::Directory;
    else if(enumerate==module+0x03b40a50 && open==module+0x03b40cc0 && stat==module+0x03b410e0)
      mount.kind=NativeVfsKind::Zip;
    else return fail("unverified filesystem backend");
    mounts.push_back(std::move(mount));node=item[6];
  }
  auto normalized=[](std::string value){for(char& c:value)if(c=='\\')c='/';return value;};
  for(size_t i=0;i<=mounts.size();++i){
    uint64_t path=0;
    if(!field(paths,uint64_t(i)*8,path))return fail("unreadable filesystem search paths");
    if(i==mounts.size()){
      if(path)return fail("filesystem search path count changed");
    }else{
      std::string value;
      if(!path || !string(path,value) || normalized(value)!=normalized(mounts[i].root))
        return fail("filesystem search order mismatch");
      configuration.searchPaths.push_back(std::move(value));
    }
  }
  auto nativeString=[&](uint64_t address,std::string& value){
    std::array<uint64_t,4> object{};
    if(!read(address,object.data(),sizeof(object)) || object[2]>4095 ||
       object[3]<15 || object[3]>1048576 || object[2]>object[3])return false;
    const uint64_t text=object[3]>15?object[0]:address;
    return text && string(text,value) && value.size()==object[2];
  };
  // Native CPdxArray headers: pointer/capacity/count. Replacement entries
  // contain two 32-byte strings (root, path); exclusions contain one.
  for(unsigned table=0;table<2;++table){
    uint64_t data=0;uint32_t capacity=0,count=0;
    const uint64_t offset=table?0x60:0x48,stride=table?0x20:0x40;
    if(!field(fs,offset,data) || !field(fs,offset+8,capacity) || !field(fs,offset+12,count) ||
       count>4096 || capacity>1048576 || count>capacity || (capacity && !data) ||
       uint64_t(count)*stride>UINT64_MAX-data)return fail("invalid filesystem rule table");
    for(uint32_t i=0;i<count;++i){
      const uint64_t entry=data+uint64_t(i)*stride;
      if(table){
        std::string path;if(!nativeString(entry,path))return fail("invalid filesystem exclusion string");
        configuration.exclusions.push_back(std::move(path));
      }else{
        NativeVfsReplacement rule;
        if(!nativeString(entry,rule.root) || !nativeString(entry+0x20,rule.path))
          return fail("invalid filesystem replacement strings");
        configuration.replacements.push_back(std::move(rule));
      }
    }
  }
  for(const auto& copy:copies){
    std::vector<uint8_t> bytes(copy.bytes.size());
    if(!reader(copy.address,bytes.data(),bytes.size()) || bytes!=copy.bytes)
      return fail("filesystem changed during copy");
  }
  if(!current())return fail("retired filesystem capture");
  if(error)error->clear();return configuration;
}
}  // namespace v3
