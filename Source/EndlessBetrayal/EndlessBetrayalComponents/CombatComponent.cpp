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
#include "EndlessBetrayal/Weapon/Shotgun.h"
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
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon);
	DOREPLIFETIME(UCombatComponent, bIsAiming);
	//CarriedAmmo will only replicate to the Owning client
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME_CONDITION(UCombatComponent, AmountOfGrenades, COND_OwnerOnly);
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

	if(Character->IsLocallyControlled())
	{
		bAimButtonPressed = bIsAiming;
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

void UCombatComponent::OnRep_Aiming()
{
	if(IsValid(Character) && Character->IsLocallyControlled())
	{
		bIsAiming = bAimButtonPressed;
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

		if(IsValid(EquippedWeapon))
		{
			CrosshairShootingFactor += EquippedWeapon->GetCrosshairShootingFactor();

			switch (EquippedWeapon->FireType)
			{
				case EFireType::EFT_ProjectileWeapon:
					FireProjectileWeapon();
					break;
				case EFireType::EFT_HitScanWeapon:
					FireHitScanWeapon();
					break;
				case EFireType::EFT_ShotgunWeapon:
					FireShotgun();
					break;
				default:
					break;
			}
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
	//If we pass this line, we're either on the server or on a client who is not controlling
	if(IsValid(Character) && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalFire(TraceHitTarget);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if(!IsValid(EquippedWeapon)) return;
	
	if(IsValid(Character) && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bIsAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	MulticastShotgunFire(TraceHitTargets);
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	//If we pass this line, we're either on the server or on a client who is not controlling
	if(IsValid(Character) && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	LocalShotgunFire(TraceHitTargets);
}

void UCombatComponent::LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if(!IsValid(EquippedWeapon) || !IsValid(Character)) return;
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if(!IsValid(Shotgun)) return;
	
	if(CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bIsAiming);
		Shotgun->FireShotgun(TraceHitTargets);
	}
}

void UCombatComponent::FireProjectileWeapon()
{
	if(IsValid(EquippedWeapon) && Character)
	{
		//Getting the new updated HitTarget with the Scatter applied
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->GetTraceEndWithScatter(HitTarget) : HitTarget;
		if(!Character->HasAuthority())	//It doesn't make sense to call LocalFire if the player is the server
		{
			LocalFire(HitTarget); //Local Fire to directly apply cosmetic effect locally
		}
		ServerFire(HitTarget); //Server Fire to authoritatively to apply damages
	}
}

void UCombatComponent::FireHitScanWeapon()
{
	if(IsValid(EquippedWeapon) && Character)
	{
		//Getting the new updated HitTarget with the Scatter applied
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->GetTraceEndWithScatter(HitTarget) : HitTarget;
		if(!Character->HasAuthority())	//It doesn't make sense to call LocalFire if the player is the server
		{
			LocalFire(HitTarget); //Local Fire to directly apply cosmetic effect locally
		}
		ServerFire(HitTarget); //Server Fire to authoritatively to apply damages
	}
}

void UCombatComponent::FireShotgun()
{
	if(IsValid(EquippedWeapon))
	{
		AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
		if(!IsValid(Shotgun)) return;
		
		TArray<FVector_NetQuantize> HitTargets;
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);

		if(!Character->HasAuthority())
		{
			LocalShotgunFire(HitTargets);
		}
		ServerShotgunFire(HitTargets);
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	if(CombatState != ECombatState::ECS_Unoccupied) return;

	if(IsValid(EquippedWeapon) && !IsValid(SecondaryWeapon))
	{
		EquipSecondaryWeapon(WeaponToEquip);
	}
	else
	{
		EquipPrimaryWeapon(WeaponToEquip);
	}
	
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::SwapWeapons()
{
	if(!ShouldSwapWeapon() || CombatState == ECombatState::ECS_Reloading) return;

	AWeapon* TemporaryWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TemporaryWeapon;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToSocket(EquippedWeapon, FName("RightHandSocket"));
	EquippedWeapon->UpdateHUDAmmo();
	UpdateWeaponCarriedAmmo();
	PlayEquipSound(EquippedWeapon);

	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToSocket(SecondaryWeapon, FName("BackpackSocket"));
}

void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	DropEquippedWeapon();
	
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	
	AttachActorToSocket(EquippedWeapon, FName("RightHandSocket"));

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->UpdateHUDAmmo();

	UpdateWeaponCarriedAmmo();
	PlayEquipSound(EquippedWeapon);
	AutomaticReload();
}

void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	SecondaryWeapon = WeaponToEquip;
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	SecondaryWeapon->SetOwner(Character);
	
	AttachActorToSocket(SecondaryWeapon, FName("BackpackSocket"));
	
	PlayEquipSound(SecondaryWeapon);
}

void UCombatComponent::DropEquippedWeapon()
{
	if(IsValid(EquippedWeapon))
	{
		//TODO: If-check to potentially remove when Switching weapon will be implemented
		if(EquippedWeapon->bIsDefaultWeapon)
		{
			EquippedWeapon->Destroy();
		}
		else
		{
			EquippedWeapon->OnWeaponDropped();
		}
	}
}

void UCombatComponent::AttachActorToSocket(AActor* ActorToAttach, FName SocketName)
{
	if(!IsValid(Character) || !IsValid(Character->GetMesh()) || !IsValid(ActorToAttach)) return;
	
	const USkeletalMeshSocket* MeshSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (MeshSocket)
	{
		MeshSocket->AttachActor(ActorToAttach, Character->GetMesh());
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

void UCombatComponent::PlayEquipSound(AWeapon* WeaponToEquip)
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

void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoToPickup)
{
	if(CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoToPickup, 0, MaxCarriedAmmo);

		UpdateWeaponCarriedAmmo();
	}

	//If weapon is Equipped Weapon is empty when picking up corresponding Ammo ->Automatic reload
	if(IsValid(EquippedWeapon) && EquippedWeapon->GetWeaponType() == WeaponType && EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::ThrowGrenade()
{
	if(AmountOfGrenades == 0) return;
	if((CombatState != ECombatState::ECS_Unoccupied) || !IsValid(EquippedWeapon)) return;
	if(!IsValid(Character)) return;

	ServerThrowGrenade();
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToSocket(EquippedWeapon, FName("RightHandSocket"));
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
			AttachActorToSocket(EquippedWeapon, SocketName);
		}
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if(AmountOfGrenades == 0) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	MulticastThrowGrenade();
	AmountOfGrenades = FMath::Clamp(--AmountOfGrenades, 0, MaxAmountOfGrenades);

	if(IsValid(Character) && Character->IsLocallyControlled())
	{
		UpdateHUDGrenadeAmount();
	}
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

void UCombatComponent::UpdateHUDGrenadeAmount()
{
	if(!(IsValid(Character)) || !(IsValid(Character->Controller))) return;
	
	if(!IsValid(PlayerController)) PlayerController = Cast<AEndlessBetrayalPlayerController>(Character->Controller);
	if(IsValid(PlayerController))
	{
		PlayerController->UpdateGrenadesAmmo(AmountOfGrenades);
	}
}

void UCombatComponent::OnRep_GrenadesAmount()
{
	UpdateHUDGrenadeAmount();
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
		EquippedWeapon->ToggleCustomDepth(false);
		EquippedWeapon->UpdateHUDAmmo();
		
		AttachActorToSocket(EquippedWeapon, FName("RightHandSocket"));

		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
		
		PlayEquipSound(EquippedWeapon);
	}
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
		
		AttachActorToSocket(SecondaryWeapon, FName("BackpackSocket"));
		
		PlayEquipSound(SecondaryWeapon);
	}
}
