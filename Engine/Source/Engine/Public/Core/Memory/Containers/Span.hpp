// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Core/CoreTypes.hpp"

#include <span>

template <typename T, size64 TExtent = std::dynamic_extent>
using TSpan = std::span<T, TExtent>;
