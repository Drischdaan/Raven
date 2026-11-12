// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Core/Memory/MemoryResource.hpp"

#include "Core/Memory/Memory.hpp"

void* FMemoryResource::do_allocate(const size64 Size, const size64 Alignment)
{
    return FMemory::Allocate(Size, Alignment);
}

void FMemoryResource::do_deallocate(void* Pointer, [[maybe_unused]] size64 Size, [[maybe_unused]] size64 Alignment)
{
    FMemory::Deallocate(Pointer);
}

bool FMemoryResource::do_is_equal(const memory_resource& Other) const noexcept
{
    return dynamic_cast<const FMemoryResource*>(&Other) != nullptr;
}
