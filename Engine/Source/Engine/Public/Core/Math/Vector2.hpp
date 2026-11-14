// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <cassert>
#include <cmath>

#include "MathUtility.hpp"

template <typename T>
struct TVector2
{
    T X = T(0);
    T Y = T(0);

public:
    [[nodiscard]] constexpr T LengthSquared() const
    {
        return X * X + Y * Y;
    }

    [[nodiscard]] T Length() const
    {
        return static_cast<T>(std::sqrt(LengthSquared()));
    }

    [[nodiscard]] TVector2 GetNormalized() const
    {
        const T VectorLength = Length();
        assert(VectorLength > T(Math::KindaSmallNumber) && "Normalizing a zero-length vector");
        return {.X = X / VectorLength, .Y = Y / VectorLength};
    }

    [[nodiscard]] TVector2 GetSafeNormal(const T Tolerance = T(Math::KindaSmallNumber)) const
    {
        const T VectorLength = Length();
        return VectorLength > Tolerance ? TVector2{X / VectorLength, Y / VectorLength} : Zero();
    }

    void Normalize()
    {
        *this = GetNormalized();
    }

public:
    [[nodiscard]] static constexpr TVector2 Zero()
    {
        return {.X = T(0), .Y = T(0)};
    }

    [[nodiscard]] static constexpr TVector2 One()
    {
        return {.X = T(1), .Y = T(1)};
    }

    [[nodiscard]] static constexpr T Dot(const TVector2& A, const TVector2& B)
    {
        return A.X * B.X + A.Y * B.Y;
    }

    [[nodiscard]] static constexpr T Cross(const TVector2& A, const TVector2& B)
    {
        return A.X * B.Y - A.Y * B.X;
    }

    [[nodiscard]] static constexpr TVector2 Min(const TVector2& A, const TVector2& B)
    {
        return {.X = Math::Min(A.X, B.X), .Y = Math::Min(A.Y, B.Y)};
    }

    [[nodiscard]] static constexpr TVector2 Max(const TVector2& A, const TVector2& B)
    {
        return {.X = Math::Max(A.X, B.X), .Y = Math::Max(A.Y, B.Y)};
    }

public:
    [[nodiscard]] constexpr TVector2 operator-() const { return {-X, -Y}; }

    constexpr TVector2& operator+=(const TVector2& Other)
    {
        X += Other.X;
        Y += Other.Y;
        return *this;
    }

    constexpr TVector2& operator-=(const TVector2& Other)
    {
        X -= Other.X;
        Y -= Other.Y;
        return *this;
    }

    constexpr TVector2& operator*=(const T Scalar)
    {
        X *= Scalar;
        Y *= Scalar;
        return *this;
    }

    constexpr TVector2& operator/=(const T Scalar)
    {
        X /= Scalar;
        Y /= Scalar;
        return *this;
    }

    [[nodiscard]] constexpr bool8 operator==(const TVector2&) const = default;
};

template <typename T>
[[nodiscard]] constexpr TVector2<T> operator+(TVector2<T> A, const TVector2<T>& B)
{
    return A += B;
}

template <typename T>
[[nodiscard]] constexpr TVector2<T> operator-(TVector2<T> A, const TVector2<T>& B)
{
    return A -= B;
}

template <typename T>
[[nodiscard]] constexpr TVector2<T> operator*(TVector2<T> A, const T Scalar)
{
    return A *= Scalar;
}

template <typename T>
[[nodiscard]] constexpr TVector2<T> operator*(const T Scalar, TVector2<T> A)
{
    return A *= Scalar;
}

template <typename T>
[[nodiscard]] constexpr TVector2<T> operator/(TVector2<T> A, const T Scalar)
{
    return A /= Scalar;
}

template <typename T>
[[nodiscard]] constexpr TVector2<T> operator*(const TVector2<T>& A, const TVector2<T>& B)
{
    return {A.X * B.X, A.Y * B.Y};
}

using FVector2 = TVector2<float32>;
using FIntVector2 = TVector2<int32>;
