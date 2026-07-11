// Copyright RavenStormStudio 2026 All Rights Reserved Copyright

#pragma once

#include "Window.hpp"

#include "GLFW/glfw3.h"

class FWindow_Default : public IWindow
{
public:
    FWindow_Default(const FWindowProperties& InProperties);
    ~FWindow_Default() override;

public:
    void Show() override;
    void Hide() override;
    void Close() override;
    void Destroy() override;

    [[nodiscard]] bool IsClosed() const override;

    [[nodiscard]] void* GetNativeHandle() const override;

private:
    GLFWwindow* Window = nullptr;
};
