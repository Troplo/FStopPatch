#pragma once

#include <AsyncLogger/Logger.hpp>
#include <MinHook.h>
#include <windows.h>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <map>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include <unordered_map>
#include <format>

#include "version.h"

#ifndef TNT_MAX_PATH
#define TNT_MAX_PATH 32768
#endif

#if defined(_MSC_VER)
    #define FORCE_INLINE __forceinline
#else
    #define FORCE_INLINE __attribute__((always_inline)) inline
#endif

#if __DEBUG
    #define DEBUG_ONLY(x) x
#else
    #define DEBUG_ONLY(x) ""
#endif

#if __RELEASE
    #define RELEASE_ONLY(x) x
#else
    #define RELEASE_ONLY(x) ""
#endif

#if !__DEBUG
    #ifndef NDEBUG
        #define NDEBUG 1
    #endif
    #undef _DEBUG
#endif

#if __DEBUG
inline void _BreakInDebugger()
{
    al::LOG(al::FATAL) << "Breaking in debugger";
    al::Logger::FlushQueue();
    if (IsDebuggerPresent())
    {
        __debugbreak();
    }
    else
    {
        assert(false && "BreakInDebugger called but no debugger is attached");
    }
}
#define TNT_DEBUGBREAK _BreakInDebugger();
#define TNT_DEBUGBREAK_IF(cond) if (cond) _BreakInDebugger();
#else
#define TNT_DEBUGBREAK {}
#define TNT_DEBUGBREAK_IF(cond) {}
#endif

using namespace al;

namespace FStop
{
    using namespace std::chrono_literals;
    using namespace std::string_view_literals;

    extern std::atomic<bool> g_Running;
    extern HINSTANCE g_DllInstance;
    extern HANDLE g_MainThread;
    extern DWORD g_MainThreadId;
}
