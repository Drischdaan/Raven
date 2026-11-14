// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Core/Memory/Containers/Array.hpp"
#include "Graphics/GfxTypes.hpp"

struct FD3D12QueueFenceValues
{
    TArray<uint64, static_cast<size64>(EGfxQueueType::Count)> Values = {};
};
