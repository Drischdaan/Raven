// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Renderer.hpp"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>

#include <Windows.h>

#include "Graphics/GfxCommandList.hpp"
#include "Graphics/GfxDevice.hpp"
#include "Graphics/GfxSwapchain.hpp"
#include "Platform/Window.hpp"
#include "Renderer/GpuTypes.hpp"
#include "Renderer/RendererLog.hpp"
#include "Renderer/Material/MaterialSystem.hpp"
#include "Renderer/Mesh/MeshPool.hpp"
#include "Renderer/Passes/DebugMeshPass.hpp"
#include "Renderer/Scene/GpuScene.hpp"
#include "Renderer/Texture/TexturePool.hpp"
#include "Renderer/Utility/TextureStateTracker.hpp"
#include "Renderer/Utility/UploadManager.hpp"

RAVEN_DEFINE_LOG_CATEGORY(LogRenderer);
RAVEN_DEFINE_LOG_CATEGORY(LogShaders);

namespace
{
    constexpr EGfxFormat DepthFormat = EGfxFormat::D32_FLOAT;

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

    float64 GetSeconds()
    {
        using FClock = std::chrono::steady_clock;
        return std::chrono::duration<float64>(FClock::now().time_since_epoch()).count();
    }

    template <typename T>
    FGfxBufferRange UploadConstants(IGfxCommandList& CommandList, const T& Constants)
    {
        const FGfxTransientAllocation Allocation = CommandList.AllocateTransient(sizeof(T), 256);
        std::memcpy(Allocation.CpuData.data(), &Constants, sizeof(T));
        return Allocation.GpuRange;
    }
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

    Uploads = MakeUnique<FUploadManager>();
    Uploads->Initialize(*Device);
    TextureStates = MakeUnique<FTextureStateTracker>();
    MeshPool = MakeUnique<FMeshPool>();
    RESULT_CHECK(MeshPool->Initialize(*Device, *Uploads, {.MaxVertices = Desc.MaxVertices, .MaxIndices = Desc.MaxIndices}));
    TexturePool = MakeUnique<FTexturePool>();
    RESULT_CHECK(TexturePool->Initialize(*Device, *Uploads));
    MaterialSystem = MakeUnique<FMaterialSystem>();
    RESULT_CHECK(MaterialSystem->Initialize(*Device, *Uploads, *TexturePool));
    RESULT_CHECK(CreateSamplers());
    GpuScene = MakeUnique<FGpuScene>();
    RESULT_CHECK(GpuScene->Initialize(*Device, *Uploads, Scene.GetMaxInstances()));

    RESULT_CHECK(CreateDepthTexture());
    RESULT_CHECK(CreatePipelines());

    StartTime = GetSeconds();
    LastFrameTime = StartTime;
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
    DebugMeshPass.reset();
    if (DepthTexture.IsValid())
    {
        Device->DestroyTexture(DepthTexture);
        DepthTexture = {};
    }
    GpuScene.reset();
    MaterialSystem.reset();
    TexturePool.reset();
    MeshPool.reset();
    TextureStates.reset();
    Uploads.reset();
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
    DebugMeshPass = MakeUnique<FDebugMeshPass>();
    RESULT_CHECK(DebugMeshPass->Initialize(PipelineCache, Swapchain->GetFormat(), DepthFormat));
    return RESULT_OK_EMPTY();
}

FResult FRenderer::CreateSamplers()
{
    const auto MakeSampler = [this](const FGfxSamplerDesc& SamplerDesc) -> TResult<uint32>
    {
        RESULT_CHECK_ASSIGN(Device->GetOrCreateSampler(SamplerDesc), Sampler);
        return Device->GetBindlessIndex(Sampler);
    };

    RESULT_CHECK_ASSIGN(MakeSampler({.MaxAnisotropy = 8}), LinearWrap);
    RESULT_CHECK_ASSIGN(MakeSampler({
                            .AddressU = EGfxAddressMode::Clamp,
                            .AddressV = EGfxAddressMode::Clamp,
                            .AddressW = EGfxAddressMode::Clamp,
                            }), LinearClamp);
    RESULT_CHECK_ASSIGN(MakeSampler({
                            .MinFilter = EGfxFilter::Point,
                            .MagFilter = EGfxFilter::Point,
                            .MipFilter = EGfxFilter::Point,
                            .AddressU = EGfxAddressMode::Clamp,
                            .AddressV = EGfxAddressMode::Clamp,
                            .AddressW = EGfxAddressMode::Clamp,
                            }), PointClamp);
    LinearWrapSampler = LinearWrap;
    LinearClampSampler = LinearClamp;
    PointClampSampler = PointClamp;
    return RESULT_OK_EMPTY();
}

FResult FRenderer::CreateDepthTexture()
{
    if (DepthTexture.IsValid())
    {
        TextureStates->Unregister(DepthTexture);
        Device->DestroyTexture(DepthTexture);
    }

    RESULT_CHECK_ASSIGN(Device->CreateTexture({
                            .Width = Width,
                            .Height = Height,
                            .Format = DepthFormat,
                            .Usage = EGfxTextureUsage::DepthStencil,
                            .OptimizedClearValue = FGfxClearValue{.Depth = 0.0f},
                            }, "SceneDepth"), Created);
    DepthTexture = Created;
    TextureStates->Register(DepthTexture);
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
    RESULT_CHECK(CreateDepthTexture());
    RAVEN_LOG(LogRenderer, Verbose, "Resized to {}x{}", Width, Height);
    return RESULT_OK_EMPTY();
}

TResult<FMeshHandle> FRenderer::CreateMesh(const FMeshData& Data, const FStringView Name)
{
    return MeshPool->CreateMesh(Data, Name);
}

void FRenderer::DestroyMesh(const FMeshHandle Mesh)
{
    MeshPool->DestroyMesh(Mesh);
}

TResult<FTextureHandle> FRenderer::CreateTexture(const FTextureData& Data, const FStringView Name)
{
    return TexturePool->CreateTexture(Data, Name);
}

void FRenderer::DestroyTexture(const FTextureHandle Texture)
{
    TexturePool->DestroyTexture(Texture);
}

TResult<FMaterialHandle> FRenderer::CreateMaterial(const FMaterialDesc& Desc, const FStringView Name)
{
    return MaterialSystem->CreateMaterial(Desc, Name);
}

FResult FRenderer::UpdateMaterial(const FMaterialHandle Material, const FMaterialDesc& Desc)
{
    return MaterialSystem->UpdateMaterial(Material, Desc);
}

void FRenderer::DestroyMaterial(const FMaterialHandle Material)
{
    MaterialSystem->DestroyMaterial(Material);
}

FMaterialHandle FRenderer::GetDefaultMaterial() const
{
    return MaterialSystem->GetDefaultMaterial();
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

FGpuViewConstants FRenderer::MakeViewConstants(const FRenderView& View) const
{
    const float32 AspectRatio = static_cast<float32>(Width) / static_cast<float32>(Height);
    const FMatrix4x4 ViewToClip = FMatrix4x4::MakePerspectiveReverseZ(View.VerticalFov, AspectRatio, View.NearPlane);
    const FMatrix4x4 WorldToClip = ViewToClip * View.ViewMatrix;

    FGpuViewConstants Constants;
    Constants.WorldToView = View.ViewMatrix;
    Constants.ViewToClip = ViewToClip;
    Constants.WorldToClip = WorldToClip;
    Constants.ClipToWorld = WorldToClip.GetInverse();
    Constants.PrevWorldToClip = PrevWorldToClip;
    Constants.CameraPosition = FVector4::FromVector3(View.Position, 1.0f);
    Constants.ViewportSize[0] = static_cast<float32>(Width);
    Constants.ViewportSize[1] = static_cast<float32>(Height);
    Constants.InvViewportSize[0] = 1.0f / static_cast<float32>(Width);
    Constants.InvViewportSize[1] = 1.0f / static_cast<float32>(Height);
    Constants.NearPlane = View.NearPlane;
    return Constants;
}

void FRenderer::RecordFrame(IGfxCommandList& CommandList, const FGfxTextureHandle Backbuffer, const FRenderView& View)
{
    CommandList.BeginEvent("Frame");

    GpuScene->Update(Scene, *MeshPool, *MaterialSystem);
    Uploads->Flush(CommandList);

    const float64 Now = GetSeconds();
    const FGpuFrameConstants FrameConstants = {
        .FrameIndex = static_cast<uint32>(Device->GetFrameIndex()),
        .Time = static_cast<float32>(Now - StartTime),
        .DeltaTime = static_cast<float32>(Now - LastFrameTime),
        .InstanceBufferIndex = Device->GetBindlessIndex(GpuScene->GetInstanceBuffer()),
        .MeshBufferIndex = Device->GetBindlessIndex(MeshPool->GetMeshBuffer()),
        .PositionBufferIndex = Device->GetBindlessIndex(MeshPool->GetPositionBuffer()),
        .AttributeBufferIndex = Device->GetBindlessIndex(MeshPool->GetAttributeBuffer()),
        .MaterialBufferIndex = Device->GetBindlessIndex(MaterialSystem->GetMaterialBuffer()),
        .LinearWrapSampler = LinearWrapSampler,
        .LinearClampSampler = LinearClampSampler,
        .PointClampSampler = PointClampSampler,
    };
    LastFrameTime = Now;

    const FGpuViewConstants ViewConstants = MakeViewConstants(View);
    PrevWorldToClip = ViewConstants.WorldToClip;

    const FDebugMeshPassInputs PassInputs = {
        .FrameConstants = UploadConstants(CommandList, FrameConstants),
        .ViewConstants = UploadConstants(CommandList, ViewConstants),
    };

    TextureStates->Register(Backbuffer, TextureStates::Present);
    TextureStates->Transition(Backbuffer, TextureStates::RenderTarget);
    TextureStates->Transition(DepthTexture, TextureStates::DepthWrite);
    TextureStates->Flush(CommandList);

    const FGfxRenderTargetAttachment ColorTarget = {
        .TextureHandle = Backbuffer,
        .LoadOp = EGfxLoadOp::Clear,
        .StoreOp = EGfxStoreOp::Store,
        .ClearColor = {0.02f, 0.02f, 0.03f, 1.0f},
    };
    CommandList.BeginRendering({
        .RenderTargets = TSpan<const FGfxRenderTargetAttachment>(&ColorTarget, 1),
        .DepthStencil = FGfxDepthStencilAttachment{
            .TextureHandle = DepthTexture,
            .DepthLoadOp = EGfxLoadOp::Clear,
            .DepthStoreOp = EGfxStoreOp::Store,
            .ClearDepth = 0.0f,
        },
    });
    CommandList.SetViewport({.Width = static_cast<float32>(Width), .Height = static_cast<float32>(Height)});
    CommandList.SetScissor({.Width = Width, .Height = Height});

    DebugMeshPass->Render(CommandList, PassInputs, Scene, *MeshPool, *MaterialSystem);

    CommandList.EndRendering();

    TextureStates->Transition(Backbuffer, TextureStates::Present);
    TextureStates->Flush(CommandList);
    TextureStates->Unregister(Backbuffer);

    CommandList.EndEvent();
}
