#pragma once

namespace Pikzel {

   using DeltaTime = std::chrono::duration<float, std::chrono::seconds::period>;


   struct UpdateEvent {
      Pikzel::DeltaTime deltaTime;
   };


   struct FixedUpdateEvent {
   };

}
