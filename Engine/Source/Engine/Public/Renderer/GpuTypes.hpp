// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Core/CoreTypes.hpp"
#include "Core/Math/Matrix4x4.hpp"
#include "Core/Math/Vector4.hpp"

using FGpuVertexPosition = FVector3;

struct FGpuVertexAttributes
{
    uint32 Normal = 0;
    uint32 Tangent = 0;
    float32 TexCoord[2] = {};
};

static_assert(sizeof(FGpuVertexAttributes) == 16);

struct alignas(16) FGpuMesh
{
    uint32 VertexOffset = 0;
    uint32 VertexCount = 0;
    uint32 IndexOffset = 0;
    uint32 IndexCount = 0;
    FVector4 BoundsCenterRadius;
    FVector4 BoundsExtent;
};

static_assert(sizeof(FGpuMesh) == 48);

struct alignas(16) FGpuInstance
{
    FMatrix4x4 LocalToWorld;
    FMatrix4x4 PrevLocalToWorld;
    FVector4 NormalMatrix[3];
    uint32 MeshIndex = UINT32_MAX;
    uint32 MaterialIndex = UINT32_MAX;
    uint32 Flags = 0;
    uint32 DebugColor = 0xFFFFFFFF;
};

static_assert(sizeof(FGpuInstance) == 192);

struct alignas(16) FGpuFrameConstants
{
    uint32 FrameIndex = 0;
    float32 Time = 0.0f;
    float32 DeltaTime = 0.0f;
    uint32 Padding0 = 0;

    uint32 InstanceBufferIndex = UINT32_MAX;
    uint32 MeshBufferIndex = UINT32_MAX;
    uint32 PositionBufferIndex = UINT32_MAX;
    uint32 AttributeBufferIndex = UINT32_MAX;
};

static_assert(sizeof(FGpuFrameConstants) == 32);

struct alignas(16) FGpuViewConstants
{
    FMatrix4x4 WorldToView;
    FMatrix4x4 ViewToClip;
    FMatrix4x4 WorldToClip;
    FMatrix4x4 ClipToWorld;
    FMatrix4x4 PrevWorldToClip;
    FVector4 CameraPosition;
    float32 ViewportSize[2] = {};
    float32 InvViewportSize[2] = {};
    float32 NearPlane = 0.1f;
    float32 Padding[3] = {};
};

static_assert(sizeof(FGpuViewConstants) == 368);
