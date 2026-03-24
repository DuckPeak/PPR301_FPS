#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveUI.h"
#include "WaveManager.generated.h"

USTRUCT(BlueprintType)
struct FWaveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<APawn> EnemyClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 EnemyCount = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnDelay = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float StartDelay = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TimeUntilNextWave = 10.0f;
};

UCLASS()
class PPR301_FPS_API AWaveManager : public AActor
{
    GENERATED_BODY()

public:
    AWaveManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    void StartNextWave();
    void SpawnEnemy();
    void HandleNextWave();

public:

    // Wave settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Waves")
    TArray<FWaveData> Waves;

    // TD Setup
    UPROPERTY(EditAnywhere, Category="TD Setup")
    TArray<AActor*> SpawnPoints;

    UPROPERTY(EditAnywhere, Category="TD Setup")
    AActor* EndPoint;
    
    // Game Complete UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UUserWidget> GameCompleteUIClass;

    UPROPERTY()
    UUserWidget* GameCompleteUI;

private:
    int32 CurrentWaveIndex = 0;
    int32 SpawnedCount = 0;

    float CountdownTimeRemaining = 0.0f;

    FTimerHandle SpawnTimerHandle;
    FTimerHandle NextWaveTimerHandle;

    // UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UWaveUI> WaveUIClass;

    UPROPERTY()
    UWaveUI* WaveUI;
};