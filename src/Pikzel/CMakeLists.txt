# The Pikzel engine runtime library.
#
# Built as a static library: there is no DLL boundary to manage, so no export macros,
# and the linker can discard anything a given program does not use.
#
# Source files are listed explicitly (no globbing) so that adding or removing a file is
# a visible change, and CMake re-configures reliably when it happens.

add_library(Pikzel STATIC)
add_library(Pikzel::Pikzel ALIAS Pikzel)

target_sources(
   Pikzel
   PRIVATE
      Core/Filesystem.h
      Core/Filesystem.cpp
      Core/Log.h
      Core/Log.cpp

      Platform/Window.h
      Platform/Window.cpp

      Renderer/Buffer.h
      Renderer/Buffer.cpp
      Renderer/Camera.h
      Renderer/Camera.cpp
      Renderer/Color.h
      Renderer/Color.cpp
      Renderer/CommandList.h
      Renderer/CommandList.cpp
      Renderer/DescriptorSet.h
      Renderer/DescriptorSet.cpp
      Renderer/Format.h
      Renderer/GraphicsDevice.h
      Renderer/GraphicsDevice.cpp
      Renderer/Nri.h
      Renderer/Nri.cpp
      Renderer/Pipeline.h
      Renderer/Pipeline.cpp
      Renderer/Sampler.h
      Renderer/Sampler.cpp
      Renderer/Shader.h
      Renderer/Shader.cpp
      Renderer/ShaderTypes.h
      Renderer/ShaderTypes.cpp
      Renderer/SwapChain.h
      Renderer/SwapChain.cpp
      Renderer/Texture.h
      Renderer/Texture.cpp
)

# Engine headers are included by their path from src, e.g. "Pikzel/Core/Log.h".
target_include_directories(Pikzel PUBLIC "${PROJECT_SOURCE_DIR}/src")

# spdlog is PUBLIC because the logging macros in Log.h expand to spdlog calls.
# RTM is PUBLIC because it is the engine's math library, used directly in public headers.
# NRI and GLFW are PRIVATE: they are implementation details of the renderer and Window,
# and the engine's public headers do not include them (see Renderer/Nri.h).
target_link_libraries(
   Pikzel
   PUBLIC
      spdlog::spdlog
      rtm::rtm
   PRIVATE
      NRI
      glfw
)

# The Vulkan binding offsets are defined once, in cmake/Shaders.cmake, and shared with
# the shader compiler.
target_compile_definitions(
   Pikzel
   PRIVATE
      PKZL_VK_BINDING_OFFSET_SAMPLER=${PKZL_VK_BINDING_OFFSET_SAMPLER}
      PKZL_VK_BINDING_OFFSET_CONSTANT_BUFFER=${PKZL_VK_BINDING_OFFSET_CONSTANT_BUFFER}
      PKZL_VK_BINDING_OFFSET_READ_WRITE=${PKZL_VK_BINDING_OFFSET_READ_WRITE}
      PKZL_VK_BINDING_OFFSET_SHADER_RESOURCE=${PKZL_VK_BINDING_OFFSET_SHADER_RESOURCE}
      $<$<BOOL:${PKZL_ENABLE_D3D12}>:PKZL_HAS_D3D12>
)

pikzel_set_compile_options(Pikzel)
set_target_properties(Pikzel PROPERTIES FOLDER "Engine")
