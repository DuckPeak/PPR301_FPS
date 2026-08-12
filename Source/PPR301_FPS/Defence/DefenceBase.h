#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PPR301_FPS/ISellable.h"
#include "DefenceBase.generated.h"

UCLASS()
class PPR301_FPS_API ADefenceBase : public AActor, public ISellable
{
	GENERATED_BODY()
	
public:
	ADefenceBase();

	/**
	 * @brief The flag to check if the defence is a preview used in build mode.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsPreview = false;
	
	virtual void Tick(float DeltaTime) override;

	/**
	 * @brief Get the sell cost of the defence.
	 * @return Return the sell cost of the defence.
	 */
	virtual int32 GetSellCost_Implementation() override
	{
		return Cost;
	}
	
	/**
	 * @brief Override to set up the defence when placed.
	 */
	virtual void SetUpOnPlaced();

protected:
	// TODO: Should selling cost more than buying cost?
	/**
	 * @brief The cost of the defence.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Build")
	int32 Cost = 100;
	
	virtual void BeginPlay() override;
};