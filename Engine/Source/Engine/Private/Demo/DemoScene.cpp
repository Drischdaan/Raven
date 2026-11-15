// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Demo/DemoScene.hpp"

#include "Renderer/Renderer.hpp"
#include "Renderer/Mesh/MeshGenerators.hpp"
#include "Renderer/Texture/TextureGenerators.hpp"
#include "Core/Math/MathUtility.hpp"

TResult<FTextureHandle> FDemoScene::AddTexture(FTextureBuilder Builder, const FStringView Name)
{
    RESULT_CHECK_ASSIGN(Renderer->CreateTexture(Builder.GetData(), Name), Texture);
    Textures.push_back(Texture);
    return Texture;
}

TResult<FMaterialHandle> FDemoScene::AddMaterial(const FMaterialDesc& Desc, const FStringView Name)
{
    RESULT_CHECK_ASSIGN(Renderer->CreateMaterial(Desc, Name), Material);
    Materials.push_back(Material);
    return Material;
}

FResult FDemoScene::Initialize(FRenderer& InRenderer)
{
    Renderer = &InRenderer;
    FRenderScene& Scene = Renderer->GetScene();

    RESULT_CHECK_ASSIGN(Renderer->CreateMesh(MeshGenerators::Cube().GetData(), "Cube"), Cube);
    RESULT_CHECK_ASSIGN(Renderer->CreateMesh(MeshGenerators::Sphere().GetData(), "Sphere"), Sphere);
    RESULT_CHECK_ASSIGN(Renderer->CreateMesh(MeshGenerators::Plane(0.5f, 4).GetData(), "Plane"), Plane);
    CubeMesh = Cube;
    SphereMesh = Sphere;
    PlaneMesh = Plane;

    RESULT_CHECK_ASSIGN(AddTexture(TextureGenerators::Checker(512, 16, {.R = 150, .G = 150, .B = 155}, {.R = 105, .G = 105, .B = 112}), "GroundChecker"), GroundColor);
    RESULT_CHECK_ASSIGN(AddTexture(TextureGenerators::BumpNormalMap(512, 16, 1.5f), "GroundBumps"), GroundNormal);
    RESULT_CHECK_ASSIGN(AddTexture(TextureGenerators::Checker(256, 4, {.R = 235, .G = 180, .B = 120}, {.R = 190, .G = 120, .B = 70}), "CrateChecker"), CrateColor);
    RESULT_CHECK_ASSIGN(AddTexture(TextureGenerators::BumpNormalMap(256, 4, 2.0f), "CrateBumps"), CrateNormal);
    RESULT_CHECK_ASSIGN(AddTexture(TextureGenerators::Checker(256, 8, {.R = 90, .G = 200, .B = 120, .A = 255}, {.A = 0}), "Holes"), HolesColor);

    RESULT_CHECK_ASSIGN(AddMaterial({
                            .Roughness = 0.9f,
                            .BaseColorTexture = GroundColor,
                            .NormalTexture = GroundNormal,
                            }, "Ground"), GroundMaterial);
    RESULT_CHECK_ASSIGN(AddMaterial({
                            .Roughness = 0.6f,
                            .BaseColorTexture = CrateColor,
                            .NormalTexture = CrateNormal,
                            }, "Crate"), CrateMaterial);
    RESULT_CHECK_ASSIGN(AddMaterial({
                            .BaseColorTexture = HolesColor,
                            .BlendMode = EBlendMode::Masked,
                            .Flags = EMaterialFlags::TwoSided,
                            }, "Holes"), HolesMaterial);
    RESULT_CHECK_ASSIGN(AddMaterial({
                            .BaseColor = {.R = 0.1f, .G = 0.1f, .B = 0.1f, .A = 1.0f},
                            .Emissive = {.R = 4.0f, .G = 1.6f, .B = 0.4f, .A = 1.0f},
                            }, "Glow"), GlowMaterial);

    RESULT_CHECK_ASSIGN(Scene.AddInstance({
                            .Mesh = PlaneMesh,
                            .Material = GroundMaterial,
                            .Transform = FMatrix4x4::MakeScale({.X = 24.0f, .Y = 1.0f, .Z = 24.0f}),
                            }), GroundInstance);
    Ground = GroundInstance;

    constexpr int32 GridHalfSize = 3;
    for (int32 Z = -GridHalfSize; Z <= GridHalfSize; ++Z)
    {
        for (int32 X = -GridHalfSize; X <= GridHalfSize; ++X)
        {
            const bool8 bCube = ((X + Z) & 1) == 0;
            FMaterialHandle Material;
            if (bCube)
            {
                Material = X == 0 && Z == 0 ? HolesMaterial : CrateMaterial;
            }
            else if (X == 0 && Z == -1)
            {
                Material = GlowMaterial;
            }
            else
            {
                const float32 Roughness = Math::Clamp(static_cast<float32>(X + GridHalfSize) / static_cast<float32>(2 * GridHalfSize), 0.05f, 1.0f);
                const bool8 bMetal = Z > 0;
                RESULT_CHECK_ASSIGN(AddMaterial({
                                        .BaseColor = bMetal ? FLinearColor{.R = 0.95f, .G = 0.75f, .B = 0.35f, .A = 1.0f}
                                                            : FLinearColor{.R = 0.2f, .G = 0.35f, .B = 0.8f, .A = 1.0f},
                                        .Metallic = bMetal ? 1.0f : 0.0f,
                                        .Roughness = Roughness,
                                        }, String::Format("Sphere_{}_{}", X, Z)), SphereMaterial);
                Material = SphereMaterial;
            }

            const FVector3 Position = {.X = static_cast<float32>(X) * 2.0f, .Y = 0.75f, .Z = static_cast<float32>(Z) * 2.0f};
            RESULT_CHECK_ASSIGN(Scene.AddInstance({
                                    .Mesh = bCube ? CubeMesh : SphereMesh,
                                    .Material = Material,
                                    .Transform = FMatrix4x4::MakeTranslation(Position),
                                    }), Instance);
            Spinners.push_back({.Instance = Instance, .Position = Position, .Speed = 0.5f + 0.1f * static_cast<float32>(Spinners.size() % 7)});
        }
    }
    return RESULT_OK_EMPTY();
}

void FDemoScene::Deinitialize()
{
    if (Renderer == nullptr)
    {
        return;
    }

    FRenderScene& Scene = Renderer->GetScene();
    for (const FSpinner& Spinner : Spinners)
    {
        Scene.RemoveInstance(Spinner.Instance);
    }
    Scene.RemoveInstance(Ground);
    Spinners.clear();

    for (const FMaterialHandle Material : Materials)
    {
        Renderer->DestroyMaterial(Material);
    }
    for (const FTextureHandle Texture : Textures)
    {
        Renderer->DestroyTexture(Texture);
    }
    Materials.clear();
    Textures.clear();

    Renderer->DestroyMesh(CubeMesh);
    Renderer->DestroyMesh(SphereMesh);
    Renderer->DestroyMesh(PlaneMesh);
    Renderer = nullptr;
}

FRenderView FDemoScene::Update(const float32 Time)
{
    FRenderScene& Scene = Renderer->GetScene();
    for (const FSpinner& Spinner : Spinners)
    {
        const FQuat Rotation = FQuat::FromAxisAngle(FVector3::UnitY(), Time * Spinner.Speed)
            * FQuat::FromAxisAngle(FVector3::UnitX(), Time * Spinner.Speed * 0.5f);
        Scene.SetTransform(Spinner.Instance, FMatrix4x4::MakeTRS(Spinner.Position, Rotation, FVector3::One()));
    }

    const float32 Angle = Time * 0.2f;
    const FVector3 Eye = {.X = Math::Sin(Angle) * 14.0f, .Y = 6.0f, .Z = -Math::Cos(Angle) * 14.0f};
    return {
        .ViewMatrix = FMatrix4x4::MakeLookAt(Eye, FVector3::Zero(), FVector3::UnitY()),
        .Position = Eye,
        .VerticalFov = Math::DegreesToRadians(60.0f),
        .NearPlane = 0.1f,
    };
}
