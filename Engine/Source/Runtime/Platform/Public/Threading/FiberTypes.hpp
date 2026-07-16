// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "CoreTypes.hpp"

#include "Memory/Containers/String.hpp"

using FFiberEntryFunction = void (*)(void*);

struct FPlatformFiberDesc
{
    FFiberEntryFunction EntryFunction = nullptr;
    void* Data = nullptr;
    size64 CommitSize = static_cast<size64>(64) * static_cast<size64>(1024);
    size64 ReserveSize = static_cast<size64>(512) * static_cast<size64>(1024);
    const TChar* Name = "UnnamedFiber";
};
