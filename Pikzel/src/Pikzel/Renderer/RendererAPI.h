#pragma once

#include <string>

namespace Pikzel {

   enum class RendererAPI {
      None,
      OpenGL,
      Vulkan
   };

   inline std::string to_string(RendererAPI value) {
      switch (value) {
         case RendererAPI::None: return "Undefined";
         case RendererAPI::OpenGL: return "OpenGL";
         case RendererAPI::Vulkan: return "Vulkan";
         default: return "invalid";
      }
   }
}
