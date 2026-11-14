// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "ShaderTypes.hpp"
#include "Graphics/GfxTypes.hpp"
#include "Core/Memory/SmartPointers.hpp"
#include "Core/Memory/Containers/Map.hpp"
#include "Core/Utility/Result.hpp"

class IGfxDevice;
class FShaderCompiler;

struct FShaderLibraryDesc
{
    FString ShaderDirectory;
    FString CacheDirectory;
    bool8 bDebugInfo = false;
};

struct FStringHash
{
    using is_transparent = void;

    [[nodiscard]] size64 operator()(const FStringView Text) const
    {
        return std::hash<FStringView>{}(Text);
    }
};

class RAVEN_API FShaderLibrary
{
public:
    static constexpr uint32 CacheVersion = 1;

public:
    FShaderLibrary();
    FShaderLibrary(const FShaderLibrary&) = delete;
    FShaderLibrary& operator=(const FShaderLibrary&) = delete;
    ~FShaderLibrary();

public:
    FResult Initialize(IGfxDevice& InDevice, const FShaderLibraryDesc& InDesc);
    void Deinitialize();

    TResult<FGfxShaderHandle> GetShader(const FShaderRequest& Request);

    [[nodiscard]] const FString& GetShaderDirectory() const
    {
        return Desc.ShaderDirectory;
    }

private:
    struct FEntry
    {
        FGfxShaderHandle Handle;
        TVector<FString> Dependencies;
    };

    [[nodiscard]] FString GetCachePath(FStringView Key) const;
    [[nodiscard]] bool8 LoadFromCache(FStringView Key, TVector<byte8>& OutBytecode, TVector<FString>& OutDependencies) const;
    void WriteToCache(FStringView Key, TSpan<const byte8> Bytecode, TSpan<const FString> Dependencies) const;

private:
    IGfxDevice* Device = nullptr;
    TUniquePtr<FShaderCompiler> Compiler;
    FShaderLibraryDesc Desc;
    TUnorderedMap<FString, FEntry, FStringHash, std::equal_to<>> Shaders;
};
