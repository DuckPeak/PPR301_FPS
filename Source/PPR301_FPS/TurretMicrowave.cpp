#include "TurretMicrowave.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ATurretMicrowave::ATurretMicrowave()
{
    PrimaryActorTick.bCanEverTick = true;

    // Visual-only sphere. NOT used for collision/overlap
    RadiusIndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RadiusIndicatorMesh"));
    RadiusIndicatorMesh->SetupAttachment(RootComponent);
    RadiusIndicatorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RadiusIndicatorMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    RadiusIndicatorMesh->SetCastShadow(false);
    RadiusIndicatorMesh->SetVisibility(true);
    // Center it on the turret base, slightly above ground so it doesn't z-fight.
    RadiusIndicatorMesh->SetRelativeLocation(FVector(0.f, 0.f, 5.f));
}

void ATurretMicrowave::BeginPlay()
{
    Super::BeginPlay();

    if (RadiusIndicatorMaterial && RadiusIndicatorMesh)
    {
        RadiusIndicatorMesh->SetMaterial(0, RadiusIndicatorMaterial);
    }

    UpdateRadiusIndicatorScale();
    TimeUntilNextScan = 0.f;
}

void ATurretMicrowave::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    RemoveAllEffects();
    Super::EndPlay(EndPlayReason);
}

void ATurretMicrowave::UpdateRadiusIndicatorScale()
{
    if (!RadiusIndicatorMesh || RadiusIndicatorMeshBaseRadius <= 0.f) return;

    float Scale = EffectRadius / RadiusIndicatorMeshBaseRadius;
    RadiusIndicatorMesh->SetWorldScale3D(FVector(Scale, Scale, Scale * 0.05f)); // flattened
}

#if WITH_EDITOR
void ATurretMicrowave::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    const FName ChangedProp = PropertyChangedEvent.GetPropertyName();
    if (ChangedProp == GET_MEMBER_NAME_CHECKED(ATurretMicrowave, EffectRadius) ||
        ChangedProp == GET_MEMBER_NAME_CHECKED(ATurretMicrowave, RadiusIndicatorMeshBaseRadius))
    {
        UpdateRadiusIndicatorScale();
    }
}
#endif

void ATurretMicrowave::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);

    if (bIsPreview) return;

    TimeUntilNextScan -= DeltaTime;
    if (TimeUntilNextScan <= 0.f)
    {
        ScanForEnemiesInRange();
        TimeUntilNextScan = ScanInterval;
    }

    TickAffectedEnemies(DeltaTime);

#if !UE_BUILD_SHIPPING
    DrawDebugSphere(GetWorld(), GetActorLocation(), EffectRadius, 32, FColor::Cyan, false, 0.f, 0, 1.f);
#endif
}

// ===== SCANNING =====

void ATurretMicrowave::ScanForEnemiesInRange()
{
    TArray<AActor*> AllEnemies;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), EnemyTag, AllEnemies);

    const FVector TurretLoc = GetActorLocation();
    const float RadiusSq = EffectRadius * EffectRadius;

    TSet<AActor*> InRangeNow;

    for (AActor* Enemy : AllEnemies)
    {
        if (!IsValid(Enemy)) continue;

        const float DistSq = FVector::DistSquared(TurretLoc, Enemy->GetActorLocation());
        if (DistSq <= RadiusSq)
        {
            InRangeNow.Add(Enemy);
        }
    }

    // Remove effects from enemies that left range (or died/became invalid).
    for (int32 i = AffectedEnemies.Num() - 1; i >= 0; --i)
    {
        FMicrowaveAffectedEnemy& Affected = AffectedEnemies[i];
        if (!IsValid(Affected.Enemy) || !InRangeNow.Contains(Affected.Enemy))
        {
            RemoveEffectsFromEnemy(Affected);
            AffectedEnemies.RemoveAt(i);
        }
    }

    // Add effects to newly-entered enemies.
    for (AActor* Enemy : InRangeNow)
    {
        const bool bAlreadyTracked = AffectedEnemies.ContainsByPredicate(
            [Enemy](const FMicrowaveAffectedEnemy& A) { return A.Enemy == Enemy; });

        if (!bAlreadyTracked)
        {
            FMicrowaveAffectedEnemy NewAffected;
            NewAffected.Enemy = Enemy;
            NewAffected.TimeUntilNextConfusionRoll = ConfusionCheckInterval;
            ApplyEffectsToEnemy(NewAffected);
            AffectedEnemies.Add(NewAffected);
        }
    }
}

void ATurretMicrowave::TickAffectedEnemies(float DeltaTime)
{
    for (FMicrowaveAffectedEnemy& Affected : AffectedEnemies)
    {
        if (!IsValid(Affected.Enemy)) continue;

        Affected.TimeUntilNextConfusionRoll -= DeltaTime;
        if (Affected.TimeUntilNextConfusionRoll <= 0.f)
        {
            Affected.TimeUntilNextConfusionRoll = ConfusionCheckInterval;
            TryConfuseEnemy(Affected.Enemy);
        }
    }
}

// ===== EFFECT APPLY / REMOVE =====

UCharacterMovementComponent* ATurretMicrowave::GetEnemyMovementComponent(AActor* Enemy) const
{
    if (!IsValid(Enemy)) return nullptr;
    return Enemy->FindComponentByClass<UCharacterMovementComponent>();
}

void ATurretMicrowave::ApplyEffectsToEnemy(FMicrowaveAffectedEnemy& Affected)
{
    AActor* Enemy = Affected.Enemy;
    if (!IsValid(Enemy)) return;

    // --- Movement slow ---
    if (UCharacterMovementComponent* MoveComp = GetEnemyMovementComponent(Enemy))
    {
        Affected.OriginalMaxWalkSpeed = MoveComp->MaxWalkSpeed;
        MoveComp->MaxWalkSpeed = Affected.OriginalMaxWalkSpeed * SlowMultiplier;
        Affected.bAppliedWalkSlow = true;
    }

    // --- Attack speed slow ---
    // Read/write the BP variable by name via reflection - no reparenting needed.
    // skipped if the enemy BP doesn't have this variable.
    if (FProperty* Prop = Enemy->GetClass()->FindPropertyByName(AttackSpeedVarName))
    {
        if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
        {
            float CurrentVal = FloatProp->GetPropertyValue_InContainer(Enemy);
            Affected.OriginalAttackSpeedMultiplier = CurrentVal;
            FloatProp->SetPropertyValue_InContainer(Enemy, CurrentVal * AttackSlowMultiplier);
            Affected.bAppliedAttackSlow = true;
        }
    }
}

void ATurretMicrowave::RemoveEffectsFromEnemy(FMicrowaveAffectedEnemy& Affected)
{
    AActor* Enemy = Affected.Enemy;
    if (!IsValid(Enemy)) return;

    if (Affected.bAppliedWalkSlow)
    {
        if (UCharacterMovementComponent* MoveComp = GetEnemyMovementComponent(Enemy))
        {
            MoveComp->MaxWalkSpeed = Affected.OriginalMaxWalkSpeed;
        }
    }

    if (Affected.bAppliedAttackSlow)
    {
        if (FProperty* Prop = Enemy->GetClass()->FindPropertyByName(AttackSpeedVarName))
        {
            if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
            {
                FloatProp->SetPropertyValue_InContainer(Enemy, Affected.OriginalAttackSpeedMultiplier);
            }
        }
    }
}

void ATurretMicrowave::RemoveAllEffects()
{
    for (FMicrowaveAffectedEnemy& Affected : AffectedEnemies)
    {
        RemoveEffectsFromEnemy(Affected);
    }
    AffectedEnemies.Empty();
}

// ===== CONFUSION =====

void ATurretMicrowave::TryConfuseEnemy(AActor* Enemy)
{
    if (!IsValid(Enemy)) return;

    if (FMath::FRand() > ConfusionChance) return; // roll failed

    // Find another enemy nearby to redirect targeting at.
    TArray<AActor*> AllEnemies;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), EnemyTag, AllEnemies);

    AActor* NewTarget = nullptr;
    float ClosestDistSq = ConfusionTargetSearchRadius * ConfusionTargetSearchRadius;
    const FVector EnemyLoc = Enemy->GetActorLocation();

    for (AActor* Other : AllEnemies)
    {
        if (!IsValid(Other) || Other == Enemy) continue;

        const float DistSq = FVector::DistSquared(EnemyLoc, Other->GetActorLocation());
        if (DistSq <= ClosestDistSq)
        {
            ClosestDistSq = DistSq;
            NewTarget = Other;
        }
    }

    if (!NewTarget) return; // no one nearby to be confused into attacking

    // Call the enemy BP's "ChaseTarget(TargetActor)" custom event by name,
    // without needing to reparent the enemy BP to a C++ class.
    UFunction* ChaseFunc = Enemy->FindFunction(ChaseTargetEventName);
    if (!ChaseFunc) return; // enemy BP doesn't implement this - skip gracefully

    // Build the parameter struct matching the function's single Actor param.
    struct FChaseTargetParams
    {
        AActor* TargetActor;
    };

    FChaseTargetParams Params;
    Params.TargetActor = NewTarget;

    Enemy->ProcessEvent(ChaseFunc, &Params);
}
