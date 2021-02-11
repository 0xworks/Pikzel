#pragma once

#include "Model.h"

#include <filesystem>
#include <memory>

namespace SponzaShadows {

   // Eventually, the Pikzel engine will have its own model serializer.
   // In the meantime, this demo uses this one
   namespace ModelSerializer {
      std::unique_ptr<Model> Import(const std::filesystem::path& path);
      void ClearTextureCache();
   }

}
