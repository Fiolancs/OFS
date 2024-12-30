#include "OFS_SDLUtil.h"

#include "OFS_Util.h"

#include <SDL3/SDL_stdinc.h>      // SDL_strncasecmp
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_filesystem.h>


std::filesystem::path OFS::util::basePath(void) noexcept
{
    char const* base = SDL_GetBasePath();
    return OFS::util::pathFromU8String(base);
}

std::filesystem::path OFS::util::resourcePath(std::string_view path) noexcept
{
    auto base = basePath() / L"data" / OFS::util::utf8ToUtf16(path);
    base.make_preferred();
    return base.string();
}

std::filesystem::path OFS::util::preferredPath(std::string_view path) noexcept
{
    static std::filesystem::path prefPath = OFS::util::pathFromU8String(SDL_GetPrefPath("OFS", "OFS3_data"));
    
    if (!path.empty())
    {
        std::filesystem::path rel = OFS::util::pathFromU8String(path);
        rel.make_preferred();
        return prefPath / rel;
    }
    return prefPath;
}

std::filesystem::path OFS::util::ffmpegPath() noexcept
{
#if _WIN32
    return preferredPath("ffmpeg.exe");
#else
    auto ffmpegPath = std::filesystem::path("ffmpeg");
    return ffmpegPath;
#endif
}

bool OFS::util::isMainThread(void) noexcept
{
    static auto Main = SDL_GetCurrentThreadID();
    return SDL_GetCurrentThreadID() == Main;
}

bool OFS::util::stringEqualsInsensitive(std::string_view string1, std::string_view string2) noexcept
{
    if (string1.length() != string2.length())
        return false;

    return stringContainsInsensitive(string1, string2);
}

bool OFS::util::stringContainsInsensitive(std::string_view haystack, std::string_view needle) noexcept
{
    for (std::size_t maxSz = haystack.size() - needle.size(), n = 0; n < maxSz; ++n)
    {
        if (SDL_strncasecmp(haystack.data() + n, needle.data(), needle.size()) == 0)
            return true;
    }

    return false;
}
