# Compiler settings shared by every Pikzel target.
#
# Applied per target (rather than through global CMAKE_CXX_FLAGS) so that third-party
# code pulled in by Dependencies.cmake is not built with our warning level.

function(pikzel_set_compile_options target)
   target_compile_features(${target} PUBLIC cxx_std_23)

   if(MSVC)
      target_compile_options(
         ${target}
         PRIVATE
            /W4             # high warning level
            /permissive-    # standards-conforming mode
            /utf-8          # source and execution character sets are UTF-8
            /Zc:__cplusplus # report the real language version in __cplusplus
            /Zc:preprocessor # standards-conforming preprocessor
            /MP             # parallel compilation within a target (MSBuild generators)
      )
      target_compile_definitions(
         ${target}
         PRIVATE
            NOMINMAX            # stop <windows.h> defining min/max macros
            WIN32_LEAN_AND_MEAN # trim rarely used parts of <windows.h>
      )
   else()
      target_compile_options(
         ${target}
         PRIVATE
            -Wall
            -Wextra
            -Wpedantic
            -Wshadow
            -Wconversion
      )
   endif()
endfunction()
