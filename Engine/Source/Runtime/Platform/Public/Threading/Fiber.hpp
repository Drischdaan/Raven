// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "CoreTypes.hpp"
#include "PlatformMacros.hpp"
#include "ThreadingConcepts.hpp"

#include PLATFORM_COMPILED_HEADER_PATH(Threading, Fiber.hpp)

static_assert(CFiberPlatform<FPlatformFiber>, "Platform Fiber implementation is incomplete");

class PLATFORM_API FFiber
{
public:
    FFiber() = default;
    explicit FFiber(const FPlatformFiberDesc& InDesc);
    FFiber(const FFiber&) = delete;
    FFiber(FFiber&& Other) noexcept = delete;
    ~FFiber();

    FFiber& operator=(const FFiber&) = delete;
    FFiber& operator=(FFiber&& Other) noexcept = delete;

public:
    [[nodiscard]] bool8 IsValid() const;

    void Initialize(const FPlatformFiberDesc& InDesc);
    void Reset();

    void* GetUserData() const;
    const TChar* GetName() const;

public:
    static void Switch(const FFiber& From, const FFiber& To);

private:
    static void Trampoline(void* Data);
    static void OnResume(const FFiber& Self);

private:
    FPlatformFiberDesc Desc;

    FPlatformFiber PlatformFiber;
    bool8 bIsThreadFiber = false;

    friend class FThreadFiberScope;
};

class PLATFORM_API FThreadFiberScope
{
public:
    FThreadFiberScope();
    FThreadFiberScope(const FThreadFiberScope&) = delete;
    FThreadFiberScope(FThreadFiberScope&& Other) noexcept = delete;
    ~FThreadFiberScope();

    FThreadFiberScope& operator=(const FThreadFiberScope&) = delete;
    FThreadFiberScope& operator=(FThreadFiberScope&& Other) noexcept = delete;

public:
    [[nodiscard]] FFiber& GetFiber() noexcept;

private:
    FFiber Fiber;
};
