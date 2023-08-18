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
#include "Kismet/KismetMathLibrary.h"


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
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 850.0f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
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

	AimOffset(DeltaTime);
}

void AEndlessBetrayalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &AEndlessBetrayalCharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Equip"), EInputEvent::IE_Pressed, this, &AEndlessBetrayalCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Crouch"), EInputEvent::IE_Pressed, this, &AEndlessBetrayalCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Aim"), EInputEvent::IE_Pressed, this, &AEndlessBetrayalCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Aim"), EInputEvent::IE_Released, this, &AEndlessBetrayalCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction(TEXT("Fire"), EInputEvent::IE_Pressed, this, &AEndlessBetrayalCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Fire"), EInputEvent::IE_Released, this, &AEndlessBetrayalCharacter::FireButtonReleased);

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

void AEndlessBetrayalCharacter::PlayFireMontage(bool bIsAiming)
{
	if(!IsValid(CombatComponent) || !IsValid(CombatComponent->EquippedWeapon)) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if(IsValid(AnimInstance) && IsValid(FireWeaponMontage))
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		
		FName SectionName;
		SectionName = bIsAiming ? FName("FireAim") : FName("FireHip");
		
		//Why not use -> AnimInstance->Montage_JumpToSection(SectionName, FireWeaponMontage);
		AnimInstance->Montage_JumpToSection(SectionName);
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

void AEndlessBetrayalCharacter::AimOffset(float DeltaTime)
{
	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr) return;
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	float Speed = Velocity.Size();

	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if ((Speed == 0.0f) && (!bIsInAir))	//We're standing still and not jumping
	{
		FRotator CurrentAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAOYaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;

		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.0f || bIsInAir)	//Running or jumping
	{
		StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		AO_Yaw = 0.0f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.0f && !IsLocallyControlled())
	{
		//Mapping pitch from the range 270-360 to the range -90 - 0
		FVector2D InRange(270.0f, 360.0);
		FVector2D OutRange(-90.0f, 0.0f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AEndlessBetrayalCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void AEndlessBetrayalCharacter::FireButtonPressed()
{
	if(ensureAlways(IsValid(CombatComponent)))
	{
		CombatComponent->FireButtonPressed(true);
	}
}

void AEndlessBetrayalCharacter::FireButtonReleased()
{
	if(ensureAlways(IsValid(CombatComponent)))
	{
		CombatComponent->FireButtonPressed(false);
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

void AEndlessBetrayalCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw < -90.0)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	else if (AO_Yaw > 90.0)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else { TurningInPlace = ETurningInPlace::ETIP_NotTurning; }

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAOYaw = FMath::FInterpTo(InterpAOYaw, 0.0f, DeltaTime, 6.0f);
		AO_Yaw = InterpAOYaw;
		if (FMath::Abs(AO_Yaw) < 15.0f)		//Once we've turned "enough"
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		}
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

AWeapon* AEndlessBetrayalCharacter::GetEquippedWeapon()
{
	if (CombatComponent == nullptr) return nullptr;
	
	return CombatComponent->EquippedWeapon;
}