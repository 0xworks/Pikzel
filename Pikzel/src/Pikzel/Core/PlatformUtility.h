#pragma once

// Platform-specific utility functions.

#include <filesystem>
#include <optional>

namespace Pikzel {

   // these return no value if the user cancels the dialog
   std::optional<std::filesystem::path> OpenFileDialog(const char* filter = nullptr);
   std::optional<std::filesystem::path> SaveFileDialog(const char* filter = nullptr);

}
