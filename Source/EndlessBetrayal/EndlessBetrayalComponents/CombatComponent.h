// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
protected:

	virtual void BeginPlay() override;
	void SetAiming(bool bAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
		
	void FireButtonPressed(bool bPressed);

private:

	UPROPERTY()
	class AEndlessBetrayalCharacter* Character;		//Set up in the PostInitializeComponent function in the EndlessBetrayalCharacter
	
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

public:	

		
};
