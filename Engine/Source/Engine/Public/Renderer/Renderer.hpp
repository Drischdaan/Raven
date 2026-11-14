// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "RenderView.hpp"
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
    FResult HandleResize();
    FResult RenderFrame(const FRenderView& View);
    void RecordFrame(IGfxCommandList& CommandList, FGfxTextureHandle Backbuffer, const FRenderView& View);

private:
    FRendererDesc Desc;
    TSharedPtr<IGfxDevice> Device = nullptr;
    TUniquePtr<IGfxSwapchain> Swapchain = nullptr;
    FShaderLibrary ShaderLibrary;
    FPipelineCache PipelineCache;

    uint32 Width = 0;
    uint32 Height = 0;
    bool8 bMinimized = false;
    bool8 bInitialized = false;

    FGfxPipelineHandle FullscreenTestPipeline;
};
