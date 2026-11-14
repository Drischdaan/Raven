// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <limits>

#include "Matrix4x4.hpp"
#include "Shapes.hpp"

enum class EFrustumPlane : uint8
{
    Left,
    Right,
    Bottom,
    Top,
    Near,
    Far,
    Count,
};

struct FFrustum
{
    FPlane Planes[static_cast<uint8>(EFrustumPlane::Count)] = {};

public:
    [[nodiscard]] constexpr const FPlane& GetPlane(const EFrustumPlane Plane) const
    {
        return Planes[static_cast<uint8>(Plane)];
    }

    [[nodiscard]] constexpr bool8 Contains(const FVector3& Point) const
    {
        for (const FPlane& Plane : Planes)
        {
            if (Plane.GetSignedDistance(Point) < 0.0f)
            {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr bool8 Intersects(const FSphere& Sphere) const
    {
        for (const FPlane& Plane : Planes)
        {
            if (Plane.GetSignedDistance(Sphere.Center) < -Sphere.Radius)
            {
                return false;
            }
        }
        return true;
    }

    [[nodiscard]] constexpr bool8 Intersects(const FBox& Box) const
    {
        if (!Box.IsValid())
        {
            return false;
        }
        for (const FPlane& Plane : Planes)
        {
            const FVector3 FurthestCorner = {
                Plane.Normal.X >= 0.0f ? Box.Max.X : Box.Min.X,
                Plane.Normal.Y >= 0.0f ? Box.Max.Y : Box.Min.Y,
                Plane.Normal.Z >= 0.0f ? Box.Max.Z : Box.Min.Z,
            };
            if (Plane.GetSignedDistance(FurthestCorner) < 0.0f)
            {
                return false;
            }
        }
        return true;
    }

public:
    [[nodiscard]] static FFrustum FromViewProjection(const FMatrix4x4& ViewProjection);
};

inline FFrustum FFrustum::FromViewProjection(const FMatrix4x4& ViewProjection)
{
    const auto GetRow = [&ViewProjection](const uint32 Row)
    {
        return FVector4{ViewProjection.At(Row, 0), ViewProjection.At(Row, 1), ViewProjection.At(Row, 2), ViewProjection.At(Row, 3)};
    };
    const FVector4 Row0 = GetRow(0);
    const FVector4 Row1 = GetRow(1);
    const FVector4 Row2 = GetRow(2);
    const FVector4 Row3 = GetRow(3);

    const FVector4 Coefficients[static_cast<uint8>(EFrustumPlane::Count)] = {
        Row3 + Row0,
        Row3 - Row0,
        Row3 + Row1,
        Row3 - Row1,
        Row3 - Row2,
        Row2,
    };

    FFrustum Frustum;
    for (uint32 Index = 0; Index < static_cast<uint32>(EFrustumPlane::Count); ++Index)
    {
        const FVector3 Normal = Coefficients[Index].GetXYZ();
        const float32 NormalLength = Normal.Length();
        if (NormalLength <= Math::SmallNumber)
        {
            Frustum.Planes[Index] = {FVector3::Zero(), std::numeric_limits<float32>::max()};
            continue;
        }
        Frustum.Planes[Index] = {Normal / NormalLength, Coefficients[Index].W / NormalLength};
    }
    return Frustum;
}
