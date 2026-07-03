#pragma once

#include "CoreMinimal.h"
#include "Turret.h"
#include "TurretMicrowave.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Microwave", meta = (ClampMin = "0.0"))
    float EffectRadius = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Microwave", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float SlowMultiplier = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Microwave", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float AttackSlowMultiplier = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Microwave", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ConfusionChance = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Microwave", meta = (ClampMin = "0.05"))
    float ConfusionCheckInterval = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Microwave", meta = (ClampMin = "0.0"))
    float ConfusionTargetSearchRadius = 800.f;

    UPROPERTY(EditAnywhere, Category = "Microwave")
    FName EnemyTag = FName("Enemy");

    UPROPERTY(EditAnywhere, Category = "Microwave")
    FName AttackSpeedVarName = FName("AttackSpeedMultiplier");

    UPROPERTY(EditAnywhere, Category = "Microwave")
    FName ChaseTargetEventName = FName("ChaseTarget");

    UPROPERTY(EditAnywhere, Category = "Microwave")
    FName ChaseTargetParamName = FName("TargetActor");

    UPROPERTY(EditAnywhere, Category = "Microwave", meta = (ClampMin = "0.02"))
    float ScanInterval = 0.25f;

    // ===== FORCEFIELD VISUAL =====

    // Assign M_ForceField (your translucent unlit material) here in the editor.
    UPROPERTY(EditAnywhere, Category = "Forcefield")
    UMaterialInterface* RadiusIndicatorMaterial;

    // Base radius of the sphere mesh asset (UE default sphere = 50 units radius).
    UPROPERTY(EditAnywhere, Category = "Forcefield", meta = (ClampMin = "1.0"))
    float RadiusIndicatorMeshBaseRadius = 50.f;

    // How high above the turret root the sphere is centered.
    // Change this in editor to move the sphere up/down without recompiling.
    UPROPERTY(EditAnywhere, Category = "Forcefield")
    float ForceFieldHeightOffset = -200.f;

    // The cyan-blue colour of the forcefield.
    UPROPERTY(EditAnywhere, Category = "Forcefield")
    FLinearColor ForceFieldColor = FLinearColor(0.f, 0.8f, 1.f, 1.f);

    // Base emissive intensity multiplied by the pulse wave.
    UPROPERTY(EditAnywhere, Category = "Forcefield", meta = (ClampMin = "0.0"))
    float PulseIntensity = 0.3f;

    // How fast the pulse breathes — full cycles per second.
    UPROPERTY(EditAnywhere, Category = "Forcefield", meta = (ClampMin = "0.1"))
    float PulseSpeed = 1.0f;

    // Minimum opacity at the trough of the pulse.
    UPROPERTY(EditAnywhere, Category = "Forcefield", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float OpacityMin = 0.03f;

    // Maximum opacity at the peak of the pulse.
    UPROPERTY(EditAnywhere, Category = "Forcefield", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float OpacityMax = 0.12f;

    // The static mesh component showing the field radius.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Forcefield")
    UStaticMeshComponent* RadiusIndicatorMesh;

    // Runtime dynamic material instance — driven every tick.
    UPROPERTY()
    UMaterialInstanceDynamic* ForceFieldMID;

    // Accumulated time used to drive the Sin pulse.
    float PulseTime = 0.f;

    void InitForceFieldMaterial();
    void TickForceField(float DeltaTime);

    // Sets scale AND re-applies height offset so they never fight each other.
    void UpdateRadiusIndicatorTransform();

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