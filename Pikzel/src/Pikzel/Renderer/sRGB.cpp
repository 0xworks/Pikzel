#include "sRGB.h"

#include "Pikzel/Core/Core.h"

namespace Pikzel {

   float sRGBToLinear(float val) {
      PKZL_CORE_ASSERT((val >= 0.0f) && (val <= 1.0f), "sRGB floating point color value should be between 0.0f and 1.0f");
      val = std::clamp(val, 0.0f, 1.0f);
      if (val < 0.04045) {
         return val * 25.0f / 323.0f;
      }
      return pow((200.0f * val + 11.0f) / 211.0f, 12.0f / 5.0f);
   }


   Pikzel::sRGB::sRGB(const float r, const float g, const float b)
   : red {sRGBToLinear(r)}
   , green {sRGBToLinear(g)}
   , blue {sRGBToLinear(b)}
   {}


   sRGB::sRGB(const int r, const int g, const int b)
   : red {sRGBToLinear(r / 255.0f)}
   , green {sRGBToLinear(g / 255.0f)}
   , blue {sRGBToLinear(b / 255.0f)}
   {}


   sRGB::operator glm::vec4() const {
      return {red, green, blue, 1.0f};
   }


   sRGB::operator glm::vec3() const {
      return {red, green, blue};
   }


   sRGBA::sRGBA(const float r, const float g, const float b, const float a)
   : red {sRGBToLinear(r)}
   , green {sRGBToLinear(g)}
   , blue {sRGBToLinear(b)}, alpha {a}
   {}


   sRGBA::sRGBA(const int r, const int g, const int b, const int a)
   : red {sRGBToLinear(r / 255.0f)}
   , green {sRGBToLinear(g / 255.0f)}
   , blue {sRGBToLinear(b / 255.0f)}
   , alpha {a / 255.0f}
   {}


   sRGBA::operator glm::vec3() const {
      return {red, green, blue};
   }


   sRGBA::operator glm::vec4() const {
      return {red, green, blue, alpha};
   }

}
