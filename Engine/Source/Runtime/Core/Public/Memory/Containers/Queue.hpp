// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <deque>
#include <queue>
#include <stack>

#include "Vector.hpp"

template <typename T> requires std::is_object_v<T>
using TDeque = std::pmr::deque<T>;

template <typename T, typename TContainer = TDeque<T>> requires std::is_object_v<T> && std::is_same_v<T, typename TContainer::value_type>
using TQueue = std::queue<T, TContainer>;

template <typename T, typename TContainer = TVector<T>> requires std::is_object_v<T> && std::is_same_v<T, typename TContainer::value_type>
using TPriorityQueue = std::priority_queue<T, TContainer>;
