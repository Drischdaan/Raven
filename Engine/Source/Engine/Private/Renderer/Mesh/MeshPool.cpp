// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Mesh/MeshPool.hpp"

#include <algorithm>
#include <cmath>

#include "Graphics/GfxDevice.hpp"
#include "Renderer/RendererLog.hpp"
#include "Renderer/Utility/UploadManager.hpp"
#include "Core/Math/MathUtility.hpp"

namespace
{
    FVector2 OctahedralEncode(const FVector3& Normal)
    {
        const float32 Sum = Math::Abs(Normal.X) + Math::Abs(Normal.Y) + Math::Abs(Normal.Z);
        FVector2 Result = {.X = Normal.X / Sum, .Y = Normal.Y / Sum};
        if (Normal.Z < 0.0f)
        {
            const float32 X = (1.0f - Math::Abs(Result.Y)) * (Result.X >= 0.0f ? 1.0f : -1.0f);
            const float32 Y = (1.0f - Math::Abs(Result.X)) * (Result.Y >= 0.0f ? 1.0f : -1.0f);
            Result = {.X = X, .Y = Y};
        }
        return Result;
    }

    uint32 PackSnorm16(const float32 Value)
    {
        const float32 Clamped = Math::Clamp(Value, -1.0f, 1.0f);
        const int32 Quantized = static_cast<int32>(std::lround(Clamped * 32767.0f));
        return static_cast<uint16>(static_cast<int16>(Quantized));
    }

    uint32 PackOctahedral(const FVector3& Vector)
    {
        const FVector2 Encoded = OctahedralEncode(Vector);
        return PackSnorm16(Encoded.X) | (PackSnorm16(Encoded.Y) << 16);
    }

    uint32 PackTangent(const FVector4& Tangent)
    {
        const uint32 Packed = PackOctahedral(Tangent.GetXYZ().GetSafeNormal());
        return (Packed & ~0x10000u) | (Tangent.W < 0.0f ? 0x10000u : 0u);
    }

    FVector4 MakeFallbackTangent(const FVector3& Normal)
    {
        const FVector3 Reference = Math::Abs(Normal.Y) < 0.999f ? FVector3::UnitY() : FVector3::UnitX();
        return FVector4::FromVector3(FVector3::Cross(Reference, Normal).GetSafeNormal(), 1.0f);
    }

    FResult ValidateMeshData(const FMeshData& Data, const FStringView Name)
    {
        const size64 VertexCount = Data.Positions.size();
        if (VertexCount == 0 || Data.Indices.empty())
        {
            return RESULT_ERROR(String::Format("Mesh '{}' has no vertices or no indices", Name));
        }
        if (Data.Normals.size() != VertexCount)
        {
            return RESULT_ERROR(String::Format("Mesh '{}' needs one normal per position ({} vs {})", Name, Data.Normals.size(), VertexCount));
        }
        if ((!Data.Tangents.empty() && Data.Tangents.size() != VertexCount) || (!Data.TexCoords.empty() && Data.TexCoords.size() != VertexCount))
        {
            return RESULT_ERROR(String::Format("Mesh '{}' has tangent or texcoord counts that don't match the positions", Name));
        }
        if (Data.Indices.size() % 3 != 0)
        {
            return RESULT_ERROR(String::Format("Mesh '{}' index count {} isn't a multiple of 3", Name, Data.Indices.size()));
        }
        if (const auto MaxIndex = std::ranges::max(Data.Indices); MaxIndex >= VertexCount)
        {
            return RESULT_ERROR(String::Format("Mesh '{}' references vertex {}, but has only {}", Name, MaxIndex, VertexCount));
        }
        return RESULT_OK_EMPTY();
    }
}

FMeshPool::~FMeshPool()
{
    Deinitialize();
}

FResult FMeshPool::Initialize(IGfxDevice& InDevice, FUploadManager& InUploads, const FMeshPoolDesc& Desc)
{
    Device = &InDevice;
    Uploads = &InUploads;

    RESULT_CHECK_ASSIGN(Device->CreateBuffer({
                            .Size = static_cast<uint64>(Desc.MaxVertices) * sizeof(FGpuVertexPosition),
                            .Stride = sizeof(FGpuVertexPosition),
                            .Usage = EGfxBufferUsage::ShaderResource | EGfxBufferUsage::CopyDestination,
                            }, "MeshPool.Positions"), Positions);
    PositionBuffer = Positions;

    RESULT_CHECK_ASSIGN(Device->CreateBuffer({
                            .Size = static_cast<uint64>(Desc.MaxVertices) * sizeof(FGpuVertexAttributes),
                            .Stride = sizeof(FGpuVertexAttributes),
                            .Usage = EGfxBufferUsage::ShaderResource | EGfxBufferUsage::CopyDestination,
                            }, "MeshPool.Attributes"), Attributes);
    AttributeBuffer = Attributes;

    RESULT_CHECK_ASSIGN(Device->CreateBuffer({
                            .Size = static_cast<uint64>(Desc.MaxIndices) * sizeof(uint32),
                            .Stride = sizeof(uint32),
                            .Usage = EGfxBufferUsage::Index | EGfxBufferUsage::ShaderResource | EGfxBufferUsage::CopyDestination,
                            }, "MeshPool.Indices"), Indices);
    IndexBuffer = Indices;

    RESULT_CHECK_ASSIGN(Device->CreateBuffer({
                            .Size = static_cast<uint64>(MaxMeshes) * sizeof(FGpuMesh),
                            .Stride = sizeof(FGpuMesh),
                            .Usage = EGfxBufferUsage::ShaderResource | EGfxBufferUsage::CopyDestination,
                            }, "MeshPool.Meshes"), MeshTable);
    MeshBuffer = MeshTable;

    VertexRanges.Initialize(Desc.MaxVertices);
    IndexRanges.Initialize(Desc.MaxIndices);
    return RESULT_OK_EMPTY();
}

void FMeshPool::Deinitialize()
{
    if (Device == nullptr)
    {
        return;
    }
    Device->DestroyBuffer(PositionBuffer);
    Device->DestroyBuffer(AttributeBuffer);
    Device->DestroyBuffer(IndexBuffer);
    Device->DestroyBuffer(MeshBuffer);
    Meshes.Clear();
    Device = nullptr;
    Uploads = nullptr;
}

TResult<FMeshHandle> FMeshPool::CreateMesh(const FMeshData& Data, const FStringView Name)
{
    RESULT_CHECK(ValidateMeshData(Data, Name));

    const uint32 VertexCount = static_cast<uint32>(Data.Positions.size());
    const uint32 IndexCount = static_cast<uint32>(Data.Indices.size());
    const std::optional<uint64> VertexOffset = VertexRanges.Allocate(VertexCount);
    if (!VertexOffset.has_value())
    {
        return RESULT_ERROR(String::Format("Mesh pool is out of vertex space ({} of {} used, '{}' needs {})", VertexRanges.GetUsedSize(), VertexRanges.GetCapacity(), Name, VertexCount));
    }
    const std::optional<uint64> IndexOffset = IndexRanges.Allocate(IndexCount);
    if (!IndexOffset.has_value())
    {
        VertexRanges.Free(*VertexOffset, VertexCount);
        return RESULT_ERROR(String::Format("Mesh pool is out of index space ({} of {} used, '{}' needs {})", IndexRanges.GetUsedSize(), IndexRanges.GetCapacity(), Name, IndexCount));
    }

    TVector<FGpuVertexAttributes> PackedAttributes(VertexCount);
    FVector3 Min = Data.Positions[0];
    FVector3 Max = Data.Positions[0];
    for (uint32 Index = 0; Index < VertexCount; ++Index)
    {
        const FVector3 Normal = Data.Normals[Index].GetSafeNormal();
        const FVector4 Tangent = Data.Tangents.empty() ? MakeFallbackTangent(Normal) : Data.Tangents[Index];
        const FVector2 TexCoord = Data.TexCoords.empty() ? FVector2{} : Data.TexCoords[Index];

        PackedAttributes[Index] = {
            .Normal = PackOctahedral(Normal),
            .Tangent = PackTangent(Tangent),
            .TexCoord = {TexCoord.X, TexCoord.Y},
        };
        Min = FVector3::Min(Min, Data.Positions[Index]);
        Max = FVector3::Max(Max, Data.Positions[Index]);
    }

    const FVector3 Center = (Min + Max) * 0.5f;
    const FVector3 Extent = (Max - Min) * 0.5f;
    const FGpuMesh GpuMesh = {
        .VertexOffset = static_cast<uint32>(*VertexOffset),
        .VertexCount = VertexCount,
        .IndexOffset = static_cast<uint32>(*IndexOffset),
        .IndexCount = IndexCount,
        .BoundsCenterRadius = FVector4::FromVector3(Center, Extent.Length()),
        .BoundsExtent = FVector4::FromVector3(Extent, 0.0f),
    };

    TResult<FMeshHandle> Allocated = Meshes.Allocate(FMesh{.Gpu = GpuMesh, .Name = FString(Name)});
    if (Allocated.HasError())
    {
        VertexRanges.Free(*VertexOffset, VertexCount);
        IndexRanges.Free(*IndexOffset, IndexCount);
        return std::unexpected(std::move(Allocated).error());
    }
    const FMeshHandle Handle = Allocated.GetValue();

    Uploads->UploadBuffer(PositionBuffer, *VertexOffset * sizeof(FGpuVertexPosition), Data.Positions);
    Uploads->UploadBuffer(AttributeBuffer, *VertexOffset * sizeof(FGpuVertexAttributes), TSpan<const FGpuVertexAttributes>(PackedAttributes));
    Uploads->UploadBuffer(IndexBuffer, *IndexOffset * sizeof(uint32), Data.Indices);
    Uploads->UploadBuffer(MeshBuffer, Handle.Index * sizeof(FGpuMesh), TSpan<const FGpuMesh>(&GpuMesh, 1));

    RAVEN_LOG(LogRenderer, Verbose, "Created mesh '{}': {} vertices, {} triangles", Name, VertexCount, IndexCount / 3);
    return Handle;
}

void FMeshPool::DestroyMesh(const FMeshHandle Handle)
{
    TResult<FMesh> Removed = Meshes.Remove(Handle);
    if (Removed.HasError())
    {
        RAVEN_LOG(LogRenderer, Error, "DestroyMesh: {}", Removed.GetError().Message);
        return;
    }
    const FGpuMesh& Mesh = Removed.GetValue().Gpu;
    VertexRanges.Free(Mesh.VertexOffset, Mesh.VertexCount);
    IndexRanges.Free(Mesh.IndexOffset, Mesh.IndexCount);
}
