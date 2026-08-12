#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PPR301_FPS/Defence/DefenceBase.h"
#include "TurretBase.generated.h"

UCLASS()
class PPR301_FPS_API ATurretBase : public ADefenceBase
{
	GENERATED_BODY()
	
public:
	ATurretBase();
	
	/**
	 * @brief The root scene component.
	 */
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;
	
	/**
	 * @brief The base mesh component. This is the legs of the turret.
	 */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BaseMesh;

	/**
	 * @brief The gun mesh component. This is the gun top of the turret.
	 */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* GunMesh;

	// TODO: Fix the bullet colliding with the turret.
	/**
	 * @brief The muzzle point component. This is the point where the bullet exits the gun.
	 */
	UPROPERTY(VisibleAnywhere)
	USceneComponent* MuzzlePoint;
	
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
};