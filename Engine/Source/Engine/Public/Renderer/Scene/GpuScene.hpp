// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Graphics/GfxTypes.hpp"
#include "Renderer/GpuTypes.hpp"
#include "Core/Memory/Containers/Vector.hpp"
#include "Core/Utility/Result.hpp"

class IGfxDevice;
class FUploadManager;
class FMeshPool;
class FMaterialSystem;
class FRenderScene;

class FGpuScene
{
public:
    FGpuScene() = default;
    FGpuScene(const FGpuScene&) = delete;
    FGpuScene& operator=(const FGpuScene&) = delete;
    ~FGpuScene();

public:
    FResult Initialize(IGfxDevice& InDevice, FUploadManager& InUploads, uint32 InMaxInstances);
    void Deinitialize();

    void Update(FRenderScene& Scene, const FMeshPool& MeshPool, const FMaterialSystem& Materials);

    [[nodiscard]] FGfxBufferHandle GetInstanceBuffer() const
    {
        return InstanceBuffer;
    }

private:
    IGfxDevice* Device = nullptr;
    FUploadManager* Uploads = nullptr;
    FGfxBufferHandle InstanceBuffer;
    uint32 MaxInstances = 0;
    TVector<FGpuInstance> Scratch;
};
