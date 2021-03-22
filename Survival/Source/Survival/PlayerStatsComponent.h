// Copyright Camden Vaughan 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerStatsComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVAL_API UPlayerStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerStatsComponent();

protected:

	UPROPERTY(Replicated)
	float Hunger;

	UPROPERTY(EditAnywhere, Category = "S|Player Stats")
	float HungerDecrementValue;

	UPROPERTY(Replicated)
	float Thirst;
	UPROPERTY(EditAnywhere, Category = "S|Player Stats")
	float ThirstDecrementValue;

	UPROPERTY(Replicated)
	float Stamina;

	UPROPERTY(Replicated)
	float Health;


	FTimerHandle HungerAndThirstTimer;

	FTimerHandle StaminaRegeneration;

protected:
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

public:	
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
};
