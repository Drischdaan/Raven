// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <memory>
#include <type_traits>

#include "Core/Memory/Memory.hpp"

template <typename T>
struct TSmartDeleter
{
    constexpr TSmartDeleter() noexcept = default;

    template <typename TOther> requires std::is_convertible_v<TOther*, T>
    constexpr TSmartDeleter(const TSmartDeleter<TOther>&) noexcept
    {
    }

    constexpr void operator()(T* Pointer) const noexcept
    {
        static_assert(sizeof(Pointer) > 0, "Can't delete an incomplete type");
        FMemory::DeleteObject(Pointer);
    }
};

template <typename T>
using TUniquePtr = std::unique_ptr<T, TSmartDeleter<T>>;

template <typename T>
using TSharedPtr = std::shared_ptr<T>;

template <typename T, typename... TArguments> requires std::is_constructible_v<T, TArguments...>
[[nodiscard]] TUniquePtr<T> MakeUnique(TArguments&&... Arguments)
{
    T* Pointer = static_cast<T*>(FMemory::Allocate(sizeof(T), alignof(T)));
    return TUniquePtr<T>(std::construct_at<T>(Pointer, std::forward<TArguments>(Arguments)...));
}

template <typename T, typename... TArguments> requires std::is_constructible_v<T, TArguments...>
[[nodiscard]] TSharedPtr<T> MakeShared(TArguments&&... Arguments)
{
    T* Pointer = static_cast<T*>(FMemory::Allocate(sizeof(T), alignof(T)));
    return TSharedPtr<T>(std::construct_at<T>(Pointer, std::forward<TArguments>(Arguments)...), TSmartDeleter<T>());
}
