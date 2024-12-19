cmake_minimum_required(VERSION 3.16)

project(OFS)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# =============
# == OPTIONS ==
# =============
option(OFS_PROFILE OFF)
option(OFS_AVX ON)

if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    if(OFS_AVX) 
        message("OFS AVX ENABLED")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:AVX2")
    endif()
endif()

# ====================
# === DEPENDENCIES ===
# ====================
add_subdirectory("3rdParty/")

# =====================
# ==== BUILD-TOOLS ====
# =====================
add_subdirectory("localization/")

# ==============
# ==== SRC ====
# ==============
add_subdirectory("OFS-lib/")
add_subdirectory("src/")

