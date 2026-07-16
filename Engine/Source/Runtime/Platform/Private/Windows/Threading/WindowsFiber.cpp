// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Windows/Threading/WindowsFiber.hpp"

#include <Windows.h>

FWindowsFiber::FWindowsFiber(FWindowsFiber&& Other) noexcept
{
    FiberHandle = Other.FiberHandle;
    bOwnsFiber = Other.bOwnsFiber;
    Other.FiberHandle = nullptr;
    Other.bOwnsFiber = false;
}

FWindowsFiber::~FWindowsFiber()
{
    Release();
}

FWindowsFiber& FWindowsFiber::operator=(FWindowsFiber&& Other) noexcept
{
    if (this != &Other)
    {
        Release();
        FiberHandle = Other.FiberHandle;
        bOwnsFiber = Other.bOwnsFiber;
        Other.FiberHandle = nullptr;
        Other.bOwnsFiber = false;
    }
    return *this;
}

bool8 FWindowsFiber::IsValid() const
{
    return FiberHandle != nullptr;
}

bool8 FWindowsFiber::InitWithStack(const FPlatformFiberDesc& Desc, const FFiberEntryFunction InInternalEntryFunction, void* InternalData)
{
    InternalEntryFunction = InInternalEntryFunction;
    FiberHandle = CreateFiberEx(Desc.CommitSize, Desc.ReserveSize, FIBER_FLAG_FLOAT_SWITCH, InternalEntryFunction, InternalData);
    bOwnsFiber = true;
    return FiberHandle != nullptr;
}

bool8 FWindowsFiber::InitFromThread()
{
    FiberHandle = ConvertThreadToFiber(nullptr);
    bOwnsFiber = false;
    return FiberHandle != nullptr;
}

void FWindowsFiber::Release()
{
    if (!FiberHandle)
    {
        return;
    }
    if (bOwnsFiber)
    {
        DeleteFiber(FiberHandle);
    }
    else
    {
        ::ConvertFiberToThread();
    }
    FiberHandle = nullptr;
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void* FWindowsFiber::GetStackBottom() const
{
    return nullptr; // Not used on Windows
}

// ReSharper disable once CppMemberFunctionMayBeStatic
size64 FWindowsFiber::GetStackSize() const
{
    return 0; // Not used on Windows
}

void FWindowsFiber::Swap([[maybe_unused]] const FWindowsFiber& From, const FWindowsFiber& To)
{
    SwitchToFiber(To.FiberHandle);
}
