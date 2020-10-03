#pragma once

#include "Buffer.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <utility>
#include <vector>

namespace Pikzel {

   enum class ShaderType {
      Vertex,
      Fragment
   };

   struct PipelineSettings {
      const VertexBuffer& VertexBuffer;         // OpenGL needs to bind vertex buffer in order that the pipeline can set the vertex attribute descriptions
      std::vector<std::pair<ShaderType, std::filesystem::path>> Shaders;
   };


   class Pipeline {
   public:
      virtual ~Pipeline() = default;


   };

}