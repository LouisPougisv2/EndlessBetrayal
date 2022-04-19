// Fill out your copyright notice in the Description page of Project Settings.


#include "EndlessBetrayalCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "EndlessBetrayal/Weapon/Weapon.h"
#include "EndlessBetrayal/EndlessBetrayalComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"


AEndlessBetrayalCharacter::AEndlessBetrayalCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.0f;
	CameraBoom->bUsePawnControlRotation = true; //So that we can rotate the camera boom along with our controller

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;	//We don't want our character rotating along with our controller (not yet)
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

}

void AEndlessBetrayalCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AEndlessBetrayalCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void AEndlessBetrayalCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AEndlessBetrayalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEndlessBetrayalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Equip"), EInputEvent::IE_Pressed, this, &AEndlessBetrayalCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Crouch"), EInputEvent::IE_Pressed, this, &AEndlessBetrayalCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Aim"), EInputEvent::IE_Pressed, this, &AEndlessBetrayalCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Aim"), EInputEvent::IE_Released, this, &AEndlessBetrayalCharacter::AimButtonReleased);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AEndlessBetrayalCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AEndlessBetrayalCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AEndlessBetrayalCharacter::Turn);
	PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AEndlessBetrayalCharacter::LookUp);

}

void AEndlessBetrayalCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (CombatComponent)
	{
		CombatComponent->Character = this;
	}
}

void AEndlessBetrayalCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));	//return a vector representing the direction of the YawRotation
		AddMovementInput(Direction, Value);	//multiplying the direction here would multiply the speed
	}
}

void AEndlessBetrayalCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AEndlessBetrayalCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AEndlessBetrayalCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AEndlessBetrayalCharacter::EquipButtonPressed()
{
	if (CombatComponent)		
	{
		if (HasAuthority())		//So only the server is calling this function
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}


void AEndlessBetrayalCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent)		
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void AEndlessBetrayalCharacter::AimButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(true);
	}
}
void AEndlessBetrayalCharacter::AimButtonReleased()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(false);
	}
}

void AEndlessBetrayalCharacter::CrouchButtonPressed()
{
	
	if (bIsCrouched)			//Inherited public variable
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AEndlessBetrayalCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void AEndlessBetrayalCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
	 
}

bool AEndlessBetrayalCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

bool AEndlessBetrayalCharacter::IsAiming() 
{
	return (CombatComponent && CombatComponent->bIsAiming);
}
