project(OFS_lib)

set(OFS_LIB_SOURCES
    "OFS_Util.cpp"
    "OFS_ThreadPool.cpp"

    "event/OFS_EventSystem.cpp"
    "event/OFS_Event.cpp"

    "funscript/Funscript.cpp"
    "funscript/FunscriptAction.cpp"
    "funscript/FunscriptUndoSystem.cpp"
    
    "io/OFS_FileDialogs.cpp"
    "io/OFS_FileLogging.cpp"
    "io/OFS_Serialization.cpp"
    "io/OFS_BinarySerialization.cpp"
    
    "state/OFS_StateManager.cpp"

    "localization/OFS_Localization.cpp"
    "localization/OFS_StringsGenerated.cpp"
)

set(OFS_LIB_HEADERS 
    "OFS_Util.h"
    "OFS_TypedID.h"
    "OFS_Profiling.h"
    "OFS_VectorSet.h"
    "OFS_Reflection.h"
    "OFS_DebugBreak.h"
    "OFS_ThreadPool.h"

    "event/OFS_Event.h"
    "event/OFS_EventSystem.h"

    "funscript/Funscript.h"
    "funscript/FunscriptAction.h"
    "funscript/FunscriptSpline.h"
    "funscript/FunscriptUndoSystem.h"

    "io/OFS_FileDialogs.h"
    "io/OFS_FileLogging.h"
    "io/OFS_Serialization.h"
    "io/OFS_BinarySerialization.h"

    "localization/OFS_Localization.h"
    "localization/OFS_StringsGenerated.h"

    "state/OFS_StateHandle.h"
    "state/OFS_StateManager.h"
)

add_library(${PROJECT_NAME} STATIC ${OFS_LIB_SOURCES} ${OFS_LIB_HEADERS})
target_include_directories(${PROJECT_NAME} PRIVATE
    "${PROJECT_SOURCE_DIR}/"
    "${CMAKE_CURRENT_BINARY_DIR}"
)

add_custom_command(OUTPUT "${PROJECT_SOURCE_DIR}/localization/OFS_StringsGenerated.cpp" "${PROJECT_SOURCE_DIR}/localization/OFS_StringsGenerated.h"
                   COMMAND gen_localization ARGS "${CMAKE_SOURCE_DIR}/localization/localization.csv" "${PROJECT_SOURCE_DIR}/localization/"
                   MAIN_DEPENDENCY "${CMAKE_SOURCE_DIR}/localization/localization.csv"
                   DEPENDS gen_localization
                   VERBATIM)

target_link_libraries(${PROJECT_NAME} PUBLIC
    dr_libs
    eventpp 
    glaze::glaze
    nlohmann_json
    scn::scn
    stb
    threadpool
    tinyfiledialogs
    tracy
    xoshiro
    zpp_bits
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

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
    target_compile_definitions(${PROJECT_NAME} PUBLIC TRACY_ENABLE)
    message("== ${PROJECT_NAME} - Profiling enabled.")
endif()


if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PUBLIC UNICODE _UNICODE)
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
