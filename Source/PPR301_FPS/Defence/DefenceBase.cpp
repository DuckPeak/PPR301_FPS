#include "DefenceBase.h"

ADefenceBase::ADefenceBase()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ADefenceBase::BeginPlay()
{
	Super::BeginPlay();
}

void ADefenceBase::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ADefenceBase::SetUpOnPlaced()
{
	
}