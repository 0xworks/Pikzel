#pragma once

#include "Buffer.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <utility>
#include <vector>

namespace Pikzel {

   enum class ShaderType {
      Vertex,
      Geometry,
      Fragment,
      Compute
   };

   struct PKZL_API PipelineSettings {
      const BufferLayout& BufferLayout;
      std::vector<std::pair<ShaderType, std::filesystem::path>> Shaders;
   };


   class PKZL_API Pipeline {
   public:
      virtual ~Pipeline() = default;
   };

}