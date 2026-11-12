// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <array>

#include "Core/CoreTypes.hpp"

template <typename T, size64 TSize> requires std::is_object_v<T>
using TArray = std::array<T, TSize>;
