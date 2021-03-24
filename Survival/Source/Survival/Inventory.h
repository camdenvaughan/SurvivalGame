// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVAL_API UInventory : public UActorComponent
{
	GENERATED_BODY()

public:	// Constructor

	// Sets default values for this component's properties
	UInventory();

protected: // Protected Variables

	UPROPERTY(Replicated)
	TArray<class APickupBase*> Items;

protected: // Protected Functions

	// Called when the game starts
	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiHideItem(class APickupBase* Item);
	bool MultiHideItem_Validate(class APickupBase* Item);
	void MultiHideItem_Implementation(class APickupBase* Item);

public: // Public Functions
	bool AddItem(class APickupBase* Item);
	void DropItem(class APickupBase* Item);
	void DropAllInventory();

	UFUNCTION(BlueprintPure)
	TArray<class APickupBase*> GetInventoryItems();

	UFUNCTION(BlueprintPure)
	int32 GetCurrentInventoryCount();
};
