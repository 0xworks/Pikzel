#pragma once

namespace Pikzel {

   struct WindowResizeEvent {
      void* Sender;
      int Width;
      int Height;
   };


   struct WindowVSyncChangedEvent {
      void* Sender;
      bool IsVSync;
   };


   struct WindowCloseEvent {
      void* Sender;
   };

}
