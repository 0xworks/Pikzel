#pragma once

// Platform detection
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
   #define PKZL_PLATFORM_WINDOWS
   #ifdef _WIN64
   #else
      #error "x86 Builds are not supported!"
   #endif
#elif __APPLE__
   #error "Apple platforms are not currently supported!"
#elif __linux__
   #define PKZL_PLATFORM_LINUX
#else
   #error "Unrecognised platform!"
#endif

#if defined(PKZL_PLATFORM_WINDOWS)
   #ifdef PKZL_BUILD_DLL
      #define PKZL_API __declspec(dllexport)
      #define ENTT_API_EXPORT
   #else
      #define PKZL_API __declspec(dllimport)
      #define ENTT_API_IMPORT
   #endif
   #define CDECL __cdecl
#else
   #define PKZL_API
   #define CDECL
#endif

#if defined(PKZL_DEBUG)
   #if defined(PKZL_PLATFORM_WINDOWS)
      #define PKZL_DEBUG_BREAK __debugbreak()
   #elif defined(PKZL_PLATFORM_LINUX)
      #include <signal.h>
      #define PKZL_DEBUG_BREAK raise(SIGTRAP)
   #else
      #define PKZL_DEBUG_BREAK
   #endif
   #define PKZL_ENABLE_ASSERTS
#endif

#include "Log.h"
#include "Instrumentor.h"

#if defined(PKZL_ENABLE_ASSERTS)
   #define PKZL_CORE_ASSERT(x, ...) { if(!(x)) { PKZL_CORE_LOG_ERROR(__VA_ARGS__); PKZL_DEBUG_BREAK; } }
   #define PKZL_ASSERT(x, ...) { if(!(x)) { PKZL_LOG_ERROR(__VA_ARGS__); PKZL_DEBUG_BREAK; } }
#else
   #define PKZL_CORE_ASSERT(x, ...)
   #define PKZL_ASSERT(x, ...)
#endif

#if defined(_MSC_VER)
   #define PKZL_FUNCSIG __FUNCSIG__
#else
   #define PKZL_FUNCSIG __func__
#endif

#define PKZL_NOT_IMPLEMENTED throw std::logic_error {PKZL_FUNCSIG + std::string(" is not implemented")}

#define PKZL_NO_COPY(T) T(const T&) = delete;            \
                        T& operator=(const T&) = delete;

#define PKZL_NO_MOVE(T) T(T&&) = delete;            \
                        T& operator=(T&&) = delete;

#define PKZL_NO_COPYMOVE(T) T(const T&) = delete;            \
                            T(T&&) = delete;                 \
                            T& operator=(const T&) = delete; \
                            T& operator=(T&&) = delete;

#include <entt/core/hashed_string.hpp>
#include <entt/entity/entity.hpp>
#include <entt/entity/fwd.hpp>

using namespace entt::literals;

namespace Pikzel {
   using Id = entt::id_type;
   using Object = entt::entity;
   inline constexpr entt::null_t Null {};
}

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
