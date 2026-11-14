// Raven Storm Studio @ 2026. All Rights Reserved.

#ifndef RAVEN_PACKING_HLSLI
#define RAVEN_PACKING_HLSLI

float3 OctahedralDecode(float2 Encoded)
{
    float3 Normal = float3(Encoded, 1.0f - abs(Encoded.x) - abs(Encoded.y));
    const float Fold = saturate(-Normal.z);
    Normal.x += Normal.x >= 0.0f ? -Fold : Fold;
    Normal.y += Normal.y >= 0.0f ? -Fold : Fold;
    return normalize(Normal);
}

float2 UnpackSnorm16x2(uint Packed)
{
    const int X = int(Packed << 16) >> 16;
    const int Y = int(Packed) >> 16;
    return max(float2(X, Y) / 32767.0f, -1.0f);
}

float3 UnpackOctahedral(uint Packed)
{
    return OctahedralDecode(UnpackSnorm16x2(Packed));
}

float4 UnpackTangent(uint Packed)
{
    const float Sign = (Packed & 0x10000u) != 0 ? -1.0f : 1.0f;
    return float4(UnpackOctahedral(Packed & ~0x10000u), Sign);
}

float4 UnpackRgba8(uint Packed)
{
    return float4(Packed & 0xFF, (Packed >> 8) & 0xFF, (Packed >> 16) & 0xFF, Packed >> 24) / 255.0f;
}

#endif