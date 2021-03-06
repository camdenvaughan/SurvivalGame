// Copyright Camden Vaughan 2021. All Rights Reserved.


#include "Survival/Public/Actors/PickupBase.h"
#include "Survival/Public/Character/SurvivalCharacter.h"
#include "Survival/Public/Components/PlayerStatsComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/Texture.h"

// Sets default values
APickupBase::APickupBase()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh Comp"));
	RootComponent = MeshComp;

	Icon = CreateDefaultSubobject<UTexture>(TEXT("Icon Image"));

	bReplicates = true;
	IncreaseAmount = 30;
}

// Called when the game starts or when spawned
void APickupBase::BeginPlay()
{
	Super::BeginPlay();
	SetReplicateMovement(true);
}

void APickupBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APickupBase, bIsObjectPickedUp);

}

void APickupBase::OnRep_PickedUp() 
{
	this->MeshComp->SetHiddenInGame(bIsObjectPickedUp);
	this->SetActorEnableCollision(!bIsObjectPickedUp);
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

void APickupBase::IsInInventory(bool bIsInInventory) 
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bIsObjectPickedUp = bIsInInventory;
		OnRep_PickedUp();
	}
}

UTexture* APickupBase::GetIcon() const
{
		return Icon;
}



