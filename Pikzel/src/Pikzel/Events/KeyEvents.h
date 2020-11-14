#pragma once

#include "Pikzel/Core/Core.h"
#include "Pikzel/Input/KeyCodes.h"

namespace Pikzel {

   // TODO: key codes...

   struct PKZL_API KeyPressedEvent {
      void* Sender;
      KeyCode KeyCode;
      int RepeatCount;
   };


   struct PKZL_API KeyReleasedEvent {
      void* Sender;
      KeyCode KeyCode;
   };


   struct PKZL_API KeyTypedEvent {
      void* Sender;
      KeyCode KeyCode;
   };

}
