cmake_minimum_required(VERSION 3.18)

project(OFS)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# =============
# == OPTIONS ==
# =============
option(OFS_PROFILE OFF)
option(OFS_AVX2 "Enable AVX2" ON)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")    
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")
if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
if(OFS_AVX2) 
    message("OFS AVX2 ENABLED")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>")
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
