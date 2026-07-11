// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <memory>
#include <type_traits>

#include "CoreAssertions.hpp"
#include "CorePlatform.hpp"
#include "CoreTypes.hpp"

namespace Memory
{
    CORE_API void Initialize() noexcept;
    CORE_API void Shutdown() noexcept;

    [[nodiscard]] CORE_API void* Allocate(size64 Size, size64 Alignment = RAVEN_PLATFORM_POINTER_ALIGNMENT) noexcept;
    [[nodiscard]] CORE_API void* Reallocate(void* Pointer, size64 Size, size64 Alignment = RAVEN_PLATFORM_POINTER_ALIGNMENT) noexcept;

    CORE_API void Free(void* Pointer, size64 Alignment = RAVEN_PLATFORM_POINTER_ALIGNMENT) noexcept;

    [[nodiscard]] CORE_API size64 GetAllocationSize(const void* Pointer) noexcept;

    CORE_API void* Copy(void* Destination, const void* Source, size64 Size) noexcept;
    CORE_API void* Move(void* Destination, const void* Source, size64 Size) noexcept;
    CORE_API void* Set(void* Destination, uint8 Value, size64 Size) noexcept;
    CORE_API void* Zero(void* Destination, size64 Size) noexcept;
    [[nodiscard]] CORE_API int32 Compare(const void* A, const void* B, size64 Size) noexcept;

    [[nodiscard]] constexpr bool8 IsPowerOfTwo(const size64 Value) noexcept
    {
        return Value != 0 && (Value & (Value - 1)) == 0;
    }

    [[nodiscard]] constexpr size64 AlignUp(const size64 Value, const size64 Alignment) noexcept
    {
        RAVEN_CORE_ASSERT(IsPowerOfTwo(Alignment), "Alignment must be a power of two");

        return (Value + Alignment - 1) & ~(Alignment - 1);
    }

    [[nodiscard]] constexpr bool IsAligned(const size64 Value, const size64 Alignment) noexcept
    {
        RAVEN_CORE_ASSERT(IsPowerOfTwo(Alignment), "Alignment must be a power of two");

        return (Value & (Alignment - 1)) == 0;
    }

    [[nodiscard]] inline bool IsAligned(const void* Pointer, const size64 Alignment) noexcept
    {
        return IsAligned(reinterpret_cast<size64>(Pointer), Alignment);
    }

    [[nodiscard]] CORE_API void* ReserveVirtual(size64 Size) noexcept;
    CORE_API bool CommitVirtual(void* Pointer, size64 Size) noexcept;
    CORE_API void DecommitVirtual(void* Pointer, size64 Size) noexcept;
    CORE_API void ReleaseVirtual(void* Pointer, size64 Size) noexcept;
    [[nodiscard]] CORE_API size64 GetPageSize() noexcept;

    template <typename T, typename... TArguments> requires std::is_constructible_v<T, TArguments...>
    [[nodiscard]] T* New(TArguments&&... Arguments) noexcept
    {
        void* Pointer = Allocate(sizeof(T), alignof(T));
        return std::construct_at(static_cast<T*>(Pointer), std::forward<TArguments>(Arguments)...);
    }

    template <typename T>
    void Delete(T* Pointer) noexcept
    {
        RAVEN_CORE_ASSERT(Pointer != nullptr, "Pointer must not be nullptr");

        std::destroy_at(Pointer);
        Free(Pointer, alignof(T));
    }

    template <typename T>
    [[nodiscard]] T* AllocateArray(const size64 Count) noexcept
    {
        RAVEN_CORE_ASSERT(Count > 0, "Count must be greater than zero");

        return static_cast<T*>(Allocate(sizeof(T) * Count, alignof(T)));
    }

    template <typename T>
    void FreeArray(T* Pointer) noexcept
    {
        RAVEN_CORE_ASSERT(Pointer != nullptr, "Pointer must not be nullptr");

        Free(Pointer, alignof(T));
    }
}
