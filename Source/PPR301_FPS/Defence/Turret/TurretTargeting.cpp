#include "TurretTargeting.h"

#include "Kismet/GameplayStatics.h"

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
			if (const float Distance = FVector::Dist(TurretLocation, Enemy->GetActorLocation()); Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				CurrentTarget = Enemy;
			}
		}
	}

	CurrentTargetCenter = GetTargetCenter(CurrentTarget);
}

FVector UTurretTargeting::GetTargetCenter(const AActor* Target)
{
	if (Target)
	{
		// Try to get the skeletal mesh first, then static mesh, else return the actor location.
		if (const USkeletalMeshComponent* SkeletalMesh = Target->FindComponentByClass<USkeletalMeshComponent>())
		{
			return SkeletalMesh->Bounds.Origin;
		}

		if (const UStaticMeshComponent* StaticMesh = Target->FindComponentByClass<UStaticMeshComponent>())
		{
			return StaticMesh->Bounds.Origin;
		}

		return Target->GetActorLocation();
	}
	
	return FVector::ZeroVector;
}