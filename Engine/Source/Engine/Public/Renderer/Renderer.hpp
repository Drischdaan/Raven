// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "RendererTypes.hpp"
#include "RenderView.hpp"
#include "Material/Material.hpp"
#include "Mesh/MeshData.hpp"
#include "Scene/RenderScene.hpp"
#include "Texture/TextureData.hpp"
#include "Graphics/GfxTypes.hpp"
#include "Core/Memory/SmartPointers.hpp"
#include "Core/Memory/Containers/String.hpp"
#include "Core/Utility/Result.hpp"
#include "Shaders/PipelineCache.hpp"
#include "Shaders/ShaderLibrary.hpp"

class FWindow;
class IGfxDevice;
class IGfxSwapchain;
class IGfxCommandList;
class FUploadManager;
class FMeshPool;
class FTexturePool;
class FMaterialSystem;
class FGpuScene;
class FTextureStateTracker;
class FDebugMeshPass;
struct FGpuViewConstants;

struct FRendererDesc
{
    TSharedPtr<FWindow> Window = nullptr;
    bool8 bEnableDebugLayer = false;
    bool8 bEnableGpuValidation = false;
    bool8 bVSync = true;
    uint32 FramesInFlight = 2;
    bool8 bShaderDebugInfo = false;
    FString ShaderDirectory;
    FString ShaderCacheDirectory;
    uint32 MaxVertices = 1u << 20;
    uint32 MaxIndices = 1u << 22;
};

class RAVEN_API FRenderer
{
public:
    FRenderer();
    FRenderer(const FRenderer&) = delete;
    FRenderer& operator=(const FRenderer&) = delete;
    ~FRenderer();

public:
    FResult Initialize(const FRendererDesc& InDesc);
    void Deinitialize();

    FResult Render(const FRenderView& View);

    TResult<FMeshHandle> CreateMesh(const FMeshData& Data, FStringView Name);
    void DestroyMesh(FMeshHandle Mesh);

    TResult<FTextureHandle> CreateTexture(const FTextureData& Data, FStringView Name);
    void DestroyTexture(FTextureHandle Texture);

    TResult<FMaterialHandle> CreateMaterial(const FMaterialDesc& Desc, FStringView Name);
    FResult UpdateMaterial(FMaterialHandle Material, const FMaterialDesc& Desc);
    void DestroyMaterial(FMaterialHandle Material);
    [[nodiscard]] FMaterialHandle GetDefaultMaterial() const;

    [[nodiscard]] FRenderScene& GetScene()
    {
        return Scene;
    }

    [[nodiscard]] IGfxDevice& GetDevice() const
    {
        return *Device;
    }

    [[nodiscard]] FShaderLibrary& GetShaderLibrary()
    {
        return ShaderLibrary;
    }

    [[nodiscard]] FPipelineCache& GetPipelineCache()
    {
        return PipelineCache;
    }

    [[nodiscard]] uint32 GetWidth() const
    {
        return Width;
    }

    [[nodiscard]] uint32 GetHeight() const
    {
        return Height;
    }

private:
    FResult ResolveDirectories();
    FResult CreatePipelines();
    FResult CreateDepthTexture();
    FResult CreateSamplers();
    FResult HandleResize();
    FResult RenderFrame(const FRenderView& View);
    void RecordFrame(IGfxCommandList& CommandList, FGfxTextureHandle Backbuffer, const FRenderView& View);
    [[nodiscard]] FGpuViewConstants MakeViewConstants(const FRenderView& View) const;

private:
    FRendererDesc Desc;
    TSharedPtr<IGfxDevice> Device = nullptr;
    TUniquePtr<IGfxSwapchain> Swapchain = nullptr;
    FShaderLibrary ShaderLibrary;
    FPipelineCache PipelineCache;
    FRenderScene Scene;

    TUniquePtr<FUploadManager> Uploads;
    TUniquePtr<FMeshPool> MeshPool;
    TUniquePtr<FTexturePool> TexturePool;
    TUniquePtr<FMaterialSystem> MaterialSystem;
    TUniquePtr<FGpuScene> GpuScene;
    TUniquePtr<FTextureStateTracker> TextureStates;
    TUniquePtr<FDebugMeshPass> DebugMeshPass;

    FGfxTextureHandle DepthTexture;
    uint32 LinearWrapSampler = UINT32_MAX;
    uint32 LinearClampSampler = UINT32_MAX;
    uint32 PointClampSampler = UINT32_MAX;
    FMatrix4x4 PrevWorldToClip = FMatrix4x4::Identity();
    float64 StartTime = 0.0;
    float64 LastFrameTime = 0.0;

    uint32 Width = 0;
    uint32 Height = 0;
    bool8 bMinimized = false;
    bool8 bInitialized = false;
};
