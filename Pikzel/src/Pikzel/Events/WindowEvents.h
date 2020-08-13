#pragma once

namespace Pikzel {

   struct WindowResizeEvent {
      void* Sender;
      int Width;
      int Height;
   };


   struct WindowCloseEvent {
      void* Sender;
   };

}
