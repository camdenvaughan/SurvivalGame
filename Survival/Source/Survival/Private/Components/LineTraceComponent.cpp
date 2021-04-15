// Copyright Camden Vaughan 2021. All Rights Reserved.


#include "Survival/Public/Components/LineTraceComponent.h"

#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

#define OUT
// Sets default values for this component's properties
ULineTraceComponent::ULineTraceComponent()
{
}


// Called when the game starts
void ULineTraceComponent::BeginPlay()
{
	Super::BeginPlay();
}

FHitResult ULineTraceComponent::LineTraceSingle(FVector Start, FVector End) const
{
	FHitResult HitResult;
	FCollisionObjectQueryParams CollisionObjectParams;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(GetOwner());
	if (GetWorld()->LineTraceSingleByObjectType(
		OUT HitResult,
		Start,
		End,
		CollisionObjectParams,
		CollisionQueryParams
	))
	{
		return HitResult;
	}
	
	return HitResult;
}

FHitResult ULineTraceComponent::LineTraceSingle(FVector Start, FVector End, bool ShowDebugLine) const
{

	FHitResult const HitResult = LineTraceSingle(Start, End);
	if (ShowDebugLine)
	{
		DrawDebugLine(
		GetWorld(),
		Start,
		End,
		FColor::Red,
		false,
		3.0f,
		0,
		5.f
		);
	}
	return HitResult;
}



