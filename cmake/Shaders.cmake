# Build-time shader compilation.
#
# Shaders are written in Slang and compiled by slangc as part of the normal build, rather
# than at run time. Errors in shader code then show up as build errors, and programs start
# without paying for shader compilation. The trade-off is that shaders cannot yet be edited
# while a program is running; run-time compilation (for hot reload) can be added alongside
# this later, using the Slang library rather than the command-line compiler.
#
# Each entry point is compiled into a file of its own, named <source>.<entry point>.<ext>:
# .spv (SPIR-V) for Vulkan, and, when PKZL_ENABLE_D3D12 is on, also .dxil for D3D12. One
# file per entry point is forced by D3D12. A DXIL file holding several entry points is a
# library, which only ray tracing and work graph pipelines can use; a graphics pipeline
# needs one compiled shader per stage. SPIR-V has no such limit, but using the same layout
# for both keeps the engine's shader loading the same for every graphics API.

# Vulkan binding offsets.
#
# HLSL-style shaders bind resources to registers of four kinds: b (constant buffers),
# t (shader resources), u (read-write resources) and s (samplers). D3D12 keeps each kind in
# its own namespace, so b0, t0 and s0 can all exist at once. Vulkan has a single binding
# namespace per descriptor set, so the kinds are shifted into separate ranges: t0 becomes
# binding 128, b0 becomes binding 32, and so on. D3D12 needs no shifting, so these only
# apply to SPIR-V.
#
# The shader compiler (when it assigns binding numbers) and NRI (when it builds the matching
# Vulkan descriptor set layouts) must use the same offsets. They are defined once here,
# passed to slangc below, and passed to the engine as compile definitions (see
# src/Pikzel/CMakeLists.txt).
set(PKZL_VK_BINDING_OFFSET_SAMPLER 0)
set(PKZL_VK_BINDING_OFFSET_CONSTANT_BUFFER 32)
set(PKZL_VK_BINDING_OFFSET_READ_WRITE 64)
set(PKZL_VK_BINDING_OFFSET_SHADER_RESOURCE 128)

# D3D12 shader model. 6.6 is widely supported by current GPU drivers, and is the first to
# allow direct bindless resource access (ResourceDescriptorHeap), which the renderer is
# likely to want. Lower it if older hardware must be supported.
set(PKZL_D3D12_SHADER_MODEL sm_6_6)

# Adds a custom command that compiles one entry point of a Slang source file for one
# target (spirv or dxil). Appends the output file to the variable named by outputsVariable.
function(_pikzel_compile_shader_entry_point sourcePath entryPoint target outputPath outputsVariable)
   cmake_path(GET outputPath FILENAME outputName)
   set(depfile "${CMAKE_CURRENT_BINARY_DIR}/Shaders/${outputName}.d")

   if(target STREQUAL "spirv")
      set(
         targetArguments
         -target spirv
         -fvk-use-entrypoint-name
         -fvk-s-shift ${PKZL_VK_BINDING_OFFSET_SAMPLER} all
         -fvk-b-shift ${PKZL_VK_BINDING_OFFSET_CONSTANT_BUFFER} all
         -fvk-u-shift ${PKZL_VK_BINDING_OFFSET_READ_WRITE} all
         -fvk-t-shift ${PKZL_VK_BINDING_OFFSET_SHADER_RESOURCE} all
      )
   elseif(target STREQUAL "dxil")
      set(
         targetArguments
         -target dxil
         -profile ${PKZL_D3D12_SHADER_MODEL}
         -dxc-path "${PKZL_DXC_DIRECTORY}"
      )
   else()
      message(FATAL_ERROR "Unknown shader target: ${target}")
   endif()

   cmake_path(GET outputPath PARENT_PATH outputDirectory)

   # No -stage is given: slangc takes each entry point's stage from its [shader("...")]
   # attribute in the source.
   #
   # Matrices in buffers are read row by row (-matrix-layout-row-major), matching how RTM
   # stores matrices on the CPU. See src/Pikzel/Renderer/ShaderTypes.h.
   add_custom_command(
      OUTPUT "${outputPath}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${outputDirectory}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/Shaders"
      COMMAND
         "${PKZL_SLANGC_EXECUTABLE}" "${sourcePath}"
         -entry ${entryPoint}
         ${targetArguments}
         -matrix-layout-row-major
         -g
         -o "${outputPath}"
         -depfile "${depfile}"
      DEPENDS "${sourcePath}"
      DEPFILE "${depfile}"
      COMMENT "Compiling shader ${outputName}"
      VERBATIM
   )

   set(outputs ${${outputsVariable}})
   list(APPEND outputs "${outputPath}")
   set(${outputsVariable} ${outputs} PARENT_SCOPE)
endfunction()

# pikzel_add_shader(<target> <source.slang> ENTRY_POINTS <name>...)
#
# Compiles the named entry points of a Slang source file into
# <runtime output directory>/Shaders/<source>.<entry point>.spv (and .dxil when D3D12 is
# enabled), and makes <target> depend on the results, so the shaders are rebuilt whenever
# their source (or any file it imports) changes. For example
#
#    pikzel_add_shader(HelloTriangle Triangle.slang ENTRY_POINTS VertexMain FragmentMain)
#
# produces Triangle.VertexMain.spv and Triangle.FragmentMain.spv, which are loaded with
# Shader(device, "Triangle", "VertexMain") and Shader(device, "Triangle", "FragmentMain").
function(pikzel_add_shader target source)
   cmake_parse_arguments(PARSE_ARGV 2 arg "" "" "ENTRY_POINTS")
   if(NOT arg_ENTRY_POINTS)
      message(FATAL_ERROR "pikzel_add_shader(${target} ${source}): ENTRY_POINTS must list at least one entry point")
   endif()

   if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
      set(outputDirectory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shaders")
   else()
      set(outputDirectory "${CMAKE_CURRENT_BINARY_DIR}/Shaders")
   endif()

   cmake_path(ABSOLUTE_PATH source NORMALIZE OUTPUT_VARIABLE sourcePath)
   cmake_path(GET source STEM name)

   set(outputs "")
   foreach(entryPoint IN LISTS arg_ENTRY_POINTS)
      _pikzel_compile_shader_entry_point("${sourcePath}" ${entryPoint} spirv "${outputDirectory}/${name}.${entryPoint}.spv" outputs)
      if(PKZL_ENABLE_D3D12)
         _pikzel_compile_shader_entry_point("${sourcePath}" ${entryPoint} dxil "${outputDirectory}/${name}.${entryPoint}.dxil" outputs)
      endif()
   endforeach()

   # The .slang file is listed as a (non-compiled) source so it shows up in IDE projects.
   target_sources(${target} PRIVATE ${source})
   set_source_files_properties(${source} PROPERTIES HEADER_FILE_ONLY ON)

   set(shaderTarget ${target}_${name}_Shaders)
   add_custom_target(${shaderTarget} DEPENDS ${outputs})
   set_target_properties(${shaderTarget} PROPERTIES FOLDER "Shaders")
   add_dependencies(${target} ${shaderTarget})
endfunction()
