// Raven Storm Studio @ 2026. All Rights Reserved.

#ifndef RAVEN_BINDLESS_HLSLI
#define RAVEN_BINDLESS_HLSLI

// Global root signature (FD3D12GfxDevice):
//   b0 space0  push constants, up to 128 bytes (32 x uint)
//   b1 space0  frame constants   (EGfxConstantSlot::Frame)
//   b2 space0  view constants    (EGfxConstantSlot::View)
//   b3 space0  pass constants    (EGfxConstantSlot::Pass)
// Everything else is bindless (SM 6.6): ResourceDescriptorHeap[Index], SamplerDescriptorHeap[Index]
//
// Usage:
//   Texture2D<float4> Albedo = ResourceDescriptorHeap[Push.AlbedoIndex];
//   SamplerState Sampler = SamplerDescriptorHeap[Push.SamplerIndex];
//   Wrap an index in NonUniformResourceIndex() when it can differ between lanes of a wave (per-pixel material ids).

#define RAVEN_PUSH_CONSTANTS(Type, Name) ConstantBuffer<Type> Name : register(b0, space0)
#define RAVEN_FRAME_CONSTANTS(Type, Name) ConstantBuffer<Type> Name : register(b1, space0)
#define RAVEN_VIEW_CONSTANTS(Type, Name) ConstantBuffer<Type> Name : register(b2, space0)
#define RAVEN_PASS_CONSTANTS(Type, Name) ConstantBuffer<Type> Name : register(b3, space0)

static const uint InvalidBindlessIndex = 0xFFFFFFFF;

#endif
