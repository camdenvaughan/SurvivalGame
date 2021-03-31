// Copyright Camden Vaughan 2021. All Rights Reserved.


#include "Survival/Public/Weapons/WeaponBase.h"
#include "Survival/LineTracer.h"
#include "Survival/SurvivalCharacter.h"
#include "Survival/PlayerStatsComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	RootComponent = SkeletalMeshComp;

	LineTracerComp = CreateDefaultSubobject<ULineTracer>(TEXT("Line Tracer"));
	DefaultWeaponName = FName("AR-15");
	bReplicates = true;
	bIsDraggedIntoWorld = false;
}



// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	if (bIsDraggedIntoWorld)
	{
		SetupWeapon(DefaultWeaponName);
		UE_LOG(LogTemp, Warning, TEXT("After Begin Play"));
	}
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AWeaponBase, MagazineAmmoCount, COND_OwnerOnly);
}

void AWeaponBase::SetupWeapon(FName WeaponName)
{
	if (WeaponDataTable)
	{
		SetWeaponName(WeaponName);
		static const FString PString = WeaponName.ToString();
		WeaponData = WeaponDataTable->FindRow<FWeaponData>(WeaponName, PString, true);
		if (WeaponData)
		{
			UE_LOG(LogTemp, Warning, TEXT("Inside Setup"));
			SkeletalMeshComp->SetSkeletalMesh(WeaponData->WeaponMesh);
			MagazineAmmoCount = WeaponData->MagazineSize;
			MaxMagazineSize = WeaponData->MagazineSize;
			AmmoType = WeaponData->AmmoType;
			ReloadTime = WeaponData->ReloadTime;
			WeaponDamage = WeaponData->WeaponDamage;
			return;
		}
		return;
	}
}

FHitResult AWeaponBase::Fire(FVector ImpactPoint)
{
	if (GetLocalRole() < ROLE_Authority)
	{
		if (MagazineAmmoCount <= 0)
		{
			// Play empty clip sound
			return FHitResult();
		}
		// Play Sound / Muzzle Flash
		
		FVector StartLocation = SkeletalMeshComp->GetSocketLocation(FName("s_muzzle"));
		FVector EndLocation = StartLocation + UKismetMathLibrary::FindLookAtRotation(StartLocation, ImpactPoint).Vector() * 3500.f;
		FHitResult HitResult = LineTracerComp->LineTraceSingle(StartLocation, EndLocation, true);
		return HitResult;
	}
	else
	{
		return FHitResult();
	}
}

bool AWeaponBase::IsValidShot(FHitResult ClientHitResult, FHitResult ServerHitResult) const
{
	return true;
}

FHitResult AWeaponBase::Fire(FHitResult ClientHitResult, FVector ImpactPoint)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (MagazineAmmoCount <= 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Out Of Ammo"));
			return FHitResult();
		}
		--MagazineAmmoCount;
		FVector StartLocation = SkeletalMeshComp->GetSocketLocation(FName("s_muzzle"));
		
		if (AActor* HitActor = ClientHitResult.GetActor())
		{
			FVector EndLocation = StartLocation + UKismetMathLibrary::FindLookAtRotation(StartLocation, ImpactPoint).Vector() * 3500.f;
			FHitResult ServerHitResult = LineTracerComp->LineTraceSingle(StartLocation, EndLocation, true);
			

			if (IsValidShot(ClientHitResult, ServerHitResult))
			{
				if (ASurvivalCharacter* Player = Cast<ASurvivalCharacter>(HitActor))
				{
					Player->TakeDamage(WeaponDamage, FDamageEvent(), nullptr, GetOwner());
				}
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Magazine has %i shots left"), MagazineAmmoCount);
	return FHitResult();
}

void AWeaponBase::Reload(UPlayerStatsComponent* PlayerStatsComp)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (MagazineAmmoCount != MaxMagazineSize)
		{
			MagazineAmmoCount += PlayerStatsComp->SubtractReloadAmmo(MaxMagazineSize - MagazineAmmoCount, AmmoType);
			UE_LOG(LogTemp, Warning, TEXT("Mag Capacity: %i"), MagazineAmmoCount);
		}
	}
}

EAmmoType AWeaponBase::GetAmmoType() const
{
	return AmmoType;
}

float AWeaponBase::GetReloadTime() const
{
	return ReloadTime;
}

float AWeaponBase::GetWeaponDamage() const
{
	return WeaponDamage;
}

FName AWeaponBase::GetWeaponName() const
{
	return DefaultWeaponName;
}

int32 AWeaponBase::GetMagazineAmmoCount() const
{
	return MagazineAmmoCount;
}

void AWeaponBase::SetWeaponName(FName WeaponName)
{
	DefaultWeaponName = WeaponName;
}
