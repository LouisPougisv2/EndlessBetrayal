// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "EndlessBetrayal/Weapon/WeaponTypes.h"
#include "AmmoPickup.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API AAmmoPickup : public APickup
{
	GENERATED_BODY()

protected:
	
	virtual void OnSphereOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:

	UPROPERTY(EditAnywhere, Category = "Pickup settings")
	int32 AmmoAmount = 30;

	UPROPERTY(EditAnywhere, Category = "Pickup settings")
	EWeaponType AmmoWeaponType;
};
