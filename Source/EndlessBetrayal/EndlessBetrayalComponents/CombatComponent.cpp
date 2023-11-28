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
#include "EndlessBetrayal/Weapon/Projectile.h"
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
	if(!IsValid(Character) || !IsValid(EquippedWeapon)) return;
	
	bIsAiming = bAiming;
	ServerSetAiming(bAiming);
	if (IsValid(Character))
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}

	//Displaying/Hiding Sniper Scope for Client
	if(Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
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
	AutomaticReload();
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
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SMG, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingSniperAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
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
	if(CombatState != ECombatState::ECS_Unoccupied) return;
	
	DropEquippedWeapon();
	
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	
	AttachActorToHand(EquippedWeapon, FName("RightHandSocket"));

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->UpdateHUDAmmo();

	UpdateWeaponCarriedAmmo();
	PlayEquipSound();
	AutomaticReload();
	
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::DropEquippedWeapon()
{
	if(IsValid(EquippedWeapon))
	{
		EquippedWeapon->OnWeaponDropped();
	}
}

void UCombatComponent::AttachActorToHand(AActor* ActorToAttach, FName SocketName)
{
	if(!IsValid(Character) || !IsValid(Character->GetMesh()) || !IsValid(ActorToAttach)) return;
	
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh());
	}
}

void UCombatComponent::UpdateWeaponCarriedAmmo()
{
	if(!IsValid(EquippedWeapon)) return;
	
	if(CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	PlayerController = PlayerController == nullptr ? Cast<AEndlessBetrayalPlayerController>(Character->Controller) : PlayerController;
	if(IsValid(PlayerController))
	{
		PlayerController->UpdateWeaponCarriedAmmo(CarriedAmmo);
	}
}

void UCombatComponent::PlayEquipSound()
{
	if(IsValid(Character) && IsValid(EquippedWeapon) && EquippedWeapon->OnEquipSoundCue)
	{
		UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->OnEquipSoundCue, Character->GetActorLocation());
	}
}

void UCombatComponent::AutomaticReload()
{
	if(IsValid(EquippedWeapon) && EquippedWeapon->bAllowAutomaticReload && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::Reload()
{
	const bool IsWeaponFullyLoaded = IsValid(EquippedWeapon) && !EquippedWeapon->IsFullyLoaded();
	if(CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && IsWeaponFullyLoaded)
	{
		ServerReload();
	}
}

void UCombatComponent::ThrowGrenade()
{
	if((CombatState != ECombatState::ECS_Unoccupied) || !IsValid(EquippedWeapon)) return;
	if(!IsValid(Character)) return;

	ServerThrowGrenade();
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToHand(EquippedWeapon, FName("RightHandSocket"));
}

void UCombatComponent::MulticastThrowGrenade_Implementation()
{
	if(IsValid(Character))
	{
		SetGrenadeVisibility(true);
		Character->PlayThrowGrenadeMontage();
		if(IsValid(EquippedWeapon))
		{
			const bool IsSingleHandWeapon = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol || EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SMG;
			const FName SocketName = IsSingleHandWeapon ? FName("SingleHandWeaponSocket") : FName("LeftHandSocket");
			AttachActorToHand(EquippedWeapon, SocketName);
		}
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	CombatState = ECombatState::ECS_ThrowingGrenade;
	MulticastThrowGrenade();
}

void UCombatComponent::LaunchGrenade()
{
	SetGrenadeVisibility(false);
	if(IsValid(Character) && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget);
	}
}

void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if(IsValid(Character) && IsValid(GrenadeClass) && IsValid(Character->GetAttachedGrenade()))
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation();
		const FVector ToTarget = Target - StartingLocation;
		
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = Character;
		SpawnParameters.Instigator = Character;
		
		UWorld* World = GetWorld();
		if(IsValid(World))
		{
			World->SpawnActor<AProjectile>(GrenadeClass, StartingLocation, ToTarget.Rotation(), SpawnParameters);
		}
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

void UCombatComponent::SetGrenadeVisibility(bool bShouldBeVisible)
{
	if(IsValid(Character) && IsValid(Character->GetAttachedGrenade()))
	{
		Character->GetAttachedGrenade()->SetVisibility(bShouldBeVisible);
	}
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
		
		AttachActorToHand(EquippedWeapon, FName("RightHandSocket"));

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		
		PlayEquipSound();
	}
}
