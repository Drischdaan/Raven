// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <type_traits>
#include <utility>

#include "Core/CoreTypes.hpp"

#define RAVEN_ENUM_FLAGS(TEnum) \
    [[nodiscard]] constexpr TEnum operator|(TEnum A, TEnum B) { return static_cast<TEnum>(std::to_underlying(A) | std::to_underlying(B)); } \
    [[nodiscard]] constexpr TEnum operator&(TEnum A, TEnum B) { return static_cast<TEnum>(std::to_underlying(A) & std::to_underlying(B)); } \
    [[nodiscard]] constexpr TEnum operator^(TEnum A, TEnum B) { return static_cast<TEnum>(std::to_underlying(A) ^ std::to_underlying(B)); } \
    [[nodiscard]] constexpr TEnum operator~(TEnum A) { return static_cast<TEnum>(~std::to_underlying(A)); } \
    constexpr TEnum& operator|=(TEnum& A, TEnum B) { return A = A | B; } \
    constexpr TEnum& operator&=(TEnum& A, TEnum B) { return A = A & B; }

template <typename TEnum> requires std::is_enum_v<TEnum>
[[nodiscard]] constexpr bool8 HasAnyFlags(TEnum Value, TEnum Flags)
{
    return (std::to_underlying(Value) & std::to_underlying(Flags)) != 0;
}

template <typename TEnum> requires std::is_enum_v<TEnum>
[[nodiscard]] constexpr bool8 HasAllFlags(TEnum Value, TEnum Flags)
{
    return (std::to_underlying(Value) & std::to_underlying(Flags)) == std::to_underlying(Flags);
}
