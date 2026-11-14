// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once
#include "D3D12Include.hpp"
#include "Core/CoreTypes.hpp"
#include "Core/Memory/Containers/Vector.hpp"
#include "Graphics/GfxPipelines.hpp"
#include "Graphics/GfxResources.hpp"

struct RAVEN_API FD3D12Buffer
{
    TComPtr<ID3D12Resource> Resource;
    D3D12_GPU_VIRTUAL_ADDRESS GpuAddress = 0;
    byte8* MappedData = nullptr;
    FGfxBufferDesc Desc = {};

    uint32 SrvIndex = GfxInvalidBindlessIndex;
    uint32 UavIndex = GfxInvalidBindlessIndex;
};

struct RAVEN_API FD3D12Texture
{
    TComPtr<ID3D12Resource> Resource;
    D3D12_RESOURCE_DESC1 NativeDesc = {};
    FGfxTextureDesc Desc = {};
    bool8 bIsSwapchainImage = false;

    uint32 SrvIndex = GfxInvalidBindlessIndex;
    uint32 UavIndex = GfxInvalidBindlessIndex;
};

struct RAVEN_API FD3D12TextureView
{
    FGfxTextureHandle TextureHandle;
    FGfxTextureViewDesc Desc = {};
    uint32 DescriptorIndex = GfxInvalidBindlessIndex;
};

struct RAVEN_API FD3D12Sampler
{
    FGfxSamplerDesc Desc = {};
    uint32 DescriptorIndex = GfxInvalidBindlessIndex;
};

struct RAVEN_API FD3D12Shader
{
    EGfxShaderStage Stage = EGfxShaderStage::Vertex;
    TVector<byte8> Bytecode;
};

struct RAVEN_API FD3D12Pipeline
{
    TComPtr<ID3D12PipelineState> PipelineState;
    bool8 bIsCompute = false;
};
