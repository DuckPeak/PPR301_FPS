#pragma once
#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "HealthBarComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PPR301_FPS_API UHealthBarComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:
	UHealthBarComponent();

	// Call whenever the obj takes damage
	UFUNCTION(BlueprintCallable, Category = "Health UI")
	void UpdateHealthBar(float CurrentHealth, float MaxHealth);
};