// Copyright Camden Vaughan 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	class USkeletalMesh* WeaponMesh;

	UPROPERTY(EditAnywhere)
	FString WeaponName;
	
	UPROPERTY(EditAnywhere)
	class UAnimationAsset* FireAnimation;
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

	UPROPERTY()
	class ULineTracer* LineTracerComp;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	bool IsValidShot(FHitResult ClientHitResult, FHitResult ServerHitResult);
public:

	void SetupWeapon(FName WeaponName);
	FHitResult Fire();
	FHitResult Fire(FHitResult ClientHitResult);

};
