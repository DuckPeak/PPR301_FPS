#include "HealthBarComponent.h"
#include "Blueprint/UserWidget.h"

UHealthBarComponent::UHealthBarComponent()
{
	SetWidgetSpace(EWidgetSpace::World);
	SetDrawSize(FVector2D(100.f, 12.f));
	SetRelativeLocation(FVector(0.f, 0.f, 120.f));
	//SetVisibility(false);
	//SetBlendMode(EWidgetBlendMode::Transparent);
}

/*/void UHealthBarComponent::UpdateHealthBar(float CurrentHealth, float MaxHealth)
{
	UUserWidget* Widget = GetUserWidgetObject();
	if (!Widget) return;

	float Percent = FMath::Clamp(CurrentHealth / MaxHealth, 0.f, 1.f);

	UFunction* Func = Widget->FindFunction(FName("SetHealthPercent"));
	if (Func)
	{
		struct { float Percent; } Params;
		Params.Percent = Percent;
		Widget->ProcessEvent(Func, &Params);
	}

	// Hide when full health, show when damaged
	//SetVisibility(Percent < 1.f);
}

*/