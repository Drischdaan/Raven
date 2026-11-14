// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Shaders/ShaderLibrary.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <ranges>

#include "Graphics/GfxDevice.hpp"
#include "Renderer/RendererLog.hpp"
#include "Renderer/Shaders/ShaderCompiler.hpp"

namespace
{
    constexpr uint32 CacheMagic = 0x43535652;

    uint64 HashString(const FStringView Text)
    {
        uint64 Hash = 0xCBF29CE484222325ull;
        for (const char Character : Text)
        {
            Hash ^= static_cast<uint8>(Character);
            Hash *= 0x100000001B3ull;
        }
        return Hash;
    }

    std::filesystem::path ToPath(const FStringView Utf8)
    {
        return std::filesystem::path(std::u8string_view(reinterpret_cast<const char8_t*>(Utf8.data()), Utf8.size()));
    }

    int64 GetWriteTime(const FStringView Path)
    {
        std::error_code Error;
        const auto Time = std::filesystem::last_write_time(ToPath(Path), Error);
        return Error ? 0 : Time.time_since_epoch().count();
    }

    template <typename T>
    void WritePod(std::ofstream& Stream, const T& Value)
    {
        Stream.write(reinterpret_cast<const char*>(&Value), sizeof(T));
    }

    template <typename T>
    bool8 ReadPod(std::ifstream& Stream, T& Value)
    {
        return static_cast<bool8>(Stream.read(reinterpret_cast<char*>(&Value), sizeof(T)));
    }
}

FShaderLibrary::FShaderLibrary() = default;

FShaderLibrary::~FShaderLibrary()
{
    Deinitialize();
}

FResult FShaderLibrary::Initialize(IGfxDevice& InDevice, const FShaderLibraryDesc& InDesc)
{
    Device = &InDevice;
    Desc = InDesc;

    std::error_code Error;
    std::filesystem::create_directories(ToPath(Desc.CacheDirectory), Error);
    if (Error)
    {
        RAVEN_LOG(LogShaders, Warning, "Can't create the shader cache directory '{}': {}", Desc.CacheDirectory, Error.message());
    }

    Compiler = MakeUnique<FShaderCompiler>();
    RESULT_CHECK(Compiler->Initialize(ToPath(Desc.ShaderDirectory), Desc.bDebugInfo));
    RAVEN_LOG(LogShaders, Info, "Shader directory '{}', cache '{}'", Desc.ShaderDirectory, Desc.CacheDirectory);
    return RESULT_OK_EMPTY();
}

void FShaderLibrary::Deinitialize()
{
    if (Device != nullptr)
    {
        for (const auto& Entry : Shaders | std::views::values)
        {
            Device->DestroyShader(Entry.Handle);
        }
    }
    Shaders.clear();
    Compiler.reset();
    Device = nullptr;
}

TResult<FGfxShaderHandle> FShaderLibrary::GetShader(const FShaderRequest& Request)
{
    const FString Key = Request.GetKey();
    if (const auto It = Shaders.find(FStringView(Key)); It != Shaders.end())
    {
        return It->second.Handle;
    }

    TVector<byte8> Bytecode;
    TVector<FString> Dependencies;
    if (!LoadFromCache(Key, Bytecode, Dependencies))
    {
        RESULT_CHECK_ASSIGN(Compiler->Compile(Request), Output);
        Bytecode = std::move(Output.Bytecode);
        Dependencies = std::move(Output.Dependencies);
        WriteToCache(Key, Bytecode, Dependencies);
        RAVEN_LOG(LogShaders, Info, "Compiled '{}' ({}), {} bytes, {} files", Request.Path, Request.EntryPoint, Bytecode.size(), Dependencies.size());
    }
    else
    {
        RAVEN_LOG(LogShaders, Verbose, "Loaded '{}' ({}) from the cache", Request.Path, Request.EntryPoint);
    }

    const FString DebugName = String::Format("{}:{}", Request.Path, Request.EntryPoint);
    RESULT_CHECK_ASSIGN(Device->CreateShader({
                            .Stage = Request.Stage,
                            .Format = EGfxShaderFormat::DXIL,
                            .Bytecode = Bytecode,
                            .EntryPoint = Request.EntryPoint,
                            .DebugName = DebugName,
                            }), Handle);

    Shaders.emplace(Key, FEntry{.Handle = Handle, .Dependencies = std::move(Dependencies)});
    return Handle;
}

FString FShaderLibrary::GetCachePath(const FStringView Key) const
{
    const uint64 Hash = HashString(String::Format("{}|v{}|debug={}", Key, CacheVersion, Desc.bDebugInfo));
    return String::Format("{}/{:016x}.dxil", Desc.CacheDirectory, Hash);
}

bool8 FShaderLibrary::LoadFromCache(const FStringView Key, TVector<byte8>& OutBytecode, TVector<FString>& OutDependencies) const
{
    std::ifstream Stream(ToPath(GetCachePath(Key)), std::ios::binary);
    if (!Stream)
    {
        return false;
    }

    uint32 Magic = 0;
    uint32 Version = 0;
    uint32 DependencyCount = 0;
    if (!ReadPod(Stream, Magic) || !ReadPod(Stream, Version) || !ReadPod(Stream, DependencyCount) || Magic != CacheMagic || Version != CacheVersion)
    {
        return false;
    }

    OutDependencies.clear();
    for (uint32 Index = 0; Index < DependencyCount; ++Index)
    {
        uint32 Length = 0;
        if (!ReadPod(Stream, Length) || Length > 4096)
        {
            return false;
        }
        FString Path(Length, '\0');
        int64 WriteTime = 0;
        if (!Stream.read(Path.data(), Length) || !ReadPod(Stream, WriteTime))
        {
            return false;
        }
        if (WriteTime == 0 || GetWriteTime(Path) != WriteTime)
        {
            return false;
        }
        OutDependencies.push_back(std::move(Path));
    }

    uint64 Size = 0;
    if (!ReadPod(Stream, Size) || Size == 0 || Size > 64ull * 1024 * 1024)
    {
        return false;
    }
    OutBytecode.resize(Size);
    return static_cast<bool8>(Stream.read(reinterpret_cast<char*>(OutBytecode.data()), static_cast<std::streamsize>(Size)));
}

void FShaderLibrary::WriteToCache(const FStringView Key, const TSpan<const byte8> Bytecode, const TSpan<const FString> Dependencies) const
{
    const FString Path = GetCachePath(Key);
    std::ofstream Stream(ToPath(Path), std::ios::binary | std::ios::trunc);
    if (!Stream)
    {
        RAVEN_LOG(LogShaders, Warning, "Can't write the shader cache file '{}'", Path);
        return;
    }

    WritePod(Stream, CacheMagic);
    WritePod(Stream, CacheVersion);
    WritePod(Stream, static_cast<uint32>(Dependencies.size()));
    for (const FString& Dependency : Dependencies)
    {
        WritePod(Stream, static_cast<uint32>(Dependency.size()));
        Stream.write(Dependency.data(), static_cast<std::streamsize>(Dependency.size()));
        WritePod(Stream, GetWriteTime(Dependency));
    }
    WritePod(Stream, Bytecode.size());
    Stream.write(reinterpret_cast<const char*>(Bytecode.data()), static_cast<std::streamsize>(Bytecode.size()));
}
