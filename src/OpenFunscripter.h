#pragma once
#include "OFS_Project.h"
#include "OFS_UndoSystem.h"
#include "OFS_ScriptingMode.h"
#include "event/OFS_SDL_Event.h"
#include "ui/OFS_Preferences.h"
#include "ui/OFS_BlockingTask.h"
#include "ui/OFS_ScriptTimeline.h"
#include "ui/OFS_ChapterManager.h"
#include "ui/OFS_KeybindingSystem.h"
#include "ui/OFS_ScriptSimulator.h"
#include "ui/OFS_SpecialFunctions.h"
#include "ui/OFS_FunscriptMetadataEditor.h"
#include "OFS_ControllerInput.h"
#include "api/OFS_WebsocketApi.h"
#include "lua/OFS_LuaExtensions.h"

#include "ui/GradientBar.h"
#include "ui/OFS_DynamicFontAtlas.h"
#include "io/OFS_FileDialogs.h"
#include "event/OFS_EventSystem.h"
#include "state/OFS_StateManager.h"
#include "ui/OFS_VideoplayerControls.h"
#include "videoplayer/OFS_Videoplayer.h"
#include "videoplayer/OFS_VideoplayerWindow.h"
#include "localization/OFS_Localization.h"

#include <SDL3/SDL.h>

#include <memory>
#include <chrono>

enum OFS_Status : uint8_t {
    OFS_None = 0x0,
    OFS_ShouldExit = 0x1,
    OFS_Fullscreen = 0x1 << 1,
    OFS_GradientNeedsUpdate = 0x1 << 2,
    OFS_GamepadSetPlaybackSpeed = 0x1 << 3,
    OFS_AutoBackup = 0x1 << 4
};

class OpenFunscripter {
private:
    SDL_Window* window;
    SDL_GLContext glContext;

    uint32_t stateHandle = 0xFFFF'FFFF;
    bool ShowMetadataEditor = false;
    bool ShowProjectEditor = false;
#ifndef NDEBUG
    bool DebugDemo = false;
#endif
    bool DebugMetrics = false;
    bool ShowAbout = false;
    bool IdleMode = false;
    uint32_t IdleTimer = 0;

    FunscriptArray CopiedSelection;
    std::chrono::steady_clock::time_point lastBackup;

    char tmpBuf[2][32];

    void setIdle(bool idle) noexcept;
    void registerBindings();

    void update() noexcept;
    void newFrame() noexcept;
    void render() noexcept;
    void autoBackup() noexcept;

    void exitApp(bool force = false) noexcept;

    bool imguiSetup() noexcept;
    void processEvents() noexcept;

    void ExportClip(const class ExportClipForChapter* ev) noexcept;

    void FunscriptChanged(const FunscriptActionsChangedEvent* ev) noexcept;
    void DragNDrop(const OFS_SDL_Event* ev) noexcept;

    void VideoDuration(const DurationChangeEvent* ev) noexcept;
    void VideoLoaded(const VideoLoadedEvent* ev) noexcept;
    void PlayPauseChange(const PlayPauseChangeEvent* ev) noexcept;

    void ControllerAxisPlaybackSpeed(const OFS_SDL_Event* ev) noexcept;

    void ScriptTimelineActionCreated(const FunscriptActionShouldCreateEvent* ev) noexcept;
    void ScriptTimelineActionMoved(const FunscriptActionShouldMoveEvent* ev) noexcept;
    void ScriptTimelineActionClicked(const FunscriptActionClickedEvent* ev) noexcept;
    void ScriptTimelineDoubleClick(const ShouldSetTimeEvent* ev) noexcept;
    void ScriptTimelineSelectTime(const FunscriptShouldSelectTimeEvent* ev) noexcept;
    void ScriptTimelineActiveScriptChanged(const ShouldChangeActiveScriptEvent* ev) noexcept;

    void selectTopPoints() noexcept;
    void selectMiddlePoints() noexcept;
    void selectBottomPoints() noexcept;

    void cutSelection() noexcept;
    void copySelection() noexcept;
    void pasteSelection() noexcept;
    void pasteSelectionExact() noexcept;
    void equalizeSelection() noexcept;
    void invertSelection() noexcept;
    void isolateAction() noexcept;
    void repeatLastStroke() noexcept;

    void saveProject() noexcept;
    void quickExport() noexcept;
    void pickDifferentMedia() noexcept;

    void saveHeatmap(const char* path, int width, int height, bool withChapters);
    void updateTitle() noexcept;

    void removeAction(FunscriptAction action) noexcept;
    void removeAction() noexcept;
    void addEditAction(int pos) noexcept;

    void saveActiveScriptAs();

    void openFile(const std::string& file) noexcept;
    void initProject() noexcept;
    bool closeProject(bool closeWithUnsavedChanges) noexcept;

    void SetFullscreen(bool fullscreen);
    void setupDefaultLayout(bool force) noexcept;

    template<typename OnCloseAction>
    void closeWithoutSavingDialog(OnCloseAction&& action) noexcept;

    // UI
    void CreateDockspace() noexcept;
    void ShowAboutWindow(bool* open) noexcept;
    void ShowStatisticsWindow(bool* open) noexcept;
    void ShowMainMenuBar() noexcept;
    bool ShowMetadataEditorWindow(bool* open) noexcept;

public:
    static OpenFunscripter* ptr;
    uint8_t Status = OFS_Status::OFS_AutoBackup;

    ~OpenFunscripter() noexcept;

    ScriptTimeline scriptTimeline;
    OFS_VideoplayerControls playerControls;
    ScriptSimulator simulator;
    OFS_BlockingTask blockingTask;

    std::unique_ptr<OFS_Videoplayer> player;
    std::unique_ptr<OFS_VideoplayerWindow> playerWindow;
    std::unique_ptr<OFS_KeybindingSystem> keys;
    std::unique_ptr<SpecialFunctionsWindow> specialFunctions;
    std::unique_ptr<ScriptingMode> scripting;
    std::unique_ptr<ControllerInput> controllerInput;
    std::unique_ptr<OFS_Preferences> preferences;
    std::unique_ptr<UndoSystem> undoSystem;
    std::unique_ptr<OFS_LuaExtensions> extensions;
    std::unique_ptr<OFS_FunscriptMetadataEditor> metadataEditor;
    std::unique_ptr<OFS_WebsocketApi> webApi;
    std::unique_ptr<OFS_ChapterManager> chapterMgr;

    std::unique_ptr<OFS_Project> LoadedProject;

    bool Init(int argc, char* argv[]);
    int Run() noexcept;
    void Step() noexcept;
    void Shutdown() noexcept;

    inline const std::vector<std::shared_ptr<Funscript>>& LoadedFunscripts() const noexcept
    {
        return LoadedProject->Funscripts;
    }

    inline std::shared_ptr<Funscript>& ActiveFunscript() noexcept
    {
        return LoadedProject->ActiveScript();
    }

    void UpdateNewActiveScript(uint32_t activeIndex) noexcept;

    inline const FunscriptArray& FunscriptClipboard() const { return CopiedSelection; }

    inline void LoadOverrideFont(const std::string& font) noexcept
    {
        OFS_DynFontAtlas::FontOverride = font;
        OFS_DynFontAtlas::ptr->forceRebuild = true;
    }
    void Undo() noexcept;
    void Redo() noexcept;
};

template <typename OnCloseAction>
inline void OpenFunscripter::closeWithoutSavingDialog(OnCloseAction&& onProjectCloseHandler) noexcept
{
    if (LoadedProject->HasUnsavedEdits()) {
        OFS::util::YesNoCancelDialog(TR(PROJECT_HAS_UNSAVED_EDITS),
            TR(CLOSE_WITHOUT_SAVING_MSG),
            [this, onProjectCloseHandler = std::move(onProjectCloseHandler)](OFS::util::YesNoCancel result) mutable {
                if (result == OFS::util::YesNoCancel::YES) {
                    LoadedProject->Save(true);
                    closeProject(true);
                    onProjectCloseHandler();
                }
                else if (result == OFS::util::YesNoCancel::NO) {
                    /* don't save */
                    closeProject(true);
                    onProjectCloseHandler();
                }
                /* do nothing on cancel */
            });
    }
    else {
        // the project has no edits and can be closed
        closeProject(true);
        onProjectCloseHandler();
    }
}
