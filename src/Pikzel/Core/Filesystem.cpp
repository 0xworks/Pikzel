#include "Pikzel/Core/Filesystem.h"

#include <format>
#include <fstream>
#include <stdexcept>
#include <string>

#if defined(_WIN32)
   #include <windows.h>
#endif

namespace Pikzel::Filesystem {

std::filesystem::path GetExecutableDirectory() {
#if defined(_WIN32)
   // GetModuleFileNameW truncates silently if the buffer is too small, so grow it until
   // the returned length is less than the buffer size.
   std::wstring buffer(MAX_PATH, L'\0');
   while (true) {
      const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
      if (length == 0) {
         throw std::runtime_error("GetModuleFileNameW failed");
      }
      if (length < buffer.size()) {
         buffer.resize(length);
         break;
      }
      buffer.resize(buffer.size() * 2);
   }
   return std::filesystem::path(buffer).parent_path();
#elif defined(__linux__)
   return std::filesystem::canonical("/proc/self/exe").parent_path();
#else
   #error "GetExecutableDirectory() is not implemented for this platform"
#endif
}


std::vector<std::byte> ReadBinaryFile(const std::filesystem::path& path) {
   std::ifstream file(path, std::ios::binary | std::ios::ate);
   if (!file) {
      throw std::runtime_error(std::format("Failed to open file '{}'", path.string()));
   }

   const auto size = static_cast<std::size_t>(file.tellg());
   std::vector<std::byte> data(size);
   file.seekg(0);
   if (!file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size))) {
      throw std::runtime_error(std::format("Failed to read file '{}'", path.string()));
   }
   return data;
}

}
