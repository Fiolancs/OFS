#include "OFS_FileDialogs.h"

#include "OFS_Util.h"
#include "OFS_ThreadPool.h"
#include "event/OFS_EventSystem.h"

#include <tinyfiledialogs.h>

#include <span>
#include <memory>
#include <ranges>
#include <vector>
#include <string>
#include <algorithm>
#include <filesystem>
#include <string_view>


namespace
{
    // tinyfiledialogs doesn't like quotes
    static void sanitizeString(std::string& str) noexcept
    {
        std::transform(str.begin(), str.end(), str.begin(), [](char c) { return c == '\'' || c == '"' ? ' ' : c; });
    }

    struct MessageBoxData
    {
        std::string title;
        std::string message;
    };

    struct YesNoCancelDialogData
    {
        OFS::util::YesNoDialogCallback callback;
        std::string title;
        std::string message;
    };

    struct DirectoryDialogData
    {
        OFS::util::FileDialogCallback callback;
        std::string title;
        std::filesystem::path path;
    };

    struct FileDialogThreadData
    {
        OFS::util::FileDialogCallback callback;
        std::string title;
        std::filesystem::path path;
        std::vector<const char*> filters;
        std::string filterText;
        bool multiple;
    };
}


void OFS::util::openFileDialog(std::string_view title, std::string_view path, FileDialogCallback&& handler, bool multiple, std::span<const char*> filters, std::string_view filterText)
{
    openFileDialog(title, OFS::util::pathFromU8String(path), std::move(handler), multiple, filters, filterText);
}

void OFS::util::openFileDialog(std::string_view title, std::filesystem::path const& path, FileDialogCallback&& handler, bool multiple, std::span<const char*> filters, std::string_view filterText)
{
    auto thread = [](std::unique_ptr<FileDialogThreadData> data) {
        if (!OFS::util::directoryExists(data->path))
            data->path = "";

#ifdef _WIN32
        std::wstring wtitle = OFS::util::utf8ToUtf16(data->title);
        std::wstring wfilterText = OFS::util::utf8ToUtf16(data->filterText);

        std::vector<std::wstring> wfilters;
        std::vector<const wchar_t*> wc_str;
        wfilters.reserve(data->filters.size());
        wc_str.reserve(data->filters.size());
        for (auto&& filter : data->filters)
        {
            wfilters.emplace_back(OFS::util::utf8ToUtf16(filter));
            wc_str.push_back(wfilters.back().c_str());
        }
        auto result = tinyfd_openFileDialogW(wtitle.c_str(), data->path.c_str(), wc_str.size(), wc_str.data(), wfilterText.empty() ? nullptr : wfilterText.c_str(), data->multiple);
#elif __APPLE__
        auto result = tinyfd_openFileDialog(data->title.c_str(), data->path.c_str(), 0, nullptr, data->filterText.empty() ? nullptr : data->filterText.c_str(), data->multiple);
#else
        auto result = tinyfd_openFileDialog(data->title.c_str(), data->path.c_str(), data->filters.size(), data->filters.data(), data->filterText.empty() ? nullptr : data->filterText.c_str(), data->multiple);
#endif
        auto dialogResult = std::make_shared<FileDialogResult>();
        if (result != nullptr)
        {
            if (data->multiple)
            {
                for (auto file : std::views::split(std::basic_string_view(result), '|'))
                {
                    dialogResult->files.emplace_back(std::basic_string_view(file));
                }
            }
            else
            {
                dialogResult->files.emplace_back(result);
            }
        }

        EV::Enqueue<OFS_DeferEvent>(
            [callback = std::move(data->callback), dialogResult = std::move(dialogResult)]() {
                callback(*dialogResult);
            });

#ifdef _WIN32
        // Free tinyfd memory or they'll never free it until the next call
        tinyfd_openFileDialogW(nullptr, nullptr, 0, nullptr, nullptr, -1);
#endif
        };
    OFS::ThreadPool::get().detachTask(thread, std::make_unique<FileDialogThreadData>(std::move(handler), std::string(title), path,
        std::vector(filters.begin(), filters.end()),
        std::string(filterText),
        multiple
    ));
}

void OFS::util::saveFileDialog(std::string_view title, std::string_view path, FileDialogCallback&& handler, std::span<const char*> filters, std::string_view filterText)
{
    saveFileDialog(title, OFS::util::pathFromU8String(path), std::move(handler), filters, filterText);
}

void OFS::util::saveFileDialog(std::string_view title, std::filesystem::path const& path, FileDialogCallback&& handler, std::span<const char*> filters, std::string_view filterText)
{
    OFS::ThreadPool::get().detachTask(
        [](std::unique_ptr<FileDialogThreadData> data) {
            auto dialogPath = data->path;
            dialogPath.remove_filename();
            std::error_code ec;
            if (!std::filesystem::exists(dialogPath, ec)) {
                data->path = "";
            }

            auto pathStr = data->path.string();
            sanitizeString(pathStr);

            auto result = tinyfd_saveFileDialog(data->title.c_str(), pathStr.c_str(), data->filters.size(), data->filters.data(), !data->filterText.empty() ? data->filterText.c_str() : nullptr);

            auto saveDialogResult = std::make_shared<FileDialogResult>();
            if (result != nullptr) {
                saveDialogResult->files.emplace_back(OFS::util::pathFromU8String(std::string_view(result)));
            }
            EV::Enqueue<OFS_DeferEvent>(
                [callback = std::move(data->callback), saveDialogResult = std::move(saveDialogResult)]() {
                    callback(*saveDialogResult);
                });
        }
        , std::make_unique<FileDialogThreadData>(std::move(handler), std::string(title), path,
            std::vector(filters.begin(), filters.end()),
            std::string(filterText),
            false
        ));
}

void OFS::util::openDirectoryDialog(std::string_view title, std::string_view path, FileDialogCallback&& handler)
{
    openDirectoryDialog(title, OFS::util::pathFromU8String(path), std::move(handler));
}

void OFS::util::openDirectoryDialog(std::string_view title, std::filesystem::path const& path, FileDialogCallback&& handler)
{
    OFS::ThreadPool::get().detachTask(
        [](std::unique_ptr<DirectoryDialogData> data) {
            if (!OFS::util::directoryExists(data->path)) {
                data->path = "";
            }

            auto result = tinyfd_selectFolderDialog(data->title.c_str(), data->path.string().c_str());

            auto directoryDialogResult = std::make_shared<FileDialogResult>();
            if (result != nullptr) {
                directoryDialogResult->files.emplace_back(OFS::util::pathFromU8String(std::string_view(result)));
            }

            EV::Enqueue<OFS_DeferEvent>([callback = std::move(data->callback), directoryDialogResult]() {
                callback(*directoryDialogResult);
            });
        }
        , std::make_unique<DirectoryDialogData>(std::move(handler), std::string(title), path));
}

void OFS::util::YesNoCancelDialog(std::string_view title, std::string_view message, YesNoDialogCallback&& handler)
{
    auto thread = [](std::unique_ptr<YesNoCancelDialogData> data) {
        auto result = tinyfd_messageBox(data->title.c_str(), data->message.c_str(), "yesnocancel", NULL, 1);
        YesNoCancel enumResult;
        switch (result) {
        case 0:
            enumResult = YesNoCancel::CANCEL;
            break;
        case 1:
            enumResult = YesNoCancel::YES;
            break;
        case 2:
            enumResult = YesNoCancel::NO;
            break;
        }
        EV::Enqueue<OFS_DeferEvent>([callback = std::move(data->callback), enumResult]() {
            callback(enumResult);
        });
    };

    auto threadData = std::make_unique<YesNoCancelDialogData>(std::move(handler), std::string(title), std::string(message));
    OFS::ThreadPool::get().detachTask(thread, std::move(threadData));
}

void OFS::util::MessageBoxAlert(std::string_view title, std::string_view message)
{
    OFS::ThreadPool::get().detachTask(
        [](std::unique_ptr<MessageBoxData> msg) {
            sanitizeString(msg->title);
            sanitizeString(msg->message);
            tinyfd_messageBox(msg->title.c_str(), msg->message.c_str(), "ok", "info", 1);
        }
        , std::make_unique<MessageBoxData>(std::string(title), std::string(message)));
}
