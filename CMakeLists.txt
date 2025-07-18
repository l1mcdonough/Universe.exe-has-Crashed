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
        -DGRAPHICS_API_OPENGL_43
)

#FetchContent_Declare(
#    OpenCL
#    GIT_REPOSITORY "https://github.com/KhronosGroup/OpenCL-SDK.git"
#    GIT_TAG "main"
#    GIT_PROGRESS TRUE
#    CMAKE_ARGS
#        -DBUILD_TESTING=OFF
#        -DBUILD_DOCS=OFF
#        -DBUILD_EXAMPLES=OFF
#        -DBUILD_TESTS=OFF
#        -DOPENCL_SDK_BUILD_SAMPLES=OFF
#        -DOPENCL_SDK_TEST_SAMPLES=OFF
#)

FetchContent_Declare(
    OpenCLHeaders
    GIT_REPOSITORY https://github.com/KhronosGroup/OpenCL-Headers.git
    GIT_TAG "main"
    GIT_PROGRESS TRUE
)


set(boost_patch "${CMAKE_CURRENT_SOURCE_DIR}/patches/random_shuffle.hpp.patch")

FetchContent_Declare(
    Boost
    GIT_REPOSITORY "https://github.com/boostorg/boost"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
    #PATCH_COMMAND git apply ${boost_patch}
    UPDATE_DISCONNECTED 1
)

FetchContent_MakeAvailable(raylib)
#FetchContent_MakeAvailable(OpenCL)
FetchContent_MakeAvailable(OpenCLHeaders)
FetchContent_MakeAvailable(Boost)

#FetchContent_GetProperties(Boost)
#if(NOT Boost_POPULATED)
#  FetchContent_Populate(Boost)
#endif()

set(BOOST_COMPUTE_SOURCE_DIR "${Boost_SOURCE_DIR}/libs/compute")

execute_process(
  COMMAND ${GIT_EXECUTABLE} apply --reverse --check ${boost_patch}
  WORKING_DIRECTORY ${BOOST_COMPUTE_SOURCE_DIR}
  RESULT_VARIABLE _patch_check_result
)

if(NOT _patch_check_result EQUAL 0)
    message(STATUS "ATTENTION: Applying patch to boost-compute in ${BOOST_COMPUTE_SOURCE_DIR} using the file ${boost_patch} and ${GIT_EXECUTABLE} apply")
    # Enter the source directory and apply the patch
    execute_process(
      COMMAND ${GIT_EXECUTABLE} apply --ignore-whitespace --whitespace=fix "${boost_patch}"
      WORKING_DIRECTORY ${BOOST_COMPUTE_SOURCE_DIR}
      RESULT_VARIABLE _patch_result
    )
    if(NOT _patch_result EQUAL 0)
      message(FATAL_ERROR "Failed to apply Boost.Compute shuffle patch")
    endif()
else()
    message(STATUS "ATTENTION: Patch already applied to to boost-compute in ${BOOST_COMPUTE_SOURCE_DIR} using the file ${boost_patch} and ${GIT_EXECUTABLE} apply")
endif()

include_directories("${OpenCLHeaders_SOURCE_DIR}")

file(GLOB_RECURSE SOURCES "source/*.cpp")
file(GLOB_RECURSE INCLUDES "include/game/*.hpp")
add_executable(${PROJECT_NAME} ${SOURCES}  "include/external/rlights.h" ${INCLUDES})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib boost_compute)
target_include_directories(${PROJECT_NAME} PRIVATE include)
#file(COPY "resources/" DESTINATION "resources/")