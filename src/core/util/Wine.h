#pragma once
#include <windows.h>

namespace FStop
{
    inline bool IsWine()
    {
        HMODULE hntdll = GetModuleHandleA("ntdll.dll");
        if (!hntdll)
        {
            return false;
        }

        void* pwine_get_version = reinterpret_cast<void*>(GetProcAddress(hntdll, "wine_get_version"));
        return pwine_get_version != nullptr;
    }
}
