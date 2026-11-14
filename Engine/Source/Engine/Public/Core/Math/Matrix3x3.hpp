// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <cassert>

#include "Matrix4x4.hpp"

struct FMatrix3x3
{
    FVector3 Columns[3] = {
        {.X = 1.0f, .Y = 0.0f, .Z = 0.0f},
        {.X = 0.0f, .Y = 1.0f, .Z = 0.0f},
        {.X = 0.0f, .Y = 0.0f, .Z = 1.0f},
    };

public:
    [[nodiscard]] constexpr float32& At(const uint32 Row, const uint32 Column)
    {
        return Columns[Column][Row];
    }

    [[nodiscard]] constexpr float32 At(const uint32 Row, const uint32 Column) const
    {
        return Columns[Column][Row];
    }

    [[nodiscard]] constexpr FMatrix4x4 ToMatrix4x4() const
    {
        FMatrix4x4 Result;
        Result.Columns[0] = FVector4::FromVector3(Columns[0], 0.0f);
        Result.Columns[1] = FVector4::FromVector3(Columns[1], 0.0f);
        Result.Columns[2] = FVector4::FromVector3(Columns[2], 0.0f);
        return Result;
    }

    [[nodiscard]] constexpr FMatrix3x3 GetTransposed() const
    {
        FMatrix3x3 Result;
        for (uint32 Row = 0; Row < 3; ++Row)
        {
            for (uint32 Column = 0; Column < 3; ++Column)
            {
                Result.At(Row, Column) = At(Column, Row);
            }
        }
        return Result;
    }

    [[nodiscard]] constexpr float32 GetDeterminant() const
    {
        return At(0, 0) * (At(1, 1) * At(2, 2) - At(1, 2) * At(2, 1))
            - At(0, 1) * (At(1, 0) * At(2, 2) - At(1, 2) * At(2, 0))
            + At(0, 2) * (At(1, 0) * At(2, 1) - At(1, 1) * At(2, 0));
    }

    [[nodiscard]] FMatrix3x3 GetInverse() const
    {
        const float32 Determinant = GetDeterminant();
        if (Math::IsNearlyZero(Determinant, Math::SmallNumber))
        {
            assert(false && "Inverting a singular matrix");
            return Identity();
        }
        const float32 InverseDeterminant = 1.0f / Determinant;

        FMatrix3x3 Result;
        Result.At(0, 0) = (At(1, 1) * At(2, 2) - At(1, 2) * At(2, 1)) * InverseDeterminant;
        Result.At(0, 1) = (At(0, 2) * At(2, 1) - At(0, 1) * At(2, 2)) * InverseDeterminant;
        Result.At(0, 2) = (At(0, 1) * At(1, 2) - At(0, 2) * At(1, 1)) * InverseDeterminant;
        Result.At(1, 0) = (At(1, 2) * At(2, 0) - At(1, 0) * At(2, 2)) * InverseDeterminant;
        Result.At(1, 1) = (At(0, 0) * At(2, 2) - At(0, 2) * At(2, 0)) * InverseDeterminant;
        Result.At(1, 2) = (At(0, 2) * At(1, 0) - At(0, 0) * At(1, 2)) * InverseDeterminant;
        Result.At(2, 0) = (At(1, 0) * At(2, 1) - At(1, 1) * At(2, 0)) * InverseDeterminant;
        Result.At(2, 1) = (At(0, 1) * At(2, 0) - At(0, 0) * At(2, 1)) * InverseDeterminant;
        Result.At(2, 2) = (At(0, 0) * At(1, 1) - At(0, 1) * At(1, 0)) * InverseDeterminant;
        return Result;
    }

    [[nodiscard]] constexpr FVector3 TransformVector(const FVector3& Vector) const
    {
        return Columns[0] * Vector.X + Columns[1] * Vector.Y + Columns[2] * Vector.Z;
    }

public:
    [[nodiscard]] static constexpr FMatrix3x3 Identity()
    {
        return {};
    }

    [[nodiscard]] static constexpr FMatrix3x3 FromMatrix4x4(const FMatrix4x4& Matrix)
    {
        FMatrix3x3 Result;
        Result.Columns[0] = Matrix.Columns[0].GetXYZ();
        Result.Columns[1] = Matrix.Columns[1].GetXYZ();
        Result.Columns[2] = Matrix.Columns[2].GetXYZ();
        return Result;
    }

    [[nodiscard]] static constexpr FMatrix3x3 MakeScale(const FVector3& Scale)
    {
        FMatrix3x3 Result;
        Result.Columns[0].X = Scale.X;
        Result.Columns[1].Y = Scale.Y;
        Result.Columns[2].Z = Scale.Z;
        return Result;
    }

    [[nodiscard]] static constexpr FMatrix3x3 MakeRotation(const FQuat& Rotation)
    {
        return FromMatrix4x4(FMatrix4x4::MakeRotation(Rotation));
    }

    [[nodiscard]] static FMatrix3x3 MakeNormalMatrix(const FMatrix4x4& Model)
    {
        return FromMatrix4x4(Model).GetInverse().GetTransposed();
    }

    [[nodiscard]] static bool8 IsNearlyEqual(const FMatrix3x3& A, const FMatrix3x3& B, const float32 Tolerance = Math::KindaSmallNumber)
    {
        for (uint32 Column = 0; Column < 3; ++Column)
        {
            if (!FVector3::IsNearlyEqual(A.Columns[Column], B.Columns[Column], Tolerance))
            {
                return false;
            }
        }
        return true;
    }
};

static_assert(sizeof(FMatrix3x3) == 36);

[[nodiscard]] constexpr FVector3 operator*(const FMatrix3x3& Matrix, const FVector3& Vector)
{
    return Matrix.TransformVector(Vector);
}

[[nodiscard]] constexpr FMatrix3x3 operator*(const FMatrix3x3& A, const FMatrix3x3& B)
{
    FMatrix3x3 Result;
    for (uint32 Column = 0; Column < 3; ++Column)
    {
        Result.Columns[Column] = A * B.Columns[Column];
    }
    return Result;
}
