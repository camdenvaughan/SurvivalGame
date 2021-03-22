// Copyright Camden Vaughan 2021. All Rights Reserved.


#include "PickupBase.h"
#include "Components/StaticMeshComponent.h"
#include "SurvivalCharacter.h"
#include "PlayerStatsComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APickupBase::APickupBase()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Comp"));
	RootComponent = MeshComp;
	
	IncreaseAmount = 30;
}

// Called when the game starts or when spawned
void APickupBase::BeginPlay()
{
	Super::BeginPlay();
	if (GetLocalRole() == ROLE_Authority)
	{
		SetReplicates(true);
	}
}

void APickupBase::UseItem(ASurvivalCharacter* Player) 
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (PickupType == EPickupType::E_Food)
		{
			if (Player->PlayerStatsComp->GetHunger() > 98.f) return; // Return if Stat is nearly full

			Player->PlayerStatsComp->AddHunger(IncreaseAmount);
		}
		else if (PickupType == EPickupType::E_Water)
		{
			if (Player->PlayerStatsComp->GetThirst() > 98.f) return; // Return if Stat is nearly full

			Player->PlayerStatsComp->AddThirst(IncreaseAmount);
		}
		else if (PickupType == EPickupType::E_Bandage)
		{
			if (Player->PlayerStatsComp->GetHealth() == 100.f) return; // Return if Health is full
			Player->PlayerStatsComp->AddHealth(IncreaseAmount);
		}
		Destroy();
	}
}



