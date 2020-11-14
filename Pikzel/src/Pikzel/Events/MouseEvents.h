#pragma once

#include "Pikzel/Core/Core.h"
#include "Pikzel/Input/MouseButtons.h"

namespace Pikzel {

   // TODO: mouse button codes

   struct PKZL_API MouseMovedEvent {
      void* Sender;
      float X;
      float Y;
   };


   struct PKZL_API MouseScrolledEvent {
      void* Sender;
      float XOffset;
      float YOffset;
   };


   struct PKZL_API MouseButtonPressedEvent {
      void* Sender;
      MouseButton Button;
   };


   struct PKZL_API MouseButtonReleasedEvent {
      void* Sender;
      MouseButton Button;
   };

}
