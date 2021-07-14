#include "Scene.h"
#include "ModelResourceLoader.h"

namespace Pikzel {

   Object Scene::CreateObject() {
      return m_Registry.create();
   }


   void Scene::DestroyObject(Object object) {
      m_Registry.destroy(object);
   }


   Id Scene::LoadModelResource(const std::filesystem::path& path) {
      auto id = entt::hashed_string(path.string().c_str()).value();
      if (!m_ModelCache.load<ModelResourceLoader>(id, path)) {
         PKZL_CORE_LOG_ERROR("Failed to load model '{0}'", path.string().c_str());
      }
      return id;
   }


   ModelResourceHandle Scene::GetModelResource(Id id) const {
      return m_ModelCache.handle(id);
   }


   void Scene::OnUpdate(DeltaTime dt) {

      // Something interesting goes here:
      //    * run scripts
      //    * physics

   }


   std::unique_ptr<Scene> CreateScene() {
      return std::make_unique<Scene>();
   }

}
