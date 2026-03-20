#include "TDSPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

ATDSPlayerController::ATDSPlayerController()
{
	bShowMouseCursor = false;
	bIsBuildMode = false;
}

void ATDSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BuildCamera = GetWorld()->SpawnActor<ACameraActor>(
		FVector(0,0,1500),
		FRotator(-90,0,0)
	);

	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (auto* Subsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMapping)
				Subsystem->AddMappingContext(InputMapping, 0);
		}
	}

	SelectedBuildClass = TurretClass; // default
}

void ATDSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (auto* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ToggleBuildAction)
			EIC->BindAction(ToggleBuildAction, ETriggerEvent::Triggered, this, &ATDSPlayerController::ToggleBuildMode);

		if (PlaceTurretAction)
			EIC->BindAction(PlaceTurretAction, ETriggerEvent::Triggered, this, &ATDSPlayerController::PlaceTurret);

		if (ZoomAction)
			EIC->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ATDSPlayerController::ZoomCamera);
	}
}

void ATDSPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsBuildMode && BuildCamera)
	{
		MoveCamera(DeltaTime);
		UpdatePreview();
	}
}

// ===== BUILD MODE =====

// ===== BUILD MODE =====

void ATDSPlayerController::ToggleBuildMode()
{
	bIsBuildMode = !bIsBuildMode;

	UE_LOG(LogTemp, Warning, TEXT("TOGGLE BUILD MODE"));

	if (bIsBuildMode)
	{
		// Switch camera and enable mouse
		SetViewTargetWithBlend(BuildCamera, 0.3f);
		bShowMouseCursor = true;
		SetInputMode(FInputModeGameAndUI());

		// Spawn & show UI
		if (BuildMenuClass && !BuildMenuInstance)
		{
			// Create the widget from the Blueprint class
			BuildMenuInstance = CreateWidget<UUserWidget>(this, BuildMenuClass);

			if (BuildMenuInstance)
			{
				BuildMenuInstance->AddToViewport(); // Show the widget
			}
		}
	}
	else
	{
		// Switch back to player camera
		SetViewTargetWithBlend(GetPawn(), 0.3f);
		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());

		// Remove UI
		if (BuildMenuInstance)
		{
			BuildMenuInstance->RemoveFromParent(); // Correct API
			BuildMenuInstance = nullptr;
		}
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
		GetWorld()->LineTraceSingleByChannel(Hit, Pos, Pos + Dir*10000, ECC_Visibility);

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

// ===== BUILD =====

bool ATDSPlayerController::CheckValidPlacement(FVector Pos)
{
	FCollisionShape Box = FCollisionShape::MakeBox(FVector(100));

	return !GetWorld()->OverlapAnyTestByChannel(
		Pos,
		FQuat::Identity,
		ECC_WorldStatic,
		Box
	);
}

void ATDSPlayerController::UpdatePreview()
{
	if (!SelectedBuildClass) return;

	FVector Pos = SnapToGrid(GetMouseWorldPosition());
	bool bValid = CheckValidPlacement(Pos);

	if (!PreviewActor)
	{
		PreviewActor = GetWorld()->SpawnActor<AActor>(SelectedBuildClass, Pos, FRotator(0, CurrentRotation, 0));
		PreviewActor->SetActorEnableCollision(false);
	}

	if (PreviewActor)
	{
		PreviewActor->SetActorLocation(Pos);
		PreviewActor->SetActorRotation(FRotator(0, CurrentRotation, 0));

		// TODO: swap material (green/red)
	}
}

void ATDSPlayerController::PlaceTurret()
{
	if (!SelectedBuildClass) return;

	FVector Pos = SnapToGrid(GetMouseWorldPosition());

	if (!CheckValidPlacement(Pos)) return;

	GetWorld()->SpawnActor<AActor>(SelectedBuildClass, Pos, FRotator(0, CurrentRotation, 0));
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