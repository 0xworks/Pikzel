#pragma once

namespace Pikzel {

   // TODO: mouse button codes

   struct MouseMovedEvent {
      void* sender;
      float X;
      float Y;
   };


   struct MouseScrolledEvent {
      void* sender;
      float XOffset;
      float YOffset;
   };


   struct MouseButtonPressedEvent {
      void* sender;
      int Button;
   };


   struct MouseButtonReleasedEvent {
      void* sender;
      int Button;
   };

}
