#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerInputs.generated.h"

class APlayerBase;
class UPlayerBuildMode;

UCLASS( ClassGroup=(Player), meta=(BlueprintSpawnableComponent) )
class PPR301_FPS_API UPlayerInputs : public UActorComponent
{
	GENERATED_BODY()

public:
	UPlayerInputs();
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	FVector GetMouseWorldPosition() const;
	
	FVector GetMovementDirection() const;
	
	float GetScrollDelta() const;
	
private:
	UPROPERTY()
	APlayerBase* PlayerOwner;
	
	UPROPERTY()
	UPlayerBuildMode* PlayerBuildMode;
	
	void ToggleBuildMode();
	void RotatePreviewLeft();
	void RotatePreviewRight();
	void PlacePreviewedObject();
};