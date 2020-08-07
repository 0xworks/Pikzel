#pragma once

#include <entt/signal/dispatcher.hpp>

namespace Pikzel {

namespace Events {
   entt::dispatcher dispatcher {};

   template<typename Event, typename... Args>
   void SendEvent(Args... args) {
      dispatcher.trigger<Event>(std::forward<Args>(args)...);
   }

}

}
