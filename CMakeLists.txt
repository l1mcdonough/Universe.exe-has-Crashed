cmake_minimum_required(VERSION 3.29)
project(MoldAdventures)
set(CMAKE_CXX_STANDARD 23)

if (EMSCRIPTEN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif ()

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