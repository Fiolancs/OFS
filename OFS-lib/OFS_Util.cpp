#include <version>
#include "OFS_Util.h"
#include "OFS_Profiling.h"

#include "event/OFS_EventSystem.h"

#include <scn/scan.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <tinyfiledialogs.h>
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
#include <iostream>
#include <algorithm>
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

std::filesystem::path OFS::util::pathFromString(std::string const& str) noexcept
{
    auto result = std::filesystem::u8path(str);
    result.make_preferred();
    return result;
}
std::filesystem::path OFS::util::pathFromString(std::u8string const& str) noexcept
{
    auto result = std::filesystem::u8path(str);
    result.make_preferred();
    return result;
}

void OFS::util::concatPathSafe(std::filesystem::path& path, std::string const& element)
{
    path /= pathFromString(element);
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

std::wstring OFS::util::utf8ToUtf16(std::string const& str) noexcept
{
    std::wstring wstr{};
    try {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter{};
        wstr = converter.from_bytes(str);
    }
    catch (const std::exception& ex) {
        LOGF_ERROR("Failed to convert to UTF-16.\n{:s}", str.c_str());
    }
    return wstr;
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


// tinyfiledialogs doesn't like quotes
static void SanitizeString(std::string& str) noexcept
{
    std::transform(str.begin(), str.end(), str.begin(), [](char c) { return c == '\'' || c == '"' ? ' ' : c; });
}


void Util::OpenFileDialog(const std::string& title, const std::string& path, FileDialogResultHandler&& handler, bool multiple, const std::vector<const char*>& filters, const std::string& filterText) noexcept
{
    struct FileDialogThreadData {
        bool multiple = false;
        std::string title;
        std::string path;
        std::vector<const char*> filters;
        std::string filterText;
        FileDialogResultHandler handler;
    };
    auto thread = [](void* ctx) {
        auto data = (FileDialogThreadData*)ctx;

        if (!OFS::util::directoryExists(data->path)) {
            data->path = "";
        }

#ifdef WIN32
        std::wstring wtitle = OFS::util::utf8ToUtf16(data->title);
        std::wstring wpath = OFS::util::utf8ToUtf16(data->path);
        std::wstring wfilterText = OFS::util::utf8ToUtf16(data->filterText);
        std::vector<std::wstring> wfilters;
        std::vector<const wchar_t*> wc_str;
        wfilters.reserve(data->filters.size());
        wc_str.reserve(data->filters.size());
        for (auto&& filter : data->filters) {
            wfilters.emplace_back(OFS::util::utf8ToUtf16(filter));
            wc_str.push_back(wfilters.back().c_str());
        }
        auto result = tinyfd_utf16to8(tinyfd_openFileDialogW(wtitle.c_str(), wpath.c_str(), wc_str.size(), wc_str.data(), wfilterText.empty() ? NULL : wfilterText.c_str(), data->multiple));
#elif __APPLE__
        auto result = tinyfd_openFileDialog(data->title.c_str(), data->path.c_str(), 0, nullptr, data->filterText.empty() ? NULL : data->filterText.c_str(), data->multiple);
#else
        auto result = tinyfd_openFileDialog(data->title.c_str(), data->path.c_str(), data->filters.size(), data->filters.data(), data->filterText.empty() ? NULL : data->filterText.c_str(), data->multiple);
#endif
        auto dialogResult = new FileDialogResult;
        if (result != nullptr) {
            if (data->multiple) {
                int last = 0;
                int index = 0;
                for (char c : std::string(result)) {
                    if (c == '|') {
                        dialogResult->files.emplace_back(std::string(result + last, index - last));
                        last = index + 1;
                    }
                    index += 1;
                }
                dialogResult->files.emplace_back(std::string(result + last, index - last));
            }
            else {
                dialogResult->files.emplace_back(result);
            }
        }

        EV::Enqueue<OFS_DeferEvent>(
            [resultHandler = std::move(data->handler), dialogResult]() {
                resultHandler(*dialogResult);
                delete dialogResult;
            });
        delete data;
        return 0;
    };
    auto threadData = new FileDialogThreadData;
    threadData->handler = std::move(handler);
    threadData->filters = filters;
    threadData->filterText = filterText;
    threadData->multiple = multiple;
    threadData->path = path;
    threadData->title = title;
    auto handle = SDL_CreateThread(thread, "OpenFileDialog", threadData);
    SDL_DetachThread(handle);
}

void Util::SaveFileDialog(const std::string& title, const std::string& path, FileDialogResultHandler&& handler, const std::vector<const char*>& filters, const std::string& filterText) noexcept
{
    struct SaveFileDialogThreadData {
        std::string title;
        std::string path;
        std::vector<const char*> filters;
        std::string filterText;
        FileDialogResultHandler handler;
    };
    auto thread = [](void* ctx) -> int32_t {
        auto data = (SaveFileDialogThreadData*)ctx;

        auto dialogPath = OFS::util::pathFromString(data->path);
        dialogPath.remove_filename();
        std::error_code ec;
        if (!std::filesystem::exists(dialogPath, ec)) {
            data->path = "";
        }

        SanitizeString(data->path);

        auto result = tinyfd_saveFileDialog(data->title.c_str(), data->path.c_str(), data->filters.size(), data->filters.data(), !data->filterText.empty() ? data->filterText.c_str() : NULL);

        FUN_ASSERT(result, "Ignore this if you pressed cancel.");
        auto saveDialogResult = new FileDialogResult;
        if (result != nullptr) {
            saveDialogResult->files.emplace_back(result);
        }
        EV::Enqueue<OFS_DeferEvent>([resultHandler = std::move(data->handler), saveDialogResult]() {
            resultHandler(*saveDialogResult);
            delete saveDialogResult;
        });
        delete data;
        return 0;
    };
    auto threadData = new SaveFileDialogThreadData;
    threadData->title = title;
    threadData->path = path;
    threadData->filters = filters;
    threadData->filterText = filterText;
    threadData->handler = std::move(handler);
    auto handle = SDL_CreateThread(thread, "SaveFileDialog", threadData);
    SDL_DetachThread(handle);
}

void Util::OpenDirectoryDialog(const std::string& title, const std::string& path, FileDialogResultHandler&& handler) noexcept
{
    struct OpenDirectoryDialogThreadData {
        std::string title;
        std::string path;
        FileDialogResultHandler handler;
    };
    auto thread = [](void* ctx) -> int32_t {
        auto data = (OpenDirectoryDialogThreadData*)ctx;

        if (!OFS::util::directoryExists(data->path)) {
            data->path = "";
        }

        auto result = tinyfd_selectFolderDialog(data->title.c_str(), data->path.c_str());

        FUN_ASSERT(result, "Ignore this if you pressed cancel.");
        auto directoryDialogResult = new FileDialogResult;
        if (result != nullptr) {
            directoryDialogResult->files.emplace_back(result);
        }

        EV::Enqueue<OFS_DeferEvent>([resultHandler = std::move(data->handler), directoryDialogResult]() {
            resultHandler(*directoryDialogResult);
            delete directoryDialogResult;
        });
        delete data;
        return 0;
    };
    auto threadData = new OpenDirectoryDialogThreadData;
    threadData->title = title;
    threadData->path = path;
    threadData->handler = std::move(handler);
    auto handle = SDL_CreateThread(thread, "SaveFileDialog", threadData);
    SDL_DetachThread(handle);
}

void Util::YesNoCancelDialog(const std::string& title, const std::string& message, YesNoDialogResultHandler&& handler)
{
    struct YesNoCancelThreadData {
        std::string title;
        std::string message;
        YesNoDialogResultHandler handler;
    };
    auto thread = [](void* user) -> int {
        YesNoCancelThreadData* data = (YesNoCancelThreadData*)user;
        auto result = tinyfd_messageBox(data->title.c_str(), data->message.c_str(), "yesnocancel", NULL, 1);
        Util::YesNoCancel enumResult;
        switch (result) {
            case 0:
                enumResult = Util::YesNoCancel::Cancel;
                break;
            case 1:
                enumResult = Util::YesNoCancel::Yes;
                break;
            case 2:
                enumResult = Util::YesNoCancel::No;
                break;
        }
        EV::Enqueue<OFS_DeferEvent>([resultHandler = std::move(data->handler), enumResult]() {
            resultHandler(enumResult);
        });
        delete data;
        return 0;
    };

    auto threadData = new YesNoCancelThreadData;
    threadData->title = title;
    threadData->message = message;
    threadData->handler = std::move(handler);
    auto handle = SDL_CreateThread(thread, "YesNoCancelDialog", threadData);
    SDL_DetachThread(handle);
}

void Util::MessageBoxAlert(const std::string& title, const std::string& message) noexcept
{
    struct MessageBoxData {
        std::string title;
        std::string message;
    };

    auto thread = [](void* data) -> int {
        MessageBoxData* msg = (MessageBoxData*)data;

        SanitizeString(msg->title);
        SanitizeString(msg->message);
        tinyfd_messageBox(msg->title.c_str(), msg->message.c_str(), "ok", "info", 1);

        delete msg;
        return 0;
    };

    auto threadData = new MessageBoxData;
    threadData->title = title;
    threadData->message = message;
    auto handle = SDL_CreateThread(thread, "MessageBoxAlert", threadData);
    SDL_DetachThread(handle);
}

std::string Util::Resource(const std::string& path) noexcept
{
    auto base = Util::Basepath() / L"data" / OFS::util::utf8ToUtf16(path);
    base.make_preferred();
    return base.string();
}

std::filesystem::path Util::FfmpegPath() noexcept
{
#if _WIN32
    return OFS::util::pathFromString(Util::Prefpath("ffmpeg.exe"));
#else
    auto ffmpegPath = std::filesystem::path("ffmpeg");
    return ffmpegPath;
#endif
}


// QQQ
std::uint32_t Util::RandomColor(float s, float v, float alpha) noexcept
{
    // This is cool :^)
    // https://martin.ankerl.com/2009/12/09/how-to-create-random-colors-programmatically/
    constexpr float goldenRatioConjugate = 0.618033988749895f;
    static float H = OFS::util::randomFloat();

    H += goldenRatioConjugate;
    H = std::fmodf(H, 1.f);

    //ImColor color;
    //color.SetHSV(H, s, v, alpha);
    //return ImGui::ColorConvertFloat4ToU32(color);
    return {};
}
