// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "JobSystem/FiberFreeList.hpp"

void FFiberFreeList::Initialize(uint32 Capacity)
{
    Next = MakeUnique<TAtomic<uint32>[]>(Capacity);
    Head.store(0, std::memory_order_relaxed);
}

void FFiberFreeList::Push(const uint32 Index)
{
    uint64 OldHead = Head.load(std::memory_order_relaxed);
    for (;;)
    {
        Next[Index].store(static_cast<uint32>(OldHead & 0xFFFFFFFFull), std::memory_order_release);
        const uint64 New = (((OldHead >> 32) + 1) << 32) | static_cast<uint64>(Index + 1);
        if (Head.compare_exchange_weak(OldHead, New, std::memory_order_release, std::memory_order_relaxed))
        {
            return;
        }
    }
}

bool8 FFiberFreeList::TryPop(uint32& OutIndex)
{
    uint64 OldHead = Head.load(std::memory_order_acquire);
    for (;;)
    {
        const uint32 Low = static_cast<uint32>(OldHead & 0xFFFFFFFFull);
        if (Low == 0)
        {
            return false;
        }
        const uint32 Index = Low - 1;
        const uint32 NextLow = Next[Index].load(std::memory_order_relaxed);
        const uint64 New = (((OldHead >> 32) + 1) << 32) | static_cast<uint64>(NextLow);
        if (Head.compare_exchange_weak(OldHead, New, std::memory_order_acq_rel, std::memory_order_acquire))
        {
            OutIndex = Index;
            return true;
        }
    }
}
