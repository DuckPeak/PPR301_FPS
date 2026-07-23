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
	UPROPERTY()
	UStaticMeshComponent* GunMesh = nullptr;
	
	UPROPERTY()
	USceneComponent* MuzzlePoint = nullptr;
	
	UPROPERTY()
	UTurretTargeting* TurretTargeting = nullptr;
	
	UPROPERTY(EditAnywhere, Category="Rotation")
	FRotator GunRotationOffset = FRotator(0, 0, 0);
	
	UPROPERTY(EditAnywhere, Category="Rotation")
	float RotationSpeed = 90.f; // Degrees per second.
	
	void RotateToTarget(float DeltaTime) const;
	void DebugAim() const;
};