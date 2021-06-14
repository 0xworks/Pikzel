#include "Scene.h"

namespace Pikzel {

   Entity Scene::CreateEntity() {
      return m_Registry.create();
   }


   void Scene::DestroyEntity(Entity entity) {
      m_Registry.destroy(entity);
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
