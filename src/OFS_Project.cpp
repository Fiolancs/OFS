#include "OFS_Project.h"

#include <algorithm>
#include <filesystem>


#include "ui/OFS_ImGui.h"
#include "ui/OFS_DynamicFontAtlas.h"
#include "ui/OFS_BlockingTask.h"
#include "io/OFS_FileDialogs.h"
#include "event/OFS_EventSystem.h"
#include "localization/OFS_Localization.h"

//#include "subprocess.h"


static std::array<const char*, 6> VideoExtensions{
    ".mp4",
    ".mkv",
    ".webm",
    ".wmv",
    ".avi",
    ".m4v",
};

static std::array<const char*, 4> AudioExtensions{
    ".mp3",
    ".ogg",
    ".flac",
    ".wav",
};

inline bool static HasMediaExtension(std::filesystem::path const& path) noexcept
{
    auto ext = path.extension().string();

    bool hasMediaExt = std::any_of(VideoExtensions.begin(), VideoExtensions.end(),
        [&ext](auto validExt) noexcept {
            return ext == validExt;
        });

    if (!hasMediaExt) {
        hasMediaExt = std::any_of(AudioExtensions.begin(), AudioExtensions.end(),
            [&ext](auto validExt) noexcept {
                return ext == validExt;
            });
    }

    return hasMediaExt;
}

inline bool FindMedia(std::filesystem::path const& path, std::string* outMedia) noexcept
{
    auto pathDir = path.parent_path();
    auto filename = path.stem().string();

    std::error_code ec;
    std::filesystem::directory_iterator dirIt(pathDir, ec);
    for (auto& entry : dirIt) {
        auto entryName = entry.path().stem().string();
        if (entryName == filename) {
            auto entryPathStr = entry.path().string();

            if (HasMediaExtension(entryPathStr)) {
                *outMedia = entryPathStr;
                return true;
            }
        }
    }

    return false;
}

OFS_Project::OFS_Project() noexcept
{
    stateHandle = OFS::ProjectState<ProjectState>::registerState(ProjectState::StateName, ProjectState::StateName);
    Funscripts.emplace_back(std::move(std::make_shared<Funscript>()));
}

OFS_Project::~OFS_Project() noexcept
{
}

void OFS_Project::loadNecessaryGlyphs() noexcept
{
    // This should be called after loading or importing.
    auto& projectState = State();
    auto& metadata = projectState.metadata;
    OFS_DynFontAtlas::Addu8Text(metadata.type);
    OFS_DynFontAtlas::Addu8Text(metadata.title);
    OFS_DynFontAtlas::Addu8Text(metadata.creator);
    OFS_DynFontAtlas::Addu8Text(metadata.script_url);
    OFS_DynFontAtlas::Addu8Text(metadata.video_url);
    for (auto& tag : metadata.tags) OFS_DynFontAtlas::Addu8Text(tag);
    for (auto& performer : metadata.performers) OFS_DynFontAtlas::Addu8Text(performer);
    OFS_DynFontAtlas::Addu8Text(metadata.description);
    OFS_DynFontAtlas::Addu8Text(metadata.license);
    OFS_DynFontAtlas::Addu8Text(metadata.notes);
    for (auto& script : Funscripts) OFS_DynFontAtlas::Addu8Text(script->Title().c_str());
    OFS_DynFontAtlas::AddText(lastPath.u8string());
}

bool OFS_Project::Load(std::filesystem::path const& path) noexcept
{
    FUN_ASSERT(!valid, "Can't import if project is already loaded.");
#if 1
    std::vector<char> projectBin;
    if (OFS::util::readFile(path, projectBin) > 0)
    {
        if (auto projectState = OFS::util::convertCBORtoJSON(projectBin); !projectState.empty()) 
        {
            valid = OFS::StateManager::get()->deserializeStateGroup<OFS::ProjectState<void>::STATE_GROUP>(projectState);
        }
    }
#else
    std::string projectJson = OFS::util::readFileString(path.c_str());
    if (!projectJson.empty()) {
        bool succ;
        auto json = Util::ParseJson(projectJson, &succ);
        if (succ) {
            valid = OFS_StateManager::Get()->DeserializeProjectAll(json, false);
        }
        else {
            valid = false;
            addError("Failed to parse project.\nIt likely is an old project file not supported in " OFS_LATEST_GIT_TAG);
        }
    }
#endif

    if (valid) {
        auto& projectState = State();
        OFS_Binary::Deserialize(projectState.binaryFunscriptData, *this);
        lastPath = path;
        loadNecessaryGlyphs();
    }

    return valid;
}

bool OFS_Project::ImportFromFunscript(std::filesystem::path const& file) noexcept
{
    FUN_ASSERT(!valid, "Can't import if project is already loaded.");

    auto& projectState = State();
    auto basePath = file;
    lastPath = basePath.replace_extension(OFS_Project::Extension);

    if (OFS::util::fileExists(file)) {
        Funscripts.clear();
        if (!AddFunscript(file)) {
            addError("Failed to load funscript.");
            return valid;
        }
        loadMultiAxis(file);

        std::string absMediaPath;
        if (FindMedia(file, &absMediaPath)) {
            projectState.relativeMediaPath = MakePathRelative(absMediaPath);
            valid = true;
            loadNecessaryGlyphs();
        }
        else {
            addError("Failed to find media for funscript.");
            return valid;
        }
    }

    return valid;
}

bool OFS_Project::ImportFromMedia(std::filesystem::path const& file) noexcept
{
    FUN_ASSERT(!valid, "Can't import if project is already loaded.");

    if (!HasMediaExtension(file)) {
        // Unsupported media.
        addError("Unsupported media file extension.");
        return false;
    }

    auto& projectState = State();
    auto basePath = file;
    lastPath = basePath.replace_extension(OFS_Project::Extension);

    basePath = file;
    if (OFS::util::fileExists(file)) {
        projectState.relativeMediaPath = MakePathRelative(file);
        auto funscriptPath = basePath;
        funscriptPath.replace_extension(".funscript");

        Funscripts.clear();
        AddFunscript(funscriptPath);
        loadMultiAxis(funscriptPath);
        valid = true;
        loadNecessaryGlyphs();
    }

    return valid;
}

bool OFS_Project::AddFunscript(std::filesystem::path const& path) noexcept
{
    bool loadedScript = false;

    bool succ = false;
    auto jsonText = OFS::util::readFileString(path);

    // QQQ
    auto json = std::string{};// Util::ParseJson(jsonText, &succ);

    auto script = std::make_shared<Funscript>();
    auto metadata = Funscript::Metadata();

    bool isFirstFunscript = Funscripts.size() == 0;
    if (succ && script->Deserialize(/*json, */&metadata, isFirstFunscript)) {
        // Add existing script to project
        script = Funscripts.emplace_back(std::move(script));
        script->UpdateRelativePath(MakePathRelative(path));
        if (isFirstFunscript) {
            // Initialize project metadata using the first funscript
            auto& projectState = State();
            projectState.metadata = metadata;
        }
        loadedScript = true;
    }
    else {
        // Add empty script to project
        script = std::make_shared<Funscript>();
        script->UpdateRelativePath(MakePathRelative(path));
        script = Funscripts.emplace_back(std::move(script));
    }
    return loadedScript;
}

void OFS_Project::RemoveFunscript(int32_t idx) noexcept
{
    if (idx >= 0 && idx < Funscripts.size()) {
        EV::Enqueue<FunscriptRemovedEvent>(Funscripts[idx]->Title());
        Funscripts.erase(Funscripts.begin() + idx);
    }
}

void OFS_Project::Save(std::filesystem::path const& path, bool clearUnsavedChanges) noexcept
{
    {
        auto& projectState = State();
        projectState.binaryFunscriptData.clear();
        auto size = OFS_Binary::Serialize(projectState.binaryFunscriptData, *this);
        projectState.binaryFunscriptData.resize(size);
    }

#if 1
    // QQQ
    //auto projectState = OFS_StateManager::Get()->SerializeProjectAll(true);
    //auto projectBin = Util::SerializeCBOR(projectState);
    //OFS::util::writeFile(path.c_str(), projectBin.data(), projectBin.size());
#else
    auto projectState = OFS_StateManager::Get()->SerializeProjectAll(false);
    auto projectJson = Util::SerializeJson(projectState, false);
    OFS::util::writeFile(path.c_str(), projectJson.data(), projectJson.size());
#endif
    if (clearUnsavedChanges) {
        for (auto& script : Funscripts) {
            script->ClearUnsavedEdits();
        }
    }
}

void OFS_Project::Update(float delta, bool idleMode) noexcept
{
    if (!idleMode) {
        auto& projectState = State();
        projectState.activeTimer += delta;
    }
    for (auto& script : Funscripts) script->Update();
}

bool OFS_Project::HasUnsavedEdits() noexcept
{
    OFS_PROFILE(__FUNCTION__);
    for (auto& script : Funscripts) {
        if (script->HasUnsavedEdits()) {
            return true;
        }
    }
    return false;
}

void OFS_Project::ShowProjectWindow(bool* open) noexcept
{
    if (*open)
    {
        ImGui::OpenPopup(TR_ID("PROJECT", Tr::PROJECT).c_str());
    }

    if (ImGui::BeginPopupModal(TR_ID("PROJECT", Tr::PROJECT).c_str(), open, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize))
    {
        OFS_PROFILE(__FUNCTION__);
        auto& projectState = State();
        auto& Metadata = projectState.metadata;
        ImGui::PushID(Metadata.title.c_str());

        ImGui::Text("%s: %s", TR(MEDIA), projectState.relativeMediaPath.c_str());

        char buffer[128]{};
        OFS::util::formatTime(buffer, projectState.activeTimer, true);
        ImGui::Text("%s: %s", TR(TIME_SPENT), buffer);
        ImGui::Separator();

        ImGui::Spacing();
        ImGui::TextDisabled(TR(SCRIPTS));
        for (auto& script : Funscripts) {
            if (ImGui::Button(script->Title().c_str(), ImVec2(-1.f, 0.f))) {
                OFS::util::saveFileDialog(TR(CHANGE_DEFAULT_LOCATION),
                    MakePathAbsolute(script->RelativePath().string()),
                    [&](auto result) {
                        if (!result.files.empty()) {
                            auto newPath = result.files[0];
                            if (newPath.extension().string() == ".funscript") {
                                script->UpdateRelativePath(MakePathRelative(newPath.string()));
                            }
                        }
                    });
            }
            OFS::Tooltip(TR(CHANGE_LOCATION));
        }
        ImGui::PopID();
        ImGui::EndPopup();
    }
}

void OFS_Project::ExportFunscripts() noexcept
{
    auto& state = State();
    for (auto& script : Funscripts) {
        FUN_ASSERT(!script->RelativePath().empty(), "path is empty");
        if (!script->RelativePath().empty()) {
            auto json = script->Serialize(state.metadata, true);
            script->ClearUnsavedEdits();
            // QQQ
            auto jsonText = std::string{};// Util::SerializeJson(json, false);
            OFS::util::writeFile(MakePathAbsolute(script->RelativePath()), jsonText);
        }
    }
}

void OFS_Project::ExportFunscripts(std::filesystem::path const& outputDir) noexcept
{
    auto& state = State();
    for (auto& script : Funscripts) {
        FUN_ASSERT(!script->RelativePath().empty(), "path is empty");
        if (!script->RelativePath().empty()) {
            auto filename = script->RelativePath().filename();
            auto outputPath = outputDir / filename;
            auto json = script->Serialize(state.metadata, true);
            script->ClearUnsavedEdits();
            // QQQ
            auto jsonText = std::string{};// Util::SerializeJson(json, false);
            OFS::util::writeFile(outputPath, jsonText);
        }
    }
}

void OFS_Project::ExportFunscript(std::filesystem::path const& outputPath, int32_t idx) noexcept
{
    FUN_ASSERT(idx >= 0 && idx < Funscripts.size(), "out of bounds");
    auto& state = State();
    auto json = Funscripts[idx]->Serialize(state.metadata, true);
    Funscripts[idx]->ClearUnsavedEdits();
    // Using this function changes the default path
    Funscripts[idx]->UpdateRelativePath(MakePathRelative(outputPath));
    // QQQ
    auto jsonText = std::string{};// Util::SerializeJson(json, false);
    OFS::util::writeFile(outputPath, jsonText);
}

void OFS_Project::loadMultiAxis(std::filesystem::path const& rootScript) noexcept
{
    std::vector<std::filesystem::path> relatedFiles;
    {
        auto filename = rootScript.string() + '.';
        auto searchDirectory = rootScript;
        searchDirectory.remove_filename();

        std::error_code ec;
        std::filesystem::directory_iterator dirIt(searchDirectory, std::filesystem::directory_options::skip_permission_denied, ec);
        for (auto&& entry : dirIt) {
            auto extension = entry.path()
                                 .extension()
                                 .string();
            auto currentFilename = entry.path().stem().string();

            if (extension == Funscript::Extension
                && currentFilename.starts_with(filename)
                && currentFilename != filename) {
                relatedFiles.emplace_back(entry.path());
            }
        }
    }
    // reorder for 3d simulator
    std::array<std::string, 3> desiredOrder{
        // it's in reverse order
        ".twist.funscript",
        ".pitch.funscript",
        ".roll.funscript"
    };
    if (relatedFiles.size() > 1) {
        for (auto& ending : desiredOrder) {
            for (int i = 0; i < relatedFiles.size(); i++) {
                auto& path = relatedFiles[i];
                if (path.string().ends_with(ending)){
                    auto move = std::move(path);
                    relatedFiles.erase(relatedFiles.begin() + i);
                    relatedFiles.emplace_back(std::move(move));
                    break;
                }
            }
        }
    }
    // load the related files
    for (int i = relatedFiles.size() - 1; i >= 0; i -= 1) {
        auto& file = relatedFiles[i];
        auto filePathString = file.string();
        AddFunscript(filePathString);
    }
}

std::filesystem::path OFS_Project::MakePathAbsolute(std::filesystem::path const& relPath) const noexcept
{
    FUN_ASSERT(relPath.is_relative(), "Path isn't relative");
    if (relPath.is_absolute())
    {
        LOGF_ERROR("Path was already absolute. \"{:s}\"", relPath.string());
        return relPath;
    }
    else
    {
        auto projectDir = lastPath;
        projectDir.remove_filename();
        std::error_code ec;
        auto absPath = std::filesystem::absolute(projectDir / relPath, ec);
        if (!ec) {
            LOGF_INFO("Convert relative path \"{:s}\" to absolute \"{:s}\"", relPath.string(), absPath.string());
            return absPath;
        }
        FUN_ASSERT(false, "This must not happen.");
        LOG_ERROR("Failed to convert path to absolute path.");
        return "";
    }
}

std::filesystem::path OFS_Project::MakePathRelative(std::filesystem::path const& absPath) const noexcept
{
    auto projectDir = lastPath.parent_path();
    auto relPath = absPath.lexically_relative(projectDir);
    LOGF_INFO("Convert absolute path \"{:s}\" to relative \"{:s}\"", absPath.string(), relPath.string());
    return relPath;
}

std::filesystem::path OFS_Project::MediaPath() const noexcept
{
    auto& projectState = State();
    return MakePathAbsolute(projectState.relativeMediaPath);
}
