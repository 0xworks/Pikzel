#pragma once

#include "Model.h"

#include <filesystem>
#include <memory>

namespace Pikzel {
   
   namespace ModelSerializer {

      PKZL_API std::unique_ptr<Model> Import(const std::filesystem::path& path);

      PKZL_API void ClearTextureCache();

   };

}
