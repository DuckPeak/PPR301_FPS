#include "PlayerBuildMode.h"

#include "Camera/CameraActor.h"
#include "PlayerBase.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "PPR301_FPS/Defence/DefenceBase.h"

UPlayerBuildMode::UPlayerBuildMode()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerBuildMode::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerBase = Cast<APlayerBase>(GetOwner());
	
	if (PlayerBase)
	{
		PlayerBasePawn = PlayerBase->GetPawn();
		PlayerInputs = PlayerBase->FindComponentByClass<UPlayerInputs>();
	}
	
	// Spawn the build camera — position will be set properly in ToggleBuildMode
	// when we actually have a valid pawn. Use a fallback location for now.
	BuildCamera = GetWorld()->SpawnActor<ACameraActor>(FVector(0.f, 0.f, 1500.f), FRotator(-90.f, 0.f, 0.f));

	if (BuildCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BuildMode] BuildCamera spawned at %s"), *BuildCamera->GetActorLocation().ToString());
		
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("[BuildMode] Failed to spawn BuildCamera!"));
}

void UPlayerBuildMode::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UPlayerBuildMode::ToggleBuildMode()
{
	bIsBuildMode = !bIsBuildMode;
	UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Toggled build mode: %s"), bIsBuildMode ? TEXT("ON") : TEXT("OFF"));

	// TODO: Do.
	//OnBuildModeToggled.Broadcast(bIsBuildMode);

	// Reposition build camera directly above the player's current XY position.
	if (BuildCamera && PlayerBasePawn)
	{
		if (bIsBuildMode)
		{
			const FVector ActorLocation = PlayerBasePawn->GetActorLocation();
			BuildCamera->SetActorLocation(FVector(ActorLocation.X, ActorLocation.Y, 1500.f));
			BuildCamera->SetActorRotation(FRotator(-90.f, 0.f, 0.f));
			UE_LOG(LogTemp, Warning, TEXT("[BuildMode] BuildCamera repositioned above player at: %s"), *BuildCamera->GetActorLocation().ToString());

			// Disable pawn input so the shoot binding on the pawn never sees mouse clicks.
			PlayerBasePawn->DisableInput(Cast<APlayerController>(PlayerBase));
			UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Input disabled"));
			
			PlayerBase->SetViewTargetWithBlend(BuildCamera, 0.3f);
			PlayerBase->bShowMouseCursor = true;
			PlayerBase->SetInputMode(FInputModeGameAndUI());

			if (BuildMenuClass && !BuildMenuInstance)
			{
				BuildMenuInstance = CreateWidget<UUserWidget>(PlayerBase, BuildMenuClass);
				
				if (BuildMenuInstance)
				{
					BuildMenuInstance->AddToViewport();
					CashTextBlock = Cast<UTextBlock>(BuildMenuInstance->GetWidgetFromName(FName("CashText")));
					UpdateCashUI();

					UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Build menu added to viewport."));
				}
			}
		}
	
		else
		{
			PlayerBase->SetViewTargetWithBlend(PlayerBasePawn, 0.3f);
			PlayerBase->bShowMouseCursor = false;
			PlayerBase->SetInputMode(FInputModeGameOnly());

			// Re-enable pawn input so the player can shoot again.
			// TODO: Do.
			//PlayerBasePawn()->EnableInput(Cast<APlayerController>(PlayerBase));
			UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Pawn input restored"));

			if (BuildMenuInstance)
			{
				BuildMenuInstance->RemoveFromParent();
				BuildMenuInstance = nullptr;
				UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Build menu removed"));
			}

			if (PreviewDefenceActor)
			{
				PreviewDefenceActor->Destroy();
				PreviewDefenceActor = nullptr;
				UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Preview actor destroyed on exit"));
			}

			// Leaving build mode entirely also means leaving sell/repair mode - restore any tinted actor.
			ClearSellHighlight();
			bIsSellMode = false;
			ClearRepairHighlight();
			bIsRepairMode = false;

			SelectedDefenceBuildClass = nullptr;
		}
	}
}

void UPlayerBuildMode::UpdatePreview()
{
    // Guard against null or GC'd UClass pointer — the source of the segfault.
    // IsValidLowLevel() checks the UObject header is intact before we dereference.
    if (PlayerInputs && SelectedDefenceBuildClass)
    {
    	if (IsValid(SelectedDefenceBuildClass))
    	{
		    // If the mouse didn't hit anything, don't try to move/spawn the preview
    		if (const FVector Position = PlayerInputs->GetMouseWorldPosition(); Position.IsZero())
    		{
    			if (!PreviewDefenceActor)
    			{
    				// TODO: Changed so that the preview always spawns at 0 rotation. This might not be the desired behaviour.
    				CurrentRotation = 0;
    				const FTransform SpawnTransform(FRotator(0.f, 0.f, 0.f), Position);

				    if (ADefenceBase* SpawnedDefence = GetWorld()->SpawnActorDeferred<ADefenceBase>(SelectedDefenceBuildClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
    				{
				    	SpawnedDefence->bIsPreview = true;
    					SpawnedDefence->SetActorEnableCollision(false);
    					SpawnedDefence->SetActorTickEnabled(false);

    					PreviewDefenceActor = UGameplayStatics::FinishSpawningActor(SpawnedDefence, SpawnTransform);
    				}
    				
    				else
    				{
    					UE_LOG(LogTemp, Error, TEXT("[BuildMode] Failed to spawn preview actor!"));
    				}
    			}

    			if (PreviewDefenceActor)
    			{
    				PreviewDefenceActor->SetActorLocation(Position);
    				PreviewDefenceActor->SetActorRotation(FRotator(0.f, CurrentRotation, 0.f));

    				bIsPreviewDefencePlacementValid = CheckValidPlacement(Position);
    				UMaterialInterface* PreviewMaterial = bIsPreviewDefencePlacementValid ? GhostMaterial : SellHighlightMaterial;

    				TArray<UStaticMeshComponent*> MeshComponents;
    				PreviewDefenceActor->GetComponents<UStaticMeshComponent>(MeshComponents);
    				
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
    		
    		return;
    	}
    				
    	SelectedDefenceBuildClass = nullptr;
    }
}

/**
 * \brief Rotate the previewed object to the left.
 */
void UPlayerBuildMode::RotatePreviewLeft()
{
	CurrentRotation -= RotationAmount;
	
	if (PreviewDefenceActor)
	{
		PreviewDefenceActor->SetActorRotation(FRotator(0.f, CurrentRotation, 0.f));
	}
	
	// TODO: Do.
	//OnPreviewRotatedLeft.Broadcast();
}

/**
 * \brief Rotate the previewed object to the right.
 */
void UPlayerBuildMode::RotatePreviewRight()
{
	CurrentRotation += RotationAmount;
	
	if (PreviewDefenceActor)
	{
		PreviewDefenceActor->SetActorRotation(FRotator(0.f, CurrentRotation, 0.f));
	}
	
	// TODO: Do.
	//OnPreviewRotatedRight.Broadcast();
}

bool UPlayerBuildMode::CheckValidPlacement(FVector Position) const
{
	// TODO: What is this position check?
	if (GetWorld() && Position.Z <= -20.f)
	{
		// Derive the overlap box from the actual PreviewDefenceActor's current bounding box.
		FVector BoxExtent = FVector(GridSize * 0.5f - 5.f); // Fallback if no preview yet.

		if (PreviewDefenceActor && IsValid(PreviewDefenceActor))
		{
			if (FBox Bounds = PreviewDefenceActor->GetComponentsBoundingBox(true); Bounds.IsValid)
			{
				// Shrink slightly so edge-adjacent pieces don't falsely flag as overlapping
				//BoxExtent = Bounds.GetExtent() * 0.9f;
				BoxExtent = Bounds.GetExtent() * 1.05f;
			}
		}

		// Bounds from GetComponentsBoundingBox is already an axis-aligned WORLD-space box,
		const FCollisionShape Box = FCollisionShape::MakeBox(BoxExtent);
		const FQuat Rotation = FQuat::Identity;

		FCollisionQueryParams Params;
		Params.AddIgnoredActor(GetOwner());
		
		if (PreviewDefenceActor)
		{
			Params.AddIgnoredActor(PreviewDefenceActor);
		}

		TArray<FOverlapResult> Overlaps;
		const bool bAnyOverlap = GetWorld()->OverlapMultiByChannel(Overlaps, Position, Rotation, ECC_Visibility, Box, Params
		);

		if (bAnyOverlap)
		{
			for (const FOverlapResult& Result : Overlaps)
			{
				if (AActor* OverlappedActor = Result.GetActor(); OverlappedActor && OverlappedActor != PreviewDefenceActor && OverlappedActor->Implements<USellable>())
				{
					return false;
				}
			}
		}

		return true;
	}
	
	return false;
}

// PlacePreviewedObject sell vs place
void UPlayerBuildMode::PlacePreviewedDefence()
{
    if (bIsBuildMode)
    {
    	// Sell mode: click sells the actor under the cursor
    	if (bIsSellMode)
    	{
    		SellActorUnderCursor();
    		
    		return;
    	}

    	// Repair mode: click repairs the actor under the cursor
    	if (bIsRepairMode)
    	{
    		// TODO: Do.
    		//RepairActorUnderCursor();
    		
    		return;
    	}

    	// Normal placement
    	if (!PreviewDefenceActor || !IsValid(PreviewDefenceActor))
    	{
    		UE_LOG(LogTemp, Warning, TEXT("[BuildMode] No valid preview actor to place!"));
    		PreviewDefenceActor = nullptr;
    		
    		return;
    	}

    	if (!SelectedDefenceBuildClass || !IsValid(SelectedDefenceBuildClass))
    	{
    		UE_LOG(LogTemp, Warning, TEXT("[BuildMode] No valid selected build class!"));
    		SelectedDefenceBuildClass = nullptr;
    		
    		return;
    	}
    	
    	if (!bIsPreviewDefencePlacementValid)
    	{
    		UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Cannot place here - overlapping another structure!"));
    		
    		return;
    	}

    	UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Placing previewed object"));
    	
    	PlaceDefence();
		// TODO: Do.
    	//OnPreviewPlaced.Broadcast();
    }
}

void UPlayerBuildMode::PlaceDefence()
{
	if (SelectedDefenceBuildClass && IsValid(SelectedDefenceBuildClass))
	{
		if (FVector Position = PlayerInputs->GetMouseWorldPosition(); CheckValidPlacement(Position))
		{
			Position.Z += 5.f;

			UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Attempting to place at: %s"), *Position.ToString());
			
			if (SelectedDefenceBuildClass.GetDefaultObject()->Implements<USellable>())
			{
				const int32 Cost = ISellable::Execute_GetSellCost(SelectedDefenceBuildClass);
				
				// Check if player can afford
				if (PlayerCash >= Cost)
				{
					// Deduct money and update UI.
					PlayerCash -= Cost;
					
					// TODO: Replace with an event which the UI component subscribes to.
					UpdateCashUI();
					UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Spent %d, Remaining: %d"), Cost, PlayerCash);

					// Spawn the real actor

					if (ADefenceBase* Placed = GetWorld()->SpawnActor<ADefenceBase>(SelectedDefenceBuildClass, Position, FRotator(0.f, CurrentRotation, 0.f)))
					{
						UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Successfully placed actor at: %s"), *Placed->GetActorLocation().ToString());

						// TODO: Call setup in defence, which child classes will implement.
						Placed->SetUpOnPlaced();
						//if (PlaceTurretSound)
						//{
						//	UGameplayStatics::PlaySound2D(this, PlaceTurretSound, 1.0f, 1.0f);
						//	UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Played turret placement sound"));
						//}

						if (PreviewDefenceActor)
						{
							PreviewDefenceActor->Destroy();
							PreviewDefenceActor = nullptr;
							UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Preview actor destroyed after placement"));
						}
					}
					
					else
					{
						UE_LOG(LogTemp, Error, TEXT("[BuildMode] Failed to spawn actor at %s"), *Position.ToString());
					}
				}
				
				UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Not enough money! Need %d, have %d"), Cost, PlayerCash);
					
				return;
			}
		}
	    	
		UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Placement blocked - position overlaps another structure"));
	    	
		return;
	}
    	
	UE_LOG(LogTemp, Error, TEXT("[BuildMode] No valid SelectedDefenceBuildClass!"));
	SelectedDefenceBuildClass = nullptr;
}

// Traces the mouse ray, finds a sellable actor, refunds and destroys it
void UPlayerBuildMode::SellActorUnderCursor()
{
	FVector RayOrigin;
	FVector RayDirection;
	
	//if (!DeprojectMousePositionToWorld(RayOrigin, RayDirection))
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("[SellMode] Could not deproject mouse position"));
	//	
	//	return;
	//}

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(PlayerBasePawn);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, RayOrigin, RayOrigin + RayDirection * 10000.f, ECC_Visibility, Params
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

	// TODO: DO.
	//AddPlayerCash(Refund);
	HitActor->Destroy();
}

// Restores original materials on the currently highlighted sell actor (if any) and clears state.
void UPlayerBuildMode::ClearSellHighlight()
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

// Restores original materials on the currently highlighted repair actor (if any) and clears state.
void UPlayerBuildMode::ClearRepairHighlight()
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

void UPlayerBuildMode::UpdateCashUI() const
{
	if (CashTextBlock)
	{
		const FText NewText = FText::Format(FText::FromString(TEXT(":{0}")), FText::AsNumber(PlayerCash));
		CashTextBlock->SetText(NewText);
		UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Cash UI updated directly to %d"), PlayerCash);
	}
	
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[BuildMode] CashTextBlock is null!"));
	}
}