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

	UPROPERTY(EditAnywhere)
	int32 InventorySize;

protected: // Protected Functions

	// Called when the game starts
	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;


	UFUNCTION(Server, Reliable, WithValidation)
	void Server_DropItem(class APickupBase* Item);
	bool Server_DropItem_Validate(class APickupBase* Item);
	void Server_DropItem_Implementation(class APickupBase* Item);

	UFUNCTION(Server, Reliable, WithValidation)
    void Server_UseItem(class APickupBase* Item);
	bool Server_UseItem_Validate(class APickupBase* Item);
	void Server_UseItem_Implementation(class APickupBase* Item);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_TransferItem(class APickupBase* Item, AActor* ContainerActor);
	bool Server_TransferItem_Validate(class APickupBase* Item, AActor* ContainerActor);
	void Server_TransferItem_Implementation(class APickupBase* Item, AActor* ContainerActor);
	
	bool CheckIfClientHasItem(class APickupBase* Item);
	bool RemoveItemFromInventory(class APickupBase* Item);

public: // Public Functions
	bool AddItem(class APickupBase* Item);

	UFUNCTION(BlueprintCallable)
	void DropItem(class APickupBase* Item);
	void DropAllInventory();

	UFUNCTION(BlueprintCallable)
	void UseItem(class APickupBase* Item);

	UFUNCTION(BlueprintCallable)
	void TransferItem(class APickupBase* Item, AActor* ContainerActor);

	UFUNCTION(BlueprintCallable, Server, Reliable, WithValidation)
	void Server_ReceiveItem(class APickupBase* Item);
	bool Server_ReceiveItem_Validate(class APickupBase* Item);
	void Server_ReceiveItem_Implementation(class APickupBase* Item);

	UFUNCTION(BlueprintPure)
	TArray<class APickupBase*> GetInventoryItems() const;

	UFUNCTION(BlueprintPure)
	int32 GetCurrentInventoryCount() const;
	
	UFUNCTION(BlueprintPure)
	int32 GetInventorySize() const;
};
