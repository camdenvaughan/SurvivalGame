// Copyright Camden Vaughan 2021. All Rights Reserved.


#include "Survival/Public/Weapons/AmmoBase.h"
#include "Survival/Public/Components/PlayerStatsComponent.h"

#include "Components/StaticMeshComponent.h"

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

	SetupAmmoPickup(AmmoType, AmmoAmount);
}

void AAmmoBase::SetupAmmoPickup(EAmmoType SetAmmoType, int32 Value)
{
	AmmoType = SetAmmoType;
	if (AmmoType == EAmmoType::E_AssaultAmmo)
	{
		AmmoMesh->SetStaticMesh(AssaultMesh);
	}
	else if (AmmoType == EAmmoType::E_SniperAmmo)
	{
		AmmoMesh->SetStaticMesh(SniperMesh);
	}
	else if (AmmoType == EAmmoType::E_ShotgunAmmo)
	{
		AmmoMesh->SetStaticMesh(ShotgunMesh);
	}
	AmmoAmount = Value;
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


