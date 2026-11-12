// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Core/Memory/MemoryResource.hpp"

#include <assert.h>

#include "Core/Memory/Memory.hpp"

void* FMemoryResource::do_allocate(const size64 Size, const size64 Alignment)
{
    void* Pointer = FMemory::Allocate(Size == 0 ? 1 : Size, Alignment);
    if (Pointer == nullptr)
    {
        assert(false && "Invalid pointer allocated");
        std::abort();
    }
    return Pointer;
}

void FMemoryResource::do_deallocate(void* Pointer, [[maybe_unused]] size64 Size, [[maybe_unused]] size64 Alignment)
{
    if (Pointer == nullptr)
    {
        return;
    }
    FMemory::Deallocate(Pointer);
}

bool FMemoryResource::do_is_equal(const memory_resource& Other) const noexcept
{
    return dynamic_cast<const FMemoryResource*>(&Other) != nullptr;
}

FFrameMemoryResource::FFrameMemoryResource(const size64 InReserverSize, const size64 InCommitBlockSize)
    : Arena(InReserverSize, InCommitBlockSize)
{
}

void FFrameMemoryResource::Reset(const bool bShouldDecommit)
{
    Arena.Reset(bShouldDecommit);
}

size64 FFrameMemoryResource::GetUsedSize() const
{
    return Arena.GetUsedSize();
}

size64 FFrameMemoryResource::GetCommittedSize() const
{
    return Arena.GetCommittedSize();
}

size64 FFrameMemoryResource::GetReservedSize() const
{
    return Arena.GetReservedSize();
}

void* FFrameMemoryResource::do_allocate(const size64 Size, const size64 Alignment)
{
    void* Pointer = Arena.Allocate(Size == 0 ? 1 : Size, Alignment);
    if (Pointer == nullptr)
    {
        assert(false && "Invalid pointer allocated from arena");
        std::abort();
    }
    return Pointer;
}

void FFrameMemoryResource::do_deallocate(void* Pointer, size64 Size, size64 Alignment)
{
    // Do nothing, as arena is only reset and never frees actual memory during frame resets
}

bool FFrameMemoryResource::do_is_equal(const memory_resource& Other) const noexcept
{
    return dynamic_cast<const FFrameMemoryResource*>(&Other) != nullptr;
}
