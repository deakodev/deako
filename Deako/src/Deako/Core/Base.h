#pragma once

#include <memory>

// Platform
#define DK_PLATFORM_MAC
#include <signal.h>

// Macros & Helpers
#define BIT(x) (1 << x)

#define DK_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

template<typename T>
using Scope = std::unique_ptr<T>;
template<typename T, typename ... Args>
constexpr Scope<T> CreateScope(Args&& ... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

template<typename T>
using Ref = std::shared_ptr<T>;
template<typename T, typename ... Args>
constexpr Ref<T> CreateRef(Args&& ... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

// Asserts
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

