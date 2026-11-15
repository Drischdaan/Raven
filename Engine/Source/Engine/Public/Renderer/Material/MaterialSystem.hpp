// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Material.hpp"
#include "Graphics/GfxResourcePool.hpp"
#include "Graphics/GfxTypes.hpp"
#include "Renderer/GpuTypes.hpp"
#include "Core/Memory/Containers/String.hpp"
#include "Core/Utility/Result.hpp"

class IGfxDevice;
class FUploadManager;
class FTexturePool;

class FMaterialSystem
{
public:
    static constexpr uint32 MaxMaterials = 1u << 14;

    struct FMaterial
    {
        FMaterialDesc Desc;
        FString Name;
    };

public:
    FMaterialSystem() = default;
    FMaterialSystem(const FMaterialSystem&) = delete;
    FMaterialSystem& operator=(const FMaterialSystem&) = delete;
    ~FMaterialSystem();

public:
    FResult Initialize(IGfxDevice& InDevice, FUploadManager& InUploads, const FTexturePool& InTextures);
    void Deinitialize();

    TResult<FMaterialHandle> CreateMaterial(const FMaterialDesc& Desc, FStringView Name);
    FResult UpdateMaterial(FMaterialHandle Handle, const FMaterialDesc& Desc);
    void DestroyMaterial(FMaterialHandle Handle);

    [[nodiscard]] const FMaterial* GetMaterial(FMaterialHandle Handle) const
    {
        return Materials.Get(Handle);
    }

    [[nodiscard]] uint32 GetGpuIndex(FMaterialHandle Handle) const;

    [[nodiscard]] FMaterialHandle GetDefaultMaterial() const
    {
        return DefaultMaterial;
    }

    [[nodiscard]] FGfxBufferHandle GetMaterialBuffer() const
    {
        return MaterialBuffer;
    }

private:
    [[nodiscard]] FGpuMaterial MakeGpuMaterial(const FMaterialDesc& Desc) const;

private:
    IGfxDevice* Device = nullptr;
    FUploadManager* Uploads = nullptr;
    const FTexturePool* Textures = nullptr;

    FGfxBufferHandle MaterialBuffer;
    FMaterialHandle DefaultMaterial;
    TGfxResourcePool<FMaterialHandle, FMaterial, MaxMaterials> Materials;
};
