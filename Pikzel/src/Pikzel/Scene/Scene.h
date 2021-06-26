#pragma once

#include "Pikzel/Core/Core.h"
#include "Pikzel/Events/ApplicationEvents.h"

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <chrono>

namespace Pikzel {

   using Object = entt::entity;

   class PKZL_API Scene {
   public:
      virtual ~Scene() = default;

      Object CreateObject();
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
      bool HasComponent(const Object object) const {
         return m_Registry.all_of<T>(object);
      }

      template<typename T>
      void RemoveComponent(const Object object) {
         PKZL_CORE_ASSERT(HasComponent<T>(object), "Object does not have component!");
         m_Registry.erase<T>(object);
      }

      void OnUpdate(DeltaTime dt);

   private:

      // HACK: At this stage I am unsure how entt "groups" and "views"
      //       will be exposed to clients of Scene.
      //       For now, just hackaround this by exposing the registry directly
      //
      //       Ideally we do not want anything in the "entt" namespace escaping from
      //       Scene.  (e.g. so the abstraction of the ECS is clean, and (in theory at least)
      //       entt could be replaced with a different ECS, and the clients of Scene do not
      //       need to change)
   public:
      entt::registry m_Registry;
   private:

   };

   std::unique_ptr<Scene> PKZL_API CreateScene();

}
