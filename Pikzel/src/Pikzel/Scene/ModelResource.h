#pragma once

#include "Mesh.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

namespace Pikzel {

   struct PKZL_API ModelResource final {
      PKZL_NO_COPY(ModelResource);

      ModelResource() = default;

      ModelResource(const std::string_view name, const std::filesystem::path& path)
      : Name{ name }
      , Path{ path }
      {}

      ~ModelResource() = default;

      ModelResource(ModelResource&& model) noexcept
      : Meshes{ std::move(model.Meshes) }
      , Path{ std::move(model.Path) }
      {}

      ModelResource& operator=(ModelResource&& model) noexcept {
         if (this != &model) {
            Meshes = std::move(model.Meshes);
            Path = std::move(model.Path);
         }
         return *this;
      }

      std::vector<Mesh> Meshes;
      std::string Name;
      std::filesystem::path Path;
   };

}
