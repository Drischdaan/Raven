// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Core/Memory/Memory.hpp"

void* FMemory::Allocate(const size64 Size, const size64 Alignment)
{
    return _aligned_malloc(Size, Alignment);
}

void FMemory::Deallocate(void* Pointer)
{
    _aligned_free(Pointer);
}
