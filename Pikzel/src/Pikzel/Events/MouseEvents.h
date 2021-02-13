#pragma once

#include "Pikzel/Core/Core.h"
#include "Pikzel/Input/MouseButtons.h"

namespace Pikzel {

   // TODO: mouse button codes

   struct PKZL_API MouseMovedEvent {
      void* sender;
      float x;
      float y;
   };


   struct PKZL_API MouseScrolledEvent {
      void* sender;
      float xOffset;
      float yOffset;
   };


   struct PKZL_API MouseButtonPressedEvent {
      void* sender;
      MouseButton button;
   };


   struct PKZL_API MouseButtonReleasedEvent {
      void* sender;
      MouseButton button;
   };

}
