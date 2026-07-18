// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <memory>
#include <type_traits>

#include "Memory.hpp"

template <typename T>
struct TSmartDeleter
{
    constexpr TSmartDeleter() noexcept = default;

    template <typename TOther> requires std::is_convertible_v<TOther*, T*>
    constexpr TSmartDeleter(const TSmartDeleter<TOther>&) noexcept
    {
    }

    constexpr void operator()(T* Pointer) const noexcept
    {
        static_assert(sizeof(Pointer) > 0, "Can't delete an incomplete type");
        Memory::Delete(Pointer);
    }
};

template <typename T>
struct TSmartDeleter<T[]>
{
    constexpr TSmartDeleter() noexcept = default;

    template <typename TOther> requires std::is_convertible_v<TOther*, T*>
    constexpr TSmartDeleter(const TSmartDeleter<TOther>&) noexcept
    {
    }

    constexpr void operator()(std::remove_extent_t<T>* Pointer) const
    {
        Memory::Free(Pointer);
    }
};

template <typename T>
using TUniquePtr = std::unique_ptr<T, TSmartDeleter<T>>;

template <typename T>
using TSharedPtr = std::shared_ptr<T>;

template <typename T, typename... TArguments> requires (!std::is_array_v<T>)
[[nodiscard]] TUniquePtr<T> MakeUnique(TArguments&&... Arguments)
{
    void* Pointer = Memory::Allocate(sizeof(T), alignof(T));
    RAVEN_CORE_ASSERT(Pointer != nullptr, "Failed to allocate memory for unique pointer");
    return TUniquePtr<T>(std::construct_at(static_cast<T*>(Pointer), std::forward<TArguments>(Arguments)...));
}

template <typename T>
    requires std::is_unbounded_array_v<T>
[[nodiscard]] TUniquePtr<T> MakeUnique(const size64 Count)
{
    using FElement = std::remove_extent_t<T>;
    static_assert(std::is_trivially_destructible_v<FElement>, "MakeUnique<T[]> requires trivially destructible elements");
    void* Pointer = Memory::Allocate(sizeof(FElement) * Count, alignof(FElement));
    RAVEN_CORE_ASSERT(Pointer != nullptr, "Failed to allocate memory for unique array");
    FElement* Elements = static_cast<FElement*>(Pointer);
    for (size64 Index = 0; Index < Count; ++Index)
    {
        std::construct_at(Elements + Index);
    }
    return TUniquePtr<T>(Elements);
}

template <typename T, typename... TArguments>
[[nodiscard]] TSharedPtr<T> MakeShared(TArguments&&... Arguments)
{
    void* Pointer = Memory::Allocate(sizeof(T), alignof(T));
    RAVEN_CORE_ASSERT(Pointer != nullptr, "Failed to allocate memory for shared pointer");
    return TSharedPtr<T>(std::construct_at(static_cast<T*>(Pointer), std::forward<TArguments>(Arguments)...), TSmartDeleter<T>());
}
