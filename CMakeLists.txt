cmake_minimum_required(VERSION 3.29)
project(MoldAdventures)
set(CMAKE_CXX_STANDARD 23)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)

file(GLOB_RECURSE SOURCES "source/*.cpp")
file(GLOB_RECURSE INCLUDES "include/game/*.hpp")
add_executable(MoldAdventures ${SOURCES}  "include/external/rlights.h" ${INCLUDES})
target_link_libraries(MoldAdventures PRIVATE raylib)
target_include_directories(MoldAdventures PRIVATE include)
file(COPY "resources/" DESTINATION "resources/")