#pragma once

#include "Pikzel/Core/Core.h"

#include <chrono>

namespace Pikzel {

   using DeltaTime = std::chrono::duration<float, std::chrono::seconds::period>;


   struct PKZL_API UpdateEvent {
      Pikzel::DeltaTime deltaTime;
   };


   struct PKZL_API FixedUpdateEvent {
   };

}
