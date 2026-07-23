#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TurretTargeting.generated.h"

UCLASS( ClassGroup=(Defence), meta=(BlueprintSpawnableComponent) )
class PPR301_FPS_API UTurretTargeting : public UActorComponent
{
	GENERATED_BODY()

public:
	UTurretTargeting();
	
	UPROPERTY(VisibleInstanceOnly)
	AActor* CurrentTarget;
	
	UPROPERTY(VisibleInstanceOnly)
	FVector CurrentTargetCenter;
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void FindNearestEnemy();
	static FVector GetTargetCenter(const AActor* Target);
};