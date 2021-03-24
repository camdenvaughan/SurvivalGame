// Copyright Epic Games, Inc. All Rights Reserved.

#include "SurvivalGameMode.h"
#include "SurvivalCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "SpawnPoint.h"
#include "EngineUtils.h"
#include "TimerManager.h"

ASurvivalGameMode::ASurvivalGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	DefaultSpawn = FVector(-400.f, 50.f, 300.f);
	RespawnTime = 5.f;
}

void ASurvivalGameMode::BeginPlay() 
{
	Super::BeginPlay();

	UClass* SpawnPointClass = ASpawnPoint::StaticClass();
	for(TActorIterator<AActor> Actor (GetWorld(), SpawnPointClass); Actor; ++Actor)
	{
		SpawnPoints.Add(Cast<ASpawnPoint>(*Actor));
	}
}

ASpawnPoint* ASurvivalGameMode::GetSpawnPoint() 
{
	for(int32 i = 0; i < SpawnPoints.Num(); ++i)
	{
		int32 SpawnIndex = FMath::RandRange(0, SpawnPoints.Num() - 1);
		if (SpawnPoints[SpawnIndex])
		{
			return SpawnPoints[SpawnIndex];
		}
		UE_LOG(LogTemp, Error, TEXT("No Valid Spawn Points"));
	}
		return nullptr;
}

void ASurvivalGameMode::Spawn(AController* Controller) 
{
			if (ASpawnPoint* SpawnPoint = GetSpawnPoint())
		{
			FVector Location = SpawnPoint->GetActorLocation();
			FRotator Rotation = SpawnPoint->GetActorRotation();
			if (APawn* Pawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, Location, Rotation))
			{
				Controller->Possess(Pawn);
			}

		}
		else
		{
			FVector Location = DefaultSpawn;
			FRotator Rotation = FRotator::ZeroRotator;
			if (APawn* Pawn = GetWorld()->SpawnActor<APawn>(DefaultPawnClass, Location, Rotation))
			{
				Controller->Possess(Pawn);
			}
		}
}

void ASurvivalGameMode::PostLogin(APlayerController* NewPlayer) 
{
	Super::PostLogin(NewPlayer);
	if (AController* Controller = Cast<AController>(NewPlayer))
	{
		Spawn(Controller);
	}
}
void ASurvivalGameMode::Respawn(AController* Controller) 
{
	UE_LOG(LogTemp, Warning, TEXT("Spawn Points: %d"), SpawnPoints.Num());
	if (!Controller) return;
	if (GetLocalRole() == ROLE_Authority)
	{
		FTimerDelegate RespawnDele;
		FTimerHandle RespawnHandle;
		RespawnDele.BindUFunction(this, FName("Spawn"), Controller);
		GetWorld()->GetTimerManager().SetTimer(RespawnHandle, RespawnDele, RespawnTime, false);
	}
}

