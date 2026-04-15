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

    if (bWaveCountdownActive && CountdownTimeRemaining > 0)
    {
        CountdownTimeRemaining -= DeltaTime;

        if (CountdownTimeRemaining < 0)
        {
            CountdownTimeRemaining = 0;
        }
    }

    if (WaveUI)
    {
        WaveUI->UpdateWave(CurrentWaveIndex, CountdownTimeRemaining, bWaveActive);
    }
}

void AWaveManager::StartNextWave()
{
    if (!Waves.IsValidIndex(CurrentWaveIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("All waves complete!"));

        if (GameCompleteUIClass && GetWorld())
        {
            APlayerController* PC = GetWorld()->GetFirstPlayerController();

            if (PC)
            {
                GameCompleteUI = CreateWidget<UUserWidget>(PC, GameCompleteUIClass);

                if (GameCompleteUI)
                {
                    GameCompleteUI->AddToViewport(9999);
                    PC->SetInputMode(FInputModeUIOnly());
                    PC->bShowMouseCursor = true;
                }
            }
        }

        return;
    }

    FWaveData& Wave = Waves[CurrentWaveIndex];

    SpawnedCount = 0;
	AliveEnemies = 0;
    bWaveActive = true;
    bWaveCountdownActive = false;
    CountdownTimeRemaining = 0.0f;

    UE_LOG(LogTemp, Warning, TEXT("Starting Wave %d"), CurrentWaveIndex + 1);

    GetWorld()->GetTimerManager().SetTimer(
        SpawnTimerHandle,
        this,
        &AWaveManager::SpawnEnemy,
        Wave.SpawnDelay,
        true,
        Wave.StartDelay
    );
}

void AWaveManager::HandleNextWave()
{
    GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(NextWaveTimerHandle);

	bWaveActive = false;
    bWaveCountdownActive = false;
    CountdownTimeRemaining = 0.0f;
    
    // === ADD CASH REWARD HERE ===
    AddCashForWaveComplete();

    CurrentWaveIndex++;

    StartNextWave();
}


void AWaveManager::SpawnEnemy()
{
    if (!Waves.IsValidIndex(CurrentWaveIndex)) return;
    if (SpawnPoints.Num() == 0) return;

    FWaveData& Wave = Waves[CurrentWaveIndex];
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
	AliveEnemies++;

    if (SpawnedCount >= Wave.EnemyCount)
    {
        GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
    }
}

void AWaveManager::OnEnemyKilled()
{
    AliveEnemies--;

    if (AliveEnemies < 0)
    {
        AliveEnemies = 0;
    }

    if (Waves.IsValidIndex(CurrentWaveIndex))
    {
        FWaveData& Wave = Waves[CurrentWaveIndex];

        if (SpawnedCount >= Wave.EnemyCount && AliveEnemies == 0)
        {
            bWaveActive = false;
            bWaveCountdownActive = true;

            float NextWaveDelay = 10.0f + (CurrentWaveIndex * 2.0f);
            CountdownTimeRemaining = NextWaveDelay;

            GetWorld()->GetTimerManager().SetTimer(
                NextWaveTimerHandle,
                this,
                &AWaveManager::HandleNextWave,
                NextWaveDelay,
                false
            );
        }
    }
}


void AWaveManager::AddCashForWaveComplete()
{
    if (!GetWorld()) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) 
    {
        UE_LOG(LogTemp, Warning, TEXT("WaveManager: No PlayerController found!"));
        return;
    }

    ATDSPlayerController* TDSPC = Cast<ATDSPlayerController>(PC);
    if (TDSPC)
    {
        int32 Reward = 25 + (CurrentWaveIndex * 25);

        TDSPC->AddPlayerCash(Reward);

        UE_LOG(LogTemp, Warning, TEXT("Wave %d completed! +%d cash awarded. Total cash now: %d"), 
               CurrentWaveIndex + 1, Reward, TDSPC->PlayerCash);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("WaveManager: PlayerController is not ATDSPlayerController!"));
    }
}