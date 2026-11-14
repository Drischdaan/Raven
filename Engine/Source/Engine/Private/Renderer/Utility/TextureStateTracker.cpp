// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Utility/TextureStateTracker.hpp"

#include <algorithm>

#include "Graphics/GfxCommandList.hpp"
#include "Renderer/RendererLog.hpp"

namespace
{
    bool8 IsWrite(const EGfxBarrierAccess Access)
    {
        return HasAnyFlags(Access, EGfxBarrierAccess::RenderTarget | EGfxBarrierAccess::UnorderedAccess | EGfxBarrierAccess::DepthStencilWrite | EGfxBarrierAccess::CopyDestination);
    }

    bool8 NeedsBarrier(const FTextureState& Previous, const FTextureState& Next)
    {
        if (Previous.Layout != Next.Layout)
        {
            return true;
        }
        if ((Previous.Access == EGfxBarrierAccess::RenderTarget && Next.Access == EGfxBarrierAccess::RenderTarget)
            || (Previous.Access == EGfxBarrierAccess::DepthStencilWrite && Next.Access == EGfxBarrierAccess::DepthStencilWrite))
        {
            return false;
        }
        return IsWrite(Previous.Access) || IsWrite(Next.Access);
    }
}

FTextureStateTracker::FEntry* FTextureStateTracker::Find(const FGfxTextureHandle Texture)
{
    const auto It = std::ranges::find(Entries, Texture, &FEntry::Texture);
    return It != Entries.end() ? &*It : nullptr;
}

void FTextureStateTracker::Register(const FGfxTextureHandle Texture, const FTextureState& State)
{
    if (FEntry* Entry = Find(Texture); Entry != nullptr)
    {
        Entry->State = State;
        return;
    }
    Entries.push_back({.Texture = Texture, .State = State});
}

void FTextureStateTracker::Unregister(const FGfxTextureHandle Texture)
{
    std::erase_if(Entries, [Texture](const FEntry& Entry) { return Entry.Texture == Texture; });
}

void FTextureStateTracker::Transition(const FGfxTextureHandle Texture, const FTextureState& Next)
{
    FEntry* Entry = Find(Texture);
    if (Entry == nullptr)
    {
        RAVEN_LOG(LogRenderer, Error, "TextureStateTracker: transition of an unregistered texture");
        return;
    }

    const FTextureState& Previous = Entry->State;
    if (!NeedsBarrier(Previous, Next))
    {
        Entry->State.Sync |= Next.Sync;
        Entry->State.Access |= Next.Access;
        return;
    }

    const bool8 bFromUndefined = Previous.Layout == EGfxTextureLayout::Undefined;
    PendingBarriers.push_back({
        .TextureHandle = Texture,
        .SyncBefore = Previous.Sync,
        .SyncAfter = Next.Sync,
        .AccessBefore = Previous.Access,
        .AccessAfter = Next.Access,
        .LayoutBefore = Previous.Layout,
        .LayoutAfter = Next.Layout,
        .bDiscard = bFromUndefined && (Next.Layout == EGfxTextureLayout::RenderTarget || Next.Layout == EGfxTextureLayout::DepthStencilWrite),
    });
    Entry->State = Next;
}

void FTextureStateTracker::Flush(IGfxCommandList& CommandList)
{
    if (PendingBarriers.empty())
    {
        return;
    }
    CommandList.Barrier({}, {}, PendingBarriers);
    PendingBarriers.clear();
}

const FTextureState* FTextureStateTracker::GetState(const FGfxTextureHandle Texture) const
{
    const auto It = std::ranges::find(Entries, Texture, &FEntry::Texture);
    return It != Entries.end() ? &It->State : nullptr;
}
