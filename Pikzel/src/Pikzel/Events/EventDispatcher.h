#pragma once

#include <entt/signal/dispatcher.hpp>

namespace Pikzel {

struct EventDispatcher {
   EventDispatcher() = delete;
   ~EventDispatcher() = delete;

   template<typename Event, typename... Args>
   static void Send(Args... args) {
      m_Dispatcher.trigger<Event>(std::forward<Args>(args)...);

   }

   template<typename Event, auto Candidate, typename Type>
   static void Connect(Type&& value_or_instance) {
      m_Dispatcher.sink<Event>().connect<Candidate>(value_or_instance);
   }

private:
   static entt::dispatcher m_Dispatcher;
};

}
