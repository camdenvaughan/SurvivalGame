// Copyright Camden Vaughan 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupBase.generated.h"


UENUM(BlueprintType)
enum class EPickupType : uint8
{
	E_Water UMETA(DisplayName = "Water"),
	E_Food UMETA(DisplayName = "Food"),
	E_Bandage UMETA(DisplayName = "Bandage")
};

UCLASS()
class SURVIVAL_API APickupBase : public AActor
{
	GENERATED_BODY()
	
public:	// Constuctor

	// Sets default values for this actor's properties
	APickupBase();
	
protected: // Protected Variables

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere)
	class UTexture2D* Icon;

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

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

public:	// Public Functions

	void UseItem(class ASurvivalCharacter* Player);

	void IsInInventory(bool bIsInInventory);

	UFUNCTION(BlueprintPure)
	class UTexture2D* GetIcon();
};
