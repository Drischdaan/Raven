// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Core/Memory/MemoryTag.hpp"
#include "Core/Memory/MemoryTagInternal.hpp"
#include "Core/Memory/Containers/Array.hpp"

#include <cassert>
#include <utility>

namespace
{
    constexpr int32 MaxTagDepth = 64;

    struct FMemoryTagStack
    {
        TArray<EMemoryTag, MaxTagDepth> Tags = {};
        int32 Depth = 0;
    };

    thread_local FMemoryTagStack GMemoryTagStack;

    TArray<FMemoryTagStats, std::to_underlying(EMemoryTag::Count)> GMemoryTagStats;

    [[nodiscard]] FMemoryTagStats& GetMutableMemoryTagStats(const EMemoryTag Tag)
    {
        assert(Tag < EMemoryTag::Count && "Invalid memory tag");
        return GMemoryTagStats[std::to_underlying(Tag)];
    }
}

namespace MemoryTag::Internal
{
    void RecordAllocation(const EMemoryTag Tag, const int64 Size)
    {
        FMemoryTagStats& Stats = GetMutableMemoryTagStats(Tag);
        const int64 Now = Stats.AllocatedBytes.fetch_add(Size, std::memory_order_relaxed) + Size;
        Stats.AllocationCount.fetch_add(1, std::memory_order_relaxed);

        int64 Peak = Stats.PeakAllocatedBytes.load(std::memory_order_relaxed);
        while (Now > Peak && !Stats.PeakAllocatedBytes.compare_exchange_weak(Peak, Now, std::memory_order_relaxed))
        {
        }
    }

    void RecordDeallocation(const EMemoryTag Tag, const int64 Size)
    {
        FMemoryTagStats& Stats = GetMutableMemoryTagStats(Tag);
        Stats.AllocatedBytes.fetch_sub(Size, std::memory_order_relaxed);
        Stats.DeallocationCount.fetch_add(1, std::memory_order_relaxed);
    }
}

void MemoryTag::PushTag(const EMemoryTag Tag)
{
    assert(GMemoryTagStack.Depth < MaxTagDepth && "Memory Tag Stack Overflow");
    GMemoryTagStack.Tags[GMemoryTagStack.Depth++] = Tag;
}

void MemoryTag::PopTag()
{
    assert(GMemoryTagStack.Depth > 0 && "Memory Tag Stack Underflow");
    --GMemoryTagStack.Depth;
}

EMemoryTag MemoryTag::GetCurrentTag()
{
    if (GMemoryTagStack.Depth == 0)
    {
        return EMemoryTag::Unknown;
    }
    return GMemoryTagStack.Tags[GMemoryTagStack.Depth - 1];
}

const FMemoryTagStats& MemoryTag::GetMemoryTagStats(const EMemoryTag Tag)
{
    return GetMutableMemoryTagStats(Tag);
}

const TChar* MemoryTag::GetMemoryTagName(const EMemoryTag Tag)
{
    switch (Tag)
    {
    case EMemoryTag::Unknown:
        return "Unknown";
    case EMemoryTag::Core:
        return "Core";
    case EMemoryTag::Strings:
        return "Strings";
    case EMemoryTag::Count:
        return "Count";
    default:
        return "InvalidTag";
    }
}

FMemoryTagScope::FMemoryTagScope(const EMemoryTag InTag)
{
    MemoryTag::PushTag(InTag);
}

FMemoryTagScope::~FMemoryTagScope()
{
    MemoryTag::PopTag();
}
