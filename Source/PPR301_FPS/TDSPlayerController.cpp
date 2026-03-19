#include "TDSPlayerController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"

ATDSPlayerController::ATDSPlayerController()
{
	bShowMouseCursor = false;
	bIsBuildMode = false;

	CameraSpeed = 2000.f;
	EdgeScrollThreshold = 20.f;
	GridSize = 200.f;

	PlayerMoney = 1000;
	TurretCost = 200;
}

void ATDSPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// Spawn build camera
	FVector Location = FVector(0, 0, 1500);
	FRotator Rotation = FRotator(-90, 0, 0);

	BuildCamera = GetWorld()->SpawnActor<ACameraActor>(Location, Rotation);

	// Add Enhanced Input Mapping
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			if (InputMapping)
			{
				Subsystem->AddMappingContext(InputMapping, 0);
			}
		}
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

void ATDSPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent);

	if (EIC)
	{
		if (ToggleBuildAction)
			EIC->BindAction(ToggleBuildAction, ETriggerEvent::Started, this, &ATDSPlayerController::ToggleBuildMode);

		if (PlaceTurretAction)
			EIC->BindAction(PlaceTurretAction, ETriggerEvent::Started, this, &ATDSPlayerController::PlaceTurret);

		if (ZoomAction)
			EIC->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ATDSPlayerController::ZoomCamera);
	}
}

void ATDSPlayerController::ToggleBuildMode()
{
	bIsBuildMode = !bIsBuildMode;

	UE_LOG(LogTemp, Warning, TEXT("TOGGLE BUILD MODE"));

	if (bIsBuildMode)
	{
		SetViewTargetWithBlend(BuildCamera, 0.3f);
		bShowMouseCursor = true;
		SetInputMode(FInputModeGameAndUI());
	}
	else
	{
		SetViewTargetWithBlend(GetPawn(), 0.3f);
		bShowMouseCursor = false;
		SetInputMode(FInputModeGameOnly());
	}
}

// ================= CAMERA =================

void ATDSPlayerController::MoveCamera(float DeltaTime)
{
	FVector MoveDir = FVector::ZeroVector;

	if (IsInputKeyDown(EKeys::W)) MoveDir.X += 1;
	if (IsInputKeyDown(EKeys::S)) MoveDir.X -= 1;
	if (IsInputKeyDown(EKeys::D)) MoveDir.Y += 1;
	if (IsInputKeyDown(EKeys::A)) MoveDir.Y -= 1;

	float MouseX, MouseY;
	GetMousePosition(MouseX, MouseY);

	int32 SizeX, SizeY;
	GetViewportSize(SizeX, SizeY);

	if (MouseX <= EdgeScrollThreshold) MoveDir.Y -= 1;
	if (MouseX >= SizeX - EdgeScrollThreshold) MoveDir.Y += 1;
	if (MouseY <= EdgeScrollThreshold) MoveDir.X += 1;
	if (MouseY >= SizeY - EdgeScrollThreshold) MoveDir.X -= 1;

	FVector NewLocation = BuildCamera->GetActorLocation() + (MoveDir * CameraSpeed * DeltaTime);
	BuildCamera->SetActorLocation(NewLocation);
}

void ATDSPlayerController::ZoomCamera(const FInputActionValue& Value)
{
	if (!BuildCamera) return;

	float AxisValue = Value.Get<float>();

	FVector Loc = BuildCamera->GetActorLocation();
	Loc.Z -= AxisValue * 300.f;
	Loc.Z = FMath::Clamp(Loc.Z, 600.f, 3000.f);

	BuildCamera->SetActorLocation(Loc);
}

// ================= GRID =================

FVector ATDSPlayerController::GetMouseWorldPosition()
{
	FVector WorldPos, WorldDir;

	if (DeprojectMousePositionToWorld(WorldPos, WorldDir))
	{
		FHitResult Hit;
		FVector End = WorldPos + (WorldDir * 10000);

		GetWorld()->LineTraceSingleByChannel(Hit, WorldPos, End, ECC_Visibility);

		if (Hit.bBlockingHit)
		{
			return Hit.Location;
		}
	}

	return FVector::ZeroVector;
}

FVector ATDSPlayerController::SnapToGrid(FVector Location)
{
	return FVector(
		FMath::GridSnap(Location.X, GridSize),
		FMath::GridSnap(Location.Y, GridSize),
		Location.Z
	);
}

// ================= BUILD =================

void ATDSPlayerController::UpdatePreview()
{
	if (!TurretClass) return;

	FVector Pos = SnapToGrid(GetMouseWorldPosition());

	if (!PreviewActor)
	{
		PreviewActor = GetWorld()->SpawnActor<AActor>(TurretClass, Pos, FRotator::ZeroRotator);
	}

	if (PreviewActor)
	{
		PreviewActor->SetActorLocation(Pos);
	}
}

void ATDSPlayerController::PlaceTurret()
{
	if (!TurretClass) return;
	if (PlayerMoney < TurretCost) return;

	FVector Pos = SnapToGrid(GetMouseWorldPosition());

	GetWorld()->SpawnActor<AActor>(TurretClass, Pos, FRotator::ZeroRotator);

	PlayerMoney -= TurretCost;

	UE_LOG(LogTemp, Warning, TEXT("Placed Turret. Money Left: %d"), PlayerMoney);
}