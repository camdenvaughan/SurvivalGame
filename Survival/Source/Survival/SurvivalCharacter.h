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

	UPROPERTY(VisibleAnywhere)
	class ULineTracer* LineTraceComp;

	UPROPERTY(EditAnywhere)
	class UInventory* InventoryComp;

	TSubclassOf<class UUserWidget> InventoryWidgetClass;

	UPROPERTY(VisibleAnywhere)
	UUserWidget* InventoryWidget;

	bool bIsSprinting;

	FTimerHandle SprintingHandle;
	FTimerHandle DestroyHandle;

	UPROPERTY(ReplicatedUsing = OnRep_OpenCloseInventory)
	class AStorageContainer* OpenedContainer;
	
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

	void HandleSprinting();

	void TryToJump();

	void Interact();

	void OpenCloseInventory();

	UFUNCTION()
	void OnRep_OpenCloseInventory();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Interact();
	bool Server_Interact_Validate();
	void Server_Interact_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_InventoryClose();
	bool Server_InventoryClose_Validate();
	void Server_InventoryClose_Implementation();

	void Attack();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Attack();
	bool Server_Attack_Validate();
	void Server_Attack_Implementation();

	void Die();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
	void Multi_Die();
	bool Multi_Die_Validate();
	void Multi_Die_Implementation();

	void CallDestroy();


	UFUNCTION(BlueprintPure)
    FString ReturnPlayerStats() const;
    UFUNCTION(BlueprintPure)
    float ReturnHealth() const;
    UFUNCTION(BlueprintPure)
    float ReturnStamina() const;
    UFUNCTION(BlueprintPure)
    float ReturnHunger() const;
    UFUNCTION(BlueprintPure)
    float ReturnThirst() const;

public: // Public Functions

	UPROPERTY(EditAnywhere)
	class UPlayerStatsComponent* PlayerStatsComp;

	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure)
    class UInventory* GetInventoryComponent() const;
	UFUNCTION(BlueprintPure)
    class AStorageContainer* GetOpenedContainer() const;

public: // Default Public
	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
};

