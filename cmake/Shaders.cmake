# Build-time shader compilation.
#
# Shaders are written in Slang and compiled to SPIR-V by slangc as part of the normal build,
# rather than at run time. Errors in shader code then show up as build errors, and programs
# start without paying for shader compilation. The trade-off is that shaders cannot yet be
# edited while a program is running; run-time compilation (for hot reload) can be added
# alongside this later, using the Slang library rather than the command-line compiler.
#
# Each .slang file becomes one .spv file holding every entry point in it (for example a
# vertex and a fragment shader). The entry points keep their Slang names, which the engine
# passes to NRI as the entry point name when creating a pipeline.

# Vulkan binding offsets.
#
# HLSL-style shaders bind resources to registers of four kinds: b (constant buffers),
# t (shader resources), u (read-write resources) and s (samplers). D3D12 keeps each kind in
# its own namespace, so b0, t0 and s0 can all exist at once. Vulkan has a single binding
# namespace per descriptor set, so the kinds are shifted into separate ranges: t0 becomes
# binding 128, b0 becomes binding 32, and so on.
#
# The shader compiler (when it assigns binding numbers) and NRI (when it builds the matching
# Vulkan descriptor set layouts) must use the same offsets. They are defined once here,
# passed to slangc below, and passed to the engine as compile definitions (see
# src/Pikzel/CMakeLists.txt).
set(PKZL_VK_BINDING_OFFSET_SAMPLER 0)
set(PKZL_VK_BINDING_OFFSET_CONSTANT_BUFFER 32)
set(PKZL_VK_BINDING_OFFSET_READ_WRITE 64)
set(PKZL_VK_BINDING_OFFSET_SHADER_RESOURCE 128)

# pikzel_add_shaders(<target> <source.slang>...)
#
# Compiles each Slang source file into <runtime output directory>/Shaders/<name>.spv and
# makes <target> depend on the results, so the shaders are rebuilt whenever their sources
# (or any file they import) change.
function(pikzel_add_shaders target)
   if(CMAKE_RUNTIME_OUTPUT_DIRECTORY)
      set(outputDirectory "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shaders")
   else()
      set(outputDirectory "${CMAKE_CURRENT_BINARY_DIR}/Shaders")
   endif()

   set(outputs "")
   foreach(source IN LISTS ARGN)
      cmake_path(ABSOLUTE_PATH source NORMALIZE OUTPUT_VARIABLE sourcePath)
      cmake_path(GET source STEM name)
      set(output "${outputDirectory}/${name}.spv")
      set(depfile "${CMAKE_CURRENT_BINARY_DIR}/Shaders/${name}.spv.d")

      add_custom_command(
         OUTPUT "${output}"
         COMMAND "${CMAKE_COMMAND}" -E make_directory "${outputDirectory}"
         COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/Shaders"
         COMMAND
            "${PKZL_SLANGC_EXECUTABLE}" "${sourcePath}"
            -target spirv
            -fvk-use-entrypoint-name
            -fvk-s-shift ${PKZL_VK_BINDING_OFFSET_SAMPLER} all
            -fvk-b-shift ${PKZL_VK_BINDING_OFFSET_CONSTANT_BUFFER} all
            -fvk-u-shift ${PKZL_VK_BINDING_OFFSET_READ_WRITE} all
            -fvk-t-shift ${PKZL_VK_BINDING_OFFSET_SHADER_RESOURCE} all
            -g
            -o "${output}"
            -depfile "${depfile}"
         DEPENDS "${sourcePath}"
         DEPFILE "${depfile}"
         COMMENT "Compiling shader ${source}"
         VERBATIM
      )
      list(APPEND outputs "${output}")
   endforeach()

   # The .slang files are listed as (non-compiled) sources so they show up in IDE projects.
   target_sources(${target} PRIVATE ${ARGN})
   set_source_files_properties(${ARGN} PROPERTIES HEADER_FILE_ONLY ON)

   add_custom_target(${target}Shaders DEPENDS ${outputs})
   set_target_properties(${target}Shaders PROPERTIES FOLDER "Shaders")
   add_dependencies(${target} ${target}Shaders)
endfunction()
