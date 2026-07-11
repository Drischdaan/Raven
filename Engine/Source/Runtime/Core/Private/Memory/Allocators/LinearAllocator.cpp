// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Memory/Allocators/LinearAllocator.hpp"

#include <algorithm>

#include "CoreAssertions.hpp"

#include "Memory/Memory.hpp"

FLinearAllocator::FLinearAllocator(const uint64 InReservedSize)
    : ReservedSize(InReservedSize)
{
    BasePointer = static_cast<uint8*>(Memory::ReserveVirtual(ReservedSize));
    RAVEN_CORE_ASSERT(BasePointer != nullptr, "Failed to reserve virtual memory");
}

FLinearAllocator::~FLinearAllocator() noexcept
{
    Memory::ReleaseVirtual(BasePointer, ReservedSize);
}

void* FLinearAllocator::Allocate(const uint64 Size, const uint64 Alignment) noexcept
{
    RAVEN_CORE_ASSERT(Size > 0, "Size must be greater than zero");

    const size64 AlignedOffset = Memory::AlignUp(Offset, Alignment);
    const size64 NewOffset = AlignedOffset + Size;
    RAVEN_CORE_ASSERT(NewOffset <= ReservedSize, "Arena overflow");

    if (NewOffset > CommitedSize) [[unlikely]]
    {
        const size64 Target = std::min(Memory::AlignUp(std::max(NewOffset, CommitedSize * 2), Memory::GetPageSize()), ReservedSize);
        RAVEN_CORE_ASSERT_PERSISTENT(Memory::CommitVirtual(BasePointer + CommitedSize, Target - CommitedSize));
        CommitedSize = Target;
    }

    Offset = NewOffset;
    HighWaterMark = std::max(HighWaterMark, Offset);
    return BasePointer + AlignedOffset;
}

void FLinearAllocator::Reset() noexcept
{
    Offset = 0;
}

size64 FLinearAllocator::GetOffset() const noexcept
{
    return Offset;
}

size64 FLinearAllocator::GetHighWaterMarker() const noexcept
{
    return HighWaterMark;
}

void FLinearAllocator::Rollback(const size64 Mark) noexcept
{
    RAVEN_CORE_ASSERT(Mark <= Offset, "Mark must be less than or equal to the current offset");

    Offset = Mark;
}
