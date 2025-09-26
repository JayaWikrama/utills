cmake_minimum_required(VERSION 3.0.0)

# Project configuration
project(utils LANGUAGES C CXX)

# Define versioning variables
set(PROJECT_VERSION_MAJOR "1")
set(PROJECT_VERSION_MINOR "0")
set(PROJECT_VERSION_PATCH "0")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

# Include necessary modules
include(FindPkgConfig)
include(FetchContent)

# Set the default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

# Verbose compile option
option(VERBOSE "Enable verbose compile" OFF)
if(VERBOSE)
  set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

# Define source files
set(SOURCE_FILES
    src/debug.cpp
    src/string.cpp
    src/time.cpp
)

# Create static library
add_library(${PROJECT_NAME}-ar STATIC ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME}-ar PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

# Create shared library from static library
add_library(${PROJECT_NAME}-lib SHARED $<TARGET_OBJECTS:${PROJECT_NAME}-ar>)
set_target_properties(${PROJECT_NAME}-lib PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

# Create test executables
add_executable(${PROJECT_NAME}-test
  test/src/debug.cpp
)

# Include directories for the project
set(INCLUDE_DIRS
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test/include>
  $<INSTALL_INTERFACE:include>
)
target_include_directories(${PROJECT_NAME}-ar PUBLIC ${INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME}-lib PUBLIC ${INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME}-test PUBLIC ${INCLUDE_DIRS})

# Add dependencies for examples
add_dependencies(${PROJECT_NAME}-lib ${PROJECT_NAME}-ar)

target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME}-ar)

# Compiler and linker flags
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fPIC")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -fPIC")
set(CMAKE_C_FLAGS_RELEASE "-O3 -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
