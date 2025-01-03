#include "OFS_Util.h"
#include "OFS_Profiling.h"

#include <scn/scan.h>
#include <stb_image_write.h>
#include <xoshiro/xoshiro256plus.h>

#if defined(_WIN32)
#define NOMINMAX
#define STBI_WINDOWS_UTF8
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shellapi.h>
#undef  WIN32_LEAN_AND_MEAN
#undef  NOMINMAX
#endif

#include <bit>
#include <span>
#include <cmath>
#include <chrono>
#include <string>
#include <format>
#include <locale>
#include <random>
#include <codecvt>
#include <filesystem>
#include <system_error>


#ifdef _WIN32
inline static int WindowsShellExecute(const wchar_t* op, const wchar_t* program, const wchar_t* params) noexcept
{
    // https://docs.microsoft.com/en-us/windows/win32/api/shellapi/nf-shellapi-shellexecutew
    // If the function succeeds, it returns a value greater than 32.
    // If the function fails, it returns an error value that indicates the cause of the failure.
    // The return value is cast as an HINSTANCE for backward compatibility with 16-bit Windows applications.
    // It is not a true HINSTANCE, however.
    // It can be cast only to an INT_PTR and compared to either 32 or the following error codes below.
    auto val = (INT_PTR)ShellExecuteW(NULL, op, program, params, NULL, SW_SHOWNORMAL);
    return val > 32;
}
#endif

std::filesystem::path OFS::util::sanitizePath(std::filesystem::path const& path)
{
#if _WIN32
    auto const& pathStr = path.native();
    if (pathStr.size() > MAX_PATH)
    {
        std::filesystem::path maxPath = std::filesystem::absolute(path);
        return std::filesystem::path(LR"(\\?\)" + maxPath.native());
    }
#endif
    return path;
}

std::filesystem::path OFS::util::pathFromU8String(std::string_view str) noexcept
{
    auto result = std::filesystem::u8path(str);
    result.make_preferred();
    return result;
}
std::filesystem::path OFS::util::pathFromU8String(std::u8string_view str) noexcept
{
    auto result = std::filesystem::u8path(str);
    result.make_preferred();
    return result;
}

void OFS::util::concatPathSafe(std::filesystem::path& path, std::string const& element)
{
    path /= pathFromU8String(element);
}

bool OFS::util::createDirectories(std::filesystem::path const& dirs) noexcept
{
    std::error_code ec{};
    std::filesystem::create_directories(sanitizePath(dirs), ec);
    if (ec) LOGF_ERROR("Failed to create directory: {:s}", ec.message().c_str());
    return !ec;
}

std::size_t OFS::util::writeFile(std::filesystem::path const& path, std::span<std::byte const> buffer)
{
    if (std::fstream file = openFile(sanitizePath(path), std::ios::out | std::ios::binary); file)
    {
        auto const before = file.tellp();
        
        if (file.write(reinterpret_cast<char const*>(buffer.data()), buffer.size()))
            return file.tellp() - before;
    }
    return 0;
}
std::size_t OFS::util::writeFile(std::filesystem::path const& path, std::span<char const> buffer)
{
    if (std::fstream file = openFile(sanitizePath(path), std::ios::out | std::ios::binary); file)
    {
        auto const before = file.tellp();

        if (file.write(reinterpret_cast<char const*>(buffer.data()), buffer.size()))
            return file.tellp() - before;
    }
    return 0;
}

std::chrono::milliseconds OFS::util::parseTime(std::string_view timeStr, bool* const success)
{
    OFS_PROFILE(__FUNCTION__);
    if (success) *success = false;
    std::chrono::milliseconds result{};
    int hours = 0;
    int minutes = 0;
    int seconds = 0;
    int milliseconds = 0;
    
    if (auto const scan_result = scn::scan(timeStr, "{:d}:{:d}:{:d}", std::make_tuple(hours, minutes, seconds)); scan_result.has_value())
    {
        std::tie(hours, minutes, seconds) = scan_result.value().values();

        if (auto scan_ms_result = scn::scan(scan_result.value().range(), ".{:d}", std::make_tuple(milliseconds)); scan_ms_result.has_value())
        {
            milliseconds = scan_ms_result.value().value();
        }
    }

    if (hours >= 0
        && minutes >= 0 && minutes <= 59
        && seconds >= 0 && seconds <= 59
        && milliseconds >= 0 && milliseconds <= 999)
    {
        result = (std::chrono::hours(hours) + std::chrono::minutes(minutes))
            + (std::chrono::seconds(seconds) + std::chrono::milliseconds(milliseconds));

        if (success) *success = true;
    }
    return result;
}

std::size_t OFS::util::formatTime(std::span<char> fixedBuffer, std::chrono::seconds time)
{
    FUN_ASSERT(fixedBuffer.size(), "Buffer cannot be empty.");
    return std::format_to_n(fixedBuffer.data(), fixedBuffer.size(), "{:%T}", time).size;
}
std::size_t OFS::util::formatTime(std::span<char> fixedBuffer, std::chrono::milliseconds time)
{
    FUN_ASSERT(fixedBuffer.size(), "Buffer cannot be empty.");
    return std::format_to_n(fixedBuffer.data(), fixedBuffer.size(), "{:%T}", time).size;
}
std::size_t OFS::util::formatTime(std::span<char> fixedBuffer, float time, bool withMs)
{
    OFS_PROFILE(__FUNCTION__);

    std::chrono::duration<float> dur(time);

    if (withMs)
        return formatTime(fixedBuffer, std::chrono::round<std::chrono::milliseconds>(dur));
    else
        return formatTime(fixedBuffer, std::chrono::round<std::chrono::seconds>(dur));
}

std::string OFS::util::formatBytes(std::size_t bytes) noexcept
{
    auto const log1024 = (std::bit_width(bytes) - 1) / 10;

    switch (log1024)
    {
    case 0:  return std::format("{:d} bytes", bytes);                                          // bytes
    case 1:  return std::format("{:.2f} KB", bytes / 1024.);                                   // kilobytes
    case 2:  return std::format("{:.2f} MB", bytes / (1024. * 1024.));                         // megabytes
    case 3:  return std::format("{:.2f} GB", bytes / (1024. * 1024. * 1024.));                 // gigabytes
    case 4:  return std::format("{:.2f} TB", bytes / (1024. * 1024. * 1024. * 1024.));         // terabytes
    default: return std::format("{:.2f} PB", bytes / (1024. * 1024. * 1024. * 1024. * 1024.)); // petabytes
    }
}

bool OFS::util::fileExists(std::filesystem::path const& file) noexcept
{
    std::error_code ec{};
    auto const status = std::filesystem::status(file, ec);
    return !ec && std::filesystem::exists(status) && std::filesystem::is_regular_file(status);
}

bool OFS::util::directoryExists(std::filesystem::path const& dir) noexcept
{
    std::error_code ec{};
    auto const status = std::filesystem::status(dir, ec);
    return !ec && std::filesystem::exists(status) && std::filesystem::is_directory(status);
}

std::wstring OFS::util::utf8ToUtf16(std::string_view str) noexcept
{
    std::wstring wstr{};
    try {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter{};
        wstr = converter.from_bytes(str.data(), str.data() + str.size());
    }
    catch (const std::exception& ex) {
        LOGF_ERROR("Failed to convert to UTF-16.\n{:s}", str);
    }
    return wstr;
}

std::wstring OFS::util::utf8ToUtf16(std::u8string_view str) noexcept
{
    return utf8ToUtf16(std::string(str.begin(), str.end()));
}

bool OFS::util::savePNG(std::string const& path, void const* buffer, std::int32_t width, std::int32_t height, std::int32_t channels, bool flipVertical) noexcept
{
    stbi_flip_vertically_on_write(flipVertical);
    bool success = stbi_write_png(path.c_str(),
        width, height,
        channels, buffer, 0);
    return success;
}

int OFS::util::openUrl(const std::string& url)
{
#if defined(WIN32)
    auto const params = std::format(L"\"{:s}\"", utf8ToUtf16(url));
    return WindowsShellExecute(L"open", params.c_str(), nullptr);
#elif defined(__APPLE__)
    LOG_ERROR("Not implemented for this platform.");
    return 1;
#else
    auto const params = std::format("xdg-open \"{:s}\"", url);
    return std::system(params.c_str());
#endif
}

int OFS::util::openFileExplorer(std::filesystem::path const& path)
{
#if defined(_WIN32)
    auto const params = std::format(L"\"{:s}\"", path.native());
    return WindowsShellExecute(nullptr, L"explorer", params.c_str());
#elif defined(__APPLE__)
    LOG_ERROR("Not implemented for this platform.");
    return 1;
#else
    return OpenUrl(path.string());
#endif
}

float OFS::util::randomFloat(void) noexcept
{
    return static_cast<float>(randomDouble());
}

double OFS::util::randomDouble(void) noexcept
{
    static thread_local xoshiro::xoshiro256plus prng{
        [] {
            std::uniform_int_distribution<std::uint64_t> d(std::numeric_limits<std::uint64_t>::min());
            std::random_device rd;

            return d(rd);
        }()
    };

    return (prng() >> 11) * 0x1.p-53;
}


static std::uint32_t convertHSV_ABGR8(float h, float s, float v, float alpha)
{
    std::uint8_t r, g, b, a;

    a = static_cast<std::uint8_t>(alpha * 0xFF + .5f);
    if (s == 0.f)
    {
        r = g = b = v;
    }
    else
    {
        h = std::fmodf(h, 1.f) * 6.f;
        int   const i = static_cast<int>(h);
        float const f = h - i;
        float const p = v * (1.f - s);
        float const q = v * (1.f - (s * f));
        float const t = v * (1.f - (s * (1.f - f)));

        auto constexpr roundf = [](float f) { return std::uint8_t(f + .5f); };
        switch (i)
        {
        default:
        case  5: r = roundf(v * 255); g = roundf(p * 255); b = roundf(q * 255); break;
        case  4: r = roundf(t * 255); g = roundf(p * 255); b = roundf(v * 255); break;
        case  3: r = roundf(p * 255); g = roundf(q * 255); b = roundf(v * 255); break;
        case  2: r = roundf(p * 255); g = roundf(v * 255); b = roundf(t * 255); break;
        case  1: r = roundf(q * 255); g = roundf(v * 255); b = roundf(p * 255); break;
        case  0: r = roundf(v * 255); g = roundf(t * 255); b = roundf(p * 255); break;
        }
    }
    return (std::uint32_t(a) << 24) | (std::uint32_t(b) << 16) | (std::uint32_t(g) << 8) | std::uint32_t(r);
}

std::uint32_t OFS::util::randomColor(float s, float v, float alpha) noexcept
{
    // This is cool :^)
    // https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
    constexpr float goldenRatioConjugate = 0.618033988749895f;
    static float H = OFS::util::randomFloat();

    H += goldenRatioConjugate;
    H = std::fmodf(H, 1.f);

    return convertHSV_ABGR8(H, s, v, alpha);
}
