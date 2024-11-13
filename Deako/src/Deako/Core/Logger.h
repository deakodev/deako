#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace Deako {

    struct DkLogger
    {
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;

        static void Init();

        static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
        static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
    };

}

// Core Log Macros
#define DK_CORE_CRITICAL(...) ::Deako::DkLogger::GetCoreLogger()->critical(__VA_ARGS__)
#define DK_CORE_ERROR(...)    ::Deako::DkLogger::GetCoreLogger()->error(__VA_ARGS__)
#define DK_CORE_WARN(...)     ::Deako::DkLogger::GetCoreLogger()->warn(__VA_ARGS__)
#define DK_CORE_INFO(...)     ::Deako::DkLogger::GetCoreLogger()->info(__VA_ARGS__)
#define DK_CORE_TRACE(...)    ::Deako::DkLogger::GetCoreLogger()->trace(__VA_ARGS__)

// Client Log Macros
#define DK_CRITICAL(...)      ::Deako::DkLogger::GetClientLogger()->critical(__VA_ARGS__)
#define DK_ERROR(...)         ::Deako::DkLogger::GetClientLogger()->error(__VA_ARGS__)
#define DK_WARN(...)          ::Deako::DkLogger::GetClientLogger()->warn(__VA_ARGS__)
#define DK_INFO(...)          ::Deako::DkLogger::GetClientLogger()->info(__VA_ARGS__)
#define DK_TRACE(...)         ::Deako::DkLogger::GetClientLogger()->trace(__VA_ARGS__)
