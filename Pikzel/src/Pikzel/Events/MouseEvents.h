#pragma once

namespace Pikzel {

   struct MouseMovedEvent {
      float X;
      float Y;
   };


   struct MouseScrolledEvent {
      float XOffset;
      float YOffset;
   };


   struct MouseButtonPressedEvent {
      int Button;
   };


   struct MouseButtonReleasedEvent {
      int Button;
   };

}
