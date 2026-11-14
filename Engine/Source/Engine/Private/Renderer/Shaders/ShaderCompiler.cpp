// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Shaders/ShaderCompiler.hpp"

#include <algorithm>

#include "Renderer/RendererLog.hpp"

using Microsoft::WRL::ComPtr;

namespace
{
    FWideString ToWide(const FStringView Text)
    {
        if (Text.empty())
        {
            return {};
        }
        const int32 Length = MultiByteToWideChar(CP_UTF8, 0, Text.data(), static_cast<int32>(Text.size()), nullptr, 0);
        FWideString Result(static_cast<size64>(Length), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, Text.data(), static_cast<int32>(Text.size()), Result.data(), Length);
        return Result;
    }

    FString ToUtf8(const std::filesystem::path& Path)
    {
        const std::u8string Utf8 = Path.generic_u8string();
        return FString(reinterpret_cast<const char*>(Utf8.data()), Utf8.size());
    }

    const wchar_t* GetProfile(const EGfxShaderStage Stage)
    {
        switch (Stage)
        {
        case EGfxShaderStage::Vertex: return L"vs_6_6";
        case EGfxShaderStage::Pixel: return L"ps_6_6";
        case EGfxShaderStage::Compute: return L"cs_6_6";
        case EGfxShaderStage::Amplification: return L"as_6_6";
        case EGfxShaderStage::Mesh: return L"ms_6_6";
        default: return L"vs_6_6";
        }
    }

    class FRecordingIncludeHandler final : public IDxcIncludeHandler
    {
    public:
        FRecordingIncludeHandler(IDxcUtils& InUtils, TVector<FString>& InDependencies)
            : Utils(InUtils)
              , Dependencies(InDependencies)
        {
        }

        HRESULT STDMETHODCALLTYPE LoadSource(_In_z_ const LPCWSTR Filename, _COM_Outptr_result_maybenull_ IDxcBlob** IncludeSource) override
        {
            *IncludeSource = nullptr;
            const std::filesystem::path Path = std::filesystem::absolute(std::filesystem::path(Filename)).lexically_normal();

            ComPtr<IDxcBlobEncoding> Blob;
            const HRESULT Result = Utils.LoadFile(Path.c_str(), nullptr, &Blob);
            if (FAILED(Result))
            {
                return Result;
            }

            FString Dependency = ToUtf8(Path);
            if (std::ranges::find(Dependencies, Dependency) == Dependencies.end())
            {
                Dependencies.push_back(std::move(Dependency));
            }
            *IncludeSource = Blob.Detach();
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID Riid, void** Object) override
        {
            if (Riid == __uuidof(IDxcIncludeHandler) || Riid == __uuidof(IUnknown))
            {
                *Object = static_cast<IDxcIncludeHandler*>(this);
                return S_OK;
            }
            *Object = nullptr;
            return E_NOINTERFACE;
        }

        ULONG STDMETHODCALLTYPE AddRef() override
        {
            return 1;
        }

        ULONG STDMETHODCALLTYPE Release() override
        {
            return 1;
        }

    private:
        IDxcUtils& Utils;
        TVector<FString>& Dependencies;
    };
}

FShaderCompiler::~FShaderCompiler()
{
    Deinitialize();
}

FResult FShaderCompiler::Initialize(const std::filesystem::path& InShaderDirectory, const bool8 bInDebugInfo)
{
    ShaderDirectory = std::filesystem::absolute(InShaderDirectory).lexically_normal();
    bDebugInfo = bInDebugInfo;

    Module = LoadLibraryW(L"dxcompiler.dll");
    if (Module == nullptr)
    {
        return RESULT_PLATFORM_ERROR("Failed to load dxcompiler.dll (is the directxshadercompiler package copied next to the executable?)");
    }

    const auto CreateInstance = reinterpret_cast<DxcCreateInstanceProc>(GetProcAddress(Module, "DxcCreateInstance"));
    if (CreateInstance == nullptr)
    {
        return RESULT_ERROR("dxcompiler.dll doesn't export DxcCreateInstance");
    }
    RESULT_TRY_PLATFORM_CALL(CreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&Utils)), "Failed to create IDxcUtils")
    RESULT_TRY_PLATFORM_CALL(CreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler)), "Failed to create IDxcCompiler3")
    return RESULT_OK_EMPTY();
}

void FShaderCompiler::Deinitialize()
{
    Compiler.Reset();
    Utils.Reset();
    if (Module != nullptr)
    {
        FreeLibrary(Module);
        Module = nullptr;
    }
}

TResult<FShaderCompileOutput> FShaderCompiler::Compile(const FShaderRequest& Request)
{
    if (!Compiler)
    {
        return RESULT_ERROR("Shader compiler isn't initialized");
    }

    const std::filesystem::path SourcePath = (ShaderDirectory / std::filesystem::path(ToWide(Request.Path))).lexically_normal();

    ComPtr<IDxcBlobEncoding> Source;
    if (FAILED(Utils->LoadFile(SourcePath.c_str(), nullptr, &Source)))
    {
        return RESULT_ERROR(String::Format("Can't read shader '{}'", ToUtf8(SourcePath)));
    }

    TVector<FWideString> Arguments;
    Arguments.emplace_back(SourcePath.wstring());
    Arguments.emplace_back(L"-E");
    Arguments.emplace_back(ToWide(Request.EntryPoint));
    Arguments.emplace_back(L"-T");
    Arguments.emplace_back(GetProfile(Request.Stage));
    Arguments.emplace_back(L"-HV");
    Arguments.emplace_back(L"2021");
    Arguments.emplace_back(L"-I");
    Arguments.emplace_back(ShaderDirectory.wstring());
    Arguments.emplace_back(L"-Zpc");
    Arguments.emplace_back(L"-WX");
    if (bDebugInfo)
    {
        Arguments.emplace_back(L"-Zi");
        Arguments.emplace_back(L"-Qembed_debug");
        Arguments.emplace_back(L"-Od");
    }
    else
    {
        Arguments.emplace_back(L"-O3");
    }
    for (const FShaderDefine& Define : Request.Defines)
    {
        Arguments.emplace_back(L"-D");
        Arguments.emplace_back(ToWide(String::Format("{}={}", Define.Name, Define.Value)));
    }

    TVector<LPCWSTR> ArgumentPointers;
    ArgumentPointers.reserve(Arguments.size());
    for (const FWideString& Argument : Arguments)
    {
        ArgumentPointers.push_back(Argument.c_str());
    }

    FShaderCompileOutput Output;
    Output.Dependencies.push_back(ToUtf8(SourcePath));
    FRecordingIncludeHandler IncludeHandler(*Utils.Get(), Output.Dependencies);

    const DxcBuffer SourceBuffer = {
        .Ptr = Source->GetBufferPointer(),
        .Size = Source->GetBufferSize(),
        .Encoding = DXC_CP_UTF8,
    };

    ComPtr<IDxcResult> Result;
    RESULT_TRY_PLATFORM_CALL(Compiler->Compile(&SourceBuffer, ArgumentPointers.data(), static_cast<UINT32>(ArgumentPointers.size()),
                                 &IncludeHandler, IID_PPV_ARGS(&Result)), "IDxcCompiler3::Compile failed")

    ComPtr<IDxcBlobUtf8> Errors;
    Result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&Errors), nullptr);
    const FStringView Messages = Errors && Errors->GetStringLength() > 0 ? FStringView(Errors->GetStringPointer(), Errors->GetStringLength()) : FStringView();

    HRESULT Status = S_OK;
    Result->GetStatus(&Status);
    if (FAILED(Status))
    {
        return RESULT_ERROR(String::Format("Failed to compile '{}' ({}):\n{}", Request.Path, Request.EntryPoint, Messages));
    }
    if (!Messages.empty())
    {
        RAVEN_LOG(LogShaders, Warning, "'{}' ({}):\n{}", Request.Path, Request.EntryPoint, Messages);
    }

    ComPtr<IDxcBlob> Object;
    RESULT_TRY_PLATFORM_CALL(Result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&Object), nullptr), "DXC produced no object")
    const byte8* Data = static_cast<const byte8*>(Object->GetBufferPointer());
    Output.Bytecode.assign(Data, Data + Object->GetBufferSize());
    return Output;
}
