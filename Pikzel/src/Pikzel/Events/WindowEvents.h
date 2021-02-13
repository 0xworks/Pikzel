#pragma once

#include "Pikzel/Core/Core.h"

namespace Pikzel {

   struct PKZL_API WindowResizeEvent {
      void* sender;
      int width;
      int height;
   };


   struct PKZL_API WindowVSyncChangedEvent {
      void* sender;
      bool isVSync;
   };


   struct PKZL_API WindowCloseEvent {
      void* sender;
   };

}
