// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Platform/Window.hpp"

#include <print>
#include <windowsx.h>

LRESULT CALLBACK WindowProc(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
{
    FWindow* Window = nullptr;
    if (uMsg == WM_CREATE)
    {
        const CREATESTRUCT* CreateStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
        Window = reinterpret_cast<FWindow*>(CreateStruct->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(Window));
    }
    else
    {
        Window = reinterpret_cast<FWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (Window != nullptr)
    {
        return Window->WindowProcess(hwnd, uMsg, wParam, lParam);
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

FWindow::FWindow(const FWindowSettings& InSettings)
    : Settings(InSettings)
{
    WNDCLASSEX WindowClass = {};
    WindowClass.cbSize = sizeof(WNDCLASSEX);
    WindowClass.lpszClassName = "RavenWindowClass";
    WindowClass.hInstance = GetModuleHandle(nullptr);
    WindowClass.lpfnWndProc = WindowProc;
    RegisterClassEx(&WindowClass);

    WindowHandle = CreateWindowEx(0,
                                  WindowClass.lpszClassName,
                                  "Raven",
                                  WS_OVERLAPPEDWINDOW,
                                  CW_USEDEFAULT,
                                  CW_USEDEFAULT,
                                  static_cast<int32>(Settings.Width),
                                  static_cast<int32>(Settings.Height),
                                  nullptr,
                                  nullptr,
                                  WindowClass.hInstance,
                                  this
    );
    if (GetLastError() != 0)
    {
        std::println("Failed to create window. Error: {}", GetLastError());
    }
}

FWindow::~FWindow()
{
    if (WindowHandle != nullptr && !State.bIsDestroyed)
    {
        DestroyWindow(WindowHandle);
        WindowHandle = nullptr;
    }
}

void FWindow::Show()
{
    ShowWindow(WindowHandle, SW_SHOW);
}

void FWindow::Hide()
{
    ShowWindow(WindowHandle, SW_HIDE);
}

const FWindowState& FWindow::GetWindowState() const
{
    return State;
}

LRESULT FWindow::WindowProcess(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SIZE:
        {
            State.ClientWidth = LOWORD(lParam);
            State.ClientHeight = HIWORD(lParam);

            RECT WindowRect = {};
            GetWindowRect(hwnd, &WindowRect);

            State.WindowWidth = WindowRect.right - WindowRect.left;
            State.WindowHeight = WindowRect.bottom - WindowRect.top;
        }
        break;
    case WM_MOVE:
        {
            State.PositionX = GET_X_LPARAM(lParam);
            State.PositionY = GET_Y_LPARAM(lParam);
        }
        break;
    case WM_CLOSE:
        {
            State.bIsClosed = true;
        }
        break;
    case WM_DESTROY:
        {
            State.bIsDestroyed = true;
            WindowHandle = nullptr;
        }
        break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
