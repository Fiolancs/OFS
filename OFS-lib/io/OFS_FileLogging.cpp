#include "OFS_FileLogging.h"

#include "OFS_Profiling.h"
#include "localization/OFS_Localization.h"
#include "localization/OFS_StringsGenerated.h"

#include <mutex>
#include <chrono>
#include <atomic>
#include <ranges>
#include <format>
#include <thread>
#include <fstream>
#include <filesystem>
#include <string_view>

namespace
{
    struct OFS_LogThreadImpl
    {
        std::string messageBuffer;
        std::mutex  messageBufferMtx;

        std::ofstream logFile;

        std::atomic<bool> shouldExit;
        std::atomic<bool> shouldFlush;
    };

    void logThreadFunction(std::shared_ptr<OFS_LogThreadImpl> threadData)
    {
        std::string logBuffer{};
        logBuffer.reserve(4096);

        while (!threadData->logFile.fail() && !threadData->shouldExit.load(std::memory_order_relaxed))
        {
            std::atomic_wait_explicit(&threadData->shouldFlush, false, std::memory_order_acquire);
            {
                std::lock_guard _{ threadData->messageBufferMtx };
                threadData->shouldFlush.store(false, std::memory_order_relaxed);
                logBuffer.swap(threadData->messageBuffer);
            }

            if (!logBuffer.empty() && !threadData->logFile.fail())
            {
                threadData->logFile.write(logBuffer.data(), logBuffer.size());
                logBuffer.clear();
            }
        }
    }

    inline static void appendToBuffer(auto& buffer, std::ranges::range auto&& message)
    {
        OFS_PROFILE(__FUNCTION__);
        buffer.append_range(message);
    };

}

 OFS::FileLogger OFS::FileLogger::instance = {};

struct OFS::FileLogger::PImpl
{
    std::shared_ptr<OFS_LogThreadImpl> threadData;
    std::thread logThread;
};

OFS::FileLogger::FileLogger(void) noexcept
    : pImpl{ std::make_unique<PImpl>(std::make_shared<OFS_LogThreadImpl>()) }
{
}

bool OFS::FileLogger::init(std::string_view logFile)
{
    if (pImpl->threadData->logFile = std::ofstream{ std::filesystem::path(logFile) }; pImpl->threadData->logFile)
    {
        pImpl->threadData->messageBuffer.reserve(4096);
        pImpl->logThread = std::thread(logThreadFunction, pImpl->threadData);

        return true;
    }
    return false;
}

void OFS::FileLogger::shutdown(void) noexcept
{
    if (pImpl->logThread.joinable())
    {
        pImpl->threadData->shouldExit.store(true, std::memory_order_relaxed);
        logToFile(LogLevel::OFS_LOG_INFO, "File log shutdown requested");
        flush();

        pImpl->logThread.join();
    }
}

OFS::FileLogger& OFS::FileLogger::get(void) noexcept
{
    return instance;
}

void OFS::FileLogger::addListener(LogListener_t* const listener)
{
    // TODO
    
}

void OFS::FileLogger::flush(void) noexcept
{
    if (!pImpl->threadData->messageBuffer.empty())
    {
        pImpl->threadData->shouldFlush.store(true, std::memory_order_release);
        pImpl->threadData->shouldFlush.notify_one();
    }
}

void OFS::FileLogger::logToFile(LogLevel level, std::string_view msg, bool newLine)
{
    return logToFile(makePrefix(level), msg, newLine);
}

void OFS::FileLogger::logToFile(std::string_view prefix, std::string_view msg, bool newLine)
{
    OFS_PROFILE(__FUNCTION__);

    using std::literals::operator ""sv;
    auto const full = { prefix, msg, newLine ? "\n"sv : ""sv };
    {
        std::lock_guard _{ pImpl->threadData->messageBufferMtx };
        appendToBuffer(pImpl->threadData->messageBuffer, full | std::views::join);
    }
}

std::string OFS::FileLogger::makePrefix(LogLevel level) noexcept
{
    auto const time_now = std::chrono::current_zone()->to_local(std::chrono::system_clock::now());

    switch (level)
    {
    case LogLevel::OFS_LOG_INFO:  return std::format("[{:%F %X}][INFO ] ", time_now);
    case LogLevel::OFS_LOG_WARN:  return std::format("[{:%F %X}][WARN ] ", time_now);
    case LogLevel::OFS_LOG_DEBUG: return std::format("[{:%F %X}][DEBUG] ", time_now);
    case LogLevel::OFS_LOG_ERROR: return std::format("[{:%F %X}][ERROR] ", time_now);
    default:                      return std::format("[{:%F %X}][-----] ", time_now);
    }
}

//static OFS::AppLog OFS_MainLog;
// 
//void OFS_FileLogger::DrawLogWindow(bool* open) noexcept
//{
//    if (!*open) return;
//    OFS_MainLog.Draw(TR_ID("OFS_LOG_OUTPUT", Tr::OFS_LOG_OUTPUT).c_str(), open);
//}
//
//inline static void LogToConsole(OFS_LogLevel level, const char* msg) noexcept
//{
//    OFS_PROFILE(__FUNCTION__);
//    switch (level) {
//        case OFS_LogLevel::OFS_LOG_INFO:
//            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, msg);
//            //OFS_MainLog.AddLog("[INFO]: %s\n", msg);
//            break;
//        case OFS_LogLevel::OFS_LOG_WARN:
//            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, msg);
//            //OFS_MainLog.AddLog("[WARN]: %s\n", msg);
//            break;
//        case OFS_LogLevel::OFS_LOG_DEBUG:
//            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, msg);
//            //OFS_MainLog.AddLog("[DEBUG]: %s\n", msg);
//            break;
//        case OFS_LogLevel::OFS_LOG_ERROR:
//            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, msg);
//            //OFS_MainLog.AddLog("[ERROR]: %s\n", msg);
//            break;
//    }
//}
