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
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

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
	InventoryComp->SetIsReplicated(true);

	bIsSprinting = false;
	bIsAiming = false;
	bWeaponIsOnBack = false;
	bDebugIsOn = false;
	InteractText = "";
	static ConstructorHelpers::FClassFinder<UUserWidget> InventoryRef(TEXT("/Game/Blueprints/Hud/WBP_InventoryBase"));
	static ConstructorHelpers::FClassFinder<UUserWidget> PauseRef(TEXT("/Game/Blueprints/Hud/WBP_PauseScreen"));

	if (InventoryRef.Class)
	{
		InventoryWidgetClass = InventoryRef.Class;
	}
	if (PauseRef.Class)
	{
		PauseWidgetClass = PauseRef.Class;
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
	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &ASurvivalCharacter::DropWeapon); 
	PlayerInputComponent->BindAction("UnEquip", IE_Pressed, this, &ASurvivalCharacter::UnEquip);
	PlayerInputComponent->BindAction("ShowDebug", IE_Pressed, this, &ASurvivalCharacter::ToggleDebug);
	PlayerInputComponent->BindAction("Pause", IE_Pressed, this, &ASurvivalCharacter::TogglePause);
	
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
//
//
void ASurvivalCharacter::BeginPlay() 
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(SprintingHandle, this, &ASurvivalCharacter::HandleSprinting, 1.f, true);
	bIsDead = false;
}

void ASurvivalCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASurvivalCharacter, OpenedContainer, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ASurvivalCharacter, PlayerPitch, COND_SkipOwner);
	DOREPLIFETIME(ASurvivalCharacter, ActiveWeapon);
	DOREPLIFETIME(ASurvivalCharacter, SpawnedWeapon);	
	DOREPLIFETIME_CONDITION(ASurvivalCharacter, DroppedWeaponName, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(ASurvivalCharacter, HoldingWeaponName, COND_OwnerOnly);
	DOREPLIFETIME(ASurvivalCharacter, bWeaponIsOnBack);
	DOREPLIFETIME(ASurvivalCharacter, bIsAiming);
	DOREPLIFETIME(ASurvivalCharacter, bIsReloading);
	DOREPLIFETIME(ASurvivalCharacter, bIsDead);
}


void ASurvivalCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!GetController()) return;
	FVector Start;
	FRotator Rotator;
	GetController()->GetPlayerViewPoint(OUT Start, OUT Rotator);
	FVector End = Start + FollowCamera->GetForwardVector() * 600.f;
	FHitResult HitResult = LineTraceComp->LineTraceSingle(Start, End, false);

	if (AActor* Actor = HitResult.GetActor())
	{
		if (Cast<APickupBase>(Actor))
		{
			InteractText = "Press E to Pickup Item";
		}
		else if (Cast<AStorageContainer>(Actor))
		{
			InteractText = "Press E to Open Container";
		}
		else if (Cast<AWeaponBase>(Actor))
		{
			if (ActiveWeapon)
			{
				InteractText = "Press E to Swap Weapon";
			}
			else
			{
				InteractText = "Press E to Pickup Weapon";			
			}
		}
		else if (Cast<AAmmoBase>(Actor))
		{
			InteractText = "Press E to Pickup Ammo";
		}
		else
		{
			InteractText = "";
		}
	}
	else
	{
		InteractText = "";
	}
	
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
		Server_CancelReload();
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
		Server_CancelReload();
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
	if (bIsDead) return;
	FVector Start;
	FRotator Rotator;
	GetController()->GetPlayerViewPoint(OUT Start, OUT Rotator);
	FVector End = Start + FollowCamera->GetForwardVector() * 600.f;
	FHitResult HitResult = LineTraceComp->LineTraceSingle(Start, End, bDebugIsOn);

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
	if (bIsDead) return;
	if (GetLocalRole() < ROLE_Authority)
	{
		if (ActiveWeapon->GetCanReload() && PlayerStatsComp->GetAmmo(ActiveWeapon->GetAmmoType()) > 0)
		{
			Server_Reload();
		}
	}
	else if (GetLocalRole() == ROLE_Authority)
	{
		if (ActiveWeapon->GetCanReload() && PlayerStatsComp->GetAmmo(ActiveWeapon->GetAmmoType()) > 0)
		{
			ActiveWeapon->Reload(PlayerStatsComp);
			bIsReloading = false;
		}
	}
}

void ASurvivalCharacter::Attack() 
{
	if (bIsDead) return;
	if (ActiveWeapon && !bWeaponIsOnBack)
	{
		FVector CamStart;
		FVector CamEnd;
		if (bIsAiming)
		{
			CamStart = FollowCamera->GetComponentLocation();
			CamEnd = CamStart + FollowCamera->GetComponentRotation().Vector() * 3500.f;
		}
		else
		{
			CamStart = ActiveWeapon->GetMuzzleLocation();
			CamEnd = ActiveWeapon->GetMuzzleRotation().Vector() * 3500.f;
		}
		FVector ImpactPoint = LineTraceComp->LineTraceSingle(CamStart, CamEnd).ImpactPoint;
		
		Server_CancelReload();
		Server_Attack(ActiveWeapon->Fire(ImpactPoint));
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

void ASurvivalCharacter::DropWeapon()
{
	if (bIsDead) return;
	if (ActiveWeapon)
	{
		if(GetLocalRole() < ROLE_Authority)
		{
			UE_LOG(LogTemp, Warning, TEXT("Call from inside DropWeapon()"));
			Server_DropWeapon(ActiveWeapon);
			ActiveWeapon->Destroy();
		}

	}
	else
	{
		Server_DropWeapon(ActiveWeapon);
	}
}

void ASurvivalCharacter::UnEquip()
{
	if (bIsDead) return;
	if (ActiveWeapon)
	{
		Server_CancelReload();
		Server_UnEquip();
	}
}

void ASurvivalCharacter::ToggleDebug()
{
	if (bDebugIsOn)
		bDebugIsOn = false;
	else
		bDebugIsOn = true;
}

void ASurvivalCharacter::TogglePause()
{
	if (PauseWidget && PauseWidget->IsInViewport())
	{
		PauseWidget->RemoveFromViewport();
		if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
		{
			PlayerController->bShowMouseCursor = false;
			PlayerController->SetInputMode(FInputModeGameOnly());
		}
	}
	else
	{
		PauseWidget = CreateWidget<UUserWidget>(GetWorld(), PauseWidgetClass);
		if (PauseWidget)
		{
			PauseWidget->AddToViewport();
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
	if (bIsReloading) return;
	if (ActiveWeapon)
	{
		Server_Aim(true);
		CameraBoom->TargetArmLength = 200.0f;
		CameraBoom->SocketOffset.Y = 50.f;
	}
}

void ASurvivalCharacter::SetIsNotAiming()
{
	if (ActiveWeapon)
	{
		Server_Aim(false);
		CameraBoom->TargetArmLength = 400.0f;
		CameraBoom->SocketOffset.Y = 0.f;
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

	FRotator NormalizedRot = (GetControlRotation() - GetActorRotation()).GetNormalized();
	PlayerPitch = NormalizedRot.Pitch;

	Server_SetPlayerPitch(PlayerPitch);
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
	if (ActiveWeapon)
	{
		if (HoldingWeaponName != "")
		{
			ActiveWeapon->SetupWeapon(HoldingWeaponName);
		}
		else
		{
			ActiveWeapon->SetupWeapon(FName("AR-15"));
			UE_LOG(LogTemp, Warning, TEXT("Something Went Wrong, Unable to Retrieve Picked Up Weapon Name."))
		}
		ActiveWeapon->SetActorEnableCollision(false);
		ActiveWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("s_hand_r"));
	}
}

void ASurvivalCharacter::OnRep_WeaponDropped()
{
	if (SpawnedWeapon)
	{
		if (DroppedWeaponName != "")
		{
			SpawnedWeapon->SetupWeapon(DroppedWeaponName);
		}
		else
		{
			SpawnedWeapon->SetupWeapon(FName("AR-15"));
			UE_LOG(LogTemp, Warning, TEXT("Something Went Wrong, Unable to Retrieve Dropped Weapon Name."))
		}
	}
}

void ASurvivalCharacter::OnRep_SetAiming()
{
	bUseControllerRotationYaw = bIsAiming;
}

bool ASurvivalCharacter::Server_SetPlayerPitch_Validate(float Pitch)
{
	return true;
}

void ASurvivalCharacter::Server_SetPlayerPitch_Implementation(float Pitch)
{
	PlayerPitch = Pitch;
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
		FHitResult HitResult = LineTraceComp->LineTraceSingle(Start, End, bDebugIsOn);

		if (AActor* Actor = HitResult.GetActor())
		{
			if (APickupBase* Pickup = Cast<APickupBase>(Actor))
			{
				InventoryComp->AddItem(Pickup);
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
				if (ActiveWeapon)
				{
					Server_DropWeapon(ActiveWeapon);
				}
				HoldingWeaponName = HitWeapon->GetWeaponName();
				ActiveWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass, FVector(0,0, 200.f), FRotator::ZeroRotator); // Spawn Weapon
				ActiveWeapon->SetOwner(this);
				HitWeapon->Destroy();
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
		if (ActiveWeapon)
		{
			if (ActiveWeapon->GetAmmoType() == EAmmoType::E_AssaultAmmo)
			{
				UE_LOG(LogTemp, Warning, TEXT("Starting Reload"))
				bIsReloading = true;
				GetWorld()->GetTimerManager().SetTimer(ReloadHandle, this, &ASurvivalCharacter::Reload, ActiveWeapon->GetReloadTime(), false);
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
	if (ActiveWeapon)
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
	if (GetLocalRole() == ROLE_Authority && ActiveWeapon)
	{
		FVector CamStart = FollowCamera->GetComponentLocation();
		FVector CamEnd = CamStart + FollowCamera->GetComponentRotation().Vector() * 3500.f;
		FVector ImpactPoint = LineTraceComp->LineTraceSingle(CamStart, CamEnd).ImpactPoint;
		
		ActiveWeapon->Fire(HitResult, ImpactPoint);
		Cast<APlayerController>(GetController())->ClientStartCameraShake(FireShake);
	}	
}

bool ASurvivalCharacter::Server_DropWeapon_Validate(AWeaponBase* WeaponToDrop)
{
	return true;
}

void ASurvivalCharacter::Server_DropWeapon_Implementation(AWeaponBase* WeaponToDrop)
{
	if (GetLocalRole() == ROLE_Authority)
	{		
		FVector Location = this->GetActorLocation() + (this->GetActorForwardVector() * 50.f);

		FVector EndRay = Location;
		EndRay.Z -= 2000.f;

		FHitResult HitResult;
		FCollisionObjectQueryParams ObjQuery;
		FCollisionQueryParams CollisionParams;
		CollisionParams.AddIgnoredActor(this);
		GetWorld()->LineTraceSingleByObjectType(
            OUT HitResult,
            Location,
            EndRay,
            ObjQuery,
            CollisionParams
        );
		
		if (bDebugIsOn)
			DrawDebugLine(GetWorld(), Location, EndRay, FColor::Red,false, 3.0f, 0, 5.f );
		
		if (HitResult.ImpactPoint != FVector::ZeroVector)
		{
			Location = HitResult.ImpactPoint;
			Location.Z += 2.f;
		}
		FRotator Rotation = FRotator(90.f, FMath::RandRange(0.f, 360.f), 0);

		SpawnedWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass, Location, Rotation);
		DroppedWeaponName = ActiveWeapon->GetWeaponName();

		ActiveWeapon->Destroy();
		ActiveWeapon = nullptr;
		
		OnRep_WeaponDropped();
	}
}

bool ASurvivalCharacter::Server_DropAmmo_Validate(EAmmoType AmmoType, int32 AmountToDrop)
{
	return true;
}

void ASurvivalCharacter::Server_DropAmmo_Implementation(EAmmoType AmmoType, int32 AmountToDrop)
{
	if (PlayerStatsComp->GetAmmo(AmmoType) <= 0) return;
	FVector Location = this->GetActorLocation() + (this->GetActorForwardVector() * 50.f);
	FVector EndRay = Location;
	EndRay.Z -= 2000.f;

	FHitResult HitResult;
	FCollisionObjectQueryParams ObjQuery;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByObjectType(
        OUT HitResult,
        Location,
        EndRay,
        ObjQuery,
        CollisionParams
    );
	
	if (bDebugIsOn)
		DrawDebugLine(GetWorld(), Location, EndRay, FColor::Red,false, 3.0f, 0, 5.f );
	
	if (HitResult.ImpactPoint != FVector::ZeroVector)
	{
		Location = HitResult.ImpactPoint;
		Location.Z += 2.f;
	}
	
	FRotator Rotation = FRotator(0, FMath::RandRange(0.f, 360.f), 0);
	if (!AmmoClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("No AmmoClass set in BP"));
		return;
	}
	AAmmoBase* SpawnedAmmo = GetWorld()->SpawnActor<AAmmoBase>(AmmoClass, Location, Rotation);
	SpawnedAmmo->SetupAmmoPickup(AmmoType, AmountToDrop);
}

bool ASurvivalCharacter::Server_UnEquip_Validate()
{
	return true;
}

void ASurvivalCharacter::Server_UnEquip_Implementation()
{
	if (ActiveWeapon)
	{
		if (!bWeaponIsOnBack)
		{
			ActiveWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("s_spine_3"));
			bWeaponIsOnBack = true;
			return;
		}
		else
		{
			ActiveWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("s_hand_r"));
			bWeaponIsOnBack = false;
		}
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

bool ASurvivalCharacter::Multi_PlayEmitterAtLocation_Validate(UParticleSystem* Emitter, FVector Location)
{
	return true;
}

void ASurvivalCharacter::Multi_PlayEmitterAtLocation_Implementation(UParticleSystem* Emitter, FVector Location)
{
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Emitter, Location);
}

bool ASurvivalCharacter::Multi_PlayEmitterAttached_Validate(UParticleSystem* Emitter, USceneComponent* Component, FName Socket)
{
	return true;
}

void ASurvivalCharacter::Multi_PlayEmitterAttached_Implementation(UParticleSystem* Emitter, USceneComponent* Component, FName Socket)
{
	UGameplayStatics::SpawnEmitterAttached(Emitter, Component, Socket);
}

bool ASurvivalCharacter::Multi_PlaySoundAtLocation_Validate(USoundBase* Sound, FVector Location)
{
	return true;
}

void ASurvivalCharacter::Multi_PlaySoundAtLocation_Implementation(USoundBase* Sound, FVector Location)
{
	UGameplayStatics::SpawnSoundAtLocation(GetWorld(), Sound, Location);
}

bool ASurvivalCharacter::Multi_PlaySoundAttached_Validate(USoundBase* Sound, USceneComponent* Component, FName Socket)
{
	return true;
}

void ASurvivalCharacter::Multi_PlaySoundAttached_Implementation(USoundBase* Sound, USceneComponent* Component, FName Socket)
{
	UGameplayStatics::SpawnSoundAttached(Sound, Component, Socket);
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
	if (!Cast<APlayerController>(GetController())) return 0.0f;
	if (GetLocalRole() < ROLE_Authority || PlayerStatsComp->GetHealth() <= 0.0f)
		return 0.0f;

	float ActualDamage = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);

	if (ActualDamage > 0.f)
	{
		PlayerStatsComp->LowerHealth(ActualDamage);
		Cast<APlayerController>(GetController())->ClientStartCameraShake(HitShake);
		if (PlayerStatsComp->GetHealth() <= 0.0f)
		{
			bIsDead = true;
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
		if (ActiveWeapon)
		{
			Server_DropWeapon(ActiveWeapon);			
		}
		Server_DropAmmo(EAmmoType::E_AssaultAmmo, PlayerStatsComp->GetAmmo(EAmmoType::E_AssaultAmmo));
		Server_DropAmmo(EAmmoType::E_ShotgunAmmo, PlayerStatsComp->GetAmmo(EAmmoType::E_ShotgunAmmo));
		Server_DropAmmo(EAmmoType::E_SniperAmmo, PlayerStatsComp->GetAmmo(EAmmoType::E_SniperAmmo));
		Multi_Die();
		ASurvivalGameMode* SurvivalGameMode = Cast<ASurvivalGameMode>(GetWorld()->GetAuthGameMode());
		if (SurvivalGameMode)
		{
			SurvivalGameMode->Respawn(GetController());
		}
		// Start destroy timer to remove actor from world
		GetWorld()->GetTimerManager().SetTimer(DestroyHandle, this, &ASurvivalCharacter::CallDestroy, 10.f, false);
	}
}

void ASurvivalCharacter::CallDestroy() 
{
	Destroy();
}

float ASurvivalCharacter::GetPlayerPitch() const
{
	return PlayerPitch;
}

// Getter Functions
//
//
bool ASurvivalCharacter::GetPlayerHasWeapon() const
{
	if (ActiveWeapon && !bWeaponIsOnBack)
	{
		return true;
	}
	return false;
}

bool ASurvivalCharacter::GetIsPlayerReloading() const
{
	return bIsReloading;
}

bool ASurvivalCharacter::GetIsPlayerAiming() const
{
	return bIsAiming;
}

bool ASurvivalCharacter::GetIsDebugOn() const
{
	return bDebugIsOn;
}

FString ASurvivalCharacter::GetPlayerStats() const
{
	if (ActiveWeapon)
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
        + FString::FromInt(PlayerStatsComp->GetAmmo(EAmmoType::E_AssaultAmmo))
		+ ", Current Magazine: "
		+ FString::FromInt(ActiveWeapon->GetMagazineAmmoCount());
		return ReturnString;
	}
	else
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
		+ FString::FromInt(PlayerStatsComp->GetAmmo(EAmmoType::E_AssaultAmmo));
		return ReturnString;
	}

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

int32 ASurvivalCharacter::GetAmmoForGun() const
{
	if (ActiveWeapon)
	{
		return PlayerStatsComp->GetAmmo(ActiveWeapon->GetAmmoType());
	}
	else
		return 0;
}

int32 ASurvivalCharacter::GetMagazineCount() const
{
	if (ActiveWeapon)
	{
		return ActiveWeapon->GetMagazineAmmoCount();
	}
	else
		return 0;
}

FString ASurvivalCharacter::GetInteractText() const
{
	return InteractText;
}


AStorageContainer* ASurvivalCharacter::GetOpenedContainer() const
{
	return OpenedContainer;
}

UInventory* ASurvivalCharacter::GetInventoryComponent() const
{
	return InventoryComp;
}
