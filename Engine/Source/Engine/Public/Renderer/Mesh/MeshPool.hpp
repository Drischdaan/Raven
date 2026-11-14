// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Graphics/GfxResourcePool.hpp"
#include "Graphics/GfxTypes.hpp"
#include "Renderer/GpuTypes.hpp"
#include "Renderer/RendererTypes.hpp"
#include "Renderer/Mesh/MeshData.hpp"
#include "Renderer/Utility/RangeAllocator.hpp"
#include "Core/Memory/Containers/String.hpp"
#include "Core/Utility/Result.hpp"

class IGfxDevice;
class FUploadManager;

struct FMeshPoolDesc
{
    uint32 MaxVertices = 1u << 20;
    uint32 MaxIndices = 1u << 22;
};

class FMeshPool
{
public:
    static constexpr uint32 MaxMeshes = 1u << 16;

    struct FMesh
    {
        FGpuMesh Gpu;
        FString Name;
    };

public:
    FMeshPool() = default;
    FMeshPool(const FMeshPool&) = delete;
    FMeshPool& operator=(const FMeshPool&) = delete;
    ~FMeshPool();

public:
    FResult Initialize(IGfxDevice& InDevice, FUploadManager& InUploads, const FMeshPoolDesc& Desc);
    void Deinitialize();

    TResult<FMeshHandle> CreateMesh(const FMeshData& Data, FStringView Name);
    void DestroyMesh(FMeshHandle Handle);

    [[nodiscard]] const FMesh* GetMesh(const FMeshHandle Handle) const
    {
        return Meshes.Get(Handle);
    }

    [[nodiscard]] FGfxBufferHandle GetPositionBuffer() const
    {
        return PositionBuffer;
    }

    [[nodiscard]] FGfxBufferHandle GetAttributeBuffer() const
    {
        return AttributeBuffer;
    }

    [[nodiscard]] FGfxBufferHandle GetIndexBuffer() const
    {
        return IndexBuffer;
    }

    [[nodiscard]] FGfxBufferHandle GetMeshBuffer() const
    {
        return MeshBuffer;
    }

private:
    IGfxDevice* Device = nullptr;
    FUploadManager* Uploads = nullptr;

    FGfxBufferHandle PositionBuffer;
    FGfxBufferHandle AttributeBuffer;
    FGfxBufferHandle IndexBuffer;
    FGfxBufferHandle MeshBuffer;

    FRangeAllocator VertexRanges;
    FRangeAllocator IndexRanges;
    TGfxResourcePool<FMeshHandle, FMesh, MaxMeshes> Meshes;
};
