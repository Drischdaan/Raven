// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <cassert>

#include "Vector3.hpp"

struct FQuat
{
    float32 X = 0.0f;
    float32 Y = 0.0f;
    float32 Z = 0.0f;
    float32 W = 1.0f;

public:
    [[nodiscard]] constexpr float32 LengthSquared() const
    {
        return X * X + Y * Y + Z * Z + W * W;
    }

    [[nodiscard]] float32 Length() const
    {
        return Math::Sqrt(LengthSquared());
    }

    [[nodiscard]] FQuat GetNormalized() const
    {
        const float32 QuatLength = Length();
        assert(QuatLength > Math::SmallNumber && "Normalizing a zero-length quaternion");
        return {.X = X / QuatLength, .Y = Y / QuatLength, .Z = Z / QuatLength, .W = W / QuatLength};
    }

    void Normalize()
    {
        *this = GetNormalized();
    }

    [[nodiscard]] bool8 IsNormalized(const float32 Tolerance = Math::KindaSmallNumber) const
    {
        return std::abs(LengthSquared() - 1.0f) <= Tolerance;
    }

    [[nodiscard]] constexpr FQuat GetConjugate() const
    {
        return {.X = -X, .Y = -Y, .Z = -Z, .W = W};
    }

    [[nodiscard]] FQuat GetInverse() const
    {
        const float32 InverseLengthSquared = 1.0f / LengthSquared();
        const FQuat Conjugate = GetConjugate();
        return {.X = Conjugate.X * InverseLengthSquared, .Y = Conjugate.Y * InverseLengthSquared, .Z = Conjugate.Z * InverseLengthSquared, .W = Conjugate.W * InverseLengthSquared};
    }

    [[nodiscard]] constexpr FVector3 RotateVector(const FVector3& Vector) const
    {
        const FVector3 Axis = {.X = X, .Y = Y, .Z = Z};
        const FVector3 T = FVector3::Cross(Axis, Vector) * 2.0f;
        return Vector + T * W + FVector3::Cross(Axis, T);
    }

    [[nodiscard]] constexpr FVector3 UnrotateVector(const FVector3& Vector) const
    {
        return GetConjugate().RotateVector(Vector);
    }

    [[nodiscard]] constexpr FVector3 GetRight() const { return RotateVector(FVector3::Right()); }
    [[nodiscard]] constexpr FVector3 GetUp() const { return RotateVector(FVector3::Up()); }
    [[nodiscard]] constexpr FVector3 GetForward() const { return RotateVector(FVector3::Forward()); }

public:
    [[nodiscard]] static FQuat FromAxisAngle(const FVector3& Axis, const float32 Angle)
    {
        assert(Axis.IsNormalized() && "FQuat::FromAxisAngle needs a normalized axis");
        const float32 HalfAngle = 0.5f * Angle;
        const float32 SinHalf = Math::Sin(HalfAngle);
        return {.X = Axis.X * SinHalf, .Y = Axis.Y * SinHalf, .Z = Axis.Z * SinHalf, .W = Math::Cos(HalfAngle)};
    }

    [[nodiscard]] static FQuat FromEuler(float32 Pitch, float32 Yaw, float32 Roll);

    [[nodiscard]] static constexpr FQuat Identity()
    {
        return {.X = 0.0f, .Y = 0.0f, .Z = 0.0f, .W = 1.0f};
    }

    [[nodiscard]] static constexpr float32 Dot(const FQuat& A, const FQuat& B)
    {
        return A.X * B.X + A.Y * B.Y + A.Z * B.Z + A.W * B.W;
    }

    [[nodiscard]] static FQuat Slerp(const FQuat& A, FQuat B, float32 Alpha);

    [[nodiscard]] static bool8 IsNearlyEqual(const FQuat& A, const FQuat& B, const float32 Tolerance = Math::KindaSmallNumber)
    {
        return std::abs(Dot(A, B)) >= 1.0f - Tolerance;
    }

    [[nodiscard]] constexpr bool8 operator==(const FQuat&) const = default;
};

[[nodiscard]] constexpr FQuat operator*(const FQuat& A, const FQuat& B)
{
    return {
        .X = A.W * B.X + A.X * B.W + A.Y * B.Z - A.Z * B.Y,
        .Y = A.W * B.Y - A.X * B.Z + A.Y * B.W + A.Z * B.X,
        .Z = A.W * B.Z + A.X * B.Y - A.Y * B.X + A.Z * B.W,
        .W = A.W * B.W - A.X * B.X - A.Y * B.Y - A.Z * B.Z,
    };
}

inline FQuat FQuat::FromEuler(const float32 Pitch, const float32 Yaw, const float32 Roll)
{
    const FQuat PitchRotation = FromAxisAngle(FVector3::UnitX(), Pitch);
    const FQuat YawRotation = FromAxisAngle(FVector3::UnitY(), Yaw);
    const FQuat RollRotation = FromAxisAngle(FVector3::UnitZ(), Roll);
    return YawRotation * PitchRotation * RollRotation;
}

inline FQuat FQuat::Slerp(const FQuat& A, FQuat B, const float32 Alpha)
{
    float32 CosAngle = Dot(A, B);
    if (CosAngle < 0.0f)
    {
        B = {-B.X, -B.Y, -B.Z, -B.W};
        CosAngle = -CosAngle;
    }

    float32 WeightA;
    float32 WeightB;
    if (CosAngle > 0.9995f)
    {
        WeightA = 1.0f - Alpha;
        WeightB = Alpha;
    }
    else
    {
        const float32 Angle = Math::Acos(CosAngle);
        const float32 InverseSin = 1.0f / Math::Sin(Angle);
        WeightA = Math::Sin((1.0f - Alpha) * Angle) * InverseSin;
        WeightB = Math::Sin(Alpha * Angle) * InverseSin;
    }

    const FQuat Result = {
        A.X * WeightA + B.X * WeightB,
        A.Y * WeightA + B.Y * WeightB,
        A.Z * WeightA + B.Z * WeightB,
        A.W * WeightA + B.W * WeightB,
    };
    return Result.GetNormalized();
}
