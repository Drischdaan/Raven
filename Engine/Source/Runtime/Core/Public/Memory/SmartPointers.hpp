// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <memory>

template <typename T>
using TUniquePtr = std::unique_ptr<T>;

template <typename T>
using TSharedPtr = std::shared_ptr<T>;

template <typename T, typename... TArguments>
[[nodiscard]] TUniquePtr<T> MakeUnique(TArguments&&... Arguments)
{
    return std::make_unique<T>(std::forward<TArguments>(Arguments)...);
}

template <typename T, typename... TArguments>
[[nodiscard]] TSharedPtr<T> MakeShared(TArguments&&... Arguments)
{
    return std::make_shared<T>(std::forward<TArguments>(Arguments)...);
}
