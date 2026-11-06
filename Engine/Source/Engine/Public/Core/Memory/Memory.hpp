// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <memory>
#include <type_traits>

#include "Core/CoreTypes.hpp"

class RAVEN_API FMemory
{
public:
    [[nodiscard]] static void* Allocate(size64 Size, size64 Alignment = 16);
    static void Deallocate(void* Pointer);

public:
    template <typename T, typename... TArguments> requires std::is_constructible_v<T, TArguments...>
    [[nodiscard]] static T* NewObject(TArguments&&... Arguments)
    {
        T* Pointer = static_cast<T*>(Allocate(sizeof(T), alignof(T)));
        return std::construct_at<T>(Pointer, std::forward<TArguments>(Arguments)...);
    }

    template <typename T>
    static void DeleteObject(T* Pointer)
    {
        std::destroy_at<T>(Pointer);
        Deallocate(static_cast<void*>(Pointer));
    }
};
