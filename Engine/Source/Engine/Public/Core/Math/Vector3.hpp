// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <cassert>

#include "MathUtility.hpp"

template <typename T>
struct TVector3
{
    T X = T(0);
    T Y = T(0);
    T Z = T(0);

public:
    [[nodiscard]] constexpr T LengthSquared() const
    {
        return X * X + Y * Y + Z * Z;
    }

    [[nodiscard]] T Length() const
    {
        return static_cast<T>(std::sqrt(LengthSquared()));
    }

    [[nodiscard]] TVector3 GetNormalized() const
    {
        const T VectorLength = Length();
        assert(VectorLength > T(Math::SmallNumber) && "Normalizing a zero-length vector");
        return {.X = X / VectorLength, .Y = Y / VectorLength, .Z = Z / VectorLength};
    }

    [[nodiscard]] TVector3 GetSafeNormal(const T Tolerance = T(Math::SmallNumber)) const
    {
        const T VectorLength = Length();
        return VectorLength > Tolerance ? TVector3{X / VectorLength, Y / VectorLength, Z / VectorLength} : Zero();
    }

    void Normalize()
    {
        *this = GetNormalized();
    }

    [[nodiscard]] bool8 IsNormalized(const T Tolerance = T(Math::KindaSmallNumber)) const
    {
        return std::abs(LengthSquared() - T(1)) <= Tolerance;
    }

    [[nodiscard]] bool8 IsNearlyZero(const T Tolerance = T(Math::KindaSmallNumber)) const
    {
        return std::abs(X) <= Tolerance && std::abs(Y) <= Tolerance && std::abs(Z) <= Tolerance;
    }

    [[nodiscard]] constexpr TVector3 GetAbs() const
    {
        return {.X = Math::Abs(X), .Y = Math::Abs(Y), .Z = Math::Abs(Z)};
    }

    [[nodiscard]] constexpr T GetMaxComponent() const
    {
        return Math::Max(X, Math::Max(Y, Z));
    }

    [[nodiscard]] constexpr T GetMinComponent() const
    {
        return Math::Min(X, Math::Min(Y, Z));
    }

    [[nodiscard]] constexpr T& operator[](const uint32 Index)
    {
        assert(Index < 3);
        return Index == 0 ? X : (Index == 1 ? Y : Z);
    }

    [[nodiscard]] constexpr const T& operator[](const uint32 Index) const
    {
        assert(Index < 3);
        return Index == 0 ? X : (Index == 1 ? Y : Z);
    }

public:
    [[nodiscard]] static constexpr TVector3 Zero()
    {
        return {.X = T(0), .Y = T(0), .Z = T(0)};
    }

    [[nodiscard]] static constexpr TVector3 One()
    {
        return {.X = T(1), .Y = T(1), .Z = T(1)};
    }

    [[nodiscard]] static constexpr TVector3 UnitX()
    {
        return {.X = T(1), .Y = T(0), .Z = T(0)};
    }

    [[nodiscard]] static constexpr TVector3 UnitY()
    {
        return {.X = T(0), .Y = T(1), .Z = T(0)};
    }

    [[nodiscard]] static constexpr TVector3 UnitZ()
    {
        return {.X = T(0), .Y = T(0), .Z = T(1)};
    }

    [[nodiscard]] static constexpr TVector3 Right()
    {
        return UnitX();
    }

    [[nodiscard]] static constexpr TVector3 Up()
    {
        return UnitY();
    }

    [[nodiscard]] static constexpr TVector3 Forward()
    {
        return UnitZ();
    }

    [[nodiscard]] static constexpr T Dot(const TVector3& A, const TVector3& B)
    {
        return A.X * B.X + A.Y * B.Y + A.Z * B.Z;
    }

    [[nodiscard]] static constexpr TVector3 Cross(const TVector3& A, const TVector3& B)
    {
        return {
            .X = A.Y * B.Z - A.Z * B.Y,
            .Y = A.Z * B.X - A.X * B.Z,
            .Z = A.X * B.Y - A.Y * B.X,
        };
    }

    [[nodiscard]] static T Distance(const TVector3& A, const TVector3& B)
    {
        return (B - A).Length();
    }

    [[nodiscard]] static constexpr T DistanceSquared(const TVector3& A, const TVector3& B)
    {
        return (B - A).LengthSquared();
    }

    [[nodiscard]] static constexpr TVector3 Min(const TVector3& A, const TVector3& B)
    {
        return {.X = Math::Min(A.X, B.X), .Y = Math::Min(A.Y, B.Y), .Z = Math::Min(A.Z, B.Z)};
    }

    [[nodiscard]] static constexpr TVector3 Max(const TVector3& A, const TVector3& B)
    {
        return {.X = Math::Max(A.X, B.X), .Y = Math::Max(A.Y, B.Y), .Z = Math::Max(A.Z, B.Z)};
    }

    [[nodiscard]] static constexpr TVector3 Reflect(const TVector3& Incident, const TVector3& Normal)
    {
        return Incident - Normal * (T(2) * Dot(Incident, Normal));
    }

    [[nodiscard]] static bool8 IsNearlyEqual(const TVector3& A, const TVector3& B, const T Tolerance = T(Math::KindaSmallNumber))
    {
        return (A - B).IsNearlyZero(Tolerance);
    }

public:
    [[nodiscard]] constexpr TVector3 operator-() const
    {
        return {.X = -X, .Y = -Y, .Z = -Z};
    }

    constexpr TVector3& operator+=(const TVector3& Other)
    {
        X += Other.X;
        Y += Other.Y;
        Z += Other.Z;
        return *this;
    }

    constexpr TVector3& operator-=(const TVector3& Other)
    {
        X -= Other.X;
        Y -= Other.Y;
        Z -= Other.Z;
        return *this;
    }

    constexpr TVector3& operator*=(const TVector3& Other)
    {
        X *= Other.X;
        Y *= Other.Y;
        Z *= Other.Z;
        return *this;
    }

    constexpr TVector3& operator*=(const T Scalar)
    {
        X *= Scalar;
        Y *= Scalar;
        Z *= Scalar;
        return *this;
    }

    constexpr TVector3& operator/=(const T Scalar)
    {
        X /= Scalar;
        Y /= Scalar;
        Z /= Scalar;
        return *this;
    }

    [[nodiscard]] constexpr bool8 operator==(const TVector3&) const = default;
};

template <typename T>
[[nodiscard]] constexpr TVector3<T> operator+(TVector3<T> A, const TVector3<T>& B)
{
    return A += B;
}

template <typename T>
[[nodiscard]] constexpr TVector3<T> operator-(TVector3<T> A, const TVector3<T>& B)
{
    return A -= B;
}

template <typename T>
[[nodiscard]] constexpr TVector3<T> operator*(TVector3<T> A, const TVector3<T>& B)
{
    return A *= B;
}

template <typename T>
[[nodiscard]] constexpr TVector3<T> operator*(TVector3<T> A, const T Scalar)
{
    return A *= Scalar;
}

template <typename T>
[[nodiscard]] constexpr TVector3<T> operator*(const T Scalar, TVector3<T> A)
{
    return A *= Scalar;
}

template <typename T>
[[nodiscard]] constexpr TVector3<T> operator/(TVector3<T> A, const T Scalar)
{
    return A /= Scalar;
}

template <typename T>
[[nodiscard]] constexpr TVector3<T> operator/(const TVector3<T>& A, const TVector3<T>& B)
{
    return {A.X / B.X, A.Y / B.Y, A.Z / B.Z};
}

using FVector3 = TVector3<float32>;
using FIntVector3 = TVector3<int32>;
