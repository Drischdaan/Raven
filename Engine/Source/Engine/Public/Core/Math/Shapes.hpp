// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <limits>

#include "Matrix4x4.hpp"

struct FBox
{
    FVector3 Min = {.X = std::numeric_limits<float32>::max(), .Y = std::numeric_limits<float32>::max(), .Z = std::numeric_limits<float32>::max()};
    FVector3 Max = {.X = std::numeric_limits<float32>::lowest(), .Y = std::numeric_limits<float32>::lowest(), .Z = std::numeric_limits<float32>::lowest()};

public:
    [[nodiscard]] constexpr bool8 IsValid() const
    {
        return Min.X <= Max.X && Min.Y <= Max.Y && Min.Z <= Max.Z;
    }

    [[nodiscard]] constexpr FVector3 GetCenter() const
    {
        return (Min + Max) * 0.5f;
    }

    [[nodiscard]] constexpr FVector3 GetExtent() const
    {
        return (Max - Min) * 0.5f;
    }

    [[nodiscard]] constexpr FVector3 GetSize() const
    {
        return Max - Min;
    }

    [[nodiscard]] constexpr bool8 Contains(const FVector3& Point) const
    {
        return Point.X >= Min.X && Point.X <= Max.X && Point.Y >= Min.Y && Point.Y <= Max.Y && Point.Z >= Min.Z && Point.Z <= Max.Z;
    }

    constexpr void Expand(const FVector3& Point)
    {
        Min = FVector3::Min(Min, Point);
        Max = FVector3::Max(Max, Point);
    }

    constexpr void Expand(const FBox& Other)
    {
        Min = FVector3::Min(Min, Other.Min);
        Max = FVector3::Max(Max, Other.Max);
    }

    [[nodiscard]] constexpr FBox GetTransformed(const FMatrix4x4& Matrix) const
    {
        const FVector3 Center = Matrix.TransformPoint(GetCenter());
        const FVector3 Extent = GetExtent();
        FVector3 NewExtent;
        for (uint32 Row = 0; Row < 3; ++Row)
        {
            NewExtent[Row] = Math::Abs(Matrix.At(Row, 0)) * Extent.X + Math::Abs(Matrix.At(Row, 1)) * Extent.Y + Math::Abs(Matrix.At(Row, 2)) * Extent.Z;
        }
        return FromCenterExtent(Center, NewExtent);
    }

public:
    [[nodiscard]] static constexpr FBox Empty()
    {
        return {};
    }

    [[nodiscard]] static constexpr FBox FromCenterExtent(const FVector3& Center, const FVector3& Extent)
    {
        return {Center - Extent, Center + Extent};
    }

    [[nodiscard]] static constexpr bool8 Intersects(const FBox& A, const FBox& B)
    {
        return A.Min.X <= B.Max.X && A.Max.X >= B.Min.X &&
            A.Min.Y <= B.Max.Y && A.Max.Y >= B.Min.Y &&
            A.Min.Z <= B.Max.Z && A.Max.Z >= B.Min.Z;
    }
};

struct FSphere
{
    FVector3 Center = FVector3::Zero();
    float32 Radius = 0.0f;

public:
    [[nodiscard]] constexpr bool8 Contains(const FVector3& Point) const
    {
        return FVector3::DistanceSquared(Center, Point) <= Radius * Radius;
    }

    [[nodiscard]] static constexpr bool8 Intersects(const FSphere& A, const FSphere& B)
    {
        const float32 RadiusSum = A.Radius + B.Radius;
        return FVector3::DistanceSquared(A.Center, B.Center) <= RadiusSum * RadiusSum;
    }
};

struct FPlane
{
    FVector3 Normal = FVector3::Up();
    float32 D = 0.0f;

public:
    [[nodiscard]] static constexpr FPlane FromPointNormal(const FVector3& Point, const FVector3& Normal)
    {
        return {Normal, -FVector3::Dot(Normal, Point)};
    }

    [[nodiscard]] static FPlane FromPoints(const FVector3& A, const FVector3& B, const FVector3& C)
    {
        const FVector3 Normal = FVector3::Cross(C - A, B - A).GetNormalized();
        return FromPointNormal(A, Normal);
    }

    [[nodiscard]] constexpr float32 GetSignedDistance(const FVector3& Point) const
    {
        return FVector3::Dot(Normal, Point) + D;
    }

    [[nodiscard]] FPlane GetNormalized() const
    {
        const float32 NormalLength = Normal.Length();
        assert(NormalLength > Math::SmallNumber);
        return {Normal / NormalLength, D / NormalLength};
    }
};
