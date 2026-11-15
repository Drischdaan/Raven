// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Renderer/RendererTypes.hpp"
#include "Core/Math/Matrix4x4.hpp"
#include "Core/Memory/Containers/Span.hpp"
#include "Core/Memory/Containers/Vector.hpp"
#include "Core/Utility/Result.hpp"

struct FInstanceDesc
{
    FMeshHandle Mesh;
    FMaterialHandle Material;
    FMatrix4x4 Transform = FMatrix4x4::Identity();
};

class RAVEN_API FRenderScene
{
public:
    static constexpr uint32 DefaultMaxInstances = 1u << 16;

    struct FInstance
    {
        FInstanceId Id;
        FMeshHandle Mesh;
        FMaterialHandle Material;
        FMatrix4x4 Transform;
    };

public:
    explicit FRenderScene(uint32 InMaxInstances = DefaultMaxInstances);

public:
    TResult<FInstanceId> AddInstance(const FInstanceDesc& Desc);
    void RemoveInstance(FInstanceId Id);
    void SetTransform(FInstanceId Id, const FMatrix4x4& Transform);
    void SetMaterial(FInstanceId Id, FMaterialHandle Material);
    [[nodiscard]] bool8 IsValid(FInstanceId Id) const;

    [[nodiscard]] TSpan<const FInstance> GetInstances() const
    {
        return Instances;
    }

    [[nodiscard]] uint32 GetMaxInstances() const
    {
        return MaxInstances;
    }

    [[nodiscard]] TSpan<const uint32> GetDirtyInstances();
    void ClearDirty();

private:
    struct FSlot
    {
        uint32 DenseIndex = UINT32_MAX;
        uint32 Generation = 0;
    };

    [[nodiscard]] FInstance* Find(FInstanceId Id);
    void MarkDirty(uint32 DenseIndex);

private:
    uint32 MaxInstances = 0;
    TVector<FInstance> Instances;
    TVector<FSlot> Slots;
    TVector<uint32> FreeSlots;
    TVector<uint32> DirtyInstances;
    TVector<bool8> DirtyFlags;
    bool8 bDirtySorted = true;
};
