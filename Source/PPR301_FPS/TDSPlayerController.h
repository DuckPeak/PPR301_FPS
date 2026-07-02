#pragma once
#include "CoreMinimal.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraActor.h"
#include "Components/TextBlock.h"
#include "Components/StaticMeshComponent.h"
#include "InputActionValue.h"
#include "Blueprint/UserWidget.h"
#include "IRepairable.h"
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="BuildMode|Preview")
	UMaterialInterface* GhostMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Mode")
	int32 PlayerCash = 250;
	UFUNCTION(BlueprintCallable, Category = "Build | UI")
	void UpdateBuildMenuCash();
	void UpdateCashUI();
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	UTextBlock* CashTextBlock = nullptr;
	/** Add money to the player and update UI */
	UFUNCTION(BlueprintCallable, Category = "Build | Cash")
	void AddPlayerCash(int32 Amount);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* PlaceTurretSound = nullptr;
	
	UFUNCTION(BlueprintCallable, Category = "Build | Sell")
	void ToggleSellMode();
	UPROPERTY(BlueprintReadOnly, Category = "Build | Sell")
	bool bIsSellMode = false;

	// Material applied to whatever sellable actor is currently under the cursor in sell mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build | Sell")
	UMaterialInterface* SellHighlightMaterial;

	// ===== REPAIR MODE =====
	UFUNCTION(BlueprintCallable, Category = "Build | Repair")
	void ToggleRepairMode();
	UPROPERTY(BlueprintReadOnly, Category = "Build | Repair")
	bool bIsRepairMode = false;

	// Material applied to whatever repairable actor is currently under the cursor in repair mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build | Repair")
	UMaterialInterface* RepairHighlightMaterial;

	// Repair cost is this fraction of the actor's ISellable sell cost (0.25 = 1/4)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build | Repair")
	float RepairCostFraction = 0.25f;

private:
	// ===== CAMERA =====
	UPROPERTY()
	ACameraActor* BuildCamera;
	bool bIsBuildMode;
	UPROPERTY(EditAnywhere, Category="Camera")
	float CameraSpeed = 2000.f;
	UPROPERTY(EditAnywhere, Category="Camera")
	float ZoomSpeed = 200.f;
	UPROPERTY(EditAnywhere, Category="Camera")
	float MinCameraHeight = 400.f;
	UPROPERTY(EditAnywhere, Category="Camera")
	float MaxCameraHeight = 3000.f;
	UPROPERTY(EditAnywhere, Category="Camera")
	float EdgeScrollThreshold = 20.f;
	void ToggleBuildMode();
	void MoveCamera(float DeltaTime);
	void HandleZoom();
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
	UFUNCTION()
	void PlacePreviewedObject();
	UPROPERTY()
	UUserWidget* BuildMenuInstance;
	bool CheckValidPlacement(FVector Pos);
	void UpdatePreview();
	void PlaceTurret();
	// Build mode rotation
	void RotatePreviewLeft();
	void RotatePreviewRight();
	
	// Sell
	void SellActorUnderCursor();
	UPROPERTY()
	UUserWidget* BuildMenu;

	// Sell highlight - which actor is currently tinted, and what its original
	// materials were so we can restore them when the hover moves off / actor is sold
	UPROPERTY()
	AActor* HighlightedSellActor = nullptr;

	TMap<UStaticMeshComponent*, TArray<UMaterialInterface*>> OriginalSellMaterials;

	void UpdateSellHighlight();
	void ClearSellHighlight();

	// Repair highlight - mirrors the sell highlight state above
	UPROPERTY()
	AActor* HighlightedRepairActor = nullptr;

	TMap<UStaticMeshComponent*, TArray<UMaterialInterface*>> OriginalRepairMaterials;

	void UpdateRepairHighlight();
	void ClearRepairHighlight();
	void RepairActorUnderCursor();

	// ===== ROTATION =====
	float CurrentRotation = 0.f;
	void RotateBuild();
};