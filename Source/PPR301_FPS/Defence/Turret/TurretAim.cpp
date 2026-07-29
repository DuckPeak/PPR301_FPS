#include "TurretAim.h"
#include "TurretBase.h"
#include "TurretTargeting.h"

UTurretAim::UTurretAim()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTurretAim::BeginPlay()
{
	Super::BeginPlay();
	
	if (const ATurretBase* TurretBase = Cast<ATurretBase>(GetOwner()))
	{
		GunMesh = TurretBase->GunMesh;
		MuzzlePoint = TurretBase->MuzzlePoint;
		TurretTargeting = TurretBase->FindComponentByClass<UTurretTargeting>();
	}
}

void UTurretAim::TickComponent(const float DeltaTime, const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

#if !UE_BUILD_SHIPPING
	DebugAim();
#endif

	RotateToTarget(DeltaTime);
}

/**
 * Sets the rotation of the gun mesh to the direction of the target.
 * @param DeltaTime The time between ticks each frame.
 */
void UTurretAim::RotateToTarget(const float DeltaTime) const
{
	if (GunMesh && TurretTargeting && TurretTargeting->CurrentTarget)
	{
		FVector Direction = TurretTargeting->CurrentTargetCenter - GunMesh->GetComponentLocation();
		Direction.Z = 0;
	
		if (!Direction.IsNearlyZero())
		{
			GunMesh->SetWorldRotation(FMath::RInterpConstantTo(GunMesh->GetComponentRotation(), Direction.Rotation() + GunRotationOffset, DeltaTime, RotationSpeed));
		}
	}
}

/**
 * @brief Draws a line from the muzzle point directly forward.
 */
void UTurretAim::DebugAim() const
{
	if (MuzzlePoint)
	{
		DrawDebugLine(GetWorld(), MuzzlePoint->GetComponentLocation(), MuzzlePoint->GetComponentLocation() + MuzzlePoint->GetForwardVector() * 1000, FColor::Red, false, 0.f, 0, 0.1f);
	}
}