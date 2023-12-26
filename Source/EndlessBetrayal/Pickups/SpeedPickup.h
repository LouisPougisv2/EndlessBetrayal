// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "SpeedPickup.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API ASpeedPickup : public APickup
{
	GENERATED_BODY()

public:

	ASpeedPickup();

protected:

	virtual void OnSphereOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:

	UPROPERTY(EditAnywhere)
	float BaseSpeedBuff = 1400.0f;

	UPROPERTY(EditAnywhere)
	float CrouchedSpeedBuff = 850.0f;

	UPROPERTY(EditAnywhere)
	float SpeedingBuffTime = 8.0f;
};
