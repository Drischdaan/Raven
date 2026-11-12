// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Core/Memory/MemoryTag.hpp"

namespace MemoryTag::Internal
{
    void RecordAllocation(EMemoryTag Tag, int64 Size);
    void RecordDeallocation(EMemoryTag Tag, int64 Size);
}
