#include "pch.h"
#include "Utility.h"

#include <fstream>

namespace Pikzel {

   std::vector<char> ReadFile(const std::filesystem::path& path, const bool readAsBinary) {
      std::ifstream file(path, std::ios::ate | (readAsBinary? std::ios::binary : 0));

      if (!file.is_open()) {
         throw std::runtime_error("failed to open file '" + path.string() + "'");
      }

      size_t fileSize = (size_t)file.tellg();
      std::vector<char> buffer(fileSize);

      file.seekg(0);
      file.read(buffer.data(), fileSize);

      file.close();

      return buffer;
   }

}
