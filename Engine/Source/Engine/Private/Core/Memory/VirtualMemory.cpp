// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Core/Memory/VirtualMemory.hpp"

#include <Windows.h>

namespace VirtualMemory
{
    namespace
    {
        bool8 bIsSystemInfoInitialized = false;
        SYSTEM_INFO GSystemInfo = {};
    }

    void* Reserve(const size64 Size)
    {
        return VirtualAlloc(nullptr, Size, MEM_RESERVE, PAGE_NOACCESS);
    }

    bool Commit(void* Pointer, const size64 Size)
    {
        return VirtualAlloc(Pointer, Size, MEM_COMMIT, PAGE_READWRITE) != nullptr;
    }

    void Decommit(void* Pointer, const size64 Size)
    {
        VirtualFree(Pointer, Size, MEM_DECOMMIT);
    }

    void Release(void* Pointer)
    {
        VirtualFree(Pointer, 0, MEM_RELEASE);
    }

    size64 GetPageSize()
    {
        if (!bIsSystemInfoInitialized)
        {
            GetSystemInfo(&GSystemInfo);
            bIsSystemInfoInitialized = true;
        }
        return GSystemInfo.dwPageSize;
    }

    size64 GetGranularity()
    {
        if (!bIsSystemInfoInitialized)
        {
            GetSystemInfo(&GSystemInfo);
            bIsSystemInfoInitialized = true;
        }
        return GSystemInfo.dwAllocationGranularity;
    }
}
