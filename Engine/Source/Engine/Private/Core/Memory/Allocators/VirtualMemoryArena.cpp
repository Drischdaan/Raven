// Raven Storm Studio @ 2026. All Rights Reserved.

#include <algorithm>

#include "Core/Memory/Allocators/VirtualMemoryArena.hpp"

#include "Core/Memory/VirtualMemory.hpp"

namespace
{
    constexpr bool8 IsPowerOfTwo(const size64 Value)
    {
        return Value != 0 && (Value & (Value - 1)) == 0;
    }

    constexpr size64 AlignUp(const size64 Value, const size64 Alignment)
    {
        return (Value + Alignment - 1) & ~(Alignment - 1);
    }
}

FVirtualMemoryArena::FVirtualMemoryArena(const size64 InReserveSize, const size64 InCommitBlockSize)
{
    Initialize(InReserveSize, InCommitBlockSize);
}

FVirtualMemoryArena::FVirtualMemoryArena(FVirtualMemoryArena&& Other) noexcept
    : BasePointer(std::exchange(Other.BasePointer, nullptr)),
      ReservedSize(std::exchange(Other.ReservedSize, nullptr)),
      CommittedSize(std::exchange(Other.CommittedSize, nullptr)),
      CommittedBlockSize(std::exchange(Other.CommittedBlockSize, nullptr)),
      Offset(std::exchange(Other.Offset, nullptr))
{
}

FVirtualMemoryArena::~FVirtualMemoryArena()
{
    Deinitialize();
}

FVirtualMemoryArena& FVirtualMemoryArena::operator=(FVirtualMemoryArena&& Other) noexcept
{
    if (this != &Other)
    {
        Deinitialize();
        BasePointer = std::exchange(Other.BasePointer, nullptr);
        ReservedSize = std::exchange(Other.ReservedSize, nullptr);
        CommittedSize = std::exchange(Other.CommittedSize, nullptr);
        CommittedBlockSize = std::exchange(Other.CommittedBlockSize, nullptr);
        Offset = std::exchange(Other.Offset, nullptr);
    }
    return *this;
}

bool8 FVirtualMemoryArena::Initialize(const size64 InReserveSize, const size64 InCommitBlockSize)
{
    Deinitialize();

    if (InReserveSize == 0)
    {
        return false;
    }

    const size64 PageSize = VirtualMemory::GetPageSize();
    const size64 Granularity = VirtualMemory::GetGranularity();

    const size64 AlignedReserveSize = AlignUp(InReserveSize, Granularity);
    void* Reserved = VirtualMemory::Reserve(AlignedReserveSize);
    if (Reserved == nullptr)
    {
        return false;
    }

    BasePointer = static_cast<uint8_t*>(Reserved);
    ReservedSize = AlignedReserveSize;
    CommittedBlockSize = AlignUp(InCommitBlockSize == 0 ? DefaultCommitBlockSize : InCommitBlockSize, PageSize);
    CommittedSize = 0;
    Offset = 0;
    return true;
}

void FVirtualMemoryArena::Deinitialize()
{
    if (BasePointer != nullptr)
    {
        VirtualMemory::Release(BasePointer);
    }
    BasePointer = nullptr;
    ReservedSize = 0;
    CommittedBlockSize = 0;
    CommittedBlockSize = 0;
    Offset = 0;
}

void* FVirtualMemoryArena::Allocate(const size64 Size, const size64 Alignment)
{
    if (BasePointer == nullptr || Size == 0 || !IsPowerOfTwo(Alignment))
    {
        return nullptr;
    }

    const std::uintptr_t BaseAddress = reinterpret_cast<std::uintptr_t>(BasePointer);
    const std::uintptr_t AlignedAddress = AlignUp(BaseAddress + Offset, Alignment);
    const size64 AlignedOffset = AlignedAddress - BaseAddress;

    if (AlignedOffset > ReservedSize || Size > ReservedSize - AlignedOffset)
    {
        return nullptr;
    }

    const size64 NewOffset = AlignedOffset + Size;
    if (NewOffset > CommittedSize)
    {
        size64 NewCommittedSize = AlignUp(NewOffset, CommittedBlockSize);
        NewCommittedSize = std::min(NewCommittedSize, ReservedSize);
        if (!VirtualMemory::Commit(BasePointer + CommittedSize, NewCommittedSize - CommittedSize))
        {
            return nullptr;
        }
        CommittedSize = NewCommittedSize;
    }
    Offset = NewOffset;
    return BasePointer + AlignedOffset;
}

size64 FVirtualMemoryArena::GetMarker() const
{
    return Offset;
}

void FVirtualMemoryArena::PopToMarker(const size64 Marker)
{
    Offset = std::min(Marker, Offset);
}

void FVirtualMemoryArena::Reset(const bool bShouldDecommit)
{
    Offset = 0;
    if (bShouldDecommit && CommittedSize > 0)
    {
        VirtualMemory::Decommit(BasePointer, CommittedSize);
        CommittedSize = 0;
    }
}

bool8 FVirtualMemoryArena::IsInitialized() const
{
    return BasePointer != nullptr;
}

void* FVirtualMemoryArena::GetBasePointer() const
{
    return BasePointer;
}

size64 FVirtualMemoryArena::GetUsedSize() const
{
    return Offset;
}

size64 FVirtualMemoryArena::GetCommittedSize() const
{
    return CommittedSize;
}

size64 FVirtualMemoryArena::GetReservedSize() const
{
    return ReservedSize;
}

bool8 FVirtualMemoryArena::Owns(const void* Pointer) const
{
    const uint8* BytePointer = static_cast<const uint8*>(Pointer);
    return BasePointer != nullptr && BytePointer >= BasePointer && BytePointer < BasePointer + ReservedSize;
}

FScopedArenaMarker::FScopedArenaMarker(FVirtualMemoryArena& InArena)
    : Arena(InArena)
{
    Marker = Arena.GetMarker();
}

FScopedArenaMarker::FScopedArenaMarker(FVirtualMemoryArena& InArena, const size64 InMarker)
    : Arena(InArena), Marker(InMarker)
{
}

FScopedArenaMarker::~FScopedArenaMarker()
{
    Arena.PopToMarker(Marker);
}
