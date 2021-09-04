#pragma once

#include "Pikzel/Core/Core.h"

namespace Pikzel {

   struct PKZL_API Relationship {
      Object FirstChild = Null;
      Object NextSibling = Null;
      Object Parent = Null;
   };

}
