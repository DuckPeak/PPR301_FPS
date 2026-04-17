#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "Components/AudioComponent.h"
#include "AmbientLayeredSound.generated.h"

UCLASS()
class PPR301_FPS_API AAmbientLayeredSound : public AActor
{
	GENERATED_BODY()

public:
	AAmbientLayeredSound();

	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* BaseLoopSound = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TArray<USoundBase*> LayeredSFXArray;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float BaseVolume = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float LayerVolume = 0.8f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float LayerFadeInTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float LayerFadeOutTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float MinInterval = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float MaxInterval = 40.0f;

private:
	UPROPERTY()
	UAudioComponent* BaseAudioComp = nullptr;

	UPROPERTY()
	UAudioComponent* LayerAudioComp = nullptr;

	FTimerHandle LayerTimerHandle;

	void PlayRandomLayeredSFX();
};