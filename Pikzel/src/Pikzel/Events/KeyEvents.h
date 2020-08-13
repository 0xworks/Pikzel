#pragma once

namespace Pikzel {

   // TODO: key codes...

   struct KeyPressedEvent {
      void* Sender;
      int KeyCode;
      int RepeatCount;
   };


   struct KeyReleasedEvent {
      void* Sender;
      int KeyCode;
   };


   struct KeyTypedEvent {
      void* Sender;
      int KeyCode;
   };

}
