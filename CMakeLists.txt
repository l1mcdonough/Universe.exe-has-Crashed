cmake_minimum_required(VERSION 3.28)
#set(CMAKE_TOOLCHAIN_FILE, "")
project(UniverseExeHasCrashed)
set(CMAKE_CXX_STANDARD 23)

add_compile_definitions(GRAPHICS_API_OPENGL_43)
add_compile_definitions(RESOURCE_DIRECTORY="${PROJECT_SOURCE_DIR}/resources")
if (EMSCRIPTEN)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1")
    set(CMAKE_EXECUTABLE_SUFFIX ".html") # This line is used to set your executable to build with the emscripten html template so that you can directly open it.
endif ()


#set(RAYLIB_GRAPHICS_API "OpenGL 4.3" CACHE STRING "Graphics API for raylib")
#set(GRAPHICS_API_OPENGL_43 ON)
add_compile_definitions(BOOST_COMPUTE_DEBUG_KERNEL_COMPILATION)
add_compile_definitions(BOOST_COMPUTE_HAVE_THREAD_LOCAL)
add_compile_definitions(BOOST_COMPUTE_THREAD_SAFE)
add_compile_definitions(BOOST_COMPUTE_USE_OFFLINE_CACHE)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
    CMAKE_ARGS 
        -DGRAPHICS_API_OPENGL_43=ON
        -DGRAPHICS_API_OPENGL_33=OFF
)
FetchContent_Declare(
    Boost
    GIT_REPOSITORY "https://github.com/boostorg/boost"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)
FetchContent_MakeAvailable(Boost)

file(GLOB_RECURSE SOURCES "source/*.cpp")
file(GLOB_RECURSE INCLUDES "include/game/*.hpp")
add_executable(${PROJECT_NAME} ${SOURCES}  "include/external/rlights.h" ${INCLUDES})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib boost_compute)
target_include_directories(${PROJECT_NAME} PRIVATE include)
#file(COPY "resources/" DESTINATION "resources/")