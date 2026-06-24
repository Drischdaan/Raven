// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Core/Containers/MemoryResource.hpp"

#include "Core/Memory/Memory.hpp"

void* FMemoryResource::do_allocate(const size_t Size, const size_t Alignment)
{
	return FMemory::Allocate(Size, Alignment);
}

void FMemoryResource::do_deallocate(void* Pointer, size_t Size, const size_t Alignment)
{
	FMemory::Free(Pointer, Alignment);
}

bool FMemoryResource::do_is_equal(const memory_resource& OtherMemoryResource) const noexcept
{
	return dynamic_cast<const FMemoryResource*>(&OtherMemoryResource) != nullptr;
}

FMemoryResource* FMemoryResource::Get()
{
	static FMemoryResource Instance;
	return &Instance;
}
