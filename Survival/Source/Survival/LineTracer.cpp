// Copyright Camden Vaughan 2021. All Rights Reserved.


#include "LineTracer.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

#define OUT
// Sets default values for this component's properties
ULineTracer::ULineTracer()
{

}


// Called when the game starts
void ULineTracer::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

FHitResult ULineTracer::LineTraceSingle(FVector Start, FVector End) 
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

FHitResult ULineTracer::LineTraceSingle(FVector Start, FVector End, bool ShowDebugLine) 
{

	FHitResult HitResult = LineTraceSingle(Start, End);
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



