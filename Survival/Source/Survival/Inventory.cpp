// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory.h"
#include "PickupBase.h"
#include "SurvivalCharacter.h"
#include "StorageContainer.h"

#include "Net/UnrealNetwork.h"

#define OUT
// Sets default values for this component's properties
UInventory::UInventory()
{
	SetIsReplicated(true);

	InventorySize = 16;
}


// Called when the game starts
void UInventory::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}
void UInventory::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


	DOREPLIFETIME(UInventory, Items);
}


bool UInventory::AddItem(APickupBase* Item) 
{
	Items.Add(Item);
	Item->IsInInventory(true);
	return true;
}


void UInventory::DropAllInventory() 
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		for (APickupBase* Pickup : Items)
		{
			DropItem(Pickup);
		}
		Items.Empty();
	}
}

bool UInventory::CheckIfClientHasItem(APickupBase* Item)
{
	for (APickupBase* Pickup : Items)
	{
		if (Pickup == Item)
		{
			return true;
		}
	}
	return false;
}
bool UInventory::RemoveItemFromInventory(APickupBase* Item)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		int32 Counter = 0;
		for (APickupBase* Pickup : Items)
		{
			if (Pickup == Item)
			{
				Items.RemoveAt(Counter);
				return true;			
			}
			++Counter;
		}
	}
	return false;
}

bool UInventory::Server_DropItem_Validate(APickupBase* Item)
{
	return CheckIfClientHasItem(Item);
}
void UInventory::Server_DropItem_Implementation(APickupBase* Item)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		FVector Location = GetOwner()->GetActorLocation();
		Location.X += FMath::RandRange(-50.f, 100.f);
		Location.Y += FMath::RandRange(-50.f, 100.f);
		FVector EndRay = Location;
		EndRay.Z -= 2000.f;

		FHitResult HitResult;
		FCollisionObjectQueryParams ObjQuery;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(GetOwner());
		GetWorld()->LineTraceSingleByObjectType(
			OUT HitResult,
			Location,
			EndRay,
			ObjQuery,
			CollisionParams
		);
		if (HitResult.ImpactPoint != FVector::ZeroVector)
		{
			Location = HitResult.ImpactPoint;
		}
		Item->SetActorLocation(Location);
		Item->IsInInventory(false);

		RemoveItemFromInventory(Item);
	}
}

void UInventory::DropItem(APickupBase* Item) 
{
	Server_DropItem(Item);
}

bool UInventory::Server_UseItem_Validate(APickupBase* Item)
{
	return CheckIfClientHasItem(Item);
}

void UInventory::Server_UseItem_Implementation(APickupBase* Item)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (ASurvivalCharacter* Player = Cast<ASurvivalCharacter>(GetOwner()))
		{
			Item->UseItem(Player);
			RemoveItemFromInventory(Item);
		}
			
	}
}

bool UInventory::Server_TransferItem_Validate(APickupBase* Item, AStorageContainer* Container)
{
	return CheckIfClientHasItem(Item);
}

void UInventory::Server_TransferItem_Implementation(APickupBase* Item, AStorageContainer* Container)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (Container)
		{
			UE_LOG(LogTemp, Warning, TEXT("passed Container check"));
			if (UInventory* CInventory = Container->GetInventoryComponent())
			{
				CInventory->AddItem(Item);
				UE_LOG(LogTemp, Warning, TEXT("Added Item"));
				RemoveItemFromInventory(Item);
				UE_LOG(LogTemp, Warning, TEXT("Removed Item"));

			}
		}
	}
}

void UInventory::UseItem(APickupBase* Item)
{
	Server_UseItem(Item);
}

void UInventory::TransferItem(APickupBase* Item, AStorageContainer* Container)
{
	Server_TransferItem(Item, Container);
}


TArray<APickupBase*> UInventory::GetInventoryItems() const
{
	return Items;
}

int32 UInventory::GetCurrentInventoryCount() const
{
	return Items.Num() - 1;
}

int32 UInventory::GetInventorySize() const
{
	return InventorySize - 1;
}

