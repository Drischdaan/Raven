// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <cassert>

#include "Quat.hpp"
#include "Vector4.hpp"

struct FMatrix4x4
{
    FVector4 Columns[4] = {
        {.X = 1.0f, .Y = 0.0f, .Z = 0.0f, .W = 0.0f},
        {.X = 0.0f, .Y = 1.0f, .Z = 0.0f, .W = 0.0f},
        {.X = 0.0f, .Y = 0.0f, .Z = 1.0f, .W = 0.0f},
        {.X = 0.0f, .Y = 0.0f, .Z = 0.0f, .W = 1.0f},
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

    [[nodiscard]] constexpr FVector3 GetTranslation() const
    {
        return Columns[3].GetXYZ();
    }

    [[nodiscard]] constexpr FMatrix4x4 GetTransposed() const
    {
        FMatrix4x4 Result;
        for (uint32 Row = 0; Row < 4; ++Row)
        {
            for (uint32 Column = 0; Column < 4; ++Column)
            {
                Result.At(Row, Column) = At(Column, Row);
            }
        }
        return Result;
    }

    [[nodiscard]] constexpr FVector3 TransformPoint(const FVector3& Point) const;

    [[nodiscard]] constexpr FVector3 TransformVector(const FVector3& Vector) const;

    [[nodiscard]] FVector3 ProjectPoint(const FVector3& Point) const;

    [[nodiscard]] float32 GetDeterminant() const;

    [[nodiscard]] FMatrix4x4 GetInverse() const;

public:
    [[nodiscard]] static constexpr FMatrix4x4 Identity()
    {
        return {};
    }

    [[nodiscard]] static constexpr FMatrix4x4 MakeTranslation(const FVector3& Translation)
    {
        FMatrix4x4 Result;
        Result.Columns[3] = FVector4::FromVector3(Translation, 1.0f);
        return Result;
    }

    [[nodiscard]] static constexpr FMatrix4x4 MakeScale(const FVector3& Scale)
    {
        FMatrix4x4 Result;
        Result.Columns[0].X = Scale.X;
        Result.Columns[1].Y = Scale.Y;
        Result.Columns[2].Z = Scale.Z;
        return Result;
    }

    [[nodiscard]] static constexpr FMatrix4x4 MakeRotation(const FQuat& Rotation)
    {
        const float32 X2 = Rotation.X * Rotation.X;
        const float32 Y2 = Rotation.Y * Rotation.Y;
        const float32 Z2 = Rotation.Z * Rotation.Z;
        const float32 XY = Rotation.X * Rotation.Y;
        const float32 XZ = Rotation.X * Rotation.Z;
        const float32 YZ = Rotation.Y * Rotation.Z;
        const float32 WX = Rotation.W * Rotation.X;
        const float32 WY = Rotation.W * Rotation.Y;
        const float32 WZ = Rotation.W * Rotation.Z;

        FMatrix4x4 Result;
        Result.Columns[0] = {.X = 1.0f - 2.0f * (Y2 + Z2), .Y = 2.0f * (XY + WZ), .Z = 2.0f * (XZ - WY), .W = 0.0f};
        Result.Columns[1] = {.X = 2.0f * (XY - WZ), .Y = 1.0f - 2.0f * (X2 + Z2), .Z = 2.0f * (YZ + WX), .W = 0.0f};
        Result.Columns[2] = {.X = 2.0f * (XZ + WY), .Y = 2.0f * (YZ - WX), .Z = 1.0f - 2.0f * (X2 + Y2), .W = 0.0f};
        return Result;
    }

    [[nodiscard]] static constexpr FMatrix4x4 MakeTRS(const FVector3& Translation, const FQuat& Rotation, const FVector3& Scale)
    {
        FMatrix4x4 Result = MakeRotation(Rotation);
        Result.Columns[0] *= Scale.X;
        Result.Columns[1] *= Scale.Y;
        Result.Columns[2] *= Scale.Z;
        Result.Columns[3] = FVector4::FromVector3(Translation, 1.0f);
        return Result;
    }

    [[nodiscard]] static FMatrix4x4 MakeLookAt(const FVector3& Eye, const FVector3& Target, const FVector3& Up)
    {
        return MakeLookTo(Eye, Target - Eye, Up);
    }

    [[nodiscard]] static FMatrix4x4 MakeLookTo(const FVector3& Eye, const FVector3& Direction, const FVector3& Up)
    {
        const FVector3 Forward = Direction.GetNormalized();
        const FVector3 Right = FVector3::Cross(Up, Forward).GetNormalized();
        const FVector3 CameraUp = FVector3::Cross(Forward, Right);

        FMatrix4x4 Result;
        Result.Columns[0] = {.X = Right.X, .Y = CameraUp.X, .Z = Forward.X, .W = 0.0f};
        Result.Columns[1] = {.X = Right.Y, .Y = CameraUp.Y, .Z = Forward.Y, .W = 0.0f};
        Result.Columns[2] = {.X = Right.Z, .Y = CameraUp.Z, .Z = Forward.Z, .W = 0.0f};
        Result.Columns[3] = {.X = -FVector3::Dot(Right, Eye), .Y = -FVector3::Dot(CameraUp, Eye), .Z = -FVector3::Dot(Forward, Eye), .W = 1.0f};
        return Result;
    }

    [[nodiscard]] static FMatrix4x4 MakePerspectiveReverseZ(const float32 FovY, const float32 AspectRatio, const float32 Near)
    {
        assert(Near > 0.0f && AspectRatio > 0.0f);
        const float32 YScale = 1.0f / Math::Tan(0.5f * FovY);
        const float32 XScale = YScale / AspectRatio;

        FMatrix4x4 Result;
        Result.Columns[0] = {.X = XScale, .Y = 0.0f, .Z = 0.0f, .W = 0.0f};
        Result.Columns[1] = {.X = 0.0f, .Y = YScale, .Z = 0.0f, .W = 0.0f};
        Result.Columns[2] = {.X = 0.0f, .Y = 0.0f, .Z = 0.0f, .W = 1.0f}; // ClipW = ViewZ
        Result.Columns[3] = {.X = 0.0f, .Y = 0.0f, .Z = Near, .W = 0.0f}; // ClipZ = Near
        return Result;
    }

    [[nodiscard]] static FMatrix4x4 MakeOrthographicReverseZ(const float32 Width, const float32 Height, const float32 Near, const float32 Far)
    {
        assert(Width > 0.0f && Height > 0.0f && Far > Near);
        const float32 DepthRange = Far - Near;

        FMatrix4x4 Result;
        Result.Columns[0] = {.X = 2.0f / Width, .Y = 0.0f, .Z = 0.0f, .W = 0.0f};
        Result.Columns[1] = {.X = 0.0f, .Y = 2.0f / Height, .Z = 0.0f, .W = 0.0f};
        Result.Columns[2] = {.X = 0.0f, .Y = 0.0f, .Z = -1.0f / DepthRange, .W = 0.0f};
        Result.Columns[3] = {.X = 0.0f, .Y = 0.0f, .Z = Far / DepthRange, .W = 1.0f};
        return Result;
    }

    [[nodiscard]] static bool8 IsNearlyEqual(const FMatrix4x4& A, const FMatrix4x4& B, const float32 Tolerance = Math::KindaSmallNumber)
    {
        for (uint32 Column = 0; Column < 4; ++Column)
        {
            for (uint32 Row = 0; Row < 4; ++Row)
            {
                if (!Math::IsNearlyEqual(A.At(Row, Column), B.At(Row, Column), Tolerance))
                {
                    return false;
                }
            }
        }
        return true;
    }
};

static_assert(sizeof(FMatrix4x4) == 64);

[[nodiscard]] constexpr FVector4 operator*(const FMatrix4x4& Matrix, const FVector4& Vector)
{
    return Matrix.Columns[0] * Vector.X + Matrix.Columns[1] * Vector.Y + Matrix.Columns[2] * Vector.Z + Matrix.Columns[3] * Vector.W;
}

[[nodiscard]] constexpr FMatrix4x4 operator*(const FMatrix4x4& A, const FMatrix4x4& B)
{
    FMatrix4x4 Result;
    for (uint32 Column = 0; Column < 4; ++Column)
    {
        Result.Columns[Column] = A * B.Columns[Column];
    }
    return Result;
}

constexpr FVector3 FMatrix4x4::TransformPoint(const FVector3& Point) const
{
    return (*this * FVector4::FromVector3(Point, 1.0f)).GetXYZ();
}

constexpr FVector3 FMatrix4x4::TransformVector(const FVector3& Vector) const
{
    return (*this * FVector4::FromVector3(Vector, 0.0f)).GetXYZ();
}

inline FVector3 FMatrix4x4::ProjectPoint(const FVector3& Point) const
{
    const FVector4 Clip = *this * FVector4::FromVector3(Point, 1.0f);
    assert(!Math::IsNearlyZero(Clip.W, Math::SmallNumber) && "Projecting a point on the camera plane");
    return Clip.GetXYZ() / Clip.W;
}

namespace MathDetail
{
    inline void ComputeCofactors(const float32 (&M)[16], float32 (&Out)[16])
    {
        Out[0] = M[5] * M[10] * M[15] - M[5] * M[11] * M[14] - M[9] * M[6] * M[15] + M[9] * M[7] * M[14] + M[13] * M[6] * M[11] - M[13] * M[7] * M[10];
        Out[4] = -M[4] * M[10] * M[15] + M[4] * M[11] * M[14] + M[8] * M[6] * M[15] - M[8] * M[7] * M[14] - M[12] * M[6] * M[11] + M[12] * M[7] * M[10];
        Out[8] = M[4] * M[9] * M[15] - M[4] * M[11] * M[13] - M[8] * M[5] * M[15] + M[8] * M[7] * M[13] + M[12] * M[5] * M[11] - M[12] * M[7] * M[9];
        Out[12] = -M[4] * M[9] * M[14] + M[4] * M[10] * M[13] + M[8] * M[5] * M[14] - M[8] * M[6] * M[13] - M[12] * M[5] * M[10] + M[12] * M[6] * M[9];
        Out[1] = -M[1] * M[10] * M[15] + M[1] * M[11] * M[14] + M[9] * M[2] * M[15] - M[9] * M[3] * M[14] - M[13] * M[2] * M[11] + M[13] * M[3] * M[10];
        Out[5] = M[0] * M[10] * M[15] - M[0] * M[11] * M[14] - M[8] * M[2] * M[15] + M[8] * M[3] * M[14] + M[12] * M[2] * M[11] - M[12] * M[3] * M[10];
        Out[9] = -M[0] * M[9] * M[15] + M[0] * M[11] * M[13] + M[8] * M[1] * M[15] - M[8] * M[3] * M[13] - M[12] * M[1] * M[11] + M[12] * M[3] * M[9];
        Out[13] = M[0] * M[9] * M[14] - M[0] * M[10] * M[13] - M[8] * M[1] * M[14] + M[8] * M[2] * M[13] + M[12] * M[1] * M[10] - M[12] * M[2] * M[9];
        Out[2] = M[1] * M[6] * M[15] - M[1] * M[7] * M[14] - M[5] * M[2] * M[15] + M[5] * M[3] * M[14] + M[13] * M[2] * M[7] - M[13] * M[3] * M[6];
        Out[6] = -M[0] * M[6] * M[15] + M[0] * M[7] * M[14] + M[4] * M[2] * M[15] - M[4] * M[3] * M[14] - M[12] * M[2] * M[7] + M[12] * M[3] * M[6];
        Out[10] = M[0] * M[5] * M[15] - M[0] * M[7] * M[13] - M[4] * M[1] * M[15] + M[4] * M[3] * M[13] + M[12] * M[1] * M[7] - M[12] * M[3] * M[5];
        Out[14] = -M[0] * M[5] * M[14] + M[0] * M[6] * M[13] + M[4] * M[1] * M[14] - M[4] * M[2] * M[13] - M[12] * M[1] * M[6] + M[12] * M[2] * M[5];
        Out[3] = -M[1] * M[6] * M[11] + M[1] * M[7] * M[10] + M[5] * M[2] * M[11] - M[5] * M[3] * M[10] - M[9] * M[2] * M[7] + M[9] * M[3] * M[6];
        Out[7] = M[0] * M[6] * M[11] - M[0] * M[7] * M[10] - M[4] * M[2] * M[11] + M[4] * M[3] * M[10] + M[8] * M[2] * M[7] - M[8] * M[3] * M[6];
        Out[11] = -M[0] * M[5] * M[11] + M[0] * M[7] * M[9] + M[4] * M[1] * M[11] - M[4] * M[3] * M[9] - M[8] * M[1] * M[7] + M[8] * M[3] * M[5];
        Out[15] = M[0] * M[5] * M[10] - M[0] * M[6] * M[9] - M[4] * M[1] * M[10] + M[4] * M[2] * M[9] + M[8] * M[1] * M[6] - M[8] * M[2] * M[5];
    }

    inline void ToArray(const FMatrix4x4& Matrix, float32 (&Out)[16])
    {
        for (uint32 Column = 0; Column < 4; ++Column)
        {
            for (uint32 Row = 0; Row < 4; ++Row)
            {
                Out[Column * 4 + Row] = Matrix.At(Row, Column);
            }
        }
    }
}

inline float32 FMatrix4x4::GetDeterminant() const
{
    float32 M[16];
    float32 Cofactors[16];
    MathDetail::ToArray(*this, M);
    MathDetail::ComputeCofactors(M, Cofactors);
    return M[0] * Cofactors[0] + M[1] * Cofactors[4] + M[2] * Cofactors[8] + M[3] * Cofactors[12];
}

inline FMatrix4x4 FMatrix4x4::GetInverse() const
{
    float32 M[16];
    float32 Cofactors[16];
    MathDetail::ToArray(*this, M);
    MathDetail::ComputeCofactors(M, Cofactors);

    const float32 Determinant = M[0] * Cofactors[0] + M[1] * Cofactors[4] + M[2] * Cofactors[8] + M[3] * Cofactors[12];
    if (Math::IsNearlyZero(Determinant, Math::SmallNumber))
    {
        assert(false && "Inverting a singular matrix");
        return Identity();
    }

    const float32 InverseDeterminant = 1.0f / Determinant;
    FMatrix4x4 Result;
    for (uint32 Column = 0; Column < 4; ++Column)
    {
        for (uint32 Row = 0; Row < 4; ++Row)
        {
            Result.At(Row, Column) = Cofactors[Column * 4 + Row] * InverseDeterminant;
        }
    }
    return Result;
}
