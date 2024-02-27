// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EndlessBetrayal/EndlessBetrayalTypes/CombatState.h"
#include "EndlessBetrayal/HUD/EndlessBetrayalHUD.h"
#include "EndlessBetrayal/Weapon/WeaponTypes.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ENDLESSBETRAYAL_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UCombatComponent();
	friend class AEndlessBetrayalCharacter;	//As Combat component will need to access a lot of variable from The character
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void SwapWeapons();
	void FireButtonPressed(bool bPressed);
	void Reload();
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoToPickup);

	FORCEINLINE bool IsAiming() const { return bIsAiming; }

	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	FORCEINLINE int32 GetGrenadesAmount() const { return AmountOfGrenades; };
	FORCEINLINE bool ShouldSwapWeapon() const { return EquippedWeapon && SecondaryWeapon && !bIsHoldingFlag; }
	
protected:

	virtual void BeginPlay() override;
	void SetAiming(bool bAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming);

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
	void EquipFlag(AWeapon* FlagToEquip);
	void DropEquippedWeapon();
	void AttachActorToSocket(AActor* ActorToAttach, FName SocketName);
	void UpdateWeaponCarriedAmmo();
	void PlayEquipSound(AWeapon* WeaponToEquip);
	void AutomaticReload();
	
	UFUNCTION()
	void OnRep_EquippedWeapon();

	UFUNCTION()
	void OnRep_SecondaryWeapon();
	
	void Fire();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void LocalFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	void LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	
	void FireProjectileWeapon();
	void FireHitScanWeapon();
	void FireShotgun();

	
	UFUNCTION(Server, Reliable)
	void ServerReload();

	UFUNCTION()
	void HandleReload();

	UFUNCTION(BlueprintCallable)
	void SetGrenadeVisibility(bool bShouldBeVisible);
	
	void ThrowGrenade();
	
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastThrowGrenade();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void FinishSwap();
	
	void TraceUnderCrosshair(FHitResult& HitResult);

	void SetHUDCrosshair(float DeltaTime);

	int32 CalculateAmountToReload();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;
	
private:

	UPROPERTY()
	class AEndlessBetrayalCharacter* Character;		//Set up in the PostInitializeComponent function in the EndlessBetrayalCharacter

	UPROPERTY()
	class AEndlessBetrayalPlayerController* PlayerController;

	UPROPERTY()
	class AEndlessBetrayalHUD* HUD;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	UPROPERTY()
	AWeapon* Flag = nullptr;

	//Used to zoom in and out
	UPROPERTY(Replicated)
	bool bIsAiming = false;
	
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	UPROPERTY()
	bool bIsFireButtonPressed = false;

	/**
	 * HUD & Crosshair
	**/
	UPROPERTY()
	float CrosshairVelocityFactor;

	UPROPERTY()
	float CrosshairInAirFactor;

	UPROPERTY()
	float CrosshairAimingFactor;

	UPROPERTY()
	float CrosshairShootingFactor;

	FVector HitTarget;

	FHUDTextures HUDTexture;
	
	/**
	* Aiming & FOV
	**/

	//Field of view when not aiming, set to camera's FOV in Begin Play
	float DefaultFOV;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFieldOfView = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomInterpSpeed = 20.0f;

	//FUnction to handle the Zoom in when aiming
	void ZoomInterpFOV(float DeltaTime);

	/**
	*	Automatic Fire handle
	**/

	FTimerHandle FireTimerHandle;

	bool bCanFire = true;
	void StartFireTimer();
	void FireTimerFinished();
	bool CanFire();

	//Carried Ammo for the Currently equipped weapon
	UPROPERTY(EditAnywhere, ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 100;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	UPROPERTY()
	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 4;

	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 15;

	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 50;

	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 7;
	
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 6;
	
	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 5;

	UPROPERTY(ReplicatedUsing = OnRep_GrenadesAmount, EditAnywhere)
	int32 AmountOfGrenades = 2;

	UPROPERTY(EditAnywhere)
	int32 MaxAmountOfGrenades = 2;
	
	
	UFUNCTION()
	void InitializeCarriedAmmo();

	UFUNCTION()
	void UpdateAmmoValues();

	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	UFUNCTION()
	void UpdateHUDGrenadeAmount();
	
	UFUNCTION()
	void OnRep_GrenadesAmount();

	UPROPERTY(ReplicatedUsing = OnRep_HoldingFlag)
	bool bIsHoldingFlag = false;

	UFUNCTION()
	void OnRep_HoldingFlag();
};
