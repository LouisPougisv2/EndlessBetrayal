// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API AFlag : public AWeapon
{
	GENERATED_BODY()

public:

	AFlag();
	virtual void BeginPlay() override;
	void ResetFlag();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastResetFlagTransform();
	
	FORCEINLINE FTransform GetFlagInitialTransform() const { return InitialTransform; }

protected:
	
	virtual void HandleWeaponEquipped() override;
	
private:
	
	FTransform InitialTransform;
};
