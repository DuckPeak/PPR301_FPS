#include "WaveUI.h"
#include "Components/CanvasPanel.h"
#include "Components/TextBlock.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetTree.h"

void UWaveUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (WaveText)
	{
		WaveText->SetText(FText::FromString("WAVE UI WORKING"));
		//WaveText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	}
}

void UWaveUI::UpdateWave(int32 Wave, float Countdown)
{
	if (WaveText)
	{
		FString Text = FString::Printf(
			TEXT("Wave: %d\nNext Wave In: %.1f"),
			Wave + 1,
			FMath::Max(0.0f, Countdown)
		);

		WaveText->SetText(FText::FromString(Text));
	}
}