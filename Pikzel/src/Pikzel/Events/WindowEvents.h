#pragma once

#include "Pikzel/Core/Core.h"

namespace Pikzel {

   struct PKZL_API WindowResizeEvent {
      void* Sender;
      int Width;
      int Height;
   };


   struct PKZL_API WindowVSyncChangedEvent {
      void* Sender;
      bool IsVSync;
   };


   struct PKZL_API WindowCloseEvent {
      void* Sender;
   };

}
