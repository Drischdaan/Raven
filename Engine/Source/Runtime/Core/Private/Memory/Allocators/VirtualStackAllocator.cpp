// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Memory/Allocators/VirtualStackAllocator.hpp"

FVirtualStackAllocator::FScope::FScope(FVirtualStackAllocator& InAllocator) noexcept
    : Allocator(InAllocator), Marker(Allocator.GetOffset())
{
}

FVirtualStackAllocator::FScope::~FScope() noexcept
{
    Allocator.Rollback(Marker);
}
