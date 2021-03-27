// Copyright Camden Vaughan 2021. All Rights Reserved.


#include "Survival/Public/Weapons/WeaponBase.h"
#include "Survival/LineTracer.h"
#include "Survival/SurvivalCharacter.h"

#include "Components/SkeletalMeshComponent.h"

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

void AWeaponBase::SetupWeapon(FName WeaponName)
{
	if (WeaponDataTable)
	{
		static const FString PString = FString("AR-15DT");
		WeaponData = WeaponDataTable->FindRow<FWeaponData>(WeaponName, PString, true);
		if (WeaponData)
		{
			SkeletalMeshComp->SetSkeletalMesh(WeaponData->WeaponMesh);
		}
	}
}

FHitResult AWeaponBase::Fire()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		if (WeaponData)
		{
			SkeletalMeshComp->PlayAnimation(WeaponData->FireAnimation, false);
		}
		FVector StartLocation = SkeletalMeshComp->GetSocketLocation(FName("s_muzzle"));
		FRotator Rotation = SkeletalMeshComp->GetSocketRotation(FName("s_muzzle"));
		FVector EndLocation = StartLocation + Rotation.Vector() * 1500.f;
		FHitResult HitResult = LineTracerComp->LineTraceSingle(StartLocation, EndLocation, true);
		return HitResult;
	}
	else
	{
		return FHitResult();
	}

}

FHitResult AWeaponBase::Fire(FHitResult ServerHitResult)
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (AActor* HitActor = ServerHitResult.GetActor())
		{
			if (ASurvivalCharacter* Player = Cast<ASurvivalCharacter>(HitActor))
			{
				float TestDamage = 10.f;
				Player->TakeDamage(TestDamage, FDamageEvent(), nullptr, GetOwner());
			}
		}
	}
	return FHitResult();
}
