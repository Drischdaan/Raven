// Raven Storm Studio @ 2026. All Rights Reserved.

#ifndef RAVEN_SCENE_DATA_HLSLI
#define RAVEN_SCENE_DATA_HLSLI

#include "Common/Bindless.hlsli"
#include "Common/Packing.hlsli"

struct FGpuVertexAttributes
{
    uint Normal;
    uint Tangent;
    float2 TexCoord;
};

struct FGpuMesh
{
    uint VertexOffset;
    uint VertexCount;
    uint IndexOffset;
    uint IndexCount;
    float4 BoundsCenterRadius;
    float4 BoundsExtent;
};

struct FGpuInstance
{
    float4x4 LocalToWorld;
    float4x4 PrevLocalToWorld;
    float4 NormalMatrix[3];
    uint MeshIndex;
    uint MaterialIndex;
    uint Flags;
    uint DebugColor;
};

struct FGpuFrameConstants
{
    uint FrameIndex;
    float Time;
    float DeltaTime;
    uint Padding0;

    uint InstanceBufferIndex;
    uint MeshBufferIndex;
    uint PositionBufferIndex;
    uint AttributeBufferIndex;
};

struct FGpuViewConstants
{
    float4x4 WorldToView;
    float4x4 ViewToClip;
    float4x4 WorldToClip;
    float4x4 ClipToWorld;
    float4x4 PrevWorldToClip;
    float4 CameraPosition;
    float2 ViewportSize;
    float2 InvViewportSize;
    float NearPlane;
    float3 Padding;
};

RAVEN_FRAME_CONSTANTS(FGpuFrameConstants, Frame);
RAVEN_VIEW_CONSTANTS(FGpuViewConstants, View);

FGpuInstance LoadInstance(uint InstanceIndex)
{
    StructuredBuffer<FGpuInstance> Instances = ResourceDescriptorHeap[Frame.InstanceBufferIndex];
    return Instances[InstanceIndex];
}

FGpuMesh LoadMesh(uint MeshIndex)
{
    StructuredBuffer<FGpuMesh> Meshes = ResourceDescriptorHeap[Frame.MeshBufferIndex];
    return Meshes[MeshIndex];
}

float3 LoadPosition(uint VertexIndex)
{
    StructuredBuffer<float3> Positions = ResourceDescriptorHeap[Frame.PositionBufferIndex];
    return Positions[VertexIndex];
}

FGpuVertexAttributes LoadAttributes(uint VertexIndex)
{
    StructuredBuffer<FGpuVertexAttributes> Attributes = ResourceDescriptorHeap[Frame.AttributeBufferIndex];
    return Attributes[VertexIndex];
}

float3 TransformNormal(FGpuInstance Instance, float3 Normal)
{
    return normalize(Instance.NormalMatrix[0].xyz * Normal.x + Instance.NormalMatrix[1].xyz * Normal.y + Instance.NormalMatrix[2].xyz * Normal.z);
}

#endif