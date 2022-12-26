#pragma once

#include "Pikzel/Core/Core.h"
#include "Pikzel/Scene/ModelAsset.h"
#include "Pikzel/Scene/ModelAssetLoader.h"

#include <entt/resource/cache.hpp>
#include <entt/resource/resource.hpp>

#include <filesystem>
#include <string>

namespace Pikzel {

   using PathCache = entt::resource_cache<std::filesystem::path>;
   using PathHandle = entt::resource<std::filesystem::path>;

   using ModelAssetCache = entt::resource_cache<ModelAsset, ModelAssetLoader>;
   using ModelAssetHandle = entt::resource<ModelAsset>;
   using ConstModelAssetHandle = entt::resource<const ModelAsset>;

   class PKZL_API AssetCache {
      AssetCache() = delete;
      PKZL_NO_COPYMOVE(AssetCache);

   public:
      static Id LoadModelAsset(const std::filesystem::path& path);

      static PathHandle GetPath(Id id);

      static ModelAssetHandle GetModelAsset(Id modelId);

      static void Clear();

   private:
      friend class AssetCacheSerializerYAML;
      inline static PathCache m_Paths;
      inline static ModelAssetCache m_Models;

   };

}
