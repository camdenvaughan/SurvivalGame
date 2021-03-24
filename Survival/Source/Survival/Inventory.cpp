// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory.h"
#include "PickupBase.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

#define OUT
// Sets default values for this component's properties
UInventory::UInventory()
{
	SetIsReplicated(true);
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


	DOREPLIFETIME_CONDITION(UInventory, Items, COND_OwnerOnly);
}


bool UInventory::AddItem(APickupBase* Item) 
{
	Items.Add(Item);
	Item->IsInInventory(true);
	for (APickupBase* Pickup: Items)
	{
		UE_LOG(LogTemp, Warning, TEXT("Item: %s:"), *Pickup->GetName());
	}
	return true;
}

bool UInventory::MultiHideItem_Validate(APickupBase* Item) 
{
	return true;
}

void UInventory::MultiHideItem_Implementation(APickupBase* Item) 
{
	Item->SetActorEnableCollision(false);
	Item->SetActorHiddenInGame(true);
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

void UInventory::DropItem(APickupBase* Item) 
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
	}
}

TArray<APickupBase*> UInventory::GetInventoryItems() 
{
	return Items;
}

int32 UInventory::GetCurrentInventoryCount() 
{
	return Items.Num()-1;
}

