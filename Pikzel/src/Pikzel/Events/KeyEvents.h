#pragma once

#include "Pikzel/Core/Core.h"
#include "Pikzel/Input/KeyCodes.h"

namespace Pikzel {

   // TODO: key codes...

   struct PKZL_API KeyPressedEvent {
      void* sender;
      KeyCode keyCode;
      int repeatCount;
   };


   struct PKZL_API KeyReleasedEvent {
      void* sender;
      KeyCode keyCode;
   };


   struct PKZL_API KeyTypedEvent {
      void* sender;
      KeyCode keyCode;
   };

}
