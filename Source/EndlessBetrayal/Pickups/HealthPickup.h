// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 
 */
UCLASS()
class ENDLESSBETRAYAL_API AHealthPickup : public APickup
{
	GENERATED_BODY()

public:

	AHealthPickup();

protected:

	virtual void OnSphereOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	virtual void Destroyed() override;
private:

	UPROPERTY(EditAnywhere)
	float HealthAmount = 75.0f;

	UPROPERTY(EditAnywhere)
	float HealingTime = 3.0f;

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComponent;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;
};
