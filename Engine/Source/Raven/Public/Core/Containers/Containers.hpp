// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "Core/Types.hpp"

#include <array>
#include <deque>
#include <map>
#include <queue>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template <typename T, size64 TSize> requires std::is_object_v<T>
using TArray = std::array<T, TSize>;

template <typename T> requires std::is_object_v<T>
using TVector = std::pmr::vector<T>;

template <typename TKey, typename TValue, typename TComparer = std::less<TKey>> requires std::is_object_v<TKey>
using TMap = std::pmr::map<TKey, TValue, TComparer>;

template <typename TKey, typename TValue, typename THasher = std::hash<TKey>, typename TComparer = std::equal_to<TKey>> requires std::is_object_v<TKey>
using TUnorderedMap = std::pmr::unordered_map<TKey, TValue, THasher>;

template <typename T, typename TComparer = std::less<T>> requires std::is_object_v<T>
using TSet = std::pmr::set<T, TComparer>;

template <typename T, typename THasher = std::hash<T>, typename TComparer = std::equal_to<T>>
using TUnorderedSet = std::pmr::unordered_set<T>;

template <typename T> requires std::is_object_v<T>
using TDeque = std::pmr::deque<T>;

template <typename T, typename TContainer = TDeque<T>> requires std::is_object_v<T> && std::is_same_v<T, typename TContainer::value_type>
using TQueue = std::queue<T, TContainer>;

template <typename T, typename TContainer = TVector<T>> requires std::is_object_v<T> && std::is_same_v<T, typename TContainer::value_type>
using TPriorityQueue = std::priority_queue<T, TContainer>;
