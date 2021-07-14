#pragma once

#include "Pikzel/Scene/ModelResource.h"

#include <entt/resource/loader.hpp>

#include <filesystem>

namespace Pikzel {

   struct ModelResourceLoader final : entt::resource_loader<ModelResourceLoader, ModelResource> {

      std::shared_ptr<ModelResource> load(const std::filesystem::path& path) const;

   };

}
