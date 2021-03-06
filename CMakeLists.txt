#
#   QOlm CMake build script
#
#   Copyright Olivier Le Doeuff 2020
#

cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ───── OPTIONS ─────

# General
set(QOLM_PROJECT "QOlm" CACHE STRING "Project Name")
set(QOLM_TARGET ${QOLM_PROJECT} CACHE STRING "QOlm library name")

set(QOLM_BUILD_SHARED OFF CACHE BOOL "Build as a shared library (ON) or as static (OFF)")
set(QOLM_FOLDER_PREFIX ${QOLM_PROJECT} CACHE STRING "Prefix folder for all QOlm generated targets in generated project (only decorative)")

# Tests
set(QOLM_ENABLE_TESTS OFF CACHE BOOL "Create or not a target for test (compatible with CTests)")
set(QOLM_TESTS_PREFIX ${QOLM_PROJECT} CACHE STRING "Prefix for all QOlm tests")

# Examples
set(QOLM_ENABLE_EXAMPLES OFF CACHE BOOL "Create or not a target for examples")
set(QOLM_EXAMPLES_PREFIX ${QOLM_PROJECT} CACHE STRING "Prefix for all QOlm tests")

# ───── VERSION ─────

set(QOLM_VERSION_MAJOR 3)
set(QOLM_VERSION_MINOR 0)
set(QOLM_VERSION_PATCH 1)
if( EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
  EXECUTE_PROCESS(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE QOLM_VERSION_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()
if(NOT QOLM_VERSION_TAG)
  set(QOLM_VERSION_TAG 00000000)
endif()
set(QOLM_VERSION_TAG_HEX 0x${QOLM_VERSION_TAG})
set(QOLM_VERSION ${QOLM_VERSION_MAJOR}.${QOLM_VERSION_MINOR}.${QOLM_VERSION_PATCH} CACHE STRING "" FORCE)
set(QOLM_VERSION ${QOLM_VERSION_MAJOR}.${QOLM_VERSION_MINOR}.${QOLM_VERSION_PATCH} CACHE STRING "QOlm current version, this is only decorative and will not configure any files" FORCE)

# ───── PROJECT ─────

project(${QOLM_PROJECT} VERSION ${QOLM_VERSION} LANGUAGES CXX)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ───── CONFIG ─────

message(STATUS "---------------- ${QOLM_PROJECT} ${QOLM_VERSION} OPTIONS. ----------------")

#General
message(STATUS "QOLM_PROJECT          : " ${QOLM_PROJECT})
message(STATUS "QOLM_TARGET           : " ${QOLM_TARGET})
message(STATUS "QOLM_VERSION          : " ${QOLM_VERSION})
message(STATUS "QOLM_VERSION_TAG      : " ${QOLM_VERSION_TAG})
message(STATUS "QOLM_FOLDER_PREFIX    : " ${QOLM_FOLDER_PREFIX})
message(STATUS "QOLM_BUILD_SHARED     : " ${QOLM_BUILD_SHARED})

# Tests
message(STATUS "QOLM_ENABLE_TESTS     : " ${QOLM_ENABLE_TESTS})
if(QOLM_ENABLE_TESTS)
  message(STATUS "QOLM_TESTS_PREFIX     : " ${QOLM_TESTS_PREFIX})
endif()

# Examples
message(STATUS "QOLM_ENABLE_EXAMPLES  : " ${QOLM_ENABLE_EXAMPLES})
if(QOLM_ENABLE_EXAMPLES)
  message(STATUS "QOLM_EXAMPLES_PREFIX  : " ${QOLM_EXAMPLES_PREFIX})
endif()

message(STATUS "---------------- DONE WITH OPTIONS. -----------------")

# ───── DEPENDENCIES ─────

find_package(Qt5 REQUIRED COMPONENTS
  Core
  Qml
)

include(cmake/FetchEventpp.cmake)

# ───── LIBRARY ─────

set(QOLM_SRCS
  include/QOlm/Export.hpp
  include/QOlm/QOlmBase.hpp
  include/QOlm/QOlm.hpp
  src/QOlmBase.cpp
)

if(QOLM_BUILD_SHARED)
  add_library(${QOLM_TARGET} SHARED ${QOLM_SRCS})
  target_compile_definitions(${QOLM_TARGET} PRIVATE "-DQOLM_DLL_EXPORT")
else()
  add_library(${QOLM_TARGET} STATIC  ${QOLM_SRCS})
  target_compile_definitions(${QOLM_TARGET} PUBLIC "-DQOLM_STATIC")
endif()

target_include_directories(${QOLM_TARGET} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(${QOLM_TARGET} PUBLIC cxx_std_17)
target_link_libraries(${QOLM_TARGET} PUBLIC eventpp Qt5::Core Qt5::Qml)

set_target_properties(${QOLM_TARGET} PROPERTIES FOLDER ${QOLM_FOLDER_PREFIX})
source_group("QOlm" FILES ${QOLM_SRCS})
set_target_properties(${QOLM_TARGET} PROPERTIES AUTOMOC TRUE)

# ───── LIBRARY TESTS ─────

if(QOLM_ENABLE_TESTS)
  include(cmake/FetchGTest.cmake)
  enable_testing()
  add_subdirectory(tests)
endif()

# ───── LIBRARY EXAMPLES ─────

if(QOLM_ENABLE_EXAMPLES)
  add_subdirectory(examples)
endif()