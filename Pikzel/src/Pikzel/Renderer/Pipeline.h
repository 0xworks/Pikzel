#pragma once

#include "Buffer.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <utility>
#include <vector>

namespace Pikzel {

   enum class ShaderType {
      Vertex,
      Fragment,
      Compute
   };

   struct PipelineSettings {
      const BufferLayout& BufferLayout;
      std::vector<std::pair<ShaderType, std::filesystem::path>> Shaders;
   };


   class Pipeline {
   public:
      virtual ~Pipeline() = default;
   };

}