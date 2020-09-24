#pragma once

#include "Pikzel/Input/KeyCodes.h"

namespace Pikzel {

   // TODO: key codes...

   struct KeyPressedEvent {
      void* Sender;
      KeyCode KeyCode;
      int RepeatCount;
   };


   struct KeyReleasedEvent {
      void* Sender;
      KeyCode KeyCode;
   };


   struct KeyTypedEvent {
      void* Sender;
      KeyCode KeyCode;
   };

}
