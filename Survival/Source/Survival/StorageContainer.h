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

	class UInventory* InventoryComp;

protected: // Protected Functions
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	UFUNCTION(BlueprintCallable)
	class UInventory* GetInventoryComponent();

};
