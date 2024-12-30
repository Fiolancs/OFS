#pragma once

#include <span>
#include <vector>
#include <string>
#include <string_view>
#include <filesystem>
#include <functional>


namespace OFS::util
{
    struct FileDialogResult
    {
        std::vector<std::string> files;
    };
    enum class YesNoCancel
    {
        YES,
        NO,
        CANCEL,
    };

    using FileDialogCallback  = std::function<void(FileDialogResult const&)>;
    using YesNoDialogCallback = std::function<void(YesNoCancel)>;

    void openFileDialog(std::string_view title,
        std::string_view path,
        FileDialogCallback&& handler,
        bool multiple = false,
        std::span<const char*> filters = {},
        std::string_view filterText = "");

    void saveFileDialog(std::string_view title,
        std::string_view path,
        FileDialogCallback&& handler,
        std::span<const char*> filters = {},
        std::string_view filterText = "");

    void openDirectoryDialog(std::string_view title, std::string_view path, FileDialogCallback&& handler);

    void YesNoCancelDialog(std::string_view title, std::string_view message, YesNoDialogCallback&& handler);

    void MessageBoxAlert(std::string_view title, std::string_view message);
}
