#include "AssetCache.h"

#include "Pikzel/Scene/ModelAssetLoader.h"

namespace Pikzel {

   Id AssetCache::LoadModelAsset(const std::filesystem::path& path) {
      auto id = entt::hashed_string(path.string().data());

      if (GetPath(id)) {
         PKZL_CORE_LOG_ERROR("Asset with path '{}' has already been loaded", path);
      } else {
         m_Paths.load(id, path);
         m_Models.load(id, path);
      }
      return id;
   }


   ModelAssetHandle AssetCache::GetModelAsset(Id id) {
      return m_Models[id];
   }


   Pikzel::PathHandle AssetCache::GetPath(Id id) {
      return m_Paths[id];
   }

   void AssetCache::Clear() {
      m_Paths.clear();
      m_Models.clear();
   }

}
