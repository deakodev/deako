#pragma once

#include <memory>

#ifdef DK_PLATFORM_MAC
// For shared lib, if we decide to change from static
// #define DEAK_API __attribute__((visibility("default")))
#include <signal.h>
#define DK_DEBUGBREAK() raise(SIGTRAP)
#else
#error Deak only supports MacOS
#endif

#ifdef DK_DEBUG
#define DK_ENABLE_ASSERTS
#endif

#ifdef DK_ENABLE_ASSERTS
#define DK_ASSERT(x, ...) { if(!(x)) { DK_ERROR("Assertion Failed: {0}", __VA_ARGS__); raise(SIGTRAP); } }
#define DK_CORE_ASSERT(x, ...) { if(!(x)) { DK_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); raise(SIGTRAP); } }
#else
#define DK_ASSERT(x, ...) 
#define DK_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define DK_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)

namespace Deak {

    template <typename T>
    using Ref = std::shared_ptr<T>;

    template <typename T>
    using Scope = std::unique_ptr<T>;

}
