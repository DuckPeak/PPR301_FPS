#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurretAim.generated.h"

class UTurretTargeting;

UCLASS( ClassGroup=(Defence), meta=(BlueprintSpawnableComponent) )
class PPR301_FPS_API UTurretAim : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTurretAim();
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	/**
	 * @brief The reference to the gun mesh component.
	 */
	UPROPERTY()
	UStaticMeshComponent* GunMesh = nullptr;

	/**
	 * @brief The reference to the muzzle point component.
	 */
	UPROPERTY()
	USceneComponent* MuzzlePoint = nullptr;

	/**
	 * @brief The reference to the @ref UTurretTargeting component.
	 */
	UPROPERTY()
	UTurretTargeting* TurretTargeting = nullptr;

	/**
	 * @brief The offset to the gun rotation.
	 */
	UPROPERTY(EditAnywhere, Category="Rotation")
	FRotator GunRotationOffset = FRotator(0, 0, 0);
	
	/**
	 * @brief The rotation speed of the turret.
	 * @remark The rotation is in degrees per second.
	 */
	UPROPERTY(EditAnywhere, Category="Rotation")
	float RotationSpeed = 90.f;

	/**
	 * @brief Rotate the turret towards the target.
	 * @param DeltaTime The time between ticks each frame.
	 */
	void RotateToTarget(float DeltaTime) const;

	/**
	 * @brief Debug the turret aim direction.
	 */
	void DebugAim() const;
};