// Copyright Camden Vaughan 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Survival/Public/Enums/SurvivalEnums.h"
#include "PickupBase.generated.h"

UCLASS()
class SURVIVAL_API APickupBase : public AActor
{
	GENERATED_BODY()
	
public:	// Constructor

	// Sets default values for this actor's properties
	APickupBase();
	
protected: // Protected Variables

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere)
	class UTexture* Icon;

	UPROPERTY(EditAnywhere)
	float IncreaseAmount;

	UPROPERTY(EditAnywhere, Category = "Survival|Enums")
	EPickupType PickupType;

	UPROPERTY(ReplicatedUsing = OnRep_PickedUp)
	bool bIsObjectPickedUp;

	UFUNCTION()
	void OnRep_PickedUp();

protected: // Protected Functions

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	// Public Functions

	void UseItem(class ASurvivalCharacter* Player);

	void IsInInventory(bool bIsInInventory);

	UFUNCTION(BlueprintPure)
	class UTexture* GetIcon() const;
};
