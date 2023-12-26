// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "ShieldPickup.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API AShieldPickup : public APickup
{
	GENERATED_BODY()

public :

	AShieldPickup();

protected:

	virtual void OnSphereOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
private:

	UPROPERTY(EditAnywhere)
	float ShieldAmount = 50.0f;

	UPROPERTY(EditAnywhere)
	float RestoringShieldTime = 3.0f;

};
