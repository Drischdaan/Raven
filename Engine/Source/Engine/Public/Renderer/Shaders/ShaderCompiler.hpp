// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include <filesystem>

#include <Windows.h>
#include <Unknwnbase.h> // IUnknown: WIN32_LEAN_AND_MEAN strips it, and dxcapi.h needs it first
#include <OleAuto.h>    // BSTR
#include <dxcapi.h>
#include <wrl/client.h>

#include "Renderer/Shaders/ShaderTypes.hpp"
#include "Core/Utility/Result.hpp"

struct FShaderCompileOutput
{
    TVector<byte8> Bytecode;
    TVector<FString> Dependencies;
};

class FShaderCompiler
{
public:
    FShaderCompiler() = default;
    FShaderCompiler(const FShaderCompiler&) = delete;
    FShaderCompiler& operator=(const FShaderCompiler&) = delete;
    ~FShaderCompiler();

public:
    FResult Initialize(const std::filesystem::path& InShaderDirectory, bool8 bInDebugInfo);
    void Deinitialize();

    TResult<FShaderCompileOutput> Compile(const FShaderRequest& Request);

private:
    HMODULE Module = nullptr;
    Microsoft::WRL::ComPtr<IDxcUtils> Utils;
    Microsoft::WRL::ComPtr<IDxcCompiler3> Compiler;
    std::filesystem::path ShaderDirectory;
    bool8 bDebugInfo = false;
};
