#pragma once

#include <filesystem>
#include <vector>

namespace Pikzel {

   std::vector<char> ReadFile(const std::filesystem::path& path, const bool readAsBinary);

}
