// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Core/Memory/Memory.hpp"

#include <mimalloc.h>

void* FMemory::Allocate(const size64 Size, const size64 Alignment)
{
	return mi_malloc_aligned(Size, Alignment);
}

void* FMemory::Reallocate(void* Pointer, const size64 Size, const size64 Alignment)
{
	return mi_realloc_aligned(Pointer, Size, Alignment);
}

void FMemory::Free(void* Pointer, const size64 Alignment)
{
	if (Pointer == nullptr)
	{
		return;
	}
	mi_free_aligned(Pointer, Alignment);
}

size64 FMemory::GetPointerSize(const void* Pointer)
{
	if (Pointer == nullptr)
	{
		return 0;
	}
	return mi_usable_size(Pointer);
}
