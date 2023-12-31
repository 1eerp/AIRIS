#pragma once
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "spdlog/sinks/stdout_color_sinks.h"

class Log
{
public:
	static void Init();

	inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_coreLogger; }

private:
	static std::shared_ptr<spdlog::logger> s_coreLogger;
};

#if _DEBUG 
// Core Logging Macros
#define CORE_TRACE(...)		Log::GetCoreLogger()->trace(__VA_ARGS__)
#define CORE_INFO(...)		Log::GetCoreLogger()->info(__VA_ARGS__)
#define CORE_WARN(...)		Log::GetCoreLogger()->warn(__VA_ARGS__)
#define CORE_ERROR(...)		Log::GetCoreLogger()->error(__VA_ARGS__)
#define CORE_CRITICAL(...)	Log::GetCoreLogger()->critical(__VA_ARGS__)

#else
#define CORE_TRACE(...)		
#define CORE_INFO(...)		
#define CORE_WARN(...)		
#define CORE_ERROR(...)		
#define CORE_CRITICAL(...)	
#endif