project(gen_localization)

set(loc_src
    "gen_localization.cpp"
)

add_executable(${PROJECT_NAME} ${loc_src})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/3rdParty/stb/")
target_link_libraries(${PROJECT_NAME} PRIVATE glaze::glaze)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)
