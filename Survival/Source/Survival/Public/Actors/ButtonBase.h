// Copyright Camden Vaughan 2021. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ButtonBase.generated.h"

UCLASS()
class SURVIVAL_API AButtonBase : public AActor
{
	GENERATED_BODY()

public: // Public Constructor
	// Sets default values for this actor's properties
	AButtonBase();
protected: // Protected Variables

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* StaticMeshComp;
	
protected: // Protected Functions
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(Server, WithValidation, Reliable)
	void Server_ResetLevel();
	bool Server_ResetLevel_Validate();
	void Server_ResetLevel_Implementation();


public:
	void Use();
};
