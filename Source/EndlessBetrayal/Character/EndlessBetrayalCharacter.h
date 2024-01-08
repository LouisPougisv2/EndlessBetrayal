// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TimelineComponent.h"
#include "EndlessBetrayal/EndlessBetrayalTypes/CombatState.h"
#include "GameFramework/Character.h"
#include "EndlessBetrayal/EndlessBetrayalTypes/TurningInPlace.h"
#include "EndlessBetrayal/Interface/InteractWithCrosshairInterface.h"
#include "EndlessBetrayalCharacter.generated.h"

UCLASS()
class ENDLESSBETRAYAL_API AEndlessBetrayalCharacter : public ACharacter, public IInteractWithCrosshairInterface
{
	GENERATED_BODY()

public:
	AEndlessBetrayalCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void OnRep_ReplicatedMovement() override;
	virtual void Destroyed() override;
	void SpawnDefaultWeapon();
	void UpdateHUDAmmo();
	void UpdateHealthHUD();
	void UpdateShieldHUD();
	void PlayFireMontage(bool bIsAiming);
	void PlayReloadMontage();
	void PlayEliminatedMontage();
	void PlayThrowGrenadeMontage();

	//Reserved for functionalities that'll happen only on the server
	void OnPlayerEliminated();
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastOnPlayerEliminated();

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShouldShowScope);

	/*
	 * Hit boxes used for Server-side rewinds
	 * Note : Head Box's name correspond to the Bone name, hence the 2, 3 for some
	 */

	UPROPERTY(EditAnywhere)
	class UBoxComponent* HeadBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* PelvisBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* SpineBox2;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* SpineBox3;

	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArmLeftBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerArmLeftBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* HandLeftBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* UpperArmRightBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* LowerArmRightBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* HandRightBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* BackpackBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* BlanketBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* ThighLeftBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* CalfLeftBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* FootLeftBox;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* ThighRightBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* CalfRightBox;

	UPROPERTY(EditAnywhere)
	UBoxComponent* FootRightBox;


protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void SwapWeaponMouseWheelRolled();
	void CrouchButtonPressed();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void CalculateAO_Pitch();
	void AimOffset(float DeltaTime);
	void SimProxiesTurn();
	void RotateInPlace(float DeltaTime);
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();
	void GrenadeButtonPressed();
	void DropOrDestroyWeapon(AWeapon* Weapon);
	void DropOrDestroyWeapons();

	UFUNCTION() //NEEDS to be UFUNCTION or we well never get our callback called in response to a damage event 
	void ReceiveDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	//Poll for any relevant classes and initialize our HUD
	void PollInitialize();
	
private: 

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);	//Function called automatically when the variable is replicated

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* BuffComponent;

	/**
	* Default Weapon
	**/

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION(Server, Reliable)
	void ServerSwapWeaponMouseWheelRolled();

	void TurnInPlace(float DeltaTime);

	void HideCameraWhenCharacterClose();

	//Animation Montages
	UPROPERTY(EditAnywhere, Category = "Combat")
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ReloadMontage;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* HitReactionMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* EliminationMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ThrowingGrenadeMontage;

	//End of Animation Montages
	UPROPERTY(EditAnywhere, Category = "Camera")
	float CameraThreshold = 200.0f;
	
	float AO_Yaw;
	float InterpAOYaw;
	float AO_Pitch;
	FRotator StartingAimRotation;
	ETurningInPlace TurningInPlace;
	bool bShouldRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;

	/**
	* Player Health
	**/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.0f;

	//Callback function when Health is updated, only called on the client
	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/**
	* Player Shield
	**/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, VisibleAnywhere, Category = "Player Stats")
	float Shield = 50.0f;

	//Callback function when Shield is updated, only called on the client
	UFUNCTION()
	void OnRep_Shield(float LastShield);
	
	bool bIsEliminated = false;

	FTimerHandle OnPlayerEliminatedTimer;

	UPROPERTY(EditDefaultsOnly)
	float OnPlayerEliminatedDelayTime = 3.0f;

	UFUNCTION()
	void OnPlayerEliminatedCallBack();

	/**
	*	Dissolve Effect
	*/

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	//Dynamic Instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = "Elimination")
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	//Material Instance set on the BP used with the Dynamic Material Instance
	UPROPERTY(EditAnywhere, Category = "Elimination")
	UMaterialInstance* DissolveMaterialInstance;
	//Every TimelineComponent needs a callback called every frame as we're updating the timeline
	UFUNCTION()	//Allows to be bound
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	/**
	*	Elimination Bot
	**/

	UPROPERTY(EditAnywhere)
	class UParticleSystem* EliminationBotEffect;

	UPROPERTY(VisibleAnywhere)
	class UParticleSystemComponent* EliminationBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* EliminationBotSound;

	UPROPERTY()
	class AEndlessBetrayalPlayerState* EndlessBetrayalPlayerState;

	UPROPERTY()
	class AEndlessBetrayalPlayerController* EndlessBetrayalPlayerController;
	
	/**
	*	Grenade
	**/

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;
public:	

	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	FORCEINLINE float GetShield() const { return Shield; }
	FORCEINLINE void SetHealth(float NewHealth) { Health = NewHealth; }
	FORCEINLINE void SetShield(const float NewShield) { Shield = NewShield; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bShouldRotateRootBone; }
	FORCEINLINE UCameraComponent* GetFollowCamera() { return FollowCamera; }
	FORCEINLINE UCombatComponent* GetCombatComponent() { return CombatComponent; }
	FORCEINLINE UBuffComponent* GetBuffComponent() { return BuffComponent; }
	FORCEINLINE bool IsGameplayDisabled() const { return bShouldDisableGameplayInput; }
	AWeapon* GetEquippedWeapon();
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() { return AttachedGrenade; }

	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE bool IsEliminated() const { return bIsEliminated; }
	FVector GetHitTarget();
	
	ECombatState GetCombatState() const;

	UPROPERTY(Replicated)
	bool bShouldDisableGameplayInput = false;
};
