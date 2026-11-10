# CMake toolchain file for cross-compiling to Windows 32-bit (x86/i686) using LLVM-MinGW
# Usage:
#   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-llvm-mingw.cmake -B build
set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR i686)

# Locate LLVM-MinGW installation
if(DEFINED ENV{LLVM_MINGW_ROOT} AND EXISTS "$ENV{LLVM_MINGW_ROOT}")
    set(LLVM_MINGW_ROOT "$ENV{LLVM_MINGW_ROOT}")
elseif(EXISTS "/opt/llvm-mingw")
    set(LLVM_MINGW_ROOT "/opt/llvm-mingw")
else()
    find_program(_LLVM_MINGW_CLANG i686-w64-mingw32-clang)
    if(_LLVM_MINGW_CLANG)
        get_filename_component(_BIN_DIR "${_LLVM_MINGW_CLANG}" DIRECTORY)
        get_filename_component(LLVM_MINGW_ROOT "${_BIN_DIR}" DIRECTORY)
    else()
        message(FATAL_ERROR
            "LLVM-MinGW root not found! Please set LLVM_MINGW_ROOT environment variable "
            "or ensure /opt/llvm-mingw exists or i686-w64-mingw32-clang is in PATH."
        )
    endif()
endif()

set(TOOLCHAIN_TRIPLE "i686-w64-mingw32")

# Set C and C++ compilers
set(CMAKE_C_COMPILER "${LLVM_MINGW_ROOT}/bin/${TOOLCHAIN_TRIPLE}-clang" CACHE FILEPATH "C compiler" FORCE)
set(CMAKE_CXX_COMPILER "${LLVM_MINGW_ROOT}/bin/${TOOLCHAIN_TRIPLE}-clang++" CACHE FILEPATH "C++ compiler" FORCE)

# Set binutils
set(CMAKE_AR "${LLVM_MINGW_ROOT}/bin/llvm-ar" CACHE FILEPATH "Archiver" FORCE)
set(CMAKE_RANLIB "${LLVM_MINGW_ROOT}/bin/llvm-ranlib" CACHE FILEPATH "Ranlib" FORCE)
set(CMAKE_RC_COMPILER "${LLVM_MINGW_ROOT}/bin/llvm-windres" CACHE FILEPATH "RC compiler" FORCE)

# MASM compiler
if(EXISTS "${LLVM_MINGW_ROOT}/bin/llvm-ml")
    set(CMAKE_ASM_MASM_COMPILER "${LLVM_MINGW_ROOT}/bin/llvm-ml" CACHE FILEPATH "MASM compiler" FORCE)
else()
    find_program(_LLVM_ML llvm-ml)
    if(_LLVM_ML)
        set(CMAKE_ASM_MASM_COMPILER "${_LLVM_ML}" CACHE FILEPATH "MASM compiler" FORCE)
    endif()
endif()
set(CMAKE_ASM_MASM_FLAGS "-m32" CACHE STRING "MASM flags" FORCE)

# Sysroot & search paths
set(CMAKE_SYSROOT "${LLVM_MINGW_ROOT}/${TOOLCHAIN_TRIPLE}")

set(CMAKE_FIND_ROOT_PATH
    "${LLVM_MINGW_ROOT}/${TOOLCHAIN_TRIPLE}"
)

# Search mode settings for cross-compilation
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
