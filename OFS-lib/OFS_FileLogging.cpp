#include "OFS_FileLogging.h"

#include "OFS_Util.h"
#include "UI/OFS_ImGui.h"
#include "UI/OFS_Profiling.h"
#include "localization/OFS_Localization.h"
#include "localization/OFS_StringsGenerated.h"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_mutex.h>
#include <SDL3/SDL_atomic.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_IOStream.h>

#include <atomic>
#include <vector>
#include <format>
#include <ranges>
#include <filesystem>
#include <string_view>

static OFS::AppLog OFS_MainLog;

SDL_IOStream* OFS_FileLogger::LogFileHandle = nullptr;

struct OFS_LogThread {
    SDL_SpinLock lock = {};
    SDL_Condition* WaitFlush = nullptr;
    std::vector<char> LogMsgBuffer;

    std::atomic<bool> ShouldExit = false;
    std::atomic<bool> Exited = false;

    void Init() noexcept
    {
        WaitFlush = SDL_CreateCondition();
        LogMsgBuffer.reserve(4096);
    }

    void Shutdown() noexcept
    {
        ShouldExit = true;
        SDL_SignalCondition(WaitFlush);
        while (!Exited) {
            SDL_Delay(1);
        }
        SDL_DestroyCondition(WaitFlush);
    }
};

static OFS_LogThread Thread;

static int LogThreadFunction(void* threadData) noexcept
{
    auto& thread = *(OFS_LogThread*)threadData;
    auto& msg = thread.LogMsgBuffer;
    auto waitMut = SDL_CreateMutex();
    SDL_LockMutex(waitMut);
    while (!thread.ShouldExit && OFS_FileLogger::LogFileHandle) {
        SDL_WaitCondition(thread.WaitFlush, waitMut);
        {
            SDL_LockSpinlock(&thread.lock);
            SDL_WriteIO(OFS_FileLogger::LogFileHandle, msg.data(), msg.size());
            msg.clear();
            SDL_UnlockSpinlock(&thread.lock);
        }
    }

    SDL_DestroyMutex(waitMut);
    thread.Exited.store(true, std::memory_order_release);

    return 0;
}

void OFS_FileLogger::Init() noexcept
{
    if (LogFileHandle) return;
#ifndef NDEBUG
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
#endif
    auto LogFilePath = Util::Prefpath("OFS.log");
    LogFileHandle = SDL_IOFromFile(std::filesystem::path(LogFilePath).string().c_str(), "w");

    Thread.Init();
    auto t = SDL_CreateThread(LogThreadFunction, "MessageLogging", &Thread);
    SDL_DetachThread(t);
}

void OFS_FileLogger::Shutdown() noexcept
{
    if (!LogFileHandle) return;
    Thread.Shutdown();
    SDL_CloseIO(LogFileHandle);
}

void OFS_FileLogger::DrawLogWindow(bool* open) noexcept
{
    if (!*open) return;
    OFS_MainLog.Draw(TR_ID("OFS_LOG_OUTPUT", Tr::OFS_LOG_OUTPUT).c_str(), open);
}

inline static void LogToConsole(OFS_LogLevel level, const char* msg) noexcept
{
    OFS_PROFILE(__FUNCTION__);
    switch (level) {
        case OFS_LogLevel::OFS_LOG_INFO:
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, msg);
            OFS_MainLog.AddLog("[INFO]: %s\n", msg);
            break;
        case OFS_LogLevel::OFS_LOG_WARN:
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, msg);
            OFS_MainLog.AddLog("[WARN]: %s\n", msg);
            break;
        case OFS_LogLevel::OFS_LOG_DEBUG:
            SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, msg);
            OFS_MainLog.AddLog("[DEBUG]: %s\n", msg);
            break;
        case OFS_LogLevel::OFS_LOG_ERROR:
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, msg);
            OFS_MainLog.AddLog("[ERROR]: %s\n", msg);
            break;
    }
}

[[deprecated]]
inline static void AppendToBuf(std::vector<char>& buffer, const char* msg, uint32_t size) noexcept
{
    OFS_PROFILE(__FUNCTION__);
    auto initialSize = buffer.size();
    buffer.resize(initialSize + size);
    std::memcpy(buffer.data() + initialSize, msg, size);
};
inline static void AppendToBuf(std::vector<char>& buffer, std::ranges::range auto msg) noexcept
{
    OFS_PROFILE(__FUNCTION__);
    buffer.append_range(msg);
};

inline static void AddNewLine() noexcept
{
    OFS_PROFILE(__FUNCTION__);
    // insert a newline if needed
    auto& buffer = Thread.LogMsgBuffer;
    if (!buffer.empty() && buffer.back() != '\n') {
        buffer.resize(buffer.size() + 1);
        buffer.back() = '\n';
    }
}

void OFS_FileLogger::LogToFileR(const char* prefix, const char* msg, bool newLine) noexcept
{
    OFS_PROFILE(__FUNCTION__);
    SDL_Log("%s %s", prefix, msg);
    SDL_LockSpinlock(&Thread.lock);

    auto& buffer = Thread.LogMsgBuffer;
    AppendToBuf(buffer, prefix, std::strlen(prefix));
    AppendToBuf(buffer, msg, std::strlen(msg));

    if (newLine) {
        AddNewLine();
    }

    SDL_UnlockSpinlock(&Thread.lock);
}

void OFS_FileLogger::LogToFileR(OFS_LogLevel level, const char* msg, uint32_t size, bool newLine) noexcept
{
    OFS_PROFILE(__FUNCTION__);
    LogToConsole(level, msg);
    SDL_LockSpinlock(&Thread.lock);

    auto& buffer = Thread.LogMsgBuffer;
    {
        constexpr std::string_view fmt = "[%6.3f][%s]: ";
        char fileFmt[32]{};
        int msgTypeLen;
        float const time = SDL_GetTicks() / 1000.f;
        char const* levelStr = nullptr;
        switch (level) {
            case OFS_LogLevel::OFS_LOG_INFO : levelStr = "INFO "; break;
            case OFS_LogLevel::OFS_LOG_WARN : levelStr = "WARN "; break;
            case OFS_LogLevel::OFS_LOG_DEBUG: levelStr = "DEBUG"; break;
            case OFS_LogLevel::OFS_LOG_ERROR: levelStr = "ERROR"; break;
            default                         : levelStr = "-----"; break;
        }
        msgTypeLen = std::format_to_n(std::begin(fileFmt), std::size(fileFmt), fmt, time, levelStr).size;
        AppendToBuf(buffer, fileFmt, msgTypeLen);
    }

    size = size == 0 ? strlen(msg) : size;
    AppendToBuf(buffer, msg, size);

    if (newLine) {
        AddNewLine();
    }

    SDL_UnlockSpinlock(&Thread.lock);
}

void OFS_FileLogger::LogToFileR(std::string_view prefix, std::string_view msg, bool newLine) noexcept
{
    OFS_PROFILE(__FUNCTION__);

    using std::literals::operator ""sv;
    auto const full = { prefix, msg, newLine ? "\n"sv : ""sv };
    //SDL_Log("%s %s", prefix, msg); // QQQ

    SDL_LockSpinlock(&Thread.lock);
    {
        auto& buffer = Thread.LogMsgBuffer;
        AppendToBuf(buffer, full | std::views::join);

        if (newLine) {
            AddNewLine();
        }
    }
    SDL_UnlockSpinlock(&Thread.lock);
}

void OFS_FileLogger::LogToFileR(OFS_LogLevel level, std::string_view msg, bool newLine) noexcept
{
    OFS_PROFILE(__FUNCTION__);
    //LogToConsole(level, std::string(msg).c_str()); // QQQ

    float const time = SDL_GetTicks() / 1000.f;
    char fileFmt[32]{};
    std::string_view const levelStr = [level]
    {
        switch (level) {
        case OFS_LogLevel::OFS_LOG_INFO : return "INFO ";
        case OFS_LogLevel::OFS_LOG_WARN : return "WARN ";
        case OFS_LogLevel::OFS_LOG_DEBUG: return "DEBUG";
        case OFS_LogLevel::OFS_LOG_ERROR: return "ERROR";
        default                         : return "-----";
        }
    } ();

    auto const msgTypeLen = std::format_to_n(std::begin(fileFmt), std::size(fileFmt), "[{:6.3f}][{:5s}]: ", time, levelStr).size;

    LogToFileR(std::string_view(fileFmt, msgTypeLen), msg, newLine);
}

void OFS_FileLogger::Flush() noexcept
{
    if (!Thread.LogMsgBuffer.empty()) SDL_SignalCondition(Thread.WaitFlush);
}
