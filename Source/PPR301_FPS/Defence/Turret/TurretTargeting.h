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

	// TODO: Update to be enemy base class.
	/**
	 * @brief The reference to the current target of the turret.
	 */
	UPROPERTY(VisibleInstanceOnly)
	AActor* CurrentTarget;

	/**
	 * @brief The centre of the current target.
	 */
	UPROPERTY(VisibleInstanceOnly)
	FVector CurrentTargetCenter;

	/**
	 * @brief Maximum range to search for enemies.
	 */
	UPROPERTY(EditAnywhere, Category="Turret")
	float MaxSearchRange = 1000.0f;

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	/**
	 * @brief Find the nearest enemy in range with line of sight.
	 */
	void FindNearestEnemy();

	/**
	 * @brief Check if the turret has line of sight to the target.
	 * @param Target The actor to check line of sight to.
	 * @return Returns whether there is a line of sight.
	 */
	bool HasLineOfSightTo(const AActor* Target) const;

	/**
	 * @brief Get the centre position of the target.
	 * @return Return the centre position of the target.
	 */
	FVector GetTargetCenter() const;
};