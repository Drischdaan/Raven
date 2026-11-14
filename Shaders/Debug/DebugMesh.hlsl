// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Common/Bindless.hlsli"
#include "Common/SceneData.hlsli"

struct FDebugMeshConstants
{
    uint InstanceIndex;
};

RAVEN_PUSH_CONSTANTS(FDebugMeshConstants, Push);

struct FVaryings
{
    float4 Position : SV_Position;
    float3 WorldNormal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    nointerpolation float4 Color : COLOR0;
};

FVaryings VSMain(uint VertexId : SV_VertexID)
{
    const FGpuInstance Instance = LoadInstance(Push.InstanceIndex);
    const FGpuMesh Mesh = LoadMesh(Instance.MeshIndex);
    const uint VertexIndex = Mesh.VertexOffset + VertexId;

    const float3 LocalPosition = LoadPosition(VertexIndex);
    const FGpuVertexAttributes Attributes = LoadAttributes(VertexIndex);
    const float4 WorldPosition = mul(Instance.LocalToWorld, float4(LocalPosition, 1.0f));

    FVaryings Output;
    Output.Position = mul(View.WorldToClip, WorldPosition);
    Output.WorldNormal = TransformNormal(Instance, UnpackOctahedral(Attributes.Normal));
    Output.TexCoord = Attributes.TexCoord;
    Output.Color = UnpackRgba8(Instance.DebugColor);
    return Output;
}

float4 PSMain(FVaryings Input) : SV_Target0
{
    const float3 Normal = normalize(Input.WorldNormal);
    const float3 SunDirection = normalize(float3(0.4f, 0.8f, -0.3f));
    const float Diffuse = saturate(dot(Normal, SunDirection));
    const float3 Ambient = lerp(float3(0.06f, 0.05f, 0.04f), float3(0.12f, 0.15f, 0.22f), Normal.y * 0.5f + 0.5f);

    const uint2 Cell = uint2(floor(Input.TexCoord * 8.0f));
    const float Checker = ((Cell.x + Cell.y) & 1) != 0 ? 0.8f : 1.0f;

    const float3 Color = Input.Color.rgb * Checker * (Diffuse * 0.9f + Ambient);
    return float4(Color, 1.0f);
}