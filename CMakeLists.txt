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
if (MSVC)
  add_compile_options(/MP)
endif()


add_compile_definitions(BOOST_COMPUTE_DEBUG_KERNEL_COMPILATION)
add_compile_definitions(BOOST_COMPUTE_HAVE_THREAD_LOCAL)
add_compile_definitions(BOOST_COMPUTE_THREAD_SAFE)
add_compile_definitions(BOOST_COMPUTE_USE_OFFLINE_CACHE)
add_compile_definitions(BOOST_COMPUTE_USE_CPP11)


include(FetchContent)

FetchContent_Declare(
	spdlog
	GIT_REPOSITORY https://github.com/gabime/spdlog.git
	GIT_TAG v1.15.0
	GIT_PROGRESS TRUE
)
message(STATUS "Fetched Spdlog")

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)
message(STATUS "Fetched Raylib")

FetchContent_Declare(
    OpenCLHeaders
    GIT_REPOSITORY https://github.com/KhronosGroup/OpenCL-Headers.git
    GIT_TAG "main"
    GIT_PROGRESS TRUE
)
message(STATUS "Fetched OpenCLHeaders")

FetchContent_Declare(
    OpenCLICDLoader 
    GIT_REPOSITORY https://github.com/KhronosGroup/OpenCL-ICD-Loader.git
    GIT_TAG "main"
    GIT_PROGRESS TRUE
)
message(STATUS "Fetched OpenCLICDLoader")

FetchContent_Declare(
    Boost
    GIT_REPOSITORY "https://github.com/boostorg/boost"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)
message(STATUS "Fetched Boost (with Boost Compute")

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
message(STATUS "Fetched Nlohmann JSON")

FetchContent_Declare(
    cpplocate
    GIT_REPOSITORY https://github.com/cginternals/cpplocate
    GIT_TAG v2.3.0
)
message(STATUS "Fetched cpplocate")

FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    GIT_TAG v3.8.1
)
message(STATUS "Fetched catch2")

FetchContent_MakeAvailable(spdlog)
message(STATUS "spdlog made availible")
FetchContent_MakeAvailable(raylib)
message(STATUS "Raylib made availible")
FetchContent_MakeAvailable(OpenCLHeaders)
message(STATUS "OpenCLHeaders made availible")
FetchContent_MakeAvailable(OpenCLICDLoader)
message(STATUS "OpenCLICDLoader made availible")
FetchContent_MakeAvailable(Boost)
message(STATUS "Boost made availible")
FetchContent_MakeAvailable(json)
message(STATUS "Nlohmann JSON made availible")
FetchContent_MakeAvailable(cpplocate)
message(STATUS "cpplocate made availible")
FetchContent_MakeAvailable(catch2)
message(STATUS "catch2 made availible")


include_directories("${OpenCLHeaders_SOURCE_DIR}")

file(GLOB_RECURSE SOURCES "source/*.cpp")
file(GLOB_RECURSE INCLUDES "include/game/*.hpp")
add_executable(${PROJECT_NAME} ${SOURCES}  "include/external/rlights.h" ${INCLUDES})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib Boost::compute OpenCL)
target_include_directories(${PROJECT_NAME} PRIVATE include)
#file(COPY "resources/" DESTINATION "resources/")