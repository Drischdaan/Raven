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
    float3 WorldPosition : POSITION1;
    float3 WorldNormal : NORMAL;
    float4 WorldTangent : TANGENT;
    float2 TexCoord : TEXCOORD0;
    nointerpolation uint MaterialIndex : MATERIAL;
};

FVaryings VSMain(uint VertexId : SV_VertexID)
{
    const FGpuInstance Instance = LoadInstance(Push.InstanceIndex);
    const FGpuMesh Mesh = LoadMesh(Instance.MeshIndex);
    const uint VertexIndex = Mesh.VertexOffset + VertexId;

    const float3 LocalPosition = LoadPosition(VertexIndex);
    const FGpuVertexAttributes Attributes = LoadAttributes(VertexIndex);
    const float4 WorldPosition = mul(Instance.LocalToWorld, float4(LocalPosition, 1.0f));
    const float4 LocalTangent = UnpackTangent(Attributes.Tangent);

    FVaryings Output;
    Output.Position = mul(View.WorldToClip, WorldPosition);
    Output.WorldPosition = WorldPosition.xyz;
    Output.WorldNormal = TransformNormal(Instance, UnpackOctahedral(Attributes.Normal));
    Output.WorldTangent = float4(normalize(mul((float3x3)Instance.LocalToWorld, LocalTangent.xyz)), LocalTangent.w);
    Output.TexCoord = Attributes.TexCoord;
    Output.MaterialIndex = Instance.MaterialIndex;
    return Output;
}

float4 PSMain(FVaryings Input, bool bFrontFace : SV_IsFrontFace) : SV_Target0
{
    const FGpuMaterial Material = LoadMaterial(Input.MaterialIndex);
    const SamplerState Sampler = GetLinearWrapSampler();

    Texture2D<float4> BaseColorTexture = ResourceDescriptorHeap[NonUniformResourceIndex(Material.BaseColorTexture)];
    const float4 BaseColor = Material.BaseColor * BaseColorTexture.Sample(Sampler, Input.TexCoord);
    if ((Material.Flags & MaterialFlag_AlphaMasked) != 0 && BaseColor.a < Material.AlphaCutoff)
    {
        discard;
    }

    Texture2D<float4> EmissiveTexture = ResourceDescriptorHeap[NonUniformResourceIndex(Material.EmissiveTexture)];
    const float3 Emissive = Material.Emissive * EmissiveTexture.Sample(Sampler, Input.TexCoord).rgb;
    if ((Material.Flags & MaterialFlag_Unlit) != 0)
    {
        return float4(BaseColor.rgb + Emissive, 1.0f);
    }

    float3 Normal = normalize(Input.WorldNormal);
    float3 Tangent = normalize(Input.WorldTangent.xyz - Normal * dot(Input.WorldTangent.xyz, Normal));
    if (!bFrontFace)
    {
        Normal = -Normal;
        Tangent = -Tangent;
    }
    const float3 Bitangent = cross(Normal, Tangent) * Input.WorldTangent.w;

    Texture2D<float4> NormalTexture = ResourceDescriptorHeap[NonUniformResourceIndex(Material.NormalTexture)];
    float3 TangentNormal = NormalTexture.Sample(Sampler, Input.TexCoord).xyz * 2.0f - 1.0f;
    TangentNormal.xy *= Material.NormalScale;
    Normal = normalize(Tangent * TangentNormal.x + Bitangent * TangentNormal.y + Normal * TangentNormal.z);

    Texture2D<float4> MetallicRoughnessTexture = ResourceDescriptorHeap[NonUniformResourceIndex(Material.MetallicRoughnessTexture)];
    const float4 MetallicRoughness = MetallicRoughnessTexture.Sample(Sampler, Input.TexCoord);
    const float Roughness = saturate(Material.Roughness * MetallicRoughness.g);
    const float Metallic = saturate(Material.Metallic * MetallicRoughness.b);

    const float3 SunDirection = normalize(float3(0.4f, 0.8f, -0.3f));
    const float3 ViewDirection = normalize(View.CameraPosition.xyz - Input.WorldPosition);
    const float3 HalfVector = normalize(SunDirection + ViewDirection);
    const float NdotL = saturate(dot(Normal, SunDirection));
    const float SpecularPower = exp2(lerp(11.0f, 1.0f, Roughness));
    const float3 SpecularColor = lerp(float3(0.04f, 0.04f, 0.04f), BaseColor.rgb, Metallic);
    const float3 DiffuseColor = BaseColor.rgb * (1.0f - Metallic);

    const float3 Ambient = lerp(float3(0.06f, 0.05f, 0.04f), float3(0.12f, 0.15f, 0.22f), Normal.y * 0.5f + 0.5f);
    const float3 Specular = SpecularColor * pow(saturate(dot(Normal, HalfVector)), SpecularPower) * (SpecularPower + 8.0f) / 8.0f;
    const float3 Color = DiffuseColor * (NdotL * 0.9f + Ambient) + Specular * NdotL + Ambient * SpecularColor * 0.5f + Emissive;
    return float4(Color, 1.0f);
}