// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Scene/GpuScene.hpp"

#include <algorithm>

#include "Graphics/GfxDevice.hpp"
#include "Renderer/Material/MaterialSystem.hpp"
#include "Renderer/Mesh/MeshPool.hpp"
#include "Renderer/Scene/RenderScene.hpp"
#include "Renderer/Utility/UploadManager.hpp"
#include "Core/Math/Matrix3x3.hpp"

namespace
{
    FGpuInstance MakeGpuInstance(const FRenderScene::FInstance& Instance, const FMeshPool& MeshPool, const FMaterialSystem& Materials)
    {
        const FMatrix3x3 Normal = FMatrix3x3::MakeNormalMatrix(Instance.Transform);
        const FMeshPool::FMesh* Mesh = MeshPool.GetMesh(Instance.Mesh);

        FGpuInstance Result;
        Result.LocalToWorld = Instance.Transform;
        Result.PrevLocalToWorld = Instance.Transform;
        for (uint32 Column = 0; Column < 3; ++Column)
        {
            Result.NormalMatrix[Column] = {.X = Normal.At(0, Column), .Y = Normal.At(1, Column), .Z = Normal.At(2, Column), .W = 0.0f};
        }
        Result.MeshIndex = Mesh != nullptr ? Instance.Mesh.Index : InvalidGpuIndex;
        Result.MaterialIndex = Materials.GetGpuIndex(Instance.Material);
        return Result;
    }
}

FGpuScene::~FGpuScene()
{
    Deinitialize();
}

FResult FGpuScene::Initialize(IGfxDevice& InDevice, FUploadManager& InUploads, const uint32 InMaxInstances)
{
    Device = &InDevice;
    Uploads = &InUploads;
    MaxInstances = InMaxInstances;

    RESULT_CHECK_ASSIGN(Device->CreateBuffer({
                            .Size = static_cast<uint64>(MaxInstances) * sizeof(FGpuInstance),
                            .Stride = sizeof(FGpuInstance),
                            .Usage = EGfxBufferUsage::ShaderResource | EGfxBufferUsage::CopyDestination,
                            }, "GpuScene.Instances"), Buffer);
    InstanceBuffer = Buffer;
    return RESULT_OK_EMPTY();
}

void FGpuScene::Deinitialize()
{
    if (Device == nullptr)
    {
        return;
    }
    Device->DestroyBuffer(InstanceBuffer);
    InstanceBuffer = {};
    Device = nullptr;
    Uploads = nullptr;
}

void FGpuScene::Update(FRenderScene& Scene, const FMeshPool& MeshPool, const FMaterialSystem& Materials)
{
    const TSpan<const uint32> Dirty = Scene.GetDirtyInstances();
    const TSpan<const FRenderScene::FInstance> Instances = Scene.GetInstances();

    for (size64 RunStart = 0; RunStart < Dirty.size();)
    {
        size64 RunEnd = RunStart + 1;
        while (RunEnd < Dirty.size() && Dirty[RunEnd] == Dirty[RunEnd - 1] + 1)
        {
            ++RunEnd;
        }

        Scratch.clear();
        for (size64 Index = RunStart; Index < RunEnd; ++Index)
        {
            Scratch.push_back(MakeGpuInstance(Instances[Dirty[Index]], MeshPool, Materials));
        }
        Uploads->UploadBuffer(InstanceBuffer, static_cast<uint64>(Dirty[RunStart]) * sizeof(FGpuInstance), TSpan<const FGpuInstance>(Scratch));
        RunStart = RunEnd;
    }
    Scene.ClearDirty();
}
