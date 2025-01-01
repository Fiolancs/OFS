#include "state/OpenFunscripterState.h"
#include "state/ScriptModeState.h"
#include "state/MetadataEditorState.h"
#include "state/PreferenceState.h"
#include "state/SimulatorState.h"
#include "state/ProjectState.h"
#include "state/SpecialFunctionsState.h"
#include "state/WebsocketApiState.h"
#include "state/states/VideoplayerWindowState.h"
#include "state/states/BaseOverlayState.h"
#include "state/states/WaveformState.h"
#include "state/states/KeybindingState.h"
#include "state/states/ChapterState.h"

#include "state/OFS_StateManager.h"
#include "state/OFS_StateHandle.h"


void OpenFunscripterState::RegisterAll() noexcept
{
	// QQQ
	// 
	// This is here so that the states are actually registered 
	// before the app attempts to deserialize them then register them again
	// kiv to see if we should do something about it
	
	// App state
	OFS::AppState<OpenFunscripterState>::registerState(OpenFunscripterState::StateName, OpenFunscripterState::StateName);
	OFS::AppState<ScriptingModeState>::registerState(ScriptingModeState::StateName, ScriptingModeState::StateName);
	OFS::AppState<FunscriptMetadataState>::registerState(FunscriptMetadataState::StateName, FunscriptMetadataState::StateName);
	OFS::AppState<PreferenceState>::registerState(PreferenceState::StateName, PreferenceState::StateName);
	OFS::AppState<SimulatorDefaultConfigState>::registerState(SimulatorDefaultConfigState::StateName, SimulatorDefaultConfigState::StateName);
	OFS::AppState<SpecialFunctionState>::registerState(SpecialFunctionState::StateName, SpecialFunctionState::StateName);
	OFS::AppState<WebsocketApiState>::registerState(WebsocketApiState::StateName, WebsocketApiState::StateName);
	OFS::AppState<OFS_KeybindingState>::registerState(OFS_KeybindingState::StateName, OFS_KeybindingState::StateName);
	OFS::AppState<BaseOverlayState>::registerState(BaseOverlayState::StateName, BaseOverlayState::StateName);
	
	// Project state
	OFS::ProjectState<TempoOverlayState>::registerState(TempoOverlayState::StateName, TempoOverlayState::StateName);
	OFS::ProjectState<ProjectState>::registerState(ProjectState::StateName, ProjectState::StateName);
	OFS::ProjectState<SimulatorState>::registerState(SimulatorState::StateName, SimulatorState::StateName);
	OFS::ProjectState<VideoPlayerWindowState>::registerState(VideoPlayerWindowState::StateName, VideoPlayerWindowState::StateName);
	OFS::ProjectState<WaveformState>::registerState(WaveformState::StateName, WaveformState::StateName);
	OFS::ProjectState<ChapterState>::registerState(ChapterState::StateName, ChapterState::StateName);
}

void OpenFunscripterState::addRecentFile(const RecentFile& recentFile) noexcept
{
	FUN_ASSERT(!recentFile.name.empty(), "bruuh");
    auto it = std::find_if(recentFiles.begin(), recentFiles.end(),
		[&](auto& file) {
			return file.projectPath == recentFile.projectPath;
		});
	if (it != recentFiles.end()) {
		recentFiles.erase(it);
	}
	recentFiles.push_back(recentFile);
	if (recentFiles.size() > 5) {
		recentFiles.erase(recentFiles.begin());
	}
}
