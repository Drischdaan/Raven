// Raven Storm Studio @ 2026. All Rights Reserved.

#include "Demo/DemoScene.hpp"

#include "Renderer/Renderer.hpp"
#include "Renderer/Mesh/MeshGenerators.hpp"
#include "Core/Math/MathUtility.hpp"

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

    RESULT_CHECK_ASSIGN(Scene.AddInstance({
                            .Mesh = PlaneMesh,
                            .Transform = FMatrix4x4::MakeScale({.X = 24.0f, .Y = 1.0f, .Z = 24.0f}),
                            .Color = {.R = 0.55f, .G = 0.55f, .B = 0.6f, .A = 1.0f},
                            }), GroundInstance);
    Ground = GroundInstance;

    constexpr int32 GridHalfSize = 3;
    for (int32 Z = -GridHalfSize; Z <= GridHalfSize; ++Z)
    {
        for (int32 X = -GridHalfSize; X <= GridHalfSize; ++X)
        {
            const bool8 bCube = ((X + Z) & 1) == 0;
            const FVector3 Position = {.X = static_cast<float32>(X) * 2.0f, .Y = 0.75f, .Z = static_cast<float32>(Z) * 2.0f};
            const FLinearColor Color = {
                .R = 0.35f + 0.1f * static_cast<float32>(X + GridHalfSize),
                .G = 0.45f,
                .B = 0.35f + 0.1f * static_cast<float32>(Z + GridHalfSize),
                .A = 1.0f,
            };

            RESULT_CHECK_ASSIGN(Scene.AddInstance({
                                    .Mesh = bCube ? CubeMesh : SphereMesh,
                                    .Transform = FMatrix4x4::MakeTranslation(Position),
                                    .Color = Color,
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
