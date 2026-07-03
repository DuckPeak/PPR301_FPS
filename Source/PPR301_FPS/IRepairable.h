#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IRepairable.generated.h"

UINTERFACE(BlueprintType)
class PPR301_FPS_API URepairable : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implement on any Blueprint that has CurrentHealth / MaxHealth and should be
 * repairable in Build Mode's Repair Mode (turrets, walls, etc).
 */
class PPR301_FPS_API IRepairable
{
	GENERATED_BODY()

public:
	// Return true if CurrentHealth < MaxHealth. Implement in the Blueprint's event graph
	// (e.g. "return CurrentHealth < MaxHealth").
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair")
	bool NeedsRepair() const;

	// Set CurrentHealth = MaxHealth (and do any repair FX/sound you want) in the
	// Blueprint's event graph implementation of this event.
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Repair")
	void Repair();
};