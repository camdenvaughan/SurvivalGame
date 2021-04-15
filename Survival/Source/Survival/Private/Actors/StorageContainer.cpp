// Copyright Camden Vaughan 2021. All Rights Reserved.

#include "Survival/Public/Actors/StorageContainer.h"
#include "Survival/Public/Components/InventoryComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AStorageContainer::AStorageContainer()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	RootComponent = StaticMesh;

	InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("Inventory"));
	InventoryComp->SetIsReplicated(true);
	bReplicates = true;
	
	bIsOpen = false;
}

// Called when the game starts or when spawned
void AStorageContainer::BeginPlay()
{
	Super::BeginPlay();
}

void AStorageContainer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


	DOREPLIFETIME(AStorageContainer, bIsOpen);
}

void AStorageContainer::OpenChest(bool bOpened)
{
	Server_OpenedChest(bOpened);
}

bool AStorageContainer::Server_OpenedChest_Validate(bool bOpened)
{
	return true;
}

void AStorageContainer::Server_OpenedChest_Implementation(bool bOpened)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bIsOpen = bOpened;
	}
}

UInventoryComponent* AStorageContainer::GetInventoryComponent() const
{
	return InventoryComp;
}

bool AStorageContainer::IsChestOpen() const
{
	return bIsOpen;
}
