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
    BoostCompute
    GIT_REPOSITORY "https://github.com/boostorg/compute"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)
set(BOOST_SUBMODULES "Any" "Array" "Algorithm" "Array" "Assert" "Atomic" "Align" "Bind" "Describe" "Chrono" "Conversion" 
        "Concept_Check" "Container_Hash" "Config" "Core" "Compute" "Compat" "Container" "Detail" "Date_Time" 
        "Exception" "Endian" "Function" "Functional" "Function_Types" "FileSystem" "Fusion" 
        "Iterator" "Integer" "Intrusive" "IO" "Lexical_Cast" "MP11" "MPL" "Multi_Index" "Numeric_Conversion"
        "Optional" "Pool" "Preprocessor" "PreDef" "Property_Tree" "Proto" "Phoenix" "Ratio" "Range" "Regex" 
        "Scope" "Serialization" "Static_Assert" "Smart_Ptr" "System" "Spirit"
        "Thread" "Throw_Exception" "Tuple" "TypeOf" "Type_Traits" "Type_Index" "Tokenizer"
        "Unordered" "Utility" "UUID" "Variant" "Variant2" "WinAPI" "Move" )

foreach(submod IN LISTS BOOST_SUBMODULES)
    set(submodString "${submod}")
    string(TOLOWER "${submodString}" submodLowerCaseString)
    FetchContent_Declare(
        Boost${submod}
        GIT_REPOSITORY "https://github.com/boostorg/${submodLowerCaseString}"
        GIT_TAG "master"
        GIT_PROGRESS TRUE
    )
    message(STATUS "Fetched Boost ${submod}")
endforeach()

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
message(STATUS "spdlog made available")
FetchContent_MakeAvailable(raylib)
message(STATUS "Raylib made available")
FetchContent_MakeAvailable(OpenCLHeaders)
message(STATUS "OpenCLHeaders made available")
FetchContent_MakeAvailable(OpenCLICDLoader)
message(STATUS "OpenCLICDLoader made available")
#FetchContent_MakeAvailable(BoostCompute)
#message(STATUS "Boost Compute made available")
foreach(submod IN LISTS BOOST_SUBMODULES)
    set(submodString "${submod}")
    string(TOLOWER "${submodString}" submodLowerCaseString)
    FetchContent_MakeAvailable(Boost${submod})
    message(STATUS "Boost ${submod} made available")
endforeach()
FetchContent_MakeAvailable(json)
message(STATUS "Nlohmann JSON made available")
FetchContent_MakeAvailable(cpplocate)
message(STATUS "cpplocate made available")
FetchContent_MakeAvailable(catch2)
message(STATUS "catch2 made available")


include_directories("${OpenCLHeaders_SOURCE_DIR}")


set(boost_libs "")
foreach(submod IN LISTS BOOST_SUBMODULES)
    set(submodString "${submod}")
    string(TOLOWER "${submodString}" submodLowerCaseString)
    list(APPEND boost_libs "Boost::${submodLowerCaseString}")
endforeach()

file(GLOB_RECURSE INCLUDES "include/game/*.hpp")
file(GLOB_RECURSE LUNDUM_DEMO_SOURCES "source/lundum_demo/*.cpp")
add_executable(LundumDemo ${LUNDUM_DEMO_SOURCES}  "include/external/rlights.h" ${INCLUDES})
target_link_libraries(LundumDemo PRIVATE raylib ${boost_libs} OpenCL)
target_include_directories(LundumDemo PRIVATE include)

file(GLOB_RECURSE TFB_SCRATCHPAD_SOURCE "source/scratchpads/tfb/current.cpp") # TFB for The Floating Brain (Username)
add_executable(TFBScratchpad ${TFB_SCRATCHPAD_SOURCE}  "include/external/rlights.h" ${INCLUDES})
target_link_libraries(TFBScratchpad PRIVATE raylib ${boost_libs} OpenCL)
target_include_directories(TFBScratchpad PRIVATE include)

file(GLOB_RECURSE L1M_SCRATCHPAD_SOURCE "source/scratchpads/l1m/current.cpp") # L1M for l1mcdonough (Username)
add_executable(L1MScratchpad ${L1M_SCRATCHPAD_SOURCE}  "include/external/rlights.h" ${INCLUDES})
target_link_libraries(L1MScratchpad PRIVATE raylib ${boost_libs} OpenCL)
target_include_directories(L1MScratchpad PRIVATE include)
