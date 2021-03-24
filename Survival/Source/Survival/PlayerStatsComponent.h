// Copyright Camden Vaughan 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStatsComponent.generated.h"


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

	FTimerHandle HungerAndThirstTimer;

	FTimerHandle StaminaRegeneration;

protected: // Protected Functions

	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void HandleHungerAndThirst();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLowerHealth(float Value);
	bool ServerLowerHealth_Validate(float Value);
	void ServerLowerHealth_Implementation(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLowerHunger(float Value);
	bool ServerLowerHunger_Validate(float Value);
	void ServerLowerHunger_Implementation(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLowerThirst(float Value);
	bool ServerLowerThirst_Validate(float Value);
	void ServerLowerThirst_Implementation(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerLowerStamina(float Value);
	bool ServerLowerStamina_Validate(float Value);
	void ServerLowerStamina_Implementation(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerControlSprintingTimer(bool bIsSprinting);
	bool ServerControlSprintingTimer_Validate(bool bIsSprinting);
	void ServerControlSprintingTimer_Implementation(bool bIsSprinting);

	void RegenerateStamina();

public:	// Public Functions
	void AddHealth(float Value);
	void AddHunger(float Value);
	void AddThirst(float Value);
	void LowerHealth(float Value);
	void LowerHunger(float Value);
	void LowerThirst(float Value);
	void LowerStamina(float Value);

	float GetHealth();
	float GetHunger();
	float GetThirst();
	float GetStamina();
	void ControlSprintingTimer(bool bIsSprinting);

private:
	void SetAllStats();
};
