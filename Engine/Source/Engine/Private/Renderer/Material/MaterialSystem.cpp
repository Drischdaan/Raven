// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Material/MaterialSystem.hpp"

#include "Graphics/GfxDevice.hpp"
#include "Renderer/RendererLog.hpp"
#include "Renderer/Texture/TexturePool.hpp"
#include "Renderer/Utility/UploadManager.hpp"

FMaterialSystem::~FMaterialSystem()
{
    Deinitialize();
}

FResult FMaterialSystem::Initialize(IGfxDevice& InDevice, FUploadManager& InUploads, const FTexturePool& InTextures)
{
    Device = &InDevice;
    Uploads = &InUploads;
    Textures = &InTextures;

    RESULT_CHECK_ASSIGN(Device->CreateBuffer({
                            .Size = static_cast<uint64>(MaxMaterials) * sizeof(FGpuMaterial),
                            .Stride = sizeof(FGpuMaterial),
                            .Usage = EGfxBufferUsage::ShaderResource | EGfxBufferUsage::CopyDestination,
                            }, "Materials"), Buffer);
    MaterialBuffer = Buffer;

    RESULT_CHECK_ASSIGN(CreateMaterial({
                            .BaseColor = {.R = 0.8f, .G = 0.8f, .B = 0.8f, .A = 1.0f},
                            .Roughness = 0.8f,
                            }, "Default"), Default);
    DefaultMaterial = Default;
    return RESULT_OK_EMPTY();
}

void FMaterialSystem::Deinitialize()
{
    if (Device == nullptr)
    {
        return;
    }
    Device->DestroyBuffer(MaterialBuffer);
    MaterialBuffer = {};
    Materials.Clear();
    DefaultMaterial = {};
    Device = nullptr;
    Uploads = nullptr;
    Textures = nullptr;
}

FGpuMaterial FMaterialSystem::MakeGpuMaterial(const FMaterialDesc& Desc) const
{
    uint32 Flags = 0;
    if (HasAnyFlags(Desc.Flags, EMaterialFlags::TwoSided))
    {
        Flags |= GpuMaterialFlags::TwoSided;
    }
    if (HasAnyFlags(Desc.Flags, EMaterialFlags::Unlit))
    {
        Flags |= GpuMaterialFlags::Unlit;
    }
    if (Desc.BlendMode == EBlendMode::Masked)
    {
        Flags |= GpuMaterialFlags::AlphaMasked;
    }

    return {
        .BaseColor = {Desc.BaseColor.R, Desc.BaseColor.G, Desc.BaseColor.B, Desc.BaseColor.A},
        .Emissive = {Desc.Emissive.R, Desc.Emissive.G, Desc.Emissive.B},
        .AlphaCutoff = Desc.AlphaCutoff,
        .Metallic = Desc.Metallic,
        .Roughness = Desc.Roughness,
        .NormalScale = Desc.NormalScale,
        .Flags = Flags,
        .BaseColorTexture = Textures->GetBindlessIndex(Desc.BaseColorTexture, EDefaultTexture::White),
        .NormalTexture = Textures->GetBindlessIndex(Desc.NormalTexture, EDefaultTexture::FlatNormal),
        .MetallicRoughnessTexture = Textures->GetBindlessIndex(Desc.MetallicRoughnessTexture, EDefaultTexture::White),
        .EmissiveTexture = Textures->GetBindlessIndex(Desc.EmissiveTexture, EDefaultTexture::White),
    };
}

TResult<FMaterialHandle> FMaterialSystem::CreateMaterial(const FMaterialDesc& Desc, const FStringView Name)
{
    RESULT_CHECK_ASSIGN(Materials.Allocate(FMaterial{.Desc = Desc, .Name = FString(Name)}), Handle);

    const FGpuMaterial GpuMaterial = MakeGpuMaterial(Desc);
    Uploads->UploadBuffer(MaterialBuffer, Handle.Index * sizeof(FGpuMaterial), TSpan<const FGpuMaterial>(&GpuMaterial, 1));
    return Handle;
}

FResult FMaterialSystem::UpdateMaterial(const FMaterialHandle Handle, const FMaterialDesc& Desc)
{
    FMaterial* Material = Materials.Get(Handle);
    if (Material == nullptr)
    {
        return RESULT_ERROR("UpdateMaterial: invalid or stale material handle");
    }
    Material->Desc = Desc;

    const FGpuMaterial GpuMaterial = MakeGpuMaterial(Desc);
    Uploads->UploadBuffer(MaterialBuffer, Handle.Index * sizeof(FGpuMaterial), TSpan<const FGpuMaterial>(&GpuMaterial, 1));
    return RESULT_OK_EMPTY();
}

void FMaterialSystem::DestroyMaterial(const FMaterialHandle Handle)
{
    if (Handle == DefaultMaterial)
    {
        RAVEN_LOG(LogRenderer, Error, "DestroyMaterial: the default material can't be destroyed");
        return;
    }
    const TResult<FMaterial> Removed = Materials.Remove(Handle);
    if (Removed.HasError())
    {
        RAVEN_LOG(LogRenderer, Error, "DestroyMaterial: {}", Removed.GetError().Message);
    }
}

uint32 FMaterialSystem::GetGpuIndex(const FMaterialHandle Handle) const
{
    return Materials.Contains(Handle) ? Handle.Index : DefaultMaterial.Index;
}
