// Copyright Camden Vaughan 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LineTracer.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SURVIVAL_API ULineTracer : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULineTracer();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	FHitResult LineTraceSingle(FVector Start, FVector End);
	FHitResult LineTraceSingle(FVector Start, FVector End, bool ShowDebugLine);
		
};
