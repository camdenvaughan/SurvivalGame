// Copyright Epic Games, Inc. All Rights Reserved.

#include "SurvivalGameMode.h"
#include "SurvivalCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASurvivalGameMode::ASurvivalGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ASurvivalGameMode::Respawn(AController* Controller) 
{
	if (!Controller) return;
	if (GetLocalRole() == ROLE_Authority)
	{
		FVector Location = FVector(-400.f, 50.f, 200.f);
		if (APawn* Pawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, Location, FRotator::ZeroRotator))
		{
			Controller->Possess(Pawn);
		}
		
	}
}
