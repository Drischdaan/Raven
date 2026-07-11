// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "CoreTypes.hpp"

struct PLATFORM_API FWindowProperties
{
    uint32 Width = 1280;
    uint32 Height = 720;
    int32 StartPositionX = 0;
    int32 StartPositionY = 0;
    bool bShowAfterCreation = true;
};

struct PLATFORM_API FWindowState
{
    uint32 Width = 0;
    uint32 Height = 0;
    uint32 ClientWidth = 0;
    uint32 ClientHeight = 0;
    int32 PositionX = 0;
    int32 PositionY = 0;
};

class PLATFORM_API IWindow
{
public:
    IWindow(const FWindowProperties& InProperties);
    virtual ~IWindow() = default;

public:
    virtual void Show() = 0;
    virtual void Hide() = 0;
    virtual void Close() = 0;
    virtual void Destroy() = 0;

    [[nodiscard]] virtual bool IsClosed() const = 0;
    [[nodiscard]] virtual void* GetNativeHandle() const = 0;

    [[nodiscard]] virtual const FWindowState& GetWindowState() const;

protected:
    FWindowState WindowState = {};
};
