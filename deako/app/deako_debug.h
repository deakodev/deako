#pragma once

#include <memory>
#include <spdlog/logger.h>

namespace Deako {

	class DebugProvider
	{
	public:
		static void Init();

		static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:
		inline static std::shared_ptr<spdlog::logger> s_CoreLogger;
		inline static std::shared_ptr<spdlog::logger> s_ClientLogger;
	};

}

// Core Log Macros
#define DK_CORE_CRITICAL(...) ::Deako::DebugProvider::GetCoreLogger()->critical(__VA_ARGS__)
#define DK_CORE_ERROR(...)    ::Deako::DebugProvider::GetCoreLogger()->error(__VA_ARGS__)
#define DK_CORE_WARN(...)     ::Deako::DebugProvider::GetCoreLogger()->warn(__VA_ARGS__)
#define DK_CORE_INFO(...)     ::Deako::DebugProvider::GetCoreLogger()->info(__VA_ARGS__)
#define DK_CORE_TRACE(...)    ::Deako::DebugProvider::GetCoreLogger()->trace(__VA_ARGS__)

// Client Log Macros
#define DK_CRITICAL(...)      ::Deako::DebugProvider::GetClientLogger()->critical(__VA_ARGS__)
#define DK_ERROR(...)         ::Deako::DebugProvider::GetClientLogger()->error(__VA_ARGS__)
#define DK_WARN(...)          ::Deako::DebugProvider::GetClientLogger()->warn(__VA_ARGS__)
#define DK_INFO(...)          ::Deako::DebugProvider::GetClientLogger()->info(__VA_ARGS__)
#define DK_TRACE(...)         ::Deako::DebugProvider::GetClientLogger()->trace(__VA_ARGS__)

// Assert Macros
#ifdef DEBUG
#if defined(DK_PLATFORM_WINDOWS)
#define DK_DEBUGBREAK() __debugbreak()
#elif defined(DK_PLATFORM_MACOS)
#include <signal.h>
#define HZ_DEBUGBREAK() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet!"
#endif
#define DK_ENABLE_ASSERTS
#else
#define HZ_DEBUGBREAK()
#endif

#ifdef DK_ENABLE_ASSERTS
#define DK_ASSERT(x, ...) { if(!(x)) { DK_ERROR("Assertion Failed: {0}", __VA_ARGS__); DK_DEBUGBREAK(); } }
#define DK_CORE_ASSERT(x, ...) { if(!(x)) { DK_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); DK_DEBUGBREAK(); } }
#else
#define HZ_ASSERT(...)
#define HZ_CORE_ASSERT(...)
#endif