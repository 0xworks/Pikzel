#pragma once

#include "Pikzel/Core/Core.h"
#include "Pikzel/Events/ApplicationEvents.h"

#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <chrono>

namespace Pikzel {

   using Entity = entt::entity;

   class PKZL_API Scene {
   public:
      virtual ~Scene() = default;

      Entity CreateEntity();
      void DestroyEntity(Entity entity);

      template<typename T, typename... Args>
      T& AddComponent(Entity entity, Args&&... args) {
         PKZL_CORE_ASSERT(!HasComponent<T>(entity), "Entity already has component!");
         return m_Registry.emplace<T>(entity, std::forward<Args>(args)...);
      }

      template<typename T>
      T& GetComponent(Entity entity) {
         PKZL_CORE_ASSERT(HasComponent<T>(entity), "Entity does not have component!");
         return m_Registry.get<T>(entity);
      }

      template<typename T>
      bool HasComponent(Entity entity) {
         return m_Registry.all_of<T>(entity);
      }

      template<typename T>
      void RemoveComponent(Entity entity) {
         PKZL_CORE_ASSERT(HasComponent<T>(entity), "Entity does not have component!");
         m_Registry.erase<T>(entity);
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
