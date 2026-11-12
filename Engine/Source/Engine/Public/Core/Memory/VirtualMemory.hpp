// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Core/CoreTypes.hpp"

namespace VirtualMemory
{
    [[nodiscard]] RAVEN_API void* Reserve(size64 Size);
    [[nodiscard]] RAVEN_API bool Commit(void* Pointer, size64 Size);
    RAVEN_API void Decommit(void* Pointer, size64 Size);
    RAVEN_API void Release(void* Pointer);

    [[nodiscard]] RAVEN_API size64 GetPageSize();
    [[nodiscard]] RAVEN_API size64 GetGranularity();
}
