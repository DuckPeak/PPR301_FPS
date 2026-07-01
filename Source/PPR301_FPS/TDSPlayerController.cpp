#include "TDSPlayerController.h"
#include "Engine/World.h"
#include "Turret.h"
#include "ISellable.h"
#include "GameFramework/Pawn.h"
#include "Components/TextBlock.h"
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

    // Spawn the build camera — position will be set properly in ToggleBuildMode
    // when we actually have a valid pawn. Use a fallback location for now.
    BuildCamera = GetWorld()->SpawnActor<ACameraActor>(
        FVector(0.f, 0.f, 1500.f),
        FRotator(-90.f, 0.f, 0.f)
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
        // Toggle build mode
        InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &ATDSPlayerController::ToggleBuildMode);

        // Rotate preview
        InputComponent->BindKey(EKeys::Q, IE_Pressed, this, &ATDSPlayerController::RotatePreviewLeft);
        InputComponent->BindKey(EKeys::E, IE_Pressed, this, &ATDSPlayerController::RotatePreviewRight);

        // Left mouse button places the selected object
        InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ATDSPlayerController::PlacePreviewedObject);
    }
}

void ATDSPlayerController::RotatePreviewLeft()
{
    CurrentRotation -= 90.f;
    if (PreviewActor)
    {
        PreviewActor->SetActorRotation(FRotator(0.f, CurrentRotation, 0.f));
    }
}

void ATDSPlayerController::RotatePreviewRight()
{
    CurrentRotation += 90.f;
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
        HandleZoom();
    }
}

// ===== CAMERA =====

void ATDSPlayerController::HandleZoom()
{
    float ScrollDelta = GetInputAnalogKeyState(EKeys::MouseWheelAxis);

    if (FMath::Abs(ScrollDelta) > 0.01f)
    {
        FVector CamLoc = BuildCamera->GetActorLocation();

        // Scroll up (positive) zooms in by lowering Z; scroll down raises Z
        CamLoc.Z = FMath::Clamp(CamLoc.Z - ScrollDelta * ZoomSpeed, MinCameraHeight, MaxCameraHeight);

        BuildCamera->SetActorLocation(CamLoc);

        UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Camera zoom - new height: %.1f"), CamLoc.Z);
    }
}

// ===== BUILD MODE =====

void ATDSPlayerController::ToggleBuildMode()
{
    bIsBuildMode = !bIsBuildMode;
    UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Toggled build mode: %s"), bIsBuildMode ? TEXT("ON") : TEXT("OFF"));

    if (bIsBuildMode)
    {
        // Reposition build camera directly above the player's current XY position
        if (BuildCamera && GetPawn())
        {
            FVector PawnLoc = GetPawn()->GetActorLocation();
            BuildCamera->SetActorLocation(FVector(PawnLoc.X, PawnLoc.Y, 1500.f));
            BuildCamera->SetActorRotation(FRotator(-90.f, 0.f, 0.f));
            UE_LOG(LogTemp, Warning, TEXT("[BuildMode] BuildCamera repositioned above player at: %s"), *BuildCamera->GetActorLocation().ToString());
        }

        // Disable pawn input so the shoot binding on the pawn never sees mouse clicks
        if (GetPawn())
        {
            GetPawn()->DisableInput(this);
            UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Pawn input disabled"));
        }

        SetViewTargetWithBlend(BuildCamera, 0.3f);
        bShowMouseCursor = true;
        SetInputMode(FInputModeGameAndUI());

        if (BuildMenuClass && !BuildMenuInstance)
        {
            BuildMenuInstance = CreateWidget<UUserWidget>(this, BuildMenuClass);
            if (BuildMenuInstance)
            {
                BuildMenuInstance->AddToViewport();
                CashTextBlock = Cast<UTextBlock>(BuildMenuInstance->GetWidgetFromName(FName("CashText")));

                if (CashTextBlock)
                {
                    UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Successfully bound CashTextBlock"));
                    UpdateCashUI();
                }
                else
                {
                    UE_LOG(LogTemp, Error, TEXT("[BuildMode] Could not find TextBlock named 'CashText' in the widget!"));
                }

                UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Build menu added to viewport"));
            }
        }
    }
    else
    {
        SetViewTargetWithBlend(GetPawn(), 0.3f);
        bShowMouseCursor = false;
        SetInputMode(FInputModeGameOnly());

        // Re-enable pawn input so the player can shoot again
        if (GetPawn())
        {
            GetPawn()->EnableInput(this);
            UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Pawn input restored"));
        }

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
    return Snapped;
}

// ===== BUILD SYSTEM =====

bool ATDSPlayerController::CheckValidPlacement(FVector Pos)
{
    return true;
}

void ATDSPlayerController::UpdatePreview()
{
    // Guard against null or GC'd UClass pointer — the source of the segfault.
    // IsValidLowLevel() checks the UObject header is intact before we dereference.
    if (!SelectedBuildClass) return;
    if (!IsValid(SelectedBuildClass)) { SelectedBuildClass = nullptr; return; }

    FVector Pos = GetMouseWorldPosition();
    // If the mouse didn't hit anything, don't try to move/spawn the preview
    if (Pos.IsZero()) return;

    if (!PreviewActor)
    {
        FTransform SpawnTransform(FRotator(0.f, CurrentRotation, 0.f), Pos);

        AActor* Spawned = GetWorld()->SpawnActorDeferred<AActor>(
            SelectedBuildClass,
            SpawnTransform,
            nullptr,
            nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );

        if (Spawned)
        {
            ATurret* Turret = Cast<ATurret>(Spawned);
            if (Turret)
            {
                Turret->bIsPreview = true;
            }

            Spawned->SetActorEnableCollision(false);
            Spawned->SetActorTickEnabled(false);

            PreviewActor = UGameplayStatics::FinishSpawningActor(Spawned, SpawnTransform);
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

        // Apply ghost material to all static mesh components
        TArray<UStaticMeshComponent*> MeshComponents;
        PreviewActor->GetComponents<UStaticMeshComponent>(MeshComponents);
        for (UStaticMeshComponent* MeshComp : MeshComponents)
        {
            if (GhostMaterial)
            {
                MeshComp->SetMaterial(0, GhostMaterial);
                MeshComp->SetRenderCustomDepth(true);
            }
        }
    }
}

// PlacePreviewedObject sell vs place
void ATDSPlayerController::PlacePreviewedObject()
{
    if (!bIsBuildMode) return;

    // Sell mode: click sells the actor under the cursor
    if (bIsSellMode)
    {
        SellActorUnderCursor();
        return;
    }

    // Normal placement
    if (!PreviewActor || !IsValid(PreviewActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildMode] No valid preview actor to place!"));
        PreviewActor = nullptr;
        return;
    }

    if (!SelectedBuildClass || !IsValid(SelectedBuildClass))
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildMode] No valid selected build class!"));
        SelectedBuildClass = nullptr;
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Placing previewed object"));
    PlaceTurret();
}

void ATDSPlayerController::PlaceTurret()
{
    if (!SelectedBuildClass || !IsValid(SelectedBuildClass))
    {
        UE_LOG(LogTemp, Error, TEXT("[BuildMode] No valid SelectedBuildClass!"));
        SelectedBuildClass = nullptr;
        return;
    }

    FVector Pos = GetMouseWorldPosition();
    Pos.Z += 5.f;

    UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Attempting to place at: %s"), *Pos.ToString());

    // Get cost from class default object
    AActor* DefaultObj = SelectedBuildClass->GetDefaultObject<AActor>();
    ATurret* TurretCDO = Cast<ATurret>(DefaultObj);

    int32 Cost = 0;
    if (TurretCDO)
    {
        Cost = TurretCDO->Cost;
    }
    else
    {
        Cost = 50;
    }

    // Check if player can afford
    if (PlayerCash < Cost)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Not enough money! Need %d, have %d"), Cost, PlayerCash);
        return;
    }

    // Deduct money and update UI
    PlayerCash -= Cost;
    UpdateCashUI();
    UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Spent %d, Remaining: %d"), Cost, PlayerCash);

    // Spawn the real actor
    AActor* Placed = GetWorld()->SpawnActor<AActor>(
        SelectedBuildClass,
        Pos,
        FRotator(0.f, CurrentRotation, 0.f)
    );

    if (Placed)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Successfully placed actor at: %s"), *Placed->GetActorLocation().ToString());

        if (PlaceTurretSound)
        {
            UGameplayStatics::PlaySound2D(this, PlaceTurretSound, 1.0f, 1.0f);
            UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Played turret placement sound"));
        }

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
    
    // Exiting sell mode when the player picks something to place
    if (bIsSellMode)
    {
        bIsSellMode = false;
        UE_LOG(LogTemp, Warning, TEXT("[SellMode] Exited sell mode via build selection"));
    }
    
    
    // Validate before storing — never store a stale/invalid UClass
    if (NewClass && !IsValid(NewClass))
    {
        UE_LOG(LogTemp, Error, TEXT("[BuildMode] SetSelectedBuild received invalid class, ignoring!"));
        return;
    }

    SelectedBuildClass = NewClass;

    if (PreviewActor)
    {
        PreviewActor->Destroy();
        PreviewActor = nullptr;
    }

    if (SelectedBuildClass && IsValid(SelectedBuildClass) && bIsBuildMode)
    {
        FVector Pos = GetMouseWorldPosition();
        FTransform SpawnTransform(FRotator(0.f, CurrentRotation, 0.f), Pos);

        AActor* Spawned = GetWorld()->SpawnActorDeferred<AActor>(
            SelectedBuildClass,
            SpawnTransform,
            nullptr,
            nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );

        if (Spawned)
        {
            ATurret* Turret = Cast<ATurret>(Spawned);
            if (Turret)
            {
                Turret->bIsPreview = true;
            }

            Spawned->SetActorEnableCollision(false);
            Spawned->SetActorTickEnabled(false);

            PreviewActor = UGameplayStatics::FinishSpawningActor(Spawned, SpawnTransform);
        }

        if (SelectedBuildClass && IsValid(SelectedBuildClass) && bIsBuildMode)
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

void ATDSPlayerController::UpdateBuildMenuCash()
{
    if (!BuildMenuInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildMode] UpdateBuildMenuCash: BuildMenuInstance is NULL"));
        return;
    }

    UFunction* UpdateFunc = BuildMenuInstance->FindFunction(FName("UpdateCash"));
    if (UpdateFunc)
    {
        BuildMenuInstance->ProcessEvent(UpdateFunc, nullptr);
        UE_LOG(LogTemp, Warning, TEXT("[BuildMode] UpdateCash function FOUND and called! Current PlayerCash = %d"), PlayerCash);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[BuildMode] UpdateCash function NOT FOUND on the widget!"));
    }
}

void ATDSPlayerController::UpdateCashUI()
{
    if (CashTextBlock)
    {
        FText NewText = FText::Format(FText::FromString(TEXT(":{0}")), FText::AsNumber(PlayerCash));
        CashTextBlock->SetText(NewText);
        UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Cash UI updated directly to %d"), PlayerCash);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildMode] CashTextBlock is null!"));
    }
}

void ATDSPlayerController::AddPlayerCash(int32 Amount)
{
    if (Amount <= 0) return;

    PlayerCash += Amount;

    UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Added %d cash. New total: %d"), Amount, PlayerCash);

    UpdateCashUI();
}

// ToggleSellMode — called by UI button
void ATDSPlayerController::ToggleSellMode()
{
    bIsSellMode = !bIsSellMode;
    UE_LOG(LogTemp, Warning, TEXT("[SellMode] Sell mode: %s"), bIsSellMode ? TEXT("ON") : TEXT("OFF"));

    // Destroy any active placement preview — the two modes are mutually exclusive
    if (bIsSellMode && PreviewActor)
    {
        PreviewActor->Destroy();
        PreviewActor = nullptr;
        SelectedBuildClass = nullptr;
        UE_LOG(LogTemp, Warning, TEXT("[SellMode] Cleared preview actor on entering sell mode"));
    }
}


// Traces the mouse ray, finds a sellable actor, refunds and destroys it
void ATDSPlayerController::SellActorUnderCursor()
{
    FVector RayOrigin, RayDir;
    if (!DeprojectMousePositionToWorld(RayOrigin, RayDir))
    {
        UE_LOG(LogTemp, Warning, TEXT("[SellMode] Could not deproject mouse position"));
        return;
    }

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetPawn());

    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        RayOrigin,
        RayOrigin + RayDir * 10000.f,
        ECC_Visibility,
        Params
    );

    if (!bHit || !Hit.GetActor())
    {
        UE_LOG(LogTemp, Warning, TEXT("[SellMode] No actor hit"));
        return;
    }

    AActor* HitActor = Hit.GetActor();

    // Works for any actor that implements ISellable — turrets, walls, microwave, anything
    if (!HitActor->Implements<USellable>())
    {
        UE_LOG(LogTemp, Warning, TEXT("[SellMode] Hit actor '%s' is not sellable"), *GetNameSafe(HitActor));
        return;
    }

    int32 Cost = ISellable::Execute_GetSellCost(HitActor);
    const int32 Refund = FMath::FloorToInt(Cost * 0.75f);

    UE_LOG(LogTemp, Warning, TEXT("[SellMode] Selling '%s' for %d (cost was %d)"),
        *GetNameSafe(HitActor), Refund, Cost);

    AddPlayerCash(Refund);
    HitActor->Destroy();
}