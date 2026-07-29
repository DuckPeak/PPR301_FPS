#include "TurretTargeting.h"

#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UTurretTargeting::UTurretTargeting()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTurretTargeting::BeginPlay()
{
	Super::BeginPlay();
}

void UTurretTargeting::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	FindNearestEnemy();
}

/**
 * @brief Find the nearest enemy in range with line of sight.
 */
void UTurretTargeting::FindNearestEnemy()
{
	CurrentTarget = nullptr;
	float ClosestDistance = TNumericLimits<float>::Max();

	TArray<AActor*> Enemies;
	
	// TODO: Consider updating to find by class for better optimisation.
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);

	const FVector TurretLocation = GetOwner()->GetActorLocation();

	for (AActor* Enemy : Enemies)
	{
		if (IsValid(Enemy))
		{
			// Check if enemy is within max search range and has line of sight
			if (const float Distance = FVector::Dist(TurretLocation, Enemy->GetActorLocation()); Distance <= MaxSearchRange && HasLineOfSightTo(Enemy))
			{
				if (Distance < ClosestDistance)
				{
					ClosestDistance = Distance;
					CurrentTarget = Enemy;
				}
			}
		}
	}

	CurrentTargetCenter = GetTargetCenter();
}

bool UTurretTargeting::HasLineOfSightTo(const AActor* Target) const
{
	if (IsValid(Target))
	{
		const FVector TurretLocation = GetOwner()->GetActorLocation();
		const FVector TargetLocation = Target->GetActorLocation();

		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(GetOwner());
		Params.AddIgnoredActor(Target);

		// Perform line trace from turret to Target.
		const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TurretLocation, TargetLocation, ECC_Visibility, Params);

		return !bHit || HitResult.GetActor() == Target;
	}
	
	return false;
}

/**
 * @brief Find the centre vector of the bounds of the target, else the actor location.
 * @return Return the centre position of the @ref CurrentTarget.
 */
FVector UTurretTargeting::GetTargetCenter() const
{
	if (CurrentTarget)
	{
		// Try to get the skeletal mesh first, then static mesh, else return the actor location.
		if (const USkeletalMeshComponent* SkeletalMesh = CurrentTarget->FindComponentByClass<USkeletalMeshComponent>())
		{
			return SkeletalMesh->Bounds.Origin;
		}

		if (const UStaticMeshComponent* StaticMesh = CurrentTarget->FindComponentByClass<UStaticMeshComponent>())
		{
			return StaticMesh->Bounds.Origin;
		}

		return CurrentTarget->GetActorLocation();
	}
	
	return FVector::ZeroVector;
}