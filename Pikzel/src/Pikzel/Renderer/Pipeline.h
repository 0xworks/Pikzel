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

   using SpecializationConstantsMap = std::unordered_map<std::string, int>; // only integer specialization constants supported at this stage.

   struct PKZL_API PipelineSettings {
      bool enableBlend = true; // enable or disable color blending.  Always src_alpha + (1 - src_alpha) at this stage.  TODO: support more complex options later
      std::vector<std::pair<ShaderType, std::filesystem::path>> shaders;
      const BufferLayout& bufferLayout = {};
      SpecializationConstantsMap specializationConstants;
   };


   class PKZL_API Pipeline {
   public:
      virtual ~Pipeline() = default;
   };

}