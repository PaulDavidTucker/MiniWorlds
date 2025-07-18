cmake_minimum_required(VERSION 3.16)
project(MiniWorlds VERSION 1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/SDL_image EXCLUDE_FROM_ALL)

add_executable(MiniWorlds main.c
    game/game.c game/player.c game/world.c
    core/timer.c
)

target_include_directories(MiniWorlds PRIVATE game core)

# Link to the actual SDL3 library.
target_link_libraries(MiniWorlds PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)
