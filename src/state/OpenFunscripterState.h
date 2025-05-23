#pragma once
#include "OFS_Reflection.h"

#include "state/OFS_StateHandle.h"

#include <string>
#include <vector>
#include <filesystem>


struct RecentFile 
{
	std::string name;
	std::filesystem::path projectPath;
};

//REFL_TYPE(RecentFile)
//	REFL_FIELD(name)
//	REFL_FIELD(projectPath)
//REFL_END

struct OpenFunscripterState 
{
	static constexpr auto StateName = "OpenFunscripter";

	std::vector<RecentFile> recentFiles;
    std::filesystem::path lastPath;

	struct HeatmapSettings {
		int32_t defaultWidth = 2000;
		int32_t defaultHeight = 50;
		std::string defaultPath = "./";
	} heatmapSettings;

    bool showDebugLog = false;
    bool showVideo = true;

    bool showActionEditor = false;
    bool showStatistics = true;
    bool alwaysShowBookmarkLabels = false;
    bool showHistory = true;
    bool showSimulator = true;
    bool showSpecialFunctions = false;
    bool showWsApi = false;
    bool showChapterManager = false;

    inline static OpenFunscripterState& State(OFS::StateHandle stateHandle) noexcept
    {
        return OFS::AppState<OpenFunscripterState>(stateHandle).get();
    }

    void addRecentFile(const RecentFile& recentFile) noexcept;

    static void RegisterAll() noexcept;
};

//REFL_TYPE(OpenFunscripterState::HeatmapSettings)
//	REFL_FIELD(defaultWidth)
//	REFL_FIELD(defaultHeight)
//	REFL_FIELD(defaultPath)
//REFL_END
//
//REFL_TYPE(OpenFunscripterState)
//    REFL_FIELD(recentFiles)
//    REFL_FIELD(lastPath)
//    REFL_FIELD(showDebugLog)
//    REFL_FIELD(showVideo)
//    REFL_FIELD(alwaysShowBookmarkLabels)
//    REFL_FIELD(showHistory)
//    REFL_FIELD(showSimulator)
//    REFL_FIELD(showSpecialFunctions)
//    REFL_FIELD(showWsApi)
//    REFL_FIELD(showChapterManager)
//REFL_END