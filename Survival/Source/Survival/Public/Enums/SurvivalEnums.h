// Copyright Camden Vaughan 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	E_AssaultAmmo UMETA(DisplayName = "Assault Ammo"),
    E_SniperAmmo UMETA(DisplayName = "Sniper"),
    E_ShotgunAmmo UMETA(DisplayName = "Shotgun Ammo")
};

UENUM(BlueprintType)
enum class EPickupType : uint8
{
	E_Water UMETA(DisplayName = "Water"),
    E_Food UMETA(DisplayName = "Food"),
    E_Bandage UMETA(DisplayName = "Bandage")
};