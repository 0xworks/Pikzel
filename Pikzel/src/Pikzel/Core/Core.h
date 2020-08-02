#include "Instrumentor.h"

#ifdef ENABLE_ASSERTS
#define CORE_ASSERT(x, ...) { if(!(x)) { CORE_LOG_ERROR(__VA_ARGS__); __debugbreak(); } }
#define ASSERT(x, ...) { if(!(x)) { LOG_ERROR(__VA_ARGS__); __debugbreak(); } }
#else
#define CORE_ASSERT(x, ...)
#define ASSERT(x, ...)
#endif
