cmake_minimum_required (VERSION 3.1)
project (sdlxboxmap)

# Set a default build type if none was specified
set(default_build_type "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
## on g++ this ensures: -std=c++11 and not -std=gnu++11
set(CMAKE_CXX_EXTENSIONS OFF)

# See https://stackoverflow.com/questions/29191855/what-is-the-proper-way-to-use-pkg-config-from-cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBEVDEV REQUIRED IMPORTED_TARGET libevdev)

add_subdirectory(libext/easyloggingpp EXCLUDE_FROM_ALL)
target_compile_definitions(easyloggingpp PUBLIC ELPP_NO_DEFAULT_LOG_FILE)

SET(CXXOPTS_BUILD_EXAMPLES OFF CACHE BOOL "Disable to build CXXOPTS examples")
SET(CXXOPTS_BUILD_TESTS OFF CACHE BOOL "Disable to build CXXOPTS tests")
add_subdirectory(libext/cxxopts-2_1 EXCLUDE_FROM_ALL)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

# Set the output folder where your program will be created
set ( EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin )
add_subdirectory("src")

# Strip symbols from release target
set_target_properties(sdlxboxmap PROPERTIES LINK_FLAGS_RELEASE -s)

if ( CMAKE_COMPILER_IS_GNUCC )
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

install(TARGETS sdlxboxmap DESTINATION bin)
