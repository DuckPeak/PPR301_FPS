#include "WaveManager.h"
#include "BaseEnemy.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Blueprint/UserWidget.h"
#include "WaveUI.h"

AWaveManager::AWaveManager()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AWaveManager::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Warning, TEXT("WaveManager BeginPlay running"));

    if (WaveUIClass && GetWorld() && GetWorld()->GetFirstPlayerController())
    {
        WaveUI = CreateWidget<UWaveUI>(
            GetWorld()->GetFirstPlayerController(),
            WaveUIClass
        );

        if (WaveUI)
        {
            WaveUI->AddToViewport(9999);
            UE_LOG(LogTemp, Warning, TEXT("Wave UI Created"));
        }
    }

    StartNextWave();
}

void AWaveManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CountdownTimeRemaining > 0)
    {
        CountdownTimeRemaining -= DeltaTime;
    }

    if (WaveUI)
    {
        WaveUI->UpdateWave(CurrentWaveIndex, CountdownTimeRemaining);
    }
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

    UE_LOG(LogTemp, Warning, TEXT("Starting Wave %d"), CurrentWaveIndex + 1);

    GetWorld()->GetTimerManager().SetTimer(
        SpawnTimerHandle,
        this,
        &AWaveManager::SpawnEnemy,
        Wave.SpawnDelay,
        true,
        Wave.StartDelay
    );

    CountdownTimeRemaining = Wave.TimeUntilNextWave;

    GetWorld()->GetTimerManager().SetTimer(
        NextWaveTimerHandle,
        this,
        &AWaveManager::HandleNextWave,
        Wave.TimeUntilNextWave,
        false
    );
}

void AWaveManager::HandleNextWave()
{
    GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);

    CurrentWaveIndex++;

    StartNextWave();
}

void AWaveManager::SpawnEnemy()
{
    if (!Waves.IsValidIndex(CurrentWaveIndex)) return;
    if (SpawnPoints.Num() == 0) return;

    FWaveData& Wave = Waves[CurrentWaveIndex];

    if (SpawnedCount >= Wave.EnemyCount)
    {
        GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
        return;
    }

    int32 Index = FMath::RandRange(0, SpawnPoints.Num() - 1);

    FVector SpawnLocation = SpawnPoints[Index]->GetActorLocation();
    FRotator SpawnRotation = FRotator::ZeroRotator;

    if (Wave.EnemyClass)
    {
        APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(
            Wave.EnemyClass,
            SpawnLocation,
            SpawnRotation
        );

        if (SpawnedPawn)
        {
            ABaseEnemy* Enemy = Cast<ABaseEnemy>(SpawnedPawn);

            if (Enemy)
            {
                Enemy->WaveManagerRef = this;
                Enemy->EndPoint = EndPoint;
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Enemy is not BaseEnemy! FIX YOUR BP"));
            }
        }
    }

    SpawnedCount++;
}