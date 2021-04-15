// Copyright Camden Vaughan 2021. All Rights Reserved.


#include "Actors/ButtonBase.h"
#include "Kismet/GameplayStatics.h"


// Sets default values
AButtonBase::AButtonBase()
{
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	RootComponent = StaticMeshComp;
}

// Called when the game starts or when spawned
void AButtonBase::BeginPlay()
{
	Super::BeginPlay();
	
}

bool AButtonBase::Server_ResetLevel_Validate()
{
	return true;
}

void AButtonBase::Server_ResetLevel_Implementation()
{
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()), false);
}

void AButtonBase::Use()
{
	Server_ResetLevel();
}


