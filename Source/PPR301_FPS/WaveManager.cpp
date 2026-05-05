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
    PlayWaveStartSound();
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
    if (!Waves.IsValidIndex(CurrentWaveIndex) || SpawnPoints.Num() == 0)
    {
        GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
        return;
    }

    FWaveData& Wave = Waves[CurrentWaveIndex];

    int32 TotalToSpawn = GetTotalEnemiesForCurrentWave();

    if (SpawnedCount >= TotalToSpawn)
    {
        GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
        return;
    }

    // === Choose enemy type (Improved & Fairer) ===
    TSubclassOf<APawn> ChosenClass = nullptr;

    // Build a list of valid enemy types with their counts
    TArray<FEnemySpawnInfo> ValidEnemies;
    for (const FEnemySpawnInfo& Info : Wave.Enemies)
    {
        if (Info.EnemyClass != nullptr && Info.Count > 0)
        {
            ValidEnemies.Add(Info);
        }
    }

    if (ValidEnemies.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("WaveManager: No valid enemy classes in wave!"));
        GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
        return;
    }

    // Simple and reliable random selection
    if (ValidEnemies.Num() == 1)
    {
        ChosenClass = ValidEnemies[0].EnemyClass;
    }
    else
    {
        // Weighted random based on Count
        int32 TotalWeight = 0;
        for (const FEnemySpawnInfo& Info : ValidEnemies)
        {
            TotalWeight += Info.Count;
        }

        int32 Roll = FMath::RandRange(0, TotalWeight - 1);
        int32 Accum = 0;

        for (const FEnemySpawnInfo& Info : ValidEnemies)
        {
            Accum += Info.Count;
            if (Roll < Accum)
            {
                ChosenClass = Info.EnemyClass;
                break;
            }
        }
    }

    if (ChosenClass == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to choose enemy class"));
        return;
    }

    // === Spawn the enemy ===
    int32 SpawnIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
    FVector SpawnLoc = SpawnPoints[SpawnIndex]->GetActorLocation();

    APawn* SpawnedPawn = GetWorld()->SpawnActor<APawn>(ChosenClass, SpawnLoc, FRotator::ZeroRotator);

    if (SpawnedPawn)
    {
        if (ABaseEnemy* Enemy = Cast<ABaseEnemy>(SpawnedPawn))
        {
            Enemy->WaveManagerRef = this;
            Enemy->EndPoint = EndPoint;
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Failed to spawn actor of class %s"), *ChosenClass->GetName());
    }

    SpawnedCount++;
    AliveEnemies++;
}

void AWaveManager::OnEnemyKilled()
{
    AliveEnemies--;

    if (AliveEnemies < 0)
        AliveEnemies = 0;

    // ADD KILL REWARD HERE 
    GiveKillReward();

    if (Waves.IsValidIndex(CurrentWaveIndex))
    {
        FWaveData& Wave = Waves[CurrentWaveIndex];

        if (SpawnedCount >= GetTotalEnemiesForCurrentWave() && AliveEnemies == 0)
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
void AWaveManager::GiveKillReward()
{
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (ATDSPlayerController* TDSPC = Cast<ATDSPlayerController>(PC))
    {
        TDSPC->AddPlayerCash(2);   // 2
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

void AWaveManager::PlayWaveStartSound()
{
    if (WaveStartSound && GetWorld())
    {
        // Play as 2D sound (global / UI-style, perfect for wave announcements)
        UGameplayStatics::PlaySound2D(this, WaveStartSound, 0.4f, 1.0f, 0.0f);

        UE_LOG(LogTemp, Warning, TEXT("Wave %d started - Playing WaveStartSound"), CurrentWaveIndex + 1);
    }
    else if (!WaveStartSound)
    {
        UE_LOG(LogTemp, Warning, TEXT("WaveStartSound is not assigned!"));
    }
}

int32 AWaveManager::GetTotalEnemiesForCurrentWave() const
{
    if (!Waves.IsValidIndex(CurrentWaveIndex)) return 0;

    int32 Total = 0;
    for (const FEnemySpawnInfo& Info : Waves[CurrentWaveIndex].Enemies)
    {
        Total += Info.Count;
    }
    return Total;
}