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
#include "EndlessBetrayal/HUD/EndlessBetrayalHUD.h"
#include "EndlessBetrayal/PlayerController/EndlessBetrayalPlayerController.h"


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
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
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

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bIsFireButtonPressed = bPressed;

	if(bIsFireButtonPressed)
	{
		FHitResult HitResult;
		TraceUnderCrosshair(HitResult);
		ServerFire(HitResult.ImpactPoint);

		if(IsValid(EquippedWeapon))
		{
			CrosshairShootingFactor += 0.75f;
		}
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
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility);
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
			FHUDTextures TempHUDTexture;
			if(IsValid(EquippedWeapon))
			{
				TempHUDTexture.CrosshairCenter = EquippedWeapon->GetCrosshairCenter();
				TempHUDTexture.CrosshairTop = EquippedWeapon->GetCrosshairTop();
				TempHUDTexture.CrosshairRight = EquippedWeapon->GetCrosshairRight();
				TempHUDTexture.CrosshairBottom = EquippedWeapon->GetCrosshairBottom();
				TempHUDTexture.CrosshairLeft = EquippedWeapon->GetCrosshairLeft();
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
			
			TempHUDTexture.CrosshairSpreadFactor = 0.25 + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimingFactor + CrosshairShootingFactor;
			
			HUD->SetHUDTexture(TempHUDTexture);
		}
		
	}
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

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	//Runs on Server and all clients when call from the server
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	//TODO : Add Ammo check here when adding Ammo in the future
	if(!IsValid(EquippedWeapon)) return;
	
	if(IsValid(Character))
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	EquippedWeapon->SetOwner(Character);

	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}