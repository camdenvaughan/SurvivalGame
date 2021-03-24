// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SurvivalGameMode.generated.h"

UCLASS(minimalapi)
class ASurvivalGameMode : public AGameModeBase
{
	GENERATED_BODY()

public: // Constuctor

	ASurvivalGameMode();
	
protected: // Protected Variables

	TArray<class ASpawnPoint*> SpawnPoints;

	FVector DefaultSpawn;

	UPROPERTY(EditAnywhere, Category = "Survival|Respawn")
	float RespawnTime;

protected: // Protected Functions

	class ASpawnPoint* GetSpawnPoint();

	UFUNCTION()
	void Spawn(AController* Controller);

	virtual void PostLogin(APlayerController* NewPlayer) override;

public: // Public Functions

	void Respawn(AController* Controller);

	virtual void BeginPlay() override;
};



