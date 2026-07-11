// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "CoreTypes.hpp"

#include <compare>

/**
 * @brief Generic 4-byte handle type for various use cases, like object references or indices.
 * @tparam TTag Just a tag type to distinguish different handle types.
 */
template <typename TTag>
struct THandle
{
public:
    static constexpr uint32 IndexBits = 24;
    static constexpr uint32 GenerationBits = 8;
    static constexpr uint32 MaxIndex = (1u << IndexBits) - 1;

public:
    uint32 Index : IndexBits = MaxIndex;
    uint32 Generation : GenerationBits = 0;

public:
    [[nodiscard]] constexpr bool8 IsValid() const noexcept { return Index != MaxIndex; }

    [[nodiscard]] constexpr uint32 ToPacked() const
    {
        return Index | (Generation << IndexBits);
    }

    constexpr bool8 operator==(const THandle& Other) const noexcept = default;

public:
    [[nodiscard]] static constexpr THandle Invalid() noexcept { return {}; }
};

static_assert(sizeof(THandle<struct FTestTag>) == 4);

template <typename TTag>
struct std::hash<THandle<TTag>> // NOLINT: intentional std specialization
{
    size_t operator()(const THandle<TTag>& Handle) const noexcept
    {
        return Handle.ToPacked();
    }
};
