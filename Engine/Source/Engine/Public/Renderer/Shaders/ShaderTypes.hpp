// Raven Storm Studio @ 2026. All Rights Reserved.

#pragma once

#include "Graphics/GfxPipelines.hpp"
#include "Core/Memory/Containers/String.hpp"
#include "Core/Memory/Containers/Vector.hpp"

struct FShaderDefine
{
    FString Name;
    FString Value = "1";
};

struct FShaderRequest
{
    FString Path;
    FString EntryPoint = "main";
    EGfxShaderStage Stage = EGfxShaderStage::Vertex;
    TVector<FShaderDefine> Defines;

public:
    [[nodiscard]] FString GetKey() const
    {
        FString Key = String::Format("{}|{}|{}", Path, EntryPoint, std::to_underlying(Stage));
        for (const FShaderDefine& Define : Defines)
        {
            Key += String::Format("|{}={}", Define.Name, Define.Value);
        }
        return Key;
    }
};
