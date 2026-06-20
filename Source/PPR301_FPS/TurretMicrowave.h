#pragma once

#include "CoreMinimal.h"
#include "Turret.h"
#include "TurretMicrowave.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UCharacterMovementComponent;

USTRUCT()
struct FMicrowaveAffectedEnemy
{
    GENERATED_BODY()

    UPROPERTY()
    AActor* Enemy = nullptr;
    
    UPROPERTY()
    float OriginalMaxWalkSpeed = 0.f;

    UPROPERTY()
    float OriginalAttackSpeedMultiplier = 1.f;

    // Local timer for the confusion roll, decoupled from global tick.
    UPROPERTY()
    float TimeUntilNextConfusionRoll = 0.f;
    
    UPROPERTY()
    bool bAppliedAttackSlow = false;

    UPROPERTY()
    bool bAppliedWalkSlow = false;
};

UCLASS()
class PPR301_FPS_API ATurretMicrowave : public ATurret
{
    GENERATED_BODY()

public:
    ATurretMicrowave();

    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    // ===== MICROWAVE FIELD SETTINGS =====

    // Radius of the slow/confuse field, in cm (UE units).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Microwave", meta = (ClampMin = "0.0"))
    float EffectRadius = 800.f;

    // Multiplier applied to MaxWalkSpeed for affected enemies. 0.5 = 50% speed.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Microwave", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SlowMultiplier = 0.25f;

    // Multiplier applied to the enemy's AttackSpeedMultiplier variable while affected.
    // 0.5 = attacks twice as slowly (assuming enemy BP treats this as a rate multiplier).
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Microwave", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AttackSlowMultiplier = 0.5f;

    // Chance [0-1] per confusion check that an affected enemy gets confused.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Microwave", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ConfusionChance = 0.1f;

    // How often (seconds) each affected enemy rolls for confusion.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Microwave", meta = (ClampMin = "0.05"))
    float ConfusionCheckInterval = 2.0f;

    // How far a confused enemy will look for another enemy to target.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Microwave", meta = (ClampMin = "0.0"))
    float ConfusionTargetSearchRadius = 800.f;

    // Tag used to find enemies, matches ATurret's convention.
    UPROPERTY(EditAnywhere, Category = "Microwave")
    FName EnemyTag = FName("Enemy");

    // Name of the float variable on the enemy BP used for attack-rate scaling.
    UPROPERTY(EditAnywhere, Category = "Microwave")
    FName AttackSpeedVarName = FName("AttackSpeedMultiplier");

    // Name of the custom event on the enemy BP used to (re)target.
    UPROPERTY(EditAnywhere, Category = "Microwave")
    FName ChaseTargetEventName = FName("ChaseTarget");

    // Name of the Actor parameter on the ChaseTarget event.
    UPROPERTY(EditAnywhere, Category = "Microwave")
    FName ChaseTargetParamName = FName("TargetActor");

    // How often (seconds) we rescan for enemies entering/leaving the radius.
    // Doesn't need to be every frame.
    UPROPERTY(EditAnywhere, Category = "Microwave", meta = (ClampMin = "0.02"))
    float ScanInterval = 0.25f;

    // ===== VISUAL RADIUS INDICATOR =====

    // Translucent sphere mesh showing the field radius, always visible in-game.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Microwave")
    UStaticMeshComponent* RadiusIndicatorMesh;

    // Material used for the radius indicator (assign a translucent material in the editor).
    UPROPERTY(EditAnywhere, Category = "Microwave")
    UMaterialInterface* RadiusIndicatorMaterial;

    // Base radius of the sphere mesh asset itself (UE default sphere = 50 units radius),
    UPROPERTY(EditAnywhere, Category = "Microwave", meta = (ClampMin = "1.0"))
    float RadiusIndicatorMeshBaseRadius = 50.f;

    void UpdateRadiusIndicatorScale();

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

    // ===== CORE LOGIC =====

    UPROPERTY()
    TArray<FMicrowaveAffectedEnemy> AffectedEnemies;

    float TimeUntilNextScan = 0.f;

    void ScanForEnemiesInRange();
    void TickAffectedEnemies(float DeltaTime);

    void ApplyEffectsToEnemy(FMicrowaveAffectedEnemy& Affected);
    void RemoveEffectsFromEnemy(FMicrowaveAffectedEnemy& Affected);
    void RemoveAllEffects();

    void TryConfuseEnemy(AActor* Enemy);

    UCharacterMovementComponent* GetEnemyMovementComponent(AActor* Enemy) const;
};
