// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Renderer.hpp"

#include <algorithm>
#include <filesystem>

#include <Windows.h>

#include "Graphics/GfxCommandList.hpp"
#include "Graphics/GfxDevice.hpp"
#include "Graphics/GfxSwapchain.hpp"
#include "Platform/Window.hpp"
#include "Renderer/RendererLog.hpp"

RAVEN_DEFINE_LOG_CATEGORY(LogRenderer);
RAVEN_DEFINE_LOG_CATEGORY(LogShaders);

namespace
{
    FString ToUtf8(const std::filesystem::path& Path)
    {
        const std::u8string Utf8 = Path.generic_u8string();
        return FString(reinterpret_cast<const char*>(Utf8.data()), Utf8.size());
    }

    TResult<std::filesystem::path> FindProjectDirectory()
    {
        wchar_t ModulePath[MAX_PATH] = {};
        const DWORD Length = GetModuleFileNameW(nullptr, ModulePath, MAX_PATH);
        if (Length == 0 || Length == MAX_PATH)
        {
            return RESULT_PLATFORM_ERROR("GetModuleFileNameW failed");
        }

        std::filesystem::path Directory = std::filesystem::path(ModulePath).parent_path();
        while (true)
        {
            std::error_code Error;
            if (std::filesystem::is_directory(Directory / "Shaders", Error))
            {
                return Directory;
            }
            const std::filesystem::path Parent = Directory.parent_path();
            if (Parent == Directory)
            {
                return RESULT_ERROR("No 'Shaders' directory found above the executable, set FRendererDesc::ShaderDirectory");
            }
            Directory = Parent;
        }
    }

    struct FFullscreenTestConstants
    {
        float32 TopColor[4];
        float32 BottomColor[4];
    };
}

FRenderer::FRenderer() = default;

FRenderer::~FRenderer()
{
    Deinitialize();
}

FResult FRenderer::Initialize(const FRendererDesc& InDesc)
{
    Desc = InDesc;
    if (Desc.Window == nullptr)
    {
        return RESULT_ERROR("FRendererDesc::Window is null");
    }
    RESULT_CHECK(ResolveDirectories());

    RESULT_CHECK_ASSIGN(CreateGfxDevice({
                            .Backend = EGfxBackend::D3D12,
                            .Window = Desc.Window,
                            .bEnableDebugLayer = Desc.bEnableDebugLayer,
                            .bEnableGpuValidation = Desc.bEnableGpuValidation,
                            .FramesInFlight = Desc.FramesInFlight,
                            }), CreatedDevice);
    Device = std::move(CreatedDevice);

    Width = std::max(1u, Desc.Window->GetWindowState().ClientWidth);
    Height = std::max(1u, Desc.Window->GetWindowState().ClientHeight);
    RESULT_CHECK_ASSIGN(Device->CreateSwapchain({
                            .Window = Desc.Window,
                            .Width = Width,
                            .Height = Height,
                            .BufferCount = 3,
                            .bUseVSync = Desc.bVSync,
                            }), CreatedSwapchain);
    Swapchain = std::move(CreatedSwapchain);

    RESULT_CHECK(ShaderLibrary.Initialize(*Device, {
        .ShaderDirectory = Desc.ShaderDirectory,
        .CacheDirectory = Desc.ShaderCacheDirectory,
        .bDebugInfo = Desc.bShaderDebugInfo,
        }));
    PipelineCache.Initialize(*Device, ShaderLibrary);
    RESULT_CHECK(CreatePipelines());

    bInitialized = true;
    RAVEN_LOG(LogRenderer, Info, "Renderer initialized ({}x{})", Width, Height);
    return RESULT_OK_EMPTY();
}

void FRenderer::Deinitialize()
{
    if (Device == nullptr)
    {
        return;
    }

    Device->WaitForIdle();
    PipelineCache.Deinitialize();
    ShaderLibrary.Deinitialize();
    Swapchain.reset();
    Device->WaitForIdle();
    Device.reset();
    bInitialized = false;
}

FResult FRenderer::ResolveDirectories()
{
    if (!Desc.ShaderDirectory.empty() && !Desc.ShaderCacheDirectory.empty())
    {
        return RESULT_OK_EMPTY();
    }

    RESULT_CHECK_ASSIGN(FindProjectDirectory(), ProjectDirectory);
    if (Desc.ShaderDirectory.empty())
    {
        Desc.ShaderDirectory = ToUtf8(ProjectDirectory / "Shaders");
    }
    if (Desc.ShaderCacheDirectory.empty())
    {
        Desc.ShaderCacheDirectory = ToUtf8(ProjectDirectory / "Intermediates" / "ShaderCache");
    }
    return RESULT_OK_EMPTY();
}

FResult FRenderer::CreatePipelines()
{
    FGraphicsPipelineRequest Request = {
        .VertexShader = {.Path = "Debug/FullscreenTest.hlsl", .EntryPoint = "VSMain", .Stage = EGfxShaderStage::Vertex},
        .PixelShader = {.Path = "Debug/FullscreenTest.hlsl", .EntryPoint = "PSMain", .Stage = EGfxShaderStage::Pixel},
        .RenderTargetFormats = {Swapchain->GetFormat()},
        .RenderTargetCount = 1,
    };
    Request.RasterState.CullMode = EGfxCullMode::None;
    Request.DepthStencilState.bDepthTestEnable = false;
    Request.DepthStencilState.bDepthWriteEnable = false;

    RESULT_CHECK_ASSIGN(PipelineCache.GetGraphicsPipeline("Debug.FullscreenTest", Request), Pipeline);
    FullscreenTestPipeline = Pipeline;
    return RESULT_OK_EMPTY();
}

FResult FRenderer::HandleResize()
{
    const FWindowState& State = Desc.Window->GetWindowState();
    bMinimized = State.ClientWidth == 0 || State.ClientHeight == 0;
    if (bMinimized || (State.ClientWidth == Width && State.ClientHeight == Height))
    {
        return RESULT_OK_EMPTY();
    }

    RESULT_CHECK(Swapchain->Resize(State.ClientWidth, State.ClientHeight));
    Width = State.ClientWidth;
    Height = State.ClientHeight;
    RAVEN_LOG(LogRenderer, Verbose, "Resized to {}x{}", Width, Height);
    return RESULT_OK_EMPTY();
}

FResult FRenderer::Render(const FRenderView& View)
{
    if (!bInitialized)
    {
        return RESULT_ERROR("Render called on an uninitialized renderer");
    }

    RESULT_CHECK(HandleResize());
    if (bMinimized)
    {
        return RESULT_OK_EMPTY();
    }

    Swapchain->WaitForFrameLatency();
    Device->BeginFrame();
    const FResult Result = RenderFrame(View);
    Device->EndFrame();
    return Result;
}

FResult FRenderer::RenderFrame(const FRenderView& View)
{
    RESULT_CHECK_ASSIGN(Swapchain->AcquireNextImage(), ImageIndex);
    const FGfxTextureHandle Backbuffer = Swapchain->GetBackBuffer(ImageIndex);

    IGfxCommandList* CommandList = Device->AcquireCommandList(EGfxQueueType::Graphics);
    if (CommandList == nullptr)
    {
        return RESULT_ERROR("Failed to acquire a graphics command list");
    }
    RecordFrame(*CommandList, Backbuffer, View);

    IGfxCommandList* const CommandLists[] = {CommandList};
    Device->Submit({
        .QueueType = EGfxQueueType::Graphics,
        .CommandLists = CommandLists,
        .WaitForAcquire = Swapchain.get(),
        .SignalForPresent = Swapchain.get(),
    });
    return Swapchain->Present(Desc.bVSync);
}

void FRenderer::RecordFrame(IGfxCommandList& CommandList, const FGfxTextureHandle Backbuffer, const FRenderView& View)
{
    (void)View; // used from step 3 on

    CommandList.BeginEvent("Frame");

    const FGfxTextureBarrier ToRenderTarget = {
        .TextureHandle = Backbuffer,
        .SyncBefore = EGfxBarrierSync::None,
        .SyncAfter = EGfxBarrierSync::RenderTarget,
        .AccessBefore = EGfxBarrierAccess::NoAccess,
        .AccessAfter = EGfxBarrierAccess::RenderTarget,
        .LayoutBefore = EGfxTextureLayout::Present,
        .LayoutAfter = EGfxTextureLayout::RenderTarget,
    };
    CommandList.Barrier({}, {}, TSpan<const FGfxTextureBarrier>(&ToRenderTarget, 1));

    const FGfxRenderTargetAttachment Target = {
        .TextureHandle = Backbuffer,
        .LoadOp = EGfxLoadOp::Clear,
        .StoreOp = EGfxStoreOp::Store,
        .ClearColor = {0.02f, 0.02f, 0.03f, 1.0f},
    };
    CommandList.BeginRendering({.RenderTargets = TSpan<const FGfxRenderTargetAttachment>(&Target, 1)});
    CommandList.SetViewport({.Width = static_cast<float32>(Width), .Height = static_cast<float32>(Height)});
    CommandList.SetScissor({.Width = Width, .Height = Height});

    const float32 Time = static_cast<float32>(Device->GetFrameIndex() % 600) / 600.0f;
    const FFullscreenTestConstants Constants = {
        .TopColor = {0.10f, 0.25f + 0.25f * Time, 0.55f, 1.0f},
        .BottomColor = {0.85f, 0.45f, 0.15f, 1.0f},
    };
    CommandList.BeginEvent("FullscreenTest");
    CommandList.SetPipeline(FullscreenTestPipeline);
    CommandList.SetPushConstants(TSpan<const byte8>(reinterpret_cast<const byte8*>(&Constants), sizeof(Constants)));
    CommandList.Draw(3);
    CommandList.EndEvent();

    CommandList.EndRendering();

    const FGfxTextureBarrier ToPresent = {
        .TextureHandle = Backbuffer,
        .SyncBefore = EGfxBarrierSync::RenderTarget,
        .SyncAfter = EGfxBarrierSync::None,
        .AccessBefore = EGfxBarrierAccess::RenderTarget,
        .AccessAfter = EGfxBarrierAccess::NoAccess,
        .LayoutBefore = EGfxTextureLayout::RenderTarget,
        .LayoutAfter = EGfxTextureLayout::Present,
    };
    CommandList.Barrier({}, {}, TSpan<const FGfxTextureBarrier>(&ToPresent, 1));

    CommandList.EndEvent();
}
