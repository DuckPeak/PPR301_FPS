#include "WaveManager.h"
#include "TimerManager.h"
#include "Engine/World.h"

AWaveManager::AWaveManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AWaveManager::BeginPlay()
{
    Super::BeginPlay();

    StartNextWave();
}

void AWaveManager::StartNextWave()
{
    if (!Waves.IsValidIndex(CurrentWaveIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("All waves complete!"));
        return;
    }

    FWaveData& Wave = Waves[CurrentWaveIndex];

    SpawnedCount = 0;

    GetWorld()->GetTimerManager().SetTimer(
        SpawnTimerHandle,
        this,
        &AWaveManager::SpawnEnemy,
        Wave.SpawnDelay,
        true,
        Wave.StartDelay
    );
}

void AWaveManager::SpawnEnemy()
{
    if (!Waves.IsValidIndex(CurrentWaveIndex)) return;

    FWaveData& Wave = Waves[CurrentWaveIndex];

    if (SpawnedCount >= Wave.EnemyCount)
    {
        GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
        return;
    }

    FVector SpawnLocation = GetActorLocation();
    FRotator SpawnRotation = FRotator::ZeroRotator;

    if (Wave.EnemyClass)
    {
        GetWorld()->SpawnActor<APawn>(Wave.EnemyClass, SpawnLocation, SpawnRotation);
        EnemiesAlive++;
    }

    SpawnedCount++;
}

void AWaveManager::EnemyDied()
{
    EnemiesAlive--;

    if (EnemiesAlive <= 0)
    {
        CurrentWaveIndex++;
        StartNextWave();
    }
}