// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <set>
#include <unordered_set>

template <typename T, typename TComparer = std::less<T>> requires std::is_object_v<T>
using TSet = std::pmr::set<T, TComparer>;

template <typename T, typename THasher = std::hash<T>, typename TComparer = std::equal_to<T>> requires std::is_object_v<T>
using TUnorderedSet = std::pmr::unordered_set<T, THasher, TComparer>;
