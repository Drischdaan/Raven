// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "Core/Types.hpp"

#include <memory>
#include <type_traits>

class FMemory
{
public:
	static void* Allocate(size64 Size, size64 Alignment = 8);
	static void* Reallocate(void* Pointer, size64 Size, size64 Alignment = 8);

	static void Free(void* Pointer, size64 Alignment = 8);

	static size64 GetPointerSize(const void* Pointer);

public:
	template <typename T, typename... TArguments>
		requires std::is_constructible_v<T, TArguments...>
	static T* AllocateObject(TArguments&&... Arguments)
	{
		T* Pointer = static_cast<T*>(Allocate(sizeof(T), alignof(T)));
		if (Pointer == nullptr)
		{
			return nullptr;
		}
		return std::construct_at(Pointer, std::forward<TArguments>(Arguments)...);
	}

	template <typename T, typename... TArguments>
		requires std::is_constructible_v<T, TArguments...>
	static T* AllocateObjectAligned(const size64 Alignment, TArguments&&... Arguments)
	{
		T* Pointer = static_cast<T*>(Allocate(sizeof(T), Alignment));
		if (Pointer == nullptr)
		{
			return nullptr;
		}
		return std::construct_at(Pointer, std::forward<TArguments>(Arguments)...);
	}

	template <typename T, typename... TArguments>
		requires std::is_constructible_v<T, TArguments&...>
	static T* AllocateObjects(const uint32 Count, TArguments&&... Arguments)
	{
		T* StartPointer = static_cast<T*>(Allocate(sizeof(T) * Count, alignof(T)));
		if (StartPointer == nullptr)
		{
			return nullptr;
		}
		for (uint32 Index = 0; Index < Count; ++Index)
		{
			std::construct_at(StartPointer + Index, Arguments...);
		}
		return StartPointer;
	}

	template <typename T>
	static void FreeObject(T* Pointer)
	{
		std::destroy_at(Pointer);
		Free(static_cast<void*>(Pointer), sizeof(T));
	}

	template <typename T>
	static void FreeObjects(T* StartPointer, const uint32 Count)
	{
		for (uint32 Index = 0; Index < Count; ++Index)
		{
			std::destroy_at(StartPointer + Index);
		}
		Free(static_cast<void*>(StartPointer), sizeof(T) * Count);
	}
};
