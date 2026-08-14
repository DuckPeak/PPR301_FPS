#include "TurretBase.h"

ATurretBase::ATurretBase()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// Root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	// Base
	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BaseMesh"));
	BaseMesh->SetupAttachment(Root);
	
	// Gun
	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GunMesh"));
	GunMesh->SetupAttachment(BaseMesh);
	
	// Muzzle
	MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint"));
	MuzzlePoint->SetupAttachment(GunMesh);
}

void ATurretBase::BeginPlay()
{
	Super::BeginPlay();
}

void ATurretBase::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}