#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PPR301_FPS/ISellable.h"
#include "TurretBase.generated.h"

UCLASS()
class PPR301_FPS_API ATurretBase : public AActor, public ISellable
{
	GENERATED_BODY()
	
public:
	ATurretBase();
	
	// Mesh components.
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* GunMesh;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* MuzzlePoint;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPreview = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Build")
	int32 Cost = 100;
	
	virtual void Tick(float DeltaTime) override;

	// ISellable interface implementation.
	virtual int32 GetSellCost_Implementation() override { return Cost; }

protected:
	virtual void BeginPlay() override;
};