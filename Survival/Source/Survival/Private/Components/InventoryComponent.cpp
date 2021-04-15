// Fill out your copyright notice in the Description page of Project Settings.


#include "Survival/Public/Components/InventoryComponent.h"
#include "Survival/Public/Actors/PickupBase.h"
#include "Survival/Public/Character/SurvivalCharacter.h"
#include "Survival/Public/Actors/StorageContainer.h"

#include "Net/UnrealNetwork.h"

#define OUT
// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	InventorySize = 16;
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


	DOREPLIFETIME(UInventoryComponent, Items);
}


bool UInventoryComponent::AddItem(APickupBase* Item) 
{
	if (Item)
	{
		Items.Add(Item);
		Item->IsInInventory(true);
		return true;
	}
return false;
}


void UInventoryComponent::DropAllInventory() 
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

bool UInventoryComponent::CheckIfClientHasItem(APickupBase* Item)
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
bool UInventoryComponent::RemoveItemFromInventory(APickupBase* Item)
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

bool UInventoryComponent::Server_DropItem_Validate(APickupBase* Item)
{
	return CheckIfClientHasItem(Item);
}
void UInventoryComponent::Server_DropItem_Implementation(APickupBase* Item)
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

void UInventoryComponent::DropItem(APickupBase* Item) 
{
	Server_DropItem(Item);
}

bool UInventoryComponent::Server_UseItem_Validate(APickupBase* Item)
{
	return CheckIfClientHasItem(Item);
}

void UInventoryComponent::Server_UseItem_Implementation(APickupBase* Item)
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

bool UInventoryComponent::Server_TransferItem_Validate(APickupBase* Item, AActor* Container)
{
	return CheckIfClientHasItem(Item);
}

void UInventoryComponent::Server_TransferItem_Implementation(APickupBase* Item, AActor* ContainerActor)
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (ContainerActor)
		{
			if (AStorageContainer* Container = Cast<AStorageContainer>(ContainerActor))
			{
				if (UInventoryComponent* CInventory = Container->GetInventoryComponent())
				{
					CInventory->AddItem(Item);
					RemoveItemFromInventory(Item);
				}
			}
			else if (ASurvivalCharacter* Character = Cast<ASurvivalCharacter>(ContainerActor))
			{
				if (UInventoryComponent* CInventory = Character->GetInventoryComponent())
				{
					CInventory->AddItem(Item);
					RemoveItemFromInventory(Item);
				}
			}

		}
	}
}

void UInventoryComponent::UseItem(APickupBase* Item)
{
	Server_UseItem(Item);
}

void UInventoryComponent::TransferItem(APickupBase* Item, AActor* ContainerActor)
{
	Server_TransferItem(Item, ContainerActor);
}

bool UInventoryComponent::Server_ReceiveItem_Validate(APickupBase* Item)
{
	if (Item) return true;
	else return false;
}

void UInventoryComponent::Server_ReceiveItem_Implementation(APickupBase* Item)
{
	if (ASurvivalCharacter* SurvivalCharacter = Cast<ASurvivalCharacter>(GetOwner()))
	{
		if (AStorageContainer* Container = SurvivalCharacter->GetOpenedContainer())
		{
			Container->GetInventoryComponent()->TransferItem(Item,SurvivalCharacter);
		}
	}
}


TArray<APickupBase*> UInventoryComponent::GetInventoryItems() const
{
	return Items;
}

int32 UInventoryComponent::GetCurrentInventoryCount() const
{
	return Items.Num() - 1;
}

int32 UInventoryComponent::GetInventorySize() const
{
	return InventorySize - 1;
}

