#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlayerBase.generated.h"

UCLASS()
class PPR301_FPS_API APlayerBase : public APlayerController
{
	GENERATED_BODY()
	
public:	
	APlayerBase();
	
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;
};