#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ISellable.generated.h"

// The U-Class used by Unreal Engine for reflection and type-checking
UINTERFACE(MinimalAPI, Blueprintable)
class USellable : public UInterface
{
	GENERATED_BODY()

public: // <--- CRITICAL FIX: Add this line right here!
};

// The I-Class you actually use to implement or execute functions
class PPR301_FPS_API ISellable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction")
	int32 GetSellCost();
};