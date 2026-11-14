// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Renderer/RendererTypes.hpp"
#include "Renderer/RenderView.hpp"
#include "Core/Memory/Containers/Vector.hpp"
#include "Core/Utility/Result.hpp"

class FRenderer;

class FDemoScene
{
public:
    FResult Initialize(FRenderer& InRenderer);
    void Deinitialize();

    [[nodiscard]] FRenderView Update(float32 Time);

private:
    struct FSpinner
    {
        FInstanceId Instance;
        FVector3 Position;
        float32 Speed = 1.0f;
    };

    FRenderer* Renderer = nullptr;
    FMeshHandle CubeMesh;
    FMeshHandle SphereMesh;
    FMeshHandle PlaneMesh;
    FInstanceId Ground;
    TVector<FSpinner> Spinners;
};
