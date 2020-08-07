#pragma once

namespace Pikzel {

   struct KeyPressedEvent {
      int KeyCode;
      int RepeatCount;
   };


   struct KeyReleasedEvent {
      int KeyCode;
   };


   struct KeyTypedEvent {
      int KeyCode;
   };

}
