// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Graphics/GfxBarriers.hpp"
#include "Graphics/GfxTypes.hpp"
#include "Core/Memory/Containers/Vector.hpp"

class IGfxCommandList;

struct FTextureState
{
    EGfxBarrierSync Sync = EGfxBarrierSync::None;
    EGfxBarrierAccess Access = EGfxBarrierAccess::NoAccess;
    EGfxTextureLayout Layout = EGfxTextureLayout::Undefined;
};

namespace TextureStates
{
    using S = EGfxBarrierSync;
    using A = EGfxBarrierAccess;
    using L = EGfxTextureLayout;

    inline constexpr FTextureState Undefined = {S::None, A::NoAccess, L::Undefined};
    inline constexpr FTextureState Present = {S::None, A::NoAccess, L::Present};
    inline constexpr FTextureState RenderTarget = {S::RenderTarget, A::RenderTarget, L::RenderTarget};
    inline constexpr FTextureState DepthWrite = {S::DepthStencil, A::DepthStencilWrite, L::DepthStencilWrite};
    inline constexpr FTextureState DepthRead = {S::DepthStencil, A::DepthStencilRead, L::DepthStencilRead};
    inline constexpr FTextureState PixelShaderRead = {S::PixelShading, A::ShaderResource, L::ShaderResource};
    inline constexpr FTextureState ComputeRead = {S::ComputeShading, A::ShaderResource, L::ShaderResource};
    inline constexpr FTextureState ComputeWrite = {S::ComputeShading, A::UnorderedAccess, L::UnorderedAccess};
    inline constexpr FTextureState CopySource = {S::Copy, A::CopySource, L::CopySource};
    inline constexpr FTextureState CopyDestination = {S::Copy, A::CopyDestination, L::CopyDestination};
}

class FTextureStateTracker
{
public:
    void Register(FGfxTextureHandle Texture, const FTextureState& State = TextureStates::Undefined);
    void Unregister(FGfxTextureHandle Texture);

    void Transition(FGfxTextureHandle Texture, const FTextureState& Next);
    void Flush(IGfxCommandList& CommandList);

    [[nodiscard]] const FTextureState* GetState(FGfxTextureHandle Texture) const;

private:
    struct FEntry
    {
        FGfxTextureHandle Texture;
        FTextureState State;
    };

    [[nodiscard]] FEntry* Find(FGfxTextureHandle Texture);

    TVector<FEntry> Entries;
    TVector<FGfxTextureBarrier> PendingBarriers;
};
