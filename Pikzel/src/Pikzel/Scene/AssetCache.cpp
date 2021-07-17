#include "AssetCache.h"

#include "Pikzel/Scene/ModelResourceLoader.h"

namespace Pikzel {

   Id AssetCache::LoadModelResource(const std::string_view name, const std::filesystem::path& path) {
      auto id = entt::hashed_string(name.data());
      if (auto handle = GetModelResource(id)) {
         if (handle->Path != path) {
            PKZL_CORE_LOG_ERROR("Model with name '{0}' has already been loaded from path '{1}'.  This conflicts with attempt to load from path '{2}'", name, handle->Path, path);
         }
      } else {
         m_ModelCache.load<ModelResourceLoader>(id, name, path);
      }
      return id;
   }


   ModelResourceHandle AssetCache::GetModelResource(Id id) {
      return m_ModelCache.handle(id);
   }


   void AssetCache::Clear() {
      m_ModelCache.clear();
   }

}
