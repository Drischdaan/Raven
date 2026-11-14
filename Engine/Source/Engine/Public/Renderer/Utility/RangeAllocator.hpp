// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <optional>

#include "Core/CoreTypes.hpp"
#include "Core/Memory/Containers/Vector.hpp"

class FRangeAllocator
{
public:
    void Initialize(uint64 InCapacity);

    [[nodiscard]] std::optional<uint64> Allocate(uint64 Size);
    void Free(uint64 Offset, uint64 Size);

    [[nodiscard]] uint64 GetCapacity() const
    {
        return Capacity;
    }

    [[nodiscard]] uint64 GetUsedSize() const
    {
        return UsedSize;
    }

private:
    struct FRange
    {
        uint64 Offset = 0;
        uint64 Size = 0;
    };

    TVector<FRange> FreeRanges;
    uint64 Capacity = 0;
    uint64 UsedSize = 0;
};
