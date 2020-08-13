#pragma once

namespace Pikzel {

   // TODO: mouse button codes

   struct MouseMovedEvent {
      void* Sender;
      float X;
      float Y;
   };


   struct MouseScrolledEvent {
      void* Sender;
      float XOffset;
      float YOffset;
   };


   struct MouseButtonPressedEvent {
      void* Sender;
      int Button;
   };


   struct MouseButtonReleasedEvent {
      void* Sender;
      int Button;
   };

}
