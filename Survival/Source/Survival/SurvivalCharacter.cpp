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

#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/SkeletalMeshComponent.h"
#include "TimerManager.h"

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

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASurvivalCharacter::Interact);
	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &ASurvivalCharacter::Attack);
	PlayerInputComponent->BindAction("Inventory", IE_Pressed, this, &ASurvivalCharacter::OpenCloseInventory);

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

void ASurvivalCharacter::BeginPlay() 
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(SprintingHandle, this, &ASurvivalCharacter::HandleSprinting, 1.f, true);

}

UInventory* ASurvivalCharacter::GetInventoryComp() 
{
	return InventoryComp;
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

void ASurvivalCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		if (!bIsSprinting)
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

// UnPause Sprinting Timer On Server
void ASurvivalCharacter::StartSprinting() 
{
	if (PlayerStatsComp->GetStamina() > 10.0f)
	{
		bIsSprinting = true;
		PlayerStatsComp->ControlSprintingTimer(true);
	}
	else if (PlayerStatsComp->GetStamina() <= 0.f)
	{
		PlayerStatsComp->ControlSprintingTimer(false);
	}

}

// Pause Sprinting Timer On Server
void ASurvivalCharacter::StopSprinting()
{
	bIsSprinting = false;
	PlayerStatsComp->ControlSprintingTimer(false);

}

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

void ASurvivalCharacter::TryToJump() 
{
	if (PlayerStatsComp->GetStamina() > 10.0f && !GetCharacterMovement()->IsFalling())
	{
		Jump();
		PlayerStatsComp->LowerStamina(10.0f);
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
		if (APickupBase* Pickup = Cast<APickupBase>(Actor))
		{
			ServerInteract();
		}
		else if (AStorageContainer* Container = Cast<AStorageContainer>(Actor))
		{
			//ServerInteract();
			OpenedContainer = Container;
			OpenCloseInventory();
		}
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
			OpenedContainer = nullptr;
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

bool ASurvivalCharacter::ServerInteract_Validate() 
{
	return true;
}

void ASurvivalCharacter::ServerInteract_Implementation() 
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
				if (UInventory* ContainerInventory = Container->GetInventoryComponent())
				{
					TArray<APickupBase*> ContainerItems = ContainerInventory->GetInventoryItems();
				}
			}
		}
	}	
}

void ASurvivalCharacter::Attack() 
{
	FVector Start;
	FRotator Rotator;
	GetController()->GetPlayerViewPoint(OUT Start, OUT Rotator);
	FVector End = Start + FollowCamera->GetForwardVector() * 750.f;
	FHitResult HitResult = LineTraceComp->LineTraceSingle(Start, End, true);

	if (AActor* Actor = HitResult.GetActor())
	{
		if (ASurvivalCharacter* Player = Cast<ASurvivalCharacter>(Actor))
		{
			ServerAttack();
		}
	}
}

bool ASurvivalCharacter::ServerAttack_Validate() 
{
	return true;
}

void ASurvivalCharacter::ServerAttack_Implementation() 
{
	if (GetLocalRole() == ROLE_Authority)
	{
		FVector Start;
		FRotator Rotator;
		GetController()->GetPlayerViewPoint(OUT Start, OUT Rotator);
		FVector End = Start + FollowCamera->GetForwardVector() * 750.f;
		FHitResult HitResult = LineTraceComp->LineTraceSingle(Start, End, true);

		if (AActor* Actor = HitResult.GetActor())
		{
			if (ASurvivalCharacter* Player = Cast<ASurvivalCharacter>(Actor))
			{
				float TempDamage = 20.f;
				Player->TakeDamage(TempDamage, FDamageEvent(), GetController(), this);
			}
		}
	}	
}

void ASurvivalCharacter::Die() 
{
	if (GetLocalRole() == ROLE_Authority)
	{
		InventoryComp->DropAllInventory();
		MultiDie();
		ASurvivalGameMode* GM = Cast<ASurvivalGameMode>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->Respawn(GetController());
		}
		// Start destory timer to remove actor from world
		GetWorld()->GetTimerManager().SetTimer(DestroyHandle, this, &ASurvivalCharacter::CallDestroy, 10.f, false);
	}
}

bool ASurvivalCharacter::MultiDie_Validate() 
{
	return true;
}

void ASurvivalCharacter::MultiDie_Implementation() 
{
	this->GetCapsuleComponent()->DestroyComponent();
	this->GetCharacterMovement()->DisableMovement();
	this->GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	this->GetMesh()->SetAllBodiesSimulatePhysics(true);
}

void ASurvivalCharacter::CallDestroy() 
{
	Destroy();
}

AStorageContainer* ASurvivalCharacter::GetOpenedContainer()
{
	return OpenedContainer;
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

FString ASurvivalCharacter::ReturnPlayerStats() 
{
	FString ReturnString = "Health: "
	+ FString::SanitizeFloat(PlayerStatsComp->GetHealth())
	+ ",  Hunger: " 
	+ FString::SanitizeFloat(PlayerStatsComp->GetHunger()) 
	+ ",  Thirst: " 
	+ FString::SanitizeFloat(PlayerStatsComp->GetThirst())
	+ ",  Stamina: "
	+ FString::SanitizeFloat(PlayerStatsComp->GetStamina());
	return ReturnString;
}

float ASurvivalCharacter::ReturnHealth() const
{
	return PlayerStatsComp->GetHealth() / 100.f;
}

float ASurvivalCharacter::ReturnStamina() const
{
	return PlayerStatsComp->GetStamina() / 100.f;
}

float ASurvivalCharacter::ReturnHunger() const
{
	return PlayerStatsComp->GetHunger() / 100.f;
}

float ASurvivalCharacter::ReturnThirst() const
{
	return PlayerStatsComp->GetThirst() / 100.f;
}
