cmake_minimum_required(VERSION 3.25)

if(NOT TARGET_PLATFORM)
    set(TARGET_PLATFORM "WIN32" CACHE STRING "Target platform")
endif()

# Cross compilation setup
include(cmake/cross-compile.cmake)

# Ccache support
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

project(FStopPatch VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/out/bin")

# External dependencies
include(cmake/async-logger.cmake)
include(cmake/minhook.cmake)

# Git version information
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE PROJECT_GIT_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${GIT_EXECUTABLE} log -1 --format=%cd
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE PROJECT_GIT_COMMIT_DATE
            OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
    set(PROJECT_GIT_COMMIT_HASH "dev")
    set(PROJECT_GIT_COMMIT_DATE "unknown")
endif()
string(TIMESTAMP PROJECT_BUILD_DATE "%Y-%m-%d %H:%M:%S")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(BUILD_TYPE_INT 1)
    add_compile_definitions(_DEBUG)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(BUILD_TYPE_INT 2)
    add_compile_definitions(NDEBUG)
    add_compile_options(-O3)
else()
    set(BUILD_TYPE_INT 0)
endif()

configure_file(
        ${SRC_DIR}/version.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/include/version.h
        @ONLY
)

file(GLOB_RECURSE SRC_FILES
        "${SRC_DIR}/**.cpp"
        "${SRC_DIR}/**.hpp"
        "${SRC_DIR}/**.h"
)

add_library(${PROJECT_NAME} SHARED ${SRC_FILES})
add_library(FStop::FStopPatch ALIAS ${PROJECT_NAME})

set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES
        PREFIX ""
        OUTPUT_NAME "FStopPatch"
        SUFFIX ".dll"
)

target_link_options(${PROJECT_NAME} PRIVATE
        "-Wl,--kill-at"
        "-static"
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${SRC_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/include
        ${minhook_SOURCE_DIR}/include
        ${minhook_SOURCE_DIR}/src/hde
        ${asynclogger_SOURCE_DIR}/src
        ${asynclogger_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME} PUBLIC
        AsyncLogger
        minhook
        comctl32
        shcore
        shell32
        dwmapi
        gdi32
        user32
        kernel32
        ws2_32
        crypt32
        advapi32
        dbghelp
        version
)

target_precompile_headers(${PROJECT_NAME} PUBLIC "${SRC_DIR}/common.hpp")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${OUTPUT_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:${PROJECT_NAME}>
        ${OUTPUT_DIR}/$<TARGET_FILE_NAME:${PROJECT_NAME}>
        COMMENT "Copying ${PROJECT_NAME} to ${OUTPUT_DIR}"
)
