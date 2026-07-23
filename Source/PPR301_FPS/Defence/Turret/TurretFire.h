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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float ProjectilePitchOffset = 0.f;
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	// The projectile to spawn.
	UPROPERTY(EditAnywhere, Category="Projectile")
	TSubclassOf<AActor> ProjectileClass;
	
	UPROPERTY()
	USceneComponent* MuzzlePoint = nullptr;
	
	UPROPERTY()
	UTurretTargeting* TurretTargeting = nullptr;

	UPROPERTY(EditAnywhere, Category="Firing")
	float FireRate = 1.f;

	float FireCooldown = 0.f;
	
	void Fire(float DeltaTime);
};