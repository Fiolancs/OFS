#pragma once
#include "state/ProjectState.h"
#include "Funscript/Funscript.h"
#include "event/OFS_Event.h"

#include <vector>
#include <memory>
#include <string>
#include <cstdint>

class ProjectLoadedEvent: public OFS_Event<ProjectLoadedEvent> {
public:
    ProjectLoadedEvent() noexcept {}
};

#define OFS_PROJECT_EXT ".ofsp"

class OFS_Project
{
public:
    static constexpr auto Extension = OFS_PROJECT_EXT;

    OFS_Project() noexcept;
    OFS_Project(const OFS_Project&) = delete;
    OFS_Project(OFS_Project&&) = delete;
    ~OFS_Project() noexcept;

    std::vector<std::shared_ptr<Funscript>> Funscripts;

    bool Load(std::filesystem::path const& path) noexcept;
    void Save(bool clearUnsavedChanges) noexcept { Save(lastPath, clearUnsavedChanges); }
    void Save(std::filesystem::path const& path, bool clearUnsavedChanges) noexcept;

    bool ImportFromFunscript(std::filesystem::path const& path) noexcept;
    bool ImportFromMedia(std::filesystem::path const& path) noexcept;

    bool AddFunscript(std::filesystem::path const& path) noexcept;
    void RemoveFunscript(int32_t idx) noexcept;

    void Update(float delta, bool idleMode) noexcept;
    void ShowProjectWindow(bool* open) noexcept;
    bool HasUnsavedEdits() noexcept;


    inline void SetActiveIdx(uint32_t activeIdx) noexcept { State().activeScriptIdx = activeIdx; }
    inline uint32_t ActiveIdx() const noexcept { return State().activeScriptIdx; }
    inline std::shared_ptr<Funscript>& ActiveScript() noexcept { return Funscripts[ActiveIdx()]; }

    inline std::filesystem::path const& Path() const noexcept { return lastPath; }
    inline bool IsValid() const noexcept { return valid; }
    inline const std::string& NotValidError() const noexcept { return notValidError; }
    inline ProjectState& State() const noexcept { return ProjectState::State(stateHandle); }

    void ExportFunscripts() noexcept;
    void ExportFunscripts(std::filesystem::path const& outputDir) noexcept;
    void ExportFunscript(std::filesystem::path const& outputPath, int32_t idx) noexcept;

    std::filesystem::path MakePathAbsolute(std::filesystem::path const& relPath) const noexcept;
    std::filesystem::path MakePathRelative(std::filesystem::path const& absPath) const noexcept;
    std::filesystem::path MediaPath() const noexcept;

    template<typename S>
    void serialize(S& s)
    {
        //s.ext(*this, bitsery::ext::Growable{},
        //    [](S& s, OFS_Project& o) {
        //        s.container(o.Funscripts, 100,
        //            [](S& s, std::shared_ptr<Funscript>& script) {
        //                s.ext(script, bitsery::ext::StdSmartPtr{});
        //            });
        //    });
    }

private:
    OFS::StateHandle stateHandle = OFS::StateManager::INVALID_ID;
    OFS::StateHandle bookmarkStateHandle = OFS::StateManager::INVALID_ID;

    std::filesystem::path lastPath;

    std::string notValidError;
    bool valid = false;

    void addError(const std::string& error) noexcept
    {
        valid = false;
        notValidError += "\n";
        notValidError += error;
    }
    void loadNecessaryGlyphs() noexcept;
    void loadMultiAxis(std::filesystem::path const& rootScript) noexcept;

};
