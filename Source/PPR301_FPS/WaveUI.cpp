#include "WaveUI.h"
#include "Components/TextBlock.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"

void UWaveUI::NativeConstruct()
{
	Super::NativeConstruct();

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());

	WaveText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WaveText"));

	if (WaveText && Root)
	{
		WaveText->SetText(FText::FromString("Wave UI"));

		Root->AddChild(WaveText);

		WidgetTree->RootWidget = Root;
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