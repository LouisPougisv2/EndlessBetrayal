// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "EndlessBetrayal/Weapon/Weapon.h"
#include "EndlessBetrayal/Character/EndlessBetrayalCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.0f;
	AimWalkSpeed = 450.0f;
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if(IsValid(Character->GetFollowCamera()))
		{
			DefaultFOV =  Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if(Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
	//CarriedAmmo will only replicate to the Owning client
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshair(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshair(DeltaTime);
		ZoomInterpFOV(DeltaTime);
	}
}

void UCombatComponent::SetAiming(bool bAiming)
{
		bIsAiming = bAiming;
		ServerSetAiming(bAiming);
		if (Character)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
		}
		else if (Character)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bAiming)
{
	bIsAiming = bAiming;
	if (Character) //The walk speed was all choppy due to the fact that our local modification wasn't overriding the max walk speed from the character movement component
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::TraceUnderCrosshair(FHitResult& HitResult)
{
	FVector2D ViewPortSize;
	if(IsValid(GetWorld()) && IsValid(GetWorld()->GetGameViewport()))
	{
		GetWorld()->GetGameViewport()->GetViewportSize(ViewPortSize);
	}
	const FVector2D CrosshairLocation(ViewPortSize.X / 2.0f, ViewPortSize.Y / 2.0f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	
	const bool bIsDeprojectScreenToWorldSuccessful = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection);

	if(bIsDeprojectScreenToWorldSuccessful)
	{
		FVector Start = CrosshairWorldPosition;

		if(IsValid(Character))
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.0f);
		}
		
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);

		//Check if we've hit an actor and if the hit actor has the UInteractWithCrosshairInterface implemented
		if(IsValid(HitResult.GetActor()) && HitResult.GetActor()->Implements<UInteractWithCrosshairInterface>())
		{
			const bool bIsHitResultLocallyControlled = Cast<AEndlessBetrayalCharacter>(HitResult.GetActor())->IsLocallyControlled();
			if(!bIsHitResultLocallyControlled)
			{
				HUDTexture.CrosshairColor = FLinearColor::Red;
			}
		}
		else
		{
			HUDTexture.CrosshairColor = FLinearColor::White;
		}
	}
}

void UCombatComponent::SetHUDCrosshair(float DeltaTime)
{
	if(!(IsValid(Character)) || !(IsValid(Character->Controller))) return;
	
	if(!IsValid(PlayerController)) PlayerController = Cast<AEndlessBetrayalPlayerController>(Character->Controller);
	if(IsValid(PlayerController))
	{
		if(!IsValid(HUD)) HUD = Cast<AEndlessBetrayalHUD>(PlayerController->GetHUD());
		if(IsValid(HUD))
		{
			if(IsValid(EquippedWeapon))
			{
				HUDTexture.CrosshairCenter = EquippedWeapon->GetCrosshairCenter();
				HUDTexture.CrosshairTop = EquippedWeapon->GetCrosshairTop();
				HUDTexture.CrosshairRight = EquippedWeapon->GetCrosshairRight();
				HUDTexture.CrosshairBottom = EquippedWeapon->GetCrosshairBottom();
				HUDTexture.CrosshairLeft = EquippedWeapon->GetCrosshairLeft();
			}
			//Calculate the Crosshair spread
			const FVector2d WalkSpeedRange (0.0f, Character->GetCharacterMovement()->MaxWalkSpeed);
			const FVector2d VelocityMultiplierRange(0.0f, 1.0f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.0f;
			
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if(Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 1.8f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.0f, DeltaTime, 10.0f);
			}

			if(IsValid(EquippedWeapon))
			{
				if(bIsAiming)
				{
					CrosshairAimingFactor = FMath::FInterpTo(CrosshairAimingFactor, 0.6f, DeltaTime, EquippedWeapon->GetZoomInterSpeed());
				}
				else
				{
					CrosshairAimingFactor = FMath::FInterpTo(CrosshairAimingFactor, 0.0f, DeltaTime, EquippedWeapon->GetZoomInterSpeed());
				}
			}

			//We want the Crosshair shooting factor to always interp to 0 after shooting, hence the following line
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 5.0f);
			
			HUDTexture.CrosshairSpreadFactor = 0.25 + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimingFactor + CrosshairShootingFactor;
			
			HUD->SetHUDTexture(HUDTexture);
		}
		
	}
}

int32 UCombatComponent::CalculateAmountToReload()
{
	if(!IsValid(EquippedWeapon)) return 0;

	const int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		const int32 AmmoCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		const auto LeastAmmo = FMath::Min(RoomInMag, AmmoCarried);
		return FMath::Clamp(RoomInMag, 0, LeastAmmo);
	}
	
	return 0;
	
}

void UCombatComponent::ZoomInterpFOV(float DeltaTime)
{
	if(!IsValid(EquippedWeapon)) return;

	if(bIsAiming)
	{
		//Zooming varies based on the Equipped Weapon
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterSpeed());
	}
	else
	{
		//When DeZooming, zooming back to normal happens at the same speed no matter the weapon
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	if(IsValid(Character) && IsValid(Character->GetFollowCamera()))
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
	
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bIsFireButtonPressed = bPressed;

	if(bIsFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if(CanFire())
	{
		bCanFire = false;
		ServerFire(HitTarget);

		if(IsValid(EquippedWeapon))
		{
			CrosshairShootingFactor += EquippedWeapon->GetCrosshairShootingFactor();
		}
		StartFireTimer();
	}
}

void UCombatComponent::StartFireTimer()
{
	if(!IsValid(EquippedWeapon) || !IsValid(Character)) return;
	Character->GetWorldTimerManager().SetTimer(FireTimerHandle, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	bCanFire = true;
	if(EquippedWeapon->bIsWeaponAutomatic && bIsFireButtonPressed)
	{
		Fire();
	}
	if(EquippedWeapon->bAllowAutomaticReload && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

bool UCombatComponent::CanFire()
{
	if(!IsValid(EquippedWeapon)) return false;
	return (bCanFire && !EquippedWeapon->IsEmpty() && CombatState != ECombatState::ECS_Reloading);
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	PlayerController = PlayerController == nullptr ? Cast<AEndlessBetrayalPlayerController>(Character->Controller) : PlayerController;
	if(IsValid(PlayerController))
	{
		PlayerController->UpdateWeaponCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmoAmount);
}

void UCombatComponent::UpdateAmmoValues()
{
	const int32 ReloadAmount = CalculateAmountToReload();
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	PlayerController = PlayerController == nullptr ? Cast<AEndlessBetrayalPlayerController>(Character->Controller) : PlayerController;
	if(IsValid(PlayerController))
	{
		PlayerController->UpdateWeaponCarriedAmmo(CarriedAmmo);
	}
	
	EquippedWeapon->UpdateAmmo(ReloadAmount);
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	//Runs on Server and all clients when call from the server
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	//TODO : Add Ammo check here when adding Ammo in the future
	if(!IsValid(EquippedWeapon)) return;
	
	if(IsValid(Character) && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	if(IsValid(EquippedWeapon))
	{
		EquippedWeapon->OnWeaponDropped();
	}
	
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->UpdateHUDAmmo();

	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	PlayerController = PlayerController == nullptr ? Cast<AEndlessBetrayalPlayerController>(Character->Controller) : PlayerController;
	if(IsValid(PlayerController))
	{
		PlayerController->UpdateWeaponCarriedAmmo(CarriedAmmo);
	}

	if(EquippedWeapon->OnEquipSoundCue)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->OnEquipSoundCue, Character->GetActorLocation());
	}

	//Automatically reload equipped weapon is empty
	if(EquippedWeapon->bAllowAutomaticReload && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
	
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::Reload()
{
	if(CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading)
	{
		ServerReload();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if(!IsValid(Character) || !IsValid(EquippedWeapon) || (CalculateAmountToReload() == 0) ) return;
	
	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
}

void UCombatComponent::FinishReloading()
{
	if(!IsValid(Character)) return;
	if(Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if(bIsFireButtonPressed)
	{
		Fire();
	}
}


void UCombatComponent::OnRep_CombatState()
{
	switch(CombatState)
	{
		case ECombatState::ECS_Reloading:
			HandleReload();
			break;
		case ECombatState::ECS_Unoccupied:
			if(bIsFireButtonPressed)
			{
				Fire();
			}
		default:
			break;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		
		if(EquippedWeapon->OnEquipSoundCue)
		{
			UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->OnEquipSoundCue, Character->GetActorLocation());
		}
	}
}
