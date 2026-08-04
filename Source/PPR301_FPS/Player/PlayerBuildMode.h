#pragma once

#include "CoreMinimal.h"
#include "PlayerInputs.h"
#include "Components/ActorComponent.h"
#include "PlayerBuildMode.generated.h"


class ADefenceBase;
class UTextBlock;

// Suffixed with 'N' for New as this is the new version of the build mode component.
// This ought to be updated when the old version is removed.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuildModeToggledN, bool, bBuildModeActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPreviewRotatedLeftN);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPreviewRotatedRightN);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPreviewPlacedN);

UCLASS( ClassGroup=(Player), meta=(BlueprintSpawnableComponent) )
class PPR301_FPS_API UPlayerBuildMode : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerBuildMode();

	/**
	 * @brief Event delegate for when the build mode is toggled.
	 */
	UPROPERTY(BlueprintAssignable, Category="Build | Event")
	FOnBuildModeToggledN OnBuildModeToggledN;

	/**
	 * @brief Event delegate for when the @ref PreviewDefenceActor is rotated to the left.
	 */
	UPROPERTY(BlueprintAssignable, Category="Build | Event")
	FOnPreviewRotatedLeftN OnPreviewRotatedLeftN;

	/**
	 * @brief Event delegate for when the @ref PreviewDefenceActor is rotated to the right.
	 */
	UPROPERTY(BlueprintAssignable, Category="Build | Event")
	FOnPreviewRotatedRightN OnPreviewRotatedRightN;

	/**
	 * @brief Event delegate for when the @ref PreviewDefenceActor is placed.
	 */
	UPROPERTY(BlueprintAssignable, Category="Build | Event")
	FOnPreviewPlacedN OnPreviewPlacedN;

	/**
	 * @brief The flag to determine if the build mode is active.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State")
	bool bIsBuildMode;

	/**
	 * @brief The flag to determine if the sell mode is active.
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	bool bIsSellMode = false;

	/**
	 * @brief The flag to determine if the repair mode is active.
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	bool bIsRepairMode = false;
	
	/**
	 * @brief The flag to determine if the preview defence placement is valid.
	 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "State")
	bool bIsPreviewDefencePlacementValid = false;

	/**
	 * @brief The class of the build menu.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
	TSubclassOf<UUserWidget> BuildMenuClass;
	
	// TODO: Move to a separate currency component.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	int32 PlayerCash = 250;
	
	// Repair cost is this fraction of the actor's ISellable sell cost (0.25 = 1/4)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Currency")
	float RepairCostFraction = 0.25f;

	/**
	 * @brief The material to use for the defence in preview mode.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	UMaterialInterface* GhostMaterial;

	/**
	 * @brief The material to use for the sell highlight.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Preview")
	UMaterialInterface* SellHighlightMaterial;
	
	// Material applied to whatever repairable actor is currently under the cursor in repair mode
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build | Repair")
	UMaterialInterface* RepairHighlightMaterial;
	
	/**
	 * @brief The text block to display the player's cash.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "UI")
	UTextBlock* CashTextBlock = nullptr;
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * @brief Toggles the build mode.
	 */
	void ToggleBuildMode();
	
	/**
	 * @brief Rotates the preview defence actor to the left.
	 */
	void RotatePreviewLeft();
	
	/**
	 * @brief Rotates the preview defence actor to the right.
	 */
	void RotatePreviewRight();

	/**
	 * @brief Places the previewed defence at the specified location.
	 */
	void PlacePreviewedDefence();
	
	/** Add money to the player and update UI */
	UFUNCTION(BlueprintCallable, Category = "Currency")
	void AddPlayerCash(int32 Amount);
	
	/**
	 * @brief Updates the cash UI.
	 */
	void UpdateCashUI() const;
	
private:
	/**
	 * @brief The reference to the @ref APlayerBase actor.
	 */
	UPROPERTY()
	APlayerBase* PlayerBase = nullptr;
	
	/**
	 * @brief The reference to the @ref APawn actor.
	 */
	UPROPERTY()
	APawn* PlayerBasePawn = nullptr;
	
	/**
	 * @brief The reference to the @ref UPlayerInputs component.
	 */
	UPROPERTY()
	UPlayerInputs* PlayerInputs = nullptr;
	
	/**
	 * @brief The reference to the @ref ACameraActor component.
	 */
	UPROPERTY()
	ACameraActor* BuildCamera = nullptr;

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
	
	/**
	 * @brief The reference to the preview actor for defence.
	 */
	UPROPERTY()
	AActor* PreviewDefenceActor = nullptr;
	
	/**
	 * @brief The currently selected class for a defence to build.
	 */
	UPROPERTY(VisibleInstanceOnly, Category="Build")
	TSubclassOf<ADefenceBase> SelectedDefenceBuildClass;
	
	/**
	 * @brief The list of available defence classes to build.
	 */
	UPROPERTY(EditAnywhere, Category="Build")
	TArray<TSubclassOf<ADefenceBase>> AvailableDefenceClasses;
	
	
	// Sell highlight - which actor is currently tinted, and what its original
	// materials were so we can restore them when the hover moves off / actor is sold.
	UPROPERTY()
	AActor* HighlightedSellActor = nullptr;
	
	// Repair highlight - mirrors the sell highlight state above
	UPROPERTY()
	AActor* HighlightedRepairActor = nullptr;

	// TODO: Are the materials not the same for both?
	TMap<UStaticMeshComponent*, TArray<UMaterialInterface*>> OriginalSellMaterials;
	
	TMap<UStaticMeshComponent*, TArray<UMaterialInterface*>> OriginalRepairMaterials;

	/**
	 * @brief The instance of the build menu.
	 */
	UPROPERTY()
	UUserWidget* BuildMenuInstance;

	/**
	 * @brief The size of the grid for placement of defence.
	 */
	UPROPERTY(EditAnywhere, Category="Grid")
	float GridSize = 200.f;

	/**
	 * @brief The amount of rotation for each rotation step.
	 */
	UPROPERTY(EditAnywhere, Category="Rotation")
	float RotationAmount = 90.f;

	/**
	 * @brief The current rotation of the preview defence actor.
	 */
	UPROPERTY(VisibleAnywhere, Category="Rotation")
	float CurrentRotation = 0.f;

	/**
	 * @brief Moves the build camera.
	 * @param DeltaTime The time since the last frame.
	 */
	void MoveCamera(float DeltaTime) const;

	/**
	 * @brief Handles zooming in and out of the build camera.
	 */
	void HandleZoom();

	/**
	 * @brief Updates the position and rotation of the preview defence actor.
	 */
	void UpdatePreview();
	
	/**
	 * @brief Determines if the given position is a valid placement for a defence.
	 */
	bool CheckValidPlacement(FVector Position) const;
	
	// TODO: Should this be merged with PlacePreviewedDefence?
	/**
	 * @brief Places the defence at the specified location.
	 */
	void PlaceDefence();
	
	/**
	 * @brief Sells the defence under the cursor.
	 */
	void SellDefenceUnderCursor();
	
	/**
	 * @brief Repairs the defence under the cursor.
	 */
	void RepairDefenceUnderCursor();

	/**
	 * @brief Updates the sell highlight.
	 */
	void UpdateSellHighlight();
	
	/**
	 * @brief Updates the repair highlight.
	 */
	void UpdateRepairHighlight();

	/**
	 * @brief Clears the highlight for selling.
	 */
	void ClearSellHighlight();

	/**
	 * @brief Clears the highlight for repairing.
	 */
	void ClearRepairHighlight();
};