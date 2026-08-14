#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

UCLASS()
class PPR301_FPS_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:
	AProjectileBase();
	
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
};
