// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Shaders/PipelineCache.hpp"

#include <ranges>

#include "Graphics/GfxDevice.hpp"
#include "Renderer/RendererLog.hpp"

FPipelineCache::~FPipelineCache()
{
    Deinitialize();
}

void FPipelineCache::Initialize(IGfxDevice& InDevice, FShaderLibrary& InShaderLibrary)
{
    Device = &InDevice;
    ShaderLibrary = &InShaderLibrary;
}

void FPipelineCache::Deinitialize()
{
    if (Device != nullptr)
    {
        for (const auto& Handle : Pipelines | std::views::values)
        {
            Device->DestroyPipeline(Handle);
        }
    }
    Pipelines.clear();
    Device = nullptr;
    ShaderLibrary = nullptr;
}

TResult<FGfxPipelineHandle> FPipelineCache::GetGraphicsPipeline(const FStringView Name, const FGraphicsPipelineRequest& Request)
{
    if (const auto It = Pipelines.find(Name); It != Pipelines.end())
    {
        return It->second;
    }

    RESULT_CHECK_ASSIGN(ShaderLibrary->GetShader(Request.VertexShader), VertexShader);
    FGfxShaderHandle PixelShader = {};
    if (!Request.PixelShader.Path.empty())
    {
        RESULT_CHECK_ASSIGN(ShaderLibrary->GetShader(Request.PixelShader), LoadedPixelShader);
        PixelShader = LoadedPixelShader;
    }

    const FGfxGraphicsPipelineDesc Desc = {
        .VertexShader = VertexShader,
        .PixelShader = PixelShader,
        .RasterState = Request.RasterState,
        .DepthStencilState = Request.DepthStencilState,
        .BlendState = Request.BlendState,
        .Topology = Request.Topology,
        .RenderTargetFormats = Request.RenderTargetFormats,
        .RenderTargetCount = Request.RenderTargetCount,
        .DepthStencilFormat = Request.DepthStencilFormat,
    };
    RESULT_CHECK_ASSIGN(Device->CreateGraphicsPipeline(Desc, Name), Handle);

    Pipelines.emplace(FString(Name), Handle);
    RAVEN_LOG(LogShaders, Verbose, "Created graphics pipeline '{}'", Name);
    return Handle;
}

TResult<FGfxPipelineHandle> FPipelineCache::GetComputePipeline(const FStringView Name, const FComputePipelineRequest& Request)
{
    if (const auto It = Pipelines.find(Name); It != Pipelines.end())
    {
        return It->second;
    }

    RESULT_CHECK_ASSIGN(ShaderLibrary->GetShader(Request.ComputeShader), ComputeShader);
    RESULT_CHECK_ASSIGN(Device->CreateComputePipeline({.ComputeShader = ComputeShader}, Name), Handle);

    Pipelines.emplace(FString(Name), Handle);
    RAVEN_LOG(LogShaders, Verbose, "Created compute pipeline '{}'", Name);
    return Handle;
}
