// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <Windows.h>

#include "Core/CoreTypes.hpp"

struct RAVEN_API FWindowSettings
{
    uint32 Width = 1280;
    uint32 Height = 720;
};

struct RAVEN_API FWindowState
{
    uint32 WindowWidth = 0;
    uint32 WindowHeight = 0;
    uint32 ClientWidth = 0;
    uint32 ClientHeight = 0;
    int32 PositionX = 0;
    int32 PositionY = 0;
    bool bIsClosed = false;
    bool bIsDestroyed = false;
};

class RAVEN_API FWindow
{
public:
    FWindow(const FWindowSettings& InSettings);
    FWindow(const FWindow&) = delete;
    FWindow(const FWindow&&) = delete;
    ~FWindow();

    FWindow& operator=(const FWindow&) = delete;
    FWindow& operator=(const FWindow&&) = delete;

public:
    void Show();
    void Hide();

    [[nodiscard]] const FWindowState& GetWindowState() const;

private:
    LRESULT WindowProcess(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    FWindowSettings Settings;
    FWindowState State = {};

    HWND WindowHandle = nullptr;

    friend LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
