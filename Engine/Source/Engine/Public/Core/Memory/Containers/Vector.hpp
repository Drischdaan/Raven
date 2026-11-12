// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <vector>

template <typename T> requires std::is_object_v<T>
using TVector = std::pmr::vector<T>;
