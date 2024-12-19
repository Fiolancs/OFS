#pragma once

#include <format>
#include <cstdint>
#include <string_view>

enum class OFS_LogLevel : int32_t {
    OFS_LOG_INFO,
    OFS_LOG_WARN,
    OFS_LOG_DEBUG,
    OFS_LOG_ERROR,
};

class OFS_FileLogger
{
public:
    static constexpr int MaxLogThreads = 1;
    static struct SDL_IOStream* LogFileHandle;

    static void Init() noexcept;
    static void Shutdown() noexcept;

    static void Flush() noexcept;
    static void DrawLogWindow(bool* open) noexcept;

    [[deprecated]]
    static void LogToFileR(const char* prefix, const char* msg, bool newLine = true) noexcept;
    [[deprecated]]
    static void LogToFileR(OFS_LogLevel level, const char* msg, uint32_t size = 0, bool newLine = true) noexcept;

    static void LogToFileR(OFS_LogLevel level, std::string_view msg, bool newLine = true) noexcept;
    static void LogToFileR(std::string_view prefix, std::string_view msg, bool newLine = true) noexcept;

    template <typename ... T>
    static void LogToFileF(OFS_LogLevel level, const char* fmt, T&&... argv)
    {
        LogToFileR(level, std::vformat(fmt, std::make_format_args(argv...)));
    }
};

#ifndef NDEBUG
#define LOG_INFO(msg) OFS_FileLogger::LogToFileR(OFS_LogLevel::OFS_LOG_INFO, msg)
#define LOG_WARN(msg) OFS_FileLogger::LogToFileR(OFS_LogLevel::OFS_LOG_WARN, msg)
#define LOG_DEBUG(msg) OFS_FileLogger::LogToFileR(OFS_LogLevel::OFS_LOG_DEBUG, msg)
#define LOG_ERROR(msg) OFS_FileLogger::LogToFileR(OFS_LogLevel::OFS_LOG_ERROR, msg)

#define LOGF_INFO(fmt, ...) OFS_FileLogger::LogToFileF(OFS_LogLevel::OFS_LOG_INFO, fmt, __VA_ARGS__)
#define LOGF_WARN(fmt, ...) OFS_FileLogger::LogToFileF(OFS_LogLevel::OFS_LOG_WARN, fmt, __VA_ARGS__)
#define LOGF_DEBUG(fmt, ...) OFS_FileLogger::LogToFileF(OFS_LogLevel::OFS_LOG_DEBUG, fmt, __VA_ARGS__)
#define LOGF_ERROR(fmt, ...) OFS_FileLogger::LogToFileF(OFS_LogLevel::OFS_LOG_ERROR, fmt, __VA_ARGS__)
#else
#define LOG_INFO(msg) OFS_FileLogger::LogToFileR(OFS_LogLevel::OFS_LOG_INFO, msg)
#define LOG_WARN(msg) OFS_FileLogger::LogToFileR(OFS_LogLevel::OFS_LOG_WARN, msg)
#define LOG_DEBUG(msg)
#define LOG_ERROR(msg) OFS_FileLogger::LogToFileR(OFS_LogLevel::OFS_LOG_ERROR, msg)

#define LOGF_INFO(fmt, ...) OFS_FileLogger::LogToFileF(OFS_LogLevel::OFS_LOG_INFO, fmt, __VA_ARGS__)
#define LOGF_WARN(fmt, ...) OFS_FileLogger::LogToFileF(OFS_LogLevel::OFS_LOG_WARN, fmt, __VA_ARGS__)
#define LOGF_DEBUG(fmt, ...)
#define LOGF_ERROR(fmt, ...) OFS_FileLogger::LogToFileF(OFS_LogLevel::OFS_LOG_ERROR, fmt, __VA_ARGS__)
#endif
