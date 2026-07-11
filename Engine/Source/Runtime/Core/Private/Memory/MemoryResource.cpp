// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Memory/MemoryResource.hpp"

#include "Memory/Memory.hpp"
#include "Memory/Allocators/LinearAllocator.hpp"

void* FMemoryResource::do_allocate(const size64 Size, const size64 Alignment)
{
    return Memory::Allocate(Size, Alignment);
}

void FMemoryResource::do_deallocate(void* Pointer, [[maybe_unused]] size64 Size, const size64 Alignment)
{
    Memory::Free(Pointer, Alignment);
}

bool8 FMemoryResource::do_is_equal(const memory_resource& Other) const noexcept
{
    return dynamic_cast<const FMemoryResource*>(&Other) != nullptr;
}

FMemoryResource* FMemoryResource::Get()
{
    static FMemoryResource Instance;
    return &Instance;
}

FLinearMemoryResource::FLinearMemoryResource(FLinearAllocator& InLinearAllocator)
    : LinearAllocator(InLinearAllocator)
{
}

void* FLinearMemoryResource::do_allocate(const size64 Size, const size64 Alignment)
{
    return LinearAllocator.Allocate(Size, Alignment);
}

void FLinearMemoryResource::do_deallocate([[maybe_unused]] void* Pointer, [[maybe_unused]] size64 Size, [[maybe_unused]] size64 Alignment)
{
}

bool8 FLinearMemoryResource::do_is_equal(const memory_resource& Other) const noexcept
{
    return dynamic_cast<const FLinearMemoryResource*>(&Other) != nullptr;
}
