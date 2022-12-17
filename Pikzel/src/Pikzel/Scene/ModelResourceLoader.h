#pragma once

#include "Pikzel/Scene/ModelResource.h"

#include <filesystem>
#include <memory>
#include <string_view>

namespace Pikzel {

   struct ModelResourceLoader {
      using result_type = std::shared_ptr<ModelResource>;

      result_type operator()(const std::string_view name, const std::filesystem::path& path) const;

   };

}
