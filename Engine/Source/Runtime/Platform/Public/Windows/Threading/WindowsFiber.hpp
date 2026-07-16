// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once
#include "Threading/FiberTypes.hpp"

class PLATFORM_API FWindowsFiber
{
public:
    FWindowsFiber() = default;
    FWindowsFiber(const FWindowsFiber&) = delete;
    FWindowsFiber(FWindowsFiber&& Other) noexcept;
    ~FWindowsFiber();

    FWindowsFiber& operator=(const FWindowsFiber&) = delete;
    FWindowsFiber& operator=(FWindowsFiber&& Other) noexcept;

public:
    [[nodiscard]] bool8 IsValid() const;

    [[nodiscard]] bool8 InitWithStack(const FPlatformFiberDesc& Desc, FFiberEntryFunction InInternalEntryFunction, void* InternalData);
    [[nodiscard]] bool8 InitFromThread();

    void Release();

    [[nodiscard]] void* GetStackBottom() const;
    [[nodiscard]] size64 GetStackSize() const;

public:
    static void Swap(const FWindowsFiber& From, const FWindowsFiber& To);

private:
    FFiberEntryFunction InternalEntryFunction = nullptr;

    void* FiberHandle = nullptr;
    bool8 bOwnsFiber = false;
};

using FPlatformFiber = FWindowsFiber;
