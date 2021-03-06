// Copyright Camden Vaughan 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LineTraceComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVAL_API ULineTraceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	// Constructor

	// Sets default values for this component's properties
	ULineTraceComponent();

protected: // Protected Functions
	// Called when the game starts
	virtual void BeginPlay() override;

public:	// Public Functions

	FHitResult LineTraceSingle(FVector Start, FVector End) const;
	FHitResult LineTraceSingle(FVector Start, FVector End, bool ShowDebugLine) const;	
};
