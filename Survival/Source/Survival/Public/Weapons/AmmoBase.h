// Copyright Camden Vaughan 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmmoBase.generated.h"

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	E_AssaultAmmo UMETA(DisplayName = "Assault Ammo"),
	E_SniperAmmo UMETA(DisplayName = "Sniper"),
	E_ShotgunAmmo UMETA(DisplayName = "Shotgun Ammo")
};

UCLASS()
class SURVIVAL_API AAmmoBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAmmoBase();
protected:

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* AmmoMesh;

	UPROPERTY(EditAnywhere)
	class UStaticMesh* AssaultMesh;

	UPROPERTY(EditAnywhere)
	class UStaticMesh* SniperMesh;

	UPROPERTY(EditAnywhere)
	class UStaticMesh* ShotgunMesh;

	UPROPERTY(EditAnywhere)
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere)
	int32 AmmoAmount;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



public:
	
	void InteractedWith(class UPlayerStatsComponent* PlayerStatsComp);

	void SetupAmmoPickup(EAmmoType SetAmmoType, int32 Value);
	
	EAmmoType GetAmmoType() const;
};
