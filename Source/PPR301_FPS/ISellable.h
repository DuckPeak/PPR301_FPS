#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ISellable.generated.h"

UINTERFACE(MinimalAPI)
class USellable : public UInterface
{
	GENERATED_BODY()
};

class PPR301_FPS_API ISellable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Sell")
	int32 GetSellCost();
};