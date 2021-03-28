// Copyright Camden Vaughan 2021. All Rights Reserved.


#include "Survival/Public/Weapons/WeaponBase.h"
#include "Survival/LineTracer.h"
#include "Survival/SurvivalCharacter.h"
#include "Survival/PlayerStatsComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
	RootComponent = SkeletalMeshComp;

	LineTracerComp = CreateDefaultSubobject<ULineTracer>(TEXT("Line Tracer"));
	DefaultWeaponName = FName("");
}



// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();

	if (DefaultWeaponName != "")
	{
		SetupWeapon(DefaultWeaponName);
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
		static const FString PString = FString("AR-15DT");
		WeaponData = WeaponDataTable->FindRow<FWeaponData>(WeaponName, PString, true);
		if (WeaponData)
		{
			SkeletalMeshComp->SetSkeletalMesh(WeaponData->WeaponMesh);
			MagazineAmmoCount = WeaponData->MagazineSize;
			MaxMagazineSize = WeaponData->MagazineSize;
			AmmoType = WeaponData->AmmoType;
			ReloadTime = WeaponData->ReloadTime;
		}
	}
}

FHitResult AWeaponBase::Fire()
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
		FRotator Rotation = SkeletalMeshComp->GetSocketRotation(FName("s_muzzle"));
		FVector EndLocation = StartLocation + Rotation.Vector() * 4000.f;
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
	if (ServerHitResult.GetActor() == nullptr)
	{
		return false;
	}
	float ClientStart = ClientHitResult.TraceStart.Size();
	float ClientEnd = ClientHitResult.TraceEnd.Size();
	float ServerStart = ServerHitResult.TraceStart.Size();
	float ServerEnd = ServerHitResult.TraceEnd.Size();

	if (ClientStart >= ServerStart - 15.0f && ClientStart <= ServerStart + 15.0f && ClientEnd >= ServerEnd - 15.0f && ClientEnd <= ServerEnd + 15.0f)
	{
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Shot Was Not Valid! START DIFFERENCE: %f, END DIFFERENCE: %f"), ClientStart - ServerStart, ClientEnd - ServerEnd);

		return false;
	}
}

FHitResult AWeaponBase::Fire(FHitResult ClientHitResult)
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

			FRotator Rotation = SkeletalMeshComp->GetSocketRotation(FName("s_muzzle"));
			FVector EndLocation = StartLocation + Rotation.Vector() * 4000.f;
			FHitResult ServerHitResult = LineTracerComp->LineTraceSingle(StartLocation, EndLocation, true);

			if (IsValidShot(ClientHitResult, ServerHitResult))
			{
				if (ASurvivalCharacter* Player = Cast<ASurvivalCharacter>(HitActor))
				{
					float TestDamage = 10.f;
					Player->TakeDamage(TestDamage, FDamageEvent(), nullptr, GetOwner());
				}
			}
			else
			{
				
			}

		}
		else
		{
			
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Magazine has %i shots left"), MagazineAmmoCount);
	return FHitResult();
}

/*
bool AWeaponBase::Server_Reload_Validate(int32 Ammo)
{
	return true;
}

void AWeaponBase::Server_Reload_Implementation(int32 Ammo)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		Reload();
	}
} */

void AWeaponBase::Reload(UPlayerStatsComponent* PlayerStatsComp)
{
	if (GetLocalRole() < ROLE_Authority)
	{

	}
	else
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
