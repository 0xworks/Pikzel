#pragma once

#include "Model.h"
#include "Pikzel/Renderer/Texture.h"

#include <filesystem>
#include <memory>

namespace Pikzel {
   
   namespace ModelSerializer {

      std::unique_ptr<Model> Import(const std::filesystem::path& path);

      void ClearTextureCache();

   };

}
