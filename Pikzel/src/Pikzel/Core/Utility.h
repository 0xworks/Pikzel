#pragma once

#include <filesystem>
#include <fstream>
#include <vector>

namespace Pikzel {

   std::vector<char> ReadFile(const std::filesystem::path& path, const bool readAsBinary);

   template<typename T>
   std::vector<T> ReadFile(const std::filesystem::path& path) {

      std::ifstream file(path, std::ios::ate | std::ios::binary);

      std::streampos fileSize = file.tellg();
      file.seekg(0);

      std::vector<T> buffer(fileSize / sizeof(T));

      file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

      return buffer;
   }

}
