#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WaveUI.h"
#include "Sound/SoundBase.h"     
#include "Kismet/GameplayStatics.h"
#include "TDSPlayerController.h"
#include "WaveManager.generated.h"



USTRUCT(BlueprintType)
struct FEnemySpawnInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    TSubclassOf<APawn> EnemyClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy", meta = (ClampMin = 1))
    int32 Count = 5;
};

USTRUCT(BlueprintType)
struct FWaveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    TArray<FEnemySpawnInfo> Enemies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    float SpawnDelay = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
    float StartDelay = 0.0f;
};

UCLASS()
class PPR301_FPS_API AWaveManager : public AActor
{
    GENERATED_BODY()

public:
    AWaveManager();
    /** Sound that plays when a new wave starts */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* WaveStartSound = nullptr;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    void StartNextWave();
    void SpawnEnemy();
    void HandleNextWave();
    void AddCashForWaveComplete();
    void GiveKillReward();
    void PlayWaveStartSound();
    int32 GetTotalEnemiesForCurrentWave() const;

public:

    // Wave settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Waves")
    TArray<FWaveData> Waves;

    UFUNCTION(BlueprintCallable)
    void OnEnemyKilled();

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
    int32 AliveEnemies = 0;
	bool bWaveCountdownActive = false;
	bool bWaveActive = false;

    float CountdownTimeRemaining = 0.0f;

    FTimerHandle SpawnTimerHandle;
    FTimerHandle NextWaveTimerHandle;

    // UI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI", meta=(AllowPrivateAccess="true"))
    TSubclassOf<UWaveUI> WaveUIClass;

    UPROPERTY()
    UWaveUI* WaveUI;
};