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
	
public:	
	// Sets default values for this actor's properties
	APickupBase();
protected:
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* MeshComp;

	UPROPERTY(EditAnywhere)
	float IncreaseAmount;

	UPROPERTY(EditAnywhere, Category = "Enums")
	EPickupType PickupType;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	void UseItem(class ASurvivalCharacter* Player);
};
