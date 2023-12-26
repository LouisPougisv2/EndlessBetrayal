// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "JumpPickup.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API AJumpPickup : public APickup
{
	GENERATED_BODY()

public:

	AJumpPickup();

protected:

	virtual void OnSphereOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:

	UPROPERTY(EditAnywhere)
	float JumpZVelocityBuff = 2300.0f;

	UPROPERTY(EditAnywhere)
	float JumpBuffTime = 15.0f;
};
