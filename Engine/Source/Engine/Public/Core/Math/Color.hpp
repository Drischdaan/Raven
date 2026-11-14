// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <cmath>

#include "MathUtility.hpp"

struct FColor;

struct FLinearColor
{
    float32 R = 0.0f;
    float32 G = 0.0f;
    float32 B = 0.0f;
    float32 A = 1.0f;

public:
    [[nodiscard]] FColor ToSrgb() const;

public:
    constexpr FLinearColor& operator+=(const FLinearColor& Other)
    {
        R += Other.R;
        G += Other.G;
        B += Other.B;
        A += Other.A;
        return *this;
    }

    constexpr FLinearColor& operator-=(const FLinearColor& Other)
    {
        R -= Other.R;
        G -= Other.G;
        B -= Other.B;
        A -= Other.A;
        return *this;
    }

    constexpr FLinearColor& operator*=(const FLinearColor& Other)
    {
        R *= Other.R;
        G *= Other.G;
        B *= Other.B;
        A *= Other.A;
        return *this;
    }

    constexpr FLinearColor& operator*=(const float32 Scalar)
    {
        R *= Scalar;
        G *= Scalar;
        B *= Scalar;
        A *= Scalar;
        return *this;
    }

    [[nodiscard]] constexpr bool8 operator==(const FLinearColor&) const = default;

public:
    [[nodiscard]] static constexpr FLinearColor Black()
    {
        return {.R = 0.0f, .G = 0.0f, .B = 0.0f, .A = 1.0f};
    }

    [[nodiscard]] static constexpr FLinearColor White()
    {
        return {.R = 1.0f, .G = 1.0f, .B = 1.0f, .A = 1.0f};
    }

    [[nodiscard]] static constexpr FLinearColor Transparent()
    {
        return {.R = 0.0f, .G = 0.0f, .B = 0.0f, .A = 0.0f};
    }

    [[nodiscard]] static FLinearColor FromSrgb(const FColor& Color);
};

[[nodiscard]] constexpr FLinearColor operator+(FLinearColor A, const FLinearColor& B)
{
    return A += B;
}

[[nodiscard]] constexpr FLinearColor operator-(FLinearColor A, const FLinearColor& B)
{
    return A -= B;
}

[[nodiscard]] constexpr FLinearColor operator*(FLinearColor A, const FLinearColor& B)
{
    return A *= B;
}

[[nodiscard]] constexpr FLinearColor operator*(FLinearColor A, const float32 Scalar)
{
    return A *= Scalar;
}

struct FColor
{
    uint8 R = 0;
    uint8 G = 0;
    uint8 B = 0;
    uint8 A = 255;

public:
    [[nodiscard]] constexpr uint32 ToPackedRGBA() const
    {
        return static_cast<uint32>(R) | (static_cast<uint32>(G) << 8) | (static_cast<uint32>(B) << 16) | (static_cast<uint32>(A) << 24);
    }

    [[nodiscard]] constexpr bool8 operator==(const FColor&) const = default;
};

namespace MathDetail
{
    inline float32 SrgbToLinear(const float32 Value)
    {
        return Value <= 0.04045f ? Value / 12.92f : std::pow((Value + 0.055f) / 1.055f, 2.4f);
    }

    inline float32 LinearToSrgb(const float32 Value)
    {
        return Value <= 0.0031308f ? Value * 12.92f : 1.055f * std::pow(Value, 1.0f / 2.4f) - 0.055f;
    }

    inline uint8 ToByte(const float32 Value)
    {
        return static_cast<uint8>(Math::Saturate(Value) * 255.0f + 0.5f);
    }
}

inline FLinearColor FLinearColor::FromSrgb(const FColor& Color)
{
    return {
        MathDetail::SrgbToLinear(Color.R / 255.0f),
        MathDetail::SrgbToLinear(Color.G / 255.0f),
        MathDetail::SrgbToLinear(Color.B / 255.0f),
        Color.A / 255.0f,
    };
}

inline FColor FLinearColor::ToSrgb() const
{
    return {
        MathDetail::ToByte(MathDetail::LinearToSrgb(R)),
        MathDetail::ToByte(MathDetail::LinearToSrgb(G)),
        MathDetail::ToByte(MathDetail::LinearToSrgb(B)),
        MathDetail::ToByte(A),
    };
}
