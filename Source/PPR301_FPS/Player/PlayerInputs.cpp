#include "PlayerInputs.h"

#include "PlayerBase.h"
#include "PlayerBuildMode.h"

UPlayerInputs::UPlayerInputs()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UPlayerInputs::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerOwner = Cast<APlayerBase>(GetOwner());
	PlayerBuildMode = PlayerOwner->FindComponentByClass<UPlayerBuildMode>();

	if (PlayerOwner && PlayerOwner->InputComponent)
	{
		// Toggle build mode.
		PlayerOwner->InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &UPlayerInputs::ToggleBuildMode);

		// Rotate preview.
		PlayerOwner->InputComponent->BindKey(EKeys::E, IE_Pressed, this, &UPlayerInputs::RotatePreviewLeft);
		PlayerOwner->InputComponent->BindKey(EKeys::E, IE_Pressed, this, &UPlayerInputs::RotatePreviewRight);

		// Left mouse button places the selected build object.
		PlayerOwner->InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &UPlayerInputs::PlacePreviewedObject);
	}
}

void UPlayerInputs::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FVector UPlayerInputs::GetMouseWorldPosition() const
{
	FVector Position;
	FVector Direction;
	
	if (PlayerOwner && PlayerOwner->DeprojectMousePositionToWorld(Position, Direction))
	{
		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit, Position, Position + Direction * 10000.f, ECC_Visibility);
		
		if (Hit.bBlockingHit)
		{
			return Hit.Location;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[BuildMode] Mouse did not hit world"));
	
	return FVector::ZeroVector;
}

void UPlayerInputs::ToggleBuildMode()
{
	if (PlayerBuildMode)
	{
		PlayerBuildMode->ToggleBuildMode();
	}
}

void UPlayerInputs::RotatePreviewLeft()
{
	if (PlayerBuildMode)
	{
		PlayerBuildMode->RotatePreviewLeft();
	}
}

void UPlayerInputs::RotatePreviewRight()
{
	if (PlayerBuildMode)
	{
		PlayerBuildMode->RotatePreviewRight();
	}
}

void UPlayerInputs::PlacePreviewedObject()
{
	if (PlayerBuildMode)
	{
		PlayerBuildMode->PlacePreviewedDefence();
	}
}