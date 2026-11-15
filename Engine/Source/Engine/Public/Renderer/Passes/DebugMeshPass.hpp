// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Graphics/GfxTypes.hpp"
#include "Core/Utility/Result.hpp"

class IGfxCommandList;
class FPipelineCache;
class FMeshPool;
class FMaterialSystem;
class FRenderScene;

struct FDebugMeshPassInputs
{
    FGfxBufferRange FrameConstants;
    FGfxBufferRange ViewConstants;
};

class FDebugMeshPass
{
public:
    FResult Initialize(FPipelineCache& PipelineCache, EGfxFormat ColorFormat, EGfxFormat DepthFormat);

    void Render(IGfxCommandList& CommandList, const FDebugMeshPassInputs& Inputs, const FRenderScene& Scene,
                const FMeshPool& MeshPool, const FMaterialSystem& Materials) const;

private:
    FGfxPipelineHandle Pipeline;
    FGfxPipelineHandle TwoSidedPipeline;
};
