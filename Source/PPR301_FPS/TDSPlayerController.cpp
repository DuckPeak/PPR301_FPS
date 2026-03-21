#include "TDSPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"

ATDSPlayerController::ATDSPlayerController()
{
    bShowMouseCursor = false;
    bIsBuildMode = false;
    SelectedBuildClass = nullptr; // default: nothing selected
    PreviewActor = nullptr;
    CameraSpeed = 2000.f;
    GridSize = 200.f;
    CurrentRotation = 0.f;
}

void ATDSPlayerController::BeginPlay()
{
    Super::BeginPlay();

    // Spawn build camera
    BuildCamera = GetWorld()->SpawnActor<ACameraActor>(
        FVector(0, 0, 1500),
        FRotator(-90, 0, 0)
    );
}

void ATDSPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    // Toggle build mode (hard-coded to Tab in editor Input)
    if (InputComponent)
    {
        InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &ATDSPlayerController::ToggleBuildMode);
    }
}

void ATDSPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsBuildMode && BuildCamera)
    {
        MoveCamera(DeltaTime);
        UpdatePreview();

        // Hard-coded Enter key placement
        if (WasInputKeyJustPressed(EKeys::Enter))
        {
            PlacePreviewedObject();
        }
    }
}

// ===== BUILD MODE =====

void ATDSPlayerController::ToggleBuildMode()
{
    bIsBuildMode = !bIsBuildMode;
    UE_LOG(LogTemp, Warning, TEXT("TOGGLE BUILD MODE"));

    if (bIsBuildMode)
    {
        // Switch to build camera
        SetViewTargetWithBlend(BuildCamera, 0.3f);
        bShowMouseCursor = true;
        SetInputMode(FInputModeGameAndUI());

        // Show build menu
        if (BuildMenuClass && !BuildMenuInstance)
        {
            BuildMenuInstance = CreateWidget<UUserWidget>(this, BuildMenuClass);
            if (BuildMenuInstance)
            {
                BuildMenuInstance->AddToViewport();
            }
        }
    }
    else
    {
        // Switch back to player camera
        SetViewTargetWithBlend(GetPawn(), 0.3f);
        bShowMouseCursor = false;
        SetInputMode(FInputModeGameOnly());

        // Remove build menu
        if (BuildMenuInstance)
        {
            BuildMenuInstance->RemoveFromParent();
            BuildMenuInstance = nullptr;
        }

        // Destroy any preview actor when exiting build mode
        if (PreviewActor)
        {
            PreviewActor->Destroy();
            PreviewActor = nullptr;
        }

        SelectedBuildClass = nullptr;
    }
}

// ===== CAMERA =====

void ATDSPlayerController::MoveCamera(float DeltaTime)
{
    FVector Dir = FVector::ZeroVector;

    if (IsInputKeyDown(EKeys::W)) Dir.X += 1;
    if (IsInputKeyDown(EKeys::S)) Dir.X -= 1;
    if (IsInputKeyDown(EKeys::D)) Dir.Y += 1;
    if (IsInputKeyDown(EKeys::A)) Dir.Y -= 1;

    FVector NewLoc = BuildCamera->GetActorLocation() + Dir * CameraSpeed * DeltaTime;
    BuildCamera->SetActorLocation(NewLoc);
}

void ATDSPlayerController::ZoomCamera(const FInputActionValue& Value)
{
    float Axis = Value.Get<float>();
    FVector Loc = BuildCamera->GetActorLocation();
    Loc.Z = FMath::Clamp(Loc.Z - Axis * 300.f, 600.f, 3000.f);
    BuildCamera->SetActorLocation(Loc);
}

// ===== GRID =====

FVector ATDSPlayerController::GetMouseWorldPosition()
{
    FVector Pos, Dir;
    if (DeprojectMousePositionToWorld(Pos, Dir))
    {
        FHitResult Hit;
        GetWorld()->LineTraceSingleByChannel(Hit, Pos, Pos + Dir * 10000.f, ECC_Visibility);
        if (Hit.bBlockingHit)
            return Hit.Location;
    }
    return FVector::ZeroVector;
}

FVector ATDSPlayerController::SnapToGrid(FVector L)
{
    return FVector(
        FMath::GridSnap(L.X, GridSize),
        FMath::GridSnap(L.Y, GridSize),
        L.Z
    );
}

// ===== BUILD SYSTEM =====

bool ATDSPlayerController::CheckValidPlacement(FVector Pos)
{
    FCollisionShape Box = FCollisionShape::MakeBox(FVector(100.f));
    return !GetWorld()->OverlapAnyTestByChannel(Pos, FQuat::Identity, ECC_WorldStatic, Box);
}

void ATDSPlayerController::UpdatePreview()
{
    if (!SelectedBuildClass) return;

    FVector Pos = SnapToGrid(GetMouseWorldPosition());
    if (!PreviewActor)
    {
        PreviewActor = GetWorld()->SpawnActor<AActor>(SelectedBuildClass, Pos, FRotator(0.f, CurrentRotation, 0.f));
        PreviewActor->SetActorEnableCollision(false);
    }

    if (PreviewActor)
    {
        PreviewActor->SetActorLocation(Pos);
        PreviewActor->SetActorRotation(FRotator(0.f, CurrentRotation, 0.f));
    }
}

void ATDSPlayerController::PlacePreviewedObject()
{
    if (PreviewActor && SelectedBuildClass)
    {
        PlaceTurret();
    }
}

void ATDSPlayerController::PlaceTurret()
{
    if (!SelectedBuildClass) return;

    FVector Pos = SnapToGrid(GetMouseWorldPosition());
    if (!CheckValidPlacement(Pos)) return;

    GetWorld()->SpawnActor<AActor>(SelectedBuildClass, Pos, FRotator(0.f, CurrentRotation, 0.f));

    if (PreviewActor)
    {
        PreviewActor->Destroy();
        PreviewActor = nullptr;
    }

    SelectedBuildClass = nullptr;
}

void ATDSPlayerController::SetSelectedBuild(TSubclassOf<AActor> NewClass)
{
    SelectedBuildClass = NewClass;

    if (PreviewActor)
    {
        PreviewActor->Destroy();
        PreviewActor = nullptr;
    }
}