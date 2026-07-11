// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <memory>

#include "Memory.hpp"

template <typename T>
using TUniquePtr = std::unique_ptr<T>;

template <typename T>
using TSharedPtr = std::shared_ptr<T>;

template <typename T, typename... TArguments>
[[nodiscard]] TUniquePtr<T> MakeUnique(TArguments&&... Arguments)
{
    void* Pointer = Memory::Allocate(sizeof(T), alignof(T));
    RAVEN_CORE_ASSERT(Pointer != nullptr, "Failed to allocate memory for unique pointer");
    return TUniquePtr<T>(std::construct_at(static_cast<T*>(Pointer), std::forward<TArguments>(Arguments)...));
}

template <typename T, typename... TArguments>
[[nodiscard]] TSharedPtr<T> MakeShared(TArguments&&... Arguments)
{
    void* Pointer = Memory::Allocate(sizeof(T), alignof(T));
    RAVEN_CORE_ASSERT(Pointer != nullptr, "Failed to allocate memory for shared pointer");
    return TSharedPtr<T>(std::construct_at(static_cast<T*>(Pointer), std::forward<TArguments>(Arguments)...));
}
