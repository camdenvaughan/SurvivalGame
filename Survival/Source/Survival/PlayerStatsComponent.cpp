// Copyright Camden Vaughan 2021. All Rights Reserved.


#include "PlayerStatsComponent.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"

// Sets default values for this component's properties
UPlayerStatsComponent::UPlayerStatsComponent()
{
	Health = 50.f;
	Hunger = 100.f;
	HungerDecrementValue = 0.3f;
	Thirst = 100.f;
	ThirstDecrementValue = 0.5f;

	Stamina = 100.f;
}


// Called when the game starts
void UPlayerStatsComponent::BeginPlay()
{
	Super::BeginPlay();
	SetIsReplicated(true);

	if (GetOwnerRole() == ROLE_Authority)
	{
		// Decrement Hunger and Thirst
		GetWorld()->GetTimerManager().SetTimer(HungerAndThirstTimer, this, &UPlayerStatsComponent::HandleHungerAndThirst, 3.f, true);
		// Regenerate Stamina
		GetWorld()->GetTimerManager().SetTimer(StaminaRegeneration, this, &UPlayerStatsComponent::RegenerateStamina, 1.f, true);
	}
}

void UPlayerStatsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UPlayerStatsComponent, Health);
	DOREPLIFETIME(UPlayerStatsComponent, Hunger);
	DOREPLIFETIME(UPlayerStatsComponent, Thirst);
	DOREPLIFETIME(UPlayerStatsComponent, Stamina);

}

void UPlayerStatsComponent::HandleHungerAndThirst() 
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		LowerHunger(HungerDecrementValue);
		LowerThirst(ThirstDecrementValue);
	}
}

// Server Health Functions
//
//
bool UPlayerStatsComponent::ServerLowerHealth_Validate(float Value) 
{
	return true;
}

void UPlayerStatsComponent::ServerLowerHealth_Implementation(float Value) 
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		LowerHealth(Value);
	}
}

// Server Hunger Functions
//
//
bool UPlayerStatsComponent::ServerLowerHunger_Validate(float Value) 
{
	return true;
}

void UPlayerStatsComponent::ServerLowerHunger_Implementation(float Value) 
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		LowerHunger(Value);
	}
}

// Server Thirst Functions
//
//
bool UPlayerStatsComponent::ServerLowerThirst_Validate(float Value) 
{
	return true;
}

void UPlayerStatsComponent::ServerLowerThirst_Implementation(float Value) 
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		LowerThirst(Value);
	}
}

// Server Stamina Functions
//
//
bool UPlayerStatsComponent::ServerLowerStamina_Validate(float Value) 
{
	return true;
}

void UPlayerStatsComponent::ServerLowerStamina_Implementation(float Value) 
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		LowerStamina(Value);
	}
}

// Server Timer Controller
//
//
bool UPlayerStatsComponent::ServerControlSprintingTimer_Validate(bool bIsSprinting) 
{
	return true;
}

void UPlayerStatsComponent::ServerControlSprintingTimer_Implementation(bool bIsSprinting) 
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		ControlSprintingTimer(bIsSprinting);
	}
}

void UPlayerStatsComponent::ControlSprintingTimer(bool bIsSprinting) 
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerControlSprintingTimer(bIsSprinting);
	}
	else
	{
		if (bIsSprinting)
		{
			GetWorld()->GetTimerManager().PauseTimer(StaminaRegeneration);
		}
		else
		{
			GetWorld()->GetTimerManager().UnPauseTimer(StaminaRegeneration);
		}
	}
}

// Adding Funcitons
//
//
void UPlayerStatsComponent::AddHealth(float Value) 
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (Health + Value > 100.f)
			Health = 100.f;
		else
			Health += Value;
	}
}

void UPlayerStatsComponent::AddHunger(float Value) 
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (Hunger + Value > 100.f)
			Hunger = 100.f;
		else
			Hunger += Value;
	}
}

void UPlayerStatsComponent::AddThirst(float Value) 
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (Thirst + Value > 100.f)
			Thirst = 100.f;
		else
			Thirst += Value;
	}
}

void UPlayerStatsComponent::RegenerateStamina() 
{
	if (GetOwnerRole() == ROLE_Authority)
	{
		if (Stamina >= 100)
		{
			Stamina = 100.f;
		}
		else
		{
			++Stamina;
		}
	}
}

// Lowering Functions
//
//
void UPlayerStatsComponent::LowerHealth(float Value) 
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerLowerHealth(Value);
	}
	else
	{
		if (Health - Value < 0.f)
		{
			Health = 0.f;
		}
		else
		{
			Health -= Value;
		}
	}
}

void UPlayerStatsComponent::LowerHunger(float Value) 
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerLowerHunger(Value);
	}
	else
	{
		Hunger -= Value;
	}
}

void UPlayerStatsComponent::LowerThirst(float Value) 
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerLowerThirst(Value);
	}
	else
	{
		Thirst -= Value;
	}
}

void UPlayerStatsComponent::LowerStamina(float Value) 
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerLowerStamina(Value);
	}
	else
	{
		if (Stamina - Value < 0.0f)
			Stamina = 0.0f;
		else
			Stamina -= Value;
	}
}

// Getter Functions
//
//
float UPlayerStatsComponent::GetHealth() 
{
	return Health;
}

float UPlayerStatsComponent::GetHunger() 
{
	return Hunger;
}

float UPlayerStatsComponent::GetThirst() 
{
	return Thirst;
}

float UPlayerStatsComponent::GetStamina() 
{
	return Stamina;
}



