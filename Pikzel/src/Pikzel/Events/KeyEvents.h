#pragma once

namespace Pikzel {

   // TODO: key codes...

   struct KeyPressedEvent {
      void* sender;
      int KeyCode;
      int RepeatCount;
   };


   struct KeyReleasedEvent {
      void* sender;
      int KeyCode;
   };


   struct KeyTypedEvent {
      void* sender;
      int KeyCode;
   };

}
