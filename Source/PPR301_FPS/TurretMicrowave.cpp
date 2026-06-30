#include "TurretMicrowave.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"

ATurretMicrowave::ATurretMicrowave()
{
    PrimaryActorTick.bCanEverTick = true;

    RadiusIndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RadiusIndicatorMesh"));
    RadiusIndicatorMesh->SetupAttachment(RootComponent);
    RadiusIndicatorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RadiusIndicatorMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
    RadiusIndicatorMesh->SetCastShadow(false);
    RadiusIndicatorMesh->SetVisibility(true);

    // Assign the built-in engine sphere mesh automatically.
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(
        TEXT("/Engine/BasicShapes/Sphere.Sphere")
    );
    if (SphereMeshAsset.Succeeded())
    {
        RadiusIndicatorMesh->SetStaticMesh(SphereMeshAsset.Object);
    }

    // NOTE: Do NOT set location or scale here in the constructor.
    // UpdateRadiusIndicatorTransform() handles both together in BeginPlay
    // so they can never fight each other.
}

void ATurretMicrowave::BeginPlay()
{
    Super::BeginPlay();

    // Set scale AND location together in one call so they're always in sync.
    UpdateRadiusIndicatorTransform();
    InitForceFieldMaterial();

    TimeUntilNextScan = 0.f;
    PulseTime = 0.f;
}

void ATurretMicrowave::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    RemoveAllEffects();
    Super::EndPlay(EndPlayReason);
}

// ===== FORCEFIELD =====

void ATurretMicrowave::UpdateRadiusIndicatorTransform()
{
    if (!RadiusIndicatorMesh || RadiusIndicatorMeshBaseRadius <= 0.f) return;

    const float Scale = EffectRadius / RadiusIndicatorMeshBaseRadius;

    // Scale and location must be set together so neither overwrites the other.
    // SetRelativeScale3D does NOT touch location, and SetRelativeLocation does
    RadiusIndicatorMesh->SetRelativeScale3D(FVector(Scale, Scale, Scale * 0.75f));
    RadiusIndicatorMesh->SetRelativeLocation(FVector(0.f, 0.f, ForceFieldHeightOffset));
}

void ATurretMicrowave::InitForceFieldMaterial()
{
    if (!RadiusIndicatorMesh) return;

    if (!RadiusIndicatorMaterial)
    {
        UE_LOG(LogTemp, Warning, TEXT("[TurretMicrowave] No RadiusIndicatorMaterial assigned — assign M_ForceField in the editor."));
        return;
    }

    ForceFieldMID = UMaterialInstanceDynamic::Create(RadiusIndicatorMaterial, this);
    if (!ForceFieldMID)
    {
        UE_LOG(LogTemp, Error, TEXT("[TurretMicrowave] Failed to create ForceField MID!"));
        return;
    }

    RadiusIndicatorMesh->SetMaterial(0, ForceFieldMID);

    // Colour is set once — doesn't change at runtime.
    ForceFieldMID->SetVectorParameterValue(TEXT("Color"), ForceFieldColor);

    // Sensible starting values so there's no pop on first frame.
    ForceFieldMID->SetScalarParameterValue(TEXT("Pulse"), PulseIntensity);
    ForceFieldMID->SetScalarParameterValue(TEXT("Opacity"), (OpacityMin + OpacityMax) * 0.5f);

    UE_LOG(LogTemp, Warning, TEXT("[TurretMicrowave] ForceField MID initialised."));
}

void ATurretMicrowave::TickForceField(float DeltaTime)
{
    if (!ForceFieldMID) return;

    PulseTime += DeltaTime;

    // Sin wave normalised to [0, 1].
    const float Wave = FMath::Sin(PulseTime * PulseSpeed * 2.f * PI) * 0.5f + 0.5f;
    const float PulseValue = FMath::Lerp(PulseIntensity * 0.4f, PulseIntensity, Wave);

    // Opacity slightly out of phase so brightness and transparency don't peak together.
    const float OpacityWave = FMath::Sin(PulseTime * PulseSpeed * 2.f * PI + 1.f) * 0.5f + 0.5f;
    const float OpacityValue = FMath::Lerp(OpacityMin, OpacityMax, OpacityWave);

    ForceFieldMID->SetScalarParameterValue(TEXT("Pulse"), PulseValue);
    ForceFieldMID->SetScalarParameterValue(TEXT("Opacity"), OpacityValue);
}

#if WITH_EDITOR
void ATurretMicrowave::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    const FName ChangedProp = PropertyChangedEvent.GetPropertyName();

    // Any property that affects the sphere transform — update both scale and location together.
    if (ChangedProp == GET_MEMBER_NAME_CHECKED(ATurretMicrowave, EffectRadius)          ||
        ChangedProp == GET_MEMBER_NAME_CHECKED(ATurretMicrowave, RadiusIndicatorMeshBaseRadius) ||
        ChangedProp == GET_MEMBER_NAME_CHECKED(ATurretMicrowave, ForceFieldHeightOffset))
    {
        UpdateRadiusIndicatorTransform();
    }

    if (ChangedProp == GET_MEMBER_NAME_CHECKED(ATurretMicrowave, ForceFieldColor))
    {
        if (ForceFieldMID)
        {
            ForceFieldMID->SetVectorParameterValue(TEXT("Color"), ForceFieldColor);
        }
    }
}
#endif

// ===== TICK =====

void ATurretMicrowave::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);

    TickForceField(DeltaTime);

    if (bIsPreview) return;

    TimeUntilNextScan -= DeltaTime;
    if (TimeUntilNextScan <= 0.f)
    {
        ScanForEnemiesInRange();
        TimeUntilNextScan = ScanInterval;
    }

    TickAffectedEnemies(DeltaTime);
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
        if (FVector::DistSquared(TurretLoc, Enemy->GetActorLocation()) <= RadiusSq)
            InRangeNow.Add(Enemy);
    }

    for (int32 i = AffectedEnemies.Num() - 1; i >= 0; --i)
    {
        FMicrowaveAffectedEnemy& Affected = AffectedEnemies[i];
        if (!IsValid(Affected.Enemy) || !InRangeNow.Contains(Affected.Enemy))
        {
            RemoveEffectsFromEnemy(Affected);
            AffectedEnemies.RemoveAt(i);
        }
    }

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

    if (UCharacterMovementComponent* MoveComp = GetEnemyMovementComponent(Enemy))
    {
        Affected.OriginalMaxWalkSpeed = MoveComp->MaxWalkSpeed;
        MoveComp->MaxWalkSpeed = Affected.OriginalMaxWalkSpeed * SlowMultiplier;
        Affected.bAppliedWalkSlow = true;
    }

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
            MoveComp->MaxWalkSpeed = Affected.OriginalMaxWalkSpeed;
    }

    if (Affected.bAppliedAttackSlow)
    {
        if (FProperty* Prop = Enemy->GetClass()->FindPropertyByName(AttackSpeedVarName))
        {
            if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
                FloatProp->SetPropertyValue_InContainer(Enemy, Affected.OriginalAttackSpeedMultiplier);
        }
    }
}

void ATurretMicrowave::RemoveAllEffects()
{
    for (FMicrowaveAffectedEnemy& Affected : AffectedEnemies)
        RemoveEffectsFromEnemy(Affected);
    AffectedEnemies.Empty();
}

// ===== CONFUSION =====

void ATurretMicrowave::TryConfuseEnemy(AActor* Enemy)
{
    if (!IsValid(Enemy)) return;
    if (FMath::FRand() > ConfusionChance) return;

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

    if (!NewTarget) return;

    UFunction* ChaseFunc = Enemy->FindFunction(ChaseTargetEventName);
    if (!ChaseFunc) return;

    struct FChaseTargetParams { AActor* TargetActor; };
    FChaseTargetParams Params;
    Params.TargetActor = NewTarget;
    Enemy->ProcessEvent(ChaseFunc, &Params);
}