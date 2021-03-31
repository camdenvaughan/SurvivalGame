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

	UPROPERTY(EditAnywhere)
	float WeaponDamage;
};

UCLASS()
class SURVIVAL_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();
protected: // Protected Components / Variables

	UPROPERTY(EditAnywhere)
	class USkeletalMeshComponent* SkeletalMeshComp;

	UPROPERTY()
	class ULineTracer* LineTracerComp;
	
	UPROPERTY(EditAnywhere)
	class UDataTable* WeaponDataTable;
	
	FWeaponData* WeaponData;

	UPROPERTY(EditAnywhere)
	FName DefaultWeaponName;

	UPROPERTY(Replicated)
	int32 MagazineAmmoCount;

	EAmmoType AmmoType;

	int32 MaxMagazineSize;

	float ReloadTime;

	float WeaponDamage;
	
	UPROPERTY(EditAnywhere)
	bool bIsDraggedIntoWorld;
	
protected: // Protected Functions
	
	virtual void BeginPlay() override;

	bool IsValidShot(FHitResult ClientHitResult, FHitResult ServerHitResult) const;
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
public: // Public Functions

	void SetupWeapon(FName WeaponName);
	FHitResult Fire(FVector ImpactPoint);
	FHitResult Fire(FHitResult ClientHitResult, FVector ImpactPoint);
	void Reload(class UPlayerStatsComponent* PlayerStatsComp);

	EAmmoType GetAmmoType() const;
	float GetReloadTime() const;
	float GetWeaponDamage() const;
	FName GetWeaponName() const;
	int32 GetMagazineAmmoCount() const;

	void SetWeaponName(FName WeaponName);
};
