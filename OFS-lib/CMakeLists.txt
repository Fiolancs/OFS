project(OFS_lib)

set(OFS_LIB_SOURCES
	"event/OFS_EventSystem.cpp"
	"event/OFS_Event.cpp"

	"state/states/KeybindingState.cpp"
	"state/states/ChapterState.cpp"

	"Funscript/Funscript.cpp"
	"Funscript/FunscriptAction.cpp"
	"Funscript/FunscriptUndoSystem.cpp"
	"Funscript/FunscriptHeatmap.cpp"

	"UI/GradientBar.cpp"
	"UI/OFS_ImGui.cpp"
	"UI/OFS_VideoplayerControls.cpp"
	"UI/OFS_Videopreview.cpp"
	"UI/OFS_BlockingTask.cpp"
	"UI/OFS_ScriptTimeline.cpp"
	"UI/ScriptPositionsOverlayMode.cpp"
	"UI/OFS_KeybindingSystem.cpp"
	"UI/OFS_Waveform.cpp"
	
	"videoplayer/OFS_VideoplayerWindow.cpp"
	"videoplayer/impl/OFS_MpvVideoplayer.cpp"

	"state/OFS_StateManager.cpp"
	"state/OFS_LibState.cpp"

	"gl/OFS_Shader.cpp"

	"OFS_ControllerInput.cpp"

	"OFS_Serialization.cpp"
	"OFS_Util.cpp"
	"OFS_FileLogging.cpp"
	"OFS_DynamicFontAtlas.cpp"
	"OFS_MpvLoader.cpp"

	"localization/OFS_Localization.cpp"
	"localization/OFS_StringsGenerated.cpp"
)

set(OFS_LIB_HEADERS 
	"OFS_Util.h"
	"OFS_VectorSet.h"
	"OFS_Serialization.h"
	"OFS_BinarySerialization.h"
	"OFS_Reflection.h"
	"OFS_MpvLoader.h"
	"OFS_GL.h"
	"OFS_FileLogging.h"
	"OFS_DynamicFontAtlas.h"
	"OFS_ControllerInput.h"

	"event/OFS_Event.h"
	"event/OFS_EventSystem.h"

	"Funscript/Funscript.h"
	"Funscript/FunscriptAction.h"
	"Funscript/FunscriptHeatmap.h"
	"Funscript/FunscriptSpline.h"
	"Funscript/FunscriptUndoSystem.h"

	"gl/OFS_Shader.h"

	"localization/OFS_Localization.h"
	"localization/OFS_StringsGenerated.h"

	"state/OFS_LibState.h"
	"state/OFS_StateHandle.h"
	"state/OFS_StateManager.h"
	"state/states/BaseOverlayState.h"
	"state/states/ChapterState.h"
	"state/states/KeybindingState.h"
	"state/states/VideoplayerWindowState.h"
	"state/states/WaveformState.h"

	"UI/GradientBar.h"
	"UI/OFS_BlockingTask.h"
	"UI/OFS_ImGui.h"
	"UI/OFS_KeybindingSystem.h"
	"UI/OFS_Profiling.h"
	"UI/OFS_ScriptTimeline.h"
	"UI/OFS_ScriptTimelineEvents.h"
	"UI/OFS_VideoplayerControls.h"
	"UI/OFS_Videopreview.h"
	"UI/OFS_Waveform.h"
	"UI/ScriptPositionsOverlayMode.h"

	"videoplayer/OFS_Videoplayer.h"
	"videoplayer/OFS_VideoplayerEvents.h"
	"videoplayer/OFS_VideoplayerWindow.h"
)

add_library(${PROJECT_NAME} STATIC ${OFS_LIB_SOURCES} ${OFS_LIB_HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE
	"${PROJECT_SOURCE_DIR}/"
	"${CMAKE_CURRENT_BINARY_DIR}"
)

add_custom_command(OUTPUT "OFS_StringsGenerated.cpp" "OFS_StringsGenerated.h"
                   COMMAND gen_localization ARGS "${CMAKE_SOURCE_DIR}/localization/localization.csv"
                   MAIN_DEPENDENCY "${CMAKE_SOURCE_DIR}/localization/localization.csv"
                   DEPENDS gen_localization
                   VERBATIM)

target_link_libraries(${PROJECT_NAME} PUBLIC
	SDL3-static
	glad2
	imgui
	tinyfiledialogs
	eventpp
	glaze::glaze
	scn::scn
	zpp_bits
	dr_libs
	stb
	tracy
	nlohmann_json
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

find_package(Git)
execute_process(
    COMMAND ${GIT_EXECUTABLE} --git-dir=${CMAKE_SOURCE_DIR}/.git describe --abbrev=0 --tags
    RESULT_VARIABLE result
    OUTPUT_VARIABLE LATEST_GIT_TAG)

execute_process(
    COMMAND ${GIT_EXECUTABLE} --git-dir=${CMAKE_SOURCE_DIR}/.git rev-parse --short HEAD
    RESULT_VARIABLE result
    OUTPUT_VARIABLE LATEST_GIT_HASH)


string(REGEX REPLACE "\n$" "" LATEST_GIT_TAG "${LATEST_GIT_TAG}")
set(LATEST_GIT_TAG "\"${LATEST_GIT_TAG}\"")
message("Compiling with git tag: ${LATEST_GIT_TAG}")

string(REGEX REPLACE "\n$" "" LATEST_GIT_HASH "${LATEST_GIT_HASH}")
set(LATEST_GIT_HASH "\"${LATEST_GIT_HASH}\"")
message("Compiling with git hash: ${LATEST_GIT_HASH}")

target_compile_definitions(${PROJECT_NAME} PUBLIC 
	"OFS_LATEST_GIT_TAG=${LATEST_GIT_TAG}"
	"OFS_LATEST_GIT_HASH=${LATEST_GIT_HASH}"
	"IMGUI_IMPL_OPENGL_LOADER_GLAD2"
	"MPV_ENABLE_DEPRECATED=0"
)

if(OFS_PROFILE)
	target_compile_definitions(${PROJECT_NAME} PUBLIC OFS_PROFILE_ENABLED)
	message("== ${PROJECT_NAME} - Profiling enabled.")
endif()


if(WIN32)
	target_include_directories(${PROJECT_NAME} PUBLIC 
		"../3rdParty/libmpv/include"
	)
elseif(UNIX AND NOT APPLE AND NOT EMSCRIPTEN)
	# linux etc. 
	find_package(PkgConfig REQUIRED) 
	pkg_check_modules(mpv REQUIRED IMPORTED_TARGET mpv)
	
	# mpv is required for the headers
	# but not linked
	# instead libmpv.so.1 is loaded at runtime
	# this avoids linking issues with mpv and lua symbols
elseif(APPLE)
	execute_process(
		COMMAND brew --prefix mpv
		RESULT_VARIABLE BREW_MPV
		OUTPUT_VARIABLE BREW_MPV_PREFIX
		OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	if (BREW_MPV EQUAL 0 AND EXISTS "${BREW_MPV_PREFIX}")
		message(STATUS "Found mpv installed by Homebrew at ${BREW_MPV_PREFIX}")
		set(MPV_INCLUDE "${BREW_MPV_PREFIX}/include")
		set(MPV_LIB "${BREW_MPV_PREFIX}/lib")
	endif()
	target_include_directories(${PROJECT_NAME} PUBLIC
		"${MPV_INCLUDE}"
	)
endif()