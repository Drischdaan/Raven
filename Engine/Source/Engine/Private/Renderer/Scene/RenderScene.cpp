// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Scene/RenderScene.hpp"

#include <algorithm>

#include "Renderer/RendererLog.hpp"

FRenderScene::FRenderScene(const uint32 InMaxInstances)
    : MaxInstances(InMaxInstances)
{
}

TResult<FInstanceId> FRenderScene::AddInstance(const FInstanceDesc& Desc)
{
    if (Instances.size() >= MaxInstances)
    {
        return RESULT_ERROR(String::Format("Render scene is full ({} instances)", MaxInstances));
    }

    uint32 SlotIndex;
    if (!FreeSlots.empty())
    {
        SlotIndex = FreeSlots.back();
        FreeSlots.pop_back();
    }
    else
    {
        SlotIndex = static_cast<uint32>(Slots.size());
        Slots.push_back({});
    }

    FSlot& Slot = Slots[SlotIndex];
    Slot.DenseIndex = static_cast<uint32>(Instances.size());
    ++Slot.Generation;

    const FInstanceId Id = {.Index = SlotIndex, .Generation = Slot.Generation};
    Instances.push_back({.Id = Id, .Mesh = Desc.Mesh, .Transform = Desc.Transform, .Color = Desc.Color});
    DirtyFlags.push_back(false);
    MarkDirty(Slot.DenseIndex);
    return Id;
}

void FRenderScene::RemoveInstance(const FInstanceId Id)
{
    if (Find(Id) == nullptr)
    {
        RAVEN_LOG(LogRenderer, Error, "RemoveInstance: invalid or stale instance id");
        return;
    }

    const uint32 DenseIndex = Slots[Id.Index].DenseIndex;
    const uint32 LastIndex = static_cast<uint32>(Instances.size()) - 1;
    if (DenseIndex != LastIndex)
    {
        Instances[DenseIndex] = Instances[LastIndex];
        Slots[Instances[DenseIndex].Id.Index].DenseIndex = DenseIndex;
        MarkDirty(DenseIndex);
    }
    Instances.pop_back();

    if (DirtyFlags[LastIndex])
    {
        std::erase(DirtyInstances, LastIndex);
    }
    DirtyFlags.pop_back();

    Slots[Id.Index].DenseIndex = UINT32_MAX;
    ++Slots[Id.Index].Generation;
    FreeSlots.push_back(Id.Index);
}

void FRenderScene::SetTransform(const FInstanceId Id, const FMatrix4x4& Transform)
{
    if (FInstance* Instance = Find(Id); Instance != nullptr)
    {
        Instance->Transform = Transform;
        MarkDirty(Slots[Id.Index].DenseIndex);
    }
}

void FRenderScene::SetColor(const FInstanceId Id, const FLinearColor& Color)
{
    if (FInstance* Instance = Find(Id); Instance != nullptr)
    {
        Instance->Color = Color;
        MarkDirty(Slots[Id.Index].DenseIndex);
    }
}

bool8 FRenderScene::IsValid(const FInstanceId Id) const
{
    return Id.Index < Slots.size() && Slots[Id.Index].Generation == Id.Generation && Slots[Id.Index].DenseIndex != UINT32_MAX;
}

FRenderScene::FInstance* FRenderScene::Find(const FInstanceId Id)
{
    return IsValid(Id) ? &Instances[Slots[Id.Index].DenseIndex] : nullptr;
}

void FRenderScene::MarkDirty(const uint32 DenseIndex)
{
    if (DirtyFlags[DenseIndex])
    {
        return;
    }
    DirtyFlags[DenseIndex] = true;
    if (!DirtyInstances.empty() && DirtyInstances.back() > DenseIndex)
    {
        bDirtySorted = false;
    }
    DirtyInstances.push_back(DenseIndex);
}

TSpan<const uint32> FRenderScene::GetDirtyInstances()
{
    if (!bDirtySorted)
    {
        std::ranges::sort(DirtyInstances);
        bDirtySorted = true;
    }
    return DirtyInstances;
}

void FRenderScene::ClearDirty()
{
    for (const uint32 DenseIndex : DirtyInstances)
    {
        DirtyFlags[DenseIndex] = false;
    }
    DirtyInstances.clear();
    bDirtySorted = true;
}
