// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#include "Threading/Fiber.hpp"

#include <iostream>

#include "CoreAssertions.hpp"

#include "Profiling/Profiling.hpp"

FFiber::FFiber(const FPlatformFiberDesc& InDesc)
{
    Initialize(InDesc);
}

FFiber::~FFiber()
{
    Reset();
}

bool8 FFiber::IsValid() const
{
    return PlatformFiber.IsValid();
}

void FFiber::Initialize(const FPlatformFiberDesc& InDesc)
{
    RAVEN_CORE_ASSERT(!IsValid() && InDesc.EntryFunction);
    Desc = InDesc;
    [[maybe_unused]] const bool8 bIsValidFiber = PlatformFiber.InitWithStack(Desc, Trampoline, this);
    RAVEN_CORE_ASSERT(bIsValidFiber, "Failed to initialize fiber");
}

void FFiber::Reset()
{
    PlatformFiber.Release();
    Desc = {};
}

void* FFiber::GetUserData() const
{
    return Desc.Data;
}

const TChar* FFiber::GetName() const
{
    return Desc.Name;
}

void FFiber::Switch(const FFiber& From, const FFiber& To)
{
    RAVEN_CORE_ASSERT(&From != &To, "Cannot switch to the same fiber");

    if (!From.bIsThreadFiber)
    {
        RAVEN_PROFILE_FIBER_LEAVE();
    }
    FPlatformFiber::Swap(From.PlatformFiber, To.PlatformFiber);
    if (!From.bIsThreadFiber)
    {
        RAVEN_PROFILE_FIBER_ENTER(From.GetName());
    }
}

void FFiber::Trampoline(void* Data)
{
    const FFiber& Self = *static_cast<FFiber*>(Data);
    OnResume(Self);
    Self.Desc.EntryFunction(Self.Desc.Data);
    RAVEN_CORE_ASSERT(false, "Fiber returned from entry function");
}

void FFiber::OnResume(const FFiber& Self)
{
    RAVEN_PROFILE_FIBER_ENTER(Self.GetName());
}

FThreadFiberScope::FThreadFiberScope()
{
    [[maybe_unused]] const bool8 bIsValidFiber = Fiber.PlatformFiber.InitFromThread();
    RAVEN_CORE_ASSERT(bIsValidFiber, "Failed to initialize fiber");
    Fiber.bIsThreadFiber = true;
    Fiber.Desc.Name = "ThreadFiber";
}

FThreadFiberScope::~FThreadFiberScope()
{
    Fiber.Reset();
    Fiber.bIsThreadFiber = false;
}

FFiber& FThreadFiberScope::GetFiber() noexcept
{
    return Fiber;
}
