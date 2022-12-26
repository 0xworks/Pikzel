#pragma once

#include "Pikzel/Scene/ModelAsset.h"

#include <filesystem>
#include <memory>
#include <string_view>

namespace Pikzel {

   struct ModelAssetLoader {
      using result_type = std::shared_ptr<ModelAsset>;

      result_type operator()(const std::filesystem::path& path) const;

   };

}
