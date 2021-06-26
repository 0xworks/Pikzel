#include "Scene.h"

namespace Pikzel {

   Object Scene::CreateObject() {
      return m_Registry.create();
   }


   void Scene::DestroyObject(Object object) {
      m_Registry.destroy(object);
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
