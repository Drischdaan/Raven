// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Graphics/GfxTypes.hpp"

using FMeshHandle = TGfxHandle<struct FMeshTag>;
using FInstanceId = TGfxHandle<struct FInstanceTag>;
using FTextureHandle = TGfxHandle<struct FTextureTag>;
using FMaterialHandle = TGfxHandle<struct FMaterialTag>;

inline constexpr uint32 InvalidGpuIndex = UINT32_MAX;
