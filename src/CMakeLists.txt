project(OpenFunscripter)

set(OPEN_FUNSCRIPTER_SOURCES
  "main.cpp"
  "OpenFunscripter.cpp"
  "OFS_ScriptingMode.cpp"
  "OFS_Project.cpp"
  
  "OFS_UndoSystem.cpp"

  "UI/OFS_Preferences.cpp"
  "UI/OFS_ScriptSimulator.cpp"
  "UI/OFS_SpecialFunctions.cpp"
  "UI/OFS_ScriptPositionsOverlays.cpp"
  "UI/OFS_DownloadFfmpeg.cpp"
  "UI/OFS_FunscriptMetadataEditor.cpp"
  "UI/OFS_ChapterManager.cpp"

  "api/OFS_WebsocketApi.cpp"
  "api/OFS_WebsocketApiClient.cpp"
  "api/OFS_WebsocketApiEvents.cpp"
  "api/OFS_WebsocketApiCommands.cpp"

  "gl/OFS_GPU.cpp"

  "state/OpenFunscripterState.cpp"

  "lua/OFS_LuaExtensions.cpp"
  "lua/OFS_LuaExtension.cpp"
  "lua/OFS_LuaCoreExtension.cpp"
  "lua/OFS_LuaExtensionAPI.cpp"
  "lua/api/OFS_LuaPlayerAPI.cpp"
  "lua/api/OFS_LuaImGuiAPI.cpp"
  "lua/api/OFS_LuaScriptAPI.cpp"
  "lua/api/OFS_LuaProcessAPI.cpp"
)

if(WIN32)
	set(OPEN_FUNSCRIPTER_SOURCES ${OPEN_FUNSCRIPTER_SOURCES} "../icon.rc")
endif()

if(APPLE)
	add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${OPEN_FUNSCRIPTER_SOURCES})
elseif(WIN32)
	add_executable(${PROJECT_NAME} WIN32 ${OPEN_FUNSCRIPTER_SOURCES})
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
  SDL3::SDL3-static
)

if(WIN32)
	# download mpv binaries
	if (NOT EXISTS "${CMAKE_BINARY_DIR}/mpv-2.dll")
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
