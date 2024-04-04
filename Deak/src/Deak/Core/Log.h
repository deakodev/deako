#pragma once

#include "dkpch.h"
#include "Deak/Core/Base.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace Deak {

    class Log
    {
    public:
        static void Init();

        inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
        inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

    private:
        static std::shared_ptr<spdlog::logger> s_CoreLogger;
        static std::shared_ptr<spdlog::logger> s_ClientLogger;
    };

}

//TODO: Strip out macros in dist build

// Core Log Macros
#define DK_CORE_CRITICAL(...)    ::Deak::Log::GetCoreLogger()->critical(__VA_ARGS__)
#define DK_CORE_ERROR(...)    ::Deak::Log::GetCoreLogger()->error(__VA_ARGS__)
#define DK_CORE_WARN(...)     ::Deak::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define DK_CORE_INFO(...)     ::Deak::Log::GetCoreLogger()->info(__VA_ARGS__)
#define DK_CORE_TRACE(...)    ::Deak::Log::GetCoreLogger()->trace(__VA_ARGS__)

// Client Log Macros
#define DK_CRITICAL(...)         ::Deak::Log::GetClientLogger()->critical(__VA_ARGS__)
#define DK_ERROR(...)         ::Deak::Log::GetClientLogger()->error(__VA_ARGS__)
#define DK_WARN(...)          ::Deak::Log::GetClientLogger()->warn(__VA_ARGS__)
#define DK_INFO(...)          ::Deak::Log::GetClientLogger()->info(__VA_ARGS__)
#define DK_TRACE(...)         ::Deak::Log::GetClientLogger()->trace(__VA_ARGS__)

