// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Renderer/Texture/TextureGenerators.hpp"

#include <cmath>

#include "Core/Math/MathUtility.hpp"
#include "Core/Math/Vector3.hpp"

namespace
{
    constexpr uint32 BytesPerPixel = 4;

    bool8 IsSrgb(const EGfxFormat Format)
    {
        return Format == EGfxFormat::RGBA8_SRGB || Format == EGfxFormat::BGRA8_SRGB;
    }

    uint8 ToByte(const float32 Value)
    {
        return static_cast<uint8>(std::lround(Math::Clamp(Value, 0.0f, 1.0f) * 255.0f));
    }

    FTextureBuilder MakeTexture(const uint32 Width, const uint32 Height, const EGfxFormat Format)
    {
        FTextureBuilder Texture;
        Texture.Width = Width;
        Texture.Height = Height;
        Texture.Format = Format;
        Texture.Mips.emplace_back(static_cast<size64>(Width) * Height * BytesPerPixel);
        return Texture;
    }

    void SetPixel(TVector<byte8>& Pixels, const uint32 Width, const uint32 X, const uint32 Y, const FColor Color)
    {
        byte8* Pixel = Pixels.data() + (static_cast<size64>(Y) * Width + X) * BytesPerPixel;
        Pixel[0] = Color.R;
        Pixel[1] = Color.G;
        Pixel[2] = Color.B;
        Pixel[3] = Color.A;
    }
}

namespace TextureGenerators
{
    FTextureBuilder Solid(const FColor Color, const EGfxFormat Format)
    {
        FTextureBuilder Texture = MakeTexture(1, 1, Format);
        SetPixel(Texture.Mips[0], 1, 0, 0, Color);
        return Texture;
    }

    FTextureBuilder Checker(const uint32 Size, const uint32 Cells, const FColor A, const FColor B)
    {
        FTextureBuilder Texture = MakeTexture(Size, Size, EGfxFormat::RGBA8_SRGB);
        const uint32 CellSize = Size / (Cells > 0 ? Cells : 1);
        for (uint32 Y = 0; Y < Size; ++Y)
        {
            for (uint32 X = 0; X < Size; ++X)
            {
                const bool8 bA = ((X / CellSize) + (Y / CellSize)) % 2 == 0;
                SetPixel(Texture.Mips[0], Size, X, Y, bA ? A : B);
            }
        }
        GenerateMips(Texture);
        return Texture;
    }

    FTextureBuilder BumpNormalMap(const uint32 Size, const uint32 Bumps, const float32 Strength)
    {
        FTextureBuilder Texture = MakeTexture(Size, Size, EGfxFormat::RGBA8_UNORM);
        const float32 Frequency = static_cast<float32>(Bumps > 0 ? Bumps : 1);

        for (uint32 Y = 0; Y < Size; ++Y)
        {
            for (uint32 X = 0; X < Size; ++X)
            {
                const float32 U = (static_cast<float32>(X) + 0.5f) / static_cast<float32>(Size);
                const float32 V = (static_cast<float32>(Y) + 0.5f) / static_cast<float32>(Size);
                const float32 SinU = Math::Sin(Math::Pi * Frequency * U);
                const float32 SinV = Math::Sin(Math::Pi * Frequency * V);
                const float32 CosU = Math::Cos(Math::Pi * Frequency * U);
                const float32 CosV = Math::Cos(Math::Pi * Frequency * V);

                const float32 DhDu = 2.0f * SinU * CosU * SinV * SinV * Strength;
                const float32 DhDv = 2.0f * SinV * CosV * SinU * SinU * Strength;
                const FVector3 Normal = FVector3{.X = -DhDu, .Y = -DhDv, .Z = 1.0f}.GetNormalized();

                SetPixel(Texture.Mips[0], Size, X, Y, {
                             .R = ToByte(Normal.X * 0.5f + 0.5f),
                             .G = ToByte(Normal.Y * 0.5f + 0.5f),
                             .B = ToByte(Normal.Z * 0.5f + 0.5f),
                             .A = 255,
                         });
            }
        }
        GenerateMips(Texture, true);
        return Texture;
    }

    void GenerateMips(FTextureBuilder& Texture, const bool8 bNormalMap)
    {
        const bool8 bSrgb = IsSrgb(Texture.Format);
        Texture.Mips.resize(1);

        for (uint32 Mip = 1;; ++Mip)
        {
            const uint32 SourceWidth = FTextureBuilder::GetMipSize(Texture.Width, Mip - 1);
            const uint32 SourceHeight = FTextureBuilder::GetMipSize(Texture.Height, Mip - 1);
            if (SourceWidth == 1 && SourceHeight == 1)
            {
                break;
            }

            const uint32 Width = FTextureBuilder::GetMipSize(Texture.Width, Mip);
            const uint32 Height = FTextureBuilder::GetMipSize(Texture.Height, Mip);
            TVector<byte8> Pixels(static_cast<size64>(Width) * Height * BytesPerPixel);
            const TVector<byte8>& Source = Texture.Mips[Mip - 1];

            for (uint32 Y = 0; Y < Height; ++Y)
            {
                for (uint32 X = 0; X < Width; ++X)
                {
                    float32 Sum[4] = {};
                    for (uint32 Sample = 0; Sample < 4; ++Sample)
                    {
                        const uint32 SourceX = Math::Min(X * 2 + (Sample & 1), SourceWidth - 1);
                        const uint32 SourceY = Math::Min(Y * 2 + (Sample >> 1), SourceHeight - 1);
                        const byte8* Pixel = Source.data() + (static_cast<size64>(SourceY) * SourceWidth + SourceX) * BytesPerPixel;
                        for (uint32 Channel = 0; Channel < 4; ++Channel)
                        {
                            float32 Value = static_cast<float32>(Pixel[Channel]) / 255.0f;
                            if (bSrgb && Channel < 3)
                            {
                                Value = MathDetail::SrgbToLinear(Value);
                            }
                            else if (bNormalMap && Channel < 3)
                            {
                                Value = Value * 2.0f - 1.0f;
                            }
                            Sum[Channel] += Value * 0.25f;
                        }
                    }

                    if (bNormalMap)
                    {
                        const FVector3 Normal = FVector3{.X = Sum[0], .Y = Sum[1], .Z = Sum[2]}.GetSafeNormal();
                        Sum[0] = Normal.X * 0.5f + 0.5f;
                        Sum[1] = Normal.Y * 0.5f + 0.5f;
                        Sum[2] = Normal.Z * 0.5f + 0.5f;
                    }
                    else if (bSrgb)
                    {
                        for (uint32 Channel = 0; Channel < 3; ++Channel)
                        {
                            Sum[Channel] = MathDetail::LinearToSrgb(Sum[Channel]);
                        }
                    }

                    byte8* Pixel = Pixels.data() + (static_cast<size64>(Y) * Width + X) * BytesPerPixel;
                    for (uint32 Channel = 0; Channel < 4; ++Channel)
                    {
                        Pixel[Channel] = ToByte(Sum[Channel]);
                    }
                }
            }
            Texture.Mips.push_back(std::move(Pixels));
        }
    }
}
