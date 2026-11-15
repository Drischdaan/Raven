// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Renderer/RendererTypes.hpp"
#include "Renderer/RenderView.hpp"
#include "Core/Memory/Containers/String.hpp"
#include "Core/Memory/Containers/Vector.hpp"
#include "Core/Utility/Result.hpp"

class FRenderer;
struct FTextureBuilder;
struct FMaterialDesc;

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

    TResult<FTextureHandle> AddTexture(FTextureBuilder Builder, FStringView Name);
    TResult<FMaterialHandle> AddMaterial(const FMaterialDesc& Desc, FStringView Name);

    FRenderer* Renderer = nullptr;
    FMeshHandle CubeMesh;
    FMeshHandle SphereMesh;
    FMeshHandle PlaneMesh;
    TVector<FTextureHandle> Textures;
    TVector<FMaterialHandle> Materials;
    FInstanceId Ground;
    TVector<FSpinner> Spinners;
};
