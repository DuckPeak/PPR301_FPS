#pragma once

#include "CoreMinimal.h"
#include "ISellable.h"
#include "GameFramework/Actor.h"
#include "Turret.generated.h"


class USceneComponent;
class UStaticMeshComponent;
class AActor;

UCLASS()
class PPR301_FPS_API ATurret : public AActor, public ISellable
{
	GENERATED_BODY()
    
public:    
	ATurret();

	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPreview = false;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Build")
	int32 Cost = 100;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turret|Fire")
	float ProjectilePitchOffset = 0.f;

protected:
	virtual void BeginPlay() override;

	// Components
	UPROPERTY(VisibleAnywhere)
	USceneComponent* Root;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* BaseMesh;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* GunMesh;

	UPROPERTY(VisibleAnywhere)
	USceneComponent* MuzzlePoint;
	

	// ===== TURRET SETTINGS =====
	UPROPERTY(EditAnywhere, Category="Turret")
	float RotationSpeed = 90.f; // degrees per second

	UPROPERTY(EditAnywhere, Category="Turret")
	float FireRate = 1.f;

	UPROPERTY(EditAnywhere, Category="Turret")
	float FireAngleThreshold = 20.0f;

	float FireCooldown = 0.f;
	
	UPROPERTY(EditAnywhere, Category="Turret")
	FRotator GunRotationOffset = FRotator(0, 0, 0);

	// Projectile to spawn
	UPROPERTY(EditAnywhere, Category="Turret")
	TSubclassOf<AActor> ProjectileClass;

	// Target tracking
	UPROPERTY()
	AActor* CurrentTarget;

	void RotateToTarget(float DeltaTime);
	bool IsAimedAtTarget() const;
	void Fire();
	FVector GetTargetCenter(AActor* Target) const;

	void FindNearestEnemy();
	
	virtual int32 GetSellCost_Implementation() override { return Cost; }
};