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
#include "EndlessBetrayal/EndlessBetrayal.h"
#include "EndlessBetrayal/EndlessBetrayalComponents/BuffComponent.h"
#include "EndlessBetrayal/GameMode/EndlessBetrayalGameMode.h"
#include "EndlessBetrayal/GameState/EndlessBetrayalPlayerState.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"


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

	//OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	//OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	BuffComponent->SetIsReplicated(true);

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>("AttachedGrenade");
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 850.0f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
}

void AEndlessBetrayalCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AEndlessBetrayalCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AEndlessBetrayalCharacter, Health);
	DOREPLIFETIME(AEndlessBetrayalCharacter, Shield);
	DOREPLIFETIME(AEndlessBetrayalCharacter, bShouldDisableGameplayInput);
}

void AEndlessBetrayalCharacter::UpdateHealthHUD()
{
	EndlessBetrayalPlayerController = !IsValid(EndlessBetrayalPlayerController) ? Cast<AEndlessBetrayalPlayerController>(Controller) : EndlessBetrayalPlayerController;
	if(IsValid(EndlessBetrayalPlayerController))
	{
		EndlessBetrayalPlayerController->UpdateHealthHUD(Health, MaxHealth);
	}
}

void AEndlessBetrayalCharacter::UpdateShieldHUD()
{
	EndlessBetrayalPlayerController = !IsValid(EndlessBetrayalPlayerController) ? Cast<AEndlessBetrayalPlayerController>(Controller) : EndlessBetrayalPlayerController;
	if(IsValid(EndlessBetrayalPlayerController))
	{
		EndlessBetrayalPlayerController->UpdateShieldHUD(Shield, MaxShield);
	}
}

void AEndlessBetrayalCharacter::PollInitialize()
{
	if(!EndlessBetrayalPlayerState)
	{
		EndlessBetrayalPlayerController = !IsValid(EndlessBetrayalPlayerController) ? Cast<AEndlessBetrayalPlayerController>(Controller) : EndlessBetrayalPlayerController;
		if(IsValid(EndlessBetrayalPlayerController))
		{
			EndlessBetrayalPlayerState = GetPlayerState<AEndlessBetrayalPlayerState>();
			if(IsValid(EndlessBetrayalPlayerState))
			{
				EndlessBetrayalPlayerState->AddToScore(0.0f);
				EndlessBetrayalPlayerState->AddToKills(0);
			}

			EndlessBetrayalPlayerController->HideMessagesOnScreenHUD();
		}
	}
	
}

void AEndlessBetrayalCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnDefaultWeapon();
	UpdateHealthHUD();
	UpdateShieldHUD();

	if(HasAuthority())
	{
		OnTakeAnyDamage.AddUniqueDynamic(this, &AEndlessBetrayalCharacter::ReceiveDamage);
	}

	if(IsValid(CombatComponent))
	{
		CombatComponent->SetGrenadeVisibility(false);
		CombatComponent->UpdateHUDGrenadeAmount();
	}
}

void AEndlessBetrayalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	HideCameraWhenCharacterClose();
	PollInitialize();
}

void AEndlessBetrayalCharacter::RotateInPlace(float DeltaTime)
{
	if(bShouldDisableGameplayInput)
	{
		TurningInPlace= ETurningInPlace::ETIP_NotTurning;
		bUseControllerRotationYaw = false;
		return;
	}
	
	if(GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if(TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void AEndlessBetrayalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Jump"), EInputEvent::IE_Pressed, this, &AEndlessBetrayalCharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Equip"), EInputEvent::IE_Pressed, this, &AEndlessBetrayalCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction(TEXT("SwapWeapons"), EInputEvent::IE_Pressed, this, &AEndlessBetrayalCharacter::SwapWeaponMouseWheelRolled);
	PlayerInputComponent->BindAction(TEXT("Crouch"), EInputEvent::IE_Pressed, this, &AEndlessBetrayalCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Aim"), EInputEvent::IE_Pressed, this, &AEndlessBetrayalCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Aim"), EInputEvent::IE_Released, this, &AEndlessBetrayalCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction(TEXT("Fire"), EInputEvent::IE_Pressed, this, &AEndlessBetrayalCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Fire"), EInputEvent::IE_Released, this, &AEndlessBetrayalCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction(TEXT("Reload"), EInputEvent::IE_Pressed, this, &AEndlessBetrayalCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction(TEXT("ThrowGrenade"), IE_Pressed, this, &AEndlessBetrayalCharacter::GrenadeButtonPressed);

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

	if(BuffComponent)
	{
		BuffComponent->Character = this;
		BuffComponent->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);

		ensureAlways(IsValid(GetCharacterMovement()));
		BuffComponent->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
}

void AEndlessBetrayalCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();
	
	TimeSinceLastMovementReplication = 0.0f;
}

void AEndlessBetrayalCharacter::Destroyed()
{
	if(EliminationBotComponent)
	{
		EliminationBotComponent->DestroyComponent();
	}
	if(CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->EquippedWeapon->OnWeaponDropped();
	}
	Super::Destroyed();
}

void AEndlessBetrayalCharacter::SpawnDefaultWeapon()
{
	//UGameplayStatics::GetGameMode(this) returns null if not on the server
	AEndlessBetrayalGameMode* EndlessBetrayalGameMode = Cast<AEndlessBetrayalGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();
	
	if(IsValid(EndlessBetrayalGameMode) && IsValid(World) && !IsEliminated() && DefaultWeaponClass)
	{
		AWeapon* SpawnedDefaultWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		SpawnedDefaultWeapon->bIsDefaultWeapon = true;
		if(IsValid(CombatComponent))
		{
			CombatComponent->EquipWeapon(SpawnedDefaultWeapon);
		}
	}
}

void AEndlessBetrayalCharacter::UpdateHUDAmmo()
{
	EndlessBetrayalPlayerController = !IsValid(EndlessBetrayalPlayerController) ? Cast<AEndlessBetrayalPlayerController>(Controller) : EndlessBetrayalPlayerController;
	if(IsValid(EndlessBetrayalPlayerController) && IsValid(CombatComponent) && IsValid(CombatComponent->EquippedWeapon))
	{
		EndlessBetrayalPlayerController->UpdateWeaponAmmo(CombatComponent->EquippedWeapon->GetAmmo());
		EndlessBetrayalPlayerController->UpdateWeaponCarriedAmmo(CombatComponent->CarriedAmmo);
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
		
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AEndlessBetrayalCharacter::PlayReloadMontage()
{
	if(!IsValid(CombatComponent) || !IsValid(CombatComponent->EquippedWeapon)) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if(IsValid(AnimInstance) && IsValid(ReloadMontage))
	{
		AnimInstance->Montage_Play(ReloadMontage);
		
		FName SectionName;
		
		switch(CombatComponent->EquippedWeapon->GetWeaponType())
		{
			case EWeaponType::EWT_AssaultRifle:
				SectionName = FName("Rifle");
				break;
			case EWeaponType::EWT_RocketLauncher:
				SectionName = FName("RocketLauncher");
				break;
			case EWeaponType::EWT_Pistol:
				SectionName = FName("Pistol");
				break;
			case EWeaponType::EWT_SMG:
				SectionName = FName("SMG");
				break;
			case EWeaponType::EWT_Shotgun:
				SectionName = FName("Shotgun");
				break;
			case EWeaponType::EWT_SniperRifle:
				SectionName = FName("SniperRifle");
				break;
			case EWeaponType::EWT_GrenadeLauncher:
				SectionName = FName("GrenadeLauncher");
				break;
			
			default:
				break;
		
		}
		
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AEndlessBetrayalCharacter::PlayEliminatedMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if(IsValid(AnimInstance) && IsValid(EliminationMontage))
	{
		AnimInstance->Montage_Play(EliminationMontage);
	}
}

void AEndlessBetrayalCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(IsValid(AnimInstance) && IsValid(ThrowingGrenadeMontage))
	{
		AnimInstance->Montage_Play(ThrowingGrenadeMontage);
	}
}

void AEndlessBetrayalCharacter::OnPlayerEliminated()
{
	DropOrDestroyWeapons();
	MulticastOnPlayerEliminated();
	GetWorldTimerManager().SetTimer(OnPlayerEliminatedTimer, this, &AEndlessBetrayalCharacter::OnPlayerEliminatedCallBack, OnPlayerEliminatedDelayTime);
}

void AEndlessBetrayalCharacter::MulticastOnPlayerEliminated_Implementation()
{
	if(IsValid(EndlessBetrayalPlayerController))
	{
		EndlessBetrayalPlayerController->UpdateWeaponAmmo(0);
	}
	
	bIsEliminated = true;
	PlayEliminatedMontage();

	//Star player's mesh dissolve effect
	if(IsValid(DissolveMaterialInstance))
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 100.0f);
		StartDissolve();
	}

	if(CombatComponent)
	{
		//If player dies while shooting, its firing stops
		CombatComponent->FireButtonPressed(false);
		
		//If player was killed while aiming at sniper
		const bool bShouldHideSniperScope = IsLocallyControlled() && CombatComponent->IsAiming() && IsValid(CombatComponent->EquippedWeapon) && CombatComponent->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
		if(bShouldHideSniperScope)
		{
			CombatComponent->SetAiming(false);
		}
	}
	//Disable movement
	GetCharacterMovement()->DisableMovement(); //Stop movement with WASD
	GetCharacterMovement()->StopMovementImmediately(); //Prevents us from rotating the character

	if(IsValid(EndlessBetrayalPlayerController))
	{
		DisableInput(EndlessBetrayalPlayerController); //To prevent from firing
	}

	//Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	//Spawn EliminationBot
	if(EliminationBotEffect && EliminationBotSound)
	{
		const FVector EliminationBotPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.0f);
		EliminationBotComponent = UGameplayStatics::SpawnEmitterAtLocation(this, EliminationBotEffect, EliminationBotPoint, GetActorRotation());

		UGameplayStatics::PlaySoundAtLocation(this, EliminationBotSound, EliminationBotPoint);
	}
}

void AEndlessBetrayalCharacter::OnPlayerEliminatedCallBack()
{
	//Respawn
	AEndlessBetrayalGameMode* EndlessBetrayalGameMode = GetWorld()->GetAuthGameMode<AEndlessBetrayalGameMode>();
	if(IsValid(EndlessBetrayalGameMode))
	{
		EndlessBetrayalGameMode->RequestRespawn(this, EndlessBetrayalPlayerController );
	}
}

void AEndlessBetrayalCharacter::PlayHitReactMontage()
{
	if(!IsValid(CombatComponent) || !IsValid(CombatComponent->EquippedWeapon)) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if(IsValid(AnimInstance) && IsValid(HitReactionMontage))
	{
		AnimInstance->Montage_Play(HitReactionMontage);

		//TODO : Modify to select Section name based on where the player is shot
		FName SectionName("FromFront");
		
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AEndlessBetrayalCharacter::GrenadeButtonPressed()
{
	if(CombatComponent)
	{
		CombatComponent->ThrowGrenade();
	}
}

void AEndlessBetrayalCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if(IsValid(Weapon))
	{
		//We don't want to drop the default AR when dying
		if(Weapon->bIsDefaultWeapon)
		{
			Weapon->Destroy();
		}
		else
		{
			Weapon->OnWeaponDropped();
		}
	}
}

void AEndlessBetrayalCharacter::DropOrDestroyWeapons()
{
	if(IsValid(CombatComponent))
	{
		if(IsValid(CombatComponent->EquippedWeapon))
		{
			DropOrDestroyWeapon(CombatComponent->EquippedWeapon);
		}

		if(IsValid(CombatComponent->SecondaryWeapon))
		{
			DropOrDestroyWeapon(CombatComponent->SecondaryWeapon);
		}
	}
}

void AEndlessBetrayalCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if(bIsEliminated) return;

	const bool bDamageFullyAbsorbed = (Shield - Damage > 0.0f);
	float DamageAfterShieldAbsorption = Damage;
	if(Shield > 0.0f)
	{
		if(!bDamageFullyAbsorbed)
		{
			DamageAfterShieldAbsorption = FMath::Abs(Shield - Damage);
		}
		Shield = FMath::Clamp(Shield - Damage, 0.0f, MaxShield);
		UpdateShieldHUD();
	}

	if(bDamageFullyAbsorbed) return;
	
	Health = FMath::Clamp(Health - DamageAfterShieldAbsorption, 0.0f, MaxHealth);
	UpdateHealthHUD();
	PlayHitReactMontage();

	if(Health > 0.0f) return;

	AEndlessBetrayalGameMode* EndlessBetrayalGameMode = GetWorld()->GetAuthGameMode<AEndlessBetrayalGameMode>();
	if(IsValid(EndlessBetrayalGameMode))
	{
		EndlessBetrayalPlayerController = !IsValid(EndlessBetrayalPlayerController) ? Cast<AEndlessBetrayalPlayerController>(Controller) : EndlessBetrayalPlayerController;
		AEndlessBetrayalPlayerController* AttackerController = Cast<AEndlessBetrayalPlayerController>(InstigatedBy);
		if(IsValid(EndlessBetrayalPlayerController) && IsValid(AttackerController))
		{
			EndlessBetrayalGameMode->OnPlayerEliminated(this, EndlessBetrayalPlayerController, AttackerController);
		}
	}
}

void AEndlessBetrayalCharacter::MoveForward(float Value)
{
	if(bShouldDisableGameplayInput) return;

	if ((Controller != nullptr) && (Value != 0.0f))
	{
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.0f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));	//return a vector representing the direction of the YawRotation
		AddMovementInput(Direction, Value);	//multiplying the direction here would multiply the speed
	}
}

void AEndlessBetrayalCharacter::MoveRight(float Value)
{
	if(bShouldDisableGameplayInput) return;

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
		ServerEquipButtonPressed();	//Whether we are on the client or server, this will be executed on the server
	}
}

void AEndlessBetrayalCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent)		
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void AEndlessBetrayalCharacter::SwapWeaponMouseWheelRolled()
{
	ServerSwapWeaponMouseWheelRolled();
}


void AEndlessBetrayalCharacter::ServerSwapWeaponMouseWheelRolled_Implementation()
{
	if(IsValid(CombatComponent))
	{
		CombatComponent->SwapWeapons();
	}
}

void AEndlessBetrayalCharacter::AimButtonPressed()
{
	if(bShouldDisableGameplayInput) return;

	if (CombatComponent)
	{
		CombatComponent->SetAiming(true);
	}
}
void AEndlessBetrayalCharacter::AimButtonReleased()
{
	if(bShouldDisableGameplayInput) return;

	if (CombatComponent)
	{
		CombatComponent->SetAiming(false);
	}
}

void AEndlessBetrayalCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.0f && !IsLocallyControlled())
	{
		//Mapping pitch from the range 270-360 to the range -90 - 0
		FVector2D InRange(270.0f, 360.0);
		FVector2D OutRange(-90.0f, 0.0f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
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
		bShouldRotateRootBone = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.0f || bIsInAir)	//Running or jumping
	{
		StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		AO_Yaw = 0.0f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		bShouldRotateRootBone = false;
	}

	CalculateAO_Pitch();
}

void AEndlessBetrayalCharacter::SimProxiesTurn()
{
	if(!IsValid(CombatComponent) || !IsValid(CombatComponent->EquippedWeapon)) return;

	FVector Velocity = GetVelocity();
	Velocity.Z = 0.0f;
	float Speed = Velocity.Size();

	bShouldRotateRootBone = false;
	if(Speed > 0.0f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	CalculateAO_Pitch();
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;
	
	if(FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if(ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if(ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	
}

void AEndlessBetrayalCharacter::Jump()
{
	if(bShouldDisableGameplayInput) return;
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
	if(bShouldDisableGameplayInput) return;

	if(ensureAlways(IsValid(CombatComponent)))
	{
		CombatComponent->FireButtonPressed(true);
	}
}

void AEndlessBetrayalCharacter::FireButtonReleased()
{
	if(bShouldDisableGameplayInput) return;

	if(ensureAlways(IsValid(CombatComponent)))
	{
		CombatComponent->FireButtonPressed(false);
	}
}

void AEndlessBetrayalCharacter::CrouchButtonPressed()
{
	if(bShouldDisableGameplayInput) return;

	if (bIsCrouched)			//Inherited public variable
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AEndlessBetrayalCharacter::ReloadButtonPressed()
{
	if(bShouldDisableGameplayInput) return;

	if(IsValid(CombatComponent))
	{
		CombatComponent->Reload();
	}
}

void AEndlessBetrayalCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if(DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void AEndlessBetrayalCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &AEndlessBetrayalCharacter::UpdateDissolveMaterial);
	if(IsValid(DissolveCurve) && IsValid(DissolveTimeline))
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void AEndlessBetrayalCharacter::OnRep_Health(float LastHealth)
{
	UpdateHealthHUD();
	if(!bIsEliminated && (LastHealth > Health))
	{
		PlayHitReactMontage();
	}
}

void AEndlessBetrayalCharacter::OnRep_Shield(float LastShield)
{
	UpdateShieldHUD();
	if(LastShield > Shield)
	{
		PlayHitReactMontage();
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

void AEndlessBetrayalCharacter::HideCameraWhenCharacterClose()
{
	if(!IsLocallyControlled()) return;

	const bool bShouldBeVisible = IsValid(FollowCamera) && (FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold;

	GetMesh()->SetVisibility(!bShouldBeVisible);
	if(IsValid(CombatComponent) && IsValid(CombatComponent->EquippedWeapon) && IsValid(CombatComponent->EquippedWeapon->GetWeaponMesh()))
	{
		CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = bShouldBeVisible;
		
		if(IsValid(CombatComponent->SecondaryWeapon) && IsValid(CombatComponent->SecondaryWeapon->GetWeaponMesh()))
		{
			CombatComponent->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = bShouldBeVisible;
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

FVector AEndlessBetrayalCharacter::GetHitTarget()
{
	if (CombatComponent == nullptr) return FVector();
	return CombatComponent->HitTarget;
}

ECombatState AEndlessBetrayalCharacter::GetCombatState() const
{
	if(!IsValid(CombatComponent)) return ECombatState::ECS_MAX;
	return CombatComponent->CombatState;
}
