#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Turret.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class AActor;

UCLASS()
class PPR301_FPS_API ATurret : public AActor
{
	GENERATED_BODY()
    
public:    
	ATurret();

	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPreview = false;

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

	void FindNearestEnemy();
};