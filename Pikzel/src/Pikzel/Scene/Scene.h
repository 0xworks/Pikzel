#pragma once

#include "Pikzel/Core/Core.h"
#include "Pikzel/Events/ApplicationEvents.h"

#include <entt/entity/registry.hpp>

#include <chrono>

namespace Pikzel {

   using Object = entt::entity;
   using Registry = entt::registry;

   template<typename T>
   concept ObjectFunc = requires(T func, Object obj) {
      func(obj);
   };


   class PKZL_API Scene {
   public:
      virtual ~Scene() = default;

      // Create a completely empty object (no components)
      Object CreateEmptyObject();

      // Create an object and initialise it with a std::string component ("NewObject"),
      // and an initial relationship component to specified parent (which may be Null)
      Object CreateObject(Object parent);

      void DestroyObject(Object entity);

      template<typename T, typename... Args>
      T& AddComponent(const Object object, Args&&... args) {
         PKZL_CORE_ASSERT(!HasComponent<T>(object), "Object already has component!");
         return m_Registry.emplace<T>(object, std::forward<Args>(args)...);
      }

      template<typename T>
      T& GetComponent(const Object object) {
         PKZL_CORE_ASSERT(HasComponent<T>(object), "Object does not have component!");
         return m_Registry.get<T>(object);
      }

      template<typename T>
      const T& GetComponent(const Object object) const {
         PKZL_CORE_ASSERT(HasComponent<T>(object), "Object does not have component!");
         return m_Registry.get<T>(object);
      }

      template<typename T>
      T* TryGetComponent(const Object object) {
         return m_Registry.try_get<T>(object);
      }

      template<typename T>
      const T* TryGetComponent(const Object object) const {
         return m_Registry.try_get<T>(object);
      }

      template<typename T>
      bool HasComponent(const Object object) const {
         return m_Registry.all_of<T>(object);
      }

      template<typename T>
      void RemoveComponent(const Object object) {
         PKZL_CORE_ASSERT(HasComponent<T>(object), "Object does not have component!");
         m_Registry.erase<T>(object);
      }

      template<ObjectFunc Func>
      void Each(Func func) const {
         m_Registry.each([func](entt::entity e) {
            func(e);
         });
      }

      template<typename... Component>
      auto GetGroup() {
         return m_Registry.group<Component...>();
      }

      template<typename... Component>
      auto GetView() {
         return m_Registry.view<Component...>();
      }

      template<typename... Component>
      auto GetView() const {
         return m_Registry.view<Component...>();
      }

      // sort objects based on their Relationship component
      // this puts them in the order in which we want to
      // draw them in the scene hierachy panel
      // TODO: later allow different sort orders
      void SortObjects();

      void OnUpdate(DeltaTime dt);

   private:
      friend class SceneSerializerYAML;
      Registry m_Registry;
   };

   std::unique_ptr<Scene> PKZL_API CreateScene();

}
