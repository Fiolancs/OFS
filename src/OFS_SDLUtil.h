#pragma once
#include <filesystem>
#include <string>
#include <string_view>

namespace OFS::util
{
    std::filesystem::path basePath(void) noexcept;
    std::filesystem::path preferredPath(std::string_view = {}) noexcept;
    std::filesystem::path resourcePath(std::string_view path) noexcept;
    std::filesystem::path ffmpegPath(void) noexcept;

    bool isMainThread(void) noexcept;

    bool stringEqualsInsensitive(std::string_view string1, std::string_view string2) noexcept;
    bool stringContainsInsensitive(std::string_view haystack, std::string_view needle) noexcept;
}
