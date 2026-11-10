# Cross-compilation settings for Windows (i686 / 32-bit) using LLVM-MinGW
if(_CROSS_COMPILE_CMAKE_INCLUDED)
    return()
endif()
set(_CROSS_COMPILE_CMAKE_INCLUDED TRUE)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_SYSTEM_NAME Windows)
    set(CMAKE_SYSTEM_PROCESSOR i686)

    # Locate LLVM-MinGW installation
    if(NOT LLVM_MINGW_ROOT)
        if(DEFINED ENV{LLVM_MINGW_ROOT})
            set(LLVM_MINGW_ROOT "$ENV{LLVM_MINGW_ROOT}")
        elseif(EXISTS "/opt/llvm-mingw")
            set(LLVM_MINGW_ROOT "/opt/llvm-mingw")
        else()
            find_program(_LLVM_MINGW_CLANG i686-w64-mingw32-clang)
            if(_LLVM_MINGW_CLANG)
                get_filename_component(_BIN_DIR "${_LLVM_MINGW_CLANG}" DIRECTORY)
                get_filename_component(LLVM_MINGW_ROOT "${_BIN_DIR}" DIRECTORY)
            else()
                message(FATAL_ERROR "LLVM-MinGW not found! Please set LLVM_MINGW_ROOT or install to /opt/llvm-mingw")
            endif()
        endif()
    endif()

    set(TOOLCHAIN_TRIPLE "i686-w64-mingw32")
    set(LLVM_MINGW_SYSROOT "${LLVM_MINGW_ROOT}/${TOOLCHAIN_TRIPLE}")

    message(STATUS "Using LLVM-MinGW 32-bit toolchain: ${LLVM_MINGW_ROOT} (${TOOLCHAIN_TRIPLE})")

    # Ensure LLVM-MinGW compilers are set
    set(CMAKE_C_COMPILER "${LLVM_MINGW_ROOT}/bin/${TOOLCHAIN_TRIPLE}-clang" CACHE FILEPATH "C compiler" FORCE)
    set(CMAKE_CXX_COMPILER "${LLVM_MINGW_ROOT}/bin/${TOOLCHAIN_TRIPLE}-clang++" CACHE FILEPATH "CXX compiler" FORCE)
    set(CMAKE_AR "${LLVM_MINGW_ROOT}/bin/llvm-ar" CACHE FILEPATH "Archiver" FORCE)
    set(CMAKE_RANLIB "${LLVM_MINGW_ROOT}/bin/llvm-ranlib" CACHE FILEPATH "Ranlib" FORCE)
    set(CMAKE_RC_COMPILER "${LLVM_MINGW_ROOT}/bin/llvm-windres" CACHE FILEPATH "RC compiler" FORCE)

    # Sysroot & search paths
    set(CMAKE_SYSROOT "${LLVM_MINGW_SYSROOT}")
    set(CMAKE_FIND_ROOT_PATH "${LLVM_MINGW_SYSROOT}")

    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

    set(CMAKE_LINKER_TYPE LLD)

    # Microsoft extensions for 32-bit Windows
    add_compile_options(
            -m32
            -fms-extensions
            -Wno-pragma-pack
    )

    add_compile_definitions(
            WIN32
            _WIN32
            _X86_
            WINVER=0x0A00
            _WIN32_WINNT=0x0A00
            NTDDI_VERSION=0x0A000000
    )
endif()
