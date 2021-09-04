#include "Scene.h"
#include "ModelResourceLoader.h"

#include "Pikzel/Components/Relationship.h"

namespace Pikzel {

   auto GetSortKey(const entt::registry& registry, const Object object) {
      std::vector<std::pair<std::string_view, Object>> keys;
      keys.emplace_back(registry.get<std::string>(object), object);
      Object parent = registry.get<Relationship>(object).Parent;
      while (parent != Null) {
         keys.emplace_back(registry.get<std::string>(parent), parent);
         parent = registry.get<Relationship>(parent).Parent;
      }
      std::reverse(keys.begin(), keys.end());
      return keys;
   }


   void AppendChildObjects(const entt::registry& registry, const Object object, std::vector<Object>& objects) {
      if (object != Null) {
         const auto& relationship = registry.get<Relationship>(object);
         Object child = relationship.FirstChild;
         while (child != Null) {
            objects.emplace_back(child);
            AppendChildObjects(registry, child, objects);
            child = registry.get<Relationship>(child).NextSibling;
         }
      }
   }


   Object Scene::CreateEmptyObject() {
      return m_Registry.create();
   }


   Object Scene::CreateObject(Object parent) {
      Object object = CreateEmptyObject();
      AddComponent<std::string>(object, "NewObject");
      AddComponent<Relationship>(object, Null, Null, parent);
      SortObjects();
      return object;
   }


   void Scene::DestroyObject(Object object) {
      std::vector<Object> children;
      AppendChildObjects(m_Registry, object, children);
      for (auto child : children) {
         m_Registry.destroy(child);
      }
      m_Registry.destroy(object);
      SortObjects();
   }


   void Scene::SortObjects() {
      m_Registry.sort<Relationship>([this](const Object lhs, const Object rhs) {
         const auto& lhsKey = GetSortKey(m_Registry, lhs);
         const auto& rhsKey = GetSortKey(m_Registry, rhs);
         return lhsKey < rhsKey;
      });

      // having sorted them, we now need to fix up the Relationship components
      // so that when we traverse Relationship (starting from the first object in the Relationship
      // component pool), that traversal happens in the right order.
      auto relationships = GetView<Relationship>();
      Object root = Null;
      for (auto&& [object, relationship] : relationships.each()) {
         relationship.FirstChild = Null;
         relationship.NextSibling = Null;
         if (relationship.Parent == Null) {
            if (root == Null) {
               root = object;
            } else {
               Object sibling = root;
               auto siblingRelationship = GetComponent<Relationship>(sibling);
               while (siblingRelationship.NextSibling != Null) {
                  sibling = siblingRelationship.NextSibling;
                  siblingRelationship = GetComponent<Relationship>(sibling);
               }
               GetComponent<Relationship>(sibling).NextSibling = object;
            }
         } else {
            auto& parentRelationship = GetComponent<Relationship>(relationship.Parent);
            if (parentRelationship.FirstChild == Null) {
               parentRelationship.FirstChild = object;
            } else {
               Object sibling = parentRelationship.FirstChild;
               auto siblingRelationship = GetComponent<Relationship>(sibling);
               while (siblingRelationship.NextSibling != Null) {
                  sibling = siblingRelationship.NextSibling;
                  siblingRelationship = GetComponent<Relationship>(sibling);
               }
               GetComponent<Relationship>(sibling).NextSibling = object;
            }
         }
      }
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
