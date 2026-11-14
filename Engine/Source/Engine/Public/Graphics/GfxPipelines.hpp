// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#pragma once

#include "GfxResources.hpp"
#include "GfxTypes.hpp"
#include "Core/Memory/Containers/Array.hpp"
#include "Core/Memory/Containers/Span.hpp"
#include "Core/Memory/Containers/String.hpp"

enum class EGfxShaderStage : uint8
{
    Vertex,
    Pixel,
    Compute,
    Amplification,
    Mesh,
};

enum class EGfxShaderFormat : uint8
{
    DXIL,
    SPIRV,
};

struct FGfxShaderDesc
{
    EGfxShaderStage Stage = EGfxShaderStage::Vertex;
    EGfxShaderFormat Format = EGfxShaderFormat::DXIL;
    TSpan<const byte8> Bytecode;
    FStringView EntryPoint = "main";
    FStringView DebugName = "UnnamedShader";
};

enum class EGfxPrimitiveTopology : uint8
{
    PointList,
    LineList,
    LineStrip,
    TriangleList,
    TriangleStrip,
};

enum class EGfxFillMode : uint8
{
    Solid,
    Wireframe,
};

enum class EGfxCullMode : uint8
{
    None,
    Front,
    Back,
};

enum class EGfxFrontFace : uint8
{
    CounterClockwise,
    Clockwise,
};

struct FGfxRasterState
{
    EGfxFillMode FillMode = EGfxFillMode::Solid;
    EGfxCullMode CullMode = EGfxCullMode::Back;
    EGfxFrontFace FrontFace = EGfxFrontFace::CounterClockwise;
    int32 DepthBias = 0;
    float32 DepthBiasClamp = 0.0f;
    float32 SlopeScaledDepthBias = 0.0f;
    bool8 bDepthClipEnable = true;
    bool8 bConservativeRasterization = false;
};

enum class EGfxStencilOp : uint8
{
    Keep,
    Zero,
    Replace,
    IncrementClamp,
    DecrementClamp,
    Invert,
    IncrementWrap,
    DecrementWrap,
};

struct FGfxStencilFaceState
{
    EGfxStencilOp FailOp = EGfxStencilOp::Keep;
    EGfxStencilOp DepthFailOp = EGfxStencilOp::Keep;
    EGfxStencilOp PassOp = EGfxStencilOp::Keep;
    EGfxCompareOp CompareOp = EGfxCompareOp::Always;
};

struct FGfxDepthStencilState
{
    bool8 bDepthTestEnable = true;
    bool8 bDepthWriteEnable = true;
    EGfxCompareOp DepthCompareOp = EGfxCompareOp::GreaterEqual;
    bool8 bStencilEnable = false;
    uint8 StencilReadMask = 0xFF;
    uint8 StencilWriteMask = 0xFF;
    FGfxStencilFaceState FrontFace;
    FGfxStencilFaceState BackFace;
};

enum class EGfxBlendFactor : uint8
{
    Zero,
    One,
    SourceColor,
    InverseSourceColor,
    SourceAlpha,
    InverseSourceAlpha,
    DestinationColor,
    InverseDestinationColor,
    DestinationAlpha,
    InverseDestinationAlpha,
    SourceAlphaSaturate,
    BlendConstant,
    InverseBlendConstant,
};

enum class EGfxBlendOp : uint8
{
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
};

enum class EGfxColorWriteMask : uint8
{
    None = 0,
    Red = 1 << 0,
    Green = 1 << 1,
    Blue = 1 << 2,
    Alpha = 1 << 3,
    All = Red | Green | Blue | Alpha,
};

RAVEN_ENUM_FLAGS(EGfxColorWriteMask)

struct FGfxRenderTargetBlendState
{
    bool8 bBlendEnable = false;
    EGfxBlendFactor SourceColor = EGfxBlendFactor::One;
    EGfxBlendFactor DestinationColor = EGfxBlendFactor::Zero;
    EGfxBlendOp ColorOp = EGfxBlendOp::Add;
    EGfxBlendFactor SourceAlpha = EGfxBlendFactor::One;
    EGfxBlendFactor DestinationAlpha = EGfxBlendFactor::Zero;
    EGfxBlendOp AlphaOp = EGfxBlendOp::Add;
    EGfxColorWriteMask WriteMask = EGfxColorWriteMask::All;
};

struct FGfxBlendState
{
    bool8 bAlphaToCoverageEnable = false;
    TArray<FGfxRenderTargetBlendState, GfxMaxRenderTargets> RenderTargets = {};
};

struct FGfxGraphicsPipelineDesc
{
    FGfxShaderHandle VertexShader;
    FGfxShaderHandle AmplificationShader;
    FGfxShaderHandle MeshShader;
    FGfxShaderHandle PixelShader;

    FGfxRasterState RasterState;
    FGfxDepthStencilState DepthStencilState;
    FGfxBlendState BlendState;
    EGfxPrimitiveTopology Topology = EGfxPrimitiveTopology::TriangleList;

    TArray<EGfxFormat, GfxMaxRenderTargets> RenderTargetFormats = {};
    uint32 RenderTargetCount = 0;
    EGfxFormat DepthStencilFormat = EGfxFormat::Unknown;
    uint32 SampleCount = 1;
};

struct FGfxComputePipelineDesc
{
    FGfxShaderHandle ComputeShader;
};
