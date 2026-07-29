#include "TurretFire.h"

#include "TurretBase.h"
#include "TurretTargeting.h"

UTurretFire::UTurretFire()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTurretFire::BeginPlay()
{
	Super::BeginPlay();
	
	if (const ATurretBase* TurretBase = Cast<ATurretBase>(GetOwner()))
	{
		MuzzlePoint = TurretBase->MuzzlePoint;
		TurretTargeting = TurretBase->FindComponentByClass<UTurretTargeting>();
	}
}

void UTurretFire::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	Fire(DeltaTime);
}

/**
 * Spawns a projectile at the muzzle point and directs it towards the target.
 * @param DeltaTime The time between ticks each frame.
 */
void UTurretFire::Fire(const float DeltaTime)
{
	CurrentCooldown -= DeltaTime;
	
	if (CurrentCooldown <= 0.f && ProjectileClass && MuzzlePoint && TurretTargeting && TurretTargeting->CurrentTarget)
	{
		const FVector SpawnLocation = MuzzlePoint->GetComponentLocation();
		FRotator LookAtRotation = (TurretTargeting->CurrentTargetCenter - SpawnLocation).Rotation();

		LookAtRotation.Pitch += ProjectilePitchOffset;

		GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, LookAtRotation);
		
		CurrentCooldown = FireRate;
	}
}
