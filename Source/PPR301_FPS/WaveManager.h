#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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
    float StartDelay = 2.0f;
};

UCLASS()
class PPR301_FPS AWaveManager : public AActor
{
    GENERATED_BODY()

public:
    AWaveManager();

protected:
    virtual void BeginPlay() override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Waves")
    TArray<FWaveData> Waves;

    UPROPERTY(BlueprintReadOnly)
    int32 CurrentWaveIndex = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 EnemiesAlive = 0;

private:
    FTimerHandle SpawnTimerHandle;
    int32 SpawnedCount = 0;

    void StartNextWave();
    void SpawnEnemy();

public:
    UFUNCTION(BlueprintCallable)
    void EnemyDied();
};