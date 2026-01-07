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

if(NOT DEFINED CMAKE_CXX_STANDARD)
  include(CheckCXXCompilerFlag)
  check_cxx_compiler_flag("-std=c++17" COMPILER_SUPPORTS_CXX17)
  check_cxx_compiler_flag("-std=c++14" COMPILER_SUPPORTS_CXX14)
  check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)
endif()

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
  src/txtlog.cpp
  src/string.cpp
  src/time.cpp
  src/error.cpp
  src/json-validator.cpp
)

set(TEST_SOURCE_FILES
  test/src/binary-tree.cpp
  test/src/time.cpp
  test/src/string.cpp
  test/src/debug.cpp
)

# Create object
add_library(${PROJECT_NAME}-obj OBJECT ${SOURCE_FILES})

# Create static library
add_library(${PROJECT_NAME}-ar STATIC $<TARGET_OBJECTS:${PROJECT_NAME}-obj>)
set_target_properties(${PROJECT_NAME}-ar PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

# Create shared library from static library
add_library(${PROJECT_NAME}-lib SHARED $<TARGET_OBJECTS:${PROJECT_NAME}-obj>)
set_target_properties(${PROJECT_NAME}-lib PROPERTIES OUTPUT_NAME ${PROJECT_NAME})

# Create test executables
add_executable(${PROJECT_NAME}-logger tools/logger.cpp)
add_executable(${PROJECT_NAME}-test test/main.cpp ${TEST_SOURCE_FILES})

# Include directories for the project
set(INCLUDE_DIRS
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/test/include>
  $<INSTALL_INTERFACE:include>
)
target_include_directories(${PROJECT_NAME}-obj PUBLIC ${INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME}-ar PUBLIC ${INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME}-lib PUBLIC ${INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME}-test PUBLIC ${INCLUDE_DIRS})

# Add dependencies for examples
add_dependencies(${PROJECT_NAME}-lib ${PROJECT_NAME}-ar)

target_link_libraries(${PROJECT_NAME}-logger PRIVATE ${PROJECT_NAME}-ar)
target_link_libraries(${PROJECT_NAME}-logger PUBLIC lzma)
target_link_libraries(${PROJECT_NAME}-test PRIVATE ${PROJECT_NAME}-ar)
target_link_libraries(${PROJECT_NAME}-test PUBLIC lzma)

# Compiler and linker flags
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -fPIC")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -fPIC")
set(CMAKE_C_FLAGS_RELEASE "-O3 -fPIC")
if(NOT DEFINED CMAKE_CXX_STANDARD)
  if(COMPILER_SUPPORTS_CXX17)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
    add_definitions(-DCXXSTD_17)
    message(STATUS "Use std=c++17...")
  elseif(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    add_definitions(-DCXXSTD_14)
    message(STATUS "Use std=c++14...")
  elseif(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    add_definitions(-DCXXSTD_11)
    message(STATUS "Use std=c++11...")
  else()
    message(FATAL_ERROR "Your compiler does not support C++11 or higher.")
  endif()
else()
  message(STATUS "Use predefined stdc++.")
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
