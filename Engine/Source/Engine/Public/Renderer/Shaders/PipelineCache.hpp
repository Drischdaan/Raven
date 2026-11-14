// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "ShaderLibrary.hpp"
#include "ShaderTypes.hpp"
#include "Graphics/GfxPipelines.hpp"
#include "Core/Memory/Containers/Map.hpp"
#include "Core/Utility/Result.hpp"

class IGfxDevice;

struct FGraphicsPipelineRequest
{
    FShaderRequest VertexShader;
    FShaderRequest PixelShader;

    FGfxRasterState RasterState;
    FGfxDepthStencilState DepthStencilState;
    FGfxBlendState BlendState;
    EGfxPrimitiveTopology Topology = EGfxPrimitiveTopology::TriangleList;

    TArray<EGfxFormat, GfxMaxRenderTargets> RenderTargetFormats = {};
    uint32 RenderTargetCount = 0;
    EGfxFormat DepthStencilFormat = EGfxFormat::Unknown;
};

struct FComputePipelineRequest
{
    FShaderRequest ComputeShader;
};

class RAVEN_API FPipelineCache
{
public:
    FPipelineCache() = default;
    FPipelineCache(const FPipelineCache&) = delete;
    FPipelineCache& operator=(const FPipelineCache&) = delete;
    ~FPipelineCache();

public:
    void Initialize(IGfxDevice& InDevice, FShaderLibrary& InShaderLibrary);
    void Deinitialize();

    TResult<FGfxPipelineHandle> GetGraphicsPipeline(FStringView Name, const FGraphicsPipelineRequest& Request);
    TResult<FGfxPipelineHandle> GetComputePipeline(FStringView Name, const FComputePipelineRequest& Request);

private:
    IGfxDevice* Device = nullptr;
    FShaderLibrary* ShaderLibrary = nullptr;
    TUnorderedMap<FString, FGfxPipelineHandle, FStringHash, std::equal_to<>> Pipelines;
};
