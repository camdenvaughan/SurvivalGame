// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Public/Weapons/WeaponBase.h"

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
	FTimerHandle ReloadHandle;
	FTimerHandle DestroyHandle;

	UPROPERTY(ReplicatedUsing = OnRep_OpenCloseInventory)
	class AStorageContainer* OpenedContainer;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeaponBase> WeaponClass;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponInteracted)
	class AWeaponBase* ActiveWeapon;
	UPROPERTY(ReplicatedUsing = OnRep_WeaponDropped)
	class AWeaponBase* SpawnedWeapon;

	UPROPERTY(Replicated)
	float PlayerPitch;
	
	UPROPERTY(Replicated)
	FName WeaponToSpawnName;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AAmmoBase> AmmoClass;

	UPROPERTY(ReplicatedUsing = OnRep_SetAiming)
	bool bIsAiming;

	UPROPERTY(Replicated)
	bool bIsReloading;

	UPROPERTY(Replicated)
	bool bWeaponIsOnBack;

	void SetIsAiming();
	void SetIsNotAiming();
	
protected: // Default Protected

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	void StartSprinting();

	void StopSprinting();

	void StartCrouch();

	void StopCrouch();

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

	void Reload();

	void OpenCloseInventory();

	void DropWeapon();

	void UnEquip();

	UFUNCTION()
    void OnRep_OpenCloseInventory();

	UFUNCTION()
    void OnRep_WeaponInteracted();

	UFUNCTION()
	void OnRep_WeaponDropped();

	UFUNCTION()
    void OnRep_SetAiming();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetPlayerPitch(float Pitch);
	bool Server_SetPlayerPitch_Validate(float Pitch);
	void Server_SetPlayerPitch_Implementation(float Pitch);

	UFUNCTION(Server, Reliable, WithValidation)
    void Server_Interact();
	bool Server_Interact_Validate();
	void Server_Interact_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Reload();
	bool Server_Reload_Validate();
	void Server_Reload_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_CancelReload();
	bool Server_CancelReload_Validate();
	void Server_CancelReload_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
    void Server_InventoryClose();
	bool Server_InventoryClose_Validate();
	void Server_InventoryClose_Implementation();

	UFUNCTION(Server, Reliable, WithValidation)
    void Server_Aim(bool bSetShouldAim);
	bool Server_Aim_Validate(bool bSetShouldAim);
	void Server_Aim_Implementation(bool bSetShouldAim);

	void Attack();

	UFUNCTION(Server, Reliable, WithValidation)
    void Server_Attack(FHitResult HitResult);
	bool Server_Attack_Validate(FHitResult HitResult);
	void Server_Attack_Implementation(FHitResult HitResult);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_DropWeapon(class AWeaponBase* WeaponToDrop);
	bool Server_DropWeapon_Validate(class AWeaponBase* WeaponToDrop);
	void Server_DropWeapon_Implementation(class AWeaponBase* WeaponToDrop);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_DropAmmo(EAmmoType AmmoType, int32 AmountToDrop);
	bool Server_DropAmmo_Validate(EAmmoType AmmoType, int32 AmountToDrop);
	void Server_DropAmmo_Implementation(EAmmoType AmmoType, int32 AmountToDrop);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_UnEquip();
	bool Server_UnEquip_Validate();
	void Server_UnEquip_Implementation();
	
	void Die();

	UFUNCTION(NetMulticast, Reliable, WithValidation)
    void Multi_Die();
	bool Multi_Die_Validate();
	void Multi_Die_Implementation();

	void CallDestroy();

	UFUNCTION(BlueprintPure)
	float GetPlayerPitch() const;
	UFUNCTION(BlueprintPure)
    bool GetPlayerHasWeapon() const;
	UFUNCTION(BlueprintPure)
	bool GetIsPlayerAiming() const;
	UFUNCTION(BlueprintPure)
    FString GetPlayerStats() const;
	UFUNCTION(BlueprintPure)
    float GetHealth() const;
	UFUNCTION(BlueprintPure)
    float GetStamina() const;
	UFUNCTION(BlueprintPure)
    float GetHunger() const;
	UFUNCTION(BlueprintPure)
    float GetThirst() const;

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


