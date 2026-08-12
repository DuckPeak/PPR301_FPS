#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurretFire.generated.h"

class UTurretTargeting;

UCLASS( ClassGroup=(Defence), meta=(BlueprintSpawnableComponent) )
class PPR301_FPS_API UTurretFire : public UActorComponent
{
	GENERATED_BODY()

public:
	UTurretFire();

	/**
	 * @brief The offset to the pitch of the projectile when firing.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ProjectilePitchOffset = 0.f;
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/**
	 * @brief The reference to the projectile for the turret.
	 */
	UPROPERTY(EditAnywhere, Category="Projectile")
	TSubclassOf<AActor> ProjectileClass;

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
	 * @brief The rate at which the turret can fire.
	 */
	UPROPERTY(EditAnywhere, Category="Firing")
	float FireRate = 1.f;

	/**
	 * @brief The cooldown time for the turret after firing.
	 */
	UPROPERTY(VisibleInstanceOnly, Category="Firing")
	float CurrentCooldown = 0.f;

	/**
	 * Fires the turret if able.
	 * @param DeltaTime The time between ticks each frame.
	 */
	void Fire(float DeltaTime);
};