// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnPoint.generated.h"

UCLASS()
class SURVIVAL_API ASpawnPoint : public AActor
{
	GENERATED_BODY()
	
public:	// Constructor

	// Sets default values for this actor's properties
	ASpawnPoint();

protected: // Protected Variables

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* DebugMesh;

protected: // Protected Functions

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
};
