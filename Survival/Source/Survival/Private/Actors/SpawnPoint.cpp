// Fill out your copyright notice in the Description page of Project Settings.


#include "Survival/Public/Actors/SpawnPoint.h"

#include "Components/StaticMeshComponent.h"

// Sets default values
ASpawnPoint::ASpawnPoint()
{
	DebugMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Debug Mesh"));
	RootComponent = DebugMesh;
	DebugMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Called when the game starts or when spawned
void ASpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	SetActorHiddenInGame(true);
	
}


