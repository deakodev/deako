#pragma once

#include <memory>
#include "Core.h"
#include "spdlog/spdlog.h"

namespace Deak {

    class DEAK_API Log
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
#define DZ_CORE_FATAL(...)    ::Deak::Log::GetCoreLogger()->fatal(__VA_ARGS__)
#define DZ_CORE_ERROR(...)    ::Deak::Log::GetCoreLogger()->error(__VA_ARGS__)
#define DZ_CORE_WARN(...)     ::Deak::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define DZ_CORE_INFO(...)     ::Deak::Log::GetCoreLogger()->info(__VA_ARGS__)
#define DZ_CORE_TRACE(...)    ::Deak::Log::GetCoreLogger()->trace(__VA_ARGS__)

// Client Log Macros
#define DZ_FATAL(...)         ::Deak::Log::GetClientLogger()->fatal(__VA_ARGS__)
#define DZ_ERROR(...)         ::Deak::Log::GetClientLogger()->error(__VA_ARGS__)
#define DZ_WARN(...)          ::Deak::Log::GetClientLogger()->warn(__VA_ARGS__)
#define DZ_INFO(...)          ::Deak::Log::GetClientLogger()->info(__VA_ARGS__)
#define DZ_TRACE(...)         ::Deak::Log::GetClientLogger()->trace(__VA_ARGS__)

