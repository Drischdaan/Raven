// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Passes/DebugMeshPass.hpp"

#include "Graphics/GfxCommandList.hpp"
#include "Renderer/Material/MaterialSystem.hpp"
#include "Renderer/Mesh/MeshPool.hpp"
#include "Renderer/Scene/RenderScene.hpp"
#include "Renderer/Shaders/PipelineCache.hpp"

namespace
{
    struct FDebugMeshConstants
    {
        uint32 InstanceIndex = 0;
    };
}

FResult FDebugMeshPass::Initialize(FPipelineCache& PipelineCache, const EGfxFormat ColorFormat, const EGfxFormat DepthFormat)
{
    FGraphicsPipelineRequest Request = {
        .VertexShader = {.Path = "Debug/DebugMesh.hlsl", .EntryPoint = "VSMain", .Stage = EGfxShaderStage::Vertex},
        .PixelShader = {.Path = "Debug/DebugMesh.hlsl", .EntryPoint = "PSMain", .Stage = EGfxShaderStage::Pixel},
        .RenderTargetFormats = {ColorFormat},
        .RenderTargetCount = 1,
        .DepthStencilFormat = DepthFormat,
    };
    Request.RasterState.CullMode = EGfxCullMode::Back;
    Request.RasterState.FrontFace = EGfxFrontFace::Clockwise;
    Request.DepthStencilState.bDepthTestEnable = true;
    Request.DepthStencilState.bDepthWriteEnable = true;
    Request.DepthStencilState.DepthCompareOp = EGfxCompareOp::GreaterEqual;

    RESULT_CHECK_ASSIGN(PipelineCache.GetGraphicsPipeline("Debug.Mesh", Request), Created);
    Pipeline = Created;

    Request.RasterState.CullMode = EGfxCullMode::None;
    RESULT_CHECK_ASSIGN(PipelineCache.GetGraphicsPipeline("Debug.Mesh.TwoSided", Request), CreatedTwoSided);
    TwoSidedPipeline = CreatedTwoSided;
    return RESULT_OK_EMPTY();
}

void FDebugMeshPass::Render(IGfxCommandList& CommandList, const FDebugMeshPassInputs& Inputs, const FRenderScene& Scene,
                            const FMeshPool& MeshPool, const FMaterialSystem& Materials) const
{
    CommandList.BeginEvent("DebugMeshes");
    CommandList.SetIndexBuffer(MeshPool.GetIndexBuffer(), EGfxIndexType::UINT32);

    FGfxPipelineHandle BoundPipeline = {};
    const TSpan<const FRenderScene::FInstance> Instances = Scene.GetInstances();
    for (uint32 InstanceIndex = 0; InstanceIndex < Instances.size(); ++InstanceIndex)
    {
        const FRenderScene::FInstance& Instance = Instances[InstanceIndex];
        const FMeshPool::FMesh* Mesh = MeshPool.GetMesh(Instance.Mesh);
        if (Mesh == nullptr)
        {
            continue;
        }

        const FMaterialSystem::FMaterial* Material = Materials.GetMaterial(Instance.Material);
        const bool8 bTwoSided = Material != nullptr && HasAnyFlags(Material->Desc.Flags, EMaterialFlags::TwoSided);
        const FGfxPipelineHandle InstancePipeline = bTwoSided ? TwoSidedPipeline : Pipeline;
        if (InstancePipeline != BoundPipeline)
        {
            CommandList.SetPipeline(InstancePipeline);
            CommandList.SetConstantBuffer(EGfxConstantSlot::Frame, Inputs.FrameConstants);
            CommandList.SetConstantBuffer(EGfxConstantSlot::View, Inputs.ViewConstants);
            BoundPipeline = InstancePipeline;
        }

        const FDebugMeshConstants Constants = {.InstanceIndex = InstanceIndex};
        CommandList.SetPushConstants(TSpan<const byte8>(reinterpret_cast<const byte8*>(&Constants), sizeof(Constants)));
        CommandList.DrawIndexed(Mesh->Gpu.IndexCount, 1, Mesh->Gpu.IndexOffset, 0, 0);
    }
    CommandList.EndEvent();
}
