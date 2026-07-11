// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Memory/Memory.hpp"

#include "CoreAssertions.hpp"
#include "mimalloc.h"

#if RAVEN_PLATFORM_WINDOWS
#   include <Windows.h>
#endif

void* Memory::Allocate(const size64 Size, const size64 Alignment) noexcept
{
    RAVEN_CORE_ASSERT(Size > 0, "Size must be greater than 0");

    return mi_malloc_aligned(Size, Alignment);
}

void* Memory::Reallocate(void* Pointer, const size64 Size, const size64 Alignment) noexcept
{
    RAVEN_CORE_ASSERT(Size > 0, "Size must be greater than 0");

    return mi_realloc_aligned(Pointer, Size, Alignment);
}

void Memory::Free(void* Pointer, const size64 Alignment) noexcept
{
    RAVEN_CORE_ASSERT(Pointer != nullptr, "Pointer must not be nullptr");

    mi_free_aligned(Pointer, Alignment);
}

size64 Memory::GetAllocationSize(const void* Pointer) noexcept
{
    RAVEN_CORE_ASSERT(Pointer != nullptr, "Pointer must not be nullptr");

    return mi_malloc_usable_size(Pointer);
}

void* Memory::Copy(void* Destination, const void* Source, const size64 Size) noexcept
{
    RAVEN_CORE_ASSERT(Destination != nullptr, "Destination must not be nullptr");
    RAVEN_CORE_ASSERT(Source != nullptr, "Source must not be nullptr");

    return std::memcpy(Destination, Source, Size);
}

void* Memory::Move(void* Destination, const void* Source, const size64 Size) noexcept
{
    RAVEN_CORE_ASSERT(Destination != nullptr, "Destination must not be nullptr");
    RAVEN_CORE_ASSERT(Source != nullptr, "Source must not be nullptr");

    return std::memmove(Destination, Source, Size);
}

void* Memory::Set(void* Destination, const uint8 Value, const size64 Size) noexcept
{
    RAVEN_CORE_ASSERT(Destination != nullptr, "Destination must not be nullptr");

    return std::memset(Destination, Value, Size);
}

void* Memory::Zero(void* Destination, const size64 Size) noexcept
{
    RAVEN_CORE_ASSERT(Destination != nullptr, "Destination must not be nullptr");

    return std::memset(Destination, 0, Size);
}

int32 Memory::Compare(const void* A, const void* B, const size64 Size) noexcept
{
    RAVEN_CORE_ASSERT(A != nullptr, "A must not be nullptr");
    RAVEN_CORE_ASSERT(B != nullptr, "B must not be nullptr");

    return std::memcmp(A, B, Size);
}

void* Memory::ReserveVirtual(const size64 Size) noexcept
{
    RAVEN_CORE_ASSERT(Size > 0, "Size must be greater than 0");

#if RAVEN_PLATFORM_WINDOWS
    return ::VirtualAlloc(nullptr, Size, MEM_RESERVE, PAGE_NOACCESS);
#else
    RAVEN_CORE_ASSERT(false, "Not implemented");
    return nullptr;
#endif
}

bool Memory::CommitVirtual(void* Pointer, const size64 Size) noexcept
{
    RAVEN_CORE_ASSERT(Pointer != nullptr, "Pointer must not be nullptr");
    RAVEN_CORE_ASSERT(Size > 0, "Size must be greater than 0");

#if RAVEN_PLATFORM_WINDOWS
    return ::VirtualAlloc(Pointer, Size, MEM_COMMIT, PAGE_READWRITE) != nullptr;
#else
    RAVEN_CORE_ASSERT(false, "Not implemented");
    return false;
#endif
}

void Memory::DecommitVirtual(void* Pointer, const size64 Size) noexcept
{
    RAVEN_CORE_ASSERT(Pointer != nullptr, "Pointer must not be nullptr");
    RAVEN_CORE_ASSERT(Size > 0, "Size must be greater than 0");

#if RAVEN_PLATFORM_WINDOWS
    ::VirtualFree(Pointer, Size, MEM_DECOMMIT);
#else
    RAVEN_CORE_ASSERT(false, "Not implemented");
    return;
#endif
}

void Memory::ReleaseVirtual(void* Pointer, const size64 Size) noexcept
{
    RAVEN_CORE_ASSERT(Pointer != nullptr, "Pointer must not be nullptr");
    RAVEN_CORE_ASSERT(Size > 0, "Size must be greater than 0");

#if RAVEN_PLATFORM_WINDOWS
    ::VirtualFree(Pointer, 0, MEM_RELEASE);
#else
    RAVEN_CORE_ASSERT(false, "Not implemented");
    return;
#endif
}

size64 Memory::GetPageSize() noexcept
{
#if RAVEN_PLATFORM_WINDOWS
    static const size64 PageSize = []
    {
        SYSTEM_INFO Info;
        ::GetSystemInfo(&Info);
        return static_cast<size64>(Info.dwPageSize);
    }();
    return PageSize;
#else
    RAVEN_CORE_ASSERT(false, "Not implemented");
    return 0;
#endif
}
