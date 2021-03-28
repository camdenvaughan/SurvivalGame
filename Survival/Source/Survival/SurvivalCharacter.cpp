// Copyright Epic Games, Inc. All Rights Reserved.

#include "SurvivalCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

#include "SurvivalGameMode.h"
#include "PlayerStatsComponent.h"
#include "LineTracer.h"
#include "PickupBase.h"
#include "Inventory.h"
#include "StorageContainer.h"
#include "Survival/Public/Weapons/WeaponBase.h"
#include "Survival/Public/Weapons/AmmoBase.h"

#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Weapons/AmmoBase.h"

//////////////////////////////////////////////////////////////////////////
// ASurvivalCharacter

ASurvivalCharacter::ASurvivalCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inheritesd from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)

	PlayerStatsComp = CreateDefaultSubobject<UPlayerStatsComponent>(TEXT("Player Stats Component"));
	LineTraceComp = CreateDefaultSubobject<ULineTracer>(TEXT("Line Tracer"));
	InventoryComp = CreateDefaultSubobject<UInventory>(TEXT("Inventory Component"));

	bIsSprinting = false;
	bIsAiming = false;
	static ConstructorHelpers::FClassFinder<UUserWidget> InventoryRef(TEXT("/Game/Blueprints/Hud/WBP_InventoryBase"));

	if (InventoryRef.Class)
	{
		InventoryWidgetClass = InventoryRef.Class;
	}
}


//////////////////////////////////////////////////////////////////////////
// Input

void ASurvivalCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASurvivalCharacter::TryToJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASurvivalCharacter::StartSprinting);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASurvivalCharacter::StopSprinting);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASurvivalCharacter::StartCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &ASurvivalCharacter::StopCrouch);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASurvivalCharacter::Interact);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ASurvivalCharacter::Reload);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ASurvivalCharacter::Attack);
	PlayerInputComponent->BindAction("Inventory", IE_Pressed, this, &ASurvivalCharacter::OpenCloseInventory);

	PlayerInputComponent->BindAction("Aiming", IE_Pressed, this, &ASurvivalCharacter::SetIsAiming);
	PlayerInputComponent->BindAction("Aiming", IE_Released, this, &ASurvivalCharacter::SetIsNotAiming);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &ASurvivalCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASurvivalCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ASurvivalCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ASurvivalCharacter::LookUpAtRate);
}

// Called On Start
void ASurvivalCharacter::BeginPlay() 
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(SprintingHandle, this, &ASurvivalCharacter::HandleSprinting, 1.f, true);
}

void ASurvivalCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASurvivalCharacter, OpenedContainer, COND_OwnerOnly);
	DOREPLIFETIME(ASurvivalCharacter, Weapon);
	DOREPLIFETIME(ASurvivalCharacter, bIsAiming);
	DOREPLIFETIME(ASurvivalCharacter, bIsReloading);
}

// Input Functions
//
//
void ASurvivalCharacter::TryToJump() 
{
	if (PlayerStatsComp->GetStamina() > 10.0f && !GetCharacterMovement()->IsFalling())
	{
		Jump();
		PlayerStatsComp->LowerStamina(10.0f);
	}
}

void ASurvivalCharacter::StartSprinting() 
{
	if (PlayerStatsComp->GetStamina() > 10.0f)
	{
		bIsSprinting = true;
		PlayerStatsComp->ControlSprintingTimer(true);
		Server_CancelReload();
	}
	else if (PlayerStatsComp->GetStamina() <= 0.f)
	{
		PlayerStatsComp->ControlSprintingTimer(false);
	}

}

void ASurvivalCharacter::StopSprinting()
{
	bIsSprinting = false;
	PlayerStatsComp->ControlSprintingTimer(false);

}

void ASurvivalCharacter::StartCrouch()
{
	if (!GetCharacterMovement()->IsCrouching() && !GetCharacterMovement()->IsFalling())
	{
		GetCharacterMovement()->bWantsToCrouch = true;
	}
}

void ASurvivalCharacter::StopCrouch()
{
	if (GetCharacterMovement()->IsCrouching())
	{
		GetCharacterMovement()->bWantsToCrouch = false;
	}
}

void ASurvivalCharacter::Interact() 
{
	FVector Start;
	FRotator Rotator;
	GetController()->GetPlayerViewPoint(OUT Start, OUT Rotator);
	FVector End = Start + FollowCamera->GetForwardVector() * 600.f;
	FHitResult HitResult = LineTraceComp->LineTraceSingle(Start, End, true);

	if (AActor* Actor = HitResult.GetActor())
	{
		if (Cast<APickupBase>(Actor))
		{
			Server_Interact();
		}
		else if (Cast<AStorageContainer>(Actor))
		{
			Server_Interact();
		}
		else if (Cast<AWeaponBase>(Actor))
		{
			Server_Interact();
		}
		else if (Cast<AAmmoBase>(Actor))
		{
			Server_Interact();
		}
	}
}

void ASurvivalCharacter::Reload()
{
	if (GetLocalRole() < ROLE_Authority)
	{
		Server_Reload();
	}
	else if (GetLocalRole() == ROLE_Authority)
	{
		Weapon->Reload(PlayerStatsComp);
		bIsReloading = false;
	}
}

void ASurvivalCharacter::Attack() 
{
	if (Weapon)
	{
		Server_CancelReload();
		Server_Attack(Weapon->Fire());
	}
}

void ASurvivalCharacter::OpenCloseInventory()
{
	if (InventoryWidget && InventoryWidget->IsInViewport())
	{
		InventoryWidget->RemoveFromViewport();
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			PlayerController->bShowMouseCursor = false;
			PlayerController->SetInputMode(FInputModeGameOnly());
		}
		if (OpenedContainer)
		{
			Server_InventoryClose();
		}
	}
	else
	{
		InventoryWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
		if (InventoryWidget)
		{
			InventoryWidget->AddToViewport();
			if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
			{
				PlayerController->bShowMouseCursor = true;
				PlayerController->SetInputMode(FInputModeGameAndUI());
			}
		}
	}
}

void ASurvivalCharacter::SetIsAiming()
{
	if (Weapon)
	{
		Server_Aim(true);
	}
}

void ASurvivalCharacter::SetIsNotAiming()
{
	if (Weapon)
	{
		Server_Aim(false);
	}
}

void ASurvivalCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		if (!bIsSprinting && !GetCharacterMovement()->IsCrouching())
			Value *= 0.5f;

		AddMovementInput(Direction, Value);
	}
}

void ASurvivalCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		if (!bIsSprinting)
			Value *= 0.5f;
			
		AddMovementInput(Direction, Value);
	}
}

void ASurvivalCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ASurvivalCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

// OnRep Functions
//
//
void ASurvivalCharacter::OnRep_OpenCloseInventory()
{
	if (InventoryWidget && InventoryWidget->IsInViewport())
	{
		InventoryWidget->RemoveFromViewport();
	}
	if (!OpenedContainer)
	{
		InventoryWidget->RemoveFromViewport();
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			PlayerController->bShowMouseCursor = false;
			PlayerController->SetInputMode(FInputModeGameOnly());
		}
	}
	else
	{
		InventoryWidget = CreateWidget<UUserWidget>(GetWorld(), InventoryWidgetClass);
		if (InventoryWidget)
		{
			InventoryWidget->AddToViewport();
			if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
			{
				PlayerController->bShowMouseCursor = true;
				PlayerController->SetInputMode(FInputModeGameAndUI());
			}
		}
	}
}

void ASurvivalCharacter::OnRep_WeaponInteracted()
{
	if (Weapon)
	{
		Weapon->SetActorEnableCollision(false);
		Weapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("s_hand_r"));
	}
	else // after drop
	{
		
	}
}

void ASurvivalCharacter::OnRep_SetAiming()
{
	bUseControllerRotationYaw = bIsAiming;
}

// Server Functions
//
//
bool ASurvivalCharacter::Server_Interact_Validate() 
{
	return true;
}

void ASurvivalCharacter::Server_Interact_Implementation() 
{
	if (GetLocalRole() == ROLE_Authority)
	{
		FVector Start;
		FRotator Rotator;
		GetController()->GetPlayerViewPoint(OUT Start, OUT Rotator);
		FVector End = Start + FollowCamera->GetForwardVector() * 600.f;
		FHitResult HitResult = LineTraceComp->LineTraceSingle(Start, End, true);

		if (AActor* Actor = HitResult.GetActor())
		{
			if (APickupBase* Pickup = Cast<APickupBase>(Actor))
			{
				if (InventoryComp->AddItem(Pickup))
				{
	
				}
			}
			else if (AStorageContainer* Container = Cast<AStorageContainer>(Actor))
			{
				if (Container->IsChestOpen() && !OpenedContainer) return;
				
				bool bOpenChest = false;
				if (OpenedContainer)
				{
					OpenedContainer = nullptr;
				}
				else
				{
					OpenedContainer = Container;
					bOpenChest = true;
				}
				Container->OpenChest(bOpenChest);	
			}
			else if (AWeaponBase* HitWeapon = Cast<AWeaponBase>(Actor))
			{
				Weapon = HitWeapon;
				Weapon->SetOwner(this);
				OnRep_WeaponInteracted();
			}
			else if (AAmmoBase* HitAmmo = Cast <AAmmoBase>(Actor))
			{
				UE_LOG(LogTemp, Warning, TEXT("Server Hit Ammo"));
				HitAmmo->InteractedWith(PlayerStatsComp);
			}
		}
	}	
}

bool ASurvivalCharacter::Server_InventoryClose_Validate()
{
	return true;
}

bool ASurvivalCharacter::Server_Reload_Validate()
{
	return true;
}

void ASurvivalCharacter::Server_Reload_Implementation()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		if (Weapon)
		{
			if (Weapon->GetAmmoType() == EAmmoType::E_AssaultAmmo)
			{
				UE_LOG(LogTemp, Warning, TEXT("Starting Reload"))
				bIsReloading = true;
				GetWorld()->GetTimerManager().SetTimer(ReloadHandle, this, &ASurvivalCharacter::Reload, Weapon->GetReloadTime(), false);
				//Weapon->Reload(PlayerStatsComp);
			}
		}
	}
}

bool ASurvivalCharacter::Server_CancelReload_Validate()
{
	return true;
}

void ASurvivalCharacter::Server_CancelReload_Implementation()
{
	if (Weapon)
	{
		if (bIsReloading)
		{
			bIsReloading = false;
			GetWorld()->GetTimerManager().ClearTimer(ReloadHandle);
			UE_LOG(LogTemp, Warning, TEXT("ReloadCancelled"));
		}
	}
}

void ASurvivalCharacter::Server_InventoryClose_Implementation()
{
	if (OpenedContainer)
	{
		OpenedContainer->OpenChest(false);
	}
	OpenedContainer = nullptr;
}

bool ASurvivalCharacter::Server_Aim_Validate(bool bSetShouldAim)
{
	return true;
}

void ASurvivalCharacter::Server_Aim_Implementation(bool bSetShouldAim)
{
	bIsAiming = bSetShouldAim;
	OnRep_SetAiming();
}

bool ASurvivalCharacter::Server_Attack_Validate(FHitResult HitResult)
{
	return true;
}

void ASurvivalCharacter::Server_Attack_Implementation(FHitResult HitResult)
{
	if (GetLocalRole() == ROLE_Authority && Weapon)
	{
		Weapon->Fire(HitResult);
	}	
}


// MultiCast Functions
//
//
bool ASurvivalCharacter::Multi_Die_Validate() 
{
	return true;
}

void ASurvivalCharacter::Multi_Die_Implementation() 
{
	this->GetCapsuleComponent()->DestroyComponent();
	this->GetCharacterMovement()->DisableMovement();
	this->GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	this->GetMesh()->SetAllBodiesSimulatePhysics(true);
}

// Helper Functions
//
//
void ASurvivalCharacter::HandleSprinting() 
{
	if (bIsSprinting && this->GetVelocity().Size())
	{
		PlayerStatsComp->LowerStamina(2.0f);
		if (PlayerStatsComp->GetStamina() <= 0.0f)
		{
			StopSprinting();
		}
	}
}

float ASurvivalCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, class AActor* DamageCauser) 
{
	if (GetLocalRole() < ROLE_Authority || PlayerStatsComp->GetHealth() <= 0.0f)
		return 0.0f;

	float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.f)
	{
		PlayerStatsComp->LowerHealth(ActualDamage);
		if (PlayerStatsComp->GetHealth() <= 0.0f)
		{
			Die();
		}
	}	
	return ActualDamage;
}

void ASurvivalCharacter::Die() 
{
	if (GetLocalRole() == ROLE_Authority)
	{
		Server_InventoryClose();
		InventoryComp->DropAllInventory();
		Multi_Die();
		ASurvivalGameMode* GM = Cast<ASurvivalGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->Respawn(GetController());
		}
		// Start destory timer to remove actor from world
		GetWorld()->GetTimerManager().SetTimer(DestroyHandle, this, &ASurvivalCharacter::CallDestroy, 10.f, false);
	}
}

void ASurvivalCharacter::CallDestroy() 
{
	Destroy();
}

// Getter Functions
//
//
bool ASurvivalCharacter::GetPlayerHasWeapon() const
{
	if (Weapon)
		return true;
	else
		return false;
}

FString ASurvivalCharacter::GetPlayerStats() const
{
	FString ReturnString = "Health: "
	+ FString::SanitizeFloat(PlayerStatsComp->GetHealth())
	+ ",  Hunger: " 
	+ FString::SanitizeFloat(PlayerStatsComp->GetHunger()) 
	+ ",  Thirst: " 
	+ FString::SanitizeFloat(PlayerStatsComp->GetThirst())
	+ ",  Stamina: "
	+ FString::SanitizeFloat(PlayerStatsComp->GetStamina())
	+ ", Assault Ammo: "
	+ FString::FromInt(PlayerStatsComp->GetAssaultAmmo());
	return ReturnString;
}

float ASurvivalCharacter::GetHealth() const
{
	return PlayerStatsComp->GetHealth() / 100.f;
}

float ASurvivalCharacter::GetStamina() const
{
	return PlayerStatsComp->GetStamina() / 100.f;
}

float ASurvivalCharacter::GetHunger() const
{
	return PlayerStatsComp->GetHunger() / 100.f;
}

float ASurvivalCharacter::GetThirst() const
{
	return PlayerStatsComp->GetThirst() / 100.f;
}

AStorageContainer* ASurvivalCharacter::GetOpenedContainer() const
{
	return OpenedContainer;
}

UInventory* ASurvivalCharacter::GetInventoryComponent() const
{
	return InventoryComp;
}
