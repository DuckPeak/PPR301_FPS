#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "WaveUI.generated.h"

UCLASS()
class PPR301_FPS_API UWaveUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	void UpdateWave(int32 Wave, float Countdown);

private:
	// Bind to TextBlock in Blueprint
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WaveText;
};