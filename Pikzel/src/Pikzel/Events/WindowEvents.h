#pragma once

namespace Pikzel {

   struct WindowResizeEvent {
      void* sender;
      int Width;
      int Height;
   };


   struct WindowCloseEvent {
      void* sender;
   };

}
