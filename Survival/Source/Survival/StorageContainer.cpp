// Copyright Camden Vaughan 2021. All Rights Reserved.

#include "StorageContainer.h"
#include "Inventory.h"

#include "Components/StaticMeshComponent.h"

// Sets default values
AStorageContainer::AStorageContainer()
{
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	RootComponent = StaticMesh;

	InventoryComp = CreateDefaultSubobject<UInventory>(TEXT("Inventory"));
	bReplicates = true;
}

// Called when the game starts or when spawned
void AStorageContainer::BeginPlay()
{
	Super::BeginPlay();
	
}

UInventory* AStorageContainer::GetInventoryComponent()
{
	return InventoryComp;
}



