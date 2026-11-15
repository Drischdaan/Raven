// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Texture/TexturePool.hpp"

#include "Graphics/GfxDevice.hpp"
#include "Renderer/RendererLog.hpp"
#include "Renderer/Texture/TextureGenerators.hpp"
#include "Renderer/Utility/UploadManager.hpp"

namespace
{
    FResult ValidateTextureData(const FTextureData& Data, const FStringView Name)
    {
        if (Data.Mips.empty() || Data.Mips[0].Width == 0 || Data.Mips[0].Height == 0)
        {
            return RESULT_ERROR(String::Format("Texture '{}' has no data", Name));
        }
        for (uint32 Mip = 1; Mip < Data.Mips.size(); ++Mip)
        {
            const uint32 ExpectedWidth = FTextureBuilder::GetMipSize(Data.Mips[0].Width, Mip);
            const uint32 ExpectedHeight = FTextureBuilder::GetMipSize(Data.Mips[0].Height, Mip);
            if (Data.Mips[Mip].Width != ExpectedWidth || Data.Mips[Mip].Height != ExpectedHeight)
            {
                return RESULT_ERROR(String::Format("Texture '{}' mip {} is {}x{}, expected {}x{}", Name, Mip, Data.Mips[Mip].Width,
                                                   Data.Mips[Mip].Height, ExpectedWidth, ExpectedHeight));
            }
        }
        return RESULT_OK_EMPTY();
    }
}

FTexturePool::~FTexturePool()
{
    Deinitialize();
}

FResult FTexturePool::Initialize(IGfxDevice& InDevice, FUploadManager& InUploads)
{
    Device = &InDevice;
    Uploads = &InUploads;

    struct FDefault
    {
        EDefaultTexture Slot;
        FColor Color;
        EGfxFormat Format;
        FStringView Name;
    };
    const FDefault DefaultTextures[] = {
        {EDefaultTexture::White, {.R = 255, .G = 255, .B = 255, .A = 255}, EGfxFormat::RGBA8_UNORM, "Default.White"},
        {EDefaultTexture::Black, {.R = 0, .G = 0, .B = 0, .A = 255}, EGfxFormat::RGBA8_UNORM, "Default.Black"},
        {EDefaultTexture::FlatNormal, {.R = 128, .G = 128, .B = 255, .A = 255}, EGfxFormat::RGBA8_UNORM, "Default.FlatNormal"},
    };
    for (const FDefault& Default : DefaultTextures)
    {
        FTextureBuilder Builder = TextureGenerators::Solid(Default.Color, Default.Format);
        RESULT_CHECK_ASSIGN(CreateTexture(Builder.GetData(), Default.Name), Handle);
        Defaults[static_cast<size64>(Default.Slot)] = Handle;
    }
    return RESULT_OK_EMPTY();
}

void FTexturePool::Deinitialize()
{
    if (Device == nullptr)
    {
        return;
    }
    Textures.ForEachAlive([this](FTextureHandle, const FTexture& Texture)
    {
        Device->DestroyTexture(Texture.GfxTexture);
    });
    Textures.Clear();
    Defaults = {};
    Device = nullptr;
    Uploads = nullptr;
}

TResult<FTextureHandle> FTexturePool::CreateTexture(const FTextureData& Data, const FStringView Name)
{
    RESULT_CHECK(ValidateTextureData(Data, Name));

    const uint32 MipCount = static_cast<uint32>(Data.Mips.size());
    RESULT_CHECK_ASSIGN(Device->CreateTexture({
                            .Width = Data.Mips[0].Width,
                            .Height = Data.Mips[0].Height,
                            .MipLevels = MipCount,
                            .Format = Data.Format,
                            .Usage = EGfxTextureUsage::ShaderResource | EGfxTextureUsage::CopyDestination,
                            }, Name), GfxTexture);

    for (uint32 Mip = 0; Mip < MipCount; ++Mip)
    {
        const FResult Uploaded = Uploads->UploadTexture(GfxTexture, Mip, Data.Mips[Mip].Data);
        if (Uploaded.HasError())
        {
            Device->DestroyTexture(GfxTexture);
            return RESULT_ERROR(String::Format("Texture '{}': {}", Name, Uploaded.GetError().Message));
        }
    }

    TResult<FTextureHandle> Allocated = Textures.Allocate(FTexture{
        .GfxTexture = GfxTexture,
        .BindlessIndex = Device->GetBindlessIndex(GfxTexture),
        .Width = Data.Mips[0].Width,
        .Height = Data.Mips[0].Height,
        .MipCount = MipCount,
        .Format = Data.Format,
        .Name = FString(Name),
    });
    if (Allocated.HasError())
    {
        Device->DestroyTexture(GfxTexture);
        return std::unexpected(std::move(Allocated).error());
    }

    RAVEN_LOG(LogRenderer, Verbose, "Created texture '{}': {}x{}, {} mips", Name, Data.Mips[0].Width, Data.Mips[0].Height, MipCount);
    return Allocated.GetValue();
}

void FTexturePool::DestroyTexture(const FTextureHandle Handle)
{
    for (const FTextureHandle Default : Defaults)
    {
        if (Default == Handle)
        {
            RAVEN_LOG(LogRenderer, Error, "DestroyTexture: default textures can't be destroyed");
            return;
        }
    }

    TResult<FTexture> Removed = Textures.Remove(Handle);
    if (Removed.HasError())
    {
        RAVEN_LOG(LogRenderer, Error, "DestroyTexture: {}", Removed.GetError().Message);
        return;
    }
    Device->DestroyTexture(Removed.GetValue().GfxTexture);
}

uint32 FTexturePool::GetBindlessIndex(const FTextureHandle Handle, const EDefaultTexture Fallback) const
{
    if (const FTexture* Texture = Textures.Get(Handle); Texture != nullptr)
    {
        return Texture->BindlessIndex;
    }
    return Textures.Get(GetDefault(Fallback))->BindlessIndex;
}
