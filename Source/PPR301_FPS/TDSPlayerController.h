#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraActor.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"

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

	// UI CALL
	UFUNCTION(BlueprintCallable)
	void SetSelectedBuild(TSubclassOf<AActor> NewClass);
	
	// Build menu widget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	TSubclassOf<UUserWidget> BuildMenuClass;

private:

	// ===== CAMERA =====
	UPROPERTY()
	ACameraActor* BuildCamera;

	bool bIsBuildMode;

	UPROPERTY(EditAnywhere, Category="Camera")
	float CameraSpeed = 2000.f;

	UPROPERTY(EditAnywhere, Category="Camera")
	float EdgeScrollThreshold = 20.f;

	void ToggleBuildMode();
	void MoveCamera(float DeltaTime);
	void ZoomCamera(const FInputActionValue& Value);

	// ===== INPUT =====
	UPROPERTY(EditAnywhere, Category="Input")
	UInputMappingContext* InputMapping;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ToggleBuildAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* PlaceTurretAction;

	UPROPERTY(EditAnywhere, Category="Input")
	UInputAction* ZoomAction;

	// ===== GRID =====
	UPROPERTY(EditAnywhere, Category="Grid")
	float GridSize = 200.f;

	FVector GetMouseWorldPosition();
	FVector SnapToGrid(FVector Location);

	// ===== BUILD SYSTEM =====
	UPROPERTY(EditAnywhere, Category="Build")
	TSubclassOf<AActor> TurretClass;

	UPROPERTY(EditAnywhere, Category="Build")
	TSubclassOf<AActor> WallClass;

	UPROPERTY()
	TSubclassOf<AActor> SelectedBuildClass;

	UPROPERTY()
	AActor* PreviewActor;

	UPROPERTY(EditAnywhere, Category="Build")
	int32 PlayerMoney = 1000;

	UPROPERTY(EditAnywhere, Category="Build")
	int32 TurretCost = 200;

	UPROPERTY(EditAnywhere, Category="Build")
	int32 WallCost = 50;
	


	UPROPERTY()
	UUserWidget* BuildMenuInstance;

	bool CheckValidPlacement(FVector Pos);
	void UpdatePreview();
	void PlaceTurret();
	
	UPROPERTY()
	UUserWidget* BuildMenu;

	// ===== ROTATION =====
	float CurrentRotation = 0.f;
	void RotateBuild();
};