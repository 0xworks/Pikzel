#pragma once

// Platform-specific utility functions.

#include "Core.h"

#include <filesystem>
#include <optional>

namespace Pikzel {

   // these return no value if the user cancels the dialog
   PKZL_API std::optional<std::filesystem::path> OpenFileDialog(const char* filter = nullptr);
   PKZL_API std::optional<std::filesystem::path> SaveFileDialog(const char* filter = nullptr);

}
