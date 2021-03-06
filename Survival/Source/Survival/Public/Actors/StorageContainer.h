// Copyright Camden Vaughan 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StorageContainer.generated.h"

UCLASS()
class SURVIVAL_API AStorageContainer : public AActor
{
	GENERATED_BODY()
	
public: // Constructor
	AStorageContainer();

protected: // Protected Variables

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(EditAnywhere)
	class UInventoryComponent* InventoryComp;
	
	UPROPERTY(Replicated)
	bool bIsOpen;
	


protected: // Protected Functions
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable, WithValidation)
    void Server_OpenedChest(bool bOpened);
	bool Server_OpenedChest_Validate(bool bOpened);
	void Server_OpenedChest_Implementation(bool bOpened);

public:
	UFUNCTION(BlueprintPure)
	class UInventoryComponent* GetInventoryComponent() const;
	bool IsChestOpen() const;
	void OpenChest(bool bOpened);

};
