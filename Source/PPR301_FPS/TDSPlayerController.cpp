#include "TDSPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ATDSPlayerController::ATDSPlayerController()
{
    bShowMouseCursor = false;
    bIsBuildMode = false;
    SelectedBuildClass = nullptr;
    PreviewActor = nullptr;
    CameraSpeed = 2000.f;
    GridSize = 200.f;
    CurrentRotation = 0.f;
}

void ATDSPlayerController::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("[BuildMode] BeginPlay"));

    BuildCamera = GetWorld()->SpawnActor<ACameraActor>(
        FVector(0, 0, 1500),
        FRotator(-90, 0, 0)
    );

    if (!BuildCamera)
    {
        UE_LOG(LogTemp, Error, TEXT("[BuildMode] Failed to spawn BuildCamera!"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildMode] BuildCamera spawned at %s"), *BuildCamera->GetActorLocation().ToString());
    }
}

void ATDSPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (InputComponent)
    {
        // Hard-coded toggle build mode to TAB
        InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &ATDSPlayerController::ToggleBuildMode);
        
        InputComponent->BindKey(EKeys::Q, IE_Pressed, this, &ATDSPlayerController::RotatePreviewLeft);
        InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ATDSPlayerController::RotatePreviewRight);
    }
}
void ATDSPlayerController::RotatePreviewLeft()
{
    CurrentRotation -= 90.f; // rotate 90 degrees left
    if (PreviewActor)
    {
        PreviewActor->SetActorRotation(FRotator(0.f, CurrentRotation, 0.f));
    }
}

void ATDSPlayerController::RotatePreviewRight()
{
    CurrentRotation += 90.f; // rotate 90 degrees right
    if (PreviewActor)
    {
        PreviewActor->SetActorRotation(FRotator(0.f, CurrentRotation, 0.f));
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
            UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Enter pressed"));
            PlacePreviewedObject();
        }
    }
}

// ===== BUILD MODE =====

void ATDSPlayerController::ToggleBuildMode()
{
    bIsBuildMode = !bIsBuildMode;
    UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Toggled build mode: %s"), bIsBuildMode ? TEXT("ON") : TEXT("OFF"));

    if (bIsBuildMode)
    {
        SetViewTargetWithBlend(BuildCamera, 0.3f);
        bShowMouseCursor = true;
        SetInputMode(FInputModeGameAndUI());

        if (BuildMenuClass && !BuildMenuInstance)
        {
            BuildMenuInstance = CreateWidget<UUserWidget>(this, BuildMenuClass);
            if (BuildMenuInstance)
            {
                BuildMenuInstance->AddToViewport();
                UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Build menu added to viewport"));
            }
        }
    }
    else
    {
        SetViewTargetWithBlend(GetPawn(), 0.3f);
        bShowMouseCursor = false;
        SetInputMode(FInputModeGameOnly());

        if (BuildMenuInstance)
        {
            BuildMenuInstance->RemoveFromParent();
            BuildMenuInstance = nullptr;
            UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Build menu removed"));
        }

        if (PreviewActor)
        {
            PreviewActor->Destroy();
            PreviewActor = nullptr;
            UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Preview actor destroyed on exit"));
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

// ===== GRID =====

FVector ATDSPlayerController::GetMouseWorldPosition()
{
    FVector Pos, Dir;
    if (DeprojectMousePositionToWorld(Pos, Dir))
    {
        FHitResult Hit;
        GetWorld()->LineTraceSingleByChannel(Hit, Pos, Pos + Dir * 10000.f, ECC_Visibility);
        if (Hit.bBlockingHit)
        {
            //UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Mouse world hit at: %s"), *Hit.Location.ToString());

            // Draw debug line
            //DrawDebugLine(GetWorld(), Pos, Hit.Location, FColor::Green, false, 1.f, 0, 2.f);
            //DrawDebugSphere(GetWorld(), Hit.Location, 25.f, 12, FColor::Green, false, 1.f);

            return Hit.Location;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Mouse did not hit world"));
    return FVector::ZeroVector;
}

FVector ATDSPlayerController::SnapToGrid(FVector L)
{
    FVector Snapped(
        FMath::GridSnap(L.X, GridSize),
        FMath::GridSnap(L.Y, GridSize),
        L.Z
    );
    //UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Snapped position: %s"), *Snapped.ToString());
    return Snapped;
}

// ===== BUILD SYSTEM =====

bool ATDSPlayerController::CheckValidPlacement(FVector Pos)
{
    //FCollisionShape Box = FCollisionShape::MakeBox(FVector(100.f));
    //bool bValid = !GetWorld()->OverlapAnyTestByChannel(Pos, FQuat::Identity, ECC_WorldStatic, Box);
    //UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Placement at %s is %s"), *Pos.ToString(), bValid ? TEXT("VALID") : TEXT("INVALID"));
    //return bValid;
    return true;
}

void ATDSPlayerController::UpdatePreview()
{
    if (!SelectedBuildClass) return;
    
    // Snap to grid
    //FVector Pos = SnapToGrid(GetMouseWorldPosition());
    
    // Non SNap
    FVector Pos = GetMouseWorldPosition();

    if (!PreviewActor)
    {
        PreviewActor = GetWorld()->SpawnActor<AActor>(SelectedBuildClass, Pos, FRotator(0.f, CurrentRotation, 0.f));
        if (PreviewActor)
        {
            PreviewActor->SetActorEnableCollision(false);
            //UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Preview actor spawned at: %s"), *Pos.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[BuildMode] Failed to spawn preview actor!"));
        }
    }

    if (PreviewActor)
    {
        PreviewActor->SetActorLocation(Pos);
        PreviewActor->SetActorRotation(FRotator(0.f, CurrentRotation, 0.f));

        // Apply ghost material
        TArray<UStaticMeshComponent*> MeshComponents;
        PreviewActor->GetComponents<UStaticMeshComponent>(MeshComponents);
        for (UStaticMeshComponent* MeshComp : MeshComponents)
        {
            if (GhostMaterial)
            {
                MeshComp->SetMaterial(0, GhostMaterial);
                MeshComp->SetRenderCustomDepth(true); // optional for outline
            }
        }
    }
}

void ATDSPlayerController::PlacePreviewedObject()
{
    if (!PreviewActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildMode] No preview actor to place!"));
        return;
    }

    if (!SelectedBuildClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildMode] No selected build class!"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Placing previewed object"));
    PlaceTurret();
}

void ATDSPlayerController::PlaceTurret()
{
    if (!SelectedBuildClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[BuildMode] No SelectedBuildClass!"));
        return;
    }

    FVector Pos = GetMouseWorldPosition();
    Pos.Z += 5.f;

    UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Attempting to place at: %s"), *Pos.ToString());

    DrawDebugBox(GetWorld(), Pos, FVector(50.f,50.f,50.f), FColor::Red, false, 5.f);

    AActor* Placed = GetWorld()->SpawnActor<AActor>(SelectedBuildClass, Pos, FRotator(0.f, CurrentRotation, 0.f));
    if (Placed)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Successfully placed actor at: %s"), *Placed->GetActorLocation().ToString());

        if (PreviewActor)
        {
            PreviewActor->Destroy();
            PreviewActor = nullptr;
            UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Preview actor destroyed after placement"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[BuildMode] Failed to spawn actor at %s"), *Pos.ToString());
    }
}

void ATDSPlayerController::SetSelectedBuild(TSubclassOf<AActor> NewClass)
{
    SelectedBuildClass = NewClass;

    if (PreviewActor)
    {
        PreviewActor->Destroy();
        PreviewActor = nullptr;
    }

    if (SelectedBuildClass && bIsBuildMode)
    {
        // Spawn preview immediately at current mouse location
        //FVector Pos = SnapToGrid(GetMouseWorldPosition()); // Snap
        
        //Non Snap
        FVector Pos = GetMouseWorldPosition();
        
        PreviewActor = GetWorld()->SpawnActor<AActor>(SelectedBuildClass, Pos, FRotator(0.f, CurrentRotation, 0.f));
        if (SelectedBuildClass && bIsBuildMode)
        {
            UpdatePreview();
        }
        if (PreviewActor)
        {
            PreviewActor->SetActorEnableCollision(false);
            UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Preview actor spawned immediately after selection"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[BuildMode] Failed to spawn preview actor on selection!"));
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Selected build class set: %s"), *GetNameSafe(NewClass));
}