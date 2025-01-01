#pragma once
#include "OFS_DebugBreak.h"
#include "io/OFS_FileLogging.h"

#include <span>
#include <chrono>
#include <vector>
#include <string>
#include <cstdint>
#include <fstream>
#include <iterator>
#include <filesystem>

#if __has_include("emmintrin.h")
#   include "emmintrin.h" // for _mm_pause
#   define OFS_PAUSE_INTRIN _mm_pause()
#elif 
#define OFS_PAUSE_INTRIN __asm__("REP NOP")
#endif

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

#ifndef NDEBUG
#define FUN_ASSERT_F(expr, format, ...)                                 \
    if (!(expr)) [[unlikely]] {                                         \
        LOG_ERROR("============== ASSERTION FAILED ==============");    \
        LOGF_ERROR("in file: \"{:s}\" line: {:d}", __FILE__, __LINE__); \
        LOGF_ERROR(format, __VA_ARGS__);                                \
        OFS_DEBUGBREAK;                                                 \
    }
// assertion without error message
#define FUN_ASSERT(expr, msg)                                           \
    if (!(expr)) [[unlikely]] {                                         \
        LOG_ERROR("============== ASSERTION FAILED ==============");    \
        LOGF_ERROR("in file: \"{:s}\" line: {:d}", __FILE__, __LINE__); \
        LOG_ERROR(msg);                                                 \
        OFS_DEBUGBREAK;                                                 \
    }
#else
#define FUN_ASSERT_F(expr, format, ...)
#define FUN_ASSERT(expr, msg)
#endif

namespace OFS::util
{
    // ====================================================================================
    //  Path functions
    // ====================================================================================

    std::filesystem::path sanitizePath(std::filesystem::path const& path);

    std::filesystem::path pathFromU8String(std::string_view str) noexcept;
    std::filesystem::path pathFromU8String(std::u8string_view str) noexcept;
    void concatPathSafe(std::filesystem::path& path, std::string const& element);

    bool fileExists(std::filesystem::path const& file) noexcept;
    bool directoryExists(std::filesystem::path const& dir) noexcept;


    // ====================================================================================
    //  FILE IO functions
    // ====================================================================================

    bool createDirectories(std::filesystem::path const& dirs) noexcept;

    std::fstream openFile(std::filesystem::path const& path, std::ios::openmode mode);

    template <typename Container>
    std::size_t readFile(std::filesystem::path const& path, Container& buffer);
    std::string readFileString(std::filesystem::path const& path);
    std::size_t writeFile(std::filesystem::path const& path, std::span<std::byte const> buffer);
    std::size_t writeFile(std::filesystem::path const& path, std::span<char const> buffer);

    std::string filename(std::string_view path) noexcept;
    std::string filename(std::filesystem::path const& path) noexcept;


    // ====================================================================================
    //  Formatting functions
    // ====================================================================================

    std::chrono::milliseconds parseTime(std::string_view timeStr, bool* const success);

    std::size_t formatTime(std::span<char> fixedBuffer, std::chrono::seconds time);
    std::size_t formatTime(std::span<char> fixedBuffer, std::chrono::milliseconds time);
    std::size_t formatTime(std::span<char> fixedBuffer, float time, bool withMs);

    std::string formatBytes(std::size_t bytes) noexcept;

    constexpr std::string& ltrim(std::string& str, std::string_view chars = " \t\n\r\v\f") noexcept;
    constexpr std::string& rtrim(std::string& str, std::string_view chars = " \t\n\r\v\f") noexcept;
    constexpr std::string&  trim(std::string& str, std::string_view chars = " \t\n\r\v\f") noexcept;

    std::wstring utf8ToUtf16(std::string_view str) noexcept;


    // ====================================================================================
    //  Image functions
    // ====================================================================================

    bool savePNG(std::string const& path, void const* buffer, std::int32_t width, std::int32_t height, std::int32_t channels = 3, bool flipVertical = true) noexcept;


    // ====================================================================================
    //  OS
    // ====================================================================================

    int openUrl(const std::string& url);
    int openFileExplorer(const std::filesystem::path& path);

    // ====================================================================================
    //  PRNG
    // ====================================================================================

    // Generates a float between [0,1)
    float  randomFloat(void) noexcept;
    double randomDouble(void) noexcept;

    // Returns a ABGR8 u32 colour (ImGui friendly)
    std::uint32_t randomColor(float s, float v, float alpha = 1.f) noexcept;
}

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
};


inline std::fstream OFS::util::openFile(std::filesystem::path const& path, std::ios::openmode mode)
{
    return std::fstream{ sanitizePath(path), mode };
}

template <typename Container>
std::size_t OFS::util::readFile(std::filesystem::path const& path, Container& buffer)
{
    if (auto file = openFile(sanitizePath(path), std::ios::in | std::ios::binary); file)
    {
        buffer.assign(std::istreambuf_iterator(file), {});
        return buffer.size();
    }
    return 0;
}

inline std::string OFS::util::readFileString(std::filesystem::path const& path)
{
    std::string str{};
    readFile(path, str);
    return str;
}

inline std::string OFS::util::filename(std::string_view path) noexcept
{
    return filename(pathFromU8String(path));
}

inline std::string OFS::util::filename(std::filesystem::path const& path) noexcept
{
    return path.stem().string();
}

// http://www.martinbroadhurst.com/how-to-trim-a-stdstring.html
constexpr std::string& OFS::util::ltrim(std::string& str, std::string_view chars) noexcept
{
    return str.erase(0, str.find_first_not_of(chars));
}

constexpr std::string& OFS::util::rtrim(std::string& str, std::string_view chars) noexcept
{
    return str.erase(str.find_last_not_of(chars) + 1);
}

constexpr std::string& OFS::util::trim(std::string& str, std::string_view chars) noexcept
{
    return ltrim(rtrim(str, chars), chars);
}
