#include "AmbientLayeredSound.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AAmbientLayeredSound::AAmbientLayeredSound()
{
    PrimaryActorTick.bCanEverTick = false;

    BaseAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("BaseAudio"));
    LayerAudioComp = CreateDefaultSubobject<UAudioComponent>(TEXT("LayerAudio"));

    BaseAudioComp->bAutoActivate = false;
    LayerAudioComp->bAutoActivate = false;
}

void AAmbientLayeredSound::BeginPlay()
{
    Super::BeginPlay();

    // === BASE LOOPING SOUND ===
    if (BaseLoopSound && BaseAudioComp)
    {
        BaseAudioComp->SetSound(BaseLoopSound);
        BaseAudioComp->SetVolumeMultiplier(BaseVolume);

        BaseAudioComp->Play();

        UE_LOG(LogTemp, Warning, TEXT("Base ambient loop started successfully"));
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BaseLoopSound is not assigned!"));
    }

    // Setup layered component
    if (LayerAudioComp)
    {
        LayerAudioComp->SetVolumeMultiplier(0.0f);
    }

    // Start random layered SFX
    if (LayeredSFXArray.Num() > 0)
    {
        float FirstDelay = FMath::RandRange(MinInterval, MaxInterval);
        GetWorldTimerManager().SetTimer(
            LayerTimerHandle,
            this,
            &AAmbientLayeredSound::PlayRandomLayeredSFX,
            FirstDelay,
            false
        );
    }
}

void AAmbientLayeredSound::PlayRandomLayeredSFX()
{
    if (LayeredSFXArray.Num() == 0 || !LayerAudioComp) 
        return;

    // Pick random sound from array
    int32 RandomIndex = FMath::RandRange(0, LayeredSFXArray.Num() - 1);
    USoundBase* SelectedSound = LayeredSFXArray[RandomIndex];

    if (SelectedSound)
    {
        LayerAudioComp->SetSound(SelectedSound);
        LayerAudioComp->SetVolumeMultiplier(0.0f);
        LayerAudioComp->Play();

        // Fade In
        LayerAudioComp->FadeIn(LayerFadeInTime, LayerVolume);

        // Calculate when to fade out (sound duration + fade in time)
        float SoundDuration = SelectedSound->GetDuration();
        float FadeOutDelay = SoundDuration + LayerFadeInTime - LayerFadeOutTime;

        if (FadeOutDelay > 0.0f)
        {
            // Schedule fade out
            GetWorldTimerManager().SetTimer(
                LayerTimerHandle,
                [this]()
                {
                    if (LayerAudioComp)
                    {
                        LayerAudioComp->FadeOut(LayerFadeOutTime, 0.0f);
                    }
                },
                FadeOutDelay,
                false
            );
        }
    }

    // Schedule next layered sound
    float NextInterval = FMath::RandRange(MinInterval, MaxInterval);
    GetWorldTimerManager().SetTimer(
        LayerTimerHandle,
        this,
        &AAmbientLayeredSound::PlayRandomLayeredSFX,
        NextInterval,
        false
    );
}