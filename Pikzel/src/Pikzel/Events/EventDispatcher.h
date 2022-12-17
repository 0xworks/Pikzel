#pragma once

#include "Pikzel/Core/Core.h"

#include <entt/signal/dispatcher.hpp>
#include <memory>

namespace Pikzel {

   struct PKZL_API EventDispatcher {
      EventDispatcher() = delete;
      ~EventDispatcher() = delete;

      static void Init() {
         s_Dispatcher = std::make_unique<entt::dispatcher>();
      }

      static void DeInit() {
         s_Dispatcher.reset(nullptr);
      }

      // Send an Event, constructed with args as specified
      template<typename Event, typename... Args>
      static void Send(Args... args) {
         s_Dispatcher->trigger<Event>({std::forward<Args>(args)...});
      }

      // Connect Event to a candidate free function
      template<typename Event, typename Candidate>
      static void Connect() {
         s_Dispatcher->sink<Event>().template connect<Candidate>();
      }

      template<typename Event, auto Candidate>
      static void Disconnect() {
         s_Dispatcher->sink<Event>().template disconnect<Candidate>();
      }

      // Connect Event to a candidate that is a member function of Type
      template<typename Event, auto Candidate, typename Type>
      static void Connect(Type&& instance) {
         s_Dispatcher->sink<Event>().template connect<Candidate>(instance);
      }

      template<typename Event, auto Candidate, typename Type>
      static void Disconnect(Type&& instance) {
         s_Dispatcher->sink<Event>().template disconnect<Candidate>(instance);
      }

   private:
      inline static std::unique_ptr<entt::dispatcher> s_Dispatcher;
   };

}
