project(OpenFunscripter)

set(OPEN_FUNSCRIPTER_SOURCES
    "main.cpp"
    "OpenFunscripter.cpp"
    "OFS_Project.cpp"
    "OFS_ScriptingMode.cpp"
    "OFS_UndoSystem.cpp"
    "OFS_ControllerInput.cpp"
    "OFS_SDLUtil.cpp"

    "api/OFS_WebsocketApi.cpp"
    "api/OFS_WebsocketApiClient.cpp"
    "api/OFS_WebsocketApiEvents.cpp"
    "api/OFS_WebsocketApiCommands.cpp"

    "funscript/FunscriptHeatmap.cpp"

    "gl/OFS_GPU.cpp"
    "gl/OFS_Shader.cpp"

    "lua/OFS_LuaExtensions.cpp"
    "lua/OFS_LuaExtension.cpp"
    "lua/OFS_LuaCoreExtension.cpp"
    "lua/OFS_LuaExtensionAPI.cpp"
    "lua/api/OFS_LuaPlayerAPI.cpp"
    "lua/api/OFS_LuaImGuiAPI.cpp"
    "lua/api/OFS_LuaScriptAPI.cpp"
    "lua/api/OFS_LuaProcessAPI.cpp"

    "state/OpenFunscripterState.cpp"
    "state/states/ChapterState.cpp"
    "state/states/KeybindingState.cpp"

    "ui/GradientBar.cpp"
    "ui/OFS_Preferences.cpp"
    "ui/OFS_ImGui.cpp"
    "ui/OFS_VideoplayerControls.cpp"
    "ui/OFS_Videopreview.cpp"
    "ui/OFS_BlockingTask.cpp"
    "ui/OFS_ScriptTimeline.cpp"
    "ui/ScriptPositionsOverlayMode.cpp"
    "ui/OFS_KeybindingSystem.cpp"
    "ui/OFS_Waveform.cpp"
    "ui/OFS_ScriptSimulator.cpp"
    "ui/OFS_SpecialFunctions.cpp"
    "ui/OFS_ScriptPositionsOverlays.cpp"
    "ui/OFS_DownloadFfmpeg.cpp"
    "ui/OFS_FunscriptMetadataEditor.cpp"
    "ui/OFS_ChapterManager.cpp"
    "ui/OFS_DynamicFontAtlas.cpp"

    "videoplayer/OFS_MpvLoader.cpp"
    "videoplayer/OFS_VideoplayerWindow.cpp"
    "videoplayer/OFS_MpvVideoplayer.cpp"
)

set(OPEN_FUNSCRIPTER_HEADERS
    "OpenFunscripter.h"
    "OFS_Project.h"
    "OFS_ScriptingMode.h"
    "OFS_UndoSystem.h"
    "OFS_ControllerInput.h"
    "OFS_SDLUtil.h"

    "api/OFS_WebsocketApi.h"
    "api/OFS_WebsocketApiClient.h"
    "api/OFS_WebsocketApiCommands.h"
    "api/OFS_WebsocketApiEvents.h"

    "event/OFS_SDL_Event.h"

    "funscript/FunscriptHeatmap.h"

    "gl/OFS_GL.h"
    "gl/OFS_Shader.h"

    "lua/OFS_Lua.h"
    "lua/OFS_LuaCoreExtension.h"
    "lua/OFS_LuaExtension.h"
    "lua/OFS_LuaExtensionAPI.h"
    "lua/OFS_LuaExtensions.h"
    "lua/api/OFS_LuaImGuiAPI.h"
    "lua/api/OFS_LuaPlayerAPI.h"
    "lua/api/OFS_LuaProcessAPI.h"
    "lua/api/OFS_LuaScriptAPI.h"

    "state/MetadataEditorState.h"
    "state/OpenFunscripterState.h"
    "state/PreferenceState.h"
    "state/ProjectState.h"
    "state/ScriptModeState.h"
    "state/SimulatorState.h"
    "state/SpecialFunctionsState.h"
    "state/WebsocketApiState.h"
    "state/states/BaseOverlayState.h"
    "state/states/ChapterState.h"
    "state/states/KeybindingState.h"
    "state/states/VideoplayerWindowState.h"
    "state/states/WaveformState.h"
    
    "ui/GradientBar.h"
    "ui/OFS_BlockingTask.h"
    "ui/OFS_ImGui.h"
    "ui/OFS_KeybindingSystem.h"
    "ui/OFS_ScriptTimeline.h"
    "ui/OFS_ScriptTimelineEvents.h"
    "ui/OFS_VideoplayerControls.h"
    "ui/OFS_Videopreview.h"
    "ui/OFS_Waveform.h"
    "ui/ScriptPositionsOverlayMode.h"
    "ui/OFS_ChapterManager.h"
    "ui/OFS_DownloadFfmpeg.h"
    "ui/OFS_FunscriptMetadataEditor.h"
    "ui/OFS_Preferences.h"
    "ui/OFS_ScriptPositionsOverlays.h"
    "ui/OFS_ScriptSimulator.h"
    "ui/OFS_SpecialFunctions.h"
    "ui/OFS_DynamicFontAtlas.h"

    "videoplayer/OFS_MpvLoader.h"
    "videoplayer/OFS_Videoplayer.h"
    "videoplayer/OFS_VideoplayerEvents.h"
    "videoplayer/OFS_VideoplayerWindow.h"
)

if(WIN32)
    set(OPEN_FUNSCRIPTER_SOURCES ${OPEN_FUNSCRIPTER_SOURCES} "../icon.rc")
endif()

if(APPLE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${OPEN_FUNSCRIPTER_SOURCES} ${OPEN_FUNSCRIPTER_HEADERS})
elseif(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${OPEN_FUNSCRIPTER_SOURCES} ${OPEN_FUNSCRIPTER_HEADERS})
    set(VS_STARTUP_PROJECT ${PROJECT_NAME})
else()
    add_executable(${PROJECT_NAME} ${OPEN_FUNSCRIPTER_SOURCES})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR})

# copy data directory
if(APPLE)
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${CMAKE_SOURCE_DIR}/data/" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/../Resources/data")
else()
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${CMAKE_SOURCE_DIR}/data/" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/data")
endif()


target_link_libraries(${PROJECT_NAME} PUBLIC
  OFS_lib
  civetweb
  glad2
  imgui
)

if(WIN32)
    # download mpv binaries
    if (NOT EXISTS "${CMAKE_BINARY_DIR}/libmpv-2.dll")
        file(DOWNLOAD "https://sourceforge.net/projects/mpv-player-windows/files/libmpv/mpv-dev-x86_64-v3-20241215-git-e52f975.7z/download" "mpv-dev.7z" SHOW_PROGRESS)
        file(ARCHIVE_EXTRACT INPUT "mpv-dev.7z" DESTINATION "${CMAKE_BINARY_DIR}" PATTERNS "libmpv-2.dll")
    endif()

    # copy mpv-2.dll to bin dir
    add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    "${CMAKE_BINARY_DIR}/libmpv-2.dll" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/libmpv-2.dll"
                    DEPENDS "${CMAKE_BINARY_DIR}/libmpv-2.dll")
endif()

target_include_directories(${PROJECT_NAME} PRIVATE 
    "${PROJECT_SOURCE_DIR}/"
    "${CMAKE_SOURCE_DIR}/OFS-lib/"
)

# c++23
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC UNICODE _UNICODE)
    if(CMAKE_BUILD_TYPE MATCHES "Release" OR CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo") 
        target_compile_options(${PROJECT_NAME} PUBLIC /GL)
        target_link_options(${PROJECT_NAME} PUBLIC /LTCG)
        message("== ${PROJECT_NAME} - Whole program optimization enabled.")
    endif()
elseif(UNIX AND NOT APPLE) # clang/gcc
    target_compile_options(${PROJECT_NAME} PUBLIC -fpermissive)
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "bin/")
elseif(APPLE)
    target_compile_options(${PROJECT_NAME} PUBLIC -fpermissive)
    # Note Mac specific extension .app
    set(APPS "\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.app")
endif()
