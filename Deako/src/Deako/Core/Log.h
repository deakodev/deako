#pragma once

#include "dkpch.h"
#include "Deako/Core/Base.h"
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace Deako {

    class Log
    {
    public:
        static void Init();

        static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
        static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

    private:
        static Ref<spdlog::logger> s_CoreLogger;
        static Ref<spdlog::logger> s_ClientLogger;
    };

}

// TODO: Strip out macros in dist build

// Core Log Macros
#define DK_CORE_CRITICAL(...) ::Deako::Log::GetCoreLogger()->critical(__VA_ARGS__)
#define DK_CORE_ERROR(...)    ::Deako::Log::GetCoreLogger()->error(__VA_ARGS__)
#define DK_CORE_WARN(...)     ::Deako::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define DK_CORE_INFO(...)     ::Deako::Log::GetCoreLogger()->info(__VA_ARGS__)
#define DK_CORE_TRACE(...)    ::Deako::Log::GetCoreLogger()->trace(__VA_ARGS__)

// Client Log Macros
#define DK_CRITICAL(...)      ::Deako::Log::GetClientLogger()->critical(__VA_ARGS__)
#define DK_ERROR(...)         ::Deako::Log::GetClientLogger()->error(__VA_ARGS__)
#define DK_WARN(...)          ::Deako::Log::GetClientLogger()->warn(__VA_ARGS__)
#define DK_INFO(...)          ::Deako::Log::GetClientLogger()->info(__VA_ARGS__)
#define DK_TRACE(...)         ::Deako::Log::GetClientLogger()->trace(__VA_ARGS__)

