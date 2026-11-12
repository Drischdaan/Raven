// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <memory>
#include <type_traits>

#include "Core/CoreTypes.hpp"

class FMemoryResource;

class RAVEN_API FMemory
{
public:
    static void Initialize();
    static void Deinitialize();

    [[nodiscard]] static void* Allocate(size64 Size, size64 Alignment = 16);
    static void Deallocate(void* Pointer);

public:
    template <typename T, typename... TArguments> requires std::is_constructible_v<T, TArguments...>
    [[nodiscard]] static T* NewObject(TArguments&&... Arguments)
    {
        T* Pointer = static_cast<T*>(Allocate(sizeof(T), alignof(T)));
        if (Pointer == nullptr)
        {
            return nullptr;
        }
        return std::construct_at<T>(Pointer, std::forward<TArguments>(Arguments)...);
    }

    template <typename T>
    static void DeleteObject(T* Pointer)
    {
        if (Pointer == nullptr)
        {
            return;
        }
        std::destroy_at<T>(Pointer);
        Deallocate(static_cast<void*>(Pointer));
    }

private:
    inline static FMemoryResource* GlobalMemoryResource = nullptr;
};
