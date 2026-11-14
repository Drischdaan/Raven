// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <cassert>

#include "Vector3.hpp"

template <typename T>
struct alignas(16) TVector4
{
    T X = T(0);
    T Y = T(0);
    T Z = T(0);
    T W = T(0);

public:
    [[nodiscard]] constexpr TVector3<T> GetXYZ() const
    {
        return {X, Y, Z};
    }

    [[nodiscard]] constexpr T LengthSquared() const
    {
        return X * X + Y * Y + Z * Z + W * W;
    }

    [[nodiscard]] T Length() const
    {
        return static_cast<T>(std::sqrt(LengthSquared()));
    }

    [[nodiscard]] TVector4 GetNormalized() const
    {
        const T VectorLength = Length();
        assert(VectorLength > T(Math::SmallNumber) && "Normalizing a zero-length vector");
        return {.X = X / VectorLength, .Y = Y / VectorLength, .Z = Z / VectorLength, .W = W / VectorLength};
    }

    void Normalize()
    {
        *this = GetNormalized();
    }

    [[nodiscard]] constexpr T& operator[](const uint32 Index)
    {
        assert(Index < 4);
        return Index == 0 ? X : (Index == 1 ? Y : (Index == 2 ? Z : W));
    }

    [[nodiscard]] constexpr const T& operator[](const uint32 Index) const
    {
        assert(Index < 4);
        return Index == 0 ? X : (Index == 1 ? Y : (Index == 2 ? Z : W));
    }

public:
    [[nodiscard]] static constexpr TVector4 Zero()
    {
        return {.X = T(0), .Y = T(0), .Z = T(0), .W = T(0)};
    }

    [[nodiscard]] static constexpr TVector4 One()
    {
        return {.X = T(1), .Y = T(1), .Z = T(1), .W = T(1)};
    }

    [[nodiscard]] static constexpr TVector4 FromVector3(const TVector3<T>& Vector, const T InW)
    {
        return {.X = Vector.X, .Y = Vector.Y, .Z = Vector.Z, .W = InW};
    }

    [[nodiscard]] static constexpr T Dot(const TVector4& A, const TVector4& B)
    {
        return A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
    }

public:
    [[nodiscard]] constexpr TVector4 operator-() const
    {
        return {.X = -X, .Y = -Y, .Z = -Z, .W = -W};
    }

    constexpr TVector4& operator+=(const TVector4& Other)
    {
        X += Other.X;
        Y += Other.Y;
        Z += Other.Z;
        W += Other.W;
        return *this;
    }

    constexpr TVector4& operator-=(const TVector4& Other)
    {
        X -= Other.X;
        Y -= Other.Y;
        Z -= Other.Z;
        W -= Other.W;
        return *this;
    }

    constexpr TVector4& operator*=(const TVector4& Other)
    {
        X *= Other.X;
        Y *= Other.Y;
        Z *= Other.Z;
        W *= Other.W;
        return *this;
    }

    constexpr TVector4& operator*=(const T Scalar)
    {
        X *= Scalar;
        Y *= Scalar;
        Z *= Scalar;
        W *= Scalar;
        return *this;
    }

    constexpr TVector4& operator/=(const T Scalar)
    {
        X /= Scalar;
        Y /= Scalar;
        Z /= Scalar;
        W /= Scalar;
        return *this;
    }

    [[nodiscard]] constexpr bool8 operator==(const TVector4&) const = default;
};

template <typename T>
[[nodiscard]] constexpr TVector4<T> operator+(TVector4<T> A, const TVector4<T>& B)
{
    return A += B;
}

template <typename T>
[[nodiscard]] constexpr TVector4<T> operator-(TVector4<T> A, const TVector4<T>& B)
{
    return A -= B;
}

template <typename T>
[[nodiscard]] constexpr TVector4<T> operator*(TVector4<T> A, const TVector4<T>& B)
{
    return A *= B;
}

template <typename T>
[[nodiscard]] constexpr TVector4<T> operator*(TVector4<T> A, const T Scalar)
{
    return A *= Scalar;
}

template <typename T>
[[nodiscard]] constexpr TVector4<T> operator*(const T Scalar, TVector4<T> A)
{
    return A *= Scalar;
}

template <typename T>
[[nodiscard]] constexpr TVector4<T> operator/(TVector4<T> A, const T Scalar)
{
    return A /= Scalar;
}

using FVector4 = TVector4<float32>;
