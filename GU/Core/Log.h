#pragma once
#include<memory>
#include"spdlog/spdlog.h"
#include"spdlog/sinks/stdout_color_sinks.h"
#include"Core.h"
namespace GU
{
    class Log
    {
    public:
        static void Init();
        static std::shared_ptr<spdlog::logger> Get();
    private:
        static std::shared_ptr<spdlog::logger> s_Log;
    };
}

#define GU_WARN(...)                GU::Log::Get()->warn(__VA_ARGS__)
#define GU_INFO(...)                GU::Log::Get()->info(__VA_ARGS__)
#define GU_ERROR(...)               SPDLOG_LOGGER_ERROR(GU::Log::Get(), __VA_ARGS__)
#define GU_SET_LOG_LEVEL(l)     GU::Log::Get()->set_level(spdlog::level::l);