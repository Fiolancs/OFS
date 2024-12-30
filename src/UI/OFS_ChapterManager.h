#pragma once

#include <string>
#include <cstdint>
#include <filesystem>

class OFS_ChapterManager
{
    private:
    uint32_t stateHandle = 0xFFFF'FFFF;

    public:
    OFS_ChapterManager() noexcept;
    OFS_ChapterManager(const OFS_ChapterManager&) = delete;
    OFS_ChapterManager(OFS_ChapterManager&&) = delete;
    ~OFS_ChapterManager() noexcept;

    static bool ExportClip(const class Chapter& chapter, std::filesystem::path const& outputDirStr) noexcept;
    void ShowWindow(bool* open) noexcept;

    class ChapterState& State() noexcept;
};