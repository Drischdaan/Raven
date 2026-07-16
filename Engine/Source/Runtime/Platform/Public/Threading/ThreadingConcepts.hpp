// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include <chrono>
#include <concepts>

#include "CoreTypes.hpp"
#include "FiberTypes.hpp"

struct FPlatformFiberDesc;
struct FThreadDesc;

template <typename T>
concept CIsMoveableNonCopy = std::default_initializable<T> && std::movable<T> && !std::copy_constructible<T>;

template <typename T>
concept CMutex = std::default_initializable<T> && !std::copy_constructible<T> && requires(T Mutex)
{
    { Mutex.Lock() } -> std::same_as<void>;
    { Mutex.TryLock() } -> std::same_as<bool8>;
    { Mutex.Unlock() } -> std::same_as<void>;
};

template <typename T, typename TMutex>
concept CConditionVariable = std::default_initializable<T> && !std::copy_constructible<T> && requires(T ConditionVariable, TMutex Mutex, std::chrono::milliseconds Timeout)
{
    { ConditionVariable.Wait(Mutex) } -> std::same_as<void>;
    { ConditionVariable.Wait(Mutex, Timeout) } -> std::same_as<bool8>;
    { ConditionVariable.NotifyOne() } -> std::same_as<void>;
    { ConditionVariable.NotifyAll() } -> std::same_as<void>;
};

template <typename T>
concept CThreadConstructable = std::is_default_constructible_v<T> && std::constructible_from<T, const FThreadDesc&> && std::constructible_from<T, const FThreadDesc&>;

template <typename T>
concept CThread = CIsMoveableNonCopy<T> && CThreadConstructable<T> && requires(T Thread, const T ConstThread)
{
    { ConstThread.IsValid() } -> std::same_as<bool8>;
    { ConstThread.IsJoinable() } -> std::same_as<bool8>;
    { ConstThread.GetThreadId() } -> std::same_as<uint64>;
    { Thread.Join() } -> std::same_as<void>;
    { Thread.Detach() } -> std::same_as<void>;
};

template <typename T>
concept CFiberPlatform = std::default_initializable<T> && !std::copy_constructible<T> && requires(T Context, T& From, T& To, const FPlatformFiberDesc& Params,
                                                                                                  const FFiberEntryFunction InternalEntryFunction, void* InternalData, const T& ConstContext)
{
    { ConstContext.IsValid() } -> std::same_as<bool8>;
    { Context.InitWithStack(Params, InternalEntryFunction, InternalData) } -> std::same_as<bool8>;
    { Context.InitFromThread() } -> std::same_as<bool8>;
    { Context.Release() } -> std::same_as<void>;
    { T::Swap(From, To) } -> std::same_as<void>;
    { ConstContext.GetStackBottom() } -> std::same_as<void*>;
    { ConstContext.GetStackSize() } -> std::same_as<size64>;
};
