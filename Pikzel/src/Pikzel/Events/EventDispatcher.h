#pragma once

#include <entt/signal/dispatcher.hpp>

namespace Pikzel {

struct EventDispatcher {
   EventDispatcher() = delete;
   ~EventDispatcher() = delete;

   // Send an Event, constructed with args as specified
   template<typename Event, typename... Args>
   static void Send(Args... args) {
      m_Dispatcher.trigger<Event>(std::forward<Args>(args)...);

   }

   // Connect Event to a candidate free function
   template<typename Event, auto Candidate>
   static void Connect() {
      m_Dispatcher.sink<Event>().connect<Candidate>();
   }

   // Connect Event to a candidate that is a member function of Type
   template<typename Event, auto Candidate, typename Type>
   static void Connect(Type&& instance) {
      m_Dispatcher.sink<Event>().connect<Candidate>(instance);
   }


private:
   static entt::dispatcher m_Dispatcher;
};

}
