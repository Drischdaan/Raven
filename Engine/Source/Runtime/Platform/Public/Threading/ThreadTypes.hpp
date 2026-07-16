// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "CoreTypes.hpp"

#include "Memory/Containers/String.hpp"

enum class PLATFORM_API EThreadPriority : uint8
{
    Low = 0,
    Normal = 1,
    High = 2,
    TimeCritical = 3,
};

using FThreadEntryFunction = void(*)(void* Data);

struct FThreadDesc
{
    FThreadEntryFunction EntryFunction = nullptr;
    void* Data = nullptr;
    const TChar* Name = "UnnamedThread";
    uint64 AffinityMask = 0;
    size64 StackSize = 0;
    EThreadPriority Priority = EThreadPriority::Normal;
};
