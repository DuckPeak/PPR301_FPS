#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseEnemy.generated.h"

UCLASS()
class PPR301_FPS_API ABaseEnemy : public ACharacter
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, Category="TD")
	class AWaveManager* WaveManagerRef;

	UPROPERTY(BlueprintReadWrite, Category="TD")
	AActor* EndPoint;
};