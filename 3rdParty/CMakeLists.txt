# ============
# == OPENGL ==
# ============
set(OpenGL_GL_PREFERENCE "GLVND")
find_package(OpenGL REQUIRED)

# ========
# = GLAD =
# ========
add_library(glad2 STATIC "glad2/src/gl.c")
target_include_directories(glad2 PUBLIC "glad2/include/")

# ========
# = SDL3 =
# ========
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_AUDIO OFF CACHE BOOL "" FORCE)
set(SDL_RENDER OFF CACHE BOOL "" FORCE)
set(SDL_SENSOR OFF CACHE BOOL "" FORCE)
set(SDL_HAPTIC OFF CACHE BOOL "" FORCE)
set(SDL_POWER OFF CACHE BOOL "" FORCE)
set(SDL_LIBC ON CACHE BOOL "" FORCE)
set(SDL_DIRECTX OFF CACHE BOOL "" FORCE)
set(SDL_RENDER_D3D OFF CACHE BOOL "" FORCE)
set(SDL_VULKAN OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
add_subdirectory("SDL/")

# ===============
# === dr_libs ===
# ===============
add_library(dr_libs INTERFACE)
target_include_directories(dr_libs INTERFACE "dr_libs/")

## ==========
## == GLM ===
## ==========
#add_subdirectory("glm")

## ==============
## == bitsery ===
## ==============
#add_library(bitsery INTERFACE)
#target_include_directories(bitsery INTERFACE "bitsery/include")
#add_subdirectory("bitsery/")

## ===========
## == SOL2 ===
## ===========
#set(SOL2_LUA_VERSION "5.4.4" CACHE BOOL "" FORCE)
#set(SOL2_BUILD_LUA OFF CACHE BOOL "" FORCE)
#add_subdirectory("sol2/")

# ==========
# == LUA ===
# ==========
#set (LUA_SOURCES 
#	"lua/lauxlib.c"
#	"lua/lbaselib.c"
#	"lua/ltablib.c"
#	"lua/lutf8lib.c"
#
#	"lua/lmathlib.c"
#	"lua/lstrlib.c"
#	"lua/loadlib.c"
#	"lua/loslib.c"
#	"lua/liolib.c"
#	"lua/ldblib.c"
#	"lua/lcorolib.c"
#
#	"lua/lapi.c"
#	"lua/lstate.c"
#	"lua/ldebug.c"
#	"lua/lzio.c"
#	"lua/llex.c"
#	"lua/lvm.c"
#	"lua/ldump.c"
#	"lua/ltm.c"
#	"lua/lstring.c"
#	"lua/lopcodes.c"
#	"lua/lobject.c"
#	"lua/lmem.c"
#	"lua/linit.c"
#	"lua/ltable.c"
#	"lua/lctype.c"
#	"lua/ldo.c"
#	"lua/lfunc.c"
#	"lua/lundump.c"
#	"lua/lgc.c"
#	"lua/lparser.c"
#	"lua/lcode.c"
#)
#set (LUA_HEADERS 
#	"lua/lua.h"
#	"lua/lauxlib.h"
#	"lua/lualib.h"
#)
#
#add_library(lua STATIC ${LUA_SOURCES} ${LUA_HEADERS})
#target_include_directories(lua PUBLIC "lua/")

# ===========
# === stb ===
# ===========
set (STB_HEADERS "stb/stb_image.h" "stb/stb_image_write.h" "stb/stb_dxt.h")
configure_file("../cmake/stb.c.in" "stb/stb.c")
add_library(stb STATIC "stb/stb.c" ${STB_HEADERS})
target_include_directories(stb PUBLIC "stb/")
target_compile_features(stb PRIVATE c_std_11)

# =========
# = IMGUI =
# =========
set (IMGUI_SOURCES "imgui/imgui.cpp" 
                   "imgui/imgui_draw.cpp" 
				   "imgui/imgui_demo.cpp" 
				   "imgui/imgui_tables.cpp"
				   "imgui/imgui_widgets.cpp"
				   "imgui/backends/imgui_impl_opengl3.cpp"
				   "imgui/backends/imgui_impl_sdl3.cpp"
				   "imgui/misc/cpp/imgui_stdlib.cpp"

				   "imgui/backends/imgui_impl_opengl3.h"
				   "imgui/backends/imgui_impl_opengl3_loader.h"
				   "imgui/backends/imgui_impl_sdl3.h"
				   "imgui/misc/cpp/imgui_stdlib.h"
)
file (GLOB IMGUI_HEADERS imgui/**.h)
add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_HEADERS})
target_include_directories(imgui PUBLIC "imgui/")
target_include_directories(imgui PUBLIC "imgui/misc/cpp/")
target_include_directories(imgui PRIVATE "stb/")
target_include_directories(imgui PUBLIC "SDL/include/")
target_compile_definitions(imgui PUBLIC "IMGUI_USE_STB_SPRINTF")
target_compile_definitions(imgui PUBLIC "IMGUI_DEFINE_MATH_OPERATORS")
target_compile_definitions(imgui PUBLIC "IMGUI_DISABLE_OBSOLETE_FUNCTIONS=1")
target_compile_definitions(imgui PUBLIC "IMGUI_DISABLE_OBSOLETE_KEYIO=1")
target_compile_features(imgui PUBLIC cxx_std_20)

# =============
# === glaze ===
# =============
set(glaze_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
if(NOT OFS_AVX) 
	set(glaze_ENABLE_AVX2 OFF CACHE BOOL "" FORCE)
endif()
add_subdirectory("glaze/")

# ==========
# == json ==
# ==========
set(JSON_Install OFF CACHE BOOL "" FORCE)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
set(JSON_MultipleHeaders ON CACHE BOOL "" FORCE)
add_subdirectory("json/")


# ==============
# = threadpool =
# ==============
add_library(threadpool INTERFACE)
target_include_directories(threadpool INTERFACE "thread-pool/include/")
target_compile_definitions(threadpool INTERFACE "BS_THREAD_POOL_NATIVE_EXTENSIONS")

# ===================
# = tinyfiledialogs = 
# ===================
set(tiny_SRC "tinyfiledialogs/tinyfiledialogs.c")
set(tiny_HEADER "tinyfiledialogs/tinyfiledialogs.h")
add_library(tinyfiledialogs STATIC ${tiny_SRC} ${tiny_HEADER})
target_include_directories(tinyfiledialogs PUBLIC "tinyfiledialogs/")
set_source_files_properties(tiny_SRC PROPERTIES LANGUAGE CXX )

# =============
# === tracy ===
# =============
if(OFS_PROFILE)
	set(tracy_SOURCES
		"tracy/public/TracyClient.cpp"
	)
	add_library(tracy STATIC ${tracy_SOURCES})
	target_compile_definitions(tracy PUBLIC "TRACY_ENABLE")
	target_include_directories(tracy PUBLIC "tracy/public/")
else()
	add_library(tracy INTERFACE)
	target_include_directories(tracy INTERFACE "tracy/public/")
endif()	

# ===========
# = eventpp =
# ===========
add_library(eventpp INTERFACE)
target_include_directories(eventpp INTERFACE "eventpp/include/")

# ============	
# = civetweb =
# ============
set (civetweb_SRC
	"civetweb/src/civetweb.c" 
)
add_library(civetweb STATIC ${civetweb_SRC})
target_include_directories(civetweb PUBLIC "civetweb/include/")
target_compile_definitions(civetweb PRIVATE "NO_SSL=1"
											"USE_WEBSOCKET=1")

# ==============
# === libscn ===
# ==============
set(SCN_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SCN_ENABLE_EXTRAS OFF CACHE BOOL "" FORCE)
add_subdirectory("libscn/")

# ============
# = xorshiro =
# ============
add_library(xoshiro STATIC "xoshiro/src/xoshiro256plus.cpp")
target_include_directories(xoshiro PUBLIC "xoshiro/include/")

# ============
# = zpp_bits =
# ============
add_library(zpp_bits INTERFACE)
target_include_directories(zpp_bits INTERFACE "zpp_bits/")
