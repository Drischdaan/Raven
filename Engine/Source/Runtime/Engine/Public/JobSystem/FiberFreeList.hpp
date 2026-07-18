// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once
#include "CoreTypes.hpp"

#include "Memory/SmartPointers.hpp"

#include "Utility/Atomic.hpp"

/**
 * Treiber stack free list implementation
 */
class ENGINE_API FFiberFreeList
{
public:
    void Initialize(uint32 Capacity);

    void Push(uint32 Index);

    [[nodiscard]] bool8 TryPop(uint32& OutIndex);

private:
    alignas(64) TAtomic<uint64> Head = 0;
    TUniquePtr<TAtomic<uint32>[]> Next;
};
