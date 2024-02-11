# pragma once

#ifdef DK_PLATFORM_MAC
// #define DEAK_API __attribute__((visibility("default")))
#include <signal.h>
#define DK_DEBUGBREAK() raise(SIGTRAP)
#else
#error Deak only supports MacOS
#endif


#ifdef DK_ENABLE_ASSERTS
#define DK_ASSERT(x, ...) { if(!(x)) { DK_ERROR("Assertion Failed: {0}", __VA_ARGS__); raise(SIGTRAP); } }
#define DK_CORE_ASSERT(x, ...) { if(!(x)) { DK_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); raise(SIGTRAP); } }
#else
#define DK_ASSERT(x, ...) 
#define DK_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)
