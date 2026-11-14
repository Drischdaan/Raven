// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Matrix4x4.hpp"

struct FTransform
{
    FVector3 Translation = FVector3::Zero();
    FQuat Rotation = FQuat::Identity();
    FVector3 Scale = FVector3::One();

public:
    [[nodiscard]] constexpr FMatrix4x4 ToMatrix() const
    {
        return FMatrix4x4::MakeTRS(Translation, Rotation, Scale);
    }

    [[nodiscard]] constexpr FVector3 TransformPoint(const FVector3& Point) const
    {
        return Rotation.RotateVector(Point * Scale) + Translation;
    }

    [[nodiscard]] constexpr FVector3 TransformVector(const FVector3& Vector) const
    {
        return Rotation.RotateVector(Vector * Scale);
    }

    [[nodiscard]] FVector3 InverseTransformPoint(const FVector3& Point) const
    {
        return Rotation.UnrotateVector(Point - Translation) / Scale;
    }

    [[nodiscard]] FTransform GetInverse() const
    {
        const FQuat InverseRotation = Rotation.GetConjugate();
        const FVector3 InverseScale = FVector3::One() / Scale;
        return {
            .Translation = InverseRotation.RotateVector(-Translation) * InverseScale,
            .Rotation = InverseRotation,
            .Scale = InverseScale,
        };
    }

    [[nodiscard]] constexpr FVector3 GetRight() const { return Rotation.GetRight(); }
    [[nodiscard]] constexpr FVector3 GetUp() const { return Rotation.GetUp(); }
    [[nodiscard]] constexpr FVector3 GetForward() const { return Rotation.GetForward(); }

public:
    [[nodiscard]] static constexpr FTransform Identity()
    {
        return {};
    }

    [[nodiscard]] static constexpr FTransform Combine(const FTransform& Parent, const FTransform& Child)
    {
        return {
            .Translation = Parent.TransformPoint(Child.Translation),
            .Rotation = Parent.Rotation * Child.Rotation,
            .Scale = Parent.Scale * Child.Scale,
        };
    }

    [[nodiscard]] static FTransform Lerp(const FTransform& A, const FTransform& B, const float32 Alpha)
    {
        return {
            .Translation = Math::Lerp(A.Translation, B.Translation, Alpha),
            .Rotation = FQuat::Slerp(A.Rotation, B.Rotation, Alpha),
            .Scale = Math::Lerp(A.Scale, B.Scale, Alpha),
        };
    }
};
