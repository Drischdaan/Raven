// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <cmath>

#include "Core/CoreTypes.hpp"

namespace Math
{
    inline constexpr float32 Pi = 3.141592653589793f;
    inline constexpr float32 TwoPi = 2.0f * Pi;
    inline constexpr float32 HalfPi = 0.5f * Pi;

    inline constexpr float32 SmallNumber = 1.0e-8f;
    inline constexpr float32 KindaSmallNumber = 1.0e-4f;

    template <typename T>
    [[nodiscard]] constexpr T Min(const T A, const T B)
    {
        return A < B ? A : B;
    }

    template <typename T>
    [[nodiscard]] constexpr T Max(const T A, const T B)
    {
        return A > B ? A : B;
    }

    template <typename T>
    [[nodiscard]] constexpr T Clamp(const T Value, const T Low, const T High)
    {
        return Value < Low ? Low : (Value > High ? High : Value);
    }

    [[nodiscard]] constexpr float32 Saturate(const float32 Value)
    {
        return Clamp(Value, 0.0f, 1.0f);
    }

    template <typename T>
    [[nodiscard]] constexpr T Abs(const T Value)
    {
        return Value < T(0) ? -Value : Value;
    }

    template <typename T>
    [[nodiscard]] constexpr T Square(const T Value)
    {
        return Value * Value;
    }

    template <typename T>
    [[nodiscard]] constexpr T Lerp(const T& A, const T& B, const float32 Alpha)
    {
        return A + (B - A) * Alpha;
    }

    [[nodiscard]] constexpr float32 DegreesToRadians(const float32 Degrees)
    {
        return Degrees * (Pi / 180.0f);
    }

    [[nodiscard]] constexpr float32 RadiansToDegrees(const float32 Radians)
    {
        return Radians * (180.0f / Pi);
    }

    [[nodiscard]] inline bool8 IsNearlyEqual(const float32 A, const float32 B, const float32 Tolerance = KindaSmallNumber)
    {
        return std::abs(A - B) <= Tolerance;
    }

    [[nodiscard]] inline bool8 IsNearlyZero(const float32 Value, const float32 Tolerance = KindaSmallNumber)
    {
        return std::abs(Value) <= Tolerance;
    }

    [[nodiscard]] inline float32 Sqrt(const float32 Value)
    {
        return std::sqrt(Value);
    }

    [[nodiscard]] inline float32 Sin(const float32 Radians)
    {
        return std::sin(Radians);
    }

    [[nodiscard]] inline float32 Cos(const float32 Radians)
    {
        return std::cos(Radians);
    }

    [[nodiscard]] inline float32 Tan(const float32 Radians)
    {
        return std::tan(Radians);
    }

    [[nodiscard]] inline float32 Acos(const float32 Value)
    {
        return std::acos(Clamp(Value, -1.0f, 1.0f));
    }

    [[nodiscard]] inline float32 Atan2(const float32 Y, const float32 X)
    {
        return std::atan2(Y, X);
    }
}
