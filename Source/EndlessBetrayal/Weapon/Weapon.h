// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.h"
#include "EndlessBetrayal/EndlessBetrayalTypes/Team.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)			//so we can use this enum as a type in blueprint
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName= "DefaultMax")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScanWeapon UMETA(DisplayName = "HitScan Weapon"),
	EFT_ProjectileWeapon UMETA(DisplayName = "Projectile Weapon"),
	EFT_ShotgunWeapon UMETA(DisplayName = "Shotgun Weapon"),

	EFT_MAX UMETA(DsplayName = "DefaultMax")
};

UCLASS()
class ENDLESSBETRAYAL_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;

	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);
	bool IsFullyLoaded();
	void UpdateHUDAmmo();
	virtual void OnWeaponDropped();
	void UpdateAmmo(int32 AmmoAmount);
	virtual FVector GetTraceEndWithScatter(const FVector& HitTarget);

	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }

	//Enable / Disable Custom Depth
	void ToggleCustomDepth(bool bEnable);

	bool bIsDefaultWeapon = false;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bIsWeaponAutomatic = true;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")	
	bool bUseScatter = false;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	float FireDelay = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAllowAutomaticReload = false;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class USoundCue* OnEquipSoundCue;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	EFireType FireType;

protected:
	virtual void BeginPlay() override;

	virtual void OnWeaponStateSet();
	virtual void HandleWeaponEquipped();
	virtual void HandleWeaponDropped();
	virtual void HandleWeaponEquippedSecondary();
	
	UFUNCTION()
	virtual void OnSphereOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);
	
	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 ServerAmmoToAdd);
	
	void SpendRound();

	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);

	/** This function will bind or remove the Delegate of type FHighPingDelegate
	* @params if bShouldBind == true -> Bind, else remove
	*/
	void BindOrRemovePingTooHighDelegate();

	FORCEINLINE class AEndlessBetrayalCharacter* GetWeaponOwnerCharacter() const { return WeaponOwnerCharacter; }

	UPROPERTY(EditAnywhere)
	float Damage = 20.0f;

	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.0f;
	
	//The number of unprocessed server request for Ammo, incremented in spend round, decremented in Client Update Ammo
	int32 SequenceNumber = 0;
	
	/*
	* Trace End With Scatter
	*/

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.0f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.0f;

	UPROPERTY(Replicated, EditAnywhere, Category = "Weapon Properties")
	bool bUseServerSideRewind = false;

	UPROPERTY()
	AEndlessBetrayalCharacter* WeaponOwnerCharacter;

	UPROPERTY()
	class AEndlessBetrayalPlayerController* WeaponOwnerController;
	
private:

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UPROPERTY(EditAnywhere, Category = "Combat")
	int32 AmmoAmount;

	UPROPERTY(EditAnywhere, Category = "Combat")
	int32 MagCapacity;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	EWeaponType WeaponType;

	UPROPERTY(EditAnywhere)
	ETeam Team;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TSubclassOf<class ABulletCasing> BulletCasingClass;

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	UTexture2D* CrosshairCenter;

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	UTexture2D* CrosshairTop;

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	UTexture2D* CrosshairBottom;

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	UTexture2D* CrosshairLeft;

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	UTexture2D* CrosshairRight;

	/**
	*	Zoomed FOV while Zooming
	**/

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float ZoomedFOV = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float ZoomInterpSpeed = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float CrosshairShootingFactor = 0.75f;
	
	
public:	

	void SetWeaponState(EWeaponState NewState);
	void SetWeaponMesh(USkeletalMeshComponent* InWeapon);
	FORCEINLINE USphereComponent* GetAreaSphere() const	{ return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE UTexture2D* GetCrosshairCenter() const { return CrosshairCenter; }
	FORCEINLINE UTexture2D* GetCrosshairTop() const { return CrosshairTop; }
	FORCEINLINE UTexture2D* GetCrosshairBottom() const { return CrosshairBottom; }
	FORCEINLINE UTexture2D* GetCrosshairLeft() const { return CrosshairLeft; }
	FORCEINLINE UTexture2D* GetCrosshairRight() const { return CrosshairRight; }
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }

	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE float GetCrosshairShootingFactor() const { return CrosshairShootingFactor; }
	FORCEINLINE bool IsEmpty() const { return AmmoAmount <= 0; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return AmmoAmount; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
};
