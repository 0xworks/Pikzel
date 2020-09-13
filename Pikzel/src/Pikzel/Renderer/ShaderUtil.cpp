#include "pch.h"
#include "ShaderUtil.h"
#include "Pikzel/Core/Utility.h"

#include <shaderc/shaderc.hpp>

namespace Pikzel {

   static shaderc_target_env APItoShaderCTarget(const RenderCore::API api) {
      switch (api) {
         case RenderCore::API::OpenGL: return shaderc_target_env_opengl_compat;
         case RenderCore::API::Vulkan: return shaderc_target_env_vulkan;
      }
      PKZL_CORE_ASSERT(false, "Unsupported RenderCore::API!");
      return shaderc_target_env_default;
   }


   static uint32_t APItoShaderCEnvVersion(const RenderCore::API api) {
      switch (api) {
         case RenderCore::API::OpenGL: return shaderc_env_version_opengl_4_5;
         case RenderCore::API::Vulkan: return shaderc_env_version_vulkan_1_2;
      }
      PKZL_CORE_ASSERT(false, "Unsupported RenderCore::API!");
      return 0;
   }


   static shaderc_shader_kind ShaderTypeToShaderCType(const ShaderType type) {
      switch (type) {
         case ShaderType::Vertex:   return shaderc_glsl_default_vertex_shader;
         case ShaderType::Fragment: return shaderc_glsl_default_fragment_shader;
      }

      PKZL_CORE_ASSERT(false, "Unknown ShaderType!");
      return shaderc_glsl_default_vertex_shader;
   }


   DataType SPIRTypeToDataType(const spirv_cross::SPIRType type) {
      switch (type.basetype) {
         case spirv_cross::SPIRType::Boolean:
            switch (type.vecsize) {
               case 1:                        return DataType::Bool;
               case 2:                        return DataType::BVec2;
               case 3:                        return DataType::BVec3;
               case 4:                        return DataType::BVec4;
            }
         case spirv_cross::SPIRType::Int:
            switch (type.vecsize) {
               case 1:                        return DataType::Int;
               case 2:                        return DataType::IVec2;
               case 3:                        return DataType::IVec3;
               case 4:                        return DataType::IVec4;
            }
         case spirv_cross::SPIRType::UInt:
         case 1:
            switch (type.vecsize) {
               case 1:                        return DataType::UInt;
               case 2:                        return DataType::UVec2;
               case 3:                        return DataType::UVec3;
               case 4:                        return DataType::UVec4;
            }
         case spirv_cross::SPIRType::Float:
            switch (type.columns) {
               case 1:
                  switch (type.vecsize) {
                     case 1:                  return DataType::Float;
                     case 2:                  return DataType::Vec2;
                     case 3:                  return DataType::Vec3;
                     case 4:                  return DataType::Vec4;
                  }
               case 2:
                  switch (type.vecsize) {
                     case 2:                  return DataType::Mat2;
                     case 3:                  return DataType::Mat2x3;
                     case 4:                  return DataType::Mat2x4;
                  }
               case 3:
                  switch (type.vecsize) {
                     case 2:                  return DataType::Mat3x2;
                     case 3:                  return DataType::Mat3;
                     case 4:                  return DataType::Mat3x4;
                  }
               case 4:
                  switch (type.vecsize) {
                     case 2:                  return DataType::Mat4x2;
                     case 3:                  return DataType::Mat3x4;
                     case 4:                  return DataType::Mat4;
                  }
            }
         case spirv_cross::SPIRType::Double:
            switch (type.columns) {
               case 1:
                  switch (type.vecsize) {
                     case 1:                  return DataType::Double;
                     case 2:                  return DataType::DVec2;
                     case 3:                  return DataType::DVec3;
                     case 4:                  return DataType::DVec4;
                  }
               case 2:
                  switch (type.vecsize) {
                     case 2:                  return DataType::DMat2;
                     case 3:                  return DataType::DMat2x3;
                     case 4:                  return DataType::DMat2x4;
                  }
               case 3:
                  switch (type.vecsize) {
                     case 2:                  return DataType::DMat3x2;
                     case 3:                  return DataType::DMat3;
                     case 4:                  return DataType::DMat3x4;
                  }
               case 4:
                  switch (type.vecsize) {
                     case 2:                  return DataType::DMat4x2;
                     case 3:                  return DataType::DMat3x4;
                     case 4:                  return DataType::DMat4;
                  }
            }
      }
      PKZL_CORE_ASSERT(false, "Unknown type!");
      return DataType::None;
   }


   std::vector<uint32_t> CompileGlslToSpv(RenderCore::API api, const ShaderType type, const std::filesystem::path path) {
      PKZL_CORE_LOG_TRACE("Compiling shader '" + path.string() + "'...");
      std::vector<char> src = ReadFile(path, /*readAsBinary=*/true);
      return CompileGlslToSpv(api, type, path, src.data(), src.size());
   }


   std::vector<uint32_t> CompileGlslToSpv(RenderCore::API api, const ShaderType type, const std::filesystem::path path, const char* source, size_t size) {
      shaderc::Compiler compiler;
      shaderc::CompileOptions options;
      options.SetTargetEnvironment(APItoShaderCTarget(api), APItoShaderCEnvVersion(api));
      //options.SetOptimizationLevel(shaderc_optimization_level_performance);
      shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, size, ShaderTypeToShaderCType(type), path.string().c_str(), options);
      if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
         PKZL_CORE_LOG_ERROR("{0}", module.GetErrorMessage());
         throw std::runtime_error("Shader compilation failure!");
      }
      return std::vector<uint32_t> {module.cbegin(), module.cend()};
   }

}