// Copyright Camden Vaughan 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "PlayerStatsComponent.generated.h"

enum class EAmmoType : uint8;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVAL_API UPlayerStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	// Constructor

	// Sets default values for this component's properties
	UPlayerStatsComponent();

protected: // Protected Variables

	UPROPERTY(Replicated)
	float Health;
	UPROPERTY(EditAnywhere, Category = "Survival|Player Stats", meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	UPROPERTY(Replicated)
	float Hunger;
	UPROPERTY(EditAnywhere, Category = "Survival|Player Stats", meta = (AllowPrivateAccess = "true"))
	float MaxHunger;
	UPROPERTY(EditAnywhere, Category = "Survival|Player Stats", meta = (AllowPrivateAccess = "true"))
	float HungerDecrementValue;
	UPROPERTY(EditAnywhere, Category = "Survival|Player Stats", meta = (AllowPrivateAccess = "true"))
	float HungerDamageValue;
	UPROPERTY(EditAnywhere, Category = "Survival|Player Stats", meta = (AllowPrivateAccess = "true"))
	float HungerHealingThreshold;
	UPROPERTY(EditAnywhere, Category = "Survival|Player Stats", meta = (AllowPrivateAccess = "true"))
	float FullHungerHealingAmount;

	UPROPERTY(Replicated)
	float Thirst;
	UPROPERTY(EditAnywhere, Category = "Survival|Player Stats", meta = (AllowPrivateAccess = "true"))
	float MaxThirst;
	UPROPERTY(EditAnywhere, Category = "Survival|Player Stats", meta = (AllowPrivateAccess = "true"))
	float ThirstDecrementValue;
	UPROPERTY(EditAnywhere, Category = "Survival|Player Stats", meta = (AllowPrivateAccess = "true"))
	float ThirstDamageValue;
	UPROPERTY(EditAnywhere, Category = "Survival|Player Stats", meta = (AllowPrivateAccess = "true"))
	float ThirstHealingThreshold;
	UPROPERTY(EditAnywhere, Category = "Survival|Player Stats", meta = (AllowPrivateAccess = "true"))
	float FullThirstHealingAmount;

	UPROPERTY(Replicated)
	float Stamina;
	UPROPERTY(EditAnywhere, Category = "Survival|Player Stats", meta = (AllowPrivateAccess = "true"))
	float MaxStamina;

	UPROPERTY(Replicated)
	int AssaultAmmo;

	UPROPERTY(Replicated)
	int SniperAmmo;

	UPROPERTY(Replicated)
	int ShotgunAmmo;

	FTimerHandle HungerAndThirstTimer;

	FTimerHandle StaminaRegeneration;

protected: // Protected Functions

	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void HandleHungerAndThirst();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_LowerHealth(float Value);
	bool Server_LowerHealth_Validate(float Value);
	void Server_LowerHealth_Implementation(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_LowerHunger(float Value);
	bool Server_LowerHunger_Validate(float Value);
	void Server_LowerHunger_Implementation(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_LowerThirst(float Value);
	bool Server_LowerThirst_Validate(float Value);
	void Server_LowerThirst_Implementation(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_LowerStamina(float Value);
	bool Server_LowerStamina_Validate(float Value);
	void Server_LowerStamina_Implementation(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_ControlSprintingTimer(bool bIsSprinting);
	bool Server_ControlSprintingTimer_Validate(bool bIsSprinting);
	void Server_ControlSprintingTimer_Implementation(bool bIsSprinting);

	void RegenerateStamina();

public:	// Public Functions
	void AddHealth(float Value);
	void AddHunger(float Value);
	void AddThirst(float Value);
	void AddAmmo(int32 Value, EAmmoType AmmoType);
	void LowerHealth(float Value);
	void LowerHunger(float Value);
	void LowerThirst(float Value);
	void LowerStamina(float Value);
	int32 SubtractReloadAmmo(int32 MagazineSize, EAmmoType AmmoType);

	float GetHealth() const;
	float GetHunger() const;
	float GetThirst() const;
	float GetStamina() const;
	int32 GetAmmo(EAmmoType AmmoType) const;
	void ControlSprintingTimer(bool bIsSprinting);

private:
	void SetAllStats();
};
