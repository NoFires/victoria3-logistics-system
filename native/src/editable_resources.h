#pragma once
#include "stored_zip.h"
#include "util.h"
#include <filesystem>

namespace v3 {
inline void WriteResourceFile(const std::wstring& path,const std::vector<uint8_t>& bytes) {
  std::filesystem::create_directories(std::filesystem::path(path).parent_path());
  const auto temporary=path+L".tmp-"+std::to_wstring(GetCurrentProcessId());
  const auto file=CreateFileW(temporary.c_str(),GENERIC_WRITE,0,nullptr,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,nullptr);
  DWORD written=0;
  bool ok=file!=INVALID_HANDLE_VALUE && WriteFile(file,bytes.data(),DWORD(bytes.size()),&written,nullptr) && written==bytes.size();
  if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);
  if(ok)ok=MoveFileExW(temporary.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
  if(!ok){DeleteFileW(temporary.c_str());throw std::runtime_error("cannot write freight resource: "+Narrow(path));}
}
// .defaults records the last shipped bytes. Upgrade untouched files only;
// player edits win, while new defaults remain available for comparison/reset.
inline std::vector<ArchiveFile> EditableResources(const std::wstring& root,const std::vector<ArchiveFile>& defaults) {
  BuildStoredZip(defaults); // validate all names/sizes before any file is written
  auto read=[](const std::wstring& path,std::vector<uint8_t>& bytes){
    if(!std::filesystem::exists(path))return false;
    if(std::filesystem::file_size(path)>(16u<<20) || !ReadFileBytes(path,&bytes))
      throw std::runtime_error("cannot read freight resource: "+Narrow(path));
    return true;
  };
  std::vector<ArchiveFile> result;
  for(const auto& file:defaults){
    const auto path=root+L"\\"+Widen(file.name),baseline=root+L"\\.defaults\\"+Widen(file.name);
    std::vector<uint8_t> bytes,old;const bool exists=read(path,bytes),haveDefault=read(baseline,old);
    if(!exists || (haveDefault && bytes==old)){
      if(!exists || bytes!=file.bytes)WriteResourceFile(path,file.bytes);
      bytes=file.bytes;
    }
    if(!haveDefault || old!=file.bytes)WriteResourceFile(baseline,file.bytes);
    result.push_back({file.name,std::move(bytes)});
  }
  return result;
}
} // namespace v3
