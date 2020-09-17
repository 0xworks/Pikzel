#pragma once

#include "Pipeline.h"
#include "RenderCore.h"

#include <spirv_cross/spirv_glsl.hpp>

#include <filesystem>
#include <unordered_map>

namespace Pikzel {

   DataType SPIRTypeToDataType(const spirv_cross::SPIRType type);

   std::vector<uint32_t> CompileGlslToSpv(RenderCore::API api, const ShaderType type, const std::filesystem::path path);
   std::vector<uint32_t> CompileGlslToSpv(RenderCore::API api, const ShaderType type, const std::filesystem::path path, const char* source, size_t size);

}
