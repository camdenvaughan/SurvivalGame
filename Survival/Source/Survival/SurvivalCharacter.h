// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SurvivalCharacter.generated.h"

UCLASS(config=Game)
class ASurvivalCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public: // Default Public
	ASurvivalCharacter();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;
	
protected: // Protected Variables

	class ULineTracer* LineTraceComp;
	class UInventory* InventoryComp;

	bool bIsSprinting;

	FTimerHandle SprintingHandle;
	FTimerHandle DestroyHandle;

	UFUNCTION(BlueprintPure)
	FString ReturnPlayerStats();

protected: // Default Protected

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	void StartSprinting();

	void StopSprinting();

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

protected: // Protected Functions
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure)
	class UInventory* GetInventoryComp();

	void HandleSprinting();

	void TryToJump();

	void Interact();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerInteract();
	bool ServerInteract_Validate();
	void ServerInteract_Implementation();

	void Attack();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerAttack();
	bool ServerAttack_Validate();
	void ServerAttack_Implementation();

	void Die();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void MultiDie();
	bool MultiDie_Validate();
	void MultiDie_Implementation();

	void CallDestroy();
public: // Public Functions

	class UPlayerStatsComponent* PlayerStatsComp;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

public: // Default Public
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
};

