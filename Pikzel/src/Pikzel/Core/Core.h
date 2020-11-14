#pragma once

// Platform detection
#ifdef _WIN32
	/* Windows x64/x86 */
#ifdef _WIN64
	/* Windows x64  */
#define PKZL_PLATFORM_WINDOWS
#else
	/* Windows x86 */
#error "x86 Builds are not supported!"
#endif
#endif

// TODO: other platforms here...

#ifndef PKZL_PLATFORM_WINDOWS
#error "Pikzel is not supported on non-windows platforms yet!"
#endif

#ifdef PKZL_PLATFORM_WINDOWS
#ifdef PKZL_BUILD_DLL
#define PKZL_API __declspec(dllexport)
#else
#define PKZL_API __declspec(dllimport)
#endif
#endif

#ifdef PKZL_DEBUG
#ifdef PKZL_PLATFORM_WINDOWS
#define PKZL_DEBUG_BREAK __debugbreak()
#else
#define PKZL_DEBUG_BREAK
#endif
#define PKZL_ENABLE_ASSERTS
#endif

#include "Log.h"
#include "Instrumentor.h"

#ifdef PKZL_ENABLE_ASSERTS
#define PKZL_CORE_ASSERT(x, ...) { if(!(x)) { PKZL_CORE_LOG_ERROR(__VA_ARGS__); PKZL_DEBUG_BREAK; } }
#define PKZL_ASSERT(x, ...) { if(!(x)) { PKZL_LOG_ERROR(__VA_ARGS__); PKZL_DEBUG_BREAK; } }
#else
#define PKZL_CORE_ASSERT(x, ...)
#define PKZL_ASSERT(x, ...)
#endif

#ifdef _MSC_VER
#define PKZL_FUNCSIG __FUNCSIG__
#else
#define PKZL_FUNCSIG __func__
#endif

#define PKZL_NOT_IMPLEMENTED throw std::logic_error {PKZL_FUNCSIG + std::string(" is not implemented")}

#include <entt/core/hashed_string.hpp>

namespace std {
   template<typename T1, typename T2>
   struct hash<std::pair<T1, T2>> {
      size_t operator()(const std::pair<T1, T2>& pair) const {
         auto hash1 = std::hash<T1> {};
         auto hash2 = std::hash<T2> {};
         size_t seed = hash1(pair.first);
         seed ^= hash2(pair.second) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
         return seed;
      }
   };
}
