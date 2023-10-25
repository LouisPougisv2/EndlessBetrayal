// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "EndlessBetrayal/EndlessBetrayalTypes/CombatState.h"
#include "EndlessBetrayal/HUD/EndlessBetrayalHUD.h"
#include "EndlessBetrayal/Weapon/WeaponTypes.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.0f

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
	void FireButtonPressed(bool bPressed);
	void Reload();
	
protected:

	virtual void BeginPlay() override;
	void SetAiming(bool bAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	void Fire();


	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	UFUNCTION()
	void HandleReload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	
	void TraceUnderCrosshair(FHitResult& HitResult);

	void SetHUDCrosshair(float DeltaTime);

	int32 CalculateAmountToReload();
private:

	UPROPERTY()
	class AEndlessBetrayalCharacter* Character;		//Set up in the PostInitializeComponent function in the EndlessBetrayalCharacter

	UPROPERTY()
	class AEndlessBetrayalPlayerController* PlayerController;

	UPROPERTY()
	class AEndlessBetrayalHUD* HUD;
	
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bIsAiming;

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

	UFUNCTION()
	void OnRep_CarriedAmmo();

	UPROPERTY()
	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmoAmount = 45	;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmoAmount = 0	;
	
	UFUNCTION()
	void InitializeCarriedAmmo();

	UFUNCTION()
	void UpdateAmmoValues();

	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();
};
