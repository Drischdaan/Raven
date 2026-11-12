// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Core/Memory/Memory.hpp"

#include <cassert>
#include <memory_resource>
#include <algorithm>
#include <limits>
#include <print>

#include "MemoryTagInternal.hpp"
#include "Core/Memory/MemoryResource.hpp"
#include "Core/Memory/MemoryTag.hpp"

namespace
{
    constexpr uint16 AllocationMagic = 0x52A7;

    struct FAllocationHeader
    {
        size64 Size;
        uint32 Offset;
        uint16 Magic;
        EMemoryTag Tag;
        uint8 Reserved;
    };

    static_assert(sizeof(FAllocationHeader) == 16);

    constexpr size64 AlignUp(const size64 Value, const size64 Alignment)
    {
        return (Value + Alignment - 1) & ~(Alignment - 1);
    }

    constexpr bool8 IsPowerOfTwo(const size64 Value)
    {
        return Value != 0 && (Value & (Value - 1)) == 0;
    }

    [[nodiscard]] FAllocationHeader* GetHeader(void* Pointer)
    {
        return static_cast<FAllocationHeader*>(Pointer) - 1;
    }
}

void FMemory::Initialize()
{
    RAVEN_MEMORY_SCOPE(Core);
    GlobalMemoryResource = NewObject<FMemoryResource>();
    std::pmr::set_default_resource(GlobalMemoryResource);
}

void FMemory::Deinitialize()
{
    std::pmr::set_default_resource(nullptr);
    DeleteObject(GlobalMemoryResource);
    GlobalMemoryResource = nullptr;

    // Basic memory leak detection
    for (uint8 Index = 0; Index < std::to_underlying(EMemoryTag::Count); ++Index)
    {
        const FMemoryTagStats& Stats = MemoryTag::GetMemoryTagStats(static_cast<EMemoryTag>(Index));
        if (Stats.AllocationCount != Stats.DeallocationCount)
        {
            std::println("Memory leak detected in {} tag category", MemoryTag::GetMemoryTagName(static_cast<EMemoryTag>(Index)));
            assert(Stats.AllocationCount == Stats.DeallocationCount && "Memory leak detected");
        }
    }
}

void* FMemory::Allocate(const size64 Size, const size64 Alignment)
{
    assert(IsPowerOfTwo(Alignment) && "Alignment must be a power of two");
    const size64 EffectiveAlignment = std::max(Alignment, alignof(FAllocationHeader));
    const size64 Padding = AlignUp(sizeof(FAllocationHeader), EffectiveAlignment);
    assert(Padding <= std::numeric_limits<uint32>::max() && "Alignment too large");
    if (Size > std::numeric_limits<size64>::max() - Padding)
    {
        return nullptr;
    }
    void* BasePointer = _aligned_malloc(Padding + Size, EffectiveAlignment);
    if (BasePointer == nullptr)
    {
        return nullptr;
    }
    void* Pointer = static_cast<uint8*>(BasePointer) + Padding;
    const EMemoryTag Tag = MemoryTag::GetCurrentTag();
    std::construct_at(GetHeader(Pointer), FAllocationHeader{
                          .Size = Size,
                          .Offset = static_cast<uint32>(Padding),
                          .Magic = AllocationMagic,
                          .Tag = Tag,
                          .Reserved = 0,
                      });
    MemoryTag::Internal::RecordAllocation(Tag, static_cast<int64>(Size));
    return Pointer;
}

void FMemory::Deallocate(void* Pointer)
{
    if (Pointer == nullptr)
    {
        return;
    }
    const FAllocationHeader* Header = GetHeader(Pointer);
    assert(Header->Magic == AllocationMagic && "Pointer was not allocated by FMemory");
    MemoryTag::Internal::RecordDeallocation(Header->Tag, static_cast<int64>(Header->Size));
    void* Base = static_cast<uint8*>(Pointer) - Header->Offset;
    _aligned_free(Base);
}
