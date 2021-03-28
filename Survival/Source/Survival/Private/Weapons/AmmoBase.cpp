// Copyright Camden Vaughan 2021. All Rights Reserved.


#include "Survival/Public/Weapons/AmmoBase.h"
#include "Components/StaticMeshComponent.h"
#include "Survival/PlayerStatsComponent.h"

// Sets default values
AAmmoBase::AAmmoBase()
{
	AmmoMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ammo Mesh"));
	RootComponent = AmmoMesh;
	bReplicates = true;
}

// Called when the game starts or when spawned
void AAmmoBase::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAmmoBase::InteractedWith(UPlayerStatsComponent* PlayerStatsComp)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hello From Ammo Interaction"));
		PlayerStatsComp->AddAmmo(AmmoAmount, AmmoType);
		Destroy();
	}
}

EAmmoType AAmmoBase::GetAmmoType() const
{
	return AmmoType;
}


