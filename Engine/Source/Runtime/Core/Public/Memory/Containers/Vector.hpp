// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <vector>

template <typename T> requires std::is_object_v<T>
using TVector = std::pmr::vector<T>;
