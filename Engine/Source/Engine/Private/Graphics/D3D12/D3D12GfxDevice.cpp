// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Graphics/D3D12/D3D12GfxDevice.hpp"

#include <print>

#include "Core/Utility/Macros.hpp"
#include "Graphics/D3D12/D3D12Conversions.hpp"
#include "Graphics/D3D12/D3D12GfxCommandList.hpp"

namespace
{
    constexpr uint32 MaxCommandListsPerSubmit = 32;

    FString WideToUtf8(const wchar_t* WideString)
    {
        if (!WideString || !*WideString)
        {
            return "";
        }
        const int size_needed = WideCharToMultiByte(CP_UTF8, 0, WideString, -1, nullptr, 0, nullptr, nullptr);
        if (size_needed <= 0)
        {
            return "";
        }
        FString String(size_needed - 1, 0);
        WideCharToMultiByte(CP_UTF8, 0, WideString, -1, String.data(), size_needed, nullptr, nullptr);
        return String;
    }

    FString FeatureLevelToString(const D3D_FEATURE_LEVEL FeatureLevel)
    {
        switch (FeatureLevel)
        {
        case D3D_FEATURE_LEVEL_1_0_GENERIC: return "D3D_FEATURE_LEVEL_1_0_GENERIC";
        case D3D_FEATURE_LEVEL_1_0_CORE: return "D3D_FEATURE_LEVEL_1_0_CORE";
        case D3D_FEATURE_LEVEL_9_1: return "D3D_FEATURE_LEVEL_9_1";
        case D3D_FEATURE_LEVEL_9_2: return "D3D_FEATURE_LEVEL_9_2";
        case D3D_FEATURE_LEVEL_9_3: return "D3D_FEATURE_LEVEL_9_3";
        case D3D_FEATURE_LEVEL_10_0: return "D3D_FEATURE_LEVEL_10_0";
        case D3D_FEATURE_LEVEL_10_1: return "D3D_FEATURE_LEVEL_10_1";
        case D3D_FEATURE_LEVEL_11_0: return "D3D_FEATURE_LEVEL_11_0";
        case D3D_FEATURE_LEVEL_11_1: return "D3D_FEATURE_LEVEL_11_1";
        case D3D_FEATURE_LEVEL_12_0: return "D3D_FEATURE_LEVEL_12_0";
        case D3D_FEATURE_LEVEL_12_1: return "D3D_FEATURE_LEVEL_12_1";
        case D3D_FEATURE_LEVEL_12_2: return "D3D_FEATURE_LEVEL_12_2";
        default:
            return "Unknown";
        }
    }

    EGfxTextureViewDimension GetDefaultViewDimension(const FGfxTextureDesc& Desc)
    {
        const uint32 ArraySize = GetTextureArraySize(Desc);
        switch (Desc.Dimension)
        {
        case EGfxTextureDimension::Texture1D: return ArraySize > 1 ? EGfxTextureViewDimension::Texture1DArray : EGfxTextureViewDimension::Texture1D;
        case EGfxTextureDimension::Texture2D: return ArraySize > 1 ? EGfxTextureViewDimension::Texture2DArray : EGfxTextureViewDimension::Texture2D;
        case EGfxTextureDimension::Texture3D: return EGfxTextureViewDimension::Texture3D;
        case EGfxTextureDimension::TextureCube: return ArraySize > 6 ? EGfxTextureViewDimension::TextureCubeArray : EGfxTextureViewDimension::TextureCube;
        default:
            return EGfxTextureViewDimension::Texture2D;
        }
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC MakeTextureSrvDesc(const FGfxTextureDesc& Desc, EGfxTextureViewDimension Dimension, const DXGI_FORMAT Format, const FGfxSubresourceRange& Range,
                                                       const uint32 PlaneSlice)
    {
        if (Dimension == EGfxTextureViewDimension::Inherit)
        {
            Dimension = GetDefaultViewDimension(Desc);
        }
        const bool8 bIsMultisampled = Desc.SampleCount > 1;

        D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
        SrvDesc.Format = Format;
        SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

        switch (Dimension)
        {
        case EGfxTextureViewDimension::Texture1D:
            SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
            SrvDesc.Texture1D.MostDetailedMip = Range.BaseMip;
            SrvDesc.Texture1D.MipLevels = Range.MipCount;
            break;
        case EGfxTextureViewDimension::Texture1DArray:
            SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
            SrvDesc.Texture1DArray.MostDetailedMip = Range.BaseMip;
            SrvDesc.Texture1DArray.MipLevels = Range.MipCount;
            SrvDesc.Texture1DArray.FirstArraySlice = Range.BaseArraySlice;
            SrvDesc.Texture1DArray.ArraySize = Range.ArraySliceCount;
            break;
        case EGfxTextureViewDimension::Texture2D:
            if (bIsMultisampled)
            {
                SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMS;
            }
            else
            {
                SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
                SrvDesc.Texture2D.MostDetailedMip = Range.BaseMip;
                SrvDesc.Texture2D.MipLevels = Range.MipCount;
                SrvDesc.Texture2D.PlaneSlice = PlaneSlice;
            }
            break;
        case EGfxTextureViewDimension::Texture2DArray:
            if (bIsMultisampled)
            {
                SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
                SrvDesc.Texture2DMSArray.FirstArraySlice = Range.BaseArraySlice;
                SrvDesc.Texture2DMSArray.ArraySize = Range.ArraySliceCount;
            }
            else
            {
                SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
                SrvDesc.Texture2DArray.MostDetailedMip = Range.BaseMip;
                SrvDesc.Texture2DArray.MipLevels = Range.MipCount;
                SrvDesc.Texture2DArray.FirstArraySlice = Range.BaseArraySlice;
                SrvDesc.Texture2DArray.ArraySize = Range.ArraySliceCount;
                SrvDesc.Texture2DArray.PlaneSlice = PlaneSlice;
            }
            break;
        case EGfxTextureViewDimension::Texture3D:
            SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
            SrvDesc.Texture3D.MostDetailedMip = Range.BaseMip;
            SrvDesc.Texture3D.MipLevels = Range.MipCount;
            break;
        case EGfxTextureViewDimension::TextureCube:
            SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            SrvDesc.TextureCube.MostDetailedMip = Range.BaseMip;
            SrvDesc.TextureCube.MipLevels = Range.MipCount;
            break;
        case EGfxTextureViewDimension::TextureCubeArray:
            SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
            SrvDesc.TextureCubeArray.MostDetailedMip = Range.BaseMip;
            SrvDesc.TextureCubeArray.MipLevels = Range.MipCount;
            SrvDesc.TextureCubeArray.First2DArrayFace = Range.BaseArraySlice;
            SrvDesc.TextureCubeArray.NumCubes = Range.ArraySliceCount / 6;
            break;
        default:
            break;
        }
        return SrvDesc;
    }

    D3D12_UNORDERED_ACCESS_VIEW_DESC MakeTextureUavDesc(const FGfxTextureDesc& Desc, EGfxTextureViewDimension Dimension, const DXGI_FORMAT Format, const FGfxSubresourceRange& Range,
                                                        const uint32 PlaneSlice)
    {
        if (Dimension == EGfxTextureViewDimension::Inherit)
        {
            Dimension = GetDefaultViewDimension(Desc);
        }
        const bool8 bIsMultisampled = Desc.SampleCount > 1;

        D3D12_UNORDERED_ACCESS_VIEW_DESC UavDesc = {};
        UavDesc.Format = Format;

        switch (Dimension)
        {
        case EGfxTextureViewDimension::Texture1D:
            UavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
            UavDesc.Texture1D.MipSlice = Range.BaseMip;
            break;
        case EGfxTextureViewDimension::Texture1DArray:
            UavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
            UavDesc.Texture1DArray.MipSlice = Range.BaseMip;
            UavDesc.Texture1DArray.FirstArraySlice = Range.BaseArraySlice;
            UavDesc.Texture1DArray.ArraySize = Range.ArraySliceCount;
            break;
        case EGfxTextureViewDimension::Texture2D:
            if (bIsMultisampled)
            {
                UavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DMS;
            }
            else
            {
                UavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
                UavDesc.Texture2D.MipSlice = Range.BaseMip;
                UavDesc.Texture2D.PlaneSlice = PlaneSlice;
            }
            break;
        case EGfxTextureViewDimension::Texture2DArray:
        case EGfxTextureViewDimension::TextureCube:
        case EGfxTextureViewDimension::TextureCubeArray:
            if (bIsMultisampled)
            {
                UavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DMSARRAY;
                UavDesc.Texture2DMSArray.FirstArraySlice = Range.BaseArraySlice;
                UavDesc.Texture2DMSArray.ArraySize = Range.ArraySliceCount;
            }
            else
            {
                UavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
                UavDesc.Texture2DArray.MipSlice = Range.BaseMip;
                UavDesc.Texture2DArray.FirstArraySlice = Range.BaseArraySlice;
                UavDesc.Texture2DArray.ArraySize = Range.ArraySliceCount;
                UavDesc.Texture2DArray.PlaneSlice = PlaneSlice;
            }
            break;
        case EGfxTextureViewDimension::Texture3D:
            UavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
            UavDesc.Texture3D.MipSlice = Range.BaseMip;
            UavDesc.Texture3D.FirstWSlice = 0;
            UavDesc.Texture3D.WSize = UINT32_MAX;
            break;
        default:
            break;
        }
        return UavDesc;
    }
}

uint64 FGfxSamplerDescHasher::operator()(const FGfxSamplerDesc& Desc) const noexcept
{
    size64 Hash = 0;
    const auto Combine = [&Hash](const size64 Value)
    {
        Hash ^= Value + 0x9e3779b97f4a7c15ull + (Hash << 6) + (Hash >> 2);
    };
    Combine(std::to_underlying(Desc.MinFilter));
    Combine(std::to_underlying(Desc.MagFilter));
    Combine(std::to_underlying(Desc.MipFilter));
    Combine(std::to_underlying(Desc.AddressU));
    Combine(std::to_underlying(Desc.AddressV));
    Combine(std::to_underlying(Desc.AddressW));
    Combine(std::to_underlying(Desc.Reduction));
    Combine(std::to_underlying(Desc.CompareOp));
    Combine(std::to_underlying(Desc.BorderColor));
    Combine(Desc.MaxAnisotropy);
    Combine(std::hash<float32>{}(Desc.MipLodBias));
    Combine(std::hash<float32>{}(Desc.MinLod));
    Combine(std::hash<float32>{}(Desc.MaxLod));
    return Hash;
}

FResult FD3D12GfxDevice::Initialize(const FGfxDeviceDesc& InDeviceDesc)
{
    if (bIsInitialized)
    {
        return RESULT_ERROR("D3D12 Graphics Device is already initialized");
    }
    DeviceDesc = InDeviceDesc;

    RESULT_CHECK(AcquireFactory())
    RESULT_CHECK(FindSuitableAdapter())
    RESULT_CHECK(CreateDevice())
    RESULT_CHECK(CheckFeatureSupport())
    RESULT_CHECK(CreateQueues())
    RESULT_CHECK(CreateDescriptorHeaps())
    RESULT_CHECK(CreateGlobalRootSignature())

    static_assert(MaxFramesInFlight <= FD3D12TransientAllocator::MaxFrameSlots);
    TransientAllocator.Initialize(*this);

    DeletionQueue.Initialize(Queues);
    FrameIndex = 0;
    bIsInFrame = false;
    FrameFenceValues = {};

    CreateCommandListPools();

    bIsInitialized = true;
    return RESULT_OK_EMPTY();
}

void FD3D12GfxDevice::Deinitialize()
{
    if (!bIsInitialized)
    {
        return;
    }
    WaitForIdle();
    TransientAllocator.Deinitialize();
    DeletionQueue.Flush();
    {
        std::scoped_lock Lock(SamplerCacheMutex);
        SamplerCache.clear();
    }
    ReportAndReleaseLeakedResources();

    for (FD3D12CommandListPool& CommandListPool : CommandListPools)
    {
        CommandListPool.Deinitialize();
    }
    for (FD3D12GfxQueue& Queue : Queues)
    {
        Queue.Deinitialize();
    }
    GlobalRootSignature.Reset();
    for (FD3D12DescriptorHeap& DescriptorHeap : DescriptorHeaps)
    {
        DescriptorHeap.Deinitialize();
    }
    Device.Reset();
    Adapter.Reset();
    Factory.Reset();
    bIsInitialized = false;
}

FD3D12GfxQueue& FD3D12GfxDevice::GetQueue(EGfxQueueType Type) const
{
    return const_cast<FD3D12GfxQueue&>(Queues[static_cast<uint8>(Type)]);
}

TResult<FGfxBufferHandle> FD3D12GfxDevice::CreateBuffer(const FGfxBufferDesc& BufferDesc, FStringView DebugName)
{
    const bool8 bIsCpuVisible = BufferDesc.MemoryType != EGfxMemoryType::GpuOnly;
    const bool8 bHasUav = HasAnyFlags(BufferDesc.Usage, EGfxBufferUsage::UnorderedAccess);
    const bool8 bHasViews = HasAnyFlags(BufferDesc.Usage, EGfxBufferUsage::ShaderResource | EGfxBufferUsage::UnorderedAccess);

    if (BufferDesc.Size == 0)
    {
        return RESULT_ERROR("Buffer size must not be zero");
    }
    if (bHasUav && (BufferDesc.MemoryType == EGfxMemoryType::Upload || BufferDesc.MemoryType == EGfxMemoryType::Readback))
    {
        return RESULT_ERROR("Upload and readback buffers can't have UnorderedAccess usage");
    }
    if (BufferDesc.MemoryType == EGfxMemoryType::GpuUpload && !Capabilities.bSupportsGpuUploadHeap)
    {
        return RESULT_ERROR("GpuUpload memory isn't supported on this device");
    }
    if (bHasViews && BufferDesc.Stride == 0 && BufferDesc.Size % 4 != 0)
    {
        return RESULT_ERROR("Raw buffers (Stride == 0) with views must have a size that's a multiple of 4");
    }
    if (BufferDesc.Stride > 0 && BufferDesc.Size % BufferDesc.Stride != 0)
    {
        return RESULT_ERROR("Buffer size must be a multiple of its stride");
    }

    D3D12_RESOURCE_FLAGS ResourceFlags = D3D12_RESOURCE_FLAG_NONE;
    if (bHasUav)
    {
        ResourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }
    if (HasAnyFlags(BufferDesc.Usage, EGfxBufferUsage::AccelerationStructure))
    {
        ResourceFlags |= D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    const D3D12_HEAP_PROPERTIES HeapProperties = {.Type = ToD3D12HeapType(BufferDesc.MemoryType)};

    D3D12_RESOURCE_DESC1 ResourceDesc = {};
    ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    ResourceDesc.Width = BufferDesc.Size;
    ResourceDesc.Height = 1;
    ResourceDesc.DepthOrArraySize = 1;
    ResourceDesc.MipLevels = 1;
    ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
    ResourceDesc.SampleDesc = {.Count = 1, .Quality = 0};
    ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ResourceDesc.Flags = ResourceFlags;

    FD3D12Buffer Buffer;
    Buffer.Desc = BufferDesc;

    RESULT_TRY_PLATFORM_CALL(Device->CreateCommittedResource3(
                                 &HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED,
                                 nullptr, nullptr, 0, nullptr, IID_PPV_ARGS(&Buffer.Resource)), "Failed to create buffer")

    Buffer.GpuAddress = Buffer.Resource->GetGPUVirtualAddress();
    SetD3D12DebugName(Buffer.Resource.Get(), DebugName);

    if (bIsCpuVisible)
    {
        constexpr D3D12_RANGE NoRead = {.Begin = 0, .End = 0};
        const D3D12_RANGE* ReadRange = BufferDesc.MemoryType == EGfxMemoryType::Readback ? nullptr : &NoRead;
        void* MappedData = nullptr;
        RESULT_TRY_PLATFORM_CALL(Buffer.Resource->Map(0, ReadRange, &MappedData), "Failed to map buffer")
        Buffer.MappedData = static_cast<byte8*>(MappedData);
    }

    RESULT_CHECK(CreateBufferViews(Buffer))

    const uint32 SrvIndex = Buffer.SrvIndex;
    const uint32 UavIndex = Buffer.UavIndex;
    TResult<FGfxBufferHandle> Handle = Buffers.Allocate(std::move(Buffer));
    if (Handle.HasError())
    {
        FreeDescriptorsNow(SrvIndex, UavIndex);
    }
    return Handle;
}

TResult<FGfxTextureHandle> FD3D12GfxDevice::CreateTexture(const FGfxTextureDesc& TextureDesc, FStringView DebugName)
{
    const FD3D12FormatInfo FormatInfo = GetD3D12FormatInfo(TextureDesc.Format);
    const bool8 bIsRenderTarget = HasAnyFlags(TextureDesc.Usage, EGfxTextureUsage::RenderTarget);
    const bool8 bIsDepthStencil = HasAnyFlags(TextureDesc.Usage, EGfxTextureUsage::DepthStencil);
    const bool8 bHasSrv = HasAnyFlags(TextureDesc.Usage, EGfxTextureUsage::ShaderResource);
    const bool8 bHasUav = HasAnyFlags(TextureDesc.Usage, EGfxTextureUsage::UnorderedAccess);

    if (FormatInfo.ResourceFormat == DXGI_FORMAT_UNKNOWN)
    {
        return RESULT_ERROR("Unknown texture format");
    }
    if (TextureDesc.Width == 0 || TextureDesc.Height == 0 || TextureDesc.DepthOrArraySize == 0 || TextureDesc.SampleCount == 0)
    {
        return RESULT_ERROR("Texture size, array size and sample count must not be zero");
    }
    if (bIsRenderTarget && bIsDepthStencil)
    {
        return RESULT_ERROR("A texture can't be both a render target and a depth stencil");
    }
    if (bIsDepthStencil != FormatInfo.bIsDepth && (bIsDepthStencil || bIsRenderTarget))
    {
        return RESULT_ERROR("DepthStencil usage needs a depth format, and RenderTarget usage a color format");
    }
    if (bHasUav && (FormatInfo.bIsSrgb || FormatInfo.bIsDepth))
    {
        return RESULT_ERROR("sRGB and depth textures can't have UnorderedAccess usage");
    }
    if (TextureDesc.Dimension == EGfxTextureDimension::TextureCube && (TextureDesc.DepthOrArraySize % 6 != 0 || TextureDesc.Width != TextureDesc.Height))
    {
        return RESULT_ERROR("Cube textures need square faces and DepthOrArraySize as a multiple of 6");
    }
    if (TextureDesc.SampleCount > 1 && (TextureDesc.MipLevels > 1 || TextureDesc.Dimension != EGfxTextureDimension::Texture2D))
    {
        return RESULT_ERROR("Multisampled textures must be 2D with a single mip");
    }

    D3D12_RESOURCE_FLAGS ResourceFlags = D3D12_RESOURCE_FLAG_NONE;
    if (bIsRenderTarget)
    {
        ResourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }
    if (bIsDepthStencil)
    {
        ResourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
        if (!bHasSrv)
        {
            ResourceFlags |= D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
        }
    }
    if (bHasUav)
    {
        ResourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }

    D3D12_RESOURCE_DESC1 ResourceDesc = {};
    ResourceDesc.Dimension = ToD3D12ResourceDimension(TextureDesc.Dimension);
    ResourceDesc.Width = TextureDesc.Width;
    ResourceDesc.Height = TextureDesc.Dimension == EGfxTextureDimension::Texture1D ? 1 : TextureDesc.Height;
    ResourceDesc.DepthOrArraySize = static_cast<uint16>(TextureDesc.DepthOrArraySize);
    ResourceDesc.MipLevels = static_cast<uint16>(TextureDesc.MipLevels);
    ResourceDesc.Format = FormatInfo.ResourceFormat;
    ResourceDesc.SampleDesc = {.Count = TextureDesc.SampleCount, .Quality = 0};
    ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    ResourceDesc.Flags = ResourceFlags;

    D3D12_CLEAR_VALUE ClearValue = {};
    const D3D12_CLEAR_VALUE* OptimizedClearValue = nullptr;
    if (TextureDesc.OptimizedClearValue.has_value() && (bIsRenderTarget || bIsDepthStencil))
    {
        const FGfxClearValue& Clear = *TextureDesc.OptimizedClearValue;
        if (bIsDepthStencil)
        {
            ClearValue.Format = FormatInfo.DepthStencilFormat;
            ClearValue.DepthStencil.Depth = Clear.Depth;
            ClearValue.DepthStencil.Stencil = Clear.Stencil;
        }
        else
        {
            ClearValue.Format = FormatInfo.ViewFormat;
            std::memcpy(ClearValue.Color, Clear.Color.data(), sizeof(ClearValue.Color));
        }
        OptimizedClearValue = &ClearValue;
    }

    constexpr D3D12_HEAP_PROPERTIES HeapProperties = {.Type = D3D12_HEAP_TYPE_DEFAULT};

    FD3D12Texture Texture;
    RESULT_TRY_PLATFORM_CALL(Device->CreateCommittedResource3(
                                 &HeapProperties, D3D12_HEAP_FLAG_NONE, &ResourceDesc, D3D12_BARRIER_LAYOUT_COMMON,
                                 OptimizedClearValue, nullptr, 0, nullptr, IID_PPV_ARGS(&Texture.Resource)), "Failed to create texture")

    const D3D12_RESOURCE_DESC CreatedDesc = Texture.Resource->GetDesc();
    Texture.NativeDesc = ResourceDesc;
    Texture.NativeDesc.MipLevels = CreatedDesc.MipLevels;
    Texture.Desc = TextureDesc;
    Texture.Desc.MipLevels = CreatedDesc.MipLevels;
    SetD3D12DebugName(Texture.Resource.Get(), DebugName);

    RESULT_CHECK(CreateTextureDefaultViews(Texture))

    const uint32 SrvIndex = Texture.SrvIndex;
    const uint32 UavIndex = Texture.UavIndex;
    TResult<FGfxTextureHandle> Handle = Textures.Allocate(std::move(Texture));
    if (Handle.HasError())
    {
        FreeDescriptorsNow(SrvIndex, UavIndex);
    }
    return Handle;
}

TResult<FGfxTextureViewHandle> FD3D12GfxDevice::CreateTextureView(FGfxTextureHandle TextureHandle, const FGfxTextureViewDesc& TextureViewDesc)
{
    const FD3D12Texture* Texture = Textures.Get(TextureHandle);
    if (Texture == nullptr)
    {
        return RESULT_ERROR("CreateTextureView: invalid or stale texture handle");
    }

    const FGfxTextureDesc& Desc = Texture->Desc;
    const FD3D12FormatInfo FormatInfo = GetD3D12FormatInfo(Desc.Format);
    const bool8 bIsUav = TextureViewDesc.ViewType == EGfxViewType::UnorderedAccess;

    if (bIsUav && !HasAnyFlags(Desc.Usage, EGfxTextureUsage::UnorderedAccess))
    {
        return RESULT_ERROR("CreateTextureView: texture wasn't created with UnorderedAccess usage");
    }
    if (!bIsUav && !HasAnyFlags(Desc.Usage, EGfxTextureUsage::ShaderResource))
    {
        return RESULT_ERROR("CreateTextureView: texture wasn't created with ShaderResource usage");
    }

    RESULT_CHECK_ASSIGN(ResolveSubresourceRange(Desc, TextureViewDesc.Range), Range)

    const bool8 bIsStencil = TextureViewDesc.Range.Aspect == EGfxTextureAspect::Stencil;
    if (bIsStencil && !FormatInfo.bHasStencil)
    {
        return RESULT_ERROR("CreateTextureView: texture has no stencil plane");
    }
    DXGI_FORMAT ViewFormat = bIsStencil ? FormatInfo.StencilViewFormat : FormatInfo.ViewFormat;
    if (TextureViewDesc.Format != EGfxFormat::Unknown)
    {
        ViewFormat = GetD3D12FormatInfo(TextureViewDesc.Format).ViewFormat;
    }
    const uint32 PlaneSlice = bIsStencil ? 1 : 0;

    FD3D12DescriptorHeap& ResourceHeap = GetDescriptorHeap(ED3D12DescriptorHeapType::Resource);
    RESULT_CHECK_ASSIGN(ResourceHeap.Allocate(), Descriptor)

    if (bIsUav)
    {
        const D3D12_UNORDERED_ACCESS_VIEW_DESC UavDesc = MakeTextureUavDesc(Desc, TextureViewDesc.Dimension, ViewFormat, Range, PlaneSlice);
        Device->CreateUnorderedAccessView(Texture->Resource.Get(), nullptr, &UavDesc, Descriptor.CpuHandle);
    }
    else
    {
        const D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = MakeTextureSrvDesc(Desc, TextureViewDesc.Dimension, ViewFormat, Range, PlaneSlice);
        Device->CreateShaderResourceView(Texture->Resource.Get(), &SrvDesc, Descriptor.CpuHandle);
    }

    FD3D12TextureView TextureView;
    TextureView.TextureHandle = TextureHandle;
    TextureView.Desc = TextureViewDesc;
    TextureView.DescriptorIndex = Descriptor.Index;

    TResult<FGfxTextureViewHandle> Handle = TextureViews.Allocate(std::move(TextureView));
    if (Handle.HasError())
    {
        ResourceHeap.Free(Descriptor.Index);
    }
    return Handle;
}

TResult<FGfxSamplerHandle> FD3D12GfxDevice::GetOrCreateSampler(const FGfxSamplerDesc& SamplerDesc)
{
    std::scoped_lock Lock(SamplerCacheMutex);
    if (const auto Iterator = SamplerCache.find(SamplerDesc); Iterator != SamplerCache.end())
    {
        return RESULT_OK(Iterator->second);
    }

    FD3D12DescriptorHeap& SamplerHeap = GetDescriptorHeap(ED3D12DescriptorHeapType::Sampler);
    RESULT_CHECK_ASSIGN(SamplerHeap.Allocate(), Descriptor)

    const D3D12_SAMPLER_DESC NativeDesc = ToD3D12SamplerDesc(SamplerDesc);
    Device->CreateSampler(&NativeDesc, Descriptor.CpuHandle);

    TResult<FGfxSamplerHandle> Handle = Samplers.Allocate(FD3D12Sampler{.Desc = SamplerDesc, .DescriptorIndex = Descriptor.Index});
    if (Handle.HasError())
    {
        SamplerHeap.Free(Descriptor.Index);
        return Handle;
    }
    SamplerCache.emplace(SamplerDesc, Handle.GetValue());
    return Handle;
}

void FD3D12GfxDevice::DestroyBuffer(const FGfxBufferHandle BufferHandle)
{
    TResult<FD3D12Buffer> Removed = Buffers.Remove(BufferHandle);
    if (Removed.HasError())
    {
        std::println("DestroyBuffer: {}", Removed.GetError().Message);
        return;
    }
    FD3D12Buffer Buffer = std::move(Removed).GetValue();
    DeferRelease(std::move(Buffer.Resource));
    FreeDescriptorDeferred(ED3D12DescriptorHeapType::Resource, Buffer.SrvIndex);
    FreeDescriptorDeferred(ED3D12DescriptorHeapType::Resource, Buffer.UavIndex);
}

void FD3D12GfxDevice::DestroyTexture(const FGfxTextureHandle TextureHandle)
{
    TResult<FD3D12Texture> Removed = Textures.Remove(TextureHandle);
    if (Removed.HasError())
    {
        std::println("DestroyTexture: {}", Removed.GetError().Message);
        return;
    }
    FD3D12Texture Texture = std::move(Removed).GetValue();
    if (!Texture.bIsSwapchainImage)
    {
        DeferRelease(std::move(Texture.Resource));
    }
    FreeDescriptorDeferred(ED3D12DescriptorHeapType::Resource, Texture.SrvIndex);
    FreeDescriptorDeferred(ED3D12DescriptorHeapType::Resource, Texture.UavIndex);
}

void FD3D12GfxDevice::DestroyTextureView(const FGfxTextureViewHandle TextureViewHandle)
{
    TResult<FD3D12TextureView> Removed = TextureViews.Remove(TextureViewHandle);
    if (Removed.HasError())
    {
        std::println("DestroyTextureView: {}", Removed.GetError().Message);
        return;
    }
    FreeDescriptorDeferred(ED3D12DescriptorHeapType::Resource, Removed.GetValue().DescriptorIndex);
}

uint32 FD3D12GfxDevice::GetBindlessIndex(const FGfxBufferHandle BufferHandle, const EGfxViewType ViewType) const
{
    const FD3D12Buffer* Buffer = Buffers.Get(BufferHandle);
    if (Buffer == nullptr)
    {
        return GfxInvalidBindlessIndex;
    }
    return ViewType == EGfxViewType::UnorderedAccess ? Buffer->UavIndex : Buffer->SrvIndex;
}

uint32 FD3D12GfxDevice::GetBindlessIndex(const FGfxTextureHandle TextureHandle, const EGfxViewType ViewType) const
{
    const FD3D12Texture* Texture = Textures.Get(TextureHandle);
    if (Texture == nullptr)
    {
        return GfxInvalidBindlessIndex;
    }
    return ViewType == EGfxViewType::UnorderedAccess ? Texture->UavIndex : Texture->SrvIndex;
}

uint32 FD3D12GfxDevice::GetBindlessIndex(const FGfxTextureViewHandle TextureViewHandle) const
{
    const FD3D12TextureView* TextureView = TextureViews.Get(TextureViewHandle);
    return TextureView != nullptr ? TextureView->DescriptorIndex : GfxInvalidBindlessIndex;
}

uint32 FD3D12GfxDevice::GetBindlessIndex(const FGfxSamplerHandle SamplerHandle) const
{
    const FD3D12Sampler* Sampler = Samplers.Get(SamplerHandle);
    return Sampler != nullptr ? Sampler->DescriptorIndex : GfxInvalidBindlessIndex;
}

FD3D12Buffer* FD3D12GfxDevice::GetBuffer(const FGfxBufferHandle BufferHandle)
{
    return Buffers.Get(BufferHandle);
}

FD3D12Texture* FD3D12GfxDevice::GetTexture(const FGfxTextureHandle TextureHandle)
{
    return Textures.Get(TextureHandle);
}

FD3D12TextureView* FD3D12GfxDevice::GetTextureView(const FGfxTextureViewHandle TextureViewHandle)
{
    return TextureViews.Get(TextureViewHandle);
}

FD3D12Pipeline* FD3D12GfxDevice::GetPipeline(const FGfxPipelineHandle PipelineHandle)
{
    return Pipelines.Get(PipelineHandle);
}

FD3D12DescriptorHeap& FD3D12GfxDevice::GetDescriptorHeap(ED3D12DescriptorHeapType HeapType)
{
    return DescriptorHeaps[static_cast<uint8>(HeapType)];
}

ID3D12RootSignature* FD3D12GfxDevice::GetGlobalRootSignature() const
{
    return GlobalRootSignature.Get();
}

void FD3D12GfxDevice::FreeDescriptorDeferred(ED3D12DescriptorHeapType HeapType, uint32 Index)
{
    if (Index == GfxInvalidBindlessIndex)
    {
        return;
    }
    DeferRelease(nullptr, [this, HeapType, Index]
    {
        GetDescriptorHeap(HeapType).Free(Index);
    });
}

TSpan<byte8> FD3D12GfxDevice::Map(const FGfxBufferHandle BufferHandle)
{
    const FD3D12Buffer* Buffer = Buffers.Get(BufferHandle);
    if (Buffer == nullptr || Buffer->MappedData == nullptr)
    {
        return {};
    }
    return TSpan<byte8>(Buffer->MappedData, Buffer->Desc.Size);
}

FGfxTextureCopyLayout FD3D12GfxDevice::GetCopyLayout(const FGfxTextureHandle TextureHandle, const uint32 Subresource) const
{
    const FD3D12Texture* Texture = Textures.Get(TextureHandle);
    if (Texture == nullptr)
    {
        return {};
    }

    D3D12_PLACED_SUBRESOURCE_FOOTPRINT Footprint = {};
    uint32 RowCount = 0;
    uint64 RowSize = 0;
    uint64 TotalSize = 0;
    Device->GetCopyableFootprints1(&Texture->NativeDesc, Subresource, 1, 0, &Footprint, &RowCount, &RowSize, &TotalSize);
    if (TotalSize == UINT64_MAX)
    {
        return {};
    }

    return FGfxTextureCopyLayout{
        .TotalSize = TotalSize,
        .OffsetAlignment = D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT,
        .RowPitch = Footprint.Footprint.RowPitch,
        .RowSize = static_cast<uint32>(RowSize),
        .RowCount = RowCount,
        .Width = Footprint.Footprint.Width,
        .Height = Footprint.Footprint.Height,
        .Depth = Footprint.Footprint.Depth,
    };
}

IGfxCommandList* FD3D12GfxDevice::AcquireCommandList(EGfxQueueType QueueType)
{
    return CommandListPools[static_cast<uint8>(QueueType)].Acquire();
}

FGfxSyncPoint FD3D12GfxDevice::Submit(const FGfxSubmitDesc& SubmitDesc)
{
    if (SubmitDesc.CommandLists.size() > MaxCommandListsPerSubmit)
    {
        std::println("Submit: too many command lists ({} > {})", SubmitDesc.CommandLists.size(), MaxCommandListsPerSubmit);
        return {};
    }

    TArray<FD3D12GfxCommandList*, MaxCommandListsPerSubmit> D3D12Lists;
    TArray<ID3D12CommandList*, MaxCommandListsPerSubmit> NativeLists;
    uint32 Count = 0;

    for (IGfxCommandList* CommandList : SubmitDesc.CommandLists)
    {
        FD3D12GfxCommandList* D3D12List = dynamic_cast<FD3D12GfxCommandList*>(CommandList);
        if (D3D12List->GetQueueType() != SubmitDesc.QueueType)
        {
            std::println("Submit: command list was acquired for a different queue type");
            return {};
        }
        if (!D3D12List->IsRecording())
        {
            std::println("Submit: command list was already submitted");
            return {};
        }
        D3D12List->Close();
        D3D12Lists[Count] = D3D12List;
        NativeLists[Count] = D3D12List->GetNative();
        ++Count;
    }

    const FGfxSyncPoint SyncPoint = GetQueue(SubmitDesc.QueueType).Submit({NativeLists.data(), Count}, SubmitDesc.WaitFor, *this);
    CommandListPools[static_cast<uint8>(SubmitDesc.QueueType)].Recycle({D3D12Lists.data(), Count}, SyncPoint.Value);
    return SyncPoint;
}

void FD3D12GfxDevice::WaitForSyncPoint(const FGfxSyncPoint SyncPoint)
{
    GetQueue(SyncPoint.QueueType).CpuWait(SyncPoint.Value);
}

bool8 FD3D12GfxDevice::IsComplete(const FGfxSyncPoint SyncPoint) const
{
    return GetQueue(SyncPoint.QueueType).IsComplete(SyncPoint.Value);
}

void FD3D12GfxDevice::WaitForIdle()
{
    for (FD3D12GfxQueue& Queue : Queues)
    {
        Queue.WaitForIdle();
    }
}

void FD3D12GfxDevice::BeginFrame()
{
    if (bIsInFrame)
    {
        std::println("BeginFrame called twice without EndFrame");
        return;
    }

    const uint32 Slot = GetFrameSlot();
    WaitForFenceValues(FrameFenceValues[Slot]);
    DeletionQueue.Process();
    TransientAllocator.BeginFrame(Slot);
    bIsInFrame = true;
}

void FD3D12GfxDevice::EndFrame()
{
    if (!bIsInFrame)
    {
        std::println("EndFrame called without BeginFrame");
        return;
    }

    GetQueue(EGfxQueueType::Graphics).Signal();

    const FD3D12QueueFenceValues FenceValues = CaptureLastSubmittedValues();
    FrameFenceValues[GetFrameSlot()] = FenceValues;
    DeletionQueue.Retire(FenceValues);

    ++FrameIndex;
    bIsInFrame = false;
}

uint64 FD3D12GfxDevice::GetFrameIndex() const
{
    return FrameIndex;
}

uint32 FD3D12GfxDevice::GetFrameSlot() const
{
    return static_cast<uint32>(FrameIndex % DeviceDesc.FramesInFlight);
}

void FD3D12GfxDevice::DeferRelease(TComPtr<IUnknown> Object, FD3D12DeferredDeletionQueue::FReleaseCallback OnRelease)
{
    DeletionQueue.Enqueue(std::move(Object), std::move(OnRelease));
}

ID3D12Device15* FD3D12GfxDevice::GetNativeDevice() const
{
    return Device.Get();
}

FGfxTransientAllocation FD3D12GfxDevice::AllocateTransient(const uint64 Size, const uint64 Alignment)
{
    if (!bIsInFrame)
    {
        std::println("AllocateTransient: only valid between BeginFrame and EndFrame");
        return {};
    }
    TResult<FGfxTransientAllocation> Allocation = TransientAllocator.Allocate(Size, Alignment);
    if (Allocation.HasError())
    {
        std::println("AllocateTransient: {}", Allocation.GetError().Message);
        return {};
    }
    return std::move(Allocation).GetValue();
}

FResult FD3D12GfxDevice::AcquireFactory()
{
    uint32 FactoryCreateFlags = 0;
    if (DeviceDesc.bEnableDebugLayer)
    {
        FactoryCreateFlags = DXGI_CREATE_FACTORY_DEBUG;
    }
    RESULT_TRY_PLATFORM_CALL(CreateDXGIFactory2(FactoryCreateFlags, IID_PPV_ARGS(&Factory)), "Unable to create dxgi factory")
    return RESULT_OK_EMPTY();
}

FResult FD3D12GfxDevice::FindSuitableAdapter()
{
    TComPtr<IDXGIAdapter4> TempAdapter;
    for (uint32 Index = 0; Factory->EnumAdapterByGpuPreference(Index, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&TempAdapter)) != DXGI_ERROR_NOT_FOUND; ++Index)
    {
        DXGI_ADAPTER_DESC3 AdapterDesc = {};
        RESULT_TRY_PLATFORM_CALL_CLEANUP(TempAdapter->GetDesc3(&AdapterDesc), TempAdapter.Reset();, "Failed to get adapter description")
        if ((AdapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE) == DXGI_ADAPTER_FLAG3_SOFTWARE)
        {
            TempAdapter.Reset();
            continue;
        }
        if ((AdapterDesc.Flags & DXGI_ADAPTER_FLAG3_REMOTE) == DXGI_ADAPTER_FLAG3_REMOTE)
        {
            TempAdapter.Reset();
            continue;
        }
        std::println("Chosen Adapter: {}", WideToUtf8(AdapterDesc.Description));
        break;
    }
    if (TempAdapter == nullptr)
    {
        return RESULT_ERROR("Unable to find suitable adapter");
    }
    Adapter = TempAdapter;
    return RESULT_OK_EMPTY();
}

FResult FD3D12GfxDevice::CreateDevice()
{
    D3D_FEATURE_LEVEL FeatureLevels[5] = {
        D3D_FEATURE_LEVEL_12_2,
        D3D_FEATURE_LEVEL_12_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

    D3D_FEATURE_LEVEL HighestSupportedFeatureLevel = FeatureLevels[_countof(FeatureLevels) - 1];
    for (const D3D_FEATURE_LEVEL& FeatureLevelProbe : FeatureLevels)
    {
        const HRESULT NativeResult = D3D12CreateDevice(Adapter.Get(), FeatureLevelProbe, __uuidof(ID3D12Device), nullptr);
        if (FAILED(NativeResult))
        {
            continue;
        }
        HighestSupportedFeatureLevel = FeatureLevelProbe;
        break;
    }

    if (HighestSupportedFeatureLevel < D3D_FEATURE_LEVEL_12_0)
    {
        return RESULT_ERROR("Adapter must at least support feature level 12.0");
    }
    FeatureLevel = HighestSupportedFeatureLevel;
    RESULT_TRY_PLATFORM_CALL_CLEANUP(D3D12CreateDevice(Adapter.Get(), FeatureLevel, IID_PPV_ARGS(&Device)), Device.Reset();, "Failed to create D3D12 device")
    std::println("Created device with feature level {}", FeatureLevelToString(FeatureLevel));
    return RESULT_OK_EMPTY();
}

FResult FD3D12GfxDevice::CheckFeatureSupport()
{
    BOOL bSupportsTearing = FALSE;
    HRESULT NativeResult = Factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &bSupportsTearing, sizeof(bSupportsTearing));
    if (FAILED(NativeResult))
    {
        bSupportsTearing = FALSE;
    }
    Capabilities.bSupportsTearing = bSupportsTearing;

    D3D12_FEATURE_DATA_D3D12_OPTIONS Options = {};
    RESULT_TRY_PLATFORM_CALL(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS, &Options, sizeof(Options)), "Failed to check feature options")
    if (Options.ResourceBindingTier < D3D12_RESOURCE_BINDING_TIER_3)
    {
        return RESULT_ERROR("Bindless rendering requires resource binding tier 3");
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS5 Options5 = {};
    RESULT_TRY_PLATFORM_CALL(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5, &Options5, sizeof(Options5)), "Failed to check feature options")
    Capabilities.bSupportsRayTracing = Options5.RaytracingTier >= D3D12_RAYTRACING_TIER_1_0;

    D3D12_FEATURE_DATA_D3D12_OPTIONS6 Options6 = {};
    RESULT_TRY_PLATFORM_CALL(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS6, &Options6, sizeof(Options6)), "Failed to check feature options")
    Capabilities.bSupportsVariableRateShading = Options6.VariableShadingRateTier >= D3D12_VARIABLE_SHADING_RATE_TIER_1;

    D3D12_FEATURE_DATA_D3D12_OPTIONS7 Options7 = {};
    RESULT_TRY_PLATFORM_CALL(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &Options7, sizeof(Options7)), "Failed to check feature options")
    Capabilities.bSupportsMeshShaders = Options7.MeshShaderTier >= D3D12_MESH_SHADER_TIER_1;

    D3D12_FEATURE_DATA_D3D12_OPTIONS12 Options12 = {};
    RESULT_TRY_PLATFORM_CALL(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS12, &Options12, sizeof(Options12)), "Failed to check feature options")
    if (!Options12.EnhancedBarriersSupported)
    {
        return RESULT_ERROR("Enhanced barriers are required");
    }

    D3D12_FEATURE_DATA_D3D12_OPTIONS16 Options16 = {};
    RESULT_TRY_PLATFORM_CALL(Device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS16, &Options16, sizeof(Options16)), "Failed to check feature options")
    Capabilities.bSupportsGpuUploadHeap = Options16.GPUUploadHeapSupported;

    D3D12_FEATURE_DATA_SHADER_MODEL ShaderModel = {.HighestShaderModel = D3D_SHADER_MODEL_6_6};
    NativeResult = Device->CheckFeatureSupport(D3D12_FEATURE_SHADER_MODEL, &ShaderModel, sizeof(ShaderModel));
    if (FAILED(NativeResult) || ShaderModel.HighestShaderModel < D3D_SHADER_MODEL_6_6)
    {
        return RESULT_ERROR("Bindless rendering requires shader model 6.6 (ResourceDescriptorHeap)");
    }

    std::println("Device Capabilities:");
    std::println(" - Tearing: {}", Capabilities.bSupportsTearing);
    std::println(" - RayTracing: {}", Capabilities.bSupportsRayTracing);
    std::println(" - VariableRateShading: {}", Capabilities.bSupportsVariableRateShading);
    std::println(" - MeshShaders: {}", Capabilities.bSupportsMeshShaders);
    std::println(" - GpuUploadHeap: {}", Capabilities.bSupportsGpuUploadHeap);

    return RESULT_OK_EMPTY();
}

FResult FD3D12GfxDevice::CreateQueues()
{
    for (uint8 Index = 0; Index < static_cast<uint8>(EGfxQueueType::Count); ++Index)
    {
        RESULT_CHECK(Queues[Index].Initialize(Device.Get(), static_cast<EGfxQueueType>(Index)))
    }
    return RESULT_OK_EMPTY();
}

void FD3D12GfxDevice::CreateCommandListPools()
{
    for (uint8 Index = 0; Index < static_cast<uint8>(EGfxQueueType::Count); ++Index)
    {
        CommandListPools[Index].Initialize(*this, Device.Get(), Queues[Index]);
    }
}

FResult FD3D12GfxDevice::CreateDescriptorHeaps()
{
    const FD3D12DescriptorHeapDesc HeapDescs[] = {
        {
            .HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
            .Capacity = D3D12ResourceHeapCapacity,
            .ReservedCount = 1,
            .bIsShaderVisible = true,
            .DebugName = L"BindlessResourceHeap",
        },
        {
            .HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER,
            .Capacity = D3D12SamplerHeapCapacity,
            .ReservedCount = 1,
            .bIsShaderVisible = true,
            .DebugName = L"BindlessSamplerHeap",
        },
        {
            .HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
            .Capacity = D3D12RenderTargetHeapCapacity,
            .ReservedCount = 0,
            .bIsShaderVisible = false,
            .DebugName = L"RenderTargetViewHeap",
        },
        {
            .HeapType = D3D12_DESCRIPTOR_HEAP_TYPE_DSV,
            .Capacity = D3D12DepthStencilHeapCapacity,
            .ReservedCount = 0,
            .bIsShaderVisible = false,
            .DebugName = L"DepthStencilViewHeap",
        },
    };
    static_assert(std::extent_v<decltype(HeapDescs)> == static_cast<size64>(ED3D12DescriptorHeapType::Count));
    for (uint8 Index = 0; Index < static_cast<uint8>(ED3D12DescriptorHeapType::Count); ++Index)
    {
        RESULT_CHECK(DescriptorHeaps[Index].Initialize(Device.Get(), HeapDescs[Index]))
    }

    WriteDefaultDescriptors();
    return RESULT_OK_EMPTY();
}

void FD3D12GfxDevice::WriteDefaultDescriptors()
{
    // Resource index 0: null Texture2D SRV. Loads and samples return 0.
    D3D12_SHADER_RESOURCE_VIEW_DESC NullSrvDesc = {};
    NullSrvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    NullSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    NullSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    NullSrvDesc.Texture2D.MipLevels = 1;
    Device->CreateShaderResourceView(nullptr, &NullSrvDesc, GetDescriptorHeap(ED3D12DescriptorHeapType::Resource).GetDescriptor(D3D12NullResourceIndex).CpuHandle);

    // Sampler index 0: trilinear wrap.
    D3D12_SAMPLER_DESC DefaultSamplerDesc = {};
    DefaultSamplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    DefaultSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    DefaultSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    DefaultSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    DefaultSamplerDesc.MaxAnisotropy = 1;
    DefaultSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    DefaultSamplerDesc.MinLOD = 0.0f;
    DefaultSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    Device->CreateSampler(&DefaultSamplerDesc, GetDescriptorHeap(ED3D12DescriptorHeapType::Sampler).GetDescriptor(D3D12DefaultSamplerIndex).CpuHandle);
}

FResult FD3D12GfxDevice::CreateGlobalRootSignature()
{
    D3D12_ROOT_PARAMETER1 Parameters[static_cast<uint8>(ED3D12RootParameter::Count)] = {};

    D3D12_ROOT_PARAMETER1& PushConstants = Parameters[static_cast<uint8>(ED3D12RootParameter::PushConstants)];
    PushConstants.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
    PushConstants.Constants.ShaderRegister = 0;
    PushConstants.Constants.RegisterSpace = 0;
    PushConstants.Constants.Num32BitValues = GfxMaxPushConstantsSize / sizeof(uint32);
    PushConstants.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    constexpr ED3D12RootParameter ConstantBufferParameters[] = {
        ED3D12RootParameter::FrameConstants,
        ED3D12RootParameter::ViewConstants,
        ED3D12RootParameter::PassConstants,
    };
    for (uint32 Index = 0; Index < std::size(ConstantBufferParameters); ++Index)
    {
        D3D12_ROOT_PARAMETER1& Parameter = Parameters[static_cast<uint8>(ConstantBufferParameters[Index])];
        Parameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
        Parameter.Descriptor.ShaderRegister = 1 + Index;
        Parameter.Descriptor.RegisterSpace = 0;
        Parameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE;
        Parameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    }

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc = {};
    RootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    RootSignatureDesc.Desc_1_1.NumParameters = static_cast<uint32>(std::size(Parameters));
    RootSignatureDesc.Desc_1_1.pParameters = Parameters;
    RootSignatureDesc.Desc_1_1.NumStaticSamplers = 0;
    RootSignatureDesc.Desc_1_1.pStaticSamplers = nullptr;
    RootSignatureDesc.Desc_1_1.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED |
        D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

    TComPtr<ID3DBlob> SignatureBlob;
    TComPtr<ID3DBlob> ErrorBlob;
    HRESULT NativeResult = D3D12SerializeVersionedRootSignature(&RootSignatureDesc, &SignatureBlob, &ErrorBlob);
    if (FAILED(NativeResult))
    {
        FString Message = "Failed to serialize global root signature";
        if (ErrorBlob)
        {
            Message += ": ";
            Message.append(static_cast<const char*>(ErrorBlob->GetBufferPointer()), ErrorBlob->GetBufferSize());
        }
        return RESULT_PLATFORM_ERROR(NativeResult, Message);
    }

    NativeResult = Device->CreateRootSignature(0, SignatureBlob->GetBufferPointer(), SignatureBlob->GetBufferSize(), IID_PPV_ARGS(&GlobalRootSignature));
    if (FAILED(NativeResult))
    {
        return RESULT_PLATFORM_ERROR(NativeResult, "Failed to create global root signature");
    }
    GlobalRootSignature->SetName(L"GlobalRootSignature");
    return RESULT_OK(FResult());
}

FD3D12QueueFenceValues FD3D12GfxDevice::CaptureLastSubmittedValues() const
{
    FD3D12QueueFenceValues Result;
    for (uint8 Index = 0; Index < static_cast<uint8>(EGfxQueueType::Count); ++Index)
    {
        Result.Values[Index] = Queues[Index].GetLastSubmittedValue();
    }
    return Result;
}

void FD3D12GfxDevice::WaitForFenceValues(const FD3D12QueueFenceValues& FenceValues)
{
    for (uint8 Index = 0; Index < static_cast<uint8>(EGfxQueueType::Count); ++Index)
    {
        Queues[Index].CpuWait(FenceValues.Values[Index]);
    }
}

void FD3D12GfxDevice::ReportAndReleaseLeakedResources()
{
    const uint32 LeakedBuffers = Buffers.GetAliveCount();
    const uint32 LeakedTextures = Textures.GetAliveCount();
    const uint32 LeakedTextureViews = TextureViews.GetAliveCount();
    if (LeakedBuffers + LeakedTextures + LeakedTextureViews > 0)
    {
        std::println("Gfx device shut down with {} buffers, {} textures and {} texture views still alive", LeakedBuffers, LeakedTextures, LeakedTextureViews);
    }
    Pipelines.Clear();
    Shaders.Clear();
    Samplers.Clear();
    TextureViews.Clear();
    Textures.Clear();
    Buffers.Clear();
}

FResult FD3D12GfxDevice::CreateBufferViews(FD3D12Buffer& Buffer)
{
    const FGfxBufferDesc& Desc = Buffer.Desc;
    const bool8 bIsRaw = Desc.Stride == 0;
    const uint64 ElementSize = bIsRaw ? 4 : Desc.Stride;
    const uint64 ElementCount = Desc.Size / ElementSize;
    if (ElementCount > UINT32_MAX)
    {
        return RESULT_ERROR("Buffer has too many elements for a view");
    }
    FD3D12DescriptorHeap& ResourceHeap = GetDescriptorHeap(ED3D12DescriptorHeapType::Resource);

    if (HasAnyFlags(Desc.Usage, EGfxBufferUsage::ShaderResource))
    {
        RESULT_CHECK_ASSIGN(ResourceHeap.Allocate(), Srv)

        D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
        SrvDesc.Format = bIsRaw ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_UNKNOWN;
        SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        SrvDesc.Buffer.FirstElement = 0;
        SrvDesc.Buffer.NumElements = static_cast<uint32>(ElementCount);
        SrvDesc.Buffer.StructureByteStride = bIsRaw ? 0 : Desc.Stride;
        SrvDesc.Buffer.Flags = bIsRaw ? D3D12_BUFFER_SRV_FLAG_RAW : D3D12_BUFFER_SRV_FLAG_NONE;
        Device->CreateShaderResourceView(Buffer.Resource.Get(), &SrvDesc, Srv.CpuHandle);
        Buffer.SrvIndex = Srv.Index;
    }

    if (HasAnyFlags(Desc.Usage, EGfxBufferUsage::UnorderedAccess))
    {
        TResult<FD3D12Descriptor> Uav = ResourceHeap.Allocate();
        if (Uav.HasError())
        {
            ResourceHeap.Free(Buffer.SrvIndex);
            Buffer.SrvIndex = GfxInvalidBindlessIndex;
            return std::unexpected(Uav.GetError());
        }

        D3D12_UNORDERED_ACCESS_VIEW_DESC UavDesc = {};
        UavDesc.Format = bIsRaw ? DXGI_FORMAT_R32_TYPELESS : DXGI_FORMAT_UNKNOWN;
        UavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        UavDesc.Buffer.FirstElement = 0;
        UavDesc.Buffer.NumElements = static_cast<uint32>(ElementCount);
        UavDesc.Buffer.StructureByteStride = bIsRaw ? 0 : Desc.Stride;
        UavDesc.Buffer.CounterOffsetInBytes = 0;
        UavDesc.Buffer.Flags = bIsRaw ? D3D12_BUFFER_UAV_FLAG_RAW : D3D12_BUFFER_UAV_FLAG_NONE;
        Device->CreateUnorderedAccessView(Buffer.Resource.Get(), nullptr, &UavDesc, Uav.GetValue().CpuHandle);
        Buffer.UavIndex = Uav.GetValue().Index;
    }
    return RESULT_OK_EMPTY();
}

FResult FD3D12GfxDevice::CreateTextureDefaultViews(FD3D12Texture& Texture)
{
    const FGfxTextureDesc& Desc = Texture.Desc;
    const FD3D12FormatInfo FormatInfo = GetD3D12FormatInfo(Desc.Format);
    FD3D12DescriptorHeap& ResourceHeap = GetDescriptorHeap(ED3D12DescriptorHeapType::Resource);

    RESULT_CHECK_ASSIGN(ResolveSubresourceRange(Desc, FGfxSubresourceRange::All()), FullRange)

    if (HasAnyFlags(Desc.Usage, EGfxTextureUsage::ShaderResource))
    {
        RESULT_CHECK_ASSIGN(ResourceHeap.Allocate(), Srv)
        const D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = MakeTextureSrvDesc(Desc, EGfxTextureViewDimension::Inherit, FormatInfo.ViewFormat, FullRange, 0);
        Device->CreateShaderResourceView(Texture.Resource.Get(), &SrvDesc, Srv.CpuHandle);
        Texture.SrvIndex = Srv.Index;
    }

    if (HasAnyFlags(Desc.Usage, EGfxTextureUsage::UnorderedAccess))
    {
        TResult<FD3D12Descriptor> Uav = ResourceHeap.Allocate();
        if (Uav.HasError())
        {
            ResourceHeap.Free(Texture.SrvIndex);
            Texture.SrvIndex = GfxInvalidBindlessIndex;
            return std::unexpected(Uav.GetError());
        }
        FGfxSubresourceRange Mip0 = FullRange;
        Mip0.MipCount = 1;
        const D3D12_UNORDERED_ACCESS_VIEW_DESC UavDesc = MakeTextureUavDesc(Desc, EGfxTextureViewDimension::Inherit, FormatInfo.ViewFormat, Mip0, 0);
        Device->CreateUnorderedAccessView(Texture.Resource.Get(), nullptr, &UavDesc, Uav.GetValue().CpuHandle);
        Texture.UavIndex = Uav.GetValue().Index;
    }
    return RESULT_OK_EMPTY();
}

void FD3D12GfxDevice::FreeDescriptorsNow(const uint32 SrvIndex, const uint32 UavIndex)
{
    FD3D12DescriptorHeap& ResourceHeap = GetDescriptorHeap(ED3D12DescriptorHeapType::Resource);
    ResourceHeap.Free(SrvIndex);
    ResourceHeap.Free(UavIndex);
}
