#pragma once
#include "OFS_FileLogging.h"
#include "UI/OFS_Profiling.h"

#include <chrono>
#include <vector>
#include <format>
#include <string>
#include <memory>
#include <filesystem>
#include <functional>

#include <scn/scan.h>
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_filesystem.h>

#include "emmintrin.h" // for _mm_pause

#define OFS_PAUSE_INTRIN _mm_pause

// helper for FontAwesome. Version 4.7.0 2016 ttf
#define ICON_FOLDER_OPEN "\xef\x81\xbc"
#define ICON_VOLUME_UP "\xef\x80\xa8"
#define ICON_VOLUME_OFF "\xef\x80\xa6"
#define ICON_LONG_ARROW_UP "\xef\x85\xb6"
#define ICON_LONG_ARROW_DOWN "\xef\x85\xb5"
#define ICON_LONG_ARROW_RIGHT "\xef\x85\xb8"
#define ICON_ARROW_RIGHT "\xef\x81\xa1"
#define ICON_PLAY "\xef\x81\x8b"
#define ICON_PAUSE "\xef\x81\x8c"
#define ICON_GAMEPAD "\xef\x84\x9b"
#define ICON_HAND_RIGHT "\xef\x82\xa4"
#define ICON_BACKWARD "\xef\x81\x8a"
#define ICON_FORWARD "\xef\x81\x8e"
#define ICON_STEP_BACKWARD "\xef\x81\x88"
#define ICON_STEP_FORWARD "\xef\x81\x91"
#define ICON_GITHUB "\xef\x82\x9b"
#define ICON_SHARE "\xef\x81\x85"
#define ICON_EXCLAMATION "\xef\x84\xaa"
#define ICON_REFRESH "\xef\x80\xa1"
#define ICON_TRASH "\xef\x87\xb8"
#define ICON_RANDOM "\xef\x81\xb4"
#define ICON_WARNING_SIGN "\xef\x81\xb1"
#define ICON_LINK "\xef\x83\x81"
#define ICON_UNLINK "\xef\x84\xa7"
#define ICON_COPY "\xef\x83\x85"
#define ICON_LEAF "\xef\x81\xac"
// when adding new ones they need to get added in OFS_DynamicFontAtlas

#if defined(_WIN32)
#define FUN_DEBUG_BREAK() __debugbreak()
#else
#define FUN_DEBUG_BREAK()
#endif

#ifndef NDEBUG
#define FUN_ASSERT_F(expr, format, ...) \
    if (expr) {} \
    else { \
        LOG_ERROR("============== ASSERTION FAILED =============="); \
        LOGF_ERROR("in file: \"%s\" line: %d", __FILE__, __LINE__); \
        LOGF_ERROR(format, __VA_ARGS__); \
        FUN_DEBUG_BREAK(); \
    }


// assertion without error message
#define FUN_ASSERT(expr, msg) \
    if (expr) {} \
    else { \
        LOG_ERROR("============== ASSERTION FAILED =============="); \
        LOGF_ERROR("in file: \"%s\" line: %d", __FILE__, __LINE__); \
        LOG_ERROR(msg); \
        FUN_DEBUG_BREAK(); \
    }
#else

#define FUN_ASSERT_F(expr, format, ...)
#define FUN_ASSERT(expr, msg)

#endif

class Util {
public:
    template <typename T>
    inline static T Clamp(T v, T mn, T mx) noexcept
    {
        return (v < mn) ? mn : (v > mx) ? mx
                                        : v;
    }

    template <typename T>
    inline static T Min(T v1, T v2) noexcept
    {
        return (v1 < v2) ? v1 : v2;
    }

    template <typename T>
    inline static T Max(T v1, T v2) noexcept
    {
        return (v1 > v2) ? v1 : v2;
    }

    template <typename T>
    inline static T MapRange(T val, T a1, T a2, T b1, T b2) noexcept
    {
        return b1 + (val - a1) * (b2 - b1) / (a2 - a1);
    }

    template <typename T>
    inline static T Lerp(T startVal, T endVal, float t) noexcept
    {
        return startVal + ((endVal - startVal) * t);
    }

#ifdef _WIN32
    inline static std::u8string WindowsMaxPath(std::u8string_view path) noexcept
    {
        std::u8string buffer(u8"\\\\?\\");
        buffer.reserve(buffer.size() + path.size());
        buffer.append(path);
        return buffer;
    }
    inline static std::string WindowsMaxPath(char const* path, std::size_t pathLen) noexcept
    {
        std::string buffer("\\\\?\\");
        buffer.reserve(buffer.size() + pathLen);
        buffer.append(path);
        return buffer;
    }
#endif

    inline static SDL_IOStream* OpenFile(const char* path, const char* mode, int32_t path_len) noexcept
    {
#ifdef _WIN32
        SDL_IOStream* handle = nullptr;
        if (path_len >= _MAX_PATH) {
            auto max = WindowsMaxPath(path, path_len);
            handle = SDL_IOFromFile(max.c_str(), mode);
        }
        else {
            handle = SDL_IOFromFile(path, mode);
        }
#else
        auto handle = SDL_IOFromFile(path, mode);
#endif
        return handle;
    }

    inline static size_t ReadFile(const char* path, std::vector<uint8_t>& buffer) noexcept
    {
        auto file = OpenFile(path, "rb", std::strlen(path));
        if (file) {
            buffer.clear();
            buffer.resize(SDL_GetIOSize(file));
            SDL_ReadIO(file, buffer.data(), buffer.size());
            SDL_CloseIO(file);
            return buffer.size();
        }
        return 0;
    }

    inline static std::string ReadFileString(const char* path) noexcept
    {
        std::string str;
        auto file = OpenFile(path, "rb", std::strlen(path));
        if (file) {
            str.resize(SDL_GetIOSize(file));
            SDL_ReadIO(file, str.data(), str.size());
            SDL_CloseIO(file);
        }
        return str;
    }

    inline static size_t WriteFile(const char* path, const void* buffer, size_t size) noexcept
    {
        auto file = OpenFile(path, "wb", std::strlen(path));
        if (file) {
            auto written = SDL_WriteIO(file, buffer, size);
            SDL_CloseIO(file);
            return written;
        }
        return 0;
    }

    // QQQ
    inline static std::string ParseJson(const std::string& jsonText, bool* success) noexcept
    {
        //nlohmann::json json;
        //*success = false;
        //if (!jsonText.empty()) {
        //    try {
        //        json = nlohmann::json::parse(jsonText, nullptr, false, true);
        //        *success = !json.is_discarded();
        //    }
        //    catch (const std::exception& e) {
        //        *success = false;
        //        LOGF_ERROR("%s", e.what());
        //    }
        //}
        //return json;
        return {};
    }

    // QQQ
    inline static std::string ParseCBOR(const std::vector<uint8_t>& data, bool* success) noexcept
    {
        //try {
        //    auto json = nlohmann::json::from_cbor(data);
        //    *success = !json.is_discarded();
        //    return json;
        //}
        //catch (const std::exception& e) {
        //    *success = false;
        //    LOGF_ERROR("%s", e.what());
        //}
        return {};
    }

    // QQQ
    inline static std::string SerializeJson(/*const nlohmann::json& json, bool pretty = false*/...) noexcept
    {
        return {};
        //auto jsonText = json.dump(pretty ? 4 : -1, ' ');
        //return jsonText;
    }

    // QQQ
    inline static std::vector<uint8_t> SerializeCBOR(/*const nlohmann::json& json*/...) noexcept
    {
        return {};
        //auto data = nlohmann::json::to_cbor(json);
        //return data;
    }

    inline static float ParseTime(const char* timeStr, bool* succ) noexcept
    {
        int hours = 0;
        int minutes = 0;
        int seconds = 0;
        int milliseconds = 0;
        *succ = false;
        auto const result = scn::scan(std::string_view(timeStr), "{:d}:{:d}:{:d}", std::make_tuple(hours, minutes, seconds));
        if (!result) {
            return NAN;
        }

        std::tie(hours, minutes, seconds) = result.value().values();
        if (auto r2 = scn::scan(result.value().range(), ".{:d}", std::make_tuple(milliseconds)); r2)
        {
            milliseconds = r2.value().value();
        }

        if (hours >= 0
            && minutes >= 0 && minutes <= 59
            && seconds >= 0 && seconds <= 59
            && milliseconds >= 0 && milliseconds <= 999) {
            float time = 0.f;
            time += (float)hours * 60.f * 60.f;
            time += (float)minutes * 60.f;
            time += (float)seconds;
            time += (float)milliseconds / 1000.f;
            *succ = true;
            return time;
        }
        return NAN;
    }

    inline static int FormatTime(char* buf, const int bufLen, float timeSeconds, bool withMs) noexcept
    {
        OFS_PROFILE(__FUNCTION__);
        namespace chrono = std::chrono;
        FUN_ASSERT(bufLen >= 0, "wat");
        if (std::isinf(timeSeconds) || std::isnan(timeSeconds))
            timeSeconds = 0.f;

        auto duration = chrono::duration<float>(timeSeconds);

        int hours = chrono::duration_cast<chrono::hours>(duration).count();
        auto timeConsumed = chrono::duration<float>(60.f * 60.f) * hours;

        int minutes = chrono::duration_cast<chrono::minutes>(duration - timeConsumed).count();
        timeConsumed += chrono::duration<float>(60.f) * minutes;

        int seconds = chrono::duration_cast<chrono::seconds>(duration - timeConsumed).count();

        if (withMs) {
            timeConsumed += chrono::duration<float>(1.f) * seconds;
            int ms = chrono::duration_cast<chrono::milliseconds>(duration - timeConsumed).count();
            return std::format_to_n(buf, bufLen, "{:02d}:{:02d}:{:02d}.{:03d}", hours, minutes, seconds, ms).size;
        }
        else {
            return std::format_to_n(buf, bufLen, "{:02d}:{:02d}:{:02d}", hours, minutes, seconds).size;
        }
    }

    static int OpenFileExplorer(const std::string& path);
    static int OpenUrl(const std::string& url);

    inline static std::filesystem::path Basepath() noexcept
    {
        char const* base = SDL_GetBasePath();
        auto path = Util::PathFromString(base);
        return path;
    }

    inline static std::u8string Filename(const std::string& path) noexcept
    {
        return Util::PathFromString(path)
            .replace_extension("")
            .filename()
            .u8string();
    }

    inline static bool FileExists(const std::string& file) noexcept
    {
        bool exists = false;
#if _WIN32
        std::wstring wfile = Util::Utf8ToUtf16(file);
        struct _stati64 s;
        exists = _wstati64(wfile.c_str(), &s) == 0;
#else
        auto handle = OpenFile(file.c_str(), "r", file.size());
        if (handle != nullptr) {
            SDL_RWclose(handle);
            exists = true;
        }
        else {
            LOGF_WARN("\"%s\" doesn't exist", file.c_str());
        }
#endif
        return exists;
    }

    inline static bool DirectoryExists(const std::string& dir) noexcept
    {
        std::error_code ec1{}, ec2{};
        auto const path = Util::PathFromString(dir);
        auto const status = std::filesystem::status(path);
        return std::filesystem::exists(status) && std::filesystem::is_directory(status);
    }

    // http://www.martinbroadhurst.com/how-to-trim-a-stdstring.html
    static std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ") noexcept
    {
        str.erase(0, str.find_first_not_of(chars));
        return str;
    }

    static std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ") noexcept
    {
        str.erase(str.find_last_not_of(chars) + 1);
        return str;
    }

    static std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ") noexcept
    {
        return ltrim(rtrim(str, chars), chars);
    }

    inline static bool StringEqualsInsensitive(const std::string& string1, const std::string string2) noexcept
    {
        if (string1.length() != string2.length()) return false;
        return ContainsInsensitive(string1.c_str(), string2.c_str());
    }

    inline static bool ContainsInsensitive(const char* haystack, const char* needle) noexcept
    {
        size_t length = SDL_strlen(needle);
        while (*haystack) {
            if (SDL_strncasecmp(haystack, needle, length) == 0) {
                return true;
            }
            ++haystack;
        }
        return false;
    }

    inline static bool StringEndsWith(std::string_view string, std::string_view ending) noexcept
    {
        return string.ends_with(ending);
    }

    inline static bool StringStartsWith(std::string_view string, std::string_view start) noexcept
    {
        return string.starts_with(start);
    }
    inline static bool StringStartsWith(std::u8string_view string, std::u8string_view start) noexcept
    {
        return string.starts_with(start);
    }

    struct FileDialogResult {
        std::vector<std::string> files;
    };

    using FileDialogResultHandler = std::function<void(FileDialogResult&)>;

    static void OpenFileDialog(const std::string& title,
        const std::string& path,
        FileDialogResultHandler&& handler,
        bool multiple = false,
        const std::vector<const char*>& filters = {},
        const std::string& filterText = "") noexcept;

    static void SaveFileDialog(const std::string& title,
        const std::string& path,
        FileDialogResultHandler&& handler,
        const std::vector<const char*>& filters = {},
        const std::string& filterText = "") noexcept;

    static void OpenDirectoryDialog(const std::string& title,
        const std::string& path,
        FileDialogResultHandler&& handler) noexcept;

    enum class YesNoCancel {
        Yes,
        No,
        Cancel
    };

    using YesNoDialogResultHandler = std::function<void(YesNoCancel)>;
    static void YesNoCancelDialog(const std::string& title, const std::string& message, YesNoDialogResultHandler&& handler);

    static void MessageBoxAlert(const std::string& title, const std::string& message) noexcept;

    static std::string Resource(const std::string& path) noexcept;

    static std::u8string Prefpath(const std::string& path = std::string()) noexcept
    {
        static const char* cachedPref = SDL_GetPrefPath("OFS", "OFS3_data");
        static std::filesystem::path prefPath = Util::PathFromString(cachedPref);
        if (!path.empty()) {
            std::filesystem::path rel = Util::PathFromString(path);
            rel.make_preferred();
            return (prefPath / rel).u8string();
        }
        return prefPath.u8string();
    }

    static std::u8string PrefpathOFP(const std::string& path) noexcept
    {
        static const char* cachedPref = SDL_GetPrefPath("OFS", "OFP_data");
        static std::filesystem::path prefPath = Util::PathFromString(cachedPref);
        std::filesystem::path rel = Util::PathFromString(path);
        rel.make_preferred();
        return (prefPath / rel).u8string();
    }

    static bool CreateDirectories(const std::filesystem::path& dirs) noexcept
    {
        std::error_code ec;
#ifdef _WIN32
        if (auto pString = dirs.u8string(); pString.size() >= _MAX_PATH) {
            
            auto max = WindowsMaxPath(pString);
            std::filesystem::create_directories(max, ec);
            if (ec) {
                LOGF_ERROR("Failed to create directory: %s", ec.message().c_str());
                return false;
            }
            return true;
        }
#endif
        std::filesystem::create_directories(dirs, ec);
        if (ec) {
            LOGF_ERROR("Failed to create directory: %s", ec.message().c_str());
            return false;
        }
        return true;
    }

    static std::wstring Utf8ToUtf16(const std::string& str) noexcept;

    static std::filesystem::path PathFromString(const std::string& str) noexcept;
    static std::filesystem::path PathFromString(const std::u8string& str) noexcept;
    static void ConcatPathSafe(std::filesystem::path& path, const std::string& element) noexcept;

    static bool SavePNG(const std::string& path, void* buffer, int32_t width, int32_t height, int32_t channels = 3, bool flipVertical = true) noexcept;

    static std::filesystem::path FfmpegPath() noexcept;

    inline static std::string FormatBytes(size_t bytes) noexcept
    {
        if (bytes < 1024) {
            return std::format("{:d} bytes", bytes); // bytes
        }
        else if (bytes >= 1024 && bytes < size_t(1024 * 1024)) {
            return std::format("{:.2f} KB", bytes / 1024.0); // kilobytes
        }
        else if (bytes >= (1024 * 1024) && bytes < size_t(1024 * 1024 * 1024)) {
            return std::format("{:.2f} MB", bytes / (1024.0 * 1024.0)); // megabytes
        }
        else /*if (bytes > (1024 * 1024 * 1024))*/ {
            return std::format("{:.2f} GB", bytes / (1024.0 * 1024.0 * 1024.0)); // gigabytes
        }
    }

    inline static bool InMainThread() noexcept
    {
        static auto Main = SDL_ThreadID();
        return SDL_ThreadID() == Main;
    }

    // https://stackoverflow.com/questions/56940199/how-to-capture-a-unique-ptr-in-a-stdfunction
    template<class F>
    auto static MakeSharedFunction(F&& f)
    {
        return
            [pf = std::make_shared<std::decay_t<F>>(std::forward<F>(f))](auto&&... args) -> decltype(auto) {
                return (*pf)(decltype(args)(args)...);
            };
    }

    static void InitRandom() noexcept;
    static float NextFloat() noexcept;
    static uint32_t RandomColor(float s, float v, float alpha = 1.f) noexcept;
};
