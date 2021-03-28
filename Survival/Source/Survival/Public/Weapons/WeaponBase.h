// Copyright Camden Vaughan 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"


#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

enum class EAmmoType : uint8;
USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	class USkeletalMesh* WeaponMesh;

	UPROPERTY(EditAnywhere)
	FString WeaponName;
	
	UPROPERTY(EditAnywhere)
	EAmmoType AmmoType;
	
	UPROPERTY(EditAnywhere)
	int32 MagazineSize;

	UPROPERTY(EditAnywhere)
	float ReloadTime;
};

UCLASS()
class SURVIVAL_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();
protected:

	UPROPERTY(EditAnywhere)
	class UDataTable* WeaponDataTable;
	
	FWeaponData* WeaponData;

	UPROPERTY(EditAnywhere)
	FName DefaultWeaponName;

	UPROPERTY(EditAnywhere)
	class USkeletalMeshComponent* SkeletalMeshComp;

	EAmmoType AmmoType;

	UPROPERTY(Replicated)
	int32 MagazineAmmoCount;

	int32 MaxMagazineSize;

	float ReloadTime;

	UPROPERTY()
	class ULineTracer* LineTracerComp;
/*
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Reload(int32 Ammo);
	bool Server_Reload_Validate(int32 Ammo);
	void Server_Reload_Implementation(int32 Ammo); */
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool IsValidShot(FHitResult ClientHitResult, FHitResult ServerHitResult) const;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public:

	void SetupWeapon(FName WeaponName);
	FHitResult Fire();
	FHitResult Fire(FHitResult ClientHitResult);
	void Reload(class UPlayerStatsComponent* PlayerStatsComp);

	EAmmoType GetAmmoType() const;
	float GetReloadTime() const;

};
