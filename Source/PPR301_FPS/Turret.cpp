#include "Turret.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

ATurret::ATurret()
{
    PrimaryActorTick.bCanEverTick = true;

    // Root
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    RootComponent = Root;

    // Base
    BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
    BaseMesh->SetupAttachment(Root);

    // Gun
    GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunMesh"));
    GunMesh->SetupAttachment(BaseMesh);

    // Muzzle
    MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
    MuzzlePoint->SetupAttachment(GunMesh);
}

void ATurret::BeginPlay()
{
    Super::BeginPlay();
    FireCooldown = 0.f; // ensure cooldown starts at 0
}

void ATurret::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (FireCooldown > 0.f)
        FireCooldown -= DeltaTime;

    FindNearestEnemy();

    if (CurrentTarget)
    {
        RotateToTarget(DeltaTime);

        // DEBUG LINE (AIM DIRECTION)
        if (MuzzlePoint)
        {
            DrawDebugLine(
                GetWorld(),
                MuzzlePoint->GetComponentLocation(),
                MuzzlePoint->GetComponentLocation() + MuzzlePoint->GetForwardVector() * 1000,
                FColor::Green,
                false,
                0.f,
                0,
                2.f
            );
        }

        if (FireCooldown <= 0.f) // && IsAimedAtTarget()
        {
            Fire();
            FireCooldown = FireRate;
        }
    }
}

// ===== TARGETING =====

void ATurret::FindNearestEnemy()
{
    CurrentTarget = nullptr;
    float ClosestDist = TNumericLimits<float>::Max();

    TArray<AActor*> Enemies;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Enemy"), Enemies);

    FVector TurretLoc = GetActorLocation();

    for (AActor* Enemy : Enemies)
    {
        if (!IsValid(Enemy)) continue;
        float Dist = FVector::Dist(TurretLoc, Enemy->GetActorLocation());
        if (Dist < ClosestDist)
        {
            ClosestDist = Dist;
            CurrentTarget = Enemy;
        }
    }
}

void ATurret::RotateToTarget(float DeltaTime)
{
    if (!GunMesh || !CurrentTarget) return;

    FVector TargetLoc = CurrentTarget->GetActorLocation() + FVector(0,0,80);
    FVector Dir = TargetLoc - GunMesh->GetComponentLocation();

    Dir.Z = 0;
    if (Dir.IsNearlyZero()) return;

    // pply offset HERE (to target)
    FRotator TargetRot = Dir.Rotation() + GunRotationOffset;

    FRotator CurrentRot = GunMesh->GetComponentRotation();

    FRotator NewRot = FMath::RInterpConstantTo(CurrentRot, TargetRot, DeltaTime, RotationSpeed);

    GunMesh->SetWorldRotation(NewRot);
}

bool ATurret::IsAimedAtTarget() const
{
    if (!GunMesh || !CurrentTarget) return false;

    FVector DirToTarget = (CurrentTarget->GetActorLocation() - GunMesh->GetComponentLocation()).GetSafeNormal();
    FVector GunForward = GunMesh->GetForwardVector();

    // clamp dot to [-1,1] to prevent NaN
    float Dot = FMath::Clamp(FVector::DotProduct(GunForward, DirToTarget), -1.f, 1.f);
    float Angle = FMath::RadiansToDegrees(acosf(Dot));

    return Angle <= FireAngleThreshold;
}

// ===== FIRE =====

void ATurret::Fire()
{
    if (!ProjectileClass || !MuzzlePoint || !CurrentTarget) return;

    FVector SpawnLocation = MuzzlePoint->GetComponentLocation();

    FVector TargetLoc = CurrentTarget->GetActorLocation() + FVector(0,0,80);

    FRotator LookAtRot = (TargetLoc - SpawnLocation).Rotation();

    GetWorld()->SpawnActor<AActor>(ProjectileClass, SpawnLocation, LookAtRot);
}