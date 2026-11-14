// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Utility/RangeAllocator.hpp"

#include <algorithm>

void FRangeAllocator::Initialize(const uint64 InCapacity)
{
    Capacity = InCapacity;
    UsedSize = 0;
    FreeRanges.clear();
    FreeRanges.push_back({.Offset = 0, .Size = Capacity});
}

std::optional<uint64> FRangeAllocator::Allocate(const uint64 Size)
{
    if (Size == 0)
    {
        return std::nullopt;
    }

    for (auto It = FreeRanges.begin(); It != FreeRanges.end(); ++It)
    {
        if (It->Size < Size)
        {
            continue;
        }

        const uint64 Offset = It->Offset;
        It->Offset += Size;
        It->Size -= Size;
        if (It->Size == 0)
        {
            FreeRanges.erase(It);
        }
        UsedSize += Size;
        return Offset;
    }
    return std::nullopt;
}

void FRangeAllocator::Free(const uint64 Offset, const uint64 Size)
{
    if (Size == 0)
    {
        return;
    }

    const auto Next = std::ranges::lower_bound(FreeRanges, Offset, {}, &FRange::Offset);
    auto Inserted = FreeRanges.insert(Next, {.Offset = Offset, .Size = Size});
    UsedSize -= Size;

    if (const auto After = Inserted + 1; After != FreeRanges.end() && Inserted->Offset + Inserted->Size == After->Offset)
    {
        Inserted->Size += After->Size;
        Inserted = FreeRanges.erase(After) - 1;
    }
    if (Inserted != FreeRanges.begin())
    {
        if (const auto Before = Inserted - 1; Before->Offset + Before->Size == Inserted->Offset)
        {
            Before->Size += Inserted->Size;
            FreeRanges.erase(Inserted);
        }
    }
}
