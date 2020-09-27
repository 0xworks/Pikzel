#pragma once

#include <filesystem>
#include <fstream>
#include <vector>

namespace Pikzel {

   std::vector<char> ReadFile(const std::filesystem::path& path, const bool readAsBinary);

   template<typename T>
   std::vector<T> ReadFile(const std::filesystem::path& path) {
      std::ifstream file(path, std::ios::ate | std::ios::binary);
      if (!file.is_open()) {
         throw std::runtime_error(fmt::format("File '{0}' could not be accessed!", path.string()));
      }

      std::streampos fileSize = file.tellg();
      if (fileSize % sizeof(T) != 0) {
         throw std::runtime_error(fmt::format("Size of file '{0}' is {1}.  Expected a multiple of {2}!", path.string(), fileSize, sizeof(T)));
      }
      file.seekg(0);

      std::vector<T> buffer(fileSize / sizeof(T));
      file.read(reinterpret_cast<char*>(buffer.data()), fileSize);

      return buffer;
   }

}
