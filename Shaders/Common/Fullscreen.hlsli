// Raven Storm Studio @ 2026. All Rights Reserved.

#ifndef RAVEN_FULLSCREEN_HLSLI
#define RAVEN_FULLSCREEN_HLSLI

struct FFullscreenVaryings
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0; // (0,0) top left, (1,1) bottom right
};

// One triangle that covers the screen: Draw(3) without vertex or index buffers
FFullscreenVaryings FullscreenTriangle(uint VertexId)
{
    FFullscreenVaryings Output;
    Output.UV = float2((VertexId << 1) & 2, VertexId & 2);
    Output.Position = float4(Output.UV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return Output;
}

#endif
