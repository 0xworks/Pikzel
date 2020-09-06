#pragma once

#include "Buffer.h"

#include <glm/glm.hpp>

#include <filesystem>
#include <utility>
#include <vector>

namespace Pikzel {

   enum ShaderType {
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

      virtual void SetInt(const std::string& name, int value) = 0;
      virtual void SetIntArray(const std::string& name, int* values, uint32_t count) = 0;
      virtual void SetFloat(const std::string& name, float value) = 0;
      virtual void SetFloat3(const std::string& name, const glm::vec3& value) = 0;
      virtual void SetFloat4(const std::string& name, const glm::vec4& value) = 0;
      virtual void SetMat4(const std::string& name, const glm::mat4& value) = 0;

   };

}