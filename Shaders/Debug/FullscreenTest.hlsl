// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Common/Bindless.hlsli"
#include "Common/Fullscreen.hlsli"

struct FFullscreenTestConstants
{
    float4 TopColor;
    float4 BottomColor;
};

RAVEN_PUSH_CONSTANTS(FFullscreenTestConstants, Push);

FFullscreenVaryings VSMain(uint VertexId : SV_VertexID)
{
    return FullscreenTriangle(VertexId);
}

float4 PSMain(FFullscreenVaryings Input) : SV_Target0
{
    return lerp(Push.TopColor, Push.BottomColor, Input.UV.y);
}
