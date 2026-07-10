#include "TDSPlayerController.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "Turret.h"
#include "ISellable.h"
#include "IRepairable.h"
#include "GameFramework/Pawn.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TurretMicrowave.h"

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
    OnPreviewRotatedLeft.Broadcast();
}

void ATDSPlayerController::RotatePreviewRight()
{
    CurrentRotation += 90.f;
    if (PreviewActor)
    {
        PreviewActor->SetActorRotation(FRotator(0.f, CurrentRotation, 0.f));
    }
    OnPreviewRotatedRight.Broadcast();
}

void ATDSPlayerController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsBuildMode && BuildCamera)
    {
        MoveCamera(DeltaTime);
        HandleZoom();

        if (bIsSellMode)
        {
            UpdateSellHighlight();
        }
        else if (bIsRepairMode)
        {
            UpdateRepairHighlight();
        }
        else
        {
            UpdatePreview();
        }
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

    OnBuildModeToggled.Broadcast(bIsBuildMode);

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

        // Leaving build mode entirely also means leaving sell/repair mode - restore any tinted actor
        ClearSellHighlight();
        bIsSellMode = false;
        ClearRepairHighlight();
        bIsRepairMode = false;

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
    if (!GetWorld()) return false;

    if (Pos.Z > -20.f)
    {
        return false;
    }

    // Derive the overlap box from the actual PreviewActor's current bounding box -
    FVector BoxExtent = FVector(GridSize * 0.5f - 5.f); // fallback if no preview yet

    if (PreviewActor && IsValid(PreviewActor))
    {
        FBox Bounds = PreviewActor->GetComponentsBoundingBox(true); // true = only colliding components
        if (Bounds.IsValid)
        {
            // Shrink slightly so edge-adjacent pieces don't falsely flag as overlapping
            //BoxExtent = Bounds.GetExtent() * 0.9f;
            BoxExtent = Bounds.GetExtent() * 1.05f;
        }
    }

    // Bounds from GetComponentsBoundingBox is already an axis-aligned WORLD-space box,
    const FCollisionShape Box = FCollisionShape::MakeBox(BoxExtent);
    const FQuat Rot = FQuat::Identity;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetPawn());
    if (PreviewActor)
    {
        Params.AddIgnoredActor(PreviewActor);
    }

    TArray<FOverlapResult> Overlaps;
    const bool bAnyOverlap = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        Pos,
        Rot,
        ECC_Visibility,
        Box,
        Params
    );

    if (!bAnyOverlap)
    {
        return true;
    }

    for (const FOverlapResult& Result : Overlaps)
    {
        AActor* OverlappedActor = Result.GetActor();
        if (!OverlappedActor || OverlappedActor == PreviewActor)
        {
            continue;
        }

        if (OverlappedActor->Implements<USellable>())
        {
            return false;
        }
    }

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

        bIsPreviewPlacementValid = CheckValidPlacement(Pos);

        UMaterialInterface* PreviewMaterial = bIsPreviewPlacementValid ? GhostMaterial : SellHighlightMaterial;

        TArray<UStaticMeshComponent*> MeshComponents;
        PreviewActor->GetComponents<UStaticMeshComponent>(MeshComponents);
        for (UStaticMeshComponent* MeshComp : MeshComponents)
        {
            if (PreviewMaterial)
            {
                MeshComp->SetMaterial(0, PreviewMaterial);
                MeshComp->SetRenderCustomDepth(true);
            }
        }
    }
}

// PlacePreviewedObject sell vs place
void ATDSPlayerController::PlacePreviewedObject()
{
    if (!bIsBuildMode) return;

    OnPreviewPlaced.Broadcast();

    // Sell mode: click sells the actor under the cursor
    if (bIsSellMode)
    {
        SellActorUnderCursor();
        return;
    }

    // Repair mode: click repairs the actor under the cursor
    if (bIsRepairMode)
    {
        RepairActorUnderCursor();
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
    if (!bIsPreviewPlacementValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Cannot place here - overlapping another structure!"));
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

    if (!CheckValidPlacement(Pos))
    {
        UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Placement blocked - position overlaps another structure"));
        return;
    }

    Pos.Z += 5.f;

    UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Attempting to place at: %s"), *Pos.ToString());

    // Get cost from class default object
    AActor* DefaultObj = SelectedBuildClass->GetDefaultObject<AActor>();
    ATurret* TurretCDO = Cast<ATurret>(DefaultObj);
    ATurretMicrowave* MicrowaveCDO = Cast<ATurretMicrowave>(DefaultObj);

    int32 Cost = 0;
    if (TurretCDO)
    {
        Cost = TurretCDO->Cost;
    }
    else if (MicrowaveCDO)
    {
        Cost = 100;
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
    
    // Exiting sell/repair mode when the player picks something to place
    if (bIsSellMode)
    {
        bIsSellMode = false;
        ClearSellHighlight();
        UE_LOG(LogTemp, Warning, TEXT("[SellMode] Exited sell mode via build selection"));
    }

    if (bIsRepairMode)
    {
        bIsRepairMode = false;
        ClearRepairHighlight();
        UE_LOG(LogTemp, Warning, TEXT("[RepairMode] Exited repair mode via build selection"));
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

    if (!bIsSellMode)
    {
        // Leaving sell mode - restore whatever was tinted
        ClearSellHighlight();
    }

    if (bIsSellMode)
    {
        // Sell/Repair/Place are mutually exclusive - leave repair mode if it was active
        if (bIsRepairMode)
        {
            bIsRepairMode = false;
            ClearRepairHighlight();
        }

        // Destroy any active placement preview — the two modes are mutually exclusive
        if (PreviewActor)
        {
            PreviewActor->Destroy();
            PreviewActor = nullptr;
            SelectedBuildClass = nullptr;
            UE_LOG(LogTemp, Warning, TEXT("[SellMode] Cleared preview actor on entering sell mode"));
        }
    }
}

// ToggleRepairMode — called by UI button
void ATDSPlayerController::ToggleRepairMode()
{
    bIsRepairMode = !bIsRepairMode;
    UE_LOG(LogTemp, Warning, TEXT("[RepairMode] Repair mode: %s"), bIsRepairMode ? TEXT("ON") : TEXT("OFF"));

    if (!bIsRepairMode)
    {
        // Leaving repair mode - restore whatever was tinted
        ClearRepairHighlight();
    }

    if (bIsRepairMode)
    {
        // Sell/Repair/Place are mutually exclusive - leave sell mode if it was active
        if (bIsSellMode)
        {
            bIsSellMode = false;
            ClearSellHighlight();
        }

        // Destroy any active placement preview — the two modes are mutually exclusive
        if (PreviewActor)
        {
            PreviewActor->Destroy();
            PreviewActor = nullptr;
            SelectedBuildClass = nullptr;
            UE_LOG(LogTemp, Warning, TEXT("[RepairMode] Cleared preview actor on entering repair mode"));
        }
    }
}

// Restores original materials on the currently highlighted repair actor (if any) and clears state
void ATDSPlayerController::ClearRepairHighlight()
{
    if (HighlightedRepairActor && IsValid(HighlightedRepairActor))
    {
        TArray<UStaticMeshComponent*> MeshComponents;
        HighlightedRepairActor->GetComponents<UStaticMeshComponent>(MeshComponents);

        for (UStaticMeshComponent* MeshComp : MeshComponents)
        {
            if (!MeshComp) continue;

            if (TArray<UMaterialInterface*>* Originals = OriginalRepairMaterials.Find(MeshComp))
            {
                for (int32 i = 0; i < Originals->Num(); i++)
                {
                    MeshComp->SetMaterial(i, (*Originals)[i]);
                }
            }
        }
    }

    OriginalRepairMaterials.Empty();
    HighlightedRepairActor = nullptr;
}

// Traces under the mouse each tick while in repair mode and tints whatever repairable actor is hovered
void ATDSPlayerController::UpdateRepairHighlight()
{
    FVector RayOrigin, RayDir;
    if (!DeprojectMousePositionToWorld(RayOrigin, RayDir))
    {
        ClearRepairHighlight();
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

    // Only highlight actors that implement IRepairable AND currently need repairing
    AActor* HitActor = nullptr;
    if (bHit && Hit.GetActor() && Hit.GetActor()->Implements<URepairable>())
    {
        if (IRepairable::Execute_NeedsRepair(Hit.GetActor()))
        {
            HitActor = Hit.GetActor();
        }
    }

    // Already highlighting this exact actor (or both null) — nothing to do
    if (HitActor == HighlightedRepairActor)
    {
        return;
    }

    // Hover moved to a new target (or off the old one entirely) — restore old, apply new
    ClearRepairHighlight();

    if (HitActor && RepairHighlightMaterial)
    {
        TArray<UStaticMeshComponent*> MeshComponents;
        HitActor->GetComponents<UStaticMeshComponent>(MeshComponents);

        for (UStaticMeshComponent* MeshComp : MeshComponents)
        {
            if (!MeshComp) continue;

            TArray<UMaterialInterface*> Originals;
            const int32 NumMats = MeshComp->GetNumMaterials();
            Originals.Reserve(NumMats);

            for (int32 i = 0; i < NumMats; i++)
            {
                Originals.Add(MeshComp->GetMaterial(i));
                MeshComp->SetMaterial(i, RepairHighlightMaterial);
            }

            OriginalRepairMaterials.Add(MeshComp, Originals);
        }

        HighlightedRepairActor = HitActor;
        UE_LOG(LogTemp, Warning, TEXT("[RepairMode] Highlighting '%s'"), *GetNameSafe(HitActor));
    }
}

// Traces the mouse ray, finds a repairable actor that needs repair, charges 1/4 sell cost, repairs to full
void ATDSPlayerController::RepairActorUnderCursor()
{
    FVector RayOrigin, RayDir;
    if (!DeprojectMousePositionToWorld(RayOrigin, RayDir))
    {
        UE_LOG(LogTemp, Warning, TEXT("[RepairMode] Could not deproject mouse position"));
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
        UE_LOG(LogTemp, Warning, TEXT("[RepairMode] No actor hit"));
        return;
    }

    AActor* HitActor = Hit.GetActor();

    if (!HitActor->Implements<URepairable>())
    {
        UE_LOG(LogTemp, Warning, TEXT("[RepairMode] Hit actor '%s' is not repairable"), *GetNameSafe(HitActor));
        return;
    }

    if (!IRepairable::Execute_NeedsRepair(HitActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("[RepairMode] '%s' is already at full health"), *GetNameSafe(HitActor));
        return;
    }

    // Cost is derived from the same sell value ISellable already exposes, so anything
    // that's sellable and repairable prices repair relative to what it's actually worth.
    int32 Cost = 0;
    if (HitActor->Implements<USellable>())
    {
        Cost = FMath::CeilToInt(ISellable::Execute_GetSellCost(HitActor) * RepairCostFraction);
    }

    if (PlayerCash < Cost)
    {
        UE_LOG(LogTemp, Warning, TEXT("[RepairMode] Not enough money to repair '%s'! Need %d, have %d"),
            *GetNameSafe(HitActor), Cost, PlayerCash);
        return;
    }

    PlayerCash -= Cost;
    UpdateCashUI();

    IRepairable::Execute_Repair(HitActor);

    UE_LOG(LogTemp, Warning, TEXT("[RepairMode] Repaired '%s' for %d"), *GetNameSafe(HitActor), Cost);

    // The actor is now at full health, so it no longer needs highlighting - refresh state
    if (HitActor == HighlightedRepairActor)
    {
        ClearRepairHighlight();
    }
}

// Restores original materials on the currently highlighted sell actor (if any) and clears state
void ATDSPlayerController::ClearSellHighlight()
{
    if (HighlightedSellActor && IsValid(HighlightedSellActor))
    {
        TArray<UStaticMeshComponent*> MeshComponents;
        HighlightedSellActor->GetComponents<UStaticMeshComponent>(MeshComponents);

        for (UStaticMeshComponent* MeshComp : MeshComponents)
        {
            if (!MeshComp) continue;

            if (TArray<UMaterialInterface*>* Originals = OriginalSellMaterials.Find(MeshComp))
            {
                for (int32 i = 0; i < Originals->Num(); i++)
                {
                    MeshComp->SetMaterial(i, (*Originals)[i]);
                }
            }
        }
    }

    OriginalSellMaterials.Empty();
    HighlightedSellActor = nullptr;
}

// Traces under the mouse each tick while in sell mode and tints whatever sellable actor is hovered
void ATDSPlayerController::UpdateSellHighlight()
{
    FVector RayOrigin, RayDir;
    if (!DeprojectMousePositionToWorld(RayOrigin, RayDir))
    {
        ClearSellHighlight();
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

    AActor* HitActor = (bHit && Hit.GetActor() && Hit.GetActor()->Implements<USellable>())
        ? Hit.GetActor()
        : nullptr;

    // Already highlighting this exact actor (or both null) — nothing to do
    if (HitActor == HighlightedSellActor)
    {
        return;
    }

    // Hover moved to a new target (or off the old one entirely) — restore old, apply new
    ClearSellHighlight();

    if (HitActor && SellHighlightMaterial)
    {
        TArray<UStaticMeshComponent*> MeshComponents;
        HitActor->GetComponents<UStaticMeshComponent>(MeshComponents);

        for (UStaticMeshComponent* MeshComp : MeshComponents)
        {
            if (!MeshComp) continue;

            TArray<UMaterialInterface*> Originals;
            const int32 NumMats = MeshComp->GetNumMaterials();
            Originals.Reserve(NumMats);

            for (int32 i = 0; i < NumMats; i++)
            {
                Originals.Add(MeshComp->GetMaterial(i));
                MeshComp->SetMaterial(i, SellHighlightMaterial);
            }

            OriginalSellMaterials.Add(MeshComp, Originals);
        }

        HighlightedSellActor = HitActor;
        UE_LOG(LogTemp, Warning, TEXT("[SellMode] Highlighting '%s'"), *GetNameSafe(HitActor));
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

    // If we're about to destroy the actor we're currently highlighting, drop the reference
    // first so ClearSellHighlight/UpdateSellHighlight never touch a dangling actor/component.
    if (HitActor == HighlightedSellActor)
    {
        HighlightedSellActor = nullptr;
        OriginalSellMaterials.Empty();
    }

    AddPlayerCash(Refund);
    HitActor->Destroy();
}