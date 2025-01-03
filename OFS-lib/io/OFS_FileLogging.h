#pragma once

#include <format>
#include <cstdint>
#include <string_view>

namespace OFS
{
    class FileLogger
    {
    public:
        enum class LogLevel : std::int32_t
        {
            OFS_LOG_INFO,
            OFS_LOG_WARN,
            OFS_LOG_DEBUG,
            OFS_LOG_ERROR,
        };
        using enum LogLevel;

        bool init(std::string_view logFile);
        void shutdown(void) noexcept;
        
        static FileLogger& get(void) noexcept;

        using LogListener_t = void(LogLevel, std::string_view log);
        void addListener(LogListener_t* const listener);

        void flush(void) noexcept;

        void logToFile(LogLevel level, std::string_view msg, bool newLine = true);
        void logToFile(std::string_view prefix, std::string_view msg, bool newLine = true);

        template <typename ... T>
        void logToFile(LogLevel level, std::string_view fmt, T&& ... args)
        {
            return logToFile(makePrefix(level), std::vformat(fmt, std::make_format_args(args ...)));
        }

        ~FileLogger(void) noexcept;

    private:
        static std::string makePrefix(LogLevel) noexcept;

        // Singleton
        FileLogger(void) noexcept;
        FileLogger& operator = (FileLogger&&) = delete;
        FileLogger& operator = (FileLogger const&) = delete;

        struct PImpl;
        std::unique_ptr<PImpl> pImpl;
    };
}


#ifndef NDEBUG
#define LOG_INFO(msg)  OFS::FileLogger::get().logToFile(OFS::FileLogger::OFS_LOG_INFO,  msg)
#define LOG_WARN(msg)  OFS::FileLogger::get().logToFile(OFS::FileLogger::OFS_LOG_WARN,  msg)
#define LOG_DEBUG(msg) OFS::FileLogger::get().logToFile(OFS::FileLogger::OFS_LOG_DEBUG, msg)
#define LOG_ERROR(msg) OFS::FileLogger::get().logToFile(OFS::FileLogger::OFS_LOG_ERROR, msg)

#define LOGF_INFO(fmt, ...)  OFS::FileLogger::get().logToFile(OFS::FileLogger::OFS_LOG_INFO,  fmt, __VA_ARGS__)
#define LOGF_WARN(fmt, ...)  OFS::FileLogger::get().logToFile(OFS::FileLogger::OFS_LOG_WARN,  fmt, __VA_ARGS__)
#define LOGF_DEBUG(fmt, ...) OFS::FileLogger::get().logToFile(OFS::FileLogger::OFS_LOG_DEBUG, fmt, __VA_ARGS__)
#define LOGF_ERROR(fmt, ...) OFS::FileLogger::get().logToFile(OFS::FileLogger::OFS_LOG_ERROR, fmt, __VA_ARGS__)
#else
#define LOG_INFO(msg)
#define LOG_WARN(msg)  OFS::FileLogger::get().logToFile(OFS::FileLogger::OFS_LOG_WARN,  msg)
#define LOG_DEBUG(msg)
#define LOG_ERROR(msg) OFS::FileLogger::get().logToFile(OFS::FileLogger::OFS_LOG_ERROR, msg)

#define LOGF_INFO(fmt, ...)
#define LOGF_WARN(fmt, ...)  OFS::FileLogger::get().logToFile(OFS::FileLogger::OFS_LOG_WARN,  fmt, __VA_ARGS__)
#define LOGF_DEBUG(fmt, ...)
#define LOGF_ERROR(fmt, ...) OFS::FileLogger::get().logToFile(OFS::FileLogger::OFS_LOG_ERROR, fmt, __VA_ARGS__)
#endif
