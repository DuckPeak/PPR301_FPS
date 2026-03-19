#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraActor.h"
#include "InputActionValue.h"
#include "TDSPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;

UCLASS()
class PPR301_FPS_API ATDSPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATDSPlayerController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

private:

	// ================= CAMERA =================
	UPROPERTY()
	ACameraActor* BuildCamera;

	bool bIsBuildMode;

	UPROPERTY(EditAnywhere, Category="Camera")
	float CameraSpeed;

	UPROPERTY(EditAnywhere, Category="Camera")
	float EdgeScrollThreshold;

	void ToggleBuildMode();

	// ================= INPUT =================
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* InputMapping;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ToggleBuildAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* PlaceTurretAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ZoomAction;

	// ================= CAMERA CONTROL =================
	void MoveCamera(float DeltaTime);
	void ZoomCamera(const FInputActionValue& Value);

	// ================= GRID =================
	UPROPERTY(EditAnywhere, Category="Grid")
	float GridSize;

	FVector GetMouseWorldPosition();
	FVector SnapToGrid(FVector Location);

	// ================= BUILD SYSTEM =================
	UPROPERTY(EditAnywhere, Category="Build")
	TSubclassOf<AActor> TurretClass;

	UPROPERTY()
	AActor* PreviewActor;

	UPROPERTY(EditAnywhere, Category="Build")
	int32 PlayerMoney;

	UPROPERTY(EditAnywhere, Category="Build")
	int32 TurretCost;

	void UpdatePreview();
	void PlaceTurret();
};